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

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <map>
#include <string>

#include "mi_venc.h"
#include "mi_common.h"

#include "ai.h"
#include "venc.h"
#include "rtsp.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"

#include "onvif_server.h"

#define RTSP_LISTEN_PORT        554
#define BUF_POOL_MAX 120

std::map<std::string, stRtspInputInfo_t> Rtsp::mRtspInputInfo;
std::map<MI_U32, stRtspDataPackageHead_t> Rtsp::mapRtspDataPackage;
std::map<unsigned int, stRtspRefInfo_t> Rtsp::mapVideoStreamRefInfo;
std::map<unsigned int, stRtspRefInfo_t> Rtsp::mapAudioStreamRefInfo;
stRtspDataMutexCond_t Rtsp::stDataMuxCond;

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

static char eventLoopWatchVariable = 0;

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:


#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

TopUsageEnvironment* TopUsageEnvironment::createNew(Rtsp &rtspRef, TaskScheduler& taskScheduler) {
    return new TopUsageEnvironment(rtspRef, taskScheduler);
}

RTSPClient* Rtsp::openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
    if (rtspClient == NULL) {
        env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
        return NULL;
    }

    ++rtspClientCount;

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);

    return rtspClient;
}


// Implementation of the RTSP 'response handlers':

void Rtsp::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void Rtsp::setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	env << "client port " << scs.subsession->clientPortNum();
      } else {
	env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void Rtsp::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void Rtsp::continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
}


// Implementation of the other event handlers:

void Rtsp::subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void Rtsp::subsessionByeHandler(void* clientData, char const* reason) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\"";
  if (reason != NULL) {
    env << " (reason:\"" << reason << "\")";
    delete[] reason;
  }
  env << " on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}
void Rtsp::streamByeBye(void* clientData) {
    ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;    
    shutdownStream(rtspClient);
}

void Rtsp::streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void Rtsp::shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out,
    // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
    eventLoopWatchVariable = 1;
  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}

ourRTSPClient::~ourRTSPClient() {
}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 200000

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
  return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
    : MediaSink(env), fSubsession(subsession){
    fStreamId = strDup(streamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    TopUsageEnvironment *pEnv = (dynamic_cast<TopUsageEnvironment *>(&env));
    rtstObj = &(pEnv->rtspReferance);
}

DummySink::~DummySink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
    // We've just received a frame of data.  (Optionally) print out information about it:
#if 0//DEBUG_PRINT_EACH_RECEIVED_FRAME
    if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
    envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
    char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
    if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
        envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif

    stStreamInfo_t stStreamInfo;
    std::string tmpMediaStr;
    std::string tmpCodecStr;
    stModDesc_t stModeDesc;
    stEsPackage_t stEsPackage;
    unsigned int outPort;
    unsigned int *pHolder = NULL;

    rtstObj->GetModDesc(stModeDesc);
    outPort = rtstObj->mapUrlToPort[fStreamId];
    std::vector<stModIoInfo_t>::iterator vectNextIter;

    tmpMediaStr = fSubsession.mediumName();
    tmpCodecStr = fSubsession.codecName();
    //envir() << "Media type : " << tmpMediaStr.c_str() << " Codec : " << tmpCodecStr.c_str();
    if (tmpMediaStr == "audio"
        && tmpCodecStr == "L16")
    {
        stStreamInfo.eStreamType = E_STREAM_PCM;
        //envir() << " Bandwidth " << fSubsession.bandwidth() << " Channel : " << fSubsession.numChannels() << " Sample rate " << fSubsession.rtpTimestampFrequency();
        //envir() << tmpCodecStr.c_str() << " Buf size " << frameSize << " Buffer : "<< (void *)fReceiveBuffer <<"\n";
    }
    else if (tmpMediaStr == "video")
    {
        if (tmpCodecStr == "H264")
        {
            stStreamInfo.eStreamType = E_STREAM_H264;
        }
        else if (tmpCodecStr == "H265")
        {
            stStreamInfo.eStreamType = E_STREAM_H265;
        }
        stStreamInfo.streamWidth = fSubsession.videoWidth();
        stStreamInfo.streamHeight = fSubsession.videoHeight();
        //envir() << " Width : " << stStreamInfo.streamWidth << " Height : " << stStreamInfo.streamHeight << " FPS : " << fSubsession.videoFPS();
        //envir() << tmpCodecStr.c_str() << " Buf size " << frameSize << " Buffer : "<< (void *)fReceiveBuffer <<"\n";
    }
    pHolder = (unsigned int *)malloc(frameSize + 4);
    assert(pHolder);
    pHolder[0] = 0x01000000;
    memcpy(&pHolder[1], fReceiveBuffer, frameSize);
    stEsPackage.pData = (char *)pHolder;
    stEsPackage.uintDataSize = frameSize + 4;
    stStreamInfo.stCodecInfo.pDataAddr = &stEsPackage;
    stStreamInfo.stCodecInfo.uintPackCnt = 1;
    rtstObj->Send(outPort, &stStreamInfo, sizeof(stStreamInfo_t));
    free(pHolder);
    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}
