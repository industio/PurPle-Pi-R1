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
#include "Live555RTSPServer.hh"

#include <ithread.h>
#if 1//defined(linux)
#include <pthread.h>
#include <sys/prctl.h>
#endif

Live555RTSPServer::Live555RTSPServer()
	: m_pBasicTaskScheduler0(NULL)
	, m_pUsageEnvironment(NULL)
	, m_pUserAuthenticationDatabase(NULL)
	, m_pRTSPServer(NULL)
	, m_Thread_handle(NULL)
	, m_iStartFlag(0)
	, m_iPauseFlag(0)
{
	m_pBasicTaskScheduler0 = BasicTaskScheduler::createNew();
	m_pUsageEnvironment = BasicUsageEnvironment::createNew(*m_pBasicTaskScheduler0);
	memset(m_UrlBuffer, 0, sizeof(m_UrlBuffer));
	memset(m_ResultURL, 0, sizeof(m_ResultURL));
}

Live555RTSPServer::~Live555RTSPServer()
{
	m_iStartFlag = 0;
	Join();

	if ( m_pRTSPServer ) {
		Medium::close(m_pRTSPServer);
		m_pRTSPServer = NULL;
	}
	if ( m_pUserAuthenticationDatabase ) {
		delete m_pUserAuthenticationDatabase;
		m_pUserAuthenticationDatabase = NULL;
	}
	if ( m_pUsageEnvironment ) {
		m_pUsageEnvironment->reclaim();
		m_pUsageEnvironment = NULL;
	}
	if ( m_pBasicTaskScheduler0 ) {
		BasicTaskScheduler* p_BasicTaskScheduler = (BasicTaskScheduler*)m_pBasicTaskScheduler0;
		delete p_BasicTaskScheduler;
		m_pBasicTaskScheduler0 = NULL;
	}
}

UsageEnvironment* Live555RTSPServer::GetUsageEnvironmentObj()
{
	return m_pUsageEnvironment;
}

char const* Live555RTSPServer::getResultMsg()
{
	if ( m_pUsageEnvironment==NULL ) {
		return NULL;
	}
	return m_pUsageEnvironment->getResultMsg();
}

int Live555RTSPServer::addUserRecord(char const* username, char const* password)
{
	if (username==NULL) {
		return -1;
	} else if (password==NULL) {
		return -2;
	}
	if ( m_pUserAuthenticationDatabase ) {
		delete m_pUserAuthenticationDatabase;
	}
	m_pUserAuthenticationDatabase = new UserAuthenticationDatabase;
	m_pUserAuthenticationDatabase->addUserRecord(username, password);
	return 0;
}

int Live555RTSPServer::removeUserRecord(char const* username)
{
	if ( m_pUserAuthenticationDatabase==NULL ) {
		return -1;
	}
	m_pUserAuthenticationDatabase->removeUserRecord(username);
	return 0;
}

char const* Live555RTSPServer::lookupPassword(char const* username)
{
	if ( m_pUserAuthenticationDatabase==NULL ) {
		return NULL;
	}
	return m_pUserAuthenticationDatabase->lookupPassword(username);
}

int Live555RTSPServer::SetRTSPServerPort(unsigned short ourPort)
{
	if ( m_pRTSPServer ) {
		return -1;
	}
	m_pRTSPServer=RTSPServer::createNew(*m_pUsageEnvironment, ourPort, m_pUserAuthenticationDatabase);
	if ( m_pRTSPServer==NULL ) {
		return -2;
	}
	return 0;
}

int Live555RTSPServer::SetDynamicRTSPServerPort(CREATENewSMS CreateNewSMSFun, unsigned short ourPort)
{
	if ( m_pRTSPServer ) {
		return -1;
	} else if ( CreateNewSMSFun==NULL ) {
		return -2;
	}
	Live555DynamicRTSPServer* p_objLive555DynamicRTSPServer =
				Live555DynamicRTSPServer::createNew(*m_pUsageEnvironment, ourPort, m_pUserAuthenticationDatabase);
	if ( p_objLive555DynamicRTSPServer==NULL ) {
		return -3;
	}
	p_objLive555DynamicRTSPServer->SetCreateNewSMSFun(CreateNewSMSFun);
	m_pRTSPServer = p_objLive555DynamicRTSPServer;
	return 0;
}

int Live555RTSPServer::CloseRTSPServer()
{
	if ( m_pRTSPServer==NULL ) {
		return -1;
	}
	Medium::close(m_pRTSPServer);
	m_pRTSPServer = NULL;
	return 0;
}

char* Live555RTSPServer::rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket)
{
	if ( serverMediaSession==NULL ) {
		return NULL;
	}
	if ( m_pRTSPServer ) {
		char *resultURL = m_pRTSPServer->rtspURL(serverMediaSession, clientSocket);
		if ( resultURL==NULL ) {
			return NULL;
		}
		memset(m_ResultURL, 0, sizeof(m_ResultURL));
		strncpy(m_ResultURL, resultURL, sizeof(m_ResultURL));
		delete[] resultURL;
		resultURL = NULL;
		return m_ResultURL;
	} else {
		return NULL;
	}
}

