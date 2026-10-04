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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MSS Stub                                                      *
 *                                                                                     *
 *                     $Archive:: /Commando/Code/Libraries/Include/MSS/mss.h                   $*
 *                                                                                             *
 *                       Author:: XAudio2 Replacement                                          *
 *                                                                                             *
 *                    $Revision:: 1                                                            $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   No-op stubs for the Miles Sound System SDK. The Miles SDK is not available, so all        *
 *   AIL_* functions are stubbed out as no-ops to allow the codebase to compile.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __MSS_STUB_H
#define __MSS_STUB_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Calling convention macros
///////////////////////////////////////////////////////////////////////////////
#ifndef AILCALLBACK
#define AILCALLBACK
#endif

#ifndef AILCALL
#define AILCALL
#endif

#ifndef DXDEC
#define DXDEC
#endif

///////////////////////////////////////////////////////////////////////////////
// Basic integer types (normally provided by the Miles SDK)
///////////////////////////////////////////////////////////////////////////////
#ifndef MSS_TYPES_DEFINED
#define MSS_TYPES_DEFINED

typedef signed long           S32;
typedef unsigned long         U32;
typedef signed short          S16;
typedef unsigned short        U16;
typedef signed char           S8;
typedef unsigned char         U8;

#endif // MSS_TYPES_DEFINED

///////////////////////////////////////////////////////////////////////////////
// Opaque handle types
///////////////////////////////////////////////////////////////////////////////
typedef void *                HSAMPLE;
typedef void *                H3DSAMPLE;
typedef void *                HSTREAM;
typedef void *                HDIGDRIVER;
typedef void *                HPROVIDER;
typedef void *                H3DPOBJECT;
typedef void *                HTIMER;
typedef void *                HAUDIO;
typedef void *                AILLPDIRECTSOBJECT;
typedef void *                AILLPDIRECTSOUND;

typedef U32                   HPROENUM;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define AIL_NO_ERROR          0
#define M3D_NOERR             0
#define HPROENUM_FIRST        0

#define WAVE_FORMAT_PCM       0x0001
#define WAVE_FORMAT_IMA_ADPCM 0x0011

#define ENVIRONMENT_GENERIC   0

#define AIL_3D_2_SPEAKER      0
#define AIL_3D_4_SPEAKER      1
#define AIL_3D_51_SPEAKER     2
#define AIL_3D_71_SPEAKER     3
#define AIL_3D_HEADPHONE      4
#define AIL_3D_SURROUND       5

#define DIG_USE_WAVEOUT       0
#define AIL_LOCK_PROTECTION   1

#define AIL_FILE_SEEK_BEGIN   0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END     2

#define DP_FILTER             0

///////////////////////////////////////////////////////////////////////////////
// Structures
///////////////////////////////////////////////////////////////////////////////
typedef struct _AILSOUNDINFO
{
	U32 rate;
	U32 channels;
	U32 bits;
	U32 format;
} AILSOUNDINFO;

typedef struct _AILFILEPROCS
{
	void *open;
	void *close;
	void *seek;
	void *read;
} AILFILEPROCS;

// Callback typedefs
typedef U32  (AILCALLBACK *AILFILEOPEN)(const char *filename, U32 *file_handle);
typedef void (AILCALLBACK *AILFILECLOSE)(U32 file_handle);
typedef S32  (AILCALLBACK *AILFILESEEK)(U32 file_handle, S32 offset, U32 type);
typedef U32  (AILCALLBACK *AILFILEREAD)(U32 file_handle, void *buffer, U32 bytes);
typedef void (AILCALLBACK *AILOS_CALLBACK)(HSAMPLE sample);
typedef void (AILCALLBACK *AILOS3D_CALLBACK)(H3DSAMPLE sample);
typedef void (AILCALLBACK *AILSTREAM_CALLBACK)(HSTREAM stream);

///////////////////////////////////////////////////////////////////////////////
// AIL_* function stubs (all no-ops)
///////////////////////////////////////////////////////////////////////////////

// Startup / shutdown
inline S32  AIL_startup(void) { return 1; }
inline void AIL_shutdown(void) { }
inline S32  AIL_set_redist_directory(const char *dir) { (void)dir; return 1; }
inline S32  AIL_quick_startup(S32 useDigital, S32 useMidi, S32 outputRate, S32 outputBits, S32 outputChannels)
{
	(void)useDigital; (void)useMidi; (void)outputRate; (void)outputBits; (void)outputChannels;
	return 1;
}
inline void AIL_quick_handles(HDIGDRIVER *digital, HPROVIDER *midi, HPROVIDER *waveOut)
{
	if (digital) *digital = NULL;
	if (midi) *midi = NULL;
	if (waveOut) *waveOut = NULL;
}

