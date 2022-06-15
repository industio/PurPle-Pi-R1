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
// Implementation

#include "WW_ADTSAudioFileSource.hh"
//#include "InputFile.hh"
#include <GroupsockHelper.hh>
#if !(defined(WIN32)&&!defined(__MINGW32__))
#include <unistd.h>
#endif
////////// WW_ADTSAudioFileSource //////////

WW_ADTSAudioFileSource*WW_ADTSAudioFileSource::createNew(UsageEnvironment& env, char const* fileName
								, u_int32_t sampleRete, u_int8_t channel, OPENSTREAM  OpenStreamFun
								, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
								, void* CallbackFunArg, int profile)
{
	void* fid = (FILE*)OpenStreamFun(fileName, CallbackFunArg);
	if (fid == NULL) return NULL;
	return new WW_ADTSAudioFileSource(env, fid, sampleRete, channel
								, OpenStreamFun, ReadStreamFun, CloseStreamFun
								, CallbackFunArg, profile);
}

WW_ADTSAudioFileSource::WW_ADTSAudioFileSource(UsageEnvironment& env, void* fid
								, u_int32_t samplingFrequency, u_int8_t channelConfiguration
								, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
								, CLOSESTREAM CloseStreamFun, void* CallbackFunArg, int profile)
	: FramedSource(env)
	, fuSecsPerFrame(0) 
	, fFid(fid) 
{
	fPresentationTime.tv_sec = 0;
	fPresentationTime.tv_usec = 0;
	m_OpenStreamFun = OpenStreamFun;
	m_ReadStreamFun = ReadStreamFun;
	m_CloseStreamFun = CloseStreamFun;
	m_CallbackFunArg = CallbackFunArg;
	fSamplingFrequency = samplingFrequency;
	int samplingFrequencyIndex = GetSampleRateIndex(fSamplingFrequency);
	fNumChannels = channelConfiguration;
	//printf("--%d %d\n", fSamplingFrequency, fNumChannels);
	fuSecsPerFrame = (1024*1000000) / fSamplingFrequency;//每个采样消耗多少时间 
	unsigned char audioSpecificConfig[2];
	u_int8_t const audioObjectType = profile + 1;
	audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
	audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
	sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
	fNumTruncatedBytes = 0;
}

WW_ADTSAudioFileSource::~WW_ADTSAudioFileSource() 
{
	m_CloseStreamFun(fFid, m_CallbackFunArg);
}

// Note: We should change the following to use asynchronous file reading, #####
// as we now do with ByteStreamFileSource. #####
void WW_ADTSAudioFileSource::doGetNextFrameStatic(FramedSource* source)
{
	source->doGetNextFrame();
}

void WW_ADTSAudioFileSource::doGetNextFrame() 
{
	fFrameSize = 0;
	//int headersLen = sizeof headers;
	//gettimeofday(&fPresentationTime, NULL);
	struct timeval oldPresentationTime=fPresentationTime;
	int iFrameSize=m_ReadStreamFun(fFid, fTo, fMaxSize, &fPresentationTime, m_CallbackFunArg);
	if ( iFrameSize < 0) {
		// The input source has ended:
		handleClosure();
		return;
	} else if (iFrameSize == 0) {
		//nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
		//		(TaskFunc*)FramedSource::afterGetting, this);
		nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
				(TaskFunc*)WW_ADTSAudioFileSource::doGetNextFrameStatic, this);
		return ;
	}

	fFrameSize = iFrameSize;
#if 0
	if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
		gettimeofday(&fPresentationTime, NULL);
	}  else if ( (oldPresentationTime.tv_sec==fPresentationTime.tv_sec)
					&&(oldPresentationTime.tv_usec==fPresentationTime.tv_usec) ) {
		unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
		fPresentationTime.tv_sec += uSeconds/1000000;
		fPresentationTime.tv_usec = uSeconds%1000000;
	}
#else
	if ( ((fPresentationTime.tv_sec==0)&&(fPresentationTime.tv_usec==0))
			||((oldPresentationTime.tv_sec==fPresentationTime.tv_sec)
				&&(oldPresentationTime.tv_usec==fPresentationTime.tv_usec)) ) {
		gettimeofday(&fPresentationTime, NULL);
	}
#endif
	fDurationInMicroseconds = fuSecsPerFrame;
	FramedSource::afterGetting(this);
}

int WW_ADTSAudioFileSource::GetSampleRateIndex(unsigned int sampleRate)
{
	if (92017 <= sampleRate)
		return 0;
	if (75132 <= sampleRate)
		return 1;
	if (55426 <= sampleRate)
		return 2;
	if (46009 <= sampleRate)
		return 3;
	if (37566 <= sampleRate)
		return 4;
	if (27713 <= sampleRate)
		return 5;
	if (23004 <= sampleRate)
		return 6;
	if (18783 <= sampleRate)
		return 7;
	if (13856 <= sampleRate)
		return 8;
	if (11502 <= sampleRate)
		return 9;
	if (9391 <= sampleRate)
		return 10;
	if (7900 <= sampleRate)
		return 11;
	return 12;
}
