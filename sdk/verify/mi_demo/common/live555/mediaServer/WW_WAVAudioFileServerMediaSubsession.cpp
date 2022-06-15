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
// Implementation

#include "WW_WAVAudioFileServerMediaSubsession.hh"
#include "WW_WAVAudioFileSource.hh"
#include "uLawAudioFilter.hh"
#include "SimpleRTPSink.hh"

WW_WAVAudioFileServerMediaSubsession* WW_WAVAudioFileServerMediaSubsession::createNew(UsageEnvironment& env
									, char const* fileName, WWAV_AUDIO_FORMAT AudioFormat
									, unsigned SamplingFrequency, unsigned NumChannels
									, unsigned char BitsPerSample, OPENSTREAM  OpenStreamFun
									, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
									, void* CallbackFunArg, int iCallbackFunArgSize
									, Boolean copyArgFlag, Boolean reuseFirstSource)
{
	if (BitsPerSample%4 != 0 || BitsPerSample < 4 || BitsPerSample > 24 || BitsPerSample == 12) {
		env << "The input file contains " << BitsPerSample << " bit-per-sample audio, which we don't handle\n";
		return NULL;
	}
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	return new WW_WAVAudioFileServerMediaSubsession(env, fileName, reuseFirstSource, AudioFormat
										, SamplingFrequency, NumChannels, BitsPerSample
										, OpenStreamFun, ReadStreamFun, CloseStreamFun
										, CallbackFunArg, iCallbackFunArgSize, copyArgFlag);
}

WW_WAVAudioFileServerMediaSubsession::WW_WAVAudioFileServerMediaSubsession(UsageEnvironment& env
									, char const* fileName, Boolean reuseFirstSource
									, WWAV_AUDIO_FORMAT AudioFormat, unsigned SamplingFrequency
									, unsigned NumChannels, unsigned char BitsPerSample
									, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
									, CLOSESTREAM CloseStreamFun, void* CallbackFunArg
									, int iCallbackFunArgSize, Boolean copyArgFlag)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, fFileDuration(0)
{
	fAudioFormat          = AudioFormat;
	fSamplingFrequency    = SamplingFrequency; 
	fNumChannels          = NumChannels;
	fBitsPerSample        = BitsPerSample;
	m_OpenStreamFun       = OpenStreamFun;
	m_ReadStreamFun       = ReadStreamFun;
	m_CloseStreamFun      = CloseStreamFun;
	m_CallbackFunArg      = CallbackFunArg;
	m_iCallbackFunArgSize = iCallbackFunArgSize;
	m_CopyArgFlag         = copyArgFlag;
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		m_CallbackFunArg = calloc(1, m_iCallbackFunArgSize);
		memcpy(m_CallbackFunArg, CallbackFunArg, m_iCallbackFunArgSize);
	}
	memset(fFileName, 0, sizeof(fFileName));
	strcpy(fFileName, fileName);
}

WW_WAVAudioFileServerMediaSubsession::~WW_WAVAudioFileServerMediaSubsession() 
{
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		free(m_CallbackFunArg);
		m_CallbackFunArg = NULL;
	}
}

FramedSource* WW_WAVAudioFileServerMediaSubsession
				::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
	FramedSource* resultSource = NULL;
	do {
		WW_WAVAudioFileSource* wavSource = WW_WAVAudioFileSource::createNew(envir(), fFileName
									, (WAV_AUDIO_FORMAT)fAudioFormat, fSamplingFrequency
									, fNumChannels, fBitsPerSample, m_OpenStreamFun
									, m_ReadStreamFun, m_CloseStreamFun, m_CallbackFunArg);
		if (wavSource == NULL) {
			break;
		}

		unsigned bitsPerSecond = fSamplingFrequency*fBitsPerSample*fNumChannels;

		//fFileDuration = (float)((8.0*wavSource->numPCMBytes())/(fSamplingFrequency*fNumChannels*fBitsPerSample));

		// Add in any filter necessary to transform the data prior to streaming:
		resultSource = wavSource; // by default
		if (fAudioFormat == WA_PCM) {
			if (fBitsPerSample == 16) {			
				resultSource = EndianSwap16::createNew(envir(), wavSource);
			} else if (fBitsPerSample == 20 || fBitsPerSample == 24) {
				resultSource = EndianSwap24::createNew(envir(), wavSource);
			}
		}

		estBitrate = (bitsPerSecond+500)/1000; // kbps
		return resultSource;
	} while (0);

	// An error occurred:
	Medium::close(resultSource);
	return NULL;
}

RTPSink* WW_WAVAudioFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock
						, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) 
{
	do {
		char const* mimeType;
		unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // by default, unless a static RTP payload type can be used
		if (fAudioFormat == WA_PCM) {
			if (fBitsPerSample == 16) {
				mimeType = "L16";
				if (fSamplingFrequency == 44100 && fNumChannels == 2) {
					payloadFormatCode = 10; // a static RTP payload type
				} else if (fSamplingFrequency == 44100 && fNumChannels == 1) {
					payloadFormatCode = 11; // a static RTP payload type
				}
			} else if (fBitsPerSample == 20) {
				mimeType = "L20";
			} else if (fBitsPerSample == 24) {
				mimeType = "L24";
			} else { // fBitsPerSample == 8 (we assume that fBitsPerSample == 4 is only for WA_IMA_ADPCM)
				mimeType = "L8";
			}
		} else if (fAudioFormat == WA_PCMU) {
			mimeType = "PCMU";
			if (fSamplingFrequency == 8000 && fNumChannels == 1) {
				payloadFormatCode = 0; // a static RTP payload type
			}
		} else if (fAudioFormat == WA_PCMA) {
			mimeType = "PCMA";
			if (fSamplingFrequency == 8000 && fNumChannels == 1) {
				payloadFormatCode = 8; // a static RTP payload type
			}
		} else if (fAudioFormat == WA_IMA_ADPCM) {
			mimeType = "DVI4";
			// Use a static payload type, if one is defined:
			if (fNumChannels == 1) {
				if (fSamplingFrequency == 8000) {
					payloadFormatCode = 5; // a static RTP payload type
				} else if (fSamplingFrequency == 16000) {
					payloadFormatCode = 6; // a static RTP payload type
				} else if (fSamplingFrequency == 11025) {
					payloadFormatCode = 16; // a static RTP payload type
				} else if (fSamplingFrequency == 22050) {
					payloadFormatCode = 17; // a static RTP payload type
				}
			}
		} else { //unknown format
			break;
		}
		return SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode
						, fSamplingFrequency, "audio", mimeType, fNumChannels);
	} while (0);

	// An error occurred:
	return NULL;
}

void WW_WAVAudioFileServerMediaSubsession::testScaleFactor(float& scale) 
{
	if (fFileDuration <= 0.0) {
		// The file is non-seekable, so is probably a live input source.
		// We don't support scale factors other than 1
		scale = 1;
	} else {
		// We support any integral scale, other than 0
		int iScale = scale < 0.0 ? (int)(scale - 0.5) : (int)(scale + 0.5); // round
		if (iScale == 0) {
			iScale = 1;
		}
		scale = (float)iScale;
	}
}

float WW_WAVAudioFileServerMediaSubsession::duration() const 
{
	return fFileDuration;
}
