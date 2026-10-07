// CrashDiag.cpp -- self-contained startup/crash diagnostics for the MinGW x64 build.
// All logging uses raw CreateFileA/WriteFile (no stdio, no heap) so it survives heap
// corruption; module attribution walks the PEB loader lists (no loader-lock APIs) so
// it survives crashes during DLL loads.
//
// Captured in one run:
//   - access violations / illegal instruction / stack overflow (VEH): codes, fault
//     address, arg registers, real stack unwind (RtlVirtualUnwind), stack scrape,
//     module list, tail of DebugLogFile.txt + dx8vk.log
//   - uncaught C++ exceptions (std::set_terminate): type + what() + stack
//   - UCRT invalid-parameter aborts: logged instead of silent death
//   - fatal signals (SIGABRT/SEGV/FPE/ILL)
//   - clean exits (atexit breadcrumb) and WinMain's swallowed catch(...)

#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <exception>
#include <typeinfo>
#include <cxxabi.h>
#include <stddef.h>

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - (size_t)&((type*)0)->field))
#endif

static ULONG64 g_imageBase = 0;
static ULONG64 g_imageSize = 0;
static char    g_logPath[MAX_PATH] = { 0 };

static void CrashDiagInit(void)
{
	if (g_imageBase) return;
	HMODULE exe = GetModuleHandleA(NULL);
	g_imageBase = (ULONG64)exe;
	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)exe;
	IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)exe + dos->e_lfanew);
	g_imageSize = nt->OptionalHeader.SizeOfImage;

	char dir[MAX_PATH];
	DWORD n = GetModuleFileNameA(exe, dir, MAX_PATH - 16);
	if (n == 0 || n >= MAX_PATH - 16) {
		lstrcpyA(g_logPath, "RTS-crash.log");
	} else {
		while (n > 0 && dir[n-1] != '\\' && dir[n-1] != '/') --n;
		dir[n] = 0;
		wsprintfA(g_logPath, "%sRTS-crash.log", dir);
	}
}

static HANDLE cdStallLog(void);
static HANDLE OpenLogRaw(void)
{
	CrashDiagInit();
	HANDLE h = CreateFileA(g_logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) SetFilePointer(h, 0, NULL, FILE_END);
	return h;
}

static void WriteStr(HANDLE h, const char* s0)
{
	DWORD wr;
	WriteFile(h, s0, (DWORD)lstrlenA(s0), &wr, NULL);
}

static void WriteHexU(HANDLE h, ULONG64 v)
{
	char buf[20];
	static const char* hx = "0123456789abcdef";
	buf[0] = '0'; buf[1] = 'x';
	int i = 2;
	for (int b = 60; b >= 0; b -= 4) buf[i++] = hx[(v >> b) & 0xf];
	buf[18] = '\n'; buf[19] = 0;
	WriteStr(h, buf);
}

static BOOL IsReadable(ULONG64 addr, SIZE_T len)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)) != sizeof(mbi)) return FALSE;
	if (mbi.State != MEM_COMMIT) return FALSE;
	if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return FALSE;
	ULONG64 regionEnd = (ULONG64)mbi.BaseAddress + mbi.RegionSize;
	return (addr >= (ULONG64)mbi.BaseAddress && addr + len <= regionEnd);
}

// Full real layout of LDR_DATA_TABLE_ENTRY (mingw's header hides fields behind
// Reserved arrays; we need SizeOfImage and DllName).
struct LdrEntryFull {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitOrderModuleList;
	void* DllBase;
	void* EntryPoint;
	ULONG SizeOfImage;
	ULONG Flags;
	struct { USHORT Len; USHORT Max; wchar_t* Buf; } FullDllName;
	struct { USHORT Len; USHORT Max; wchar_t* Buf; } DllName;
};

