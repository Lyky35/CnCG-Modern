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
// Project:   Generals
//
// Module:    FFmpegVideo
//
// File name: FFmpegVideoPlayer.cpp
//
// Created:   10/22/01	TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Lib/BaseType.h"
#include "FFmpegVideoPlayer.h"
#include "Common/GlobalData.h"
#include "Common/Registry.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------
#define VIDEO_LANG_PATH_FORMAT "Data/%s/Movies/%s.%s"
#define VIDEO_PATH	"Data\\Movies"
#define VIDEO_EXT		"bik"
#define VIDEO_EXT_FFMPEG "mp4"

//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//============================================================================
// FFmpegVideoPlayer::FFmpegVideoPlayer
//============================================================================

FFmpegVideoPlayer::FFmpegVideoPlayer()
{

}

//============================================================================
// FFmpegVideoPlayer::~FFmpegVideoPlayer
//============================================================================

FFmpegVideoPlayer::~FFmpegVideoPlayer()
{
	deinit();
}

//============================================================================
// FFmpegVideoPlayer::init
//============================================================================

void	FFmpegVideoPlayer::init( void )
{
	VideoPlayer::init();
}

//============================================================================
// FFmpegVideoPlayer::deinit
//============================================================================

void FFmpegVideoPlayer::deinit( void )
{
	VideoPlayer::deinit();
}

//============================================================================
// FFmpegVideoPlayer::reset
//============================================================================

void	FFmpegVideoPlayer::reset( void )
{
	VideoPlayer::reset();
}

//============================================================================
// FFmpegVideoPlayer::update
//============================================================================

void	FFmpegVideoPlayer::update( void )
{
	VideoPlayer::update();
}

//============================================================================
// FFmpegVideoPlayer::loseFocus
//============================================================================

void	FFmpegVideoPlayer::loseFocus( void )
{
	VideoPlayer::loseFocus();
}

//============================================================================
// FFmpegVideoPlayer::regainFocus
//============================================================================

void	FFmpegVideoPlayer::regainFocus( void )
{
	VideoPlayer::regainFocus();
}

//============================================================================
// FFmpegVideoPlayer::createStream
//============================================================================

VideoStreamInterface* FFmpegVideoPlayer::createStream( const char *filename )
{
	if ( filename == NULL || filename[0] == '\0' )
	{
		return NULL;
	}

	FFmpegVideoStream *stream = NEW FFmpegVideoStream;

	if ( stream )
	{
		if ( !stream->openFile( filename ) )
		{
			delete stream;
			return NULL;
		}

		stream->m_next = m_firstStream;
		stream->m_player = this;
		m_firstStream = stream;
	}

	return stream;
}

//============================================================================
// FFmpegVideoPlayer::open
//============================================================================

VideoStreamInterface*	FFmpegVideoPlayer::open( AsciiString movieTitle )
{
	VideoStreamInterface*	stream = NULL;

	const Video* pVideo = getVideo(movieTitle);
	if (pVideo) {
		if (TheGlobalData->m_modDir.isNotEmpty())
		{
			char filePath[ _MAX_PATH ];
			sprintf( filePath, "%s%s\\%s.%s", TheGlobalData->m_modDir.str(), VIDEO_PATH, pVideo->m_filename.str(), VIDEO_EXT_FFMPEG );
			stream = createStream( filePath );
			if (stream)
			{
				return stream;
			}
		}

		char localizedFilePath[ _MAX_PATH ];
		sprintf( localizedFilePath, VIDEO_LANG_PATH_FORMAT, GetRegistryLanguage().str(), pVideo->m_filename.str(), VIDEO_EXT_FFMPEG );
		stream = createStream( localizedFilePath );
		if (stream)
		{
			return stream;
		}

		char filePath[ _MAX_PATH ];
		sprintf( filePath, "%s\\%s.%s", VIDEO_PATH, pVideo->m_filename.str(), VIDEO_EXT_FFMPEG );
		stream = createStream( filePath );
	}

	return stream;
}

//============================================================================
// FFmpegVideoPlayer::load
//============================================================================

VideoStreamInterface*	FFmpegVideoPlayer::load( AsciiString movieTitle )
{
	return open(movieTitle);
}

