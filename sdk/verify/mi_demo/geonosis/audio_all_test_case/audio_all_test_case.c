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

#if defined(HDMI)
#include "mi_hdmi.h"
#endif

#define WAV_G711A 	(0x06)
#define WAV_G711U 	(0x07)
#define WAV_G726 	(0x45)
#define WAV_PCM  	(0x1)

#define G726_16 	(2)
#define G726_24 	(3)
#define G726_32 	(4)
#define G726_40 	(5)

#define AI_AMIC_CHN_MAX 	        (2)
#define AI_DMIC_CHN_MAX		        (4)
#define AI_I2S_NOR_CHN 		        (2)
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

#if defined(HDMI)
#define AO_DEV_HDMI   	(2)
#define AO_DEV_HDMI_AND_LINEOUT (3)
#endif

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

#define ExecFuncNoExit(func, _ret_, __ret) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    __ret = s32TmpRet;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
        printf("%d End test: %s\n", __LINE__, #func);	\
    }\
}while(0);

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
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
    MI_S8 *ps8SrcPcmOutputFile;
    MI_AUDIO_DEV AiDevId;
    MI_AUDIO_Attr_t stAiAttr;
    MI_BOOL bSetVqeWorkingSampleRate;
    MI_AUDIO_SampleRate_e eVqeWorkingSampleRate;
    MI_BOOL bEnableRes;
    MI_AUDIO_SampleRate_e eOutSampleRate;
    MI_BOOL bEnableAenc;
    MI_AI_AencConfig_t stSetAencConfig;
    MI_AI_VqeConfig_t stSetVqeConfig;
    MI_BOOL bEnableBf;
} AiOutFileName_t;

typedef struct AiChnPriv_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_S32 s32Fd;
    MI_U32 u32TotalSize;
    MI_U32 u32SrcPcmTotalSize;
    MI_U32 u32ChnCnt;
    MI_BOOL bEnableAed;
    pthread_t tid;
    MI_S32 s32SrcPcmFd;
} AiChnPriv_t;

static MI_BOOL  bEnableAI = FALSE;
static MI_U8*   pu8AiOutputPath = NULL;
static MI_BOOL  bAiEnableVqe = FALSE;
static MI_BOOL  bAiEnableHpf = FALSE;
static MI_BOOL  bAiEnableAgc = FALSE;
static MI_BOOL  bAiEnableNr = FALSE;
static MI_BOOL  bAiEnableEq = FALSE;
static MI_BOOL  bAiEnableAec = FALSE;
static MI_BOOL  bAiEnableResample = FALSE;
static MI_BOOL	bAiEnableSsl = FALSE;
static MI_BOOL	bAiEnableBf = FALSE;
static MI_AUDIO_SampleRate_e eAiOutputResampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_BOOL  bAiEnableAenc = FALSE;
static MI_AUDIO_AencType_e eAiAencType = E_MI_AUDIO_AENC_TYPE_INVALID;
static MI_AUDIO_G726Mode_e eAiAencG726Mode = E_MI_AUDIO_G726_MODE_INVALID;
static SoundMode_e eAiWavSoundMode = E_SOUND_MODE_MONO;
static AencType_e eAiWavAencType = PCM;
static MI_BOOL  bAiEnableAed = FALSE;
static MI_U32   u32AiChnCnt = 0;
static MI_S32   s32AiVolume = 0;
static MI_BOOL  bAiSetVolume = FALSE;
static MI_AUDIO_DEV AiDevId = -1;
static MI_AUDIO_SampleRate_e eAiWavSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_AUDIO_SampleRate_e eAiSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static AiSoundMode_e eAiSoundMode = E_AI_SOUND_MODE_MONO;
static MI_BOOL 	bAiI2sNormalMode = TRUE;
static MI_U32	u32MicDistance = 0;
static MI_U32 	u32VqeWorkingSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_BOOL  bAiDumpPcmData = FALSE;
static MI_BOOL  bAiSetBfDoa = FALSE;
static MI_S32   s32AiBfDoa = 0;

static MI_BOOL  bEnableAO = FALSE;
static MI_U8*   pu8AoInputPath = NULL;
static MI_BOOL  bAoEnableVqe = FALSE;
static MI_BOOL  bAoEnableHpf = FALSE;
static MI_BOOL  bAoEnableAgc = FALSE;
static MI_BOOL  bAoEnableNr = FALSE;
static MI_BOOL  bAoEnableEq = FALSE;
static MI_BOOL  bAoEnableResample = FALSE;
static MI_BOOL  bAoEnableAdec = FALSE;
static MI_AUDIO_SampleRate_e eAoOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_AUDIO_SampleRate_e eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_S32   s32AoVolume = 0;
static MI_BOOL  bAoSetVolume = FALSE;
static MI_AUDIO_DEV AoDevId = -1;

static MI_U32   u32RunTime = 0;
static MI_BOOL  bAiExit = FALSE;
static MI_BOOL  bAoExit = FALSE;

static MI_S32   AiChnFd[MI_AUDIO_MAX_CHN_NUM] = {[0 ... MI_AUDIO_MAX_CHN_NUM-1] = -1};
static MI_S32   AiChnSrcPcmFd[MI_AUDIO_MAX_CHN_NUM] = {[0 ... MI_AUDIO_MAX_CHN_NUM-1] = -1};

static AiChnPriv_t stAiChnPriv[MI_AUDIO_MAX_CHN_NUM];

static MI_S32   AoReadFd = -1;
static WaveFileHeader_t stWavHeaderInput;
static MI_AO_CHN AoChn = 0;
static MI_S32 s32NeedSize = 0;
static pthread_t Aotid;

MI_U8 u8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];
static MI_U32   res[100];

MI_AUDIO_HpfConfig_t stHpfCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .eHpfFreq = E_MI_AUDIO_HPF_FREQ_150,
};

MI_AI_AecConfig_t stAecCfg = {
    .bComfortNoiseEnable = FALSE,
    .s16DelaySample = 0,
    .u32AecSupfreq = {4, 6, 36, 49, 50, 51},
    .u32AecSupIntensity = {5, 4, 4, 5, 10, 10, 10},
};

MI_AUDIO_AnrConfig_t stAnrCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_MUSIC,
    .u32NrIntensity = 15,
    .u32NrSmoothLevel = 10,
    .eNrSpeed = E_MI_AUDIO_NR_SPEED_MID,
};

MI_AUDIO_AgcConfig_t stAgcCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .s32NoiseGateDb = -60,
    .s32TargetLevelDb = -3,
    .stAgcGainInfo = {
        .s32GainInit = 0,
        .s32GainMax = 20,
        .s32GainMin = 0,
    },
    .u32AttackTime = 1,
    .s16Compression_ratio_input = {-80, -60, -40, -25, 0},
    .s16Compression_ratio_output = {-80, -30, -15, -10, -3},
    .u32DropGainMax = 12,
    .u32NoiseGateAttenuationDb = 0,
    .u32ReleaseTime = 3,
};

MI_AUDIO_EqConfig_t stEqCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .s16EqGainDb = {[0 ... 128] = 3},
};