void *Rtsp::ClientMonitor(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stUsrData)
{
    stRtspClientEvent_t *pstEvenv;
    Rtsp *pSendClass = dynamic_cast<Rtsp *>((Sys *)stBuf.pTemBuffer);

    ASSERT(pSendClass);
    if (sizeof(stRtspClientEvent_t) != stUsrData.u32UserDataSize)
    {
        printf("Not match!\n");

        return NULL;
    }
    pstEvenv = (stRtspClientEvent_t *)stUsrData.pUserData;
    if (pstEvenv->ucharCmd == 0)
    {
        pSendClass->mapUrlToClient[*pstEvenv->pstrUrl] = openURL(*pSendClass->env, "rtsp", pstEvenv->pstrUrl->c_str());
    }
    else if (pstEvenv->ucharCmd == 1)
    {
        // All subsequent activity takes place within the event loop:
        pSendClass->env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
        // This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.
        // If you choose to continue the application past this point (i.e., if you comment out the "return 0;" statement above),
        // and if you don't intend to do anything more with the "TaskScheduler" and "UsageEnvironment" objects,
        // then you can also reclaim the (small) memory used by these objects by uncommenting the following code:
        /*
          env->reclaim(); env = NULL;
          delete scheduler; scheduler = NULL;
        */

    }
    return NULL;
}

void Rtsp::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    stRtspDataPackage_t *pstRtspDataPackage = NULL;
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    stStreamInfo_t *pstStream = (stStreamInfo_t *)pData;
    MI_U32 u32Len = 0;

    ASSERT(pstStream);
    ASSERT(sizeof(stStreamInfo_t) == dataSize);
    pstRtspDataPackage = (stRtspDataPackage_t *)malloc(sizeof(stRtspDataPackage_t));
    ASSERT(pstRtspDataPackage);
    memset(pstRtspDataPackage, 0, sizeof(stRtspDataPackage_t));
    for (MI_U8 i = 0; i < pstStream->stCodecInfo.uintPackCnt; i++)
    {
        ASSERT(pstStream->stCodecInfo.pDataAddr[i].uintDataSize);
        pstRtspDataPackage->u32DataLen += pstStream->stCodecInfo.pDataAddr[i].uintDataSize;
    }
    pstRtspDataPackage->pDataAddr = (void *)malloc(pstRtspDataPackage->u32DataLen);
    ASSERT(pstRtspDataPackage->pDataAddr);
    for (MI_U8 i = 0; i < pstStream->stCodecInfo.uintPackCnt; i++)
    {
        memcpy((char *)pstRtspDataPackage->pDataAddr + u32Len, pstStream->stCodecInfo.pDataAddr[i].pData, pstStream->stCodecInfo.pDataAddr[i].uintDataSize);
        u32Len += pstStream->stCodecInfo.pDataAddr[i].uintDataSize;
    }
    pthread_mutex_lock(&stDataMuxCond.mutex);
    iter = mapRtspDataPackage.find(portId);
    if (iter == mapRtspDataPackage.end())
    {
        free(pstRtspDataPackage->pDataAddr);
        pstRtspDataPackage->pDataAddr =  NULL;
        free(pstRtspDataPackage);
        pthread_mutex_unlock(&stDataMuxCond.mutex);
        
        return;
    }
    if (!list_empty(&iter->second.stDataList))
    {
        stRtspDataPackage_t *pstLastPackage = NULL;

        pstLastPackage = list_last_entry(&iter->second.stDataList, stRtspDataPackage_t, stDataList);
        ASSERT(pstLastPackage);
        if (pstLastPackage->bExit)
        {
            free(pstRtspDataPackage->pDataAddr);
            pstRtspDataPackage->pDataAddr =  NULL;
            free(pstRtspDataPackage);
            pthread_mutex_unlock(&stDataMuxCond.mutex);

            return;
        }
    }
    //printf("in port id %d type %d addr %p size %d\n", portId, pstStream->eStreamType, stRtspDataPackage.pDataAddr, stRtspDataPackage.u32DataLen);
    if(iter == mapRtspDataPackage.end())
    {
        printf("Error!!!! input port %d not open!!!\n", portId);
        free(pstRtspDataPackage->pDataAddr);
        pstRtspDataPackage->pDataAddr = NULL;
        free(pstRtspDataPackage);
        pthread_mutex_unlock(&stDataMuxCond.mutex);

        return;
    }
    if (iter->second.totalCount > BUF_POOL_MAX)
    {
        stRtspDataPackage_t *pstFirstPackage = NULL;

        printf("Error!!!!Port %d buf pool full!!!!! Maybe client did not close or the network environment is bad.\n", portId);
        pstFirstPackage = list_first_entry(&iter->second.stDataList, stRtspDataPackage_t, stDataList);
        free(pstFirstPackage->pDataAddr);
        pstFirstPackage->pDataAddr = NULL;
        list_del(&pstFirstPackage->stDataList);
        free(pstFirstPackage);
        iter->second.totalCount--;
    }
    pstRtspDataPackage->u32FrmRef = iter->second.uintRefCnt;
    pstRtspDataPackage->u32FrmCnt = ++(iter->second.uintCurFrmCnt);
    list_add_tail(&pstRtspDataPackage->stDataList, &iter->second.stDataList);
    iter->second.totalCount++;
    PTH_RET_CHK(pthread_cond_signal(&stDataMuxCond.cond));
    pthread_mutex_unlock(&stDataMuxCond.mutex);

}
MI_S32 Rtsp::TermBufPool(void)
{
    stRtspDataPackage_t *pstRtspDataPackage;
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    std::map<std::string, stRtspInputInfo_t>::iterator itRtspInputInfo;

    for(itRtspInputInfo = mRtspInputInfo.begin(); itRtspInputInfo != mRtspInputInfo.end(); itRtspInputInfo++)
    {
        printf("Term input port video %d had pcm %d audio port %d\n", itRtspInputInfo->second.uintPreModChnId, itRtspInputInfo->second.intHasPcmData, itRtspInputInfo->second.uintAuidioInPortId);
        pthread_mutex_lock(&stDataMuxCond.mutex);
        iter = mapRtspDataPackage.find(itRtspInputInfo->second.uintPreModChnId);
        if(iter != mapRtspDataPackage.end())
        {
            pstRtspDataPackage = (stRtspDataPackage_t *)malloc(sizeof(stRtspDataPackage_t));
            ASSERT(pstRtspDataPackage);
            memset(pstRtspDataPackage, 0, sizeof(stRtspDataPackage_t));
            pstRtspDataPackage->bExit = TRUE;
            list_add_tail(&pstRtspDataPackage->stDataList, &iter->second.stDataList);
            PTH_RET_CHK(pthread_cond_signal(&stDataMuxCond.cond));
        }
        if (itRtspInputInfo->second.intHasPcmData)
        {
            iter = mapRtspDataPackage.find(itRtspInputInfo->second.uintAuidioInPortId);
            if(iter != mapRtspDataPackage.end())
            {
                pstRtspDataPackage = (stRtspDataPackage_t *)malloc(sizeof(stRtspDataPackage_t));
                ASSERT(pstRtspDataPackage);
                memset(pstRtspDataPackage, 0, sizeof(stRtspDataPackage_t));
                pstRtspDataPackage->bExit = TRUE;
                list_add_tail(&pstRtspDataPackage->stDataList, &iter->second.stDataList);
                PTH_RET_CHK(pthread_cond_signal(&stDataMuxCond.cond));
            }
        }
        pthread_mutex_unlock(&stDataMuxCond.mutex);
        printf("term end!\n");
    }

    return MI_SUCCESS;

    return 0;
}

