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

////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2001-2003 Electronic Arts Inc.
//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// File name:  bink.h  (stub)
//
// Created:    10/22/01
//
//  Stub replacement for the Bink SDK header.  All Bink functions are
//  no-op stubs so the game builds without the Bink library.  Video
//  playback is handled by FFmpegVideoPlayer instead.
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __BINK_H_STUB_
#define __BINK_H_STUB_

#include <cstddef>

typedef unsigned int u32;
struct _BINK;
typedef struct _BINK * HBINK;

#define BINKPRELOADALL	0x00000001
#define BINKSURFACE32		0x00000002
#define BINKSURFACE24		0x00000003
#define BINKSURFACE565	0x00000004
#define BINKSURFACE555	0x00000005

typedef struct _BINKRECT
{
	int Left;
	int Top;
	int Width;
	int Height;
} BINKRECT;

typedef struct _BINK
{
	unsigned int Width;
	unsigned int Height;
	unsigned int Frames;
	unsigned int FrameNum;
} BINK;

inline HBINK BinkOpen( const char *name, unsigned int flags ) { (void)name; (void)flags; return NULL; }
inline void BinkClose( HBINK bink ) { (void)bink; }
inline int BinkWait( HBINK bink ) { (void)bink; return 1; }
inline void BinkDoFrame( HBINK bink ) { (void)bink; }
inline void BinkNextFrame( HBINK bink ) { (void)bink; }
inline int BinkGoto( HBINK bink, unsigned int frame, unsigned int flags ) { (void)bink; (void)frame; (void)flags; return 0; }
inline void BinkCopyToBuffer( HBINK bink, void *dest, int destpitch, unsigned int destheight, unsigned int destx, unsigned int desty, unsigned int flags ) { (void)bink; (void)dest; (void)destpitch; (void)destheight; (void)destx; (void)desty; (void)flags; }
inline void BinkSetVolume( HBINK bink, int trackid, int volume ) { (void)bink; (void)trackid; (void)volume; }
inline void BinkSetSoundTrack( unsigned int track, unsigned int total ) { (void)track; (void)total; }
inline int BinkSoundUseDirectSound( void *ds ) { (void)ds; return 0; }

#endif // __BINK_H_STUB_