// Memory
inline void *AIL_mem_alloc_lock(U32 size) { (void)size; return NULL; }
inline void  AIL_mem_free_lock(void *ptr) { (void)ptr; }
inline U32   AIL_MMX_available(void) { return 0; }

// Locking
inline void AIL_lock(void) { }
inline void AIL_unlock(void) { }

// Error / version
inline char *AIL_last_error(void) { return (char *)"Miles Sound System stub"; }
inline void  AIL_MSS_version(char *buffer, S32 size)
{
	if (buffer && size > 0) buffer[0] = '\0';
}
inline S32   AIL_get_timer_highest_delay(void) { return 0; }

// Preferences
inline S32  AIL_set_preference(S32 preference, S32 value) { (void)preference; (void)value; return 1; }

// File callbacks
inline void AIL_set_file_callbacks(AILFILEOPEN open, AILFILECLOSE close, AILFILESEEK seek, AILFILEREAD read)
{
	(void)open; (void)close; (void)seek; (void)read;
}

// WaveOut (2D driver)
inline S32  AIL_waveOutOpen(HDIGDRIVER *driver, void *unused, S32 flags, LPWAVEFORMAT format)
{
	(void)unused; (void)flags; (void)format;
	if (driver) *driver = NULL;
	return AIL_NO_ERROR;
}
inline void AIL_waveOutClose(HDIGDRIVER driver) { (void)driver; }

// Sample handles (2D)
inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER driver) { (void)driver; return NULL; }
inline void    AIL_release_sample_handle(HSAMPLE sample) { (void)sample; }
inline void    AIL_init_sample(HSAMPLE sample) { (void)sample; }
inline void    AIL_set_named_sample_file(HSAMPLE sample, char *filename, void *data, U32 size, U32 flags)
{
	(void)sample; (void)filename; (void)data; (void)size; (void)flags;
}
inline void    AIL_set_sample_file(HSAMPLE sample, void *file, U32 flags) { (void)sample; (void)file; (void)flags; }
inline void    AIL_start_sample(HSAMPLE sample) { (void)sample; }
inline void    AIL_stop_sample(HSAMPLE sample) { (void)sample; }
inline void    AIL_resume_sample(HSAMPLE sample) { (void)sample; }
inline void    AIL_end_sample(HSAMPLE sample) { (void)sample; }
inline void    AIL_register_EOS_callback(HSAMPLE sample, AILOS_CALLBACK callback) { (void)sample; (void)callback; }
inline void    AIL_set_sample_user_data(HSAMPLE sample, S32 index, U32 value) { (void)sample; (void)index; (void)value; }
inline U32     AIL_sample_user_data(HSAMPLE sample, S32 index) { (void)sample; (void)index; return 0; }
inline void    AIL_set_sample_volume(HSAMPLE sample, S32 volume) { (void)sample; (void)volume; }
inline S32     AIL_sample_volume(HSAMPLE sample) { (void)sample; return 0; }
inline void    AIL_set_sample_pan(HSAMPLE sample, S32 pan) { (void)sample; (void)pan; }
inline S32     AIL_sample_pan(HSAMPLE sample) { (void)sample; return 0; }
inline void    AIL_set_sample_loop_count(HSAMPLE sample, U32 count) { (void)sample; (void)count; }
inline U32     AIL_sample_loop_count(HSAMPLE sample) { (void)sample; return 0; }
inline void    AIL_set_sample_ms_position(HSAMPLE sample, U32 ms) { (void)sample; (void)ms; }
inline void    AIL_sample_ms_position(HSAMPLE sample, S32 *len, S32 *pos)
{
	(void)sample;
	if (len) *len = 0;
	if (pos) *pos = 0;
}
inline S32     AIL_sample_playback_rate(HSAMPLE sample) { (void)sample; return 0; }
inline void    AIL_set_sample_playback_rate(HSAMPLE sample, S32 rate) { (void)sample; (void)rate; }
inline void    AIL_set_sample_volume_pan(HSAMPLE sample, float volume, float pan) { (void)sample; (void)volume; (void)pan; }
inline void    AIL_sample_volume_pan(HSAMPLE sample, float *volume, float *pan)
{
	(void)sample;
	if (volume) *volume = 0.0f;
	if (pan) *pan = 0.0f;
}
inline void    AIL_set_sample_processor(HSAMPLE sample, S32 processor, HPROVIDER provider) { (void)sample; (void)processor; (void)provider; }
inline void    AIL_set_filter_sample_preference(HSAMPLE sample, const char *name, void *value) { (void)sample; (void)name; (void)value; }

