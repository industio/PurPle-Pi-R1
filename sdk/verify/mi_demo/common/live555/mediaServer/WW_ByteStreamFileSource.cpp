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
// A file source that is a plain byte stream (rather than frames)
// Implementation

#include "WW_ByteStreamFileSource.hh"
#include "InputFile.hh"
#include "GroupsockHelper.hh"
#if !(defined(WIN32)&&!defined(__MINGW32__))
#include <unistd.h>
#endif
////////// WW_ByteStreamFileSource //////////
#define MAX_VIDEO_FRAME_SIZE 1000000
WW_ByteStreamFileSource*
WW_ByteStreamFileSource::createNew(UsageEnvironment& env, char const* StreamName
	, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun, void* CallbackFunArg) 
{
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	void* fid = OpenStreamFun(StreamName, CallbackFunArg);
	if ( fid==NULL ) {
		return NULL;
	}
	WW_ByteStreamFileSource* newSource = new WW_ByteStreamFileSource(env, fid
											, OpenStreamFun, ReadStreamFun
											, CloseStreamFun, CallbackFunArg);

	return newSource;
}


WW_ByteStreamFileSource::WW_ByteStreamFileSource(UsageEnvironment& env, void* fid, OPENSTREAM  OpenStreamFun
  					, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun, void* CallbackFunArg)
	: FramedSource(env)
	, fFid(fid)
{
	m_OpenStreamFun  = OpenStreamFun;
	m_ReadStreamFun  = ReadStreamFun;
	m_CloseStreamFun = CloseStreamFun;
	m_CallbackFunArg = CallbackFunArg;
}

WW_ByteStreamFileSource::~WW_ByteStreamFileSource() 
{
	if (fFid == NULL) {
		return;
	}
	m_CloseStreamFun(fFid, m_CallbackFunArg);
}

unsigned int WW_ByteStreamFileSource::maxFrameSize() const
{	
    return MAX_VIDEO_FRAME_SIZE;
}

void WW_ByteStreamFileSource::doGetNextFrame() 
{
	doReadFromFile();
}

void WW_ByteStreamFileSource::doStopGettingFrames() 
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void WW_ByteStreamFileSource::fileReadableHandler(WW_ByteStreamFileSource* source, int /*mask*/) 
{
	if ( !source->isCurrentlyAwaitingData() ) {
		source->doStopGettingFrames(); // we're not ready for the data yet
		return;
	}
	source->doReadFromFile();
}

void WW_ByteStreamFileSource::doGetNextFrameStatic(FramedSource* source)
{
	source->doGetNextFrame();
}


void WW_ByteStreamFileSource::doReadFromFile() 
{
	//gettimeofday(&fPresentationTime, NULL);
	struct timeval oldPresentationTime=fPresentationTime;
	fFrameSize = 0;
	int iFrameSize = m_ReadStreamFun(fFid, fTo, fMaxSize, &fPresentationTime, m_CallbackFunArg);
	

	if ( iFrameSize<0 ) {
		handleClosure();
		return;
	} else if (iFrameSize == 0) {
		//nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
		//		(TaskFunc*)FramedSource::afterGetting, this);
		nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
				(TaskFunc*)WW_ByteStreamFileSource::doGetNextFrameStatic, this);
		return ;
	}
	fFrameSize = iFrameSize;
	if ( ((fPresentationTime.tv_sec==0)&&(fPresentationTime.tv_usec==0))
			||((oldPresentationTime.tv_sec==fPresentationTime.tv_sec)
				&&(oldPresentationTime.tv_usec==fPresentationTime.tv_usec)) ) {
		gettimeofday(&fPresentationTime, NULL);
	}

	FramedSource::afterGetting(this);
}
