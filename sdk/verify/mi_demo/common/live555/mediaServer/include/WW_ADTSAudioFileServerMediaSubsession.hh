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
// on demand, from an AAC audio file in ADTS format
// C++ header

#ifndef _WW_ADTS_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH
#define _WW_ADTS_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH

#include "OnDemandServerMediaSubsession.hh"
#include "WW_CallbackFunction.hh"
static unsigned const samplingFrequencyTable[16] = 
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025, 8000,
	7350, 0, 0, 0
};



class WW_ADTSAudioFileServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
	typedef struct
	{//ISO/IEC 14496-3 ADTS²¿·Ö
	    //adts_fixed_header
	    int synword;                                        //0~11      12 bslbf
	    unsigned char ID;                                   //12        1  bslbf
	    unsigned char layer;                                //13~14     2  uimsbf
	    unsigned char protection_absent;                    //15        1  bslbf
	    unsigned char profile_ObjectType;                   //16~17     2  uimsbf
	    unsigned char sampling_frequency_index;             //18~21     4  uimsbf
	    unsigned char private_bit;                          //22        1  bslbf
	    unsigned char channel_configuration;                //23~25     3  uimsbf
	    unsigned char original_copy;                        //26        1  bslbf
	    unsigned char home;                                 //27        1  bslbf
	    //adts_variable_header
	    unsigned char copyright_identification_bit;         //28        1  bslbf
	    unsigned char copyright_identification_start;       //29        1  bslbf
	    unsigned char _[1];
	    int aac_frame_length;                               //30~42     13 bslbf
	    int adts_buffer_fullness;                           //33~53     11 bslbf
	    unsigned char number_of_raw_data_blocks_in_frame;   //54~55     2 uimsfb
	    unsigned char __[3];
	}ADTSHeader;
	static WW_ADTSAudioFileServerMediaSubsession* createNew(UsageEnvironment& env
							, char const* fileName
							, u_int32_t sampleRete, u_int8_t channel
							, OPENSTREAM  OpenStreamFun, READSTREAM  ReadStreamFun
							, CLOSESTREAM CloseStreamFun, void* CallbackFunArg=NULL
							, int iCallbackFunArgSize=0, Boolean copyArgFlag=True
							, int profile=1, Boolean reuseFirstSource=False);
	static int AnalyzeAdtsHead(ADTSHeader* pstAdtsHead, unsigned char *pucAacData
					, int iAacDataLen);
	static int GetRealValue(unsigned char* pucSrcData, int iStartPos, int iBitCount);
	static int GetADTSFrame(char* pcAacBuf, int iAacDataLen, char** pucData, int* iAacLen); 
protected:
	WW_ADTSAudioFileServerMediaSubsession(UsageEnvironment& env, char const* fileName
							, Boolean reuseFirstSource, u_int32_t sampleRete
							, u_int8_t  channel, OPENSTREAM  OpenStreamFun
							, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
							, void* CallbackFunArg, int iCallbackFunArgSize
							, Boolean copyArgFlag, int profile);
	  // called only by createNew();
	virtual ~WW_ADTSAudioFileServerMediaSubsession();

protected: // redefined virtual functions
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock
							, unsigned char rtpPayloadTypeIfDynamic
							, FramedSource* inputSource);
protected:
	char        fFileName[1024];
	u_int64_t   fFileSize; // if known
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void*       m_CallbackFunArg;
	int         m_iCallbackFunArgSize;
	Boolean     m_CopyArgFlag;
	int         m_AACProfile;
	unsigned    fSamplingFrequency;
	unsigned    fNumChannels;
	
};

#endif
