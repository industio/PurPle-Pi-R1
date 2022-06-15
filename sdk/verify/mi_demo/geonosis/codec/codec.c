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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"

#include "es7210.h"
#include "es7243.h"
#include "es8156.h"

#define WAV_PCM  	(0x1)

#define AI_AMIC_CHN_MAX 	        (2)
#define AI_DMIC_CHN_MAX		        (4)
#define AI_I2S_NOR_CHN_MAX 		    (2)
#define AI_LINE_IN_CHN_MAX 	        (2)
#define AI_AMIC_AND_I2S_RX_CHN_MAX 	(4)
#define AI_DMIC_AND_I2S_RX_CHN_MAX 	(4)

#define AI_DEV_ID_MAX   (6)
#define AO_DEV_ID_MAX   (2)

#define AI_DEV_AMIC             (0)
#define AI_DEV_DMIC             (1)
#define AI_DEV_I2S_RX           (2)
#define AI_DEV_LINE_IN          (3)
#define AI_DEV_AMIC_AND_I2S_RX  (4)
#define AI_DEV_DMIC_AND_I2S_RX  (5)

#define AO_DEV_LINE_OUT         (0)
#define AO_DEV_I2S_TX           (1)

#define AI_VOLUME_AMIC_MIN      (0)
#define AI_VOLUME_AMIC_MAX      (21)
#define AI_VOLUME_LINEIN_MAX    (7)

#define AI_VOLUME_DMIC_MIN      (0)
#define AI_VOLUME_DMIC_MAX      (4)

#define AO_VOLUME_MIN           (-60)
#define AO_VOLUME_MAX           (30)

#define MI_AUDIO_SAMPLE_PER_FRAME	(1024)

#define DMA_BUF_SIZE_8K     (8000)
#define DMA_BUF_SIZE_16K    (16000)
#define DMA_BUF_SIZE_32K    (32000)
#define DMA_BUF_SIZE_48K    (48000)

#define AI_DMA_BUFFER_MAX_SIZE	(256 * 1024)
#define AI_DMA_BUFFER_MID_SIZE	(128 * 1024)
#define AI_DMA_BUFFER_MIN_SIZE	(64 * 1024)

#define AO_DMA_BUFFER_MAX_SIZE	(256 * 1024)
#define AO_DMA_BUFFER_MID_SIZE	(128 * 1024)
#define AO_DMA_BUFFER_MIN_SIZE	(64 * 1024)

#define MIU_WORD_BYTE_SIZE	(8)
#define TOTAL_BUF_DEPTH		(8)

// case id
#define ES7243_2AMIC        (0)
#define ES7210_2AMIC        (1)
#define ES7210_4AMIC        (2)
#define ES7210_3AMIC        (3)

#define ES8156_STEREO       (0)

void* normalAiGetFrame(void* data);
void* es7210_4ChnGetFrame(void* data);
void* aoSendFrame(void* data);

typedef enum
{
	E_AI_SOUND_MODE_MONO = 0,
	E_AI_SOUND_MODE_STEREO,
	E_AI_SOUND_MODE_QUEUE,
} AiSoundMode_e;

#define ExecFunc(func, _ret_) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
        return 1;\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);	\
}while(0);

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
typedef int codec_func(void);
typedef void* thread_func(void*);

typedef enum
{
    E_SOUND_MODE_MONO = 0, /* mono */
    E_SOUND_MODE_STEREO = 1, /* stereo */
} SoundMode_e;

typedef enum
{
    E_SAMPLE_RATE_8000 = 8000, /* 8kHz sampling rate */
    E_SAMPLE_RATE_16000 = 16000, /* 16kHz sampling rate */
    E_SAMPLE_RATE_32000 = 32000, /* 32kHz sampling rate */
    E_SAMPLE_RATE_48000 = 48000, /* 48kHz sampling rate */
} SampleRate_e;

typedef enum
{
    E_AENC_TYPE_G711A = 0,
    E_AENC_TYPE_G711U,
    E_AENC_TYPE_G726_16,
    E_AENC_TYPE_G726_24,
    E_AENC_TYPE_G726_32,
    E_AENC_TYPE_G726_40,
    PCM,
} AencType_e;

typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char chRIFF[4];
    unsigned int  dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    unsigned int  dwFMTLen;
    WaveFormat_t wave;
    char chDATA[4];
    unsigned int  dwDATALen;
} WaveFileHeader_t;

typedef struct AiOutFilenName_s
{
    MI_S8 *ps8OutputPath;
    MI_S8 *ps8OutputFile;
    MI_AUDIO_DEV AiDevId;
    MI_AUDIO_Attr_t stAiAttr;
} AiOutFileName_t;

typedef struct AiChnPriv_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_S32 s32Fd;
    MI_U32 u32TotalSize;
    MI_U32 u32ChnCnt;
    pthread_t tid;
} AiChnPriv_t;

typedef struct TestCaseInfo_s
{
    MI_U32          u32CaseId;
    codec_func      *codec_init;
    codec_func      *codec_deinit;
    thread_func     *frame_deal;
} TestCaseInfo_t;

static TestCaseInfo_t aiTestCase[] = {
    // ES7243
    {ES7243_2AMIC , ES7243_Init, ES7243_Deinit, normalAiGetFrame},
    // ES7210 2Amic
    {ES7210_2AMIC, ES7210_2AmicInit, ES7210_2AmicDeInit, normalAiGetFrame},
    // ES7210 4Amic
    {ES7210_4AMIC, ES7210_4AmicInit, ES7210_4AmicDeInit, es7210_4ChnGetFrame},
    // ES7210 3Amic
    {ES7210_3AMIC, ES7210_3AmicInit, ES7210_2AmicDeInit, normalAiGetFrame},
/*
    // ES7210 2Dmic
    {},
*/
};

static TestCaseInfo_t aoTestCase[] = {
    // ES8156
    {ES8156_STEREO, ES8156_Init, ES8156_Deinit, aoSendFrame},
};

static MI_BOOL  bEnableAI = FALSE;
static MI_U8*   pu8AiOutputPath = NULL;
static SoundMode_e eAiWavSoundMode = E_SOUND_MODE_MONO;
static AencType_e eAiWavAencType = PCM;
static MI_U32   u32AiChnCnt = 0;
static MI_S32   s32AiVolume = 0;
static MI_BOOL  bAiSetVolume = FALSE;
static MI_AUDIO_DEV AiDevId = -1;
static MI_AUDIO_SampleRate_e eAiWavSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_AUDIO_SampleRate_e eAiSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static AiSoundMode_e eAiSoundMode = E_AI_SOUND_MODE_MONO;
static MI_BOOL 	bAiI2sNormalMode = TRUE;

static MI_BOOL  bEnableAO = FALSE;
static MI_U8*   pu8AoInputPath = NULL;
static MI_S32   s32AoVolume = 0;
static MI_BOOL  bAoSetVolume = FALSE;
static MI_AUDIO_DEV AoDevId = -1;

static MI_U32   u32RunTime = 0;
static MI_BOOL  bAiExit = FALSE;
static MI_BOOL  bAoExit = FALSE;

static MI_S32   AiChnFd[MI_AUDIO_MAX_CHN_NUM] = {[0 ... MI_AUDIO_MAX_CHN_NUM-1] = -1};

static AiChnPriv_t stAiChnPriv[MI_AUDIO_MAX_CHN_NUM];

static MI_S32   AoReadFd = -1;
static WaveFileHeader_t stWavHeaderInput;
static MI_AO_CHN AoChn = 0;
static MI_S32 s32NeedSize = 0;
static pthread_t Aotid;

static MI_U8 u8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];

