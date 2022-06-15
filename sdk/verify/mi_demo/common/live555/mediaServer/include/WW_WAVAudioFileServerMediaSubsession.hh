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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from an WAV audio file.
// C++ header

#ifndef _WW_WAV_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH
#define _WW_WAV_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH


#include "OnDemandServerMediaSubsession.hh"

#include "WW_CallbackFunction.hh"
typedef enum {
	WWA_PCM       = 0x01,
	WWA_PCMA      = 0x06,
	WWA_PCMU      = 0x07,
	WWA_IMA_ADPCM = 0x11,
	WWA_UNKNOWN
} WWAV_AUDIO_FORMAT;
class WW_WAVAudioFileServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
	static WW_WAVAudioFileServerMediaSubsession*createNew(UsageEnvironment& env
								, char const* fileName
								, WWAV_AUDIO_FORMAT AudioFormat, unsigned SamplingFrequency
								, unsigned NumChannels, unsigned char BitsPerSample
								, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
								, CLOSESTREAM CloseStreamFun, void* CallbackFunArg=NULL
								, int iCallbackFunArgSize=0, Boolean copyArgFlag=True
								, Boolean reuseFirstSource=False);

protected:
	WW_WAVAudioFileServerMediaSubsession(UsageEnvironment& env, char const* fileName
								, Boolean reuseFirstSource, WWAV_AUDIO_FORMAT AudioFormat
								, unsigned SamplingFrequency, unsigned NumChannels
								, unsigned char BitsPerSample, OPENSTREAM  OpenStreamFun
								, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
								, void* CallbackFunArg, int iCallbackFunArgSize
								, Boolean copyArgFlag);
	virtual ~WW_WAVAudioFileServerMediaSubsession();

protected:
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic
										, FramedSource* inputSource);
	virtual void testScaleFactor(float& scale);
	virtual float duration() const;

protected:
	unsigned char fAudioFormat;
	unsigned char fBitsPerSample;
	unsigned    fSamplingFrequency;
	unsigned    fNumChannels;
	float       fFileDuration;
	char        fFileName[1024];
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void*       m_CallbackFunArg;
	int         m_iCallbackFunArgSize;
	Boolean     m_CopyArgFlag;
};

#endif
