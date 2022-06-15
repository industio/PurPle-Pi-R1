#ifndef _LIVE555_RTSP_SERVER_HH
#define _LIVE555_RTSP_SERVER_HH
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555DynamicRTSPServer.hh"
#include "WW_ByteStreamFileSource.hh"
#include "WW_H264VideoFileServerMediaSubsession.hh"
#include "WW_H265VideoFileServerMediaSubsession.hh"
#include "WW_JPEGVideoFileServerMediaSubsession.hh"
#include "WW_ADTSAudioFileServerMediaSubsession.hh"
#include "WW_WAVAudioFileServerMediaSubsession.hh"


class Live555RTSPServer {
public:
	Live555RTSPServer();
	~Live555RTSPServer();
public:
	UsageEnvironment* GetUsageEnvironmentObj();
	char  const* getResultMsg();
	char* rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket = -1);
	char* rtspURLPrefix(int clientSocket=-1);
	int   addUserRecord(char const* username, char const* password);
	int   removeUserRecord(char const* username);
	char  const* lookupPassword(char const* username);
	int   SetRTSPServerPort(unsigned short ourPort=554);
	int   SetDynamicRTSPServerPort(CREATENewSMS CreateNewSMSFun, unsigned short ourPort=554);
	int   CloseRTSPServer();
	int   createServerMediaSession(ServerMediaSession* &sms, char const* streamName=NULL
								, char const* info=NULL, char const* description=NULL
								, Boolean isSSM=False, char const* miscSDPLines=NULL
								, unsigned maxSize=1000000);
	int addSubsession(ServerMediaSession* sms, ServerMediaSubsession* subsession);
	int addServerMediaSession(ServerMediaSession* serverMediaSession);
	int deleteServerMediaSession(ServerMediaSession* serverMediaSession);
	int setUpTunnelingOverHTTP(unsigned short httpPort);
	int httpServerPortNum();
	int Start();
	int isRunning();
	int Pause();
	int Join();
	int SingleStep();
	static void *SingleStepThread(void* arg);
private:

protected:
	BasicTaskScheduler0*        m_pBasicTaskScheduler0;
	UsageEnvironment*           m_pUsageEnvironment;
	UserAuthenticationDatabase* m_pUserAuthenticationDatabase;
	RTSPServer*                 m_pRTSPServer;
	void* m_Thread_handle;
	int   m_iStartFlag;
	int   m_iPauseFlag;
	char  m_UrlBuffer[1024];
	char  m_ResultURL[1024];
};
#endif
