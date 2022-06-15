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
// C++ header

#ifndef _WW_BYTE_STREAM_FILE_SOURCE_HH
#define _WW_BYTE_STREAM_FILE_SOURCE_HH

#include "FramedSource.hh"
#include "WW_CallbackFunction.hh"
class WW_ByteStreamFileSource: public FramedSource {
public:
	static WW_ByteStreamFileSource* createNew(UsageEnvironment& env, char const* StreamName
					, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
					, CLOSESTREAM CloseStreamFun, void* CallbackFunArg);
protected:
	WW_ByteStreamFileSource(UsageEnvironment& env, void* fid, OPENSTREAM  OpenStreamFun
						, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun, void* CallbackFunArg);
	virtual ~WW_ByteStreamFileSource();

	static void fileReadableHandler(WW_ByteStreamFileSource* source, int mask);
	void doReadFromFile();
	virtual unsigned int maxFrameSize() const;  
private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();
	static  void doGetNextFrameStatic(FramedSource* source);
protected:
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void* m_CallbackFunArg;
	void* fFid;

};


#endif