void Rtsp::BufPoolEmptyAndWait(void)
{
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    std::map<std::string, stRtspInputInfo_t>::iterator itRtspInputInfo;

    pthread_mutex_lock(&stDataMuxCond.mutex);
    for(itRtspInputInfo = mRtspInputInfo.begin(); itRtspInputInfo != mRtspInputInfo.end(); itRtspInputInfo++)
    {
        iter = mapRtspDataPackage.find(itRtspInputInfo->second.uintPreModChnId);
        if(iter != mapRtspDataPackage.end())
        {
            if (!list_empty(&iter->second.stDataList))
            {
                pthread_mutex_unlock(&stDataMuxCond.mutex);
                return;
            }
        }
        if (itRtspInputInfo->second.intHasPcmData)
        {
            iter = mapRtspDataPackage.find(itRtspInputInfo->second.uintAuidioInPortId);
            if(iter != mapRtspDataPackage.end())
            {
                if (!list_empty(&iter->second.stDataList))
                {
                    pthread_mutex_unlock(&stDataMuxCond.mutex);
                    return;
                }
            }
        }
    }
    pthread_cond_wait(&stDataMuxCond.cond, &stDataMuxCond.mutex);
    pthread_mutex_unlock(&stDataMuxCond.mutex);

    return;
}
MI_S32 Rtsp::OpenBufPool(unsigned int inPort)
{
    pthread_mutex_lock(&stDataMuxCond.mutex);
    INIT_LIST_HEAD(&mapRtspDataPackage[inPort].stDataList);
    mapRtspDataPackage[inPort].totalCount = 0;
    mapRtspDataPackage[inPort].uintCurFrmCnt = 0;
    mapRtspDataPackage[inPort].uintRefCnt = 0;
    pthread_mutex_unlock(&stDataMuxCond.mutex);

    return MI_SUCCESS;

}
MI_S32 Rtsp::CloseBufPool(unsigned int inPort)
{
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;

    pthread_mutex_lock(&stDataMuxCond.mutex);
    iter = mapRtspDataPackage.find(inPort);
    if(iter != mapRtspDataPackage.end())
    {
        stRtspDataPackage_t *pos, *posN;
        list_for_each_entry_safe(pos, posN, &iter->second.stDataList, stDataList)
        {
            if (pos->bExit == FALSE)
            {
                free(pos->pDataAddr);
                iter->second.totalCount--;
            }
            list_del(&pos->stDataList);
            free(pos);
        }
        ASSERT(iter->second.totalCount == 0);
        mapRtspDataPackage.erase(iter);
    }
    pthread_mutex_unlock(&stDataMuxCond.mutex);

    return MI_SUCCESS;
}
MI_S32 Rtsp::DequeueBufPool(unsigned int inPort, void *pData, MI_U32 u32Size, stRtspRefInfo_t *pRef)
{
    MI_U32 sizeRet = 0;
    char * pCharAddr = NULL;
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    MI_BOOL bBeginToSend = FALSE;

    ASSERT(pRef);
    ASSERT(pData);
    BufPoolEmptyAndWait();
    pthread_mutex_lock(&stDataMuxCond.mutex);
    iter = mapRtspDataPackage.find(inPort);
    if(iter != mapRtspDataPackage.end())
    {
        pCharAddr = (char *)pData;
        stRtspDataPackage_t *pos, *posN;
        list_for_each_entry_safe(pos, posN, &iter->second.stDataList, stDataList)
        {
            if (!pos->bExit)
            {
                if (bBeginToSend == FALSE
                    && pRef->uintDequeueCnt < pos->u32FrmCnt)
                {
                    /*0xFFFFFFFF/(60fps*3600*24*365)= 2.xx. Two years is enough*/
                    bBeginToSend = TRUE;
                }
                if (bBeginToSend)
                {
                    if (u32Size >= sizeRet + pos->u32DataLen)
                    {
                        memcpy(pCharAddr, pos->pDataAddr, pos->u32DataLen);
                        pCharAddr += pos->u32DataLen;
                        sizeRet += pos->u32DataLen;
                    }
                    else
                    {
                        printf("Data max! ref %d size %d total count %d total ref %d frm ref %d\n", pRef->refId, sizeRet, iter->second.totalCount, iter->second.uintRefCnt, pos->u32FrmRef);
                        break;
                    }
                    //printf("size %d data_length %d Addr %p total %d ref %d, frm cnt %d refId %d\n", sizeRet, pos->u32DataLen, pos->pDataAddr, iter->second.totalCount, pos->u32FrmRef, pos->u32FrmCnt, pRef->refId);
                    ASSERT(pos->u32FrmRef);
                    pos->u32FrmRef--;
                    pRef->uintDequeueCnt = pos->u32FrmCnt;
                    if (pos->u32FrmRef == 0)
                    {
                        free(pos->pDataAddr);
                        iter->second.totalCount--;
                        list_del(&pos->stDataList);
                        free(pos);
                    }
                }
            }
            else
            {
                break;
            }
        }        
    }
    pthread_mutex_unlock(&stDataMuxCond.mutex);

    return sizeRet;

}
MI_S32 Rtsp::FlushBufPool(unsigned int inPort, stRtspRefInfo_t *pRef)
{
    std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    std::vector<stRtspDataPackage_t>::iterator it;
    MI_BOOL bBeginToSend = FALSE;

    ASSERT(pRef);
    iter = mapRtspDataPackage.find(inPort);
    if(iter != mapRtspDataPackage.end())
    {
        stRtspDataPackage_t *pos, *posN;

        pthread_mutex_lock(&stDataMuxCond.mutex);
        list_for_each_entry_safe(pos, posN, &iter->second.stDataList, stDataList)
        {
            if (!pos->bExit)
            {
                if (bBeginToSend == FALSE
                    && pRef->uintDequeueCnt < pos->u32FrmCnt)
                {
                    /*0xFFFFFFFF/(60fps*3600*24*365)= 2.xx. Two years is enough*/
                    bBeginToSend = TRUE;
                }
                if (bBeginToSend)
                {
                    ASSERT(pos->u32FrmRef);
                    pos->u32FrmRef--;
                    pRef->uintDequeueCnt = pos->u32FrmCnt;
                    if (pos->u32FrmRef == 0)
                    {
                        free(pos->pDataAddr);
                        iter->second.totalCount--;
                        list_del(&pos->stDataList);
                        free(pos);
                    }
                }
            }
        }
        iter->second.uintRefCnt--;
        pthread_mutex_unlock(&stDataMuxCond.mutex);
    }

    return MI_SUCCESS;
}
MI_S32 Rtsp::GetDataDirect(int chnId, void *pData, MI_U32 u32Maxlen)
{
    MI_U32 u32Size = 0;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack[16];
    MI_VENC_ChnStat_t stStat;
    MI_S32 s32Fd;
    MI_S32 s32Ret = MI_SUCCESS;
    struct timeval tv;
    fd_set read_fds;
    MI_U32 u32Len = 0;
    MI_U8 i = 0;

    s32Fd = MI_VENC_GetFd((MI_VENC_CHN)chnId);
    if (s32Fd < 0)
        return 0;

    FD_ZERO(&read_fds);
    FD_SET(s32Fd, &read_fds);

    tv.tv_sec = 0;
    tv.tv_usec = 10 * 1000;

    s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
    if (s32Ret < 0)
    {
        goto RET;
    }
    else if (0 == s32Ret)
    {
        goto RET;
    }
    else
    {
        memset(&stStream, 0, sizeof(stStream));
        memset(stPack, 0, sizeof(stPack));
        stStream.pstPack = stPack;
        s32Ret = MI_VENC_Query((MI_VENC_CHN)chnId, &stStat);
        if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
        {
            return -1;
        }
        stStream.u32PackCount = stStat.u32CurPacks;
        s32Ret = MI_VENC_GetStream((MI_VENC_CHN)chnId, &stStream, 40);
        if(MI_SUCCESS == s32Ret)
        {
            for (i = 0; i < stStream.u32PackCount; i++)
            {
                u32Size += stStream.pstPack[i].u32Len;
            }
            if (u32Size < u32Maxlen)
            {
                for (i = 0; i < stStream.u32PackCount; i++)
                {
                    memcpy((char *)pData + u32Len, stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
                    u32Len += stStream.pstPack[i].u32Len;
                }
                //printf("Get stream size %d addr %p\n", stStream.pstPack[0].u32Len, stRtspDataPackage.pDataAddr);
            }
            else
            {
                printf("Data too large! limitation %d, cur %d\n", u32Maxlen, u32Size);
            }
            MI_VENC_ReleaseStream((MI_VENC_CHN)chnId, &stStream);
        }
    }
 RET:
    MI_VENC_CloseFd(s32Fd);

    return u32Size;
}
void Rtsp::LoadDb()
{
    std::map<unsigned int, stModInputInfo_t>::iterator itMapRtspIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapRtspOut;

    for (itMapRtspIn = mapModInputInfo.begin(); itMapRtspIn != mapModInputInfo.end(); itMapRtspIn++)
    {
        stModDesc_t stPreDesc;
        stRtspInputInfo_t stRtspInputInfo;
        std::map<std::string, stRtspInputInfo_t>::iterator itMapRtspInfo;
        Sys *pObj = GetInstance(itMapRtspIn->second.stPrev.modKeyString);
        std::string strName;
        
        ASSERT(pObj);
        pObj->GetModDesc(stPreDesc);
        strName = GetIniString(itMapRtspIn->second.curIoKeyString, "STREAM_NAME");
        itMapRtspInfo = mRtspInputInfo.find(strName);
        if (itMapRtspInfo != mRtspInputInfo.end())
        {
            stRtspInputInfo = itMapRtspInfo->second;
        }
        else
        {
            /*init*/
            stRtspInputInfo.intHasPcmData = 0;
        }
        stRtspInputInfo.uintPreModChnId = stPreDesc.chnId;
        stRtspInputInfo.pInstance = this;
        switch (stPreDesc.modId)
        {
            case E_SYS_MOD_AI:
            {
                Ai *pAiObj = dynamic_cast<Ai *>(pObj);
                stAiInfo_t stAiInfo;
                ASSERT(pAiObj);
                stRtspInputInfo.intHasPcmData = 1;
                pAiObj->GetInfo(stAiInfo);
                stRtspInputInfo.uintAuidioInPortId = itMapRtspIn->second.curPortId;
                stRtspInputInfo.uintAudioSampleRate = stAiInfo.uintSampleRate;
                stRtspInputInfo.uintAudioChannel = stAiInfo.uintChannelCnt;
                stRtspInputInfo.uintAudioSoundMode = stAiInfo.uintSoundMode;
                stRtspInputInfo.uintAudioBitWidth = stAiInfo.uintBitWidth;
                stRtspInputInfo.uintAudioStreamRefCnt = 0;
                mRtspInputInfo[strName] = stRtspInputInfo;
            }
            break;
            case E_SYS_MOD_VENC:
            {
                Venc *pVencObj = dynamic_cast<Venc *>(pObj);
                stVencInfo_t stVencInfo;
                ASSERT(pVencObj);
                pVencObj->GetInfo(stVencInfo);
                stRtspInputInfo.uintVideoInPortId = itMapRtspIn->second.curPortId;
                stRtspInputInfo.intEncodeType = stVencInfo.intEncodeType;
                stRtspInputInfo.uintFrameRate = itMapRtspIn->second.curFrmRate;
                stRtspInputInfo.intUseBulPool = GetIniInt(itMapRtspIn->second.curIoKeyString, "IS_USE_BUF_POOL");
                stRtspInputInfo.uintVideoStreamRefCnt = 0;
                mRtspInputInfo[strName] = stRtspInputInfo;
            }
            break;
            default:
                ASSERT(0);
                break;
        }
        mapInportToStream[itMapRtspIn->second.curPortId] = strName;
    }
    bOpenOnvif = (unsigned char)GetIniUnsignedInt(stModDesc.modKeyString, "ONVIF");
    for (itMapRtspOut = mapModOutputInfo.begin(); itMapRtspOut != mapModOutputInfo.end(); itMapRtspOut++)
    {
        mapUrlToPort[GetIniString(itMapRtspOut->second.curIoKeyString, "URL")] = itMapRtspOut->second.curPortId;
        mapPortToUrl[itMapRtspOut->second.curPortId] = GetIniString(itMapRtspOut->second.curIoKeyString, "URL");
    }
}
Rtsp::Rtsp()
{
    pRTSPServer = NULL;
    bOpenOnvif = FALSE;
}
Rtsp::~Rtsp()
{
}

void Rtsp::Init()
{
    int iRet = 0;
    unsigned int rtspServerPortNum = RTSP_LISTEN_PORT;
    std::map<std::string, stRtspInputInfo_t>::iterator itMapRtspInfo;

    if (mapModInputInfo.size())
    {
        PTH_RET_CHK(pthread_mutex_init(&stDataMuxCond.mutex, NULL));
        PTH_RET_CHK(pthread_condattr_init(&stDataMuxCond.condattr));
        PTH_RET_CHK(pthread_condattr_setclock(&stDataMuxCond.condattr, CLOCK_MONOTONIC));
        PTH_RET_CHK(pthread_cond_init(&stDataMuxCond.cond, &stDataMuxCond.condattr));

        for (itMapRtspInfo = mRtspInputInfo.begin(); itMapRtspInfo != mRtspInputInfo.end(); ++itMapRtspInfo)
        {
            if (itMapRtspInfo->second.intUseBulPool)
            {
                OpenBufPool(itMapRtspInfo->second.uintVideoInPortId);
                CreateReceiver(itMapRtspInfo->second.uintVideoInPortId, DataReceiver, NULL, NULL, NULL);
            }
            if (itMapRtspInfo->second.intHasPcmData)
            {
                OpenBufPool(itMapRtspInfo->second.uintAuidioInPortId);
                CreateReceiver(itMapRtspInfo->second.uintAuidioInPortId, DataReceiver, NULL, NULL, NULL);
            }
        }
        pRTSPServer = new Live555RTSPServer();
        if(pRTSPServer == NULL)
        {
            assert(0);
        }
#if 0
        pRTSPServer->addUserRecord("admin", "888888");
#endif
        iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
        while(iRet < 0)
        {
            rtspServerPortNum++;

            if(rtspServerPortNum > 65535)
            {
                printf("Failed to create RTSP server: %s\n", pRTSPServer->getResultMsg());
                delete pRTSPServer;
                pRTSPServer = NULL;
                return;
            }
            iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
        }
    }
    if (mapModOutputInfo.size())
    {
        ST_TEM_ATTR stTemAttr;
        std::map<unsigned int, stModOutputInfo_t>::iterator itMapRtspOut;
        
        PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
        memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
        stTemAttr.fpThreadDoSignal = ClientMonitor;
        stTemAttr.fpThreadWaitTimeOut = NULL;
        stTemAttr.u32ThreadTimeoutMs = -1;
        stTemAttr.bSignalResetTimer = 0;
        stTemAttr.stTemBuf.pTemBuffer = (void *)this;
        stTemAttr.stTemBuf.u32TemBufferSize = 0;
        TemOpen(stModDesc.modKeyString.c_str(), stTemAttr);
        // Begin by setting up our usage environment:
        scheduler = BasicTaskScheduler::createNew();
        env = TopUsageEnvironment::createNew(*this, *scheduler);
    }
}

void* Rtsp::OpenStream(char const * szStreamName, void * arg)
{
    stRtspInputInfo_t *pInfo = NULL;
    Rtsp *pThisClass = NULL;
    stRtspRefInfo_t stRefInfo;
    unsigned int uintTempRefId = 0;

    pInfo = &mRtspInputInfo[szStreamName];
    pThisClass = dynamic_cast<Rtsp *>(pInfo->pInstance);
    ASSERT(pThisClass);
    while(mapVideoStreamRefInfo.find(uintTempRefId) != mapVideoStreamRefInfo.end())
    {
        uintTempRefId++;
    }
    memset(&stRefInfo, 0, sizeof(stRtspRefInfo_t));
    stRefInfo.pstInputInfo = pInfo;
    stRefInfo.refId = uintTempRefId;
    if (pInfo->intUseBulPool)
    {
        if (pInfo->uintVideoStreamRefCnt == 0)
        {
            std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;

            pthread_mutex_lock(&stDataMuxCond.mutex);
            iter = mapRtspDataPackage.find(pInfo->uintVideoInPortId);
            if (iter != mapRtspDataPackage.end())
            {
                stRefInfo.uintDequeueCnt = 0;
                iter->second.uintRefCnt = ++pInfo->uintVideoStreamRefCnt;
                mapVideoStreamRefInfo[stRefInfo.refId] = stRefInfo;
            }
            pthread_mutex_unlock(&stDataMuxCond.mutex);
            pThisClass->StartReceiver(pInfo->uintVideoInPortId);
        }
        else
        {
            std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;

            pthread_mutex_lock(&stDataMuxCond.mutex);
            iter = mapRtspDataPackage.find(pInfo->uintVideoInPortId);
            if (iter != mapRtspDataPackage.end())
            {
                stRefInfo.uintDequeueCnt = iter->second.uintCurFrmCnt;
                iter->second.uintRefCnt = ++pInfo->uintVideoStreamRefCnt;
                mapVideoStreamRefInfo[stRefInfo.refId] = stRefInfo;
            }
            pthread_mutex_unlock(&stDataMuxCond.mutex);
        }
    }
    if (pInfo->intEncodeType == E_MI_VENC_MODTYPE_H265E
        || pInfo->intEncodeType == E_MI_VENC_MODTYPE_H264E)
    {
        MI_VENC_RequestIdr((MI_VENC_CHN)pInfo->uintPreModChnId, FALSE);
    }
    printf("open stream \"%s\" success, in port:%d refcnt %d dequeue cnt %d addr %p\n", szStreamName, pInfo->uintVideoInPortId, stRefInfo.refId, stRefInfo.uintDequeueCnt, &mapVideoStreamRefInfo[stRefInfo.refId]);

    return &mapVideoStreamRefInfo[stRefInfo.refId];
}
void* Rtsp::OpenAudioStream(char const * szStreamName, void * arg)
{
    stRtspInputInfo_t *pInfo = NULL;
    Rtsp *pThisClass = NULL;
    stRtspRefInfo_t stRefInfo;
    unsigned int uintTempRefId = 0;

    pInfo = &mRtspInputInfo[szStreamName];
    pThisClass = dynamic_cast<Rtsp *>(pInfo->pInstance);
    ASSERT(pThisClass);
    while(mapAudioStreamRefInfo.find(uintTempRefId) != mapAudioStreamRefInfo.end())
    {
        uintTempRefId++;
    }
    memset(&stRefInfo, 0, sizeof(stRtspRefInfo_t));
    stRefInfo.pstInputInfo = pInfo;
    stRefInfo.refId = uintTempRefId;
    if (pInfo->uintAudioStreamRefCnt == 0)
    {
        std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;

        pthread_mutex_lock(&stDataMuxCond.mutex);
        iter = mapRtspDataPackage.find(pInfo->uintAuidioInPortId);
        if (iter != mapRtspDataPackage.end())
        {
            stRefInfo.uintDequeueCnt = 0;
            iter->second.uintRefCnt = ++pInfo->uintAudioStreamRefCnt;
             mapAudioStreamRefInfo[stRefInfo.refId] = stRefInfo;
        }
        pthread_mutex_unlock(&stDataMuxCond.mutex);
        pThisClass->StartReceiver(pInfo->uintAuidioInPortId);
    }    
    else
    {
        std::map<MI_U32, stRtspDataPackageHead_t>::iterator iter;
    
        pthread_mutex_lock(&stDataMuxCond.mutex);
        iter = mapRtspDataPackage.find(pInfo->uintAuidioInPortId);
        if (iter != mapRtspDataPackage.end())
        {
            stRefInfo.uintDequeueCnt = iter->second.uintCurFrmCnt;
            iter->second.uintRefCnt = ++pInfo->uintAudioStreamRefCnt;
            mapAudioStreamRefInfo[stRefInfo.refId] = stRefInfo;
        }
        pthread_mutex_unlock(&stDataMuxCond.mutex);
    }
    printf("open audio stream \"%s\" success, in port:%d refcnt %d dequeue cnt %d addr %p\n", szStreamName, pInfo->uintAuidioInPortId, stRefInfo.refId, stRefInfo.uintDequeueCnt, &mapAudioStreamRefInfo[stRefInfo.refId]);

    return &mapAudioStreamRefInfo[stRefInfo.refId];
}

int Rtsp::ReadStream(void *handle, unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp, void *arg)
{
    stRtspRefInfo_t *pRef = (stRtspRefInfo_t *)handle;

    if (pRef->pstInputInfo->intUseBulPool)
        return (int)DequeueBufPool(pRef->pstInputInfo->uintVideoInPortId, (void *)ucpBuf, BufLen, pRef);
    else
        return (int)GetDataDirect((MI_VENC_CHN)pRef->pstInputInfo->uintPreModChnId, (void *)ucpBuf, BufLen);

    return -1;
}
int Rtsp::ReadAudioStream(void *handle, unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp, void *arg)
{
    stRtspRefInfo_t *pRef = (stRtspRefInfo_t *)handle;

    return (int)DequeueBufPool(pRef->pstInputInfo->uintAuidioInPortId, (void *)ucpBuf, BufLen, pRef);
}

int Rtsp::CloseStream(void *handle, void *arg)
{
    stRtspRefInfo_t *pRef = (stRtspRefInfo_t *)handle;
    Rtsp *pThisClass = NULL;

    printf("Close video pRef %p refid %d port %d ref %d\n", pRef, pRef->refId, pRef->pstInputInfo->uintVideoInPortId, pRef->pstInputInfo->uintVideoStreamRefCnt);
    if (pRef->pstInputInfo->intUseBulPool && pRef->pstInputInfo->uintVideoStreamRefCnt == 1)
    {
        pThisClass = dynamic_cast<Rtsp *>(pRef->pstInputInfo->pInstance);
        ASSERT(pThisClass);
        pThisClass->StopReceiver(pRef->pstInputInfo->uintVideoInPortId);
    }
    FlushBufPool(pRef->pstInputInfo->uintVideoInPortId, pRef);
    if (pRef->pstInputInfo->uintVideoStreamRefCnt)
        pRef->pstInputInfo->uintVideoStreamRefCnt--;
    mapVideoStreamRefInfo.erase(pRef->refId);
    pRef = NULL;

    return 0;
}
int Rtsp::CloseAudioStream(void *handle, void *arg)
{
    stRtspRefInfo_t *pRef = (stRtspRefInfo_t *)handle;
    Rtsp *pThisClass = NULL;

    printf("Close audio pRef %p refid %d port %d ref %d\n", pRef, pRef->refId, pRef->pstInputInfo->uintAuidioInPortId, pRef->pstInputInfo->uintAudioStreamRefCnt);
    if (pRef->pstInputInfo->uintAudioStreamRefCnt == 1)
    {
        pThisClass = dynamic_cast<Rtsp *>(pRef->pstInputInfo->pInstance);
        ASSERT(pThisClass);
        pThisClass->StopReceiver(pRef->pstInputInfo->uintAuidioInPortId);
    }
    FlushBufPool(pRef->pstInputInfo->uintAuidioInPortId, pRef);
    if (pRef->pstInputInfo->uintAudioStreamRefCnt)
        pRef->pstInputInfo->uintAudioStreamRefCnt--;
    mapAudioStreamRefInfo.erase(pRef->refId);
    pRef = NULL;

    return 0;
}

void Rtsp::BindBlock(stModInputInfo_t & stIn)
{
    //printf("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    //printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

}

void Rtsp::Start()
{
    char *urlPrefix = NULL;
    unsigned int audioChannel = 0;

#if 0
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOutput;
    std::vector<stModIoInfo_t>::iterator itVectIo;

    printf("======Mod %s dump in!======\n", stModDesc.modKeyString.c_str());
    for (itMapOutput = mapModOutputInfo.begin(); itMapOutput != mapModOutputInfo.end(); ++itMapOutput)
    {
        printf("OutKey %s port %d frm %d\n", itMapOutput->second.curIoKeyString.c_str(), itMapOutput->second.curPortId, itMapOutput->second.curFrmRate);
        for (itVectIo = itMapOutput->second.vectNext.begin(); itVectIo != itMapOutput->second.vectNext.end(); ++itVectIo)
        {
            printf("Next mod %s port %d frm %d\n", itVectIo->modKeyString.c_str(), itVectIo->portId, itVectIo->frmRate);
        }
    }
    printf("======Mod %s dump out!=====\n", stModDesc.modKeyString.c_str());
#endif
    std::map<std::string, stRtspInputInfo_t>::iterator itMapRtspInfo;

    for (itMapRtspInfo = mRtspInputInfo.begin(); itMapRtspInfo != mRtspInputInfo.end(); ++itMapRtspInfo)
    {
        urlPrefix = pRTSPServer->rtspURLPrefix();
        printf("=================URL===================\n");
        printf("%s%s\n", urlPrefix, itMapRtspInfo->first.c_str());
        printf("=================URL===================\n");

        pRTSPServer->createServerMediaSession(itMapRtspInfo->second.mediaSession,
                                              itMapRtspInfo->first.c_str(),
                                              NULL, NULL);

        switch (itMapRtspInfo->second.intEncodeType)
        {
            case E_MI_VENC_MODTYPE_H264E:
            {
                itMapRtspInfo->second.subSession = WW_H264VideoFileServerMediaSubsession::createNew(
                                 *(pRTSPServer->GetUsageEnvironmentObj()),
                                 itMapRtspInfo->first.c_str(),
                                 OpenStream,
                                 ReadStream,
                                 CloseStream, itMapRtspInfo->second.uintFrameRate);
            }
            break;
            case E_MI_VENC_MODTYPE_H265E:
            {
                itMapRtspInfo->second.subSession = WW_H265VideoFileServerMediaSubsession::createNew(
                                 *(pRTSPServer->GetUsageEnvironmentObj()),
                                 itMapRtspInfo->first.c_str(),
                                 OpenStream,
                                 ReadStream,
                                 CloseStream, itMapRtspInfo->second.uintFrameRate);
            }
            break;
            case E_MI_VENC_MODTYPE_JPEGE:
            {
                itMapRtspInfo->second.subSession = WW_JPEGVideoFileServerMediaSubsession::createNew(
                                 *(pRTSPServer->GetUsageEnvironmentObj()),
                                 itMapRtspInfo->first.c_str(),
                                 OpenStream,
                                 ReadStream,
                                 CloseStream, itMapRtspInfo->second.uintFrameRate);
            }
            break;
            default:
                assert(0);
        }
        if (itMapRtspInfo->second.intHasPcmData)
        {
            switch (itMapRtspInfo->second.uintAudioSoundMode)
            {
                case 0: //mono
                {
                    audioChannel = itMapRtspInfo->second.uintAudioChannel;
                }
                break;
                case 1: //stereo
                {
                    audioChannel = 2;
                }
                break;
                default:
                    assert(0);
            }
            itMapRtspInfo->second.audioSubSession = WW_WAVAudioFileServerMediaSubsession::createNew(
                             *(pRTSPServer->GetUsageEnvironmentObj()), itMapRtspInfo->first.c_str(), WWA_PCM,
                             itMapRtspInfo->second.uintAudioSampleRate, audioChannel, (unsigned char)itMapRtspInfo->second.uintAudioBitWidth,
                             OpenAudioStream,
                             ReadAudioStream,
                             CloseAudioStream);
            pRTSPServer->addSubsession(itMapRtspInfo->second.mediaSession, itMapRtspInfo->second.audioSubSession);
        }
        pRTSPServer->addSubsession(itMapRtspInfo->second.mediaSession, itMapRtspInfo->second.subSession);
        pRTSPServer->addServerMediaSession(itMapRtspInfo->second.mediaSession);
    }
    if (mapModInputInfo.size())
    {
        pRTSPServer->Start();
        if (bOpenOnvif)
        {
            MST_ONVIF_Init();
            MST_ONVIF_StartTask();
        }
    }
}

int Rtsp::CreateSender(unsigned int outPortId)
{
    ST_TEM_USER_DATA stUserData;
    stRtspClientEvent_t stClientEvent;
    
    stClientEvent.pstrUrl = &mapPortToUrl[outPortId];
    stClientEvent.ucharCmd = 0;
    stUserData.pUserData = (void *)&stClientEvent;
    stUserData.u32UserDataSize = sizeof(stRtspClientEvent_t);
    TemSend(stModDesc.modKeyString.c_str(), stUserData);

    if (outPortId + 1 == mapModOutputInfo.size()) // last port
    {
        stClientEvent.pstrUrl = NULL;
        stClientEvent.ucharCmd = 1;
        stUserData.pUserData = (void *)&stClientEvent;
        stUserData.u32UserDataSize = sizeof(stRtspClientEvent_t);
        TemSend(stModDesc.modKeyString.c_str(), stUserData);

    }
    return 0;
}
int Rtsp::DestroySender(unsigned int outPortId)
{
    return 0;
}
int Rtsp::StartSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc)
{
    return 0;
}
int Rtsp::StopSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc)
{
    return 0;
}

