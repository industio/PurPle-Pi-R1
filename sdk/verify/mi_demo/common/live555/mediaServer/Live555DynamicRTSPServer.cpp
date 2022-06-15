/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.
  
  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include "Live555DynamicRTSPServer.hh"

Live555DynamicRTSPServer* Live555DynamicRTSPServer::createNew(
									UsageEnvironment& env, Port ourPort
									, UserAuthenticationDatabase* authDatabase
									, unsigned reclamationTestSeconds) 
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if ( ourSocket == -1 ) {
		return NULL;
	}
	return new Live555DynamicRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

Live555DynamicRTSPServer::Live555DynamicRTSPServer(UsageEnvironment& env
									, int ourSocket, Port ourPort
									, UserAuthenticationDatabase* authDatabase
									, unsigned reclamationTestSeconds)
	: RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase
			, reclamationTestSeconds)
	, m_CreateNewSMSFun(NULL)
{

}

Live555DynamicRTSPServer::~Live555DynamicRTSPServer() 
{
	m_CreateNewSMSFun = NULL;
}

int Live555DynamicRTSPServer::SetCreateNewSMSFun(CREATENewSMS CreateNewSMSFun)
{
	if ( CreateNewSMSFun==NULL ) {
		return -1;
	}
	m_CreateNewSMSFun = CreateNewSMSFun;
	return 0;
}

ServerMediaSession* Live555DynamicRTSPServer::lookupServerMediaSession(
						char const* streamName, Boolean isFirstLookupInSession)
{
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
	if ( sms && isFirstLookupInSession ) {
		removeServerMediaSession(sms); 
		sms = NULL;
	} 
	if ( sms == NULL ) {
		if ( m_CreateNewSMSFun ) {
			sms = m_CreateNewSMSFun(envir(), streamName); 
			if ( sms ) {
				addServerMediaSession(sms);
			}
		}
	}
	return sms;
}

