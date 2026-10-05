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

static FILE* OpenLog(void)
{
	CrashDiagInit();
	return fopen(g_logPath, "a");
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

static void LogFrame(FILE* f, const char* tag, ULONG64 addr)
{
	if (addr >= g_imageBase && addr < g_imageBase + g_imageSize) {
		fprintf(f, "%s RTS.exe+0x%llx\n", tag, (unsigned long long)(addr - g_imageBase));
		return;
	}
	// attribute to a loaded module if we can identify one
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (snap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 me; me.dwSize = sizeof(me);
		if (Module32First(snap, &me)) {
			do {
				ULONG64 b = (ULONG64)me.modBaseAddr;
				if (addr >= b && addr < b + me.modBaseSize) {
					fprintf(f, "%s %s+0x%lx\n", tag, me.szModule, (unsigned long)(addr - b));
					CloseHandle(snap);
					return;
				}
			} while (Module32Next(snap, &me));
		}
		CloseHandle(snap);
	}
	fprintf(f, "%s 0x%llx\n", tag, (unsigned long long)addr);
}

static LONG WINAPI CrashDiagHandler(PEXCEPTION_POINTERS ep)
{
	FILE* f = OpenLog();
	if (!f) return EXCEPTION_CONTINUE_SEARCH;

	EXCEPTION_RECORD* er = ep->ExceptionRecord;
	CONTEXT* c = ep->ContextRecord;

	fprintf(f, "--- exception ---\n");
	fprintf(f, "code=0x%08lx flags=0x%lx addr=0x%llx\n",
		(unsigned long)er->ExceptionCode, (unsigned long)er->ExceptionFlags,
		(unsigned long long)er->ExceptionAddress);
	if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2) {
		fprintf(f, "AV: %s 0x%llx\n",
			er->ExceptionInformation[0] == 1 ? "WRITE to" : (er->ExceptionInformation[0] == 2 ? "DEP EXEC of" : "READ of"),
			(unsigned long long)er->ExceptionInformation[1]);
	}
	LogFrame(f, "RIP", c->Rip);
	fprintf(f, "  args: rcx=0x%llx rdx=0x%llx r8=0x%llx r9=0x%llx r10=0x%llx r11=0x%llx\n",
		(unsigned long long)c->Rcx, (unsigned long long)c->Rdx,
		(unsigned long long)c->R8, (unsigned long long)c->R9,
		(unsigned long long)c->R10, (unsigned long long)c->R11);

	// Proper unwind via ntdll's table-based unwinder.
	typedef PRUNTIME_FUNCTION (WINAPI *LookupFn)(ULONG64, PULONG64, PVOID);
	typedef VOID (WINAPI *UnwindFn)(ULONG, ULONG64, ULONG64, PRUNTIME_FUNCTION,
	                                PCONTEXT, PULONG64, PVOID, PVOID);
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	LookupFn lookup = (LookupFn)GetProcAddress(ntdll, "RtlLookupFunctionEntry");
	UnwindFn unwind = (UnwindFn)GetProcAddress(ntdll, "RtlVirtualUnwind");
	BOOL unwound = FALSE;
	if (lookup && unwind) {
		CONTEXT cur = *c;
		for (int i = 0; i < 40; ++i) {
			ULONG64 ib = 0;
			PRUNTIME_FUNCTION rf = lookup(cur.Rip, &ib, NULL);
			if (!rf) break;
			ULONG64 est = 0;
			unwind(0, ib, cur.Rip, rf, &cur, &est, NULL, NULL);
			if (!cur.Rip) break;
			unwound = TRUE;
			LogFrame(f, "  frame", cur.Rip);
		}
	}
	if (!unwound) {
		// RBP-chain fallback.
		ULONG64 rbp = c->Rbp;
		for (int i = 0; i < 80; ++i) {
			if (rbp & 7) break;
			if (!IsReadable(rbp, 16)) break;
			ULONG64 nextRbp = *(ULONG64*)rbp;
			ULONG64 retAddr = *(ULONG64*)(rbp + 8);
			if (nextRbp <= rbp) break;
			LogFrame(f, "  frame", retAddr);
			rbp = nextRbp;
		}
		fprintf(f, "  stack scrape (RTS.exe code addresses only):\n");
		ULONG64 sp = c->Rsp;
		ULONG64 prev = 0;
		int printed = 0;
		for (ULONG64 a = sp; a < sp + 0x4000 && printed < 48; a += 8) {
			if (!IsReadable(a, 8)) break;
			ULONG64 v = *(ULONG64*)a;
			if (v > g_imageBase + 0x1000 && v < g_imageBase + g_imageSize && v != prev) {
				LogFrame(f, "    *", v);
				prev = v;
				++printed;
			}
		}
	}
	fflush(f);
	fclose(f);
	return EXCEPTION_EXECUTE_HANDLER;
}

__attribute__((constructor(101)))
static void CrashDiagCtor(void)
{
	CrashDiagInit();
	AddVectoredExceptionHandler(1, CrashDiagHandler);
	FILE* f = fopen(g_logPath, "a");
	if (f) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(f, "=== session %04d-%02d-%02d %02d:%02d:%02d base=0x%llx size=0x%llx ===\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
			(unsigned long long)g_imageBase, (unsigned long long)g_imageSize);
		fclose(f);
	}
}

void CrashDiagMarker(const char* msg)
{
	FILE* f = OpenLog();
	if (!f) return;
	fprintf(f, "marker: %s\n", msg);
	fclose(f);
}

void CrashDiagReportException(const char* where)
{
	FILE* f = OpenLog();
	if (!f) return;
	fprintf(f, "exception escaped at: %s\n", where);
	try {
		throw;
	} catch (const std::exception& e) {
		fprintf(f, "  std::exception: %s\n", e.what());
	} catch (const char* s) {
		fprintf(f, "  char*: %s\n", s ? s : "(null)");
	} catch (...) {
		fprintf(f, "  unknown exception type\n");
	}
	fclose(f);
}
