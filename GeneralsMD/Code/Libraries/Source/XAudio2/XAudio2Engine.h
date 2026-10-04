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
 *                 Project Name : XAudio2Engine                                                 *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Libraries/Source/XAudio2/XAudio2Engine.h     $*
 *                                                                                             *
 *                       Author:: XAudio2 Replacement                                          *
 *                                                                                             *
 *                    $Revision:: 1                                                            $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   XAudio2-based audio engine replacing the Miles Sound System based MilesAudioManager.       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __XAUDIO2_ENGINE_H
#define __XAUDIO2_ENGINE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "Common/GameAudio.h"
#include "Common/AudioAffect.h"
#include "Common/AudioHandleSpecialValues.h"

#include <xaudio2.h>

class AudioEventRTS;

class XAudio2Engine : public AudioManager
{
public:
	XAudio2Engine();
	virtual ~XAudio2Engine();

	// XAudio2 specific interface
	bool Initialize();
	void Shutdown();
	bool PlaySound(const char *filename);
	void StopSound();
	void SetVolume(float volume);

	// From SubsystemInterface
	virtual void init();
	virtual void postProcessLoad();
	virtual void reset();
	virtual void update();

#if defined(_DEBUG) || defined(_INTERNAL)
	virtual void audioDebugDisplay(DebugDisplayInterface *dd, void *userData, FILE *fp = NULL);
#endif

	// From AudioManager (device dependent)
	virtual void stopAudio(AudioAffect which);
	virtual void pauseAudio(AudioAffect which);
	virtual void resumeAudio(AudioAffect which);
	virtual void pauseAmbient(Bool shouldPause);
	virtual void stopAllAmbientsBy(Object *obj);
	virtual void stopAllAmbientsBy(Drawable *draw);
	virtual void killAudioEventImmediately(AudioHandle audioEvent);
	virtual void nextMusicTrack(void);
	virtual void prevMusicTrack(void);
	virtual Bool isMusicPlaying(void) const;
	virtual Bool hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const;
	virtual AsciiString getMusicTrackName(void) const;
	virtual void openDevice(void);
	virtual void closeDevice(void);
	virtual void *getDevice(void);
	virtual void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags);
	virtual UnsignedInt getProviderCount(void) const;
	virtual AsciiString getProviderName(UnsignedInt providerNum) const;
	virtual UnsignedInt getProviderIndex(AsciiString providerName) const;
	virtual void selectProvider(UnsignedInt providerNdx);
	virtual void unselectProvider(void);
	virtual UnsignedInt getSelectedProvider(void) const;
	virtual void setSpeakerType(UnsignedInt speakerType);
	virtual UnsignedInt getSpeakerType(void);
	virtual void *getHandleForBink(void);
	virtual void releaseHandleForBink(void);
	virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS *eventToPlay);
	virtual UnsignedInt getNum2DSamples(void) const;
	virtual UnsignedInt getNum3DSamples(void) const;
	virtual UnsignedInt getNumStreams(void) const;
	virtual Bool doesViolateLimit(AudioEventRTS *event) const;
	virtual Bool isPlayingLowerPriority(AudioEventRTS *event) const;
	virtual Bool isPlayingAlready(AudioEventRTS *event) const;
	virtual Bool isObjectPlayingVoice(UnsignedInt objID) const;
	virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume);
	virtual void removePlayingAudio(AsciiString eventName);
	virtual void removeAllDisabledAudio();
	virtual void setHardwareAccelerated(Bool accel);
	virtual void setSpeakerSurround(Bool surround);
	virtual void setPreferredProvider(AsciiString provider);
	virtual void setPreferredSpeaker(AsciiString speakerType);
	virtual Real getFileLengthMS(AsciiString strToLoad) const;
	virtual void closeAnySamplesUsingFile(const void *fileToClose);

protected:
	virtual void setDeviceListenerPosition(void);

private:
	bool LoadWavFile(const char *filename, XAUDIO2_BUFFER &buffer, WAVEFORMATEX &wfx);
	void ReleaseCurrentSound();

	IXAudio2 *m_xaudio2;
	IXAudio2MasteringVoice *m_masteringVoice;
	IXAudio2SourceVoice *m_sourceVoice;

	XAUDIO2_BUFFER m_currentBuffer;
	bool m_isPlaying;
	float m_volume;
};

#endif // __XAUDIO2_ENGINE_H