// 3D sample handles
inline H3DSAMPLE AIL_allocate_3D_sample_handle(HPROVIDER provider) { (void)provider; return NULL; }
inline void      AIL_release_3D_sample_handle(H3DSAMPLE sample) { (void)sample; }
inline void      AIL_set_3D_sample_file(H3DSAMPLE sample, void *file) { (void)sample; (void)file; }
inline void      AIL_start_3D_sample(H3DSAMPLE sample) { (void)sample; }
inline void      AIL_stop_3D_sample(H3DSAMPLE sample) { (void)sample; }
inline void      AIL_resume_3D_sample(H3DSAMPLE sample) { (void)sample; }
inline void      AIL_end_3D_sample(H3DSAMPLE sample) { (void)sample; }
inline void      AIL_register_3D_EOS_callback(H3DSAMPLE sample, AILOS3D_CALLBACK callback) { (void)sample; (void)callback; }
inline void      AIL_set_3D_object_user_data(H3DSAMPLE sample, S32 index, U32 value) { (void)sample; (void)index; (void)value; }
inline U32       AIL_3D_object_user_data(H3DSAMPLE sample, S32 index) { (void)sample; (void)index; return 0; }
inline void      AIL_set_3D_user_data(H3DSAMPLE sample, S32 index, U32 value) { (void)sample; (void)index; (void)value; }
inline U32       AIL_3D_user_data(H3DSAMPLE sample, S32 index) { (void)sample; (void)index; return 0; }
inline void      AIL_set_3D_sample_volume(H3DSAMPLE sample, float volume) { (void)sample; (void)volume; }
inline float     AIL_3D_sample_volume(H3DSAMPLE sample) { (void)sample; return 0.0f; }
inline void      AIL_set_3D_sample_loop_count(H3DSAMPLE sample, U32 count) { (void)sample; (void)count; }
inline U32       AIL_3D_sample_loop_count(H3DSAMPLE sample) { (void)sample; return 0; }
inline void      AIL_set_3D_sample_offset(H3DSAMPLE sample, U32 offset) { (void)sample; (void)offset; }
inline U32       AIL_3D_sample_offset(H3DSAMPLE sample) { (void)sample; return 0; }
inline U32       AIL_3D_sample_length(H3DSAMPLE sample) { (void)sample; return 0; }
inline S32       AIL_3D_sample_playback_rate(H3DSAMPLE sample) { (void)sample; return 0; }
inline void      AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, S32 rate) { (void)sample; (void)rate; }
inline void      AIL_set_3D_sample_distances(H3DSAMPLE sample, float minDist, float maxDist) { (void)sample; (void)minDist; (void)maxDist; }
inline void      AIL_set_3D_sample_occlusion(H3DSAMPLE sample, float occlusion) { (void)sample; (void)occlusion; }
inline void      AIL_set_3D_sample_effects_level(H3DSAMPLE sample, float level) { (void)sample; (void)level; }
inline void      AIL_set_3D_position(H3DSAMPLE sample, float x, float y, float z) { (void)sample; (void)x; (void)y; (void)z; }
inline void      AIL_set_3D_orientation(H3DPOBJECT listener, float x, float y, float z, float fx, float fy, float fz)
{
	(void)listener; (void)x; (void)y; (void)z; (void)fx; (void)fy; (void)fz;
}
inline void      AIL_set_3D_velocity_vector(H3DSAMPLE sample, float x, float y, float z) { (void)sample; (void)x; (void)y; (void)z; }

