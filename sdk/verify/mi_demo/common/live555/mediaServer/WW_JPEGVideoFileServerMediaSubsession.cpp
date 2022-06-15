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
// on demand, from a H265 video file.
// Implementation

#include "WW_JPEGVideoFileServerMediaSubsession.hh"
#include "JPEGVideoRTPSink.hh"
#include "WW_ByteStreamFileSource.hh"
#include "WW_JPEGVideoStreamFramer.hh"

WW_JPEGVideoFileServerMediaSubsession*
WW_JPEGVideoFileServerMediaSubsession::createNew(UsageEnvironment& env
										, char const* fileName, OPENSTREAM  OpenStreamFun
										, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
										, double iFrameRate
										, void* CallbackFunArg, int iCallbackFunArgSize
										, Boolean copyArgFlag, Boolean reuseFirstSource)
{
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	return new WW_JPEGVideoFileServerMediaSubsession(env, fileName, reuseFirstSource
									, OpenStreamFun, ReadStreamFun, CloseStreamFun
									, iFrameRate
									, CallbackFunArg, iCallbackFunArgSize, copyArgFlag);
}

WW_JPEGVideoFileServerMediaSubsession::WW_JPEGVideoFileServerMediaSubsession(UsageEnvironment& env
										, char const* fileName, Boolean reuseFirstSource
										, OPENSTREAM  OpenStreamFun, READSTREAM  ReadStreamFun
										, CLOSESTREAM CloseStreamFun, double iFrameRate
										, void* CallbackFunArg
										, int iCallbackFunArgSize, Boolean copyArgFlag)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, fAuxSDPLine(NULL)
	, fDoneFlag(0)
	, fFileSize(0)
{
	m_OpenStreamFun       = OpenStreamFun;
	m_ReadStreamFun       = ReadStreamFun;
	m_CloseStreamFun      = CloseStreamFun;
	m_CallbackFunArg      = CallbackFunArg;
	m_iCallbackFunArgSize = iCallbackFunArgSize;
	m_CopyArgFlag         = copyArgFlag;
	m_fFrameRate          = iFrameRate;
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		m_CallbackFunArg = calloc(1, m_iCallbackFunArgSize);
		memcpy(m_CallbackFunArg, CallbackFunArg, m_iCallbackFunArgSize);
	}
	memset(fFileName, 0, sizeof(fFileName));
	strcpy(fFileName, fileName);
}

WW_JPEGVideoFileServerMediaSubsession::~WW_JPEGVideoFileServerMediaSubsession()
{
	if ( fAuxSDPLine ) {
		delete[] fAuxSDPLine;
		fAuxSDPLine = NULL;
	}
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		free(m_CallbackFunArg);
		m_CallbackFunArg = NULL;
	}
}

static void afterPlayingDummy(void* clientData)
{
	WW_JPEGVideoFileServerMediaSubsession* subsess
		= (WW_JPEGVideoFileServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
}

void WW_JPEGVideoFileServerMediaSubsession::afterPlayingDummy1()
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

char const* WW_JPEGVideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	WW_JPEGVideoStreamSource *fDummySource = (WW_JPEGVideoStreamSource *)inputSource;

	if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

	// we're not already setting it up for another, concurrent stream
	// Note: For H265 video files, the 'config' information (used for several payload-format
	// specific parameters in the SDP description) isn't known until we start reading the file.
	// This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
	// and we need to start reading data from our file until this changes.

	// Start reading the file:
	rtpSink->startPlaying(*inputSource, afterPlayingDummy, this);

	// Check whether the sink's 'auxSDPLine()' is ready:
	// checkForAuxSDPLine(this);

	char const* addSDPFormat =
            "b=AS:%d\r\n"
            "a=framerate:%d\r\n"
            "a=framesize:%d %d-%d\r\n"
            ;

	unsigned addSDPFormatSize = strlen(addSDPFormat)
                            + 2 /* max char len */
                            + 4 /* max char len */
                            + 2 + 4 + 4
                            + 1;

	char* fmtp = new char[addSDPFormatSize];
	sprintf(fmtp,
		    addSDPFormat,
		    4096,
		    30,
		    rtpSink->rtpPayloadType(),
		    fDummySource->widthPixels(),
		    fDummySource->heightPixels()
	    );

	fAuxSDPLine = strDup(fmtp);
	delete[] fmtp;

	return fAuxSDPLine;
}

FramedSource* WW_JPEGVideoFileServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	estBitrate = 500; // kbps, estimate

	// Create the video source:
	WW_ByteStreamFileSource* fileSource = WW_ByteStreamFileSource::createNew(envir(), fFileName
						, m_OpenStreamFun, m_ReadStreamFun, m_CloseStreamFun, m_CallbackFunArg);
	if (fileSource == NULL) return NULL;
	//fFileSize = fileSource->fileSize();

	// Create a framer for the Video Elementary Stream:
	return WW_JPEGVideoStreamSource::createNew(envir(), fileSource);
}

RTPSink* WW_JPEGVideoFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock
					, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
	return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}