static BOOL ModuleNameFor(ULONG64 addr, char* out, int outSz, ULONG64* modBase)
{
	_PEB* peb = (_PEB*)__readgsqword(0x60);
	if (!peb || !peb->Ldr) return FALSE;
	PEB_LDR_DATA* ldr = peb->Ldr;
	ULONG64 head = (ULONG64)&ldr->InMemoryOrderModuleList;
	ULONG64 link = (ULONG64)ldr->InMemoryOrderModuleList.Flink;
	for (int guard = 0; guard < 512 && link && link != head && IsReadable(link, sizeof(LdrEntryFull)); ++guard) {
		LdrEntryFull* e = (LdrEntryFull*)((char*)link - offsetof(LdrEntryFull, InMemoryOrderLinks));
		ULONG64 b = (ULONG64)e->DllBase;
		if (addr >= b && addr < b + e->SizeOfImage) {
			if (modBase) *modBase = b;
			int n = (int)e->DllName.Len / 2;
			if (n > outSz - 1) n = outSz - 1;
			if (n > 0 && IsReadable((ULONG64)e->DllName.Buf, n * 2)) {
				for (int i = 0; i < n; ++i) out[i] = (char)e->DllName.Buf[i];
				out[n] = 0;
			} else out[0] = 0;
			return TRUE;
		}
		link = (ULONG64)e->InMemoryOrderLinks.Flink;
	}
	return FALSE;
}

static void LogFrame(HANDLE h, const char* tag, ULONG64 addr)
{
	char line[128];
	if (addr >= g_imageBase && addr < g_imageBase + g_imageSize) {
		wsprintfA(line, "%s RTS.exe+0x%x\n", tag, (unsigned int)(addr - g_imageBase));
		WriteStr(h, line);
		return;
	}
	char name[64]; ULONG64 mb = 0;
	if (ModuleNameFor(addr, name, sizeof(name), &mb)) {
		wsprintfA(line, "%s %s+0x%x\n", tag, name, (unsigned int)(addr - mb));
		WriteStr(h, line);
		return;
	}
	wsprintfA(line, "%s ", tag);
	WriteStr(h, line);
	WriteHexU(h, addr);
}

static void DumpModules(HANDLE h)
{
	WriteStr(h, "  modules:\n");
	_PEB* peb = (_PEB*)__readgsqword(0x60);
	if (!peb || !peb->Ldr) { WriteStr(h, "  (no peb)\n"); return; }
	PEB_LDR_DATA* ldr = peb->Ldr;
	ULONG64 head = (ULONG64)&ldr->InMemoryOrderModuleList;
	ULONG64 link = (ULONG64)ldr->InMemoryOrderModuleList.Flink;
	for (int guard = 0; guard < 512 && link && link != head && IsReadable(link, sizeof(LdrEntryFull)); ++guard) {
		LdrEntryFull* e = (LdrEntryFull*)((char*)link - offsetof(LdrEntryFull, InMemoryOrderLinks));
		char name[64]; int n = (int)e->DllName.Len / 2; if (n > 63) n = 63;
		if (n > 0 && IsReadable((ULONG64)e->DllName.Buf, n * 2)) {
			for (int i = 0; i < n; ++i) name[i] = (char)e->DllName.Buf[i];
			name[n] = 0;
		} else name[0] = 0;
		WriteStr(h, "    "); WriteStr(h, name); WriteStr(h, " ");
		WriteHexU(h, (ULONG64)e->DllBase);
		link = (ULONG64)e->InMemoryOrderLinks.Flink;
	}
}

static void DumpTailOf(HANDLE h, const char* path, const char* label)
{
	HANDLE f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE) return;
	DWORD size = GetFileSize(f, NULL);
	if (size == 0 || size == 0xFFFFFFFF) { CloseHandle(f); return; }
	DWORD want = size < 6000 ? size : 6000;
	SetFilePointer(f, (LONG)(size - want), NULL, FILE_BEGIN);
	static char buf[6200];
	DWORD got = 0;
	ReadFile(f, buf, want, &got, NULL);
	CloseHandle(f);
	buf[got] = 0;
	wsprintfA(buf + sizeof(buf) - 64, "\n  --- end %s ---\n", label);
	char hdr[80];
	wsprintfA(hdr, "  --- tail of %s ---\n", label);
	WriteStr(h, hdr);
	DWORD wr; WriteFile(h, buf, lstrlenA(buf), &wr, NULL);
}

static void DumpEngineLogs(HANDLE h)
{
	DumpTailOf(h, "DebugLogFile.txt", "DebugLogFile.txt");
	DumpTailOf(h, "dx8vk.log", "dx8vk.log");
}