// 3D provider / listener
inline S32         AIL_enumerate_3D_providers(HPROENUM *next, HPROVIDER *provider, char **name)
{
	(void)next; (void)provider; (void*name;
	return 0;
}
inline S32         AIL_open_3D_provider(HPROVIDER provider) { (void)provider; return M3D_NOERR; }
inline void        AIL_close_3D_provider(HPROVIDER provider) { (void)provider; }
inline H3DPOBJECT  AIL_open_3D_listener(HPROVIDER provider) { (void)provider; return NULL; }
inline void        AIL_close_3D_listener(H3DPOBJECT listener) { (void)listener; }
inline void        AIL_set_3D_speaker_type(HPROVIDER provider, U32 type) { (void)provider; (void)type; }
inline S32         AIL_enumerate_filters(HPROENUM *next, HPROVIDER *provider, char **name)
{
	(void)next; (void)provider; (void*name;
	return 0;
}
inline void        AIL_get_DirectSound_info(HSAMPLE sample, void **ds, void *unused)
{
	(void)sample; (void)unused;
	if (ds) *ds = NULL;
}

// Streams
inline HSTREAM AIL_open_stream(HDIGDRIVER driver, const char *filename, U32 flags) { (void)driver; (void)filename; (void)flags; return NULL; }
inline HSTREAM AIL_open_stream_by_sample(HDIGDRIVER driver, HSAMPLE sample, const char *filename, U32 flags)
{
	(void)driver; (void)sample; (void)filename; (void)flags;
	return NULL;
}
inline void    AIL_close_stream(HSTREAM stream) { (void)stream; }
inline void    AIL_start_stream(HSTREAM stream) { (void)stream; }
inline void    AIL_pause_stream(HSTREAM stream, S32 pause) { (void)stream; (void)pause; }
inline void    AIL_register_stream_callback(HSTREAM stream, AILSTREAM_CALLBACK callback) { (void)stream; (void)callback; }
inline void    AIL_set_stream_loop_count(HSTREAM stream, U32 count) { (void)stream; (void)count; }
inline U32     AIL_stream_loop_count(HSTREAM stream) { (void)stream; return 0; }
inline void    AIL_set_stream_loop_block(HSTREAM stream, S32 start, S32 end) { (void)stream; (void)start; (void)end; }
inline void    AIL_set_stream_ms_position(HSTREAM stream, U32 ms) { (void)stream; (void)ms; }
inline void    AIL_stream_ms_position(HSTREAM stream, S32 *len, S32 *pos)
{
	(void)stream;
	if (len) *len = 0;
	if (pos) *pos = 0;
}
inline void    AIL_set_stream_pan(HSTREAM stream, S32 pan) { (void)stream; (void)pan; }
inline S32     AIL_stream_pan(HSTREAM stream) { (void)stream; return 0; }
inline void    AIL_set_stream_volume(HSTREAM stream, S32 volume) { (void)stream; (void)volume; }
inline S32     AIL_stream_volume(HSTREAM stream) { (void)stream; return 0; }
inline S32     AIL_stream_playback_rate(HSTREAM stream) { (void)stream; return 0; }
inline void    AIL_set_stream_playback_rate(HSTREAM stream, S32 rate) { (void)stream; (void)rate; }
inline void    AIL_set_stream_volume_pan(HSTREAM stream, float volume, float pan) { (void)stream; (void)volume; (void)pan; }
inline void    AIL_stream_volume_pan(HSTREAM stream, float *volume, float *pan)
{
	(void)stream;
	if (volume) *volume = 0.0f;
	if (pan) *pan = 0.0f;
}

// Quick play
inline HAUDIO AIL_quick_load_and_play(const char *filename, S32 loop, U32 flags) { (void)filename; (void)loop; (void)flags; return NULL; }
inline void   AIL_quick_set_volume(HAUDIO audio, float volume, float pan) { (void)audio; (void)volume; (void)pan; }
inline void   AIL_quick_unload(HAUDIO audio) { (void)audio; }

// Timers
inline void AIL_stop_timer(HTIMER timer) { (void)timer; }
inline void AIL_release_timer_handle(HTIMER timer) { (void)timer; }

// WAV info / decompression
inline S32 AIL_WAV_info(void *buffer, AILSOUNDINFO *info)
{
	(void)buffer;
	if (info) {
		info->rate = 0;
		info->channels = 0;
		info->bits = 0;
		info->format = 0;
	}
	return 0;
}
inline void AIL_decompress_ADPCM(AILSOUNDINFO *info, void **decompressed, U32 *size)
{
	(void)info;
	if (decompressed) *decompressed = NULL;
	if (size) *size = 0;
}

#ifdef __cplusplus
}
#endif

#endif // __MSS_STUB_H