char *Live555RTSPServer::rtspURLPrefix(int clientSocket)
{
	if ( m_pRTSPServer ) {
		char *urlBuffer = m_pRTSPServer->rtspURLPrefix(clientSocket);
		if ( urlBuffer==NULL ) {
			return NULL;
		}
		memset(m_UrlBuffer, 0, sizeof(m_UrlBuffer));
		strncpy(m_UrlBuffer, urlBuffer, sizeof(m_UrlBuffer));
		delete[] urlBuffer;
		urlBuffer = NULL;
		return m_UrlBuffer;
	} else {
		return NULL;
	}
}

int Live555RTSPServer::createServerMediaSession(ServerMediaSession* &sms, char const* streamName, char const* info,
	    char const* description, Boolean isSSM, char const* miscSDPLines, unsigned maxSize)
{
	if ( m_pUsageEnvironment==NULL ) {
		return -1;
	}
	sms = ServerMediaSession::createNew(*m_pUsageEnvironment, streamName, info, description, isSSM, miscSDPLines);
	OutPacketBuffer::maxSize = maxSize;
	if ( sms==NULL ) {
		return -2;
	}
	return 0;
}

int Live555RTSPServer::addSubsession(ServerMediaSession* sms, ServerMediaSubsession* subsession)
{
	if ( sms==NULL ) {
		return -1;
	} else if ( subsession==NULL ) {
		return -2;
	}
	sms->addSubsession(subsession);
	return 0;
}

int Live555RTSPServer::addServerMediaSession(ServerMediaSession* serverMediaSession)
{
	if ( serverMediaSession==NULL ) {
		return -1;
	} else if ( m_pRTSPServer==NULL ) {
		return -2;
	}
	m_pRTSPServer->addServerMediaSession(serverMediaSession);
	return 0;
}

int Live555RTSPServer::deleteServerMediaSession(ServerMediaSession* serverMediaSession)
{
	if ( serverMediaSession==NULL ) {
		return -1;
	} else if ( m_pRTSPServer==NULL ) {
		return -2;
	}
	m_pRTSPServer->deleteServerMediaSession(serverMediaSession);
	return 0;
}

int Live555RTSPServer::setUpTunnelingOverHTTP(unsigned short httpPort)
{
	if ( m_pRTSPServer==NULL ) {
		return -1;
	}
	Boolean iRet = m_pRTSPServer->setUpTunnelingOverHTTP(httpPort);
	if ( iRet==false ) {
		return -2;
	}
	return 0;
}

int Live555RTSPServer::httpServerPortNum()
{
	if ( m_pRTSPServer==NULL ) {
		return -1;
	}
	return m_pRTSPServer->httpServerPortNum();
}

int Live555RTSPServer::SingleStep()
{
	if (m_pBasicTaskScheduler0==NULL) {
		return -1;
	} else if ( m_iStartFlag ) {
		if ( m_iPauseFlag==0 ) {
			return -2;
		}
	}
	m_pBasicTaskScheduler0->SingleStep();
	return 0;
}


void *Live555RTSPServer::SingleStepThread(void* arg)
{
	Live555RTSPServer* p_objLive555RTSPServer = (Live555RTSPServer*)arg;
#if 1//defined(linux)
	prctl(PR_SET_NAME, p_objLive555RTSPServer->rtspURLPrefix());
#endif
	while( p_objLive555RTSPServer->m_iStartFlag ) {
		if ( p_objLive555RTSPServer->m_iPauseFlag ) {
			ithread_msleep(1);
			continue;
		}
		p_objLive555RTSPServer->m_pBasicTaskScheduler0->SingleStep();
	}
	return NULL;
}

int Live555RTSPServer::Start()
{
	m_iPauseFlag = 0;
	if ( m_Thread_handle ) {
		return -1;
	} else if ( m_iStartFlag ) {
		return -2;
	}
	m_Thread_handle = calloc(1, ithread_get_handle_size());
	m_iStartFlag = 1;
	ithread_create(m_Thread_handle, NULL, (void*)Live555RTSPServer::SingleStepThread, this);
	return 0;
}

int Live555RTSPServer::isRunning()
{
	if ( m_Thread_handle==NULL ) {
		return 0;
	}
	if ( m_iPauseFlag ) {
		return 0;
	}
	if ( m_iStartFlag ) {
		return 1;
	} else {
		return 0;
	}
}

int Live555RTSPServer::Pause()
{
	m_iPauseFlag = 1;
	if (m_Thread_handle==NULL) {
		return -1;
	}
	return 0;
}

int Live555RTSPServer::Join()
{
	m_iPauseFlag = 1;
	m_iStartFlag = 0;
	if ( m_Thread_handle==NULL ) {
		return -1;
	}
	int iRet = ithread_join(m_Thread_handle, NULL);
	free(m_Thread_handle);
	m_Thread_handle=NULL;
	return iRet;
}