//============================================================================
// FFmpegVideoPlayer::Open
//============================================================================

Bool FFmpegVideoPlayer::Open( const char *filename )
{
	Close();
	return ( createStream( filename ) != NULL );
}

//============================================================================
// FFmpegVideoPlayer::Close
//============================================================================

void FFmpegVideoPlayer::Close( void )
{
	closeAllStreams();
}

//============================================================================
// FFmpegVideoPlayer::Play
//============================================================================

void FFmpegVideoPlayer::Play( void )
{
	VideoStreamInterface *stream = firstStream();
	while ( stream )
	{
		FFmpegVideoStream *ffmpegStream = (FFmpegVideoStream*) stream;
		ffmpegStream->play();
		stream = stream->next();
	}
}

//============================================================================
// FFmpegVideoPlayer::Stop
//============================================================================

void FFmpegVideoPlayer::Stop( void )
{
	VideoStreamInterface *stream = firstStream();
	while ( stream )
	{
		FFmpegVideoStream *ffmpegStream = (FFmpegVideoStream*) stream;
		ffmpegStream->stop();
		stream = stream->next();
	}
}

//============================================================================
// FFmpegVideoPlayer::Update
//============================================================================

void FFmpegVideoPlayer::Update( void )
{
	update();
}

//============================================================================
// FFmpegVideoPlayer::Render
//============================================================================

void FFmpegVideoPlayer::Render( void )
{
	VideoStreamInterface *stream = firstStream();
	if ( stream )
	{
		FFmpegVideoStream *ffmpegStream = (FFmpegVideoStream*) stream;
		if ( ffmpegStream->isFrameReady() )
		{
			ffmpegStream->frameDecompress();
		}
	}
}

//============================================================================
// FFmpegVideoPlayer::IsPlaying
//============================================================================

Bool FFmpegVideoPlayer::IsPlaying( void )
{
	VideoStreamInterface *stream = firstStream();
	if ( stream )
	{
		FFmpegVideoStream *ffmpegStream = (FFmpegVideoStream*) stream;
		return ffmpegStream->playing();
	}
	return FALSE;
}

//============================================================================
// FFmpegVideoStream::FFmpegVideoStream
//============================================================================

FFmpegVideoStream::FFmpegVideoStream()
: m_formatContext(NULL),
	m_codecContext(NULL),
	m_frame(NULL),
	m_frameRGB(NULL),
	m_packet(NULL),
	m_swsContext(NULL),
	m_videoStreamIndex(-1),
	m_frameIndex(0),
	m_frameReady(FALSE),
	m_playing(FALSE),
	m_finished(FALSE),
	m_width(0),
	m_height(0),
	m_frameCount(0),
	m_frameRate(0.0),
	m_frameTime(0.0)
{
}

//============================================================================
// FFmpegVideoStream::~FFmpegVideoStream
//============================================================================

FFmpegVideoStream::~FFmpegVideoStream()
{
	closeFile();
}

//============================================================================
// FFmpegVideoStream::openFile
//============================================================================

