// CrashDiag.cpp -- early crash logger for the MinGW x64 build.
// Writes RTS-crash.log next to the executable with the exception code, faulting
// address, and a list of RTS.exe image offsets (RVA) so the offline binary can
// be used to resolve them. Installed from an init constructor so crashes during
// static initialization are captured too.

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <exception>
#include <typeinfo>
#include <cxxabi.h>
#include <csignal>
#include <stdlib.h>

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

static HANDLE OpenLogRaw(void)
{
	CrashDiagInit();
	HANDLE h = CreateFileA(g_logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) SetFilePointer(h, 0, NULL, FILE_END);
	return h;
}

void CrashDiagMarker(const char* msg);

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
	while (i > 3 && buf[i-1] == '0' && buf[2] != '0' ) { } /* keep fixed width, simpler */
	buf[18] = '\n'; buf[19] = 0;
	WriteStr(h, buf);
}

static BOOL IsReadable(ULONG64 addr, SIZE_T len)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)) != sizeof(mbi)) return FALSE;
	if (mbi.State != MEM_COMMIT) return FALSE;
	if (mbi.AllocationProtect == PAGE_NOACCESS) return FALSE;
	if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return FALSE;
	ULONG64 regionEnd = (ULONG64)mbi.BaseAddress + mbi.RegionSize;
	return (addr >= (ULONG64)mbi.BaseAddress && addr + len <= regionEnd);
}

static void LogFrame(HANDLE h, const char* tag, ULONG64 addr)
{
	char line[80];
	if (addr >= g_imageBase && addr < g_imageBase + g_imageSize) {
		wsprintfA(line, "%s RTS.exe+0x%x\n", tag, (unsigned int)(addr - g_imageBase));
		WriteStr(h, line);
		return;
	}
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (snap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 me; me.dwSize = sizeof(me);
		if (Module32First(snap, &me)) {
			do {
				ULONG64 b = (ULONG64)me.modBaseAddr;
				if (addr >= b && addr < b + me.modBaseSize) {
					wsprintfA(line, "%s %s+0x%x\n", tag, me.szModule, (unsigned int)(addr - b));
					WriteStr(h, line);
					CloseHandle(snap);
					return;
				}
			} while (Module32Next(snap, &me));
		}
		CloseHandle(snap);
	}
	wsprintfA(line, "%s ", tag);
	WriteStr(h, line);
	WriteHexU(h, addr);
}

static void DumpModules(HANDLE h);
static void CrashDiagUnwindAndScrape(HANDLE h, CONTEXT* c);

static LONG WINAPI CrashDiagHandler(PEXCEPTION_POINTERS ep)
{
	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return EXCEPTION_CONTINUE_SEARCH;

	EXCEPTION_RECORD* er = ep->ExceptionRecord;
	CONTEXT* c = ep->ContextRecord;
	ULONG code = er->ExceptionCode;

	// Pass-through for benign notification exceptions:
	//  0x40010006 DBG_PRINTEXCEPTION_C (OutputDebugString)
	//  0x4001000F DBG_PRINTEXCEPTION_WIDE_C
	//  0x406D1388 thread name, 0x40010005 DBG_CONTROL_BREAK,
	//  any WARNING-severity (0x4.......), and MSVC-style C++ throws
	//  (0xE06D7363) which must reach the real EH machinery.
	if (code == 0x40010006u || code == 0x4001000Fu || code == 0x406D1388u ||
	    code == 0x40010005u) {
		CloseHandle(h);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	if (code == 0xE06D7363u || (code & 0xC0000000u) == 0x40000000u) {
		CloseHandle(h);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	WriteStr(h, "--- exception ---\n");
	WriteStr(h, "code="); WriteHexU(h, code);
	WriteStr(h, " addr="); WriteHexU(h, (ULONG64)er->ExceptionAddress);
	if (code == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2) {
		WriteStr(h, er->ExceptionInformation[0] == 1 ? "AV: WRITE to " :
			(er->ExceptionInformation[0] == 2 ? "AV: DEP EXEC of " : "AV: READ of "));
		WriteHexU(h, er->ExceptionInformation[1]);
	}
	if (code == (ULONG)0xC00000FD) {
		WriteStr(h, "STACK OVERFLOW\n");
		CloseHandle(h);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	LogFrame(h, "RIP", c->Rip);
	WriteStr(h, "  args: rcx="); WriteHexU(h, c->Rcx);
	WriteStr(h, "  rdx="); WriteHexU(h, c->Rdx);
	WriteStr(h, "  r8="); WriteHexU(h, c->R8);
	WriteStr(h, "  r9="); WriteHexU(h, c->R9);

	CrashDiagUnwindAndScrape(h, c);
	DumpModules(h);
	CloseHandle(h);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void __cdecl CrashDiagInvalidParam(const wchar_t*, const wchar_t*, const wchar_t*,
                                          unsigned, uintptr_t)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) { WriteStr(h, "UCRT INVALID PARAMETER (would-be silent abort)\n"); CloseHandle(h); }
	// do not return: ucrt would abort anyway; make it a clean recorded stop
	ExitProcess(0x1234);
}

static LONG WINAPI CrashDiagUef(_EXCEPTION_POINTERS* ep)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) { WriteStr(h, "UNHANDLED EXCEPTION FILTER reached\n"); CloseHandle(h); }
	return EXCEPTION_EXECUTE_HANDLER;
}

