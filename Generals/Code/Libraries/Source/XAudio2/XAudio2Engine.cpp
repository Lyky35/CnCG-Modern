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
 *                     $Archive:: /Commando/Code/Libraries/Source/XAudio2/XAudio2Engine.cpp   $*
 *                                                                                             *
 *                       Author:: XAudio2 Replacement                                          *
 *                                                                                             *
 *                    $Revision:: 1                                                            $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   XAudio2 implementation of the audio engine. Replaces the Miles Sound System.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "XAudio2Engine.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioSettings.h"
#include "Common/FileSystem.h"
#include "Common/file.h"

#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// XAudio2Engine
///////////////////////////////////////////////////////////////////////////////
XAudio2Engine::XAudio2Engine()
	: m_xaudio2(NULL),
	  m_masteringVoice(NULL),
	  m_sourceVoice(NULL),
	  m_isPlaying(false),
	  m_volume(1.0f)
{
	ZeroMemory(&m_currentBuffer, sizeof(m_currentBuffer));
}

///////////////////////////////////////////////////////////////////////////////
// ~XAudio2Engine
///////////////////////////////////////////////////////////////////////////////
XAudio2Engine::~XAudio2Engine()
{
	Shutdown();
}

///////////////////////////////////////////////////////////////////////////////
// Initialize
///////////////////////////////////////////////////////////////////////////////
bool XAudio2Engine::Initialize()
{
	if (m_xaudio2) {
		return true;
	}

	HRESULT hr = XAudio2Create(&m_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) {
		return false;
	}

	hr = m_xaudio2->CreateMasteringVoice(&m_masteringVoice);
	if (FAILED(hr)) {
		m_xaudio2->Release();
		m_xaudio2 = NULL;
		return false;
	}

	m_masteringVoice->SetVolume(m_volume);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Shutdown
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::Shutdown()
{
	ReleaseCurrentSound();

	if (m_masteringVoice) {
		m_masteringVoice->DestroyVoice();
		m_masteringVoice = NULL;
	}

	if (m_xaudio2) {
		m_xaudio2->Release();
		m_xaudio2 = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
// LoadWavFile
///////////////////////////////////////////////////////////////////////////////
bool XAudio2Engine::LoadWavFile(const char *filename, XAUDIO2_BUFFER &buffer, WAVEFORMATEX &wfx)
{
	ZeroMemory(&buffer, sizeof(buffer));
	ZeroMemory(&wfx, sizeof(wfx));

	File *file = TheFileSystem->openFile(filename, File::READ);
	if (!file) {
		return false;
	}

	Int fileSize = file->size();
	if (fileSize < 44) {
		file->close();
		return false;
	}

	unsigned char *fileData = new unsigned char[fileSize];
	if (!fileData) {
		file->close();
		return false;
	}

	Int bytesRead = file->read(fileData, fileSize);
	file->close();

	if (bytesRead < 44) {
		delete[] fileData;
		return false;
	}


	// Parse RIFF/WAVE header
	// "RIFF" + size + "WAVE"
	if (memcmp(fileData, "RIFF", 4) != 0 || memcmp(fileData + 8, "WAVE", 4) != 0) {
		delete[] fileData;
		return false;
	}

	// Find "fmt " chunk
	unsigned int pos = 12;
	bool foundFmt = false;
	bool foundData = false;

	while (pos + 8 <= fileSize) {
		char chunkId[5] = {0};
		memcpy(chunkId, fileData + pos, 4);
		unsigned int chunkSize = fileData[pos + 4] |
			(fileData[pos + 5] << 8) |
			(fileData[pos + 6] << 16) |
			(fileData[pos + 7] << 24);

		if (memcmp(chunkId, "fmt ", 4) == 0 && pos + 8 + chunkSize <= (UnsignedInt)fileSize) {
			unsigned int fmtPos = pos + 8;
			wfx.wFormatTag = fileData[fmtPos] | (fileData[fmtPos + 1] << 8);
			wfx.nChannels = fileData[fmtPos + 2] | (fileData[fmtPos + 3] << 8);
			wfx.nSamplesPerSec = fileData[fmtPos + 4] |
				(fileData[fmtPos + 5] << 8) |
				(fileData[fmtPos + 6] << 16) |
				(fileData[fmtPos + 7] << 24);
			wfx.nAvgBytesPerSec = fileData[fmtPos + 8] |
				(fileData[fmtPos + 9] << 8) |
				(fileData[fmtPos + 10] << 16) |
				(fileData[fmtPos + 11] << 24);
			wfx.nBlockAlign = fileData[fmtPos + 12] | (fileData[fmtPos + 13] << 8);
			wfx.wBitsPerSample = fileData[fmtPos + 14] | (fileData[fmtPos + 15] << 8);
			wfx.cbSize = 0;
			foundFmt = true;
		}

		if (memcmp(chunkId, "data", 4) == 0 && pos + 8 + chunkSize <= (UnsignedInt)fileSize) {
			buffer.AudioBytes = chunkSize;
			buffer.pAudioData = fileData + pos + 8;
			buffer.PlayBegin = 0;
			buffer.PlayLength = 0;
			buffer.LoopBegin = 0;
			buffer.LoopLength = 0;
			buffer.LoopCount = 0;
			buffer.pContext = fileData;
			foundData = true;
			break;
		}

		pos += 8 + chunkSize;
		if (chunkSize & 1) {
			pos++;
		}
	}

	if (!foundFmt || !foundData) {
		delete[] fileData;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// ReleaseCurrentSound
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::ReleaseCurrentSound()
{
	if (m_sourceVoice) {
		m_sourceVoice->Stop();
		m_sourceVoice->FlushSourceBuffers();
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = NULL;
	}

	if (m_currentBuffer.pContext) {
		delete[] (unsigned char *)m_currentBuffer.pContext;
		ZeroMemory(&m_currentBuffer, sizeof(m_currentBuffer));
	}

	m_isPlaying = false;
}

///////////////////////////////////////////////////////////////////////////////
// PlaySound
///////////////////////////////////////////////////////////////////////////////
bool XAudio2Engine::PlaySound(const char *filename)
{
	if (!m_xaudio2 || !filename) {
		return false;
	}

	ReleaseCurrentSound();

	XAUDIO2_BUFFER buffer;
	WAVEFORMATEX wfx;
	if (!LoadWavFile(filename, buffer, wfx)) {
		return false;
	}

	HRESULT hr = m_xaudio2->CreateSourceVoice(&m_sourceVoice, &wfx);
	if (FAILED(hr)) {
		ReleaseCurrentSound();
		return false;
	}

	hr = m_sourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr)) {
		ReleaseCurrentSound();
		return false;
	}

	hr = m_sourceVoice->Start();
	if (FAILED(hr)) {
		ReleaseCurrentSound();
		return false;
	}

	m_currentBuffer = buffer;
	m_isPlaying = true;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// StopSound
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::StopSound()
{
	ReleaseCurrentSound();
}

///////////////////////////////////////////////////////////////////////////////
// SetVolume
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::SetVolume(float volume)
{
	m_volume = volume;
	if (m_volume < 0.0f) m_volume = 0.0f;
	if (m_volume > 1.0f) m_volume = 1.0f;

	if (m_masteringVoice) {
		m_masteringVoice->SetVolume(m_volume);
	}
}

///////////////////////////////////////////////////////////////////////////////
// init
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::init()
{
	AudioManager::init();
	Initialize();
}

///////////////////////////////////////////////////////////////////////////////
// postProcessLoad
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::postProcessLoad()
{
	AudioManager::postProcessLoad();
}

///////////////////////////////////////////////////////////////////////////////
// reset
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::reset()
{
	AudioManager::reset();
	StopSound();
}

///////////////////////////////////////////////////////////////////////////////
// update
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::update()
{
	AudioManager::update();
}

#if defined(_DEBUG) || defined(_INTERNAL)
///////////////////////////////////////////////////////////////////////////////
// audioDebugDisplay
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::audioDebugDisplay(DebugDisplayInterface *dd, void *, FILE *fp)
{
	if (dd) {
		dd->printf("XAudio2 Engine\n");
		dd->printf("Playing: %s\n", m_isPlaying ? "Yes" : "No");
		dd->printf("Volume: %d%%\n", (int)(m_volume * 100.0f));
	}
	if (fp) {
		fprintf(fp, "XAudio2 Engine\n");
		fprintf(fp, "Playing: %s\n", m_isPlaying ? "Yes" : "No");
		fprintf(fp, "Volume: %d%%\n", (int)(m_volume * 100.0f));
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// AudioManager pure virtual stubs
///////////////////////////////////////////////////////////////////////////////
void XAudio2Engine::stopAudio(AudioAffect which) { (void)which; }
void XAudio2Engine::pauseAudio(AudioAffect which) { (void)which; }
void XAudio2Engine::resumeAudio(AudioAffect which) { (void)which; }
void XAudio2Engine::pauseAmbient(Bool shouldPause) { (void)shouldPause; }
void XAudio2Engine::stopAllAmbientsBy(Object *obj) { (void)obj; }
void XAudio2Engine::stopAllAmbientsBy(Drawable *draw) { (void)draw; }
void XAudio2Engine::killAudioEventImmediately(AudioHandle audioEvent) { (void)audioEvent; }
void XAudio2Engine::nextMusicTrack(void) { }
void XAudio2Engine::prevMusicTrack(void) { }
Bool XAudio2Engine::isMusicPlaying(void) const { return FALSE; }
Bool XAudio2Engine::hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const { (void)trackName; (void)numberOfTimes; return FALSE; }
AsciiString XAudio2Engine::getMusicTrackName(void) const { return AsciiString::TheEmptyString; }
void XAudio2Engine::openDevice(void) { }
void XAudio2Engine::closeDevice(void) { }
void *XAudio2Engine::getDevice(void) { return m_xaudio2; }
void XAudio2Engine::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) { (void)audioCompleted; (void)flags; }
UnsignedInt XAudio2Engine::getProviderCount(void) const { return 0; }
AsciiString XAudio2Engine::getProviderName(UnsignedInt providerNum) const { (void)providerNum; return AsciiString::TheEmptyString; }
UnsignedInt XAudio2Engine::getProviderIndex(AsciiString providerName) const { (void)providerName; return PROVIDER_ERROR; }
void XAudio2Engine::selectProvider(UnsignedInt providerNdx) { (void)providerNdx; }
void XAudio2Engine::unselectProvider(void) { }
UnsignedInt XAudio2Engine::getSelectedProvider(void) const { return PROVIDER_ERROR; }
void XAudio2Engine::setSpeakerType(UnsignedInt speakerType) { (void)speakerType; }
UnsignedInt XAudio2Engine::getSpeakerType(void) { return 0; }
void *XAudio2Engine::getHandleForBink(void) { return NULL; }
void XAudio2Engine::releaseHandleForBink(void) { }
void XAudio2Engine::friend_forcePlayAudioEventRTS(const AudioEventRTS *eventToPlay) { (void)eventToPlay; }
UnsignedInt XAudio2Engine::getNum2DSamples(void) const { return 0; }
UnsignedInt XAudio2Engine::getNum3DSamples(void) const { return 0; }
UnsignedInt XAudio2Engine::getNumStreams(void) const { return 0; }
Bool XAudio2Engine::doesViolateLimit(AudioEventRTS *event) const { (void)event; return FALSE; }
Bool XAudio2Engine::isPlayingLowerPriority(AudioEventRTS *event) const { (void)event; return FALSE; }
Bool XAudio2Engine::isPlayingAlready(AudioEventRTS *event) const { (void)event; return FALSE; }
Bool XAudio2Engine::isObjectPlayingVoice(UnsignedInt objID) const { (void)objID; return FALSE; }
void XAudio2Engine::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) { (void)eventName; (void)newVolume; }
void XAudio2Engine::removePlayingAudio(AsciiString eventName) { (void)eventName; }
void XAudio2Engine::removeAllDisabledAudio() { }
void XAudio2Engine::setHardwareAccelerated(Bool accel) { (void)accel; }
void XAudio2Engine::setSpeakerSurround(Bool surround) { (void)surround; }
void XAudio2Engine::setPreferredProvider(AsciiString provider) { (void)provider; }
void XAudio2Engine::setPreferredSpeaker(AsciiString speakerType) { (void)speakerType; }
Real XAudio2Engine::getFileLengthMS(AsciiString strToLoad) const { (void)strToLoad; return 0.0f; }
void XAudio2Engine::closeAnySamplesUsingFile(const void *fileToClose) { (void)fileToClose; }
void XAudio2Engine::setDeviceListenerPosition(void) { }