MI_AI_AedConfig_t stAedCfg = {
    .bEnableNr = TRUE,
    .eSensitivity = E_MI_AUDIO_AED_SEN_HIGH,
    .s32OperatingPoint = -5,
    .s32VadThresholdDb = -40,
    .s32LsdThresholdDb = -15,
};

MI_AI_SslInitAttr_t stSslInit = {
	.bBfMode = FALSE,
};

MI_AI_SslConfigAttr_t stSslConfig = {
	.s32Temperature = 25,
	.s32NoiseGateDbfs = -40,
	.s32DirectionFrameNum = 300,
};

MI_AI_BfInitAttr_t stBfInit = {
	.u32ChanCnt = 2,
};

MI_AI_BfConfigAttr_t stBfConfig = {
	.s32Temperature = 25,
	.s32NoiseGateDbfs = -40,
	.s32NoiseSupressionMode = 8,
	.s32NoiseEstimation = 1,
	.outputGain = 0.7,
};


MI_U32 u32AiDevHeapSize = 0;
MI_U32 u32AiChnOutputHeapSize = 0;
MI_U32 u32AoDevHeapSize = 0;

MI_BOOL bAllocAiDevPool = FALSE;
MI_BOOL bAllocAiChnOutputPool = FALSE;
MI_BOOL bAllocAoDevPool = FALSE;

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
    printf("-h : AI Enable Hpf\n");
    printf("-g : AI Enable Agc\n");
    printf("-e : AI Enable Eq\n");
    printf("-n : AI Enable Nr\n");
    printf("-r : AI Resample Rate\n");
    printf("-a : AI Aenc Type [g711a g711u g726_16 g726_24 g726_32 g726_40]\n");
    printf("-A : AI Enable Aed\n");
    printf("-b : AI Enable SW Aec\n");
    printf("-S : AI Enable Ssl\n");
    printf("-F : AI Enable Beamforming\n");
    printf("-M : AI mic disttance for Ssl/Bf (cm, step 1 cm)\n");
    printf("-C : AI Bf doc (0~180)\n");
    printf("-w : AI Aec Working Sample Rate(Not necessary)\n");
    printf("-W : AI enable dump pcm data\n");
    printf("\n\n");
    printf("AO Device Id: Lineout[0] I2S TX[1] "
#if defined(HDMI)
            "HDMI[2] HDMI/LineOut[3]"
#endif
            "\n");
    printf("AO test option :\n");
    printf("-O : Enable AO\n");
    printf("-i : AO Input Path\n");
    printf("-D : AO Device Id\n");
    printf("-V : AO Volume\n");
    printf("-H : AO Enable Hpf\n");
    printf("-G : AO Enable Agc\n");
    printf("-E : AO Enable Eq\n");
    printf("-N : AO Enable Nr\n");
    printf("-R : AO Resample Rate\n");
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
            			|| (AI_DEV_LINE_IN == AiDevId)

            		)
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
					if ((AI_I2S_NOR_CHN != u32AiChnCnt)
						)
					{
						printf("Ai channel count invalid!!!\n");
	                    break;
					}
				}
				else
				{
					if ((AI_I2S_NOR_CHN / 2 != u32AiChnCnt)
						)
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
			    else if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
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
			    else if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
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

            if (bAiEnableAec)
            {
                if (
                	((E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate) && (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate))
                    &&
                    ((E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                		|| ((E_MI_AUDIO_SAMPLE_RATE_8000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_16000 != u32VqeWorkingSampleRate)))
                	)
                {
                    printf("Aec only support 8K/16K!!!\n");
                    break;
                }

                if (AI_DEV_I2S_RX == AiDevId)
                {
                    printf("I2S RX not support AEC!!!\n");
                    break;
                }
            }

            if (bAiEnableVqe)
            {
                if (
                	((E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate)
                		&& (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate)
                    	&& (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiSampleRate))
                    &&
                    ((E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                		|| ((E_MI_AUDIO_SAMPLE_RATE_8000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_16000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_48000 != u32VqeWorkingSampleRate))))
                {
                    printf("Vqe only support 8K/16/48K!!!\n");
                    break;
                }

                if (bAiEnableHpf)
                {
                    if (((E_MI_AUDIO_SAMPLE_RATE_48000 == eAiSampleRate)
                    	&& (E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate))
                    		|| (E_MI_AUDIO_SAMPLE_RATE_48000 == u32VqeWorkingSampleRate))
                    {

                        printf("Hpf not support 48K!!!\n");
                        break;
                    }
                }
            }

            if (TRUE == bAiEnableResample)
            {
                if (
                        (E_MI_AUDIO_SAMPLE_RATE_8000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiOutputResampleRate)
                )
                {
                    printf("Ai resample rate invalid!!!\n");
                    break;
                }

                if (E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                {
                    if (eAiOutputResampleRate == eAiSampleRate)
                    {
                        printf("eAiSampleRate:%d eAiOutputResampleRate:%d. It does not need to resample.\n",
                                eAiSampleRate, eAiOutputResampleRate);
                        break;
                    }
                }
                else
                {
                    if (eAiOutputResampleRate == u32VqeWorkingSampleRate)
                    {
                        printf("u32VqeWorkingSampleRate:%d eAiOutputResampleRate:%d. It does not need to resample.\n",
                                u32VqeWorkingSampleRate, eAiOutputResampleRate);
                        break;
                    }
                }
            }

            if (TRUE == bAiEnableAenc)
            {
                if (
                        (E_MI_AUDIO_AENC_TYPE_G711A != eAiAencType)
                    &&  (E_MI_AUDIO_AENC_TYPE_G711U != eAiAencType)
                    &&  (E_MI_AUDIO_AENC_TYPE_G726 != eAiAencType)
                )
                {
                    printf("Ai aenc type invalid!!!\n");
                    break;
                }

                if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
                {
                    if (
                            (E_MI_AUDIO_G726_MODE_16 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_24 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_32 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_40 != eAiAencG726Mode)
                    )
                    {
                        printf("Ai G726 mode invalid!!!\n");
                        break;
                    }
                }

                if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
                {
                    if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
                    {
                        printf("Wav not support stereo g726!!!\n");
                        break;
                    }
                }
            }

            if (bAiEnableSsl || bAiEnableBf)
            {
				if (0 == u32MicDistance)
				{
					printf("Ai mic distance invalid!!!\n");
					break;
				}

				if (E_AI_SOUND_MODE_STEREO != eAiSoundMode)
				{
					printf("Ssl/Bf only support stereo mode!!!\n");
					break;
				}

				if (bAiEnableBf && bAiSetBfDoa)
				{
                    if (0 < s32AiBfDoa || s32AiBfDoa > 180)
                    {
                        printf("s32AiBfDoa only supports 0~180 currently.\n");
                        break;
                    }
				}
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

            if (TRUE == bAoEnableResample)
            {
                if (
                        (E_MI_AUDIO_SAMPLE_RATE_8000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAoOutSampleRate)
                )
                {
                    printf("AO resample rate invalid!!!\n");
                    break;
                }
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
        }
        bCheckPass = TRUE;
    }while(0);
    return bCheckPass;
}

void initParam(void)
{
    eAiWavSampleRate = eAiSampleRate;

    if (E_MI_AUDIO_AENC_TYPE_G711A == eAiAencType)
    {
        eAiWavAencType = E_AENC_TYPE_G711A;
    }
    else if (E_MI_AUDIO_AENC_TYPE_G711U == eAiAencType)
    {
        eAiWavAencType = E_AENC_TYPE_G711U;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_16 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_16;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_24 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_24;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_32 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_32;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_40 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_40;
    }
    else
    {
        eAiWavAencType = PCM;
    }

	if (AI_DEV_I2S_RX == AiDevId)
	{
		if (
			(((E_AI_SOUND_MODE_MONO == eAiSoundMode) || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode)) && (AI_I2S_NOR_CHN == u32AiChnCnt))
			|| ((E_AI_SOUND_MODE_STEREO == eAiSoundMode) && (AI_I2S_NOR_CHN / 2 == u32AiChnCnt)))
		{
			bAiI2sNormalMode = TRUE;
		}
		else
		{
			bAiI2sNormalMode = FALSE;
		}
	}

	if (bAiEnableSsl || bAiEnableBf)
	{
		stSslInit.u32MicDistance = u32MicDistance;
		stBfInit.u32MicDistance = u32MicDistance;
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

        // aed
        printf("Aed:");
        if (bAiEnableAed)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

		// vqe working sample rate
		printf("Vqe working sample rate:");
		if (E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
		{
			printf("not specified");
		}
		else
		{
			printf("%dK", u32VqeWorkingSampleRate / 1000);
		}
		printf("\n");

        // aec
        printf("Aec:");
        if (bAiEnableAec)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        if (bAiEnableAec)
        {
            printf("Aec Type:");
            {
                printf("SW");
            }
            printf("\n");
        }

        // hpf
        printf("Hpf:");
        if (bAiEnableHpf)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // nr
        printf("Nr:");
        if (bAiEnableNr)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // agc
        printf("Agc:");
        if (bAiEnableAgc)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // eq
        printf("Eq:");
        if (bAiEnableEq)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // ssl
        printf("Ssl:");
        if (bAiEnableSsl)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

		// bf
		printf("Bf:");
        if (bAiEnableBf)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }

        if (TRUE == bAiSetBfDoa)
        {
            printf("Bf Doa:%d.\n", s32AiBfDoa);
        }
        printf("\n");

        // resample
        printf("Resample:");
        if (bAiEnableResample)
        {
            printf("Enable %dK", eAiOutputResampleRate / 1000);
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // aenc
        printf("Aenc:");
        if (bAiEnableAenc)
        {
            printf("Enable ");
            if (E_AENC_TYPE_G711A == eAiWavAencType)
            {
                printf("g711a");
            }
            else if (E_AENC_TYPE_G711U == eAiWavAencType)
            {
                printf("g711u");
            }
            else if (E_AENC_TYPE_G726_16 == eAiWavAencType)
            {
                printf("g726_16");
            }
            else if (E_AENC_TYPE_G726_24 == eAiWavAencType)
            {
                printf("g726_24");
            }
            else if (E_AENC_TYPE_G726_32 == eAiWavAencType)
            {
                printf("g726_32");
            }
            else if (E_AENC_TYPE_G726_40 == eAiWavAencType)
            {
                printf("g726_40");
            }
        }
        else
        {
            printf("Disable");
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

#if defined(HDMI)
        else if (AoDevId == AO_DEV_HDMI)
        {
            printf("HDMI");
        }
        else if (AoDevId == AO_DEV_HDMI_AND_LINEOUT)
        {
			printf("HDMI/LineOut");
        }
#endif

        printf("\n");

        // hpf
        printf("Hpf:");
        if (bAoEnableHpf)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // nr
        printf("Nr:");
        if (bAoEnableNr)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // agc
        printf("Agc:");
        if (bAoEnableAgc)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // eq
        printf("Eq:");
        if (bAoEnableEq)
        {
            printf("Enable");
        }
        else
        {
            printf("Disable");
        }
        printf("\n");

        // resample
        printf("Resample:");
        if (bAoEnableResample)
        {
            printf("Enable %dK", eAoOutSampleRate / 1000);
        }
        else
        {
            printf("Disable");
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
    char asAencG726Mode[4][15] = {"_G726_16", "_G726_24", "_G726_32", "_G726_40"};

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
    if (E_MI_AUDIO_SOUND_MODE_MONO == pstAiOutFileName->stAiAttr.eSoundmode)
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

    strcpy((char *)pstAiOutFileName->ps8SrcPcmOutputFile, (char *)pstAiOutFileName->ps8OutputFile);

	// set vqe working sample rate
	if (pstAiOutFileName->bSetVqeWorkingSampleRate)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_%dKVqeWork", pstAiOutFileName->eVqeWorkingSampleRate / 1000);
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    //set  OutpuFile ResampleRate
    if (pstAiOutFileName->bEnableRes)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_%dKRES", pstAiOutFileName->eOutSampleRate / 1000);
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    //************set OutpuFile Veq Mod START******************
	if (TRUE == pstAiOutFileName->stSetVqeConfig.bAecOpen)
	{
		memset(as8Tmp, 0, sizeof(as8Tmp));
		sprintf(as8Tmp, "_Aec");
		strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
	}

    if (TRUE == pstAiOutFileName->bEnableBf)
    {
		memset(as8Tmp, 0, sizeof(as8Tmp));
		sprintf(as8Tmp, "_Bf");
		strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    if (TRUE == pstAiOutFileName->stSetVqeConfig.bHpfOpen)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_Hpf");
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    if (TRUE == pstAiOutFileName->stSetVqeConfig.bAnrOpen)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_Anr");
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    if (TRUE == pstAiOutFileName->stSetVqeConfig.bAgcOpen)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_Agc");
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }

    if(TRUE == pstAiOutFileName->stSetVqeConfig.bEqOpen)
    {
        memset(as8Tmp, 0, sizeof(as8Tmp));
        sprintf(as8Tmp, "_Eq");
        strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    }
    //************set OutpuFile Veq Mod END******************

    // set OutpufFile Aenc
    if (TRUE == pstAiOutFileName->bEnableAenc)
    {
        if (E_MI_AUDIO_AENC_TYPE_G711A == pstAiOutFileName->stSetAencConfig.eAencType)
        {
            memset(as8Tmp, 0, sizeof(as8Tmp));
            sprintf(as8Tmp, "_G711a");
            strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
        }
        else if (E_MI_AUDIO_AENC_TYPE_G711U == pstAiOutFileName->stSetAencConfig.eAencType)
        {
            memset(as8Tmp, 0, sizeof(as8Tmp));
            sprintf(as8Tmp, "_G711u");
            strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
        }
        else if (E_MI_AUDIO_AENC_TYPE_G726 == pstAiOutFileName->stSetAencConfig.eAencType)
        {
            memset(as8Tmp, 0, sizeof(as8Tmp));
            sprintf(as8Tmp, asAencG726Mode[pstAiOutFileName->stSetAencConfig.stAencG726Cfg.eG726Mode]);
            strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
        }
    }

    memset(as8Tmp, 0, sizeof(as8Tmp));
    sprintf(as8Tmp, ".wav");
    strcat((char *)pstAiOutFileName->ps8OutputFile, as8Tmp);
    strcat((char *)pstAiOutFileName->ps8SrcPcmOutputFile, as8Tmp);
    return 0;
}

void* aiGetChnPortBuf(void* data)
{
    AiChnPriv_t* priv = (AiChnPriv_t*)data;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;
    MI_S32 s32Ret;
    MI_U32 u32ChnIndex;
    struct timeval tv_before, tv_after;
    MI_S64 before_us, after_us;
	MI_AI_AedResult_t stAedResult;
    WaveFileHeader_t stWavHead;
    MI_S32 s32Doa;

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));

    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode) || (E_AI_SOUND_MODE_STEREO == eAiSoundMode))
    {
        while(FALSE == bAiExit)
        {
            s32Ret = MI_AI_GetFrame(priv->AiDevId, priv->AiChn, &stAiChFrame, &stAecFrame, -1);
            if (MI_SUCCESS == s32Ret)
            {
                if (priv->bEnableAed)
                {
                    MI_AI_GetAedResult(priv->AiDevId, priv->AiChn, &stAedResult);
                    printf("Dev%d Chn%d, bAcousticEventDetected[%d] bLoudSoundDetected[%d].\n",
                        priv->AiDevId, priv->AiChn, stAedResult.bAcousticEventDetected, stAedResult.bLoudSoundDetected);
                }
				else if ((E_AI_SOUND_MODE_STEREO == eAiSoundMode) && (bAiEnableSsl) &&
				        ((!bAiEnableBf) || (bAiEnableBf && bAiSetBfDoa)))
                {
					MI_AI_GetSslDoa(AiDevId, priv->AiChn, &s32Doa);
					printf("SSL Doa is %d.\n", s32Doa);
                }
                else
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

	                if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
	                {
                        if (TRUE == bAiDumpPcmData)
                        {
	                	    write(priv->s32SrcPcmFd, stAiChFrame.apSrcPcmVirAddr[0], stAiChFrame.u32SrcPcmLen);
	                	    priv->u32SrcPcmTotalSize += stAiChFrame.u32SrcPcmLen;
	                    }
                    }
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

        if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
        {
            if (TRUE == bAiDumpPcmData)
            {
			    memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
                if (TRUE == bAiEnableBf)
                {
                    eAiWavSoundMode = E_SOUND_MODE_STEREO;
                }
	            addWaveHeader(&stWavHead, PCM, eAiWavSoundMode, eAiSampleRate, priv->u32SrcPcmTotalSize);
	            lseek(priv->s32SrcPcmFd, 0, SEEK_SET);
	            write(priv->s32SrcPcmFd, &stWavHead, sizeof(WaveFileHeader_t));
	            close(priv->s32SrcPcmFd);
            }
        }
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
                    if (priv->bEnableAed)
                    {
                        MI_AI_GetAedResult(priv->AiDevId, u32ChnIndex, &stAedResult);
                        printf("Dev%d Chn%d, bAcousticEventDetected[%d] bLoudSoundDetected[%d].\n",
                            priv->AiDevId, u32ChnIndex, stAedResult.bAcousticEventDetected, stAedResult.bLoudSoundDetected);
                    }
                    write(AiChnFd[u32ChnIndex], stAiChFrame.apVirAddr[u32ChnIndex], stAiChFrame.u32Len / priv->u32ChnCnt);

                    if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
	                {
                        if (TRUE == bAiDumpPcmData)
                        {
	                	    write(AiChnSrcPcmFd[u32ChnIndex], stAiChFrame.apSrcPcmVirAddr[u32ChnIndex],
	                		    stAiChFrame.u32SrcPcmLen / priv->u32ChnCnt);
                        }
                    }
                }
                priv->u32TotalSize += stAiChFrame.u32Len / priv->u32ChnCnt;

                if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
                {
                    if (TRUE == bAiDumpPcmData)
                    {
					    priv->u32SrcPcmTotalSize += stAiChFrame.u32SrcPcmLen / priv->u32ChnCnt;
                    }
                }
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

        if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
        {
            if (TRUE == bAiDumpPcmData)
            {
			    memset(&stWavHead, 0, sizeof(WaveFileHeader_t));
                if (TRUE == bAiEnableBf)
                {
                    eAiWavSoundMode = E_SOUND_MODE_STEREO;
                }
	            addWaveHeader(&stWavHead, PCM, eAiWavSoundMode, eAiSampleRate, priv->u32SrcPcmTotalSize);
	            for (u32ChnIndex = 0; u32ChnIndex < priv->u32ChnCnt; u32ChnIndex++)
	            {
	                lseek(AiChnSrcPcmFd[u32ChnIndex], 0, SEEK_SET);
	                write(AiChnSrcPcmFd[u32ChnIndex], &stWavHead, sizeof(WaveFileHeader_t));
	                close(AiChnSrcPcmFd[u32ChnIndex]);
	            }
            }
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
		if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
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

		if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
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
    MI_AI_AencConfig_t  stAiSetAencConfig, stAiGetAencConfig;
    MI_AI_VqeConfig_t   stAiSetVqeConfig, stAiGetVqeConfig;
    MI_AI_AedConfig_t   stAiSetAedConfig, stAiGetAedConfig;
    MI_U32              u32ChnIdx;
    MI_U32              u32ChnCnt;
    MI_S8               s8OutputFileName[512];
    MI_S8               s8SrcPcmOutputFileName[512];
    MI_S32              s32Ret;
    WaveFileHeader_t    stAiWavHead;
    MI_SYS_ChnPort_t    stAiChnOutputPort0[MI_AUDIO_MAX_CHN_NUM];
    MI_AI_ChnParam_t    stAiChnParam;
    MI_AI_SslInitAttr_t	stAiGetSslInitAttr;
    MI_AI_SslConfigAttr_t	stAiGetSslConfigAttr;
    MI_AI_BfInitAttr_t		stAiGetBfInitAttr;
    MI_AI_BfConfigAttr_t	stAiGetBfConfigAttr;

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

	if (bAiEnableBf)
	{
		eAiWavSoundMode = E_SOUND_MODE_MONO;
	}

    memset(&stAiSetVqeConfig, 0x0, sizeof(MI_AI_VqeConfig_t));
    if (bAiEnableVqe)
    {
		if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
			|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
		{
			stAiSetVqeConfig.u32ChnNum = 1;
		}
		else
		{
			stAiSetVqeConfig.u32ChnNum = 2;
		}

		if (bAiEnableBf)
		{
			stAiSetVqeConfig.u32ChnNum = 1;
		}

        stAiSetVqeConfig.bAecOpen = bAiEnableAec;
        stAiSetVqeConfig.bAgcOpen = bAiEnableAgc;
        stAiSetVqeConfig.bAnrOpen = bAiEnableNr;
        stAiSetVqeConfig.bEqOpen = bAiEnableEq;
        stAiSetVqeConfig.bHpfOpen = bAiEnableHpf;
        stAiSetVqeConfig.s32FrameSample = 128;
        if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
        {
        	stAiSetVqeConfig.s32WorkSampleRate = u32VqeWorkingSampleRate;
        }
        else
        {
			stAiSetVqeConfig.s32WorkSampleRate = eAiSampleRate;
        }

        // AEC
        memcpy(&stAiSetVqeConfig.stAecCfg, &stAecCfg, sizeof(MI_AI_AecConfig_t));

        // AGC
        memcpy(&stAiSetVqeConfig.stAgcCfg, &stAgcCfg, sizeof(MI_AUDIO_AgcConfig_t));

        // ANR
        memcpy(&stAiSetVqeConfig.stAnrCfg, &stAnrCfg, sizeof(MI_AUDIO_AnrConfig_t));

        // EQ
        memcpy(&stAiSetVqeConfig.stEqCfg, &stEqCfg, sizeof(MI_AUDIO_EqConfig_t));

        // HPF
        memcpy(&stAiSetVqeConfig.stHpfCfg, &stHpfCfg, sizeof(MI_AUDIO_HpfConfig_t));
    }

    memset(&stAiSetAencConfig, 0x0, sizeof(MI_AI_AencConfig_t));
    if (bAiEnableAenc)
    {
        stAiSetAencConfig.eAencType = eAiAencType;
        if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
        {
            stAiSetAencConfig.stAencG726Cfg.eG726Mode = eAiAencG726Mode;
        }
    }

    memset(&stAiFileName, 0x0, sizeof(AiOutFileName_t));
    stAiFileName.AiDevId = AiDevId;
    stAiFileName.bEnableAenc = bAiEnableAenc;
    stAiFileName.bEnableRes = bAiEnableResample;
    stAiFileName.eOutSampleRate = eAiOutputResampleRate;
    stAiFileName.ps8OutputFile = (MI_S8*)s8OutputFileName;
    stAiFileName.ps8SrcPcmOutputFile = (MI_S8*)s8SrcPcmOutputFileName;
    stAiFileName.ps8OutputPath = (MI_S8*)pu8AiOutputPath;
    memcpy(&stAiFileName.stAiAttr, &stAiSetAttr, sizeof(MI_AUDIO_Attr_t));
    memcpy(&stAiFileName.stSetVqeConfig, &stAiSetVqeConfig, sizeof(MI_AI_VqeConfig_t));
    memcpy(&stAiFileName.stSetAencConfig, &stAiSetAencConfig, sizeof(MI_AI_AencConfig_t));

	if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
    {
		stAiFileName.bSetVqeWorkingSampleRate = TRUE;
		stAiFileName.eVqeWorkingSampleRate = u32VqeWorkingSampleRate;
    }

    if (TRUE == bAiEnableBf)
    {
        stAiFileName.bEnableBf = TRUE;
    }

    memset(&stAiWavHead, 0x0, sizeof(WaveFileHeader_t));

    u32ChnCnt = u32AiChnCnt;

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        memset(s8OutputFileName, 0x0, sizeof(s8OutputFileName));
        memset(s8SrcPcmOutputFileName, 0x0, sizeof(s8SrcPcmOutputFileName));
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

		if (bAiEnableBf || bAiEnableVqe || bAiEnableResample || bAiEnableAenc)
		{
            if (TRUE == bAiDumpPcmData)
            {
	            printf("OutputName:%s\n", s8SrcPcmOutputFileName);
	            AiChnSrcPcmFd[u32ChnIdx] = open((const char *)s8SrcPcmOutputFileName, O_RDWR | O_CREAT, 0777);
	            if(AiChnSrcPcmFd[u32ChnIdx] < 0)
	            {
	                printf("Open output file path:%s fail \n", s8SrcPcmOutputFileName);
	                printf("error:%s", strerror(errno));
	                return -1;
	            }

	            s32Ret = write(AiChnSrcPcmFd[u32ChnIdx], &stAiWavHead, sizeof(WaveFileHeader_t));
	            if (s32Ret < 0)
	            {
	                printf("write wav head failed\n");
	                return -1;
	            }
            }
        }
    }

    ExecFuncNoExit(MI_AI_SetPubAttr(AiDevId, &stAiSetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AI_GetPubAttr(AiDevId, &stAiGetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AI_Enable(AiDevId), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    if (bAiSetVolume)
    {
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            if ((AI_DEV_AMIC_AND_I2S_RX == AiDevId)
                || ((AI_DEV_DMIC_AND_I2S_RX == AiDevId)))
            {
                if ((E_MI_AUDIO_SOUND_MODE_MONO == eAiSoundMode)
                    || (E_MI_AUDIO_SOUND_MODE_QUEUE == eAiSoundMode))
                {
                    if ((2 == u32ChnIdx) || (3 == u32ChnIdx))
                    {
                        continue;
                    }
                }
                else if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
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
        stAiChnPriv[u32ChnIdx].s32SrcPcmFd = AiChnSrcPcmFd[u32ChnIdx];
        stAiChnPriv[u32ChnIdx].u32ChnCnt = u32AiChnCnt;
        stAiChnPriv[u32ChnIdx].u32TotalSize = 0;
        stAiChnPriv[u32ChnIdx].u32SrcPcmTotalSize = 0;

        stAiChnOutputPort0[u32ChnIdx].eModId = E_MI_MODULE_ID_AI;
        stAiChnOutputPort0[u32ChnIdx].u32DevId = AiDevId;
        stAiChnOutputPort0[u32ChnIdx].u32ChnId = u32ChnIdx;
        stAiChnOutputPort0[u32ChnIdx].u32PortId = 0;
        ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiChnOutputPort0[u32ChnIdx], 1, TOTAL_BUF_DEPTH), MI_SUCCESS);
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        ExecFuncNoExit(MI_AI_EnableChn(AiDevId, u32ChnIdx), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
        {
			goto DISABLE_DEVICE;
        }
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        // AED
        if (bAiEnableAed)
        {
            memcpy(&stAiSetAedConfig, &stAedCfg, sizeof(MI_AI_AedConfig_t));
            ExecFunc(MI_AI_SetAedAttr(AiDevId, u32ChnIdx, &stAiSetAedConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetAedAttr(AiDevId, u32ChnIdx, &stAiGetAedConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableAed(AiDevId, u32ChnIdx), MI_SUCCESS);
            stAiChnPriv[u32ChnIdx].bEnableAed = TRUE;
        }

		// SSL
		if (bAiEnableSsl)
		{
			if ((TRUE == bAiEnableBf) && (FALSE == bAiSetBfDoa))
			{
				stSslInit.bBfMode = TRUE;
			}

			if (E_MI_AUDIO_SAMPLE_RATE_8000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 50;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_16000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 100;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_32000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 200;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_48000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 300;
			}

			ExecFunc(MI_AI_SetSslInitAttr(AiDevId, u32ChnIdx, &stSslInit), MI_SUCCESS);
			ExecFunc(MI_AI_GetSslInitAttr(AiDevId, u32ChnIdx, &stAiGetSslInitAttr), MI_SUCCESS);
			ExecFunc(MI_AI_SetSslConfigAttr(AiDevId, u32ChnIdx, &stSslConfig), MI_SUCCESS);
			ExecFunc(MI_AI_GetSslConfigAttr(AiDevId, u32ChnIdx, &stAiGetSslConfigAttr), MI_SUCCESS);
			ExecFunc(MI_AI_EnableSsl(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

		// BF
		if (bAiEnableBf)
		{
			ExecFunc(MI_AI_SetBfInitAttr(AiDevId, u32ChnIdx, &stBfInit), MI_SUCCESS);
			ExecFunc(MI_AI_GetBfInitAttr(AiDevId, u32ChnIdx, &stAiGetBfInitAttr), MI_SUCCESS);
			ExecFunc(MI_AI_SetBfConfigAttr(AiDevId, u32ChnIdx, &stBfConfig), MI_SUCCESS);
			ExecFunc(MI_AI_GetBfConfigAttr(AiDevId, u32ChnIdx, &stAiGetBfConfigAttr), MI_SUCCESS);
			if (bAiSetBfDoa)
			{
			    ExecFunc(MI_AI_SetBfAngle(AiDevId, u32ChnIdx, s32AiBfDoa), MI_SUCCESS);
			}
			ExecFunc(MI_AI_EnableBf(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

        // VQE
        if(bAiEnableVqe)
        {
            ExecFunc(MI_AI_SetVqeAttr(AiDevId, u32ChnIdx, 0, 0, &stAiSetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetVqeAttr(AiDevId, u32ChnIdx, &stAiGetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableVqe(AiDevId, u32ChnIdx), MI_SUCCESS);
            if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
            {
            	eAiWavSampleRate = u32VqeWorkingSampleRate;
            }
        }

        // RES
        if(bAiEnableResample)
        {
            ExecFunc(MI_AI_EnableReSmp(AiDevId, u32ChnIdx, eAiOutputResampleRate), MI_SUCCESS);
            eAiWavSampleRate = eAiOutputResampleRate;
        }

        // AENC
        if (bAiEnableAenc)
        {
            ExecFunc(MI_AI_SetAencAttr(AiDevId, u32ChnIdx, &stAiSetAencConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetAencAttr(AiDevId, u32ChnIdx, &stAiGetAencConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableAenc(AiDevId, u32ChnIdx), MI_SUCCESS);
        }
        pthread_create(&stAiChnPriv[u32ChnIdx].tid, NULL, aiGetChnPortBuf, &stAiChnPriv[u32ChnIdx]);
    }
    printf("create ai thread.\n");
    return 0;

DISABLE_CHANNEL:
	for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
	{
		ExecFunc(MI_AI_DisableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
	}
DISABLE_DEVICE:
	ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);

ERROR_RETURN:
	return s32Ret;
}

MI_S32 initAo(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Attr_t stAoSetAttr, stAoGetAttr;
    MI_AO_AdecConfig_t stAoSetAdecConfig, stAoGetAdecConfig;
    MI_AO_VqeConfig_t stAoSetVqeConfig, stAoGetVqeConfig;
    MI_S32 s32AoGetVolume;
    MI_AO_ChnParam_t stAoChnParam;
    MI_U32 u32DmaBufSize;
	MI_BOOL bInitAdec = FALSE, bInitRes = FALSE, bInitVqe = FALSE;
#if defined(HDMI)
    MI_HDMI_InitParam_t stHdmiInitParam;
    MI_HDMI_Attr_t stHdmiAttr;
#endif

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

#if defined(HDMI)
    if (
        ((AO_DEV_HDMI == AoDevId) || (AO_DEV_HDMI_AND_LINEOUT == AoDevId))
        &&
            ((E_MI_AUDIO_SAMPLE_RATE_32000 != stWavHeaderInput.wave.dwSamplesPerSec)
        && (E_MI_AUDIO_SAMPLE_RATE_44100 != stWavHeaderInput.wave.dwSamplesPerSec)
        && (E_MI_AUDIO_SAMPLE_RATE_48000 != stWavHeaderInput.wave.dwSamplesPerSec)))
    {
        printf("HDMI not support current sample rate.Only support 32/44.1/48K.\n");
        return -1;
    }

    memset(&stHdmiInitParam, 0x0, sizeof(MI_HDMI_InitParam_t));
    memset(&stHdmiAttr, 0x0, sizeof(MI_HDMI_Attr_t));
    stHdmiAttr.stAudioAttr.bEnableAudio = TRUE;
    stHdmiAttr.stAudioAttr.bIsMultiChannel = stWavHeaderInput.wave.wChannels == 1 ? FALSE : TRUE;
    stHdmiAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
    stHdmiAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    stHdmiAttr.stAudioAttr.eSampleRate =
        E_MI_AUDIO_SAMPLE_RATE_32000 == stWavHeaderInput.wave.dwSamplesPerSec ? E_MI_HDMI_AUDIO_SAMPLERATE_32K : E_MI_HDMI_AUDIO_SAMPLERATE_48K;

    stHdmiAttr.stEnInfoFrame.bEnableAudInfoFrame = FALSE;
    stHdmiAttr.stEnInfoFrame.bEnableAviInfoFrame = FALSE;
    stHdmiAttr.stEnInfoFrame.bEnableSpdInfoFrame = FALSE;

    if ((AO_DEV_HDMI == AoDevId) || (AO_DEV_HDMI_AND_LINEOUT == AoDevId))
    {
        ExecFunc(MI_HDMI_Init(&stHdmiInitParam), MI_SUCCESS);
        ExecFunc(MI_HDMI_Open(E_MI_HDMI_ID_0), MI_SUCCESS);
        ExecFunc(MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stHdmiAttr), MI_SUCCESS);
        ExecFunc(MI_HDMI_Start(E_MI_HDMI_ID_0), MI_SUCCESS);
        ExecFunc(MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, FALSE), MI_SUCCESS);
    }
#endif
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
    eAoInSampleRate = (MI_AUDIO_SampleRate_e)stWavHeaderInput.wave.dwSamplesPerSec;
    if(bAoEnableResample)
    {
        stAoSetAttr.eSamplerate = eAoOutSampleRate;
    }

    memset(&stAoSetAdecConfig, 0x0, sizeof(MI_AO_AdecConfig_t));
    if (WAV_G711A == stWavHeaderInput.wave.wFormatTag)
    {
        bAoEnableAdec = TRUE;
        stAoSetAdecConfig.eAdecType = E_MI_AUDIO_ADEC_TYPE_G711A;
        stAoSetAdecConfig.stAdecG711Cfg.eSamplerate = stAoSetAttr.eSamplerate;
        stAoSetAdecConfig.stAdecG711Cfg.eSoundmode = stAoSetAttr.eSoundmode;
    }
    else if (WAV_G711U == stWavHeaderInput.wave.wFormatTag)
    {
        bAoEnableAdec = TRUE;
        stAoSetAdecConfig.eAdecType = E_MI_AUDIO_ADEC_TYPE_G711U;
        stAoSetAdecConfig.stAdecG711Cfg.eSamplerate = stAoSetAttr.eSamplerate;
        stAoSetAdecConfig.stAdecG711Cfg.eSoundmode = stAoSetAttr.eSoundmode;
    }
    else if (WAV_G726 == stWavHeaderInput.wave.wFormatTag)
    {
        bAoEnableAdec = TRUE;
        stAoSetAdecConfig.eAdecType = E_MI_AUDIO_ADEC_TYPE_G726;
        stAoSetAdecConfig.stAdecG726Cfg.eSamplerate = stAoSetAttr.eSamplerate;
        stAoSetAdecConfig.stAdecG726Cfg.eSoundmode = stAoSetAttr.eSoundmode;

        switch(stWavHeaderInput.wave.wBitsPerSample)
        {
            case G726_16:
                stAoSetAdecConfig.stAdecG726Cfg.eG726Mode = E_MI_AUDIO_G726_MODE_16;
                break;
            case G726_24:
                stAoSetAdecConfig.stAdecG726Cfg.eG726Mode = E_MI_AUDIO_G726_MODE_24;
                break;
            case G726_32:
                stAoSetAdecConfig.stAdecG726Cfg.eG726Mode = E_MI_AUDIO_G726_MODE_32;
                break;
            case G726_40:
                stAoSetAdecConfig.stAdecG726Cfg.eG726Mode = E_MI_AUDIO_G726_MODE_40;
                break;
            default:
                printf("G726 Mode Error:%d\n", stWavHeaderInput.wave.wBitsPerSample);
                return -1;
        }
    }

    stAoSetVqeConfig.bAgcOpen = bAoEnableAgc;
    stAoSetVqeConfig.bAnrOpen = bAoEnableNr;
    stAoSetVqeConfig.bEqOpen = bAoEnableEq;
    stAoSetVqeConfig.bHpfOpen = bAoEnableHpf;
    stAoSetVqeConfig.s32FrameSample = 128;
    stAoSetVqeConfig.s32WorkSampleRate = bAoEnableResample == TRUE ? eAoOutSampleRate : eAoInSampleRate;
    memcpy(&stAoSetVqeConfig.stAgcCfg, &stAgcCfg, sizeof(MI_AUDIO_AgcConfig_t));
    memcpy(&stAoSetVqeConfig.stAnrCfg, &stAnrCfg, sizeof(MI_AUDIO_AnrConfig_t));
    memcpy(&stAoSetVqeConfig.stEqCfg, &stEqCfg, sizeof(MI_AUDIO_EqConfig_t));
    memcpy(&stAoSetVqeConfig.stHpfCfg, &stHpfCfg, sizeof(MI_AUDIO_HpfConfig_t));

    ExecFuncNoExit(MI_AO_SetPubAttr(AoDevId, &stAoSetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AO_GetPubAttr(AoDevId, &stAoGetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AO_Enable(AoDevId), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AO_EnableChn(AoDevId, AoChn), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto DISABLE_DEVICE;
    }

    if (bAoEnableAdec)
    {
        ExecFuncNoExit(MI_AO_SetAdecAttr(AoDevId, AoChn, &stAoSetAdecConfig), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

        ExecFuncNoExit(MI_AO_GetAdecAttr(AoDevId, AoChn, &stAoGetAdecConfig), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

        ExecFuncNoExit(MI_AO_EnableAdec(AoDevId, AoChn), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

	    bInitAdec = TRUE;
    }

    if(bAoEnableResample)
    {
        ExecFuncNoExit(MI_AO_EnableReSmp(AoDevId, AoChn, eAoInSampleRate), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

	    bInitRes = TRUE;
    }

    if(bAoEnableVqe)
    {
        ExecFuncNoExit(MI_AO_SetVqeAttr(AoDevId, AoChn, &stAoSetVqeConfig), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

        ExecFuncNoExit(MI_AO_GetVqeAttr(AoDevId, AoChn, &stAoGetVqeConfig), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

        ExecFuncNoExit(MI_AO_EnableVqe(AoDevId, AoChn), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
	    {
			goto DISABLE_ALG;
	    }

	    bInitVqe = TRUE;
    }

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

    if (bAoEnableResample)
    {
        s32NeedSize = s32NeedSize * eAoInSampleRate / eAoOutSampleRate;
        if (s32NeedSize > MI_AUDIO_SAMPLE_PER_FRAME * 4)
        {
            s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 4;
        }
    }

    if (bAoEnableAdec)
    {
        if (stAoSetAdecConfig.eAdecType == E_MI_AUDIO_ADEC_TYPE_G711A
            || stAoSetAdecConfig.eAdecType == E_MI_AUDIO_ADEC_TYPE_G711U)
        {
            s32NeedSize /= 2;
        }
        else if (stAoSetAdecConfig.eAdecType == E_MI_AUDIO_ADEC_TYPE_G726)
        {
            switch (stAoSetAdecConfig.stAdecG726Cfg.eG726Mode)
            {
                case E_MI_AUDIO_G726_MODE_16:
                    s32NeedSize = s32NeedSize * 2 / 16;
                    break;
                case E_MI_AUDIO_G726_MODE_24:
                    s32NeedSize = s32NeedSize * 3 / 16;
                    break;
                case E_MI_AUDIO_G726_MODE_32:
                    s32NeedSize = s32NeedSize * 4 / 16;
                    break;
                case E_MI_AUDIO_G726_MODE_40:
                    s32NeedSize = s32NeedSize * 5 / 16;
                    break;
                default:
                    assert(0);
                    break;
            }
        }
    }

    s32NeedSize = s32NeedSize / (stAoSetAttr.u32ChnCnt * 2 * 1) * (stAoSetAttr.u32ChnCnt * 2 * 1);
    pthread_create(&Aotid, NULL, aoSendFrame, NULL);
    printf("create ao thread.\n");
	return 0;

DISABLE_ALG:
	if (bAoEnableAdec && bInitAdec)
    {
        ExecFunc(MI_AO_DisableAdec(AoDevId, AoChn), MI_SUCCESS);
    }

    if(bAoEnableResample && bInitRes)
    {
        ExecFunc(MI_AO_DisableReSmp(AoDevId, AoChn), MI_SUCCESS);
    }

    if(bAoEnableVqe && bInitVqe)
    {
        ExecFunc(MI_AO_DisableVqe(AoDevId, AoChn), MI_SUCCESS);
    }

DISABLE_CHANNEL:
	ExecFunc(MI_AO_DisableChn(AoDevId, AoChn), MI_SUCCESS);
DISABLE_DEVICE:
	ExecFunc(MI_AO_Disable(AoDevId), MI_SUCCESS);
ERROR_RETURN:
	return s32Ret;
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

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        if(bAiEnableAed)
        {
            ExecFunc(MI_AI_DisableAed(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if (bAiEnableAenc)
        {
            ExecFunc(MI_AI_DisableAenc(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if(bAiEnableResample)
        {
            ExecFunc(MI_AI_DisableReSmp(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if(bAiEnableVqe)
        {
            ExecFunc(MI_AI_DisableVqe(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

		if (bAiEnableSsl)
		{
			ExecFunc(MI_AI_DisableSsl(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

		if (bAiEnableBf)
		{
			ExecFunc(MI_AI_DisableBf(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

        ExecFunc(MI_AI_DisableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
    }
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);
    return 0;
}

MI_S32 deinitAo(void)
{
    if (bAoEnableAdec)
    {
        ExecFunc(MI_AO_DisableAdec(AoDevId, AoChn), MI_SUCCESS);
    }

    if(bAoEnableResample)
    {
        ExecFunc(MI_AO_DisableReSmp(AoDevId, AoChn), MI_SUCCESS);
    }

    if(bAoEnableVqe)
    {
        ExecFunc(MI_AO_DisableVqe(AoDevId, AoChn), MI_SUCCESS);
    }

    ExecFunc(MI_AO_DisableChn(AoDevId, AoChn), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(AoDevId), MI_SUCCESS);

#if defined(HDMI)
    if ((AO_DEV_HDMI == AoDevId) || (AO_DEV_HDMI_AND_LINEOUT == AoDevId))
    {
        ExecFunc(MI_HDMI_Stop(E_MI_HDMI_ID_0), MI_SUCCESS);
        ExecFunc(MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, TRUE), MI_SUCCESS);
        ExecFunc(MI_HDMI_Close(E_MI_HDMI_ID_0), MI_SUCCESS);
        ExecFunc(MI_HDMI_DeInit(), MI_SUCCESS);
    }
#endif
    return 0;
}

int main (int argc, char *argv[])
{
    MI_S32      s32Ret = MI_SUCCESS;
    MI_S32      s32Opt = 0;
    MI_BOOL     bCheckPass = FALSE;
    MI_U32      u32ChnIdx;
    MI_U32      u32ChnCnt;

    // parsing command line
    while ((s32Opt = getopt(argc, argv, "t:I::o:h::g::e::n::r:a:A::c:b::B::v:d:s:S::F::C:m:M:O::i:H::G::E::N::R:V:D:w:W::")) != -1)
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
                    pu8AiOutputPath = optarg;
                }
                break;

            // enable Ai hpf
            case 'h':
                {
                    bAiEnableVqe = TRUE;
                    bAiEnableHpf = TRUE;
                }
                break;

            // enable Ai nr
            case'n':
                {
                    bAiEnableVqe = TRUE;
                    bAiEnableNr = TRUE;
                }
                break;

            // enable Ai agc
            case'g':
                {
                    bAiEnableVqe = TRUE;
                    bAiEnableAgc = TRUE;
                }
                break;

            // enable Ai Eq
            case 'e':
                {
                    bAiEnableVqe = TRUE;
                    bAiEnableEq = TRUE;
                }
                break;

            // set Ai Resample rate
            case 'r':
                {
                    bAiEnableResample = TRUE;
                    eAiOutputResampleRate = (MI_AUDIO_SampleRate_e)atoi(optarg);
                }
                break;

            // set Ai Aenc Type
            case 'a':
                {
                    bAiEnableAenc = TRUE;
                    if (!strncmp(optarg, "g711a", 5))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G711A;
                    }
                    else if (!strncmp(optarg, "g711u", 5))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G711U;
                    }
                    else if (!strncmp(optarg, "g726_16", 7))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G726;
                        eAiAencG726Mode = E_MI_AUDIO_G726_MODE_16;
                    }
                    else if (!strncmp(optarg, "g726_24", 7))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G726;
                        eAiAencG726Mode = E_MI_AUDIO_G726_MODE_24;
                    }
                    else if (!strncmp(optarg, "g726_32", 7))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G726;
                        eAiAencG726Mode = E_MI_AUDIO_G726_MODE_32;
                    }
                    else if (!strncmp(optarg, "g726_40", 7))
                    {
                        eAiAencType = E_MI_AUDIO_AENC_TYPE_G726;
                        eAiAencG726Mode = E_MI_AUDIO_G726_MODE_40;
                    }
                    else
                    {
                        printf("Aenc Type Error\n");
                        return -1;
                    }
                }
                break;

            // enable Ai Aed
            case 'A':
                {
                    bAiEnableAed = TRUE;
                }
                break;

            // set Ai chn num
            case 'c':
                {
                    u32AiChnCnt = atoi(optarg);
                }
                break;

            // enable Ai sw AEC
            case 'b':
                {
                    bAiEnableVqe = TRUE;
                    bAiEnableAec = TRUE;
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

            case 'M':
            	{
					u32MicDistance = (MI_U32)atoi(optarg);
            	}
            	break;

            case 'S':
            	{
					bAiEnableSsl = TRUE;
            	}
            	break;

           	case 'F':
           		{
					bAiEnableBf = TRUE;
           		}
           		break;

            case 'C':
                {
                    bAiSetBfDoa = TRUE;
                    s32AiBfDoa = (MI_S32)atoi(optarg);
                }
                break;

           	case 'w':
           		{
					u32VqeWorkingSampleRate = (MI_U32)atoi(optarg);
					if ((E_MI_AUDIO_SAMPLE_RATE_8000 != u32VqeWorkingSampleRate)
						&& (E_MI_AUDIO_SAMPLE_RATE_16000 != u32VqeWorkingSampleRate)
						&& (E_MI_AUDIO_SAMPLE_RATE_32000 != u32VqeWorkingSampleRate)
						&& (E_MI_AUDIO_SAMPLE_RATE_48000 != u32VqeWorkingSampleRate))
					{
						printf("Illegal Ai vqe working sample rate!!!\n");
						return -1;
					}
				}
				break;

            case 'W':
                {
                    bAiDumpPcmData = TRUE;
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
                    pu8AoInputPath = optarg;
                }
                break;

            // enable ao hpf
            case 'H':
                {
                    bAoEnableVqe = TRUE;
                    bAoEnableHpf = TRUE;
                }
                break;

            // enable ao agc
            case 'G':
                {
                    bAoEnableVqe = TRUE;
                    bAoEnableAgc = TRUE;
                }
                break;

            // enable ao eq
            case 'E':
                {
                    bAoEnableVqe = TRUE;
                    bAoEnableEq = TRUE;
                }
                break;

            // enable ao nr
            case 'N':
                {
                    bAoEnableVqe = TRUE;
                    bAoEnableNr = TRUE;
                }
                break;

            // set Ao Resample rate
            case 'R':
                {
                    bAoEnableResample = TRUE;
                    eAoOutSampleRate = (MI_AUDIO_SampleRate_e)atoi(optarg);
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

            case '?':
                if(optopt == 'o')
                {
                    printf("Missing Ai output file path, please -o 'output file path' \n");
                }
                else if(optopt == 'r')
                {
                    printf("Missing Ai output sample of resample, please -r 'output sample rate' \n");
                }
                else if(optopt == 'a')
                {
                    printf("Missing Ai aenc type, please -a 'aenc type' \n");
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
                else if (optopt == 'M')
                {
					printf("Missing Ai mic distance, please -M 'Ai mic distance' \n");
                }
                else if (optopt == 'C')
                {
					printf("Missing Ai Bf doa, please -C 'Ai Bf doa' \n");
                }
                else if (optopt == 'w')
                {
					printf("Missing Ai vqe working sample rate, please -M 'Ai vqe working sample rate' \n");
                }
                else if(optopt == 'i')
                {
                    printf("Missing Ao input file path, please -i 'input file path' \n");
                }
                else if (optopt == 'R')
                {
                    printf("Missing Ao input sample of resample, please -r 'input sample rate'  \n");
                }
                else if(optopt == 'V')
                {
                    printf("Missing Ao volume, pleae -V 'volume' \n");
                }
                else if(optopt == 'D')
                {
                    printf("Missing Ao device ID, please -d 'Ao device id' \n");
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
