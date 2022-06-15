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
// Implementation

#include "WW_ADTSAudioFileServerMediaSubsession.hh"
#include "WW_ADTSAudioFileSource.hh"
#include "MPEG4GenericRTPSink.hh"

WW_ADTSAudioFileServerMediaSubsession*
WW_ADTSAudioFileServerMediaSubsession::createNew(UsageEnvironment& env
										, char const* fileName
										, u_int32_t sampleRete
										, u_int8_t channel
										, OPENSTREAM  OpenStreamFun
										, READSTREAM  ReadStreamFun
										, CLOSESTREAM CloseStreamFun
										, void* CallbackFunArg
										, int iCallbackFunArgSize
										, Boolean copyArgFlag
										, int profile
										, Boolean reuseFirstSource)
{
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	return new WW_ADTSAudioFileServerMediaSubsession(env, fileName, reuseFirstSource
										, sampleRete, channel, OpenStreamFun
										, ReadStreamFun, CloseStreamFun
										, CallbackFunArg, iCallbackFunArgSize
										, copyArgFlag, profile);
}

WW_ADTSAudioFileServerMediaSubsession::WW_ADTSAudioFileServerMediaSubsession(UsageEnvironment& env
										, char const* fileName, Boolean reuseFirstSource
									    , u_int32_t sampleRete, u_int8_t channel
									    , OPENSTREAM  OpenStreamFun
										, READSTREAM  ReadStreamFun
										, CLOSESTREAM CloseStreamFun
										, void* CallbackFunArg
										, int iCallbackFunArgSize
										, Boolean copyArgFlag
										, int profile)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, fFileSize(0) 
{
	memset(fFileName, 0, sizeof(fFileName));
	strcpy(fFileName, fileName);
	fSamplingFrequency    = sampleRete;
	fNumChannels          = channel;
	m_OpenStreamFun       = OpenStreamFun;
	m_ReadStreamFun       = ReadStreamFun;
	m_CloseStreamFun      = CloseStreamFun;
	m_CallbackFunArg      = CallbackFunArg;
	m_AACProfile          = profile;
	m_iCallbackFunArgSize = iCallbackFunArgSize;
	m_CopyArgFlag         = copyArgFlag;
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		m_CallbackFunArg = calloc(1, m_iCallbackFunArgSize);
		memcpy(m_CallbackFunArg, CallbackFunArg, m_iCallbackFunArgSize);
	}
}

WW_ADTSAudioFileServerMediaSubsession::~WW_ADTSAudioFileServerMediaSubsession() 
{
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		free(m_CallbackFunArg);
		m_CallbackFunArg = NULL;
	}
}

FramedSource* WW_ADTSAudioFileServerMediaSubsession::createNewStreamSource(
						unsigned clientSessionId, unsigned& estBitrate) {
	estBitrate = 96; // kbps, estimate

	return WW_ADTSAudioFileSource::createNew(envir(), fFileName, fSamplingFrequency
				, fNumChannels, m_OpenStreamFun, m_ReadStreamFun, m_CloseStreamFun
				, m_CallbackFunArg, m_AACProfile);
}

RTPSink* WW_ADTSAudioFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock
						, unsigned char rtpPayloadTypeIfDynamic
						, FramedSource* inputSource) {
	WW_ADTSAudioFileSource* adtsSource = (WW_ADTSAudioFileSource*)inputSource;
	return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock
									, rtpPayloadTypeIfDynamic
									, adtsSource->samplingFrequency()
									, "audio", "AAC-hbr", adtsSource->configStr()
									, adtsSource->numChannels());
}