static MI_U32 u32AiDevHeapSize = 0;
static MI_U32 u32AiChnOutputHeapSize = 0;
static MI_U32 u32AoDevHeapSize = 0;

static MI_BOOL bAllocAiDevPool = FALSE;
static MI_BOOL bAllocAiChnOutputPool = FALSE;
static MI_BOOL bAllocAoDevPool = FALSE;

static MI_U32 u32AiCodecTestCase = -1, u32AoCodecTestCase = -1;

void display_help(void)
{
    printf("----- audio all test -----\n");
    printf("-t : AI/AO run time(s)\n");
	printf("AI Device Id: Amic[0] Dmic[1] I2S RX[2] Linein[3] Amic+I2S RX[4] Dmic+I2S RX[5]\n");
    printf("AI test option :\n");
    printf("-I : Enable AI\n");
    printf("-o : AI output Path\n");
    printf("-d : AI Device Id\n");
    printf("-m : AI Sound Mode: Mono[0] Stereo[1] Queue[2]\n");
    printf("-c : AI channel count\n");
    printf("-s : AI Sample Rate\n");
    printf("-v : AI Volume\n");
    printf("-w : AI codec test: ES7243_2Amic[0] ES7210_2AmicInit[1] ES7210_4AmicInit[2] ES7210_3AmicInit[3]\n");
    printf("\n\n");

    printf("AO Device Id: Lineout[0] I2S TX[1]\n");
    printf("AO test option :\n");
    printf("-O : Enable AO\n");
    printf("-i : AO Input Path\n");
    printf("-D : AO Device Id\n");
    printf("-V : AO Volume\n");
    printf("-W : AO codec test: ES8156[0]\n");
    return;
}


void signalHandler(int signo)
{
    switch (signo){
        case SIGALRM:
        case SIGINT:
            printf("Catch signal!!!\n");
            bAiExit = TRUE;
            bAoExit = TRUE;
            break;
   }
   return;
}

void setTimer(MI_U32 u32RunTime)
{
    struct itimerval new_value, old_value;
    if (0 != u32RunTime)
    {
        new_value.it_value.tv_sec = u32RunTime;
        new_value.it_value.tv_usec = 0;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &new_value, &old_value);
    }
    return;
}

MI_BOOL checkParam(void)
{
    MI_BOOL bCheckPass = FALSE;
    do{
            if ((FALSE == bEnableAI) && (FALSE == bEnableAO))
            {
                printf("Neither AI nor AO enabled!!!\n");
                break;
            }

            if (bEnableAI)
            {
                if (NULL == pu8AiOutputPath)
                {
                    printf("Ai output path invalid!!!\n");
                    break;
                }

                if (AiDevId < 0 || AiDevId > AI_DEV_ID_MAX)
                {
                    printf("Ai device id invalid!!!\n");
                    break;
                }

			    if ((eAiSoundMode != E_AI_SOUND_MODE_MONO)
				    && (eAiSoundMode != E_AI_SOUND_MODE_STEREO)
				    && (eAiSoundMode != E_AI_SOUND_MODE_QUEUE))
			    {
				    printf("Ai sound mode invalid!!!\n");
				    break;
			    }

                if (u32AiChnCnt <= 0)
                {
                    printf("Ai channel count invalid!!!\n");
                    break;
                }

                if (AI_DEV_DMIC == AiDevId)
                {
            	    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
            		    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
            	    {
                	    if ((u32AiChnCnt > AI_DMIC_CHN_MAX)
                		    || (3 == u32AiChnCnt)
                	    )
                	    {
                    	    printf("Ai channel count invalid!!!\n");
                    	    break;
                	    }
                    }
                    else
                    {
					    if (u32AiChnCnt > AI_DMIC_CHN_MAX / 2)
                	    {
                    	    printf("Ai channel count invalid!!!\n");
                    	    break;
                	    }
                    }
                }
                else if ((AI_DEV_AMIC == AiDevId)
            			|| (AI_DEV_LINE_IN == AiDevId))
                {
            	    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
            		    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
            	    {
	                    if (u32AiChnCnt > AI_AMIC_CHN_MAX)
	                    {
	                        printf("Ai channel count invalid!!!\n");
	                        break;
	                    }
                    }
                    else
                    {
					    if (u32AiChnCnt > AI_AMIC_CHN_MAX / 2)
	                    {
	                        printf("Ai channel count invalid!!!\n");
	                        break;
	                    }
                    }
                }
                else if (AI_DEV_I2S_RX == AiDevId)
                {
				    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
					    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
				    {
					    if ((AI_I2S_NOR_CHN_MAX != u32AiChnCnt))
					    {
						    printf("Ai channel count invalid!!!\n");
	                        break;
					    }
				    }
				    else
				    {
					    if ((AI_I2S_NOR_CHN_MAX / 2 != u32AiChnCnt))
					    {
						    printf("Ai channel count invalid!!!\n");
	                        break;
					    }
				    }
                }
                else if (AI_DEV_AMIC_AND_I2S_RX == AiDevId)
                {
                    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
					    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
			        {
                        if (AI_AMIC_AND_I2S_RX_CHN_MAX != u32AiChnCnt)
                        {
                            printf("Ai channel count invalid!!!\n");
	                        break;
                        }
			        }
			        else if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
			        {
                        if (AI_AMIC_AND_I2S_RX_CHN_MAX / 2 != u32AiChnCnt)
                        {
                            printf("Ai channel count invalid!!!\n");
	                        break;
                        }
			        }
                }
                else if (AI_DEV_DMIC_AND_I2S_RX == AiDevId)
                {
                    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
					    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
			        {
                        if (AI_DMIC_AND_I2S_RX_CHN_MAX != u32AiChnCnt)
                        {
                            printf("Ai channel count invalid!!!\n");
	                        break;
                        }
			        }
			        else if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
			        {
                        if (AI_DMIC_AND_I2S_RX_CHN_MAX / 2 != u32AiChnCnt)
                        {
                            printf("Ai channel count invalid!!!\n");
	                        break;
                        }
			        }
                }

                if (
                        (E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAiSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiSampleRate)
                )
                {
                    printf("AI sample rate invalid!!!\n");
                    break;
                }

                if (TRUE == bAiSetVolume)
                {
                    if (AI_DEV_AMIC == AiDevId || AI_DEV_AMIC_AND_I2S_RX == AiDevId)
                    {
                        if ((s32AiVolume < AI_VOLUME_AMIC_MIN) || (s32AiVolume > AI_VOLUME_AMIC_MAX))
                        {
                            printf("Ai volume invalid!!!\n");
                            break;
                        }
                    }
                    else if (AI_DEV_DMIC == AiDevId || AI_DEV_DMIC_AND_I2S_RX == AiDevId)
                    {
                        if ((s32AiVolume < AI_VOLUME_DMIC_MIN) || (s32AiVolume > AI_VOLUME_DMIC_MAX))
                        {
                            printf("Ai volume invalid!!!\n");
                            break;
                        }   
                    }
                    else if (AI_DEV_LINE_IN == AiDevId)
                    {
                        if ((s32AiVolume < AI_VOLUME_AMIC_MIN) || (s32AiVolume > AI_VOLUME_LINEIN_MAX))
                        {
                            printf("Ai volume invalid!!!\n");
                            break;
                        }
                    }
				    else if (AI_DEV_I2S_RX == AiDevId)
                    {
                        printf("I2S RX is not supported volume setting!!!\n");
                        break;
                    }
			    }

                if (AI_DEV_I2S_RX == AiDevId || AI_DEV_AMIC_AND_I2S_RX == AiDevId || AI_DEV_DMIC_AND_I2S_RX == AiDevId)
                {
                    if ((u32AiCodecTestCase < 0) || (u32AiCodecTestCase >= (sizeof(aiTestCase) / sizeof(aiTestCase[0]))))
                    {
                        printf("Ai codec test case id invalid!!!\n");
                        break;
                    }
                }
            }

            if (bEnableAO)
            {
                if (NULL == pu8AoInputPath)
                {
                    printf("AO input path invalid!!!\n");
                    break;
                }

                if ((AoDevId < 0) || (AoDevId > AO_DEV_ID_MAX))
                {
                    printf("Ao device id invalid!!!\n");
                    break;
                }

                if (TRUE == bAoSetVolume)
                {
                    if ((s32AoVolume < AO_VOLUME_MIN) || (s32AoVolume > AO_VOLUME_MAX))
                    {
                        printf("AO Volume invalid!!!\n");
                        break;
                    }
                }

                if ( NULL == strstr((const char*)pu8AoInputPath, ".wav") )
                {
                    printf("Only support wav file.\n");
                    break;
                }

                if (AO_DEV_I2S_TX == AoDevId)
                {
                    if (u32AoCodecTestCase < 0 || u32AoCodecTestCase >= (sizeof(aoTestCase) / sizeof(aoTestCase[0])))
                    {
                        printf("Ao codec test case id invalid!!!\n");
                        break;
                    }
                }
            }
            bCheckPass = TRUE;
    }while(0);
    return bCheckPass;
}