static void CrashDiagUnwindAndScrape(HANDLE h, CONTEXT* c)
{
	typedef PRUNTIME_FUNCTION (WINAPI *LookupFn)(ULONG64, PULONG64, PVOID);
	typedef VOID (WINAPI *UnwindFn)(ULONG, ULONG64, ULONG64, PRUNTIME_FUNCTION,
	                                PCONTEXT, PULONG64, PVOID, PVOID);
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	LookupFn lookup = (LookupFn)GetProcAddress(ntdll, "RtlLookupFunctionEntry");
	UnwindFn unwind = (UnwindFn)GetProcAddress(ntdll, "RtlVirtualUnwind");
	WriteStr(h, "  stack scrape (RTS.exe code addresses only):\n");
	ULONG64 prev = 0;
	int printed = 0;
	for (ULONG64 a = c->Rsp; a < c->Rsp + 0x4000 && printed < 48; a += 8) {
		if (!IsReadable(a, 8)) break;
		ULONG64 v = *(ULONG64*)a;
		if (v > g_imageBase + 0x1000 && v < g_imageBase + g_imageSize && v != prev) {
			LogFrame(h, "    *", v);
			prev = v;
			++printed;
		}
	}
	if (c->Rip == 0) {
		WriteStr(h, "  null-call return candidates at [rsp]:\n");
		for (int i = 0; i < 4; ++i)
			if (IsReadable(c->Rsp + (ULONG64)i * 8, 8))
				LogFrame(h, "    [rsp]", *(ULONG64*)(c->Rsp + (ULONG64)i * 8));
	}
	if (lookup && unwind) {
		CONTEXT cur = *c;
		for (int i = 0; i < 40; ++i) {
			ULONG64 ib = 0;
			PRUNTIME_FUNCTION rf = lookup(cur.Rip, &ib, NULL);
			if (!rf) break;
			ULONG64 est = 0;
			unwind(0, ib, cur.Rip, rf, &cur, &est, NULL, NULL);
			if (!cur.Rip) break;
			LogFrame(h, "  frame", cur.Rip);
		}
	}
}

static LONG WINAPI CrashDiagHandler(PEXCEPTION_POINTERS ep)
{
	EXCEPTION_RECORD* er = ep->ExceptionRecord;
	CONTEXT* c = ep->ContextRecord;
	ULONG code = er->ExceptionCode;

	// pass through benign notification exceptions
	if (code == 0x40010006u || code == 0x4001000Fu || code == 0x406D1388u ||
	    code == 0x40010005u)
		return EXCEPTION_CONTINUE_EXECUTION;
	if (code == 0xE06D7363u || (code & 0xC0000000u) == 0x40000000u)
		return EXCEPTION_CONTINUE_SEARCH;

	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return EXCEPTION_CONTINUE_SEARCH;

	WriteStr(h, "--- exception ---\n");
	WriteStr(h, "code="); WriteHexU(h, code);
	WriteStr(h, " addr="); WriteHexU(h, (ULONG64)er->ExceptionAddress);
	if (code == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2) {
		WriteStr(h, er->ExceptionInformation[0] == 1 ? "AV: WRITE to " :
			(er->ExceptionInformation[0] == 2 ? "AV: DEP EXEC of " : "AV: READ of "));
		WriteHexU(h, er->ExceptionInformation[1]);
	}
	if (code == 0xC00000FDu) { WriteStr(h, "STACK OVERFLOW\n"); CloseHandle(h); return EXCEPTION_EXECUTE_HANDLER; }
	LogFrame(h, "RIP", c->Rip);
	WriteStr(h, "  args: rcx="); WriteHexU(h, c->Rcx);
	WriteStr(h, "  rdx="); WriteHexU(h, c->Rdx);
	WriteStr(h, "  r8="); WriteHexU(h, c->R8);
	WriteStr(h, "  r9="); WriteHexU(h, c->R9);

	CrashDiagUnwindAndScrape(h, c);
	DumpModules(h);
	DumpEngineLogs(h);
	CloseHandle(h);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void CrashDiagTerminateHandler()
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		WriteStr(h, "=== TERMINATE: uncaught C++ exception ===\n");
		try { throw; }
		catch (const std::exception& e) {
			WriteStr(h, "  std::exception: "); WriteStr(h, e.what()); WriteStr(h, "\n");
		}
		catch (...) {
			const std::type_info* ti = abi::__cxa_current_exception_type();
			WriteStr(h, "  unknown exception, type: ");
			WriteStr(h, (ti && ti->name()) ? ti->name() : "?");
			WriteStr(h, "\n");
		}
		CONTEXT c; RtlCaptureContext(&c);
		CrashDiagUnwindAndScrape(h, &c);
		DumpEngineLogs(h);
		CloseHandle(h);
	}
	abort();
}

