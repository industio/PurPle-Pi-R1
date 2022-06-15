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
// A WAV audio file source
// Implementation

#include "WW_WAVAudioFileSource.hh"
//#include "InputFile.hh"
#include "GroupsockHelper.hh"
#if !(defined(WIN32)&&!defined(__MINGW32__))
#include <unistd.h>
#endif
////////// WW_WAVAudioFileSource //////////

WW_WAVAudioFileSource*WW_WAVAudioFileSource::createNew(UsageEnvironment& env, char const* fileName
							, WAV_AUDIO_FORMAT AudioFormat, unsigned SamplingFrequency
							, unsigned NumChannels, unsigned char BitsPerSample
							, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
							, CLOSESTREAM CloseStreamFun, void* CallbackFunArg) 
{
	do {
		void* fid = OpenStreamFun(fileName, CallbackFunArg);
		if (fid == NULL) {
			break;
		}
		
		WW_WAVAudioFileSource* newSource = new WW_WAVAudioFileSource(env, fid, AudioFormat
											, SamplingFrequency, NumChannels, BitsPerSample
											, OpenStreamFun, ReadStreamFun, CloseStreamFun
											, CallbackFunArg);
		if (newSource != NULL && newSource->bitsPerSample() == 0) {
			// The WAV file header was apparently invalid.
			Medium::close(newSource);
			break;
		}
		return newSource;
	} while (0);

	return NULL;
}

WW_WAVAudioFileSource::WW_WAVAudioFileSource(UsageEnvironment& env, void* fid, WAV_AUDIO_FORMAT AudioFormat
  						, unsigned SamplingFrequency, unsigned NumChannels, unsigned char BitsPerSample
  						, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
  						, void* CallbackFunArg)
	: AudioInputDevice(env, 0, 0, 0, 0)/* set the real parameters later */
	, fFid(fid)
	, fPlayTimePerSample(0)
	, fLastPlayTime(0)
	, fFileSize(0)
	, fAudioFormat(WA_UNKNOWN) 
{
	fAudioFormat        = AudioFormat;
	fSamplingFrequency  = SamplingFrequency; 
	fNumChannels        = NumChannels;
	fBitsPerSample      = BitsPerSample;
	m_OpenStreamFun     = OpenStreamFun;
	m_ReadStreamFun     = ReadStreamFun;
	m_CloseStreamFun    = CloseStreamFun;
	m_CallbackFunArg    = CallbackFunArg;
	
	fPlayTimePerSample  = 1e6/(double)fSamplingFrequency;
	fPreferredFrameSize = fSamplingFrequency*fNumChannels*fBitsPerSample/8/2;
}

WW_WAVAudioFileSource::~WW_WAVAudioFileSource() 
{
	if (fFid == NULL) {
		return;
	}
	m_CloseStreamFun(fFid, m_CallbackFunArg);
}

unsigned int WW_WAVAudioFileSource::maxFrameSize() const
{	
    return fPreferredFrameSize;
}

void WW_WAVAudioFileSource::doGetNextFrame() 
{
	
	doReadFromFile();
}

void WW_WAVAudioFileSource::doStopGettingFrames() 
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void WW_WAVAudioFileSource::fileReadableHandler(WW_WAVAudioFileSource* source, int /*mask*/) 
{
	if ( !source->isCurrentlyAwaitingData() ) {
		source->doStopGettingFrames(); // we're not ready for the data yet
		return;
	}
	source->doReadFromFile();
}

void WW_WAVAudioFileSource::doGetNextFrameStatic(FramedSource* source)
{
	source->doGetNextFrame();
}


void WW_WAVAudioFileSource::doReadFromFile() 
{
	fFrameSize = 0;
	if (fPreferredFrameSize>fMaxSize){
		fPreferredFrameSize = fMaxSize;
	}
	unsigned bytesPerSample = (fNumChannels*fBitsPerSample)/8;
	if (bytesPerSample == 0) {
		bytesPerSample = 1; // because we can't read less than a byte at a time
	}
	//gettimeofday(&fPresentationTime, NULL);
	struct timeval oldPresentationTime=fPresentationTime;
	int iFrameSize = m_ReadStreamFun(fFid, fTo, fPreferredFrameSize, &fPresentationTime, m_CallbackFunArg);
	if (iFrameSize < 0) {
		handleClosure();
		return;
	} else if (iFrameSize == 0) {
		//nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
		//		(TaskFunc*)FramedSource::afterGetting, this);
		nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
				(TaskFunc*)WW_WAVAudioFileSource::doGetNextFrameStatic, this);
		return ;
	}
	fFrameSize = iFrameSize;
	// Set the 'presentation time' and 'duration' of this frame:
#if 0
	if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
		// This is the first frame, so use the current time:
		gettimeofday(&fPresentationTime, NULL);
 	} else if ( (oldPresentationTime.tv_sec==fPresentationTime.tv_sec)
 				&&(oldPresentationTime.tv_usec==fPresentationTime.tv_usec) ) {
		unsigned uSeconds = fPresentationTime.tv_usec + fLastPlayTime;
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
	fLastPlayTime = (unsigned)((fPlayTimePerSample*fFrameSize)/bytesPerSample);
	fDurationInMicroseconds = fLastPlayTime;
	FramedSource::afterGetting(this);
}

Boolean WW_WAVAudioFileSource::setInputPort(int /*portIndex*/) 
{
	return True;
}

double WW_WAVAudioFileSource::getAverageLevel() const 
{
	return 0.0;//##### fix this later
}