int WW_ADTSAudioFileServerMediaSubsession::AnalyzeAdtsHead(ADTSHeader* pstAdtsHead, unsigned char *pucAacData, int iAacDataLen)
{
	if((NULL==pstAdtsHead) || (NULL==pucAacData) || (iAacDataLen<7) ) {
		//printf("%d: Input parameters is error!\n", __LINE__);
	}
	
	pstAdtsHead->synword =GetRealValue(pucAacData, 0, 12);							//0~11		12 bslbf
	if(pstAdtsHead->synword != 0xFFF) {
		////printf("%d: Aac flag is error!\n", __LINE__);
		return -1;
	}
	////printf("pstAdtsHead->synword[%x]\n",pstAdtsHead->synword);
	pstAdtsHead->ID = GetRealValue(pucAacData, 12, 1);									//12			1  bslbf
	pstAdtsHead->layer = GetRealValue(pucAacData, 13, 2);								//13~14		2  uimsbf
	pstAdtsHead->protection_absent = GetRealValue(pucAacData, 15, 1);					//15			1  bslbf
	pstAdtsHead->profile_ObjectType = GetRealValue(pucAacData, 16, 2);					//16~17		2  uimsbf
	pstAdtsHead->sampling_frequency_index = GetRealValue(pucAacData, 18, 4);			//18~21		4  uimsbf
	pstAdtsHead->private_bit = GetRealValue(pucAacData, 22, 1);						    //22			1  bslbf
	pstAdtsHead->channel_configuration = GetRealValue(pucAacData, 23, 3);				//23~25		3  uimsbf
	pstAdtsHead->original_copy = GetRealValue(pucAacData, 26, 1);						//26			1  bslbf	
	pstAdtsHead->home = GetRealValue(pucAacData, 27, 1);						//27			1  bslbf	
	pstAdtsHead->copyright_identification_bit = GetRealValue(pucAacData, 28, 1);			//28			1  bslbf
	pstAdtsHead->copyright_identification_start = GetRealValue(pucAacData, 29, 1);		//29			1  bslbf
	pstAdtsHead->aac_frame_length = GetRealValue(pucAacData, 30, 13);					//30~42		13 bslbf
	pstAdtsHead->adts_buffer_fullness = GetRealValue(pucAacData, 33, 11);				//33~53		11 bslbf
	pstAdtsHead->number_of_raw_data_blocks_in_frame = GetRealValue(pucAacData, 54, 2);//54~55		2 uimsfb

	return 0;
}

int WW_ADTSAudioFileServerMediaSubsession::GetRealValue(unsigned char* pucSrcData, int iStartPos, int iBitCount)
{
	int ret=0;
	int i = 0;
	int iBit = 0;
	int iIndex = 0;
	int iOffset = 0;
	
	if( NULL==pucSrcData ) {
		//printf("%d: Input parameters is error!\n", __LINE__);
		return -1;
	}

	//unsigned char *SrcData = pucSrcData;
	int StartPos = iStartPos;
	int BitCount = iBitCount;
	
	for( i=0; i<BitCount; i++ ) {
		iBit = StartPos+i;
		iIndex = iBit/8;
		iOffset = iBit%8;
		ret = ret << 1;
		ret += (pucSrcData[iIndex]&(1<<(7-iOffset)))>0?1:0;
	}
	return ret;
}


int WW_ADTSAudioFileServerMediaSubsession::GetADTSFrame(char* pcAacBuf, int iAacDataLen, char** pucData, int* iAacLen)  
{  
	int lLen = 0; 
	if ( !pcAacBuf || !pucData || !iAacDataLen ) {
		return -1;  
	}  
	while ( 1 ) {  
		if ( iAacDataLen  < 7 ) {
			return -1;  
		}
		
		if ( ((pcAacBuf[0]&0xFF) == 0xFF) && ((pcAacBuf[1] & 0xF0) == 0xF0) ) {  
			// profile; 2 uimsbf
			// sampling_frequency_index; 4 uimsbf
			// private_bit; 1 bslbf
			// channel_configuration; 3 uimsbf
			// original/copy; 1 bslbf
			// home; 1 bslbf
			// copyright_identification_bit; 1 bslbf
			// copyright_identification_start; 1 bslbf
			// frame_length; 13 bslbf
			lLen |= ( (pcAacBuf[3]&0x03)<<11 );     // high 2 bit
			lLen |= ( (pcAacBuf[4]&0xFF)<<3 );      // middle 8 bit
			lLen |= ( (pcAacBuf[5]&0xE0)>>5 );      // low 3bit
			break;  
		}  
		--iAacDataLen;  
		++pcAacBuf;  
	}
	if ( iAacDataLen < lLen ) {  
		return -1;  
	} 
	*pucData = pcAacBuf;
	*iAacLen = lLen; 
	return 0; 
}  