static void __cdecl CrashDiagInvalidParam(const wchar_t*, const wchar_t*, const wchar_t*,
                                          unsigned, uintptr_t)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		WriteStr(h, "UCRT INVALID PARAMETER (would-be silent abort)\n");
		DumpEngineLogs(h);
		CloseHandle(h);
	}
	ExitProcess(0x1234);
}

static void __cdecl CrashDiagSignalHandler(int sig)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		char line[64];
		wsprintfA(line, "=== FATAL SIGNAL %d ===\n", sig);
		WriteStr(h, line);
		DumpEngineLogs(h);
		CloseHandle(h);
	}
	_exit(0x2000 + sig);
}

static void CrashDiagMarkerInner(const char* msg)
{
	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return;
	WriteStr(h, "marker: ");
	WriteStr(h, msg);
	WriteStr(h, "\n");
	CloseHandle(h);
}


// ---- stall watchdog ----
static volatile LONG g_cdTicks = 0;
static volatile LONG g_cdLastSeenTicks = 0;
static volatile LONG g_cdLastTickChangeMs = 0;
static volatile LONG g_cdStallDumps = 0;

static LONG CdTickCountMs(void)
{
	return (LONG)GetTickCount();
}

static HANDLE cdStallLog(void){ return OpenLogRaw(); }
static void CdDumpThreadContext(HANDLE thr, DWORD tid, BOOL isMain)
{
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(ctx));
	ctx.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(thr, &ctx)) {
		char line[80];
		wsprintfA(line, "  thread %u (ctx unavailable, isMain=%d)\n", (unsigned)tid, (int)isMain);
		WriteStr(cdStallLog(), line);
		return;
	}
	char line[80];
	wsprintfA(line, "  thread %u%s:\n", (unsigned)tid, isMain ? " (MAIN)" : "");
	WriteStr(cdStallLog(), line);
	LogFrame(cdStallLog(), "    rip", (ULONG64)ctx.Rip);
	// unwind
	typedef PRUNTIME_FUNCTION (WINAPI *LookupFn)(ULONG64, PULONG64, PVOID);
	typedef VOID (WINAPI *UnwindFn)(ULONG, ULONG64, ULONG64, PRUNTIME_FUNCTION,
	                                PCONTEXT, PULONG64, PVOID, PVOID);
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	LookupFn lookup = (LookupFn)GetProcAddress(ntdll, "RtlLookupFunctionEntry");
	UnwindFn unwind = (UnwindFn)GetProcAddress(ntdll, "RtlVirtualUnwind");
	if (lookup && unwind) {
		CONTEXT cur = ctx;
		for (int i = 0; i < 24; ++i) {
			ULONG64 ib = 0;
			PRUNTIME_FUNCTION rf = lookup(cur.Rip, &ib, NULL);
			if (!rf) break;
			ULONG64 est = 0;
			unwind(0, ib, cur.Rip, rf, &cur, &est, NULL, NULL);
			if (!cur.Rip) break;
			LogFrame(cdStallLog(), "    frame", (ULONG64)cur.Rip);
		}
	}
}

static HANDLE cdStallLog(void);

static void CdDumpAllThreads(void)
{
	HANDLE h = cdStallLog();
	if (h == INVALID_HANDLE_VALUE) return;
	DWORD mainTid = GetCurrentThreadId();
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snap != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te; te.dwSize = sizeof(te);
		if (Thread32First(snap, &te)) {
			do {
				if (te.th32OwnerProcessID != GetCurrentProcessId()) continue;
				HANDLE thr = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
					FALSE, te.th32ThreadID);
				if (!thr) continue;
				SuspendThread(thr);
				CONTEXT c2; memset(&c2,0,sizeof(c2)); c2.ContextFlags = CONTEXT_FULL;
				if (GetThreadContext(thr, &c2)) {
					char line[96];
					wsprintfA(line, "  thread %u%s rip=", te.th32ThreadID, te.th32ThreadID==mainTid?" (MAIN)":"");
					WriteStr(h, line);
					ULONG64 rip = (ULONG64)c2.Rip;
					// rip of suspended thread is its wait return address; also walk
					if (rip >= g_imageBase && rip < g_imageBase + g_imageSize) {
						wsprintfA(line, "RTS.exe+0x%x\n", (unsigned int)(rip - g_imageBase));
						WriteStr(h, line);
					} else {
						char name[64]; ULONG64 mb = 0;
						if (ModuleNameFor(rip, name, sizeof(name), &mb)) {
							wsprintfA(line, "%s+0x%x\n", name, (unsigned int)(rip - mb));
							WriteStr(h, line);
						} else { WriteHexU(h, rip); }
					}
					// stack scrape for this thread
					for (ULONG64 a = (ULONG64)c2.Rsp; a < (ULONG64)c2.Rsp + 0x2000; a += 8) {
						if (!IsReadable(a, 8)) break;
						ULONG64 v = *(ULONG64*)a;
						if (v > g_imageBase + 0x1000 && v < g_imageBase + g_imageSize) {
							LogFrame(h, "      *", v);
						}
					}
				} else {
					char line[64];
					wsprintfA(line, "  thread %u: no context\n", te.th32ThreadID);
					WriteStr(h, line);
				}
				ResumeThread(thr);
				CloseHandle(thr);
			} while (Thread32Next(snap, &te));
		}
		CloseHandle(snap);
	}
	CloseHandle(h);
}

