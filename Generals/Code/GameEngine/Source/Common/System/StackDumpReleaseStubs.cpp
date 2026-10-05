/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//
// Release-build fallbacks for the stack-dump helpers.
//
// Common/StackDump.h unconditionally defines IG_DEBUG_STACKTRACE (a typo in its
// own include guard means the "release" branch is never taken), so the helpers are
// declared extern in every configuration. StackDump.cpp itself only compiles the
// real implementations under _DEBUG/_INTERNAL/IG_DEBUG_STACKTRACE *before* the
// header defines the macro - i.e. never in Release. The declarations therefore had
// no definitions to link against. These no-op fallbacks fill that gap.
//

#include "PreRTS.h"

#if !defined(_DEBUG) && !defined(_INTERNAL)

#include "Common/StackDump.h"

AsciiString g_LastErrorDump;

void StackDump(void (*callback)(const char*))
{
	(void)callback;
}

void StackDumpFromContext(DWORD eip, DWORD esp, DWORD ebp, void (*callback)(const char*))
{
	(void)eip; (void)esp; (void)ebp; (void)callback;
}

void FillStackAddresses(void **addresses, unsigned int count, unsigned int skip)
{
	(void)addresses; (void)count; (void)skip;
}

void StackDumpFromAddresses(void **addresses, unsigned int count, void (*callback)(const char*))
{
	(void)addresses; (void)count; (void)callback;
}

void GetFunctionDetails(void *pointer, char *name, char *filename, unsigned int *linenumber, unsigned int *address)
{
	(void)pointer;
	if (name) name[0] = 0;
	if (filename) filename[0] = 0;
	if (linenumber) *linenumber = 0;
	if (address) *address = 0;
}

void DumpExceptionInfo(unsigned int u, EXCEPTION_POINTERS *e_info)
{
	(void)u; (void)e_info;
}

#endif // !_DEBUG && !_INTERNAL
