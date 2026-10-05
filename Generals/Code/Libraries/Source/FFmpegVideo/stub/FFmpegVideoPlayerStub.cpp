/*
**	FFmpegVideoPlayer stub implementation (see FFmpegVideoPlayer.h in this folder).
*/
#include "FFmpegVideoPlayer.h"

FFmpegVideoPlayer::FFmpegVideoPlayer()
{
}

FFmpegVideoPlayer::~FFmpegVideoPlayer()
{
}

VideoStreamInterface* FFmpegVideoPlayer::createStream( const char *filename )
{
	// No FFmpeg available in this build: movies are disabled.
	(void)filename;
	return NULL;
}

Bool FFmpegVideoPlayer::Open( const char *filename ) { (void)filename; return FALSE; }
void FFmpegVideoPlayer::Close( void ) { }
void FFmpegVideoPlayer::Play( void ) { }
void FFmpegVideoPlayer::Stop( void ) { }
void FFmpegVideoPlayer::Update( void ) { }
void FFmpegVideoPlayer::Render( void ) { }
Bool FFmpegVideoPlayer::IsPlaying( void ) { return FALSE; }
