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

#ifndef __RTSP_H__
#define __RTSP_H__

#include <map>
#include <string>

#include "mi_common.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "list.h"
#ifdef __cplusplus
}
#endif

#include "tem.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"

#include "sys.h"

typedef struct
{
    struct list_head stDataList;
    void *pDataAddr;
    MI_U32 u32DataLen;
    MI_U32 u32FrmCnt;
    MI_U32 u32FrmRef;
    MI_BOOL bExit;
}stRtspDataPackage_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_condattr_t condattr;
}stRtspDataMutexCond_t;
typedef struct
{
    unsigned int totalCount;
    unsigned int uintCurFrmCnt;
    unsigned int uintRefCnt;
    struct list_head stDataList;    
}stRtspDataPackageHead_t;
typedef struct stRtspInputInfo_s
{
    int intUseBulPool;
    int intEncodeType;
    unsigned int uintPreModChnId;
    unsigned int uintVideoInPortId;
    unsigned int uintFrameRate;
    unsigned int uintVideoStreamRefCnt;

    int intHasPcmData;
    unsigned int uintAuidioInPortId;
    unsigned int uintAudioSampleRate;
    unsigned int uintAudioChannel;
    unsigned int uintAudioSoundMode;
    unsigned int uintAudioBitWidth;
    unsigned int uintAudioStreamRefCnt;

    Sys *pInstance;

    ServerMediaSession *mediaSession;
    ServerMediaSubsession *subSession;
    ServerMediaSubsession *audioSubSession;
}stRtspInputInfo_t;
typedef struct stRtspRefInfo_s
{
    stRtspInputInfo_t *pstInputInfo;
    unsigned int uintDequeueCnt;
    unsigned int refId;
}stRtspRefInfo_t;
typedef struct stRtspClientEvent_s
{
    unsigned char ucharCmd; //0: open url, 1: while loop
    std::string *pstrUrl;
}stRtspClientEvent_t;

class Rtsp: public Sys
{
    public:
        Rtsp();
        virtual ~Rtsp();
        void GetInfo(std::map<std::string, stRtspInputInfo_t> &info, unsigned char &isOpenOnvif, std::map<unsigned int, std::string> &outUrl)
        {
            info = mRtspInputInfo;
            isOpenOnvif = bOpenOnvif;
            outUrl = mapPortToUrl;
        }
        void UpdateInfo(std::map<std::string, stRtspInputInfo_t> &info, unsigned char &isOpenOnvif, std::map<unsigned int, std::string> &outUrl)
        {
            mRtspInputInfo = info;
            bOpenOnvif = isOpenOnvif;
            mapPortToUrl = outUrl;
        };
    private:
        virtual void Init();
        virtual void Deinit();
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        virtual void Start();
        virtual void Stop();
        virtual void LoadDb();
        //Delivery api
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        virtual int StartSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc);
        virtual int StopSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc);

        static void *ClientMonitor(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stUsrData);
        //RTSP client use.
        // The main streaming routine (for each "rtsp://" URL):
        static RTSPClient* openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);
        // Used to iterate through each stream's 'subsessions', setting up each one:
        static void setupNextSubsession(RTSPClient* rtspClient);

        // RTSP 'response handlers':
        static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
        static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
        static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
        // Other event handler functions:
        static void subsessionAfterPlaying(void* clientData);
        static void subsessionByeHandler(void* clientData, char const* reason);
        // called when a RTCP "BYE" is received for a subsession
        static void streamTimerHandler(void* clientData);
        // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

        static void streamByeBye(void* clientData);

        // Used to shut down and close a stream (including its "RTSPClient" object):
        static void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

        Live555RTSPServer *pRTSPServer;
        unsigned char bOpenOnvif;
        static void *OpenStream(char const *szStreamName, void *arg);
        static void *OpenAudioStream(char const *szStreamName, void *arg);
        static int CloseStream(void *handle, void *arg);
        static int CloseAudioStream(void *handle, void *arg);
        static int ReadStream(void *handle, unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp, void *arg);
        static int ReadAudioStream(void *handle, unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp, void *arg);
        static MI_S32 TermBufPool(void);
        static void BufPoolEmptyAndWait(void);
        static MI_S32 OpenBufPool(unsigned int inPort);
        static MI_S32 CloseBufPool(unsigned int inPort);
        static MI_S32 DequeueBufPool(unsigned int inPort, void *pData, MI_U32 u32Size, stRtspRefInfo_t *pRef);
        static MI_S32 FlushBufPool(unsigned int inPort, stRtspRefInfo_t *pRef);
        static MI_S32 GetDataDirect(int chnid, void *pData, MI_U32 u32Maxlen);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId);

        static std::map<std::string, stRtspInputInfo_t> mRtspInputInfo;
        static std::map<MI_U32, stRtspDataPackageHead_t> mapRtspDataPackage;
        static std::map<unsigned int, stRtspRefInfo_t> mapVideoStreamRefInfo;
        static std::map<unsigned int, stRtspRefInfo_t> mapAudioStreamRefInfo;
        static stRtspDataMutexCond_t stDataMuxCond;
        UsageEnvironment *env;
        TaskScheduler* scheduler;
        std::map<unsigned int, std::string> mapInportToStream;
        std::map<std::string, unsigned int> mapUrlToPort;
        std::map<unsigned int, std::string> mapPortToUrl;
        std::map<std::string, RTSPClient*> mapUrlToClient;
        friend class DummySink;
};
class TopUsageEnvironment : public BasicUsageEnvironment{
    public:
        static TopUsageEnvironment* createNew(Rtsp &rtspRef, TaskScheduler& taskScheduler);
    protected:
        TopUsageEnvironment(Rtsp &rtspRef, TaskScheduler& taskScheduler)
            : BasicUsageEnvironment(taskScheduler), rtspReferance(rtspRef){
        };
        virtual ~TopUsageEnvironment(){};
    public:
        Rtsp &rtspReferance;
};
class StreamClientState {
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
    static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
    ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~ourRTSPClient();

public:
    StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
    static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

private:
    DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t* fReceiveBuffer;
    MediaSubsession& fSubsession;
    char* fStreamId;
    Rtsp *rtstObj;
};

#endif

