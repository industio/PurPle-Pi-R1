#ifndef _WW_CALLBACK_FUNCTION_HH
#define _WW_CALLBACK_FUNCTION_HH

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*OPENSTREAM)(char const* StreamName, void* arg);
typedef int   (*READSTREAM)(void* handle, unsigned char* ucpBuf, int BufLen, struct timeval *p_Timestamp, void* arg);
typedef int   (*CLOSESTREAM)(void* handle, void* arg);

#ifdef __cplusplus
}
#endif

#endif