Bool FFmpegVideoStream::openFile( const char *filename )
{
	if ( avformat_open_input( &m_formatContext, filename, NULL, NULL ) != 0 )
	{
		return FALSE;
	}

	if ( avformat_find_stream_info( m_formatContext, NULL ) < 0 )
	{
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	m_videoStreamIndex = -1;
	for ( unsigned int i = 0; i < m_formatContext->nb_streams; i++ )
	{
		if ( m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
		{
			m_videoStreamIndex = i;
			break;
		}
	}

	if ( m_videoStreamIndex == -1 )
	{
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	AVCodecParameters *codecParams = m_formatContext->streams[m_videoStreamIndex]->codecpar;
	const AVCodec *codec = avcodec_find_decoder( codecParams->codec_id );
	if ( codec == NULL )
	{
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	m_codecContext = avcodec_alloc_context3( codec );
	if ( m_codecContext == NULL )
	{
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	if ( avcodec_parameters_to_context( m_codecContext, codecParams ) < 0 )
	{
		avcodec_free_context( &m_codecContext );
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	if ( avcodec_open2( m_codecContext, codec, NULL ) < 0 )
	{
		avcodec_free_context( &m_codecContext );
		avformat_close_input( &m_formatContext );
		return FALSE;
	}

	m_width = m_codecContext->width;
	m_height = m_codecContext->height;

	AVRational frameRate = m_formatContext->streams[m_videoStreamIndex]->avg_frame_rate;
	if ( frameRate.den > 0 )
	{
		m_frameRate = av_q2d( frameRate );
	}
	else
	{
		m_frameRate = 25.0;
	}

	if ( m_formatContext->streams[m_videoStreamIndex]->nb_frames > 0 )
	{
		m_frameCount = m_formatContext->streams[m_videoStreamIndex]->nb_frames;
	}

	m_frame = av_frame_alloc();
	m_frameRGB = av_frame_alloc();
	m_packet = av_packet_alloc();

	if ( !m_frame || !m_frameRGB || !m_packet )
	{
		closeFile();
		return FALSE;
	}

	return TRUE;
}

//============================================================================
// FFmpegVideoStream::closeFile
//============================================================================

void FFmpegVideoStream::closeFile( void )
{
	if ( m_swsContext )
	{
		sws_freeContext( m_swsContext );
		m_swsContext = NULL;
	}

	if ( m_packet )
	{
		av_packet_free( &m_packet );
	}

	if ( m_frameRGB )
	{
		av_frame_free( &m_frameRGB );
	}

	if ( m_frame )
	{
		av_frame_free( &m_frame );
	}

	if ( m_codecContext )
	{
		avcodec_free_context( &m_codecContext );
	}

	if ( m_formatContext )
	{
		avformat_close_input( &m_formatContext );
	}

	m_videoStreamIndex = -1;
	m_frameIndex = 0;
	m_frameReady = FALSE;
	m_playing = FALSE;
	m_finished = FALSE;
	m_width = 0;
	m_height = 0;
	m_frameCount = 0;
	m_frameRate = 0.0;
	m_frameTime = 0.0;
}

//============================================================================
// FFmpegVideoStream::decodeNextFrame
//============================================================================

Bool FFmpegVideoStream::decodeNextFrame( void )
{
	if ( m_finished || !m_formatContext || !m_codecContext )
	{
		return FALSE;
	}

	int ret;
	while ( (ret = av_read_frame( m_formatContext, m_packet )) >= 0 )
	{
		if ( m_packet->stream_index == m_videoStreamIndex )
		{
			ret = avcodec_send_packet( m_codecContext, m_packet );
			av_packet_unref( m_packet );
			if ( ret < 0 )
			{
				return FALSE;
			}

			ret = avcodec_receive_frame( m_codecContext, m_frame );
			if ( ret == 0 )
			{
				m_frameIndex++;
				m_frameReady = TRUE;
				AVRational timeBase = m_formatContext->streams[m_videoStreamIndex]->time_base;
				m_frameTime = (double)m_frame->pts * av_q2d( timeBase );
				return TRUE;
			}
			else if ( ret == AVERROR(EAGAIN) )
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}
		av_packet_unref( m_packet );
	}

	ret = avcodec_send_packet( m_codecContext, NULL );
	if ( ret < 0 )
	{
		m_finished = TRUE;
		return FALSE;
	}

	ret = avcodec_receive_frame( m_codecContext, m_frame );
	if ( ret == 0 )
	{
		m_frameIndex++;
		m_frameReady = TRUE;
		AVRational timeBase = m_formatContext->streams[m_videoStreamIndex]->time_base;
		m_frameTime = (double)m_frame->pts * av_q2d( timeBase );
		return TRUE;
	}

	m_finished = TRUE;
	return FALSE;
}

//============================================================================
// FFmpegVideoStream::convertFrame
//============================================================================

void FFmpegVideoStream::convertFrame( VideoBuffer *buffer )
{
	if ( !buffer || !m_frame || !m_frameRGB || !m_codecContext )
	{
		return;
	}

	AVPixelFormat dstFormat;
	int dstBytesPerPixel;

	switch ( buffer->format() )
	{
		case VideoBuffer::TYPE_X8R8G8B8:
			dstFormat = AV_PIX_FMT_BGRA;
			dstBytesPerPixel = 4;
			break;

		case VideoBuffer::TYPE_R8G8B8:
			dstFormat = AV_PIX_FMT_BGR24;
			dstBytesPerPixel = 3;
			break;

		case VideoBuffer::TYPE_R5G6B5:
			dstFormat = AV_PIX_FMT_RGB565LE;
			dstBytesPerPixel = 2;
			break;

		case VideoBuffer::TYPE_X1R5G5B5:
			dstFormat = AV_PIX_FMT_RGB555LE;
			dstBytesPerPixel = 2;
			break;

		default:
			return;
	}

	if ( !m_swsContext )
	{
		m_swsContext = sws_getContext(
			m_width, m_height, m_codecContext->pix_fmt,
			m_width, m_height, dstFormat,
			SWS_BILINEAR, NULL, NULL, NULL );
	}

	if ( !m_swsContext )
	{
		return;
	}

	void *mem = buffer->lock();
	if ( mem != NULL )
	{
		m_frameRGB->data[0] = (uint8_t*)mem + buffer->yPos() * buffer->pitch() + buffer->xPos() * dstBytesPerPixel;
		m_frameRGB->linesize[0] = buffer->pitch();

		sws_scale( m_swsContext, m_frame->data, m_frame->linesize,
			0, m_height, m_frameRGB->data, m_frameRGB->linesize );

		buffer->unlock();
	}
}

//============================================================================
// FFmpegVideoStream::update
//============================================================================

void FFmpegVideoStream::update( void )
{
	if ( m_playing && !m_finished )
	{
		decodeNextFrame();
	}
}

//============================================================================
// FFmpegVideoStream::isFrameReady
//============================================================================

Bool FFmpegVideoStream::isFrameReady( void )
{
	return m_frameReady;
}

//============================================================================
// FFmpegVideoStream::frameDecompress
//============================================================================

void FFmpegVideoStream::frameDecompress( void )
{
	if ( !m_frameReady )
	{
		decodeNextFrame();
	}
}

//============================================================================
// FFmpegVideoStream::frameRender
//============================================================================

void FFmpegVideoStream::frameRender( VideoBuffer *buffer )
{
	if ( buffer && m_frameReady )
	{
		convertFrame( buffer );
	}
}

//============================================================================
// FFmpegVideoStream::frameNext
//============================================================================

void FFmpegVideoStream::frameNext( void )
{
	if ( !m_finished )
	{
		decodeNextFrame();
	}
}

//============================================================================
// FFmpegVideoStream::frameIndex
//============================================================================

Int FFmpegVideoStream::frameIndex( void )
{
	return m_frameIndex;
}

//============================================================================
// FFmpegVideoStream::frameCount
//============================================================================

Int	FFmpegVideoStream::frameCount( void )
{
	return m_frameCount;
}

//============================================================================
// FFmpegVideoStream::frameGoto
//============================================================================

void FFmpegVideoStream::frameGoto( Int index )
{
	if ( !m_formatContext || m_videoStreamIndex < 0 )
	{
		return;
	}

	AVRational timeBase = m_formatContext->streams[m_videoStreamIndex]->time_base;
	int64_t timestamp = (int64_t)( index * timeBase.den / timeBase.num );

	if ( av_seek_frame( m_formatContext, m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD ) >= 0 )
	{
		avcodec_flush_buffers( m_codecContext );
		m_frameIndex = index;
		m_frameReady = FALSE;
		m_finished = FALSE;
	}
}

//============================================================================
// FFmpegVideoStream::height
//============================================================================

Int		FFmpegVideoStream::height( void )
{
	return m_height;
}

//============================================================================
// FFmpegVideoStream::width
//============================================================================

Int		FFmpegVideoStream::width( void )
{
	return m_width;
}

//============================================================================
// FFmpegVideoStream::play
//============================================================================

void FFmpegVideoStream::play( void )
{
	m_playing = TRUE;
}

//============================================================================
// FFmpegVideoStream::stop
//============================================================================

void FFmpegVideoStream::stop( void )
{
	m_playing = FALSE;
}

//============================================================================
// FFmpegVideoStream::seek
//============================================================================

void FFmpegVideoStream::seek( Int index )
{
	frameGoto( index );
}
