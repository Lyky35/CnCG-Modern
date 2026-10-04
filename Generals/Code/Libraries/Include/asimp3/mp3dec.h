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

#ifndef __ASIMP3_MP3DEC_H
#define __ASIMP3_MP3DEC_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "mss.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// ASI (Audio Stream Interface) MP3 decoder stubs
///////////////////////////////////////////////////////////////////////////////

typedef void *ASISTREAM;
typedef void *HASISTREAM;

typedef S32 (AILCALLBACK *ASI_FETCH_CB)(U32 user, void *dest, S32 bytes, S32 offset);

inline ASISTREAM *ASI_stream_open(U32 user, ASI_FETCH_CB fetch, U32 flags)
{
	(void)user; (void)fetch; (void)flags;
	return NULL;
}

inline S32 ASI_stream_process(HASISTREAM stream, void *dst, S32 *dbytes)
{
	(void)stream; (void)dst;
	if (dbytes) *dbytes = 0;
	return 0;
}

inline void ASI_stream_close(HASISTREAM stream) { (void)stream; }

inline void ASI_startup(void) { }
inline void ASI_shutdown(void) { }

#ifdef __cplusplus
}
#endif

#endif // __ASIMP3_MP3DEC_H
