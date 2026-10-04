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
// File name:  FFmpegVideoPlayer.h
//
// Created:    10/22/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __FFMPEGVIDEOPLAYER_H_
#define __FFMPEGVIDEOPLAYER_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "GameClient/VideoPlayer.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class FFmpegVideoPlayer;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// FFmpegVideoStream
//===============================

class FFmpegVideoStream : public VideoStream
{
	friend class FFmpegVideoPlayer;

	protected:

		AVFormatContext	*m_formatContext;							///< FFmpeg format context
		AVCodecContext	*m_codecContext;							///< FFmpeg codec context
		AVFrame					*m_frame;											///< Decoded frame
		AVFrame					*m_frameRGB;									///< Converted RGB frame
		AVPacket				*m_packet;										///< Packet read from file
		SwsContext			*m_swsContext;								///< Colorspace conversion context
		Int							m_videoStreamIndex;						///< Index of the video stream
		Int							m_frameIndex;									///< Zero based index of current frame
		Bool						m_frameReady;									///< Is a frame ready to be displayed
		Bool						m_playing;										///< Is playback active
		Bool						m_finished;									///< Has playback reached the end
		Int							m_width;											///< Video width in pixels
		Int							m_height;										///< Video height in pixels
		Int							m_frameCount;									///< Total number of frames (0 if unknown)
		double					m_frameRate;								///< Frames per second
		double					m_frameTime;								///< Presentation time of current frame

		FFmpegVideoStream();													///< only FFmpegVideoPlayer can create these
		virtual ~FFmpegVideoStream();

		Bool openFile( const char *filename );					///< Open and prepare a video file
		void closeFile( void );												///< Close the video file
		Bool decodeNextFrame( void );									///< Decode the next frame
		void convertFrame( VideoBuffer *buffer );				///< Convert current frame to buffer format

	public:

		virtual void update( void );									///< Update stream (decode next frame)

		virtual Bool	isFrameReady( void );						///< Is the frame ready to be displayed
		virtual void	frameDecompress( void );					///< Decode current frame
		virtual void	frameRender( VideoBuffer *buffer );	///< Render current frame in to buffer
		virtual void	frameNext( void );								///< Advance to next frame
		virtual Int		frameIndex( void );								///< Returns zero based index of current frame
		virtual Int		frameCount( void );								///< Returns the total number of frames in the stream
		virtual void	frameGoto( Int index );						///< Go to the specified frame index
		virtual Int		height( void );										///< Return the height of the video
		virtual Int		width( void );										///< Return the width of the video

		Bool				playing( void ) { return m_playing; }		///< Is playback active
		void				play( void );											///< Start/resume playback
		void				stop( void );											///< Stop playback
		void				seek( Int index );								///< Seek to frame index

};

//===============================
// FFmpegVideoPlayer
//===============================
/**
  *	FFmpeg video playback code.  Replaces the Bink video player.
	*/
//===============================

class FFmpegVideoPlayer : public VideoPlayer
{

	protected:

		VideoStreamInterface* createStream( const char *filename );

	public:

		// subsystem requirements
		virtual void	init( void );														///< Initialize video playback code
		virtual void	reset( void );													///< Reset video playback
		virtual void	update( void );													///< Services all video tasks. Should be called frequently

		virtual void	deinit( void );													///< Close down player

		FFmpegVideoPlayer();
		~FFmpegVideoPlayer();

		// service
		virtual void	loseFocus( void );											///< Should be called when application loses focus
		virtual void	regainFocus( void );										///< Should be called when application regains focus

		virtual VideoStreamInterface*	open( AsciiString movieTitle );	///< Open video file for playback
		virtual VideoStreamInterface*	load( AsciiString movieTitle );	///< Load video file in to memory for playback

		// FFmpeg specific convenience API
		Bool				Open( const char *filename );						///< Open a video file
		void				Close( void );												///< Close the video
		void				Play( void );													///< Start playback
		void				Stop( void );													///< Stop playback
		void				Update( void );												///< Decode a frame
		void				Render( void );												///< Render the current frame
		Bool				IsPlaying( void );											///< Check if video is playing

};

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------


#endif // __FFMPEGVIDEOPLAYER_H_
