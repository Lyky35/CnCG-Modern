/*
**	FFmpegVideoPlayer stub for builds without target-platform FFmpeg.
**	Same class name / public surface as the real player, but video creation is
**	disabled (createStream returns NULL, so no movie ever plays). The engine
**	tolerates missing movies (intro videos are optional).
*/
#ifndef __FFMPEGVIDEOPLAYER_H_
#define __FFMPEGVIDEOPLAYER_H_

#include "GameClient/VideoPlayer.h"

class FFmpegVideoPlayer : public VideoPlayer
{
	protected:

		VideoStreamInterface* createStream( const char *filename );

	public:

		FFmpegVideoPlayer();
		virtual ~FFmpegVideoPlayer();

		// FFmpeg specific convenience API (mirrors the real player)
		Bool				Open( const char *filename );
		void				Close( void );
		void				Play( void );
		void				Stop( void );
		void				Update( void );
		void				Render( void );
		Bool				IsPlaying( void );
};

#endif // __FFMPEGVIDEOPLAYER_H_