static DWORD WINAPI CdWatchdog(LPVOID)
{
	for (;;) {
		Sleep(10000);
		LONG now = CdTickCountMs();
		LONG ticks = g_cdTicks;
		if (ticks != g_cdLastSeenTicks) {
			g_cdLastSeenTicks = ticks;
			g_cdLastTickChangeMs = now;
			continue;
		}
		if (now - g_cdLastTickChangeMs > 90000 && g_cdStallDumps < 4) {
			g_cdStallDumps++;
			HANDLE h = OpenLogRaw();
			if (h != INVALID_HANDLE_VALUE) {
				char line[96];
				wsprintfA(line, "=== STALL DETECTED: no loop progress for %d ms (dumps=%d) ===\n",
					(int)(now - g_cdLastTickChangeMs), (int)g_cdStallDumps);
				WriteStr(h, line);
				CloseHandle(h);
			}
			CdDumpAllThreads();
		}
	}
	return 0;
}

void CrashDiagTick(void)
{
	InterlockedIncrement(&g_cdTicks);
}

void CrashDiagMarker(const char* msg)
{
	CrashDiagMarkerInner(msg);
}

void CrashDiagReportException(const char* where)
{
	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return;
	WriteStr(h, "exception escaped at: ");
	WriteStr(h, where);
	WriteStr(h, "\n");
	try { throw; }
	catch (const std::exception& e) {
		WriteStr(h, "  std::exception: "); WriteStr(h, e.what()); WriteStr(h, "\n");
	}
	catch (const char* sp) {
		WriteStr(h, "  char*: "); WriteStr(h, sp ? sp : "(null)"); WriteStr(h, "\n");
	}
	catch (...) {
		WriteStr(h, "  unknown exception type\n");
	}
	DumpEngineLogs(h);
	CloseHandle(h);
}

static BOOL WINAPI CrashDiagUef(_EXCEPTION_POINTERS* ep)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		WriteStr(h, "UNHANDLED EXCEPTION FILTER reached\n");
		if (ep && ep->ExceptionRecord) {
			WriteStr(h, "  code="); WriteHexU(h, ep->ExceptionRecord->ExceptionCode);
		}
		CloseHandle(h);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

__attribute__((constructor(101)))
static void CrashDiagCtor(void)
{
	CrashDiagInit();
	AddVectoredExceptionHandler(1, CrashDiagHandler);
	_set_invalid_parameter_handler(CrashDiagInvalidParam);
	atexit([]() { CrashDiagMarkerInner("atexit: process exiting normally"); });
	SetUnhandledExceptionFilter(CrashDiagUef);
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
	std::set_terminate(CrashDiagTerminateHandler);
	signal(SIGABRT, CrashDiagSignalHandler);
	signal(SIGSEGV, CrashDiagSignalHandler);
	signal(SIGFPE, CrashDiagSignalHandler);
	signal(SIGILL, CrashDiagSignalHandler);

	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		SYSTEMTIME st;
		char line[128];
		GetLocalTime(&st);
		wsprintfA(line, "=== session %04d-%02d-%02d %02d:%02d:%02d base=",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		WriteStr(h, line);
		WriteHexU(h, g_imageBase);
		WriteStr(h, " size=");
		WriteHexU(h, g_imageSize);
		CloseHandle(h);
	}
}
