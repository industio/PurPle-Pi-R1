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
// NOTE: Samples are returned in little-endian order (the same order in which
// they were stored in the file).
// C++ header

#ifndef _WW_WAV_AUDIO_FILE_SOURCE_HH
#define _WW_WAV_AUDIO_FILE_SOURCE_HH


#include "AudioInputDevice.hh"

#include "WW_CallbackFunction.hh"
typedef enum {
  WA_PCM = 0x01,
  WA_PCMA = 0x06,
  WA_PCMU = 0x07,
  WA_IMA_ADPCM = 0x11,
  WA_UNKNOWN
} WAV_AUDIO_FORMAT;


class WW_WAVAudioFileSource: public AudioInputDevice {
public:
	static WW_WAVAudioFileSource* createNew(UsageEnvironment& env, char const* fileName
						, WAV_AUDIO_FORMAT AudioFormat, unsigned SamplingFrequency
						, unsigned NumChannels, unsigned char BitsPerSample
						, OPENSTREAM  OpenStreamFun, READSTREAM ReadStreamFun
						, CLOSESTREAM CloseStreamFun, void* CallbackFunArg);
protected:
	WW_WAVAudioFileSource(UsageEnvironment& env, void* fid, WAV_AUDIO_FORMAT AudioFormat
						, unsigned SamplingFrequency, unsigned NumChannels
						, unsigned char BitsPerSample, OPENSTREAM  OpenStreamFun
						, READSTREAM ReadStreamFun, CLOSESTREAM CloseStreamFun
						, void* CallbackFunArg);
	// called only by createNew()

	virtual ~WW_WAVAudioFileSource();
	virtual unsigned int maxFrameSize() const;  
	static  void fileReadableHandler(WW_WAVAudioFileSource* source, int mask);
	void    doReadFromFile();

private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();
	virtual Boolean setInputPort(int portIndex);
	virtual double getAverageLevel() const;
	static  void doGetNextFrameStatic(FramedSource* source);
protected:
	unsigned fPreferredFrameSize;

private:
	void*       fFid;
	double      fPlayTimePerSample; // useconds
	unsigned    fLastPlayTime; // useconds
	unsigned    fFileSize;
	unsigned    char fAudioFormat;
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void*       m_CallbackFunArg;
};

#endif
