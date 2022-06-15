/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2017 Live Networks, Inc.  All rights reserved.
// A source object for AAC audio files in ADTS format
// C++ header

#ifndef _WW_ADTS_AUDIO_FILE_SOURCE_HH
#define _WW_ADTS_AUDIO_FILE_SOURCE_HH


#include "FramedSource.hh"
#include "WW_CallbackFunction.hh"

class WW_ADTSAudioFileSource: public FramedSource {
public:
	static WW_ADTSAudioFileSource* createNew(UsageEnvironment& env, char const* fileName
					, u_int32_t sampleRete, u_int8_t channel, OPENSTREAM  OpenStreamFun
					, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
					, void* CallbackFunArg, int profile=1);

	unsigned samplingFrequency() const { return fSamplingFrequency; }
	unsigned numChannels() const { return fNumChannels; }
	char const* configStr() const { return fConfigStr; }
	// returns the 'AudioSpecificConfig' for this stream (in ASCII form)

private:
	WW_ADTSAudioFileSource(UsageEnvironment& env, void* fid
					, u_int32_t samplingFrequency, u_int8_t channelConfiguration
					, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
					, CLOSESTREAM CloseStreamFun, void* CallbackFunArg
					, int profile);//profile,0:MAIN,1:LC,2:SSR,3:reserved
	// called only by createNew()
	int GetSampleRateIndex(unsigned int sampleRate);
	virtual ~WW_ADTSAudioFileSource();

private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	static  void doGetNextFrameStatic(FramedSource* source);
private:
	unsigned fSamplingFrequency;
	unsigned fNumChannels;
	unsigned fuSecsPerFrame;
	char fConfigStr[5];
	void* fFid;
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void* m_CallbackFunArg;
};

#endif