#include <stdlib.h>
#include <malloc.h>

static void DumpModules(HANDLE h)
{
	WriteStr(h, "  modules:\n");
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (snap == INVALID_HANDLE_VALUE) return;
	MODULEENTRY32 me; me.dwSize = sizeof(me);
	if (Module32First(snap, &me)) {
		char line[300];
		do {
			wsprintfA(line, "    %s ", me.szModule);
			WriteStr(h, line);
			WriteHexU(h, (ULONG64)me.modBaseAddr);
		} while (Module32Next(snap, &me));
	}
	CloseHandle(snap);
}

static void CrashDiagUnwindAndScrape(HANDLE h, CONTEXT* c)
{
	typedef PRUNTIME_FUNCTION (WINAPI *LookupFn)(ULONG64, PULONG64, PVOID);
	typedef VOID (WINAPI *UnwindFn)(ULONG, ULONG64, ULONG64, PRUNTIME_FUNCTION,
	                                PCONTEXT, PULONG64, PVOID, PVOID);
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	LookupFn lookup = (LookupFn)GetProcAddress(ntdll, "RtlLookupFunctionEntry");
	UnwindFn unwind = (UnwindFn)GetProcAddress(ntdll, "RtlVirtualUnwind");
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
	WriteStr(h, "  stack scrape (RTS.exe code addresses only):\n");
	ULONG64 sp = c->Rsp;
	ULONG64 prev = 0;
	int printed = 0;
	for (ULONG64 a = sp; a < sp + 0x4000 && printed < 48; a += 8) {
		if (!IsReadable(a, 8)) break;
		ULONG64 v = *(ULONG64*)a;
		if (v > g_imageBase + 0x1000 && v < g_imageBase + g_imageSize && v != prev) {
			LogFrame(h, "    *", v);
			prev = v;
			++printed;
		}
	}
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
		DumpModules(h);
		CloseHandle(h);
	}
	abort();
}

static void __cdecl CrashDiagSignalHandler(int sig)
{
	HANDLE h = OpenLogRaw();
	if (h != INVALID_HANDLE_VALUE) {
		char line[64];
		wsprintfA(line, "=== FATAL SIGNAL %d ===\n", sig);
		WriteStr(h, line);
		CloseHandle(h);
	}
	_exit(0x2000 + sig);
}

__attribute__((constructor(101)))
static void CrashDiagCtor(void)
{
	CrashDiagInit();
	AddVectoredExceptionHandler(1, CrashDiagHandler);
	_set_invalid_parameter_handler(CrashDiagInvalidParam);
	atexit([]() { CrashDiagMarker("atexit: process exiting normally"); });
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
		WriteStr(h, "size=");
		WriteHexU(h, g_imageSize);
		CloseHandle(h);
	}
}

void CrashDiagMarker(const char* msg)
{
	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return;
	WriteStr(h, "marker: ");
	WriteStr(h, msg);
	WriteStr(h, "\n");
	CloseHandle(h);
}

void CrashDiagReportException(const char* where)
{
	HANDLE h = OpenLogRaw();
	if (h == INVALID_HANDLE_VALUE) return;
	WriteStr(h, "exception escaped at: ");
	WriteStr(h, where);
	WriteStr(h, "\n");
	try {
		throw;
	} catch (const std::exception& e) {
		WriteStr(h, "  std::exception: ");
		WriteStr(h, e.what());
		WriteStr(h, "\n");
	} catch (const char* sp) {
		WriteStr(h, "  char*: ");
		WriteStr(h, sp ? sp : "(null)");
		WriteStr(h, "\n");
	} catch (...) {
		WriteStr(h, "  unknown exception type\n");
	}
	CloseHandle(h);
}