void initParam(void)
{
    eAiWavSampleRate = eAiSampleRate;
    eAiWavAencType = PCM;

	if (AI_DEV_I2S_RX == AiDevId)
	{
		if (
			(((E_AI_SOUND_MODE_MONO == eAiSoundMode) || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode)) && (AI_I2S_NOR_CHN_MAX == u32AiChnCnt))
			|| ((E_AI_SOUND_MODE_STEREO == eAiSoundMode) && (AI_I2S_NOR_CHN_MAX / 2 == u32AiChnCnt)))
		{
			bAiI2sNormalMode = TRUE;
		}
		else
		{
			bAiI2sNormalMode = FALSE;
		}
	}

    signal(SIGALRM, signalHandler);
    signal(SIGINT, signalHandler);

    setTimer(u32RunTime);
    return;
}

void printParam(void)
{
    if (bEnableAI)
    {
        printf("Ai Param:\n");

        // output path
        printf("AI OutPut Path:%s\n", pu8AiOutputPath);

        // device
        printf("Device:");
        if (AiDevId == AI_DEV_AMIC)
        {
            printf("Amic");
        }
        else if (AiDevId == AI_DEV_DMIC)
        {
            printf("Dmic");
        }
        else if (AiDevId == AI_DEV_I2S_RX)
        {
            printf("I2S_RX");
        }
        else if (AiDevId == AI_DEV_LINE_IN)
        {
            printf("Linein");
        }
        else if (AiDevId == AI_DEV_AMIC_AND_I2S_RX)
        {
            printf("Amic+I2S RX");
        }
        else if (AiDevId == AI_DEV_DMIC_AND_I2S_RX)
        {
            printf("Dmic+I2S RX");
        }

        printf("\n");

        // chn cnt
        printf("ChnNum:%d\n", u32AiChnCnt);

        // mode
        printf("Mode:");
        if (E_AI_SOUND_MODE_QUEUE == eAiSoundMode)
        {
            printf("Queue");
        }
        else if (E_AI_SOUND_MODE_MONO == eAiSoundMode)
        {
            printf("Mono");
        }
        else if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
        {
			printf("Stereo");
        }
        printf("\n");
    }

    if (bEnableAO)
    {
        printf("Ao Param:\n");

        // input path
        printf("AO InPut Path:%s\n", pu8AoInputPath);

        // device
        printf("Device:");
        if (AoDevId == AO_DEV_LINE_OUT)
        {
            printf("LineOut");
        }
        else if (AoDevId == AO_DEV_I2S_TX)
        {
            printf("I2S_TX");
        }
        printf("\n");
    }
}

