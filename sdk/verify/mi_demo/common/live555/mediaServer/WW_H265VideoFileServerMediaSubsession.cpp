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

#include "WW_H265VideoFileServerMediaSubsession.hh"
#include "H265VideoRTPSink.hh"
#include "WW_ByteStreamFileSource.hh"
#include "WW_H265VideoStreamFramer.hh"

WW_H265VideoFileServerMediaSubsession*
WW_H265VideoFileServerMediaSubsession::createNew(UsageEnvironment& env
										, char const* fileName, OPENSTREAM  OpenStreamFun
										, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
										, double iFrameRate
										, void* CallbackFunArg, int iCallbackFunArgSize
										, Boolean copyArgFlag, Boolean reuseFirstSource)
{
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	return new WW_H265VideoFileServerMediaSubsession(env, fileName, reuseFirstSource
									, OpenStreamFun, ReadStreamFun, CloseStreamFun
									, iFrameRate
									, CallbackFunArg, iCallbackFunArgSize, copyArgFlag);
}

WW_H265VideoFileServerMediaSubsession::WW_H265VideoFileServerMediaSubsession(UsageEnvironment& env
										, char const* fileName, Boolean reuseFirstSource
										, OPENSTREAM  OpenStreamFun, READSTREAM  ReadStreamFun
										, CLOSESTREAM CloseStreamFun, double iFrameRate
										, void* CallbackFunArg
										, int iCallbackFunArgSize, Boolean copyArgFlag)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, fAuxSDPLine(NULL)
	, fDoneFlag(0)
	, fDummyRTPSink(NULL)
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

WW_H265VideoFileServerMediaSubsession::~WW_H265VideoFileServerMediaSubsession() 
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
	WW_H265VideoFileServerMediaSubsession* subsess
		= (WW_H265VideoFileServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
}

void WW_H265VideoFileServerMediaSubsession::afterPlayingDummy1() 
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) 
{
	WW_H265VideoFileServerMediaSubsession* subsess
		= (WW_H265VideoFileServerMediaSubsession*)clientData;
	subsess->checkForAuxSDPLine1();
}

void WW_H265VideoFileServerMediaSubsession::checkForAuxSDPLine1() 
{
	nextTask() = NULL;

	char const* dasl;
	if (fAuxSDPLine != NULL) {
		// Signal the event loop that we're done:
		setDoneFlag();
	} else if (fDummyRTPSink != NULL 
				&& (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;

		// Signal the event loop that we're done:
		setDoneFlag();
	} else if (!fDoneFlag) {
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
										(TaskFunc*)checkForAuxSDPLine, this);
	}
}

char const* WW_H265VideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) 
{
	if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

	if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
		// Note: For H265 video files, the 'config' information (used for several payload-format
		// specific parameters in the SDP description) isn't known until we start reading the file.
		// This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
		// and we need to start reading data from our file until this changes.
		fDummyRTPSink = rtpSink;

		// Start reading the file:
		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

		// Check whether the sink's 'auxSDPLine()' is ready:
		checkForAuxSDPLine(this);
	}

	envir().taskScheduler().doEventLoop(&fDoneFlag);

	return fAuxSDPLine;
}

FramedSource* WW_H265VideoFileServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
	estBitrate = 500; // kbps, estimate

	// Create the video source:
	WW_ByteStreamFileSource* fileSource = WW_ByteStreamFileSource::createNew(envir(), fFileName
						, m_OpenStreamFun, m_ReadStreamFun, m_CloseStreamFun, m_CallbackFunArg);
	if (fileSource == NULL) return NULL;
	//fFileSize = fileSource->fileSize();

	// Create a framer for the Video Elementary Stream:
	return WW_H265VideoStreamFramer::createNew(envir(), fileSource, False, m_fFrameRate);
}

RTPSink* WW_H265VideoFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock
					, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) 
{
	return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

int WW_H265VideoFileServerMediaSubsession::GetVideoFrame(char* data, int iDataLen, char** pcOutData, int* piOutLen, char* NALUHead) 
{
	int i = 0;
	if(iDataLen==0){printf("what the fuck!!\n");return -1;}
	for (i = 0; i < iDataLen; i++) {
		if ( data[i] == 0x00 ) {
			if ( (data[i+1]==0x00) && (data[i+2]==0x00) && (data[i+3]==0x01) ) {
				*NALUHead = data[i+4];
				*pcOutData = &data[i];
				//printf("NALUHead 1 = %x\n",*NALUHead);
				break;
			} else if ( (data[i+1]==0x00) && (data[i+2]==0x01) ) {
				*NALUHead = data[i+3];
				*pcOutData = &data[i];
				//printf("NALUHead 2 = %x\n",*NALUHead);
				break;
			}
		}
	}
	if(i==iDataLen){printf("GetVideoFrame error!!!\n");return -1;}
	for ( int j=i+3; j<iDataLen; j++) {
		if ( data[j] == 0x00 ) {
			if ( (data[j+1]==0x00) && (data[j+2]==0x00) && (data[j+3]==0x01) ) {
				*piOutLen = j-i;
				return 0;
			} else if ( (data[j+1]==0x00) && (data[j+2]==0x01) ) {
				*piOutLen = j-i;
				return 0;
			}
		}
	}
	//if(j==iDataLen){*piOutLen = j-i;return 0;}  //the last frame of buf
	//return -1;
	*piOutLen = iDataLen;
	return 0;
}