void Rtsp::Stop()
{
    std::map<std::string, stRtspInputInfo_t>::iterator itMapRtspInfo;

    TermBufPool();
    if (mapModInputInfo.size())
    {
        if (bOpenOnvif)
        {
            MST_ONVIF_StopTask();
        }
        if(pRTSPServer)
        {
            pRTSPServer->Join();
            for (itMapRtspInfo = mRtspInputInfo.begin(); itMapRtspInfo != mRtspInputInfo.end(); ++itMapRtspInfo)
            {
                pRTSPServer->deleteServerMediaSession(itMapRtspInfo->second.mediaSession);
            }
        }
    }
}
void Rtsp::UnBindBlock(stModInputInfo_t &stIn)
{

    //printf("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    //printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
}

void Rtsp::Deinit()
{
    std::map<std::string, stRtspInputInfo_t>::iterator itMapRtspInfo;

    if (mapModOutputInfo.size())
    {
        std::map<std::string, RTSPClient*>::iterator itMapUrlToClient;
        for (itMapUrlToClient = mapUrlToClient.begin(); itMapUrlToClient != mapUrlToClient.end(); ++itMapUrlToClient)
        {
            env->taskScheduler().scheduleDelayedTask(1, (TaskFunc*)streamByeBye, itMapUrlToClient->second);
        }
        TemClose(stModDesc.modKeyString.c_str());
        env->reclaim();
        delete scheduler;
        scheduler = NULL;
    }
    if (mapModInputInfo.size())
    {
        if(pRTSPServer)
        {
            delete pRTSPServer;
            pRTSPServer = NULL;
        }
        for (itMapRtspInfo = mRtspInputInfo.begin(); itMapRtspInfo != mRtspInputInfo.end(); ++itMapRtspInfo)
        {
            if (itMapRtspInfo->second.intUseBulPool)
            {
                DestroyReceiver(itMapRtspInfo->second.uintVideoInPortId);
                CloseBufPool(itMapRtspInfo->second.uintVideoInPortId);
            }
            if (itMapRtspInfo->second.intHasPcmData)
            {
                DestroyReceiver(itMapRtspInfo->second.uintAuidioInPortId);
                CloseBufPool(itMapRtspInfo->second.uintAuidioInPortId);
            }
        }
    }
    PTH_RET_CHK(pthread_condattr_destroy(&stDataMuxCond.condattr));
    PTH_RET_CHK(pthread_cond_destroy(&stDataMuxCond.cond));
    PTH_RET_CHK(pthread_mutex_destroy(&stDataMuxCond.mutex));
    mRtspInputInfo.clear();
    mapVideoStreamRefInfo.clear();
    mapAudioStreamRefInfo.clear();
    mapRtspDataPackage.clear();
    mapUrlToPort.clear();
    mapPortToUrl.clear();
    mapUrlToClient.clear();
}