int addWaveHeader(WaveFileHeader_t* tWavHead, AencType_e eAencType, SoundMode_e eSoundMode, SampleRate_e eSampleRate, int raw_len)
{
    tWavHead->chRIFF[0] = 'R';
    tWavHead->chRIFF[1] = 'I';
    tWavHead->chRIFF[2] = 'F';
    tWavHead->chRIFF[3] = 'F';

    tWavHead->chWAVE[0] = 'W';
    tWavHead->chWAVE[1] = 'A';
    tWavHead->chWAVE[2] = 'V';
    tWavHead->chWAVE[3] = 'E';

    tWavHead->chFMT[0] = 'f';
    tWavHead->chFMT[1] = 'm';
    tWavHead->chFMT[2] = 't';
    tWavHead->chFMT[3] = 0x20;
    tWavHead->dwFMTLen = 0x10;

    if(eAencType == E_AENC_TYPE_G711A)
    {
        tWavHead->wave.wFormatTag = 0x06;
    }

    if(eAencType == E_AENC_TYPE_G711U)
    {
        tWavHead->wave.wFormatTag = 0x07;
    }

    if(eAencType == E_AENC_TYPE_G711U || eAencType == E_AENC_TYPE_G711A)
    {
        if(eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wBitsPerSample = 8;//bitWidth;g711encode出来是8bit，这里需要写死
        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec = (tWavHead->wave.wBitsPerSample  * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
        tWavHead->wave.wBlockAlign = (tWavHead->wave.wBitsPerSample  * tWavHead->wave.wChannels) / 8;
    }
    else if(eAencType == PCM)
    {
        if(eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wFormatTag = 0x1;
        tWavHead->wave.wBitsPerSample = 16; //16bit
        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec = (tWavHead->wave.wBitsPerSample  * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
        tWavHead->wave.wBlockAlign = 1024;
    }
    else //g726
    {
		if(eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wFormatTag = 0x45;
        switch(eAencType)
        {
            case E_AENC_TYPE_G726_40:
                tWavHead->wave.wBitsPerSample = 5;
                tWavHead->wave.wBlockAlign =  5;
                break;
            case E_AENC_TYPE_G726_32:
                tWavHead->wave.wBitsPerSample = 4;
                tWavHead->wave.wBlockAlign =  4;
                break;
            case E_AENC_TYPE_G726_24:
                tWavHead->wave.wBitsPerSample = 3;
                tWavHead->wave.wBlockAlign =  3;
                break;
            case E_AENC_TYPE_G726_16:
                tWavHead->wave.wBitsPerSample = 2;
                tWavHead->wave.wBlockAlign =  2;
                break;
            default:
                printf("eAencType error:%d\n", eAencType);
                return -1;
        }

        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec = (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
    }

    tWavHead->chDATA[0] = 'd';
    tWavHead->chDATA[1] = 'a';
    tWavHead->chDATA[2] = 't';
    tWavHead->chDATA[3] = 'a';
    tWavHead->dwDATALen = raw_len;
    tWavHead->dwRIFFLen = raw_len + sizeof(WaveFileHeader_t) - 8;

    return 0;
}

MI_S32 createOutputFileName(AiOutFileName_t *pstAiOutFileName, MI_U32 u32ChnIdx)
{
    char as8Tmp[512] = {0};

    //set OutpuFile prefix
    memset(as8Tmp, 0, sizeof(as8Tmp));
    if ('/' == pstAiOutFileName->ps8OutputPath[strlen((char *)pstAiOutFileName->ps8OutputPath) - 1])
    {
        sprintf(as8Tmp, "%s", pstAiOutFileName->ps8OutputPath);
    }
    else
    {
        sprintf(as8Tmp, "%s/", pstAiOutFileName->ps8OutputPath);
    }
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    //set Chn num
    memset(as8Tmp, 0, sizeof(as8Tmp));
    sprintf(as8Tmp, "Chn%d_", u32ChnIdx);
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    //set OutpuFile Device ID
    memset(as8Tmp, 0, sizeof(as8Tmp));
    if (AI_DEV_AMIC == pstAiOutFileName->AiDevId)
    {
        sprintf(as8Tmp, "Amic_");
    }
    else if (AI_DEV_DMIC == pstAiOutFileName->AiDevId)
    {
        sprintf(as8Tmp, "Dmic_");
    }
    else if (AI_DEV_I2S_RX == pstAiOutFileName->AiDevId)
    {
        sprintf(as8Tmp, "I2SRx_" );
    }
    else if (AI_DEV_LINE_IN == pstAiOutFileName->AiDevId)
    {
        sprintf(as8Tmp, "LineIn_" );
    }
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    //set OutpuFile SampleRate
    memset(as8Tmp, 0, sizeof(as8Tmp));
    sprintf(as8Tmp, "%dK", pstAiOutFileName->stAiAttr.eSamplerate / 1000);
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    //set OutpuFile BitWidth
    memset(as8Tmp, 0, sizeof(as8Tmp));
    sprintf(as8Tmp, "_%dbit", 16);
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    //set OutpuFile Channel Mode
    memset(as8Tmp, 0, sizeof(as8Tmp));
    if ((E_MI_AUDIO_SOUND_MODE_MONO == pstAiOutFileName->stAiAttr.eSoundmode)
            || (ES7210_4AMIC == u32AiCodecTestCase))
    {
        sprintf(as8Tmp, "_MONO");
    }
    else if (E_MI_AUDIO_SOUND_MODE_STEREO == pstAiOutFileName->stAiAttr.eSoundmode)
    {
        sprintf(as8Tmp, "_STEREO");
    }
    else if (E_MI_AUDIO_SOUND_MODE_QUEUE == pstAiOutFileName->stAiAttr.eSoundmode)
    {
        sprintf(as8Tmp, "_QUEUE");
    }
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);

    memset(as8Tmp, 0, sizeof(as8Tmp));
    sprintf(as8Tmp, ".wav");
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    return 0;
}

void* es7210_4ChnGetFrame(void* data)
{
    AiChnPriv_t* priv = (AiChnPriv_t*)data;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;
    MI_S32 s32Ret;
    WaveFileHeader_t stWavHead;

    MI_S16 *ps16TmpBuf = NULL;
    MI_U32 u32BufSize = 0, u32SampleCount = 0, u32SampleIndex = 0,
           u32Count0 = 0, u32Count1 = 0, u32Count2 = 0, u32Count3 = 0,
           u32CountOthers = 0,
           u32Chn0TotalSize = 0, u32Chn1TotalSize = 0, u32Chn2TotalSize = 0, u32Chn3TotalSize = 0;
    MI_BOOL bFoundDataHeader = FALSE;
    MI_S16 *ps16Chn0Buf = NULL, *ps16Chn1Buf = NULL, *ps16Chn2Buf = NULL, *ps16Chn3Buf = NULL;
    MI_U32 u32TmpChnBufSize = 1024 * 4;

    ps16Chn0Buf = (MI_S16*)malloc(u32TmpChnBufSize);
    ps16Chn1Buf = (MI_S16*)malloc(u32TmpChnBufSize);
    ps16Chn2Buf = (MI_S16*)malloc(u32TmpChnBufSize);
    ps16Chn3Buf = (MI_S16*)malloc(u32TmpChnBufSize);
    if ((NULL == ps16Chn0Buf) || (NULL == ps16Chn1Buf) || (NULL == ps16Chn2Buf) || (NULL == ps16Chn3Buf))
    {
        printf("failed to alloc memory.\n");
        free(ps16Chn0Buf);
        free(ps16Chn1Buf);
        free(ps16Chn2Buf);
        free(ps16Chn3Buf);
    }

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));

    if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
    {
        while(FALSE == bAiExit)
        {
            s32Ret = MI_AI_GetFrame(priv->AiDevId, priv->AiChn, &stAiChFrame, &stAecFrame, -1);
            if (MI_SUCCESS == s32Ret)
            {
                ps16TmpBuf = stAiChFrame.apVirAddr[0];
                u32BufSize = stAiChFrame.u32Len;
                u32SampleCount = u32BufSize / sizeof(MI_S16);

                if (FALSE == bFoundDataHeader)
                {
                    for (u32SampleIndex = 0; u32SampleIndex < u32SampleCount; u32SampleIndex++)
                    {
                        if ((u32SampleIndex < u32SampleCount) && (u32SampleIndex + 1 < u32SampleCount))
                        {
                            if ((1 == (ps16TmpBuf[u32SampleIndex] & 0x0001)) && (1 == (ps16TmpBuf[u32SampleIndex + 1] & 0x0001)))
                            {
                                printf("Found frist data header.\n");
                                bFoundDataHeader= TRUE;
                                ps16TmpBuf += u32SampleIndex;
                                u32BufSize -= u32SampleIndex * sizeof(MI_S16);
                                u32SampleCount -= u32SampleIndex;
                                break;
                            }
                        }
                    }

                    if (FALSE == bFoundDataHeader)
                    {
                        printf("[%s:%d] failed to found the data header.\n", __func__, __LINE__);
                        MI_AI_ReleaseFrame(priv->AiDevId, priv->AiChn,  &stAiChFrame, &stAecFrame);
                        continue;
                    }
                }

                memset(ps16Chn0Buf, 0x0, u32TmpChnBufSize);
                memset(ps16Chn1Buf, 0x0, u32TmpChnBufSize);
                memset(ps16Chn2Buf, 0x0, u32TmpChnBufSize);
                memset(ps16Chn3Buf, 0x0, u32TmpChnBufSize);

                for (u32SampleIndex = 0; u32SampleIndex < u32SampleCount; u32SampleIndex++)
                {
                    if (u32SampleIndex + 1 < u32SampleCount)
                    {
                        if ((1 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (1 == (ps16TmpBuf[u32SampleIndex + 1] & 0x0001)))
                        {
                            ps16Chn0Buf[u32Count0++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((1 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (0 == (ps16TmpBuf[u32SampleIndex + 1] & 0x0001)))
                        {
                            ps16Chn1Buf[u32Count1++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((0 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (0 == (ps16TmpBuf[u32SampleIndex + 1] & 0x0001)))
                        {
                            ps16Chn2Buf[u32Count2++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((0 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (1 == (ps16TmpBuf[u32SampleIndex + 1] & 0x0001)))
                        {
                            ps16Chn3Buf[u32Count3++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else
                        {
                            u32CountOthers++;
                        }
                    }
                    else
                    {
                        if ((1 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (0 == (ps16TmpBuf[u32SampleIndex - 1] & 0x0001)))
                        {
                            ps16Chn0Buf[u32Count0++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((1 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (1 == (ps16TmpBuf[u32SampleIndex - 1] & 0x0001)))
                        {
                            ps16Chn1Buf[u32Count1++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((0 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (1 == (ps16TmpBuf[u32SampleIndex - 1] & 0x0001)))
                        {
                            ps16Chn2Buf[u32Count2++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else if ((0 == (ps16TmpBuf[u32SampleIndex] & 0X0001)) && (0 == (ps16TmpBuf[u32SampleIndex - 1] & 0x0001)))
                        {
                            ps16Chn3Buf[u32Count3++] = ps16TmpBuf[u32SampleIndex];
                        }
                        else
                        {
                            u32CountOthers++;
                        }
                    }
                }

	            write(AiChnFd[0], ps16Chn0Buf, u32Count0 * sizeof(MI_S16));
                u32Chn0TotalSize += u32Count0 * sizeof(MI_S16);
	            write(AiChnFd[1], ps16Chn1Buf, u32Count1 * sizeof(MI_S16));
                u32Chn1TotalSize += u32Count1 * sizeof(MI_S16);
	            write(AiChnFd[2], ps16Chn2Buf, u32Count2 * sizeof(MI_S16));
                u32Chn2TotalSize += u32Count2 * sizeof(MI_S16);
	            write(AiChnFd[3], ps16Chn3Buf, u32Count3 * sizeof(MI_S16));
                u32Chn3TotalSize += u32Count3 * sizeof(MI_S16);

                //printf("%d %d %d %d %d.\n", u32Count0, u32Count1, u32Count2, u32Count3, u32CountOthers);
                u32Count0 = 0;
                u32Count1 = 0;
                u32Count2 = 0;
                u32Count3 = 0;
                u32CountOthers = 0;

                MI_AI_ReleaseFrame(priv->AiDevId, priv->AiChn,  &stAiChFrame,  NULL);
            }
            else
            {
                printf("Dev%dChn%d get frame failed!!!error:0x%x\n", priv->AiDevId, priv->AiChn, s32Ret);
            }
        }

        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, E_SOUND_MODE_MONO, 16000, u32Chn0TotalSize);
        lseek(AiChnFd[0], 0, SEEK_SET);
        write(AiChnFd[0], &stWavHead, sizeof(WaveFileHeader_t));
        close(AiChnFd[0]);

        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, E_SOUND_MODE_MONO, 16000, u32Chn1TotalSize);
        lseek(AiChnFd[1], 0, SEEK_SET);
        write(AiChnFd[1], &stWavHead, sizeof(WaveFileHeader_t));
        close(AiChnFd[1]);

        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, E_SOUND_MODE_MONO, 16000, u32Chn2TotalSize);
        lseek(AiChnFd[2], 0, SEEK_SET);
        write(AiChnFd[2], &stWavHead, sizeof(WaveFileHeader_t));
        close(AiChnFd[2]);

        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, E_SOUND_MODE_MONO, 16000, u32Chn3TotalSize);
        lseek(AiChnFd[3], 0, SEEK_SET);
        write(AiChnFd[3], &stWavHead, sizeof(WaveFileHeader_t));
        close(AiChnFd[3]);
    }
    free(ps16Chn0Buf);
    free(ps16Chn1Buf);
    free(ps16Chn2Buf);
    free(ps16Chn3Buf);
    return NULL;
}

void* normalAiGetFrame(void* data)
{
    AiChnPriv_t* priv = (AiChnPriv_t*)data;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;
    MI_S32 s32Ret;
    MI_U32 u32ChnIndex;
    struct timeval tv_before, tv_after;
    MI_S64 before_us, after_us;
    WaveFileHeader_t stWavHead;

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));

    if ((E_AI_SOUND_MODE_STEREO == eAiSoundMode) || (E_AI_SOUND_MODE_MONO == eAiSoundMode))
    {
        while(FALSE == bAiExit)
        {
            s32Ret = MI_AI_GetFrame(priv->AiDevId, priv->AiChn, &stAiChFrame, &stAecFrame, -1);
            if (MI_SUCCESS == s32Ret)
            {
                {
	                gettimeofday(&tv_before, NULL);
	                write(priv->s32Fd, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len);
	                gettimeofday(&tv_after, NULL);
	                before_us = tv_before.tv_sec * 1000000 + tv_before.tv_usec;
	                after_us = tv_after.tv_sec * 1000000 + tv_after.tv_usec;
	                if (after_us - before_us > 10 * 1000)
	                {
	                    printf("Chn:%d, cost time:%lldus = %lldms.\n", priv->AiChn, after_us - before_us, (after_us - before_us) / 1000);
	                }
	                priv->u32TotalSize += stAiChFrame.u32Len;

                }
                MI_AI_ReleaseFrame(priv->AiDevId, priv->AiChn,  &stAiChFrame,  NULL);
            }
            else
            {
                printf("Dev%dChn%d get frame failed!!!error:0x%x\n", priv->AiDevId, priv->AiChn, s32Ret);
            }
        }
        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, eAiWavSoundMode, eAiWavSampleRate, priv->u32TotalSize);
        lseek(priv->s32Fd, 0, SEEK_SET);
        write(priv->s32Fd, &stWavHead, sizeof(WaveFileHeader_t));
        close(priv->s32Fd);

    }
    else
    {
        while(FALSE == bAiExit)
        {
            s32Ret = MI_AI_GetFrame(priv->AiDevId, priv->AiChn, &stAiChFrame, &stAecFrame, -1);
            if(s32Ret == MI_SUCCESS)
            {
                for (u32ChnIndex = 0; u32ChnIndex < priv->u32ChnCnt; u32ChnIndex++)
                {
                    write(AiChnFd[u32ChnIndex], stAiChFrame.apVirAddr[u32ChnIndex], stAiChFrame.u32Len / priv->u32ChnCnt);
                }
                priv->u32TotalSize += stAiChFrame.u32Len / priv->u32ChnCnt;

                MI_AI_ReleaseFrame(priv->AiDevId, priv->AiChn, &stAiChFrame,  NULL);
            }
        }

        memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
        addWaveHeader(&stWavHead, eAiWavAencType, eAiWavSoundMode, eAiWavSampleRate, priv->u32TotalSize);
        for (u32ChnIndex = 0; u32ChnIndex < priv->u32ChnCnt; u32ChnIndex++)
        {
            lseek(AiChnFd[u32ChnIndex], 0, SEEK_SET);
            write(AiChnFd[u32ChnIndex], &stWavHead, sizeof(WaveFileHeader_t));
            close(AiChnFd[u32ChnIndex]);
        }
    }

    return NULL;
}

void* aoSendFrame(void* data)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;

    while(FALSE == bAoExit)
    {
        s32Ret = read(AoReadFd, &u8TempBuf, s32NeedSize);
        if(s32Ret != s32NeedSize)
        {
            lseek(AoReadFd, sizeof(WaveFileHeader_t), SEEK_SET);
            s32Ret = read(AoReadFd, &u8TempBuf, s32NeedSize);
            if (s32Ret < 0)
            {
                printf("Input file does not has enough data!!!\n");
                break;
            }
        }

        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        stAoSendFrame.u32Len = s32Ret;
        stAoSendFrame.apVirAddr[0] = u8TempBuf;
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if(s32Ret != MI_SUCCESS)
        {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }
    }
    close(AoReadFd);
    return NULL;
}


MI_S32 allocPrivatePool(void)
{
	MI_SYS_GlobalPrivPoolConfig_t stGlobalPrivPoolConf;
	MI_U32 u32HeapSize = 0;
	MI_U32 u32ChnCnt = 0;
	MI_U32 u32PtNum = 0;
	MI_U32 u32BufSize;
	MI_U32 u32BitWidth = 2;
	MI_U32 u32AlignSize = 4 * 1024;
	MI_U32 u32ChnIdx;

	if (bEnableAI)
	{
		u32ChnCnt = u32AiChnCnt;
		if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
		{
			u32ChnCnt *= 2;
		}

		u32HeapSize = u32BitWidth * u32ChnCnt * 2 * eAiSampleRate;

		u32HeapSize += (MIU_WORD_BYTE_SIZE - (u32HeapSize % MIU_WORD_BYTE_SIZE));

        if (u32HeapSize >= AI_DMA_BUFFER_MAX_SIZE)
        {
			u32HeapSize = AI_DMA_BUFFER_MAX_SIZE;
        }
        else if (u32HeapSize >= AI_DMA_BUFFER_MID_SIZE)
        {
			u32HeapSize = AI_DMA_BUFFER_MID_SIZE;
        }
		else
        {
			u32HeapSize = AI_DMA_BUFFER_MIN_SIZE;
        }

		u32AiDevHeapSize = u32HeapSize;

		memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
		stGlobalPrivPoolConf.bCreate = TRUE;
		stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AI;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AiDevId;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32HeapSize;
		ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);

		bAllocAiDevPool = TRUE;

		u32PtNum = eAiSampleRate / 16;
		u32BufSize = u32PtNum * u32BitWidth;	// for one date channel

		if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
		{
			u32BufSize *= 2;
		}

		if (AI_DEV_I2S_RX != AiDevId)
		{
			u32BufSize *= 2;
		}

		// 4K alignment
		if (0 != (u32BufSize % u32AlignSize))
		{
			u32BufSize = ((u32BufSize / u32AlignSize) + 1) * u32AlignSize;
		}

		u32HeapSize = u32BufSize * TOTAL_BUF_DEPTH;
		u32AiChnOutputHeapSize = u32HeapSize;

		for (u32ChnIdx = 0; u32ChnIdx < u32AiChnCnt; u32ChnIdx++)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = TRUE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.eModule = E_MI_MODULE_ID_AI;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Channel = u32ChnIdx;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Devid = AiDevId;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Port = 0;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32PrivateHeapSize = u32HeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
		}
		bAllocAiChnOutputPool = TRUE;
	}

	if (bEnableAO)
	{
		// I can not get sampling rate of ao device, so use max sample rate to alloc heap
		u32HeapSize = u32BitWidth * E_MI_AUDIO_SAMPLE_RATE_48000 / 2;
		u32HeapSize += (MIU_WORD_BYTE_SIZE - (u32HeapSize % MIU_WORD_BYTE_SIZE));
		if (u32HeapSize >= AO_DMA_BUFFER_MAX_SIZE)
		{
			u32HeapSize = AO_DMA_BUFFER_MAX_SIZE;
		}
		else if (u32HeapSize >= AO_DMA_BUFFER_MID_SIZE)
		{
			u32HeapSize = AO_DMA_BUFFER_MID_SIZE;
		}
		else
		{
			u32HeapSize = AO_DMA_BUFFER_MIN_SIZE;
		}

		// 2 buf, hw buffer + copy buffer
		u32HeapSize *= 2;
		u32AoDevHeapSize = u32HeapSize;
		memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
		stGlobalPrivPoolConf.bCreate = TRUE;
		stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AO;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AoDevId;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32HeapSize;
		ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
		bAllocAoDevPool = TRUE;
	}
	return MI_SUCCESS;
}

MI_S32 freePrivatePool(void)
{
	MI_SYS_GlobalPrivPoolConfig_t stGlobalPrivPoolConf;
	MI_U32 u32ChnIdx;

	if (bEnableAI)
	{
		if (TRUE == bAllocAiDevPool)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = FALSE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AI;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AiDevId;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32AiDevHeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);

			bAllocAiDevPool = FALSE;
		}

		if (TRUE == bAllocAiChnOutputPool)
		{
			for (u32ChnIdx = 0; u32ChnIdx < u32AiChnCnt; u32ChnIdx++)
			{
				memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
				stGlobalPrivPoolConf.bCreate = FALSE;
				stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.eModule = E_MI_MODULE_ID_AI;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Channel = u32ChnIdx;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Devid = AiDevId;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Port = 0;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32PrivateHeapSize = u32AiChnOutputHeapSize;
				ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
			}
			bAllocAiChnOutputPool = FALSE;
		}
	}

	if (bEnableAO)
	{
		if (TRUE == bAllocAoDevPool)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = FALSE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AO;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AoDevId;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32AoDevHeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
			bAllocAoDevPool = FALSE;
		}
	}
	return MI_SUCCESS;
}

void freePrivatePoolExit(void)
{
	MI_S32 s32Ret = MI_SUCCESS;
	s32Ret = freePrivatePool();
	if (MI_SUCCESS != s32Ret)
	{
		printf("Failed to free private pool!!!\n");
	}
	return;
}


MI_S32 initAi(void)
{
    MI_AUDIO_Attr_t     stAiSetAttr;
    MI_AUDIO_Attr_t     stAiGetAttr;
    AiOutFileName_t     stAiFileName;
    MI_U32              u32ChnIdx;
    MI_U32              u32ChnCnt;
    MI_S8               s8OutputFileName[512];
    MI_S32              s32Ret;
    WaveFileHeader_t    stAiWavHead;
    MI_SYS_ChnPort_t    stAiChnOutputPort0[MI_AUDIO_MAX_CHN_NUM];
    //MI_AI_ChnParam_t    stAiChnParam;
    MI_U32              u32FileCount;

    memset(&stAiSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    memset(&stAiGetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    stAiSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiSetAttr.eSamplerate = eAiSampleRate;
    stAiSetAttr.eSoundmode = eAiSoundMode;
    stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAiSetAttr.u32ChnCnt = u32AiChnCnt;
    stAiSetAttr.u32CodecChnCnt = 0; // useless
    stAiSetAttr.u32FrmNum = 6;  // useless
    stAiSetAttr.u32PtNumPerFrm = stAiSetAttr.eSamplerate / 16; // for aec
    stAiSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE; // useless
    stAiSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;

    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
		|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
	{
		eAiWavSoundMode = E_SOUND_MODE_MONO;
	}
	else
	{
		eAiWavSoundMode = E_SOUND_MODE_STEREO;
	}

    memset(&stAiFileName, 0x0, sizeof(AiOutFileName_t));
    stAiFileName.AiDevId = AiDevId;
    stAiFileName.ps8OutputFile = (MI_S8*)s8OutputFileName;
    stAiFileName.ps8OutputPath = (MI_S8*)pu8AiOutputPath;
    memcpy(&stAiFileName.stAiAttr, &stAiSetAttr, sizeof(MI_AUDIO_Attr_t));
    memset(&stAiWavHead, 0x0, sizeof(WaveFileHeader_t));

    // change channel file count
    // ES7210 4Amic
    if (u32AiCodecTestCase == ES7210_4AMIC)
    {
        u32FileCount = 4;
    }
    else
    {
        u32FileCount = u32AiChnCnt;
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32FileCount; u32ChnIdx++)
    {
        memset(s8OutputFileName, 0x0, sizeof(s8OutputFileName));
        createOutputFileName(&stAiFileName, u32ChnIdx);
        printf("OutputName:%s\n", s8OutputFileName);
        AiChnFd[u32ChnIdx] = open((const char *)s8OutputFileName, O_RDWR | O_CREAT, 0777);
        if(AiChnFd[u32ChnIdx] < 0)
        {
            printf("Open output file path:%s fail \n", s8OutputFileName);
            printf("error:%s", strerror(errno));
            return -1;
        }

        s32Ret = write(AiChnFd[u32ChnIdx], &stAiWavHead, sizeof(WaveFileHeader_t));
        if (s32Ret < 0)
        {
            printf("write wav head failed\n");
            return -1;
        }
    }

    ExecFunc(MI_AI_SetPubAttr(AiDevId, &stAiSetAttr), MI_SUCCESS);
    ExecFunc(MI_AI_GetPubAttr(AiDevId, &stAiGetAttr), MI_SUCCESS);
    ExecFunc(MI_AI_Enable(AiDevId), MI_SUCCESS);

    if (bAiSetVolume)
    {
        u32ChnCnt = u32AiChnCnt;
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            if ((AI_DEV_AMIC_AND_I2S_RX == AiDevId)
                || ((AI_DEV_DMIC_AND_I2S_RX == AiDevId)))
            {
                if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
                    || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
                {
                    if ((2 == u32ChnIdx) || (3 == u32ChnIdx))
                    {
                        continue;
                    }
                }
                else if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
                {
                    if (1 == u32ChnIdx)
                    {
                        continue;
                    }
                }
            }
            ExecFunc(MI_AI_SetVqeVolume(AiDevId, u32ChnIdx, s32AiVolume), MI_SUCCESS);
        }
    }

#if 0
    // for test new API
    memset(&stAiChnParam, 0x0, sizeof(MI_AI_ChnParam_t));
    stAiChnParam.stChnGain.bEnableGainSet = TRUE;
    stAiChnParam.stChnGain.s16FrontGain = 0;
    stAiChnParam.stChnGain.s16RearGain = 15;
    ExecFunc(MI_AI_SetChnParam(AiDevId, 0, &stAiChnParam), MI_SUCCESS);
    memset(&stAiChnParam, 0x0, sizeof(MI_AI_ChnParam_t));
    ExecFunc(MI_AI_GetChnParam(AiDevId, 0, &stAiChnParam), MI_SUCCESS);
    printf("Dev%dChn%d bEnableGainSetting:%d Front Gain:%d, Rear Gain:%d.\n",
            AoDevId, 0, stAiChnParam.stChnGain.bEnableGainSet, stAiChnParam.stChnGain.s16FrontGain,
            stAiChnParam.stChnGain.s16RearGain);
    memset(&stAiChnParam, 0x0, sizeof(MI_AI_ChnParam_t));
    stAiChnParam.stChnGain.bEnableGainSet = TRUE;
    stAiChnParam.stChnGain.s16FrontGain = 2;
    stAiChnParam.stChnGain.s16RearGain = 30;
    ExecFunc(MI_AI_SetChnParam(AiDevId, 1, &stAiChnParam), MI_SUCCESS);
    memset(&stAiChnParam, 0x0, sizeof(MI_AI_ChnParam_t));
    ExecFunc(MI_AI_GetChnParam(AiDevId, 1, &stAiChnParam), MI_SUCCESS);
    printf("Dev%dChn%d bEnableGainSetting:%d Front Gain:%d, Rear Gain:%d.\n",
            AoDevId, 1, stAiChnParam.stChnGain.bEnableGainSet, stAiChnParam.stChnGain.s16FrontGain,
            stAiChnParam.stChnGain.s16RearGain);
#endif
    memset(&stAiChnPriv, 0x0, sizeof(stAiChnPriv));
    memset(&stAiChnOutputPort0, 0x0, sizeof(stAiChnOutputPort0));

    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        stAiChnPriv[u32ChnIdx].AiChn = u32ChnIdx;
        stAiChnPriv[u32ChnIdx].AiDevId = AiDevId;
        stAiChnPriv[u32ChnIdx].s32Fd = AiChnFd[u32ChnIdx];
        stAiChnPriv[u32ChnIdx].u32ChnCnt = u32AiChnCnt;
        stAiChnPriv[u32ChnIdx].u32TotalSize = 0;

        stAiChnOutputPort0[u32ChnIdx].eModId = E_MI_MODULE_ID_AI;
        stAiChnOutputPort0[u32ChnIdx].u32DevId = AiDevId;
        stAiChnOutputPort0[u32ChnIdx].u32ChnId = u32ChnIdx;
        stAiChnOutputPort0[u32ChnIdx].u32PortId = 0;
        ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiChnOutputPort0[u32ChnIdx], 1, TOTAL_BUF_DEPTH), MI_SUCCESS);
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        ExecFunc(MI_AI_EnableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
    }

    if (AiDevId == AI_DEV_I2S_RX || AiDevId == AI_DEV_AMIC_AND_I2S_RX || AiDevId == AI_DEV_DMIC_AND_I2S_RX)
    {
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            pthread_create(&stAiChnPriv[u32ChnIdx].tid, NULL, aiTestCase[u32AiCodecTestCase].frame_deal, &stAiChnPriv[u32ChnIdx]);
        }
        aiTestCase[u32AiCodecTestCase].codec_init();
    }
    else
    {
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            pthread_create(&stAiChnPriv[u32ChnIdx].tid, NULL, normalAiGetFrame, &stAiChnPriv[u32ChnIdx]);
        }
    }
    printf("create ai thread.\n");
    return 0;
}

MI_S32 initAo(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Attr_t stAoSetAttr, stAoGetAttr;
    MI_S32 s32AoGetVolume;
    //MI_AO_ChnParam_t stAoChnParam;
    MI_U32 u32DmaBufSize;

    AoReadFd = open((const char *)pu8AoInputPath, O_RDONLY, 0666);
    if(AoReadFd <= 0)
    {
        printf("Open input file failed:%s \n", pu8AoInputPath);
        printf("error:%s", strerror(errno));
        return -1;
    }

    s32Ret = read(AoReadFd, &stWavHeaderInput, sizeof(WaveFileHeader_t));
    if (s32Ret < 0)
    {
        printf("Read wav header failed!!!\n");
        return -1;
    }

    memset(&stAoSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAoSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    stAoSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stAoSetAttr.u32ChnCnt = stWavHeaderInput.wave.wChannels;

    if(stAoSetAttr.u32ChnCnt == 2)
    {
        stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    }
    else if(stAoSetAttr.u32ChnCnt == 1)
    {
        stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    }

    stAoSetAttr.eSamplerate = (MI_AUDIO_SampleRate_e)stWavHeaderInput.wave.dwSamplesPerSec;

    ExecFunc(MI_AO_SetPubAttr(AoDevId, &stAoSetAttr), MI_SUCCESS);
    ExecFunc(MI_AO_GetPubAttr(AoDevId, &stAoGetAttr), MI_SUCCESS);
    ExecFunc(MI_AO_Enable(AoDevId), MI_SUCCESS);
    ExecFunc(MI_AO_EnableChn(AoDevId, AoChn), MI_SUCCESS);

    if (bAoSetVolume)
    {
        ExecFunc(MI_AO_SetVolume(AoDevId, s32AoVolume), MI_SUCCESS);
        ExecFunc(MI_AO_GetVolume(AoDevId, &s32AoGetVolume), MI_SUCCESS);
    }

#if 0
    // for test new API
    memset(&stAoChnParam, 0x0, sizeof(MI_AO_ChnParam_t));
    stAoChnParam.stChnGain.bEnableGainSet = TRUE;
    stAoChnParam.stChnGain.s16Gain = -10;
    ExecFunc(MI_AO_SetChnParam(AoDevId, 0, &stAoChnParam), MI_SUCCESS);
    memset(&stAoChnParam, 0x0, sizeof(MI_AO_ChnParam_t));
    ExecFunc(MI_AO_GetChnParam(AoDevId, 0, &stAoChnParam), MI_SUCCESS);
    printf("Dev%dChn%d bEnableSetting:%d gain:%d.\n",
            AoDevId, 0, stAoChnParam.stChnGain.bEnableGainSet,
            stAoChnParam.stChnGain.s16Gain);

    memset(&stAoChnParam, 0x0, sizeof(MI_AO_ChnParam_t));
    stAoChnParam.stChnGain.bEnableGainSet = TRUE;
    stAoChnParam.stChnGain.s16Gain = 5;
    ExecFunc(MI_AO_SetChnParam(AoDevId, 1, &stAoChnParam), MI_SUCCESS);
    memset(&stAoChnParam, 0x0, sizeof(MI_AO_ChnParam_t));
    ExecFunc(MI_AO_GetChnParam(AoDevId, 1, &stAoChnParam), MI_SUCCESS);
    printf("Dev%dChn%d bEnableSetting:%d gain:%d.\n",
            AoDevId, 1, stAoChnParam.stChnGain.bEnableGainSet,
            stAoChnParam.stChnGain.s16Gain);
#endif

    s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 2 * (stAoSetAttr.u32ChnCnt);
    if (E_MI_AUDIO_SAMPLE_RATE_8000 == stAoSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_8K;;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stAoSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_16K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stAoSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_32K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stAoSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_48K;
    }

    if (stAoSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO)
    {
        if (s32NeedSize > (u32DmaBufSize / 4))
        {
            s32NeedSize = u32DmaBufSize / 4;
        }
    }
    else if (stAoSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO)
    {
        if (s32NeedSize > (u32DmaBufSize / 8))
        {
            s32NeedSize = u32DmaBufSize / 8;
        }
    }

    if (AoDevId == AO_DEV_I2S_TX)
    {
        pthread_create(&Aotid, NULL, aoTestCase[u32AoCodecTestCase].frame_deal, NULL);
        aoTestCase[u32AoCodecTestCase].codec_init();
    }
    else if (AoDevId == AO_DEV_LINE_OUT)
    {
        pthread_create(&Aotid, NULL, aoSendFrame, NULL);
    }
    printf("create ao thread.\n");
	return 0;

}

MI_S32 deinitAi(void)
{
    MI_U32 u32ChnIdx;
    MI_U32 u32ChnCnt;

    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    if (AI_DEV_I2S_RX == AiDevId || AI_DEV_AMIC_AND_I2S_RX == AiDevId || AI_DEV_DMIC_AND_I2S_RX == AiDevId)
    {
        aiTestCase[u32AiCodecTestCase].codec_deinit();
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        ExecFunc(MI_AI_DisableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
    }
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);
    return 0;
}

MI_S32 deinitAo(void)
{
    if (AO_DEV_I2S_TX == AoDevId)
    {
        aoTestCase[u32AoCodecTestCase].codec_deinit();
    }
    ExecFunc(MI_AO_DisableChn(AoDevId, AoChn), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(AoDevId), MI_SUCCESS);

    return 0;
}

int main (int argc, char *argv[])
{
    MI_S32      s32Opt = 0;
    MI_BOOL     bCheckPass = FALSE;
    MI_U32      u32ChnIdx;
    MI_U32      u32ChnCnt;

    // parsing command line
    while ((s32Opt = getopt(argc, argv, "t:I::o:c:v:d:s:m:w:O::i:V:D:W:")) != -1)
    {
        switch(s32Opt)
        {
            // run time
            case 't':
                {
                    u32RunTime = atoi(optarg);
                }
                break;

            // enable Ai
            case 'I':
                {
                    bEnableAI = TRUE;
                }
                break;

            // set Ai output path
            case 'o':
                {
                    pu8AiOutputPath = (MI_U8 *)optarg;
                }
                break;

            // set Ai chn num
            case 'c':
                {
                    u32AiChnCnt = atoi(optarg);
                }
                break;

            // set Ai volume
            case 'v':
                {
                    s32AiVolume = (MI_S32)atoi(optarg);
                    bAiSetVolume = TRUE;
                }
                break;

            // set Ai device ID
            case 'd':
                {
                    AiDevId = atoi(optarg);
                }
                break;

            // set Ai sample rate
            case 's':
                {
                    eAiSampleRate = (MI_AUDIO_SampleRate_e)atoi(optarg);
                }
                break;

            // set Ai Sound Mode
            case 'm':
                {
                    eAiSoundMode = (MI_U32)atoi(optarg);
                    if (eAiSoundMode < E_AI_SOUND_MODE_MONO || eAiSoundMode > E_AI_SOUND_MODE_QUEUE)
                    {
						printf("Illegal sound mode!!!\n");
						return -1;
                    }
                }
                break;

            // set Ai codec test
            case 'w':
                {
                    u32AiCodecTestCase = atoi(optarg);
                }
                break;

            // enable AO
            case 'O':
                {
                    bEnableAO = TRUE;
                }
                break;

            // set Ao input path
            case 'i':
                {
                    pu8AoInputPath = (MI_U8 *)optarg;
                }
                break;

            // set Ao volume
            case 'V':
                {
                    s32AoVolume = (MI_S32)atoi(optarg);
                    bAoSetVolume = TRUE;
                }
                break;

            case 'D':
                {
                    AoDevId = atoi(optarg);
                }
                break;

            // set Ao codec test
            case 'W':
                {
                    u32AoCodecTestCase = atoi(optarg);
                }
                break;

            case '?':
                if(optopt == 'o')
                {
                    printf("Missing Ai output file path, please -o 'output file path' \n");
                }
                else if(optopt == 'c')
                {
                    printf("Missing Ai channel num, please -c 'channel num' \n");
                }
                else if(optopt == 'v')
                {
                    printf("Missing Volume, pleae -v 'volume' \n");
                }
                else if(optopt == 'd')
                {
                    printf("Missing Ai device ID, please -d 'Ai device id' \n");
                }
                else if (optopt == 's')
                {
                    printf("Missing Ai input sample rate, please -s 'input sample rate' \n");
                }
                else if (optopt == 'm')
                {
					printf("Missing Ai sound mode, please -m 'Ai sound mode' \n");
                }
                else if (optopt == 'w')
                {
					printf("Missing input codec for test, please -w 'codec test case id' \n");
                }
                else if(optopt == 'i')
                {
                    printf("Missing Ao input file path, please -i 'input file path' \n");
                }
                else if(optopt == 'V')
                {
                    printf("Missing Ao volume, pleae -V 'volume' \n");
                }
                else if(optopt == 'D')
                {
                    printf("Missing Ao device ID, please -d 'Ao device id' \n");
                }
                else if (optopt == 'W')
                {
					printf("Missing output codec for test, please -W 'codec test case id' \n");
                }
                else
                {
                    printf("Invalid option received \n");
                }

            default:
                display_help();
                return -1;
        }
    }

    bCheckPass = checkParam();
    if (FALSE == bCheckPass)
    {
        printf("Fail to check param.\n");
        display_help();
        return -1;
    }
    else
    {
        initParam();
        printParam();
    }

    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    ExecFunc(MI_SYS_Init(), MI_SUCCESS);

   	//atexit(freePrivatePoolExit);
    //ExecFunc(allocPrivatePool(), MI_SUCCESS);

    // enable ai
    if (bEnableAI)
    {
        ExecFunc(initAi(), MI_SUCCESS);
    }

    // enable ao
    if (bEnableAO)
    {
        ExecFunc(initAo(), MI_SUCCESS);
    }

    // join ai
    if (bEnableAI)
    {
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            pthread_join(stAiChnPriv[u32ChnIdx].tid, NULL);
        }
        printf("join Ai thread done.\n");
    }

    // join ao
    if (bEnableAO)
    {
        pthread_join(Aotid, NULL);
        printf("join Ao thread done.\n");
    }

    // disable ai
    if (bEnableAI)
    {
        ExecFunc(deinitAi(), MI_SUCCESS);
    }

    // disable ao
    if (bEnableAO)
    {
        ExecFunc(deinitAo(), MI_SUCCESS);
    }

	//ExecFunc(freePrivatePool(), MI_SUCCESS);
    ExecFunc(MI_SYS_Exit(), MI_SUCCESS);
    return 0;
}
