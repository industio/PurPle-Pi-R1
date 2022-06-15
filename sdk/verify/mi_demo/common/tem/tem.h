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
#ifndef _TEM_
#define _TEM_

#ifdef __cplusplus
extern "C" {
#endif
#if 1
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/prctl.h>

#include "mi_common.h"

//TEM: Thread event manager writed by Malloc.Peng from SigmaStar
// Start######
#define MUTEXCHECK(x) \
    do{   \
        if (x != 0)\
        {\
            fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        }\
    } while(0);
#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
#endif
#ifndef PTH_RET_CHK
// Just Temp Solution
#define PTH_RET_CHK(_pf_) \
    ({ \
        int r = _pf_; \
        if ((r != 0) && (r != ETIMEDOUT)) \
            printf("[PTHREAD] %s: %d: %s\n", __FILE__, __LINE__, #_pf_); \
        r; \
    })
#endif

typedef struct
{
	void *pUserData;
	MI_U32 u32UserDataSize;
}ST_TEM_USER_DATA;
typedef struct
{
	void *pTemBuffer;
	MI_U32 u32TemBufferSize;
}ST_TEM_BUFFER;

typedef void *(*FP_TEM_DOSIGNAL)(ST_TEM_BUFFER, ST_TEM_USER_DATA);
typedef void *(*FP_TEM_DOMONITOR)(ST_TEM_BUFFER);

typedef struct{
    pthread_attr_t thread_attr;
    MI_U32 u32ThreadTimeoutMs;
    FP_TEM_DOMONITOR fpThreadWaitTimeOut;
    FP_TEM_DOSIGNAL fpThreadDoSignal;
    ST_TEM_BUFFER stTemBuf;
    MI_BOOL bSignalResetTimer; // Reset timer after get signal.
}ST_TEM_ATTR;


MI_BOOL TemOpen(const char* pStr, ST_TEM_ATTR stAttr);
MI_BOOL TemClose(const char* pStr);
MI_BOOL TemStartMonitor(const char* pStr);
MI_BOOL TemStartOneShot(const char* pStr);
MI_BOOL TemConfigTimer(const char* pStr, MI_U32 u32TimeOut, MI_BOOL bSignalResetTimer); //if u32TimeOut is 0, use default setting from TemOpen
MI_BOOL TemStop(const char* pStr);
MI_BOOL TemSend(const char* pStr, ST_TEM_USER_DATA stUserData);
MI_BOOL TemSetBuffer(const char* pStr, void *pBufferData);
MI_BOOL TemGetBuffer(const char* pStr,  void *pBufferData);
MI_BOOL TemSetPartBufData(const char* pStr, void *pstBufHeadAddr, void *pstBufPartAddr, MI_U32 u32DataSize);
MI_BOOL TemGetPartBufData(const char* pStr, void *pstBufHeadAddr, void *pstBufPartAddr, MI_U32 u32DataSize);

#define TEM_GET_PART_BUFFER(name, type, member, value)	\
	do{		\
        type tmp;   \
        TemGetPartBufData(name, &tmp, &(tmp.member), sizeof(tmp.member));   \
	}while(0);
#define TEM_SET_PART_BUFFER(name, type, member, value)	\
    do{     \
        type tmp;   \
        TemSetPartBufData(name, &tmp, &(tmp.member), sizeof(tmp.member));   \
    }while(0);

//end###################################################
#endif

#ifdef __cplusplus
}
#endif

#endif //_TEM_
