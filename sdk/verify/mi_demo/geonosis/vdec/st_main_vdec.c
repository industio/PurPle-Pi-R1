#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_divp.h"

#include "st_hdmi.h"
#include "st_common.h"
#include "st_disp.h"
#include "st_fb.h"
#include "st_divp.h"


typedef struct
{
    pthread_t pt;
    MI_VDEC_CHN vdecChn;
    char szFileName[64];
    MI_BOOL bRunFlag;
} VDEC_Thread_Args_t;

static MI_BOOL g_bExit = FALSE;
static MI_BOOL g_subExit = FALSE;
static MI_BOOL g_PushEsExit = FALSE;
static MI_BOOL g_bScaleDownByDivp[MAX_CHN_NUM];

static MI_U32 g_u32CaseIndex = 0;
static MI_U32 g_u32SubCaseIndex = 0;
static MI_U32 g_u32LastSubCaseIndex = 0;
static MI_U32 g_u32CurSubCaseIndex = 0;
VDEC_Thread_Args_t g_stVdecThreadArgs[MAX_CHN_NUM];
ST_DispoutTiming_e e_curDispTiming;

#define DISP_XPOS_ALIGN     2
#define DISP_YPOS_ALIGN     2
#define DISP_WIDTH_ALIGN    2
#define DISP_HEIGHT_ALIGN   2

#define DIVP_WIDTH_ALIGN    2
#define DIVP_HEIGHT_ALIGN   2
#define DIVP_OUT_WIDTH_ALIGN    16

#define VDEC_WIDTH_ALIGN    2
#define VDEC_HEIGHT_ALIGN   2

#define VENC_WIDTH_ALIGN   16
#define VENC_HEIGHT_ALIGN  2

#define VDEC_MAX_SCALEDOWN_TIMES 8
#define TWO_STEP_SCALEDOWN_VDEC_TIMES 4

#define DIVP_MAX_SIZE     1920

#define ADD_HEADER_ES

#define CAPTURE_FULL_SIZE   1

//#define PLAY_WITH_ARGS
#ifdef PLAY_WITH_ARGS
char EsFileName[64] = "720P25.es";
MI_U32 g_u32VdecWidth = 1920;
MI_U32 g_u32VdecHeight = 1080;
MI_VDEC_DPB_BufMode_e g_u32RefFrame = 1;
MI_U32 g_u32DispWin = 1;
MI_VDEC_CodecType_e videoType = 0;
ST_DispoutTiming_e dispTiming = E_ST_TIMING_1080P_60;
#endif

#define _IS_OVER_VDEC_MAX_SCALEDOWN_TIMES(picSize, showSize) ((picSize) > (showSize)*VDEC_MAX_SCALEDOWN_TIMES)

ST_CaseDesc_t g_stCaseDesc[] =
{
    {
        .stDesc =
        {
            .u32CaseIndex = 0,
            .szDesc = "1x1080P@30 H264 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 15,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P@50 timing",
                .eDispoutTiming = E_ST_TIMING_720P_50,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 720P@60 timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1080P@50 timing",
                .eDispoutTiming = E_ST_TIMING_1080P_50,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P@60 timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "change to 1600x1200 timing",
                .eDispoutTiming = E_ST_TIMING_1600x1200_60,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "change to 1280x800 timing",
                .eDispoutTiming = E_ST_TIMING_1280x800_60,
            },
            {
                .u32CaseIndex = 9,
                .szDesc = "change to 1366x768 timing",
                .eDispoutTiming = E_ST_TIMING_1366x768_60,
            },
            {
                .u32CaseIndex = 10,
                .szDesc = "change to 1680x1050 timing",
                .eDispoutTiming = E_ST_TIMING_1680x1050_60,
            },
            {
                .u32CaseIndex = 11,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 12,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 13,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 14,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 1,
            .szDesc = "1x1080P@30 H265 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 2,
            .szDesc = "1xD1 H264 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 3,
            .szDesc = "1xD1 H265 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 4,
            .szDesc = "4x720P@30 H264 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 9,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1600x1200 timing",
                .eDispoutTiming = E_ST_TIMING_1600x1200_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 5,
            .szDesc = "4x720P@30 H265 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 6,
            .szDesc = "4x1080P@30 H264 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 7,
            .szDesc = "4x1080P@30 H265 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 8,
            .szDesc = "2x1080P@30 H264 + 2x1080P@30 H265 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 9,
            .szDesc = "4xD1 H264 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 10,
            .szDesc = "4xD1 H265 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 11,
            .szDesc = "1x1080P30 H264 + 7xD1 H264 Decode",
            .u32WndNum = 8,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 8 channel",
                .u32WndNum = 8,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 12,
            .szDesc = "1x1080P30 H265 + 7xD1 H265 Decode",
            .u32WndNum = 8,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 8 channel",
                .u32WndNum = 8,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 13,
            .szDesc = "1x1080P30 H264 + 7xD1 H265 Decode",
            .u32WndNum = 8,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 8 channel",
                .u32WndNum = 8,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 14,
            .szDesc = "9xD1 H264 Decode",
            .u32WndNum = 9,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 11,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 9 channel",
                .u32WndNum = 9,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 9,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 10,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 8,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 15,
            .szDesc = "9xD1 H265 Decode",
            .u32WndNum = 9,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 8,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 9 channel",
                .u32WndNum = 9,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 8,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 16,
            .szDesc = "1x1080P@30 H264 Decode + PIP",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .bPIP = TRUE,
        .u32SubCaseNum = 10,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1280x1024 timing",
                .eDispoutTiming = E_ST_TIMING_1280x1024_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1024x768 timing",
                .eDispoutTiming = E_ST_TIMING_1024x768_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 1600x1200 timing",
                .eDispoutTiming = E_ST_TIMING_1600x1200_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 9,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 17,
            .szDesc = "1x1080P@30 H264 Decode + 5 x D1 H264 Decode",
            .u32WndNum = 6,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 18,
            .szDesc = "1x720P@30 H264 Decode + 5 x D1 H264 Decode",
            .u32WndNum = 6,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_25_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 19,
            .szDesc = "1x1080P@25 H264 Decode + 5xVGA(640x480) H264 Decode",
            .u32WndNum = 6,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_25_FILE,
                        .u32MaxWidth = 1920,
                        .u32MaxHeight = 1080,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 20,
            .szDesc = "16xD1 H264 Decode",
            .u32WndNum = 16,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 10,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 9 channel",
                .u32WndNum = 9,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "show 16 channel",
                .u32WndNum = 16,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 9,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 8,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 9,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 10,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 11,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 12,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 13,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 14,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 15,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 21,
            .szDesc = "16xD1 H265 Decode",
            .u32WndNum = 16,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 10,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "show 9 channel",
                .u32WndNum = 9,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "show 16 channel",
                .u32WndNum = 16,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "change to 1440x900 timing",
                .eDispoutTiming = E_ST_TIMING_1440x900_60,
            },
            {
                .u32CaseIndex = 7,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 8,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 9,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 4,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 5,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 6,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 7,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 8,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 9,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 10,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 11,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 12,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 13,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 14,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 15,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_25_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 576,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 22,
            .szDesc = "4xSIZE(640x360) H264 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_AA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 360,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_AA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 360,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_AA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 360,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_AA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 360,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 23,
            .szDesc = "1x720P H264 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_25_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 24,
            .szDesc = "1x720P H265 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 5,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_25_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 25,
            .szDesc = "4chn 2592x1944 H264 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_5MP_H264_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_5MP_H264_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_5MP_H264_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_5MP_H264_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 26,
            .szDesc = "4chn 2592x1944 H265 Decode",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "show 4 channel",
                .u32WndNum = 4,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_5MP_H265_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_5MP_H265_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_5MP_H265_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_5MP_H265_FILE,
                        .u32MaxWidth = 2592,
                        .u32MaxHeight = 1944,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 27,
            .szDesc = "1x8MP(3840x2160) H264 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_8MP_H264_25_FILE,
                        .u32MaxWidth = 3840,
                        .u32MaxHeight = 2160,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 28,
            .szDesc = "1x8MP(3840x2160) H265 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 6,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_8MP_H265_25_FILE,
                        .u32MaxWidth = 3840,
                        .u32MaxHeight = 2160,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 29,
            .szDesc = "1x4MP(2560x1440) H264 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 7,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 6,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_4MP_H264_25_FILE,
                        .u32MaxWidth = 2560,
                        .u32MaxHeight = 1440,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 30,
            .szDesc = "1x4MP(2560x1440) H265 Decode",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 6,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "show 1 channel",
                .u32WndNum = 1,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "test region",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 4,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 5,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_4MP_H265_25_FILE,
                        .u32MaxWidth = 2560,
                        .u32MaxHeight = 1440,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 31,
            .szDesc = "1 x 4K@30 H264 Decode (corridor)",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_4K_H264_CORRIDOR_FILE,
                        .u32MaxWidth = 2160,
                        .u32MaxHeight = 3840,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 32,
            .szDesc = "3xD1@30 H264 Decode (corridor)",
            .u32WndNum = 3,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_D1_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 33,
            .szDesc = "3x720P@30 H264 Decode (corridor)",
            .u32WndNum = 3,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 34,
            .szDesc = "1x1080P@30 H264 Decode (corridor) 16 win",
            .u32WndNum = 16,
        },
        .eDispoutTiming = E_ST_TIMING_1024x768_60,
        .u32SubCaseNum = 3,
        .u32ShowWndNum = 1,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_1080P_H264_30_CORRIDOR_FILE,
                        .u32MaxWidth = 1080,
                        .u32MaxHeight = 1920,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 35,
            .szDesc = "1 x 4K@30 H265 Decode (corridor)",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_4K_H265_CORRIDOR_FILE,
                        .u32MaxWidth = 2160,
                        .u32MaxHeight = 3840,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 36,
            .szDesc = "3xD1@30 H265 Decode (corridor)",
            .u32WndNum = 3,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_D1_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 576,
                        .u32MaxHeight = 720,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 37,
            .szDesc = "3x720P@30 H265 Decode (corridor)",
            .u32WndNum = 3,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 720,
                        .u32MaxHeight = 1280,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 38,
            .szDesc = "3x1080P@30 H265 Decode (corridor)",
            .u32WndNum = 3,
        },
        .eDispoutTiming = E_ST_TIMING_1280x1024_60,
        .u32SubCaseNum = 3,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "zoom",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
           	{
           	    .u32CaseIndex = 2,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 1080,
                        .u32MaxHeight = 1920,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 1080,
                        .u32MaxHeight = 1920,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_1080P_H265_30_CORRIDOR_FILE,
                        .u32MaxWidth = 1080,
                        .u32MaxHeight = 1920,
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 39,
            .szDesc = "1x720P@30 H265 + 3x720P@30 H264",
            .u32WndNum = 4,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 4,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_720P_H265_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 1,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 2,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 3,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_720P_H264_30_FILE,
                        .u32MaxWidth = 1280,
                        .u32MaxHeight = 720,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 40,
            .szDesc = "1xVGA@30 H264",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 4,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H264,
                        .szFilePath = PREFIX_PATH ST_VGA_H264_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .u32CaseIndex = 41,
            .szDesc = "1xVGA@30 H265",
            .u32WndNum = 1,
        },
        .eDispoutTiming = E_ST_TIMING_1080P_60,
        .u32SubCaseNum = 4,
        .stSubDesc =
        {
            {
                .u32CaseIndex = 0,
                .szDesc = "change to 720P timing",
                .eDispoutTiming = E_ST_TIMING_720P_60,
            },
            {
                .u32CaseIndex = 1,
                .szDesc = "change to 1080P timing",
                .eDispoutTiming = E_ST_TIMING_1080P_60,
            },
            {
                .u32CaseIndex = 2,
                .szDesc = "Capture JPEG",
                .eDispoutTiming = 0,
            },
            {
                .u32CaseIndex = 3,
                .szDesc = "exit",
                .eDispoutTiming = 0,
            },
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VDEC_CHN,
                .uChnArg =
                {
                    .stVdecChnArg =
                    {
                        .u32Chn = 0,
                        .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
                        .szFilePath = PREFIX_PATH ST_VGA_H265_25_FILE,
                        .u32MaxWidth = 640,
                        .u32MaxHeight = 480,
                    },
                }
            },
        },
    }
};

//==============================================================================
FILE *g_pStreamFile[32] = {NULL};

typedef struct
{
    int startcodeprefix_len;
    unsigned int len;
    unsigned int max_size;
    char *buf;
    unsigned short lost_packets;
} NALU_t;

static int info2[32] = {0}, info3[32] = {0};

static int FindStartCode2 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    {
        printf("AllocNALU: n");
        exit(0);
    }
    n->max_size=buffersize;
    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
    {
        free (n);
        printf ("AllocNALU: n->buf");
        exit(0);
    }
    return n;
}

void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
}

int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn)
{
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
        printf ("GetAnnexbNALU: Could not allocate Buf memory\n");
    nalu->startcodeprefix_len=3;
    if (3 != fread (Buf, 1, 3, g_pStreamFile[chn]))
    {
        free(Buf);
        return 0;
    }
    info2[chn] = FindStartCode2 (Buf);
    if(info2[chn] != 1)
    {
        if(1 != fread(Buf+3, 1, 1, g_pStreamFile[chn]))
        {
            free(Buf);
            return 0;
        }
        info3[chn] = FindStartCode3 (Buf);
        if (info3[chn] != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2[chn] = 0;
    info3[chn] = 0;
    while (!StartCodeFound)
    {
        if (feof (g_pStreamFile[chn]))
        {
            nalu->len = (pos-1)-nalu->startcodeprefix_len;
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            free(Buf);
            fseek(g_pStreamFile[chn], 0, 0);
            return pos-1;
        }
        Buf[pos++] = fgetc (g_pStreamFile[chn]);
        info3[chn] = FindStartCode3(&Buf[pos-4]);
        if(info3[chn] != 1)
            info2[chn] = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2[chn] == 1 || info3[chn] == 1);
    }
    rewind = (info3[chn] == 1) ? -4 : -3;
    if (0 != fseek (g_pStreamFile[chn], rewind, SEEK_CUR))
    {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }
    nalu->len = (pos+rewind);
    memcpy (nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos+rewind);
}

void dump(NALU_t *n)
{
    if (!n)
        return;
    //printf(" len: %d  ", n->len);
    //printf("nal_unit_type: %x\n", n->nal_unit_type);
}

void ST_VdecUsage(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32Size = ARRAY_SIZE(g_stCaseDesc);
    MI_U32 i = 0;

    for (i = 0; i < u32Size; i ++)
    {
        printf("%d)\t %s\n", pstCaseDesc[i].stDesc.u32CaseIndex + 1, pstCaseDesc[i].stDesc.szDesc);
    }

    printf("print twice Enter to exit\n");
}

void ST_CaseUsage(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 i = 0;

    if (u32CaseIndex < 0 || u32CaseIndex > u32CaseSize)
    {
        return;
    }

    for (i = 0; i < pstCaseDesc[u32CaseIndex].u32SubCaseNum; i ++)
    {
        printf("\t%d) %s\n", pstCaseDesc[u32CaseIndex].stSubDesc[i].u32CaseIndex + 1,
            pstCaseDesc[u32CaseIndex].stSubDesc[i].szDesc);
    }
}

void *ST_VdecSendStream(void *args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hSysBuf;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_CHN vdecChn = 0;
    MI_S32 s32TimeOutMs = 20, s32ChannelId = 0, s32TempHeight = 0;
    MI_S32 s32Ms = 30;
    MI_BOOL bVdecChnEnable;
    MI_U16 u16Times = 20000;

    MI_S32 s32ReadCnt = 0;
    FILE *readfp = NULL;
    MI_U8 *pu8Buf = NULL;
    MI_S32 s32Len = 0;
    MI_U32 u32FrameLen = 0;
    MI_U64 u64Pts = 0;
    MI_U8 au8Header[32] = {0};
    MI_U32 u32Pos = 0;
    MI_VDEC_ChnStat_t stChnStat;
    MI_VDEC_VideoStream_t stVdecStream;

    MI_U32 u32FpBackLen = 0; // if send stream failed, file pointer back length

    VDEC_Thread_Args_t *pstArgs = (VDEC_Thread_Args_t *)args;

    char tname[32];
    memset(tname, 0, 32);

#ifndef ADD_HEADER_ES
    NALU_t *n;
    n = AllocNALU(2000000);
#endif

    vdecChn = pstArgs->vdecChn;
    snprintf(tname, 32, "push_t_%u", vdecChn);
    prctl(PR_SET_NAME, tname);

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = vdecChn;//0 1 2 3
    stChnPort.u32PortId = 0;

    readfp = fopen(pstArgs->szFileName, "rb"); //ES
    if (!readfp)
    {
        ST_ERR("Open %s failed!\n", pstArgs->szFileName);
        return NULL;
    }
    else
    {
        g_pStreamFile[vdecChn] = readfp;
    }

    printf("open %s success, vdec chn:%d\n", pstArgs->szFileName, vdecChn);
    // s32Ms = _stTestParam.stChannelInfo[s32VoChannel].s32PushDataMs;
    // bVdecChnEnable = _stTestParam.stChannelInfo[0].bVdecChnEnable;

    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufConf.u64TargetPts = 0;
    pu8Buf = malloc(NALU_PACKET_SIZE);

    s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 4);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_SYS_SetChnOutputPortDepth error, %X\n", s32Ret);
        return NULL;
    }

    //stChnPort.eModId = E_MI_MODULE_ID_DIVP;
    //STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5));

    s32Ms = 1;
    //printf("----------------------%d------------------\n", stChnPort.u32ChnId);
    while (!g_PushEsExit)
    {
        //usleep(s32Ms * 1000);//33
        if (pstArgs->bRunFlag == FALSE)
        {
		    usleep(10 * 1000);
            continue;
        }

#ifdef ADD_HEADER_ES
        memset(au8Header, 0, 16);
        u32Pos = fseek(readfp, 0, SEEK_CUR);
        s32Len = fread(au8Header, 1, 16, readfp);
        if(s32Len <= 0)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }

        u32FrameLen = MI_U32VALUE(au8Header, 4);
        // printf("vdecChn:%d, u32FrameLen:%d, %d\n", vdecChn, u32FrameLen, NALU_PACKET_SIZE);
        if(u32FrameLen > NALU_PACKET_SIZE)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }
        s32Len = fread(pu8Buf, 1, u32FrameLen, readfp);
        if(s32Len <= 0)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }

        stVdecStream.pu8Addr = pu8Buf;
        stVdecStream.u32Len = s32Len;
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len + 16; //back length
#else
        GetAnnexbNALU(n, vdecChn);
        dump(n);
        stVdecStream.pu8Addr = (MI_U8 *)n->buf;
        stVdecStream.u32Len = n->len;
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len; //back length
#endif

        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && 0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4])
        {
            usleep(s32Ms * 1000);
        }

        if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
        {
            //ST_ERR("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", vdecChn, s32Ret);
            fseek(readfp, - u32FpBackLen, SEEK_CUR);
        }

        u64Pts = u64Pts + ST_Sys_GetPts(30);


        if (0 == (s32ReadCnt++ % 30))
            ;// printf("vdec(%d) push buf cnt (%d)...\n", s32VoChannel, s32ReadCnt)
            ;//printf("###### ==> Chn(%d) push frame(%d) Frame Dec:%d  Len:%d\n", s32VoChannel, s32ReadCnt, stChnStat.u32DecodeStreamFrames, u32Len);
    }
    printf("\n\n");
    usleep(300000);
    free(pu8Buf);
    fclose(readfp);

    printf("End----------------------%d------------------End\n", stChnPort.u32ChnId);

    return NULL;
}

int ST_TestRegion()
{
    pid_t fPid;

    fPid = fork();
    if (fPid < 0)
    {
        printf("error in fork!");
    }
    else if(fPid == 0)
    {
        if(execl("mdb", NULL, NULL) < 0)
        {
            perror("execlp error!");
            return -1 ;
        }
    }
    waitpid(fPid, NULL, 0);

    return 0;
}

int ST_ChangeDisplayTiming(ST_DispoutTiming_e eChangeTiming)
{
#if 1
    if(e_curDispTiming == eChangeTiming)
    {
        printf("the same timing, skip\n");
        return 0;
    }
    ST_SubExit();
    ST_VdecH26X(eChangeTiming);
#else
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32CurSubCaseIndex = g_u32CurSubCaseIndex;
    MI_U32 u32LastSubCaseIndex = g_u32LastSubCaseIndex;
    ST_DispoutTiming_e eLastDispoutTiming = E_ST_TIMING_MAX;
    ST_DispoutTiming_e eCurDispoutTiming = E_ST_TIMING_MAX;
    MI_S32 s32LastHdmiTiming, s32CurHdmiTiming;
    MI_S32 s32LastDispTiming, s32CurDispTiming;
    MI_U16 u16LastDispWidth = 0, u16LastDispHeight = 0;
    MI_U16 u16CurDispWidth = 0, u16CurDispHeight = 0;
    MI_U32 u32CurWndNum = 0;
    MI_U32 u32TotalWnd = 0;
    MI_U32 i = 0;
    MI_U32 u32Square = 0;
    ST_Sys_BindInfo_t stBindInfo;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_HDMI_Attr_t stHdmiAttr;
    MI_DISP_PubAttr_t stPubAttr;

    memset(&stHdmiAttr, 0, sizeof(MI_HDMI_Attr_t));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    if (u32CurSubCaseIndex < 0 || u32LastSubCaseIndex < 0)
    {
        printf("error index\n");
        return 0;
    }

    eCurDispoutTiming = pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].eDispoutTiming;
    eLastDispoutTiming = pstCaseDesc[u32CaseIndex].eDispoutTiming;

    if (eCurDispoutTiming == eLastDispoutTiming)
    {
        printf("the same timing, skip\n");
        return 0;
    }

    u32CurWndNum = pstCaseDesc[u32CaseIndex].u32ShowWndNum;

    STCHECKRESULT(ST_GetTimingInfo(eCurDispoutTiming,
                &s32CurHdmiTiming, &s32CurDispTiming, &u16CurDispWidth, &u16CurDispHeight));

    STCHECKRESULT(ST_GetTimingInfo(eLastDispoutTiming,
                &s32LastHdmiTiming, &s32LastDispTiming, &u16LastDispWidth, &u16LastDispHeight));

    printf("change from %dx%d to %dx%d\n", u16LastDispWidth, u16LastDispHeight, u16CurDispWidth,
                u16CurDispHeight);

    /*
    (1) stop HDMI
    (2) stop DISP and disable input port
    (3) start disp
    (4) start HDMI
    (5) set disp input port attribute and enable
    */

    // stop hdmi
    ExecFunc(MI_HDMI_Stop(E_MI_HDMI_ID_0), MI_SUCCESS);

    // stop disp dev and disable input port
    ExecFunc(MI_DISP_DeviceDetach(ST_DISP_DEV0, ST_DISP_DEV1), MI_SUCCESS);
    ExecFunc(MI_DISP_Disable(ST_DISP_DEV1), MI_SUCCESS);
    ExecFunc(MI_DISP_Disable(ST_DISP_DEV0), MI_SUCCESS);

    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    stPubAttr.u32BgColor = YUYV_BLACK;
    stPubAttr.eIntfSync = s32CurDispTiming;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(ST_DISP_DEV0, &stPubAttr), MI_SUCCESS);

    ExecFunc(MI_DISP_GetVideoLayerAttr(ST_DISP_LAYER0, &stLayerAttr), MI_SUCCESS);
    stLayerAttr.stVidLayerDispWin.u16Width = u16CurDispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16CurDispHeight;
    //stLayerAttr.stVidLayerSize.u16Width = u16CurDispWidth;
    //stLayerAttr.stVidLayerSize.u16Height = u16CurDispHeight;
    ExecFunc(MI_DISP_SetVideoLayerAttr(ST_DISP_LAYER0, &stLayerAttr), MI_SUCCESS);

    ExecFunc(MI_DISP_Enable(ST_DISP_DEV0), MI_SUCCESS);

    // start disp
    if ((u16CurDispWidth > 1920) ||
        (u16CurDispHeight > 1080))
    {
        ST_DBG("VGA not support this timing:%d, sizeo: %dx%d\n", s32CurDispTiming,
            u16CurDispWidth, u16CurDispHeight);
    }
    else
    {
        memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
        stPubAttr.u32BgColor = YUYV_BLACK;
        stPubAttr.eIntfSync = s32CurDispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        ExecFunc(MI_DISP_SetPubAttr(ST_DISP_DEV1, &stPubAttr), MI_SUCCESS);
        ExecFunc(MI_DISP_Enable(ST_DISP_DEV1), MI_SUCCESS);
        ExecFunc(MI_DISP_DeviceAttach(ST_DISP_DEV0, ST_DISP_DEV1), MI_SUCCESS);
    }

    // start HDMI
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32CurHdmiTiming));

    pstCaseDesc[u32CaseIndex].eDispoutTiming = eCurDispoutTiming;
#endif
    g_u32LastSubCaseIndex = g_u32CurSubCaseIndex;
}

int _ST_ChangeScaleDownFromVdecToDivp(MI_U32 chn)
{
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_VDEC_OutputPortAttr_t stVdecOutAttr;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    ST_Sys_BindInfo_t stBindInfo;
    MI_U32 dispWinWidth = 0;
    MI_U32 dispWinHeight = 0;

    memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_GetChnAttr(chn, &stVdecChnAttr));

    memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
    STCHECKRESULT(MI_VDEC_GetOutputPortAttr(chn, &stVdecOutAttr));
    dispWinWidth = stVdecOutAttr.u16Width;
    dispWinHeight = stVdecOutAttr.u16Height;

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

    printf("%s %d vdec(%d)->disp(%d) unbind\n", __func__, __LINE__,chn,chn);

    stVdecOutAttr.u16Width = stVdecChnAttr.u32PicWidth > DIVP_MAX_SIZE ? DIVP_MAX_SIZE : stVdecChnAttr.u32PicWidth;
    stVdecOutAttr.u16Height = stVdecChnAttr.u32PicHeight > DIVP_MAX_SIZE ? DIVP_MAX_SIZE : stVdecChnAttr.u32PicHeight;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(chn, &stVdecOutAttr));

    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = stVdecOutAttr.u16Width;
    stDivpChnAttr.stCropRect.u16Height  = stVdecOutAttr.u16Height;
    stDivpChnAttr.u32MaxWidth           = stVdecOutAttr.u16Width;
    stDivpChnAttr.u32MaxHeight          = stVdecOutAttr.u16Height;

    ExecFunc(MI_DIVP_CreateChn(chn, &stDivpChnAttr), MI_SUCCESS);
    ExecFunc(MI_DIVP_StartChn(chn), MI_SUCCESS);

    memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
    stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stDivpOutAttr.u32Width           = ALIGN_BACK(dispWinWidth, DIVP_OUT_WIDTH_ALIGN);
    stDivpOutAttr.u32Height          = dispWinHeight;

    STCHECKRESULT(MI_DIVP_SetOutputPortAttr(chn, &stDivpOutAttr));

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;

    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    printf("%s %d vdec(%d)->divp(%d) bind\n", __func__, __LINE__, chn, chn);

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;

    printf("%s %d divp(%d)->disp(%d) bind\n", __func__, __LINE__,chn,chn);
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    g_bScaleDownByDivp[chn] = TRUE;
}

int _ST_ChangeScaleDownFromDivpToVdec(MI_U32 chn)
{
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_VDEC_OutputPortAttr_t stVdecOutAttr;
    ST_Sys_BindInfo_t stBindInfo;
    MI_U32 dispWinWidth = 0;
    MI_U32 dispWinHeight = 0;

    memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
    STCHECKRESULT(MI_DIVP_GetOutputPortAttr(chn, &stDivpOutAttr));
    dispWinWidth = stDivpOutAttr.u32Width;
    dispWinHeight = stDivpOutAttr.u32Height;

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    printf("%s %d vdec(%d)->divp(%d) unbind\n", __func__, __LINE__, chn, chn);

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    printf("%s %d divp(%d)->disp(%d) unbind\n", __func__, __LINE__, chn, chn);

    STCHECKRESULT(MI_DIVP_StopChn(chn));
    STCHECKRESULT(MI_DIVP_DestroyChn(chn));

    memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
    STCHECKRESULT(MI_VDEC_GetOutputPortAttr(chn, &stVdecOutAttr));
    stVdecOutAttr.u16Width = dispWinWidth;
    stVdecOutAttr.u16Height = dispWinHeight;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(chn, &stVdecOutAttr));

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = chn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = chn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    printf("%s %d vdec(%d)->disp(%d) bind\n", __func__, __LINE__, chn, chn);
    g_bScaleDownByDivp[chn] = FALSE;
}

int ST_SplitWindow()
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32CurSubCaseIndex = g_u32CurSubCaseIndex;
    MI_U32 u32LastSubCaseIndex = g_u32LastSubCaseIndex;
    MI_U32 u32CurWndNum = 0;
    MI_U32 u32LastWndNum = 0;
    MI_U32 i = 0;
    MI_U32 u32Square = 1;
    MI_U16 u16DispWidth = 0, u16DispHeight = 0;
    ST_Sys_BindInfo_t stBindInfo;
    MI_SYS_WindowRect_t stVpeRect;
    ST_Rect_t stDispWndRect[32] = {0};
    MI_U32 u32VideoWidth = 0;
    MI_U32 u32VideoHeight = 0;

    if (u32CurSubCaseIndex < 0 || u32LastSubCaseIndex < 0)
    {
        printf("error index\n");
        return 0;
    }

    /*
    VDEC->DIVP->DISP

    (1) stop send stream
    (2) unbind VDEC DIVP
    (3) stop divp, disable disp
    (4) set disp port attribute
    (5) start divp, enable disp
    (6) bind vdec divp
    (7) start send stream
    */

    u32CurWndNum = pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].u32WndNum;
    u32LastWndNum = pstCaseDesc[u32CaseIndex].u32ShowWndNum;

    if (u32CurWndNum == u32LastWndNum)
    {
        printf("same wnd num, skip\n");
        return 0;
    }
    else
    {
        printf("split window from %d to %d\n", u32LastWndNum, u32CurWndNum);
    }

    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    ExecFunc(MI_DISP_GetVideoLayerAttr(ST_DISP_LAYER0, &stLayerAttr), MI_SUCCESS);
    u16DispWidth = stLayerAttr.stVidLayerSize.u16Width;
    u16DispHeight = stLayerAttr.stVidLayerSize.u16Height;
    printf("%s %d, u16DispWidth:%d,u16DispHeight:%d\n", __func__, __LINE__, u16DispWidth,
        u16DispHeight);

    // stop send es stream to vdec
    for (i = 0; i < u32LastWndNum; i++)
    {
        g_stVdecThreadArgs[i].bRunFlag = FALSE;
    }

    // 1, unbind VDEC to DIVP
    for (i = 0; i < u32LastWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
        if(g_bScaleDownByDivp[i])
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d vdec(%d)->divp(%d) unbind\n", __func__, __LINE__, i, i);
        }
        else
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d vdec(%d)->disp(%d) unbind\n", __func__, __LINE__, i, i);
        }
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    // stop divp
    for (i = 0; i < u32LastWndNum; i++)
    {
        if(g_bScaleDownByDivp[i])
        {
            STCHECKRESULT(MI_DIVP_StopChn(i));
        }
        //STCHECKRESULT(MI_VDEC_StopChn(i));
    }

    // disable disp port
    for (i = 0; i < u32LastWndNum; i ++)
    {
        ExecFunc(MI_DISP_DisableInputPort(ST_DISP_LAYER0, i), MI_SUCCESS);
    }


    if (u32CurWndNum <= 1)
    {
        u32Square = 1;
    }
    else if (u32CurWndNum <= 4)
    {
        u32Square = 2;
    }
    else if (u32CurWndNum <= 9)
    {
        u32Square = 3;
    }
    else if (u32CurWndNum <= 16)
    {
        u32Square = 4;
    }
    else if (u32CurWndNum <= 25)
    {
        u32Square = 5;
    }
    else if (u32CurWndNum <= 36)
    {
        u32Square = 6;
    }

    if(u32CurWndNum == 25)
    {
        u32Square = 5;
    }

    for(i = 0; i < u32CurWndNum; i++)
    {
      stDispWndRect[i].s32X = ALIGN_BACK((u16DispWidth / u32Square) * (i % u32Square), DISP_XPOS_ALIGN);
      stDispWndRect[i].s32Y = ALIGN_BACK((u16DispHeight / u32Square) * (i / u32Square), DISP_YPOS_ALIGN);
      stDispWndRect[i].u16PicW = ALIGN_BACK(u16DispWidth / u32Square, DISP_WIDTH_ALIGN);
      stDispWndRect[i].u16PicH = ALIGN_BACK(u16DispHeight / u32Square, DISP_HEIGHT_ALIGN);
    }

#if 0
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    ExecFunc(MI_DISP_Disable(ST_DISP_DEV0), MI_SUCCESS);

    stPubAttr.u32BgColor = YUYV_BLACK;
    stPubAttr.eIntfSync = s32DispTiming;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(ST_DISP_DEV0, &stPubAttr), MI_SUCCESS);

    ExecFunc(MI_DISP_GetVideoLayerAttr(ST_DISP_LAYER0, &stLayerAttr), MI_SUCCESS);
    stLayerAttr.stVidLayerDispWin.u16Width = u16DispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u16DispHeight;
    stLayerAttr.stVidLayerSize.u16Width = u16DispWidth;
    stLayerAttr.stVidLayerSize.u16Height = u16DispHeight;
    ExecFunc(MI_DISP_SetVideoLayerAttr(ST_DISP_LAYER0, &stLayerAttr), MI_SUCCESS);
    ExecFunc(MI_DISP_Enable(ST_DISP_DEV0), MI_SUCCESS);
#endif
    // set divp outport attribute
    MI_DIVP_CHN divpChn = 0;
    MI_VDEC_OutputPortAttr_t stVdecOutAttr;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_VDEC_ChnAttr_t stVdecChnAttr;

    for (i = 0; i < u32CurWndNum; i++)
    {
        u32VideoWidth = pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.u32MaxWidth;
        u32VideoHeight= pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.u32MaxHeight;
        if(g_bScaleDownByDivp[i])
        {
            if(_IS_OVER_VDEC_MAX_SCALEDOWN_TIMES(u32VideoWidth, stDispWndRect[i].u16PicW) || _IS_OVER_VDEC_MAX_SCALEDOWN_TIMES(u32VideoHeight, stDispWndRect[i].u16PicH))
            {
                memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
                STCHECKRESULT(MI_DIVP_GetOutputPortAttr(i, &stDivpOutAttr));
                stDivpOutAttr.u32Width      = ALIGN_BACK(stDispWndRect[i].u16PicW, DIVP_OUT_WIDTH_ALIGN);
                stDivpOutAttr.u32Height     = ALIGN_BACK(stDispWndRect[i].u16PicH, DISP_HEIGHT_ALIGN);

                STCHECKRESULT(MI_DIVP_SetOutputPortAttr(i, &stDivpOutAttr));
            }
            else
            {
                memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
                stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
                stBindInfo.stSrcChnPort.u32DevId = 0;
                stBindInfo.stSrcChnPort.u32ChnId = i;
                stBindInfo.stSrcChnPort.u32PortId = 0;

                stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
                stBindInfo.stDstChnPort.u32DevId = 0;
                stBindInfo.stDstChnPort.u32ChnId = i;
                stBindInfo.stDstChnPort.u32PortId = 0;
                STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

                printf("%s %d divp(%d)->disp(%d) unbind\n", __func__, __LINE__, i, i);
                STCHECKRESULT(MI_DIVP_StopChn(i));
                STCHECKRESULT(MI_DIVP_DestroyChn(i));

                memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
                STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
                stVdecOutAttr.u16Width = stDispWndRect[i].u16PicW;
                stVdecOutAttr.u16Height = stDispWndRect[i].u16PicH;
                STCHECKRESULT(MI_VDEC_SetOutputPortAttr(i, &stVdecOutAttr));

                memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
                stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
                stBindInfo.stSrcChnPort.u32DevId = 0;
                stBindInfo.stSrcChnPort.u32ChnId = i;
                stBindInfo.stSrcChnPort.u32PortId = 0;

                stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
                stBindInfo.stDstChnPort.u32DevId = 0;
                stBindInfo.stDstChnPort.u32ChnId = i;
                stBindInfo.stDstChnPort.u32PortId = 0;
                stBindInfo.u32SrcFrmrate = 0;
                stBindInfo.u32DstFrmrate = 0;
                STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

                printf("%s %d vdec(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
                g_bScaleDownByDivp[i] = FALSE;
            }
        }
        else
        {
            if(u32VideoWidth > stDispWndRect[i].u16PicW*VDEC_MAX_SCALEDOWN_TIMES || u32VideoHeight > stDispWndRect[i].u16PicH*VDEC_MAX_SCALEDOWN_TIMES)
            {
                memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
                STCHECKRESULT(MI_VDEC_GetChnAttr(i, &stVdecChnAttr));
                memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
                STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
                stVdecOutAttr.u16Width = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN);
                stVdecOutAttr.u16Height = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN);
                STCHECKRESULT(MI_VDEC_SetOutputPortAttr(i, &stVdecOutAttr));

                memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
                stDivpChnAttr.bHorMirror            = FALSE;
                stDivpChnAttr.bVerMirror            = FALSE;
                stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
                stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
                stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
                stDivpChnAttr.stCropRect.u16X       = 0;
                stDivpChnAttr.stCropRect.u16Y       = 0;
                stDivpChnAttr.stCropRect.u16Width   = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //Vdec pic w
                stDivpChnAttr.stCropRect.u16Height  = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN); //Vdec pic h
                stDivpChnAttr.u32MaxWidth           = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //max size picture can pass
                stDivpChnAttr.u32MaxHeight          = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN);

                ExecFunc(MI_DIVP_CreateChn(i, &stDivpChnAttr), MI_SUCCESS);
                ExecFunc(MI_DIVP_StartChn(i), MI_SUCCESS);

                memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
                stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
                stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                stDivpOutAttr.u32Width           = ALIGN_BACK(stDispWndRect[i].u16PicW, DIVP_OUT_WIDTH_ALIGN);
                stDivpOutAttr.u32Height          = ALIGN_BACK(stDispWndRect[i].u16PicH, DISP_HEIGHT_ALIGN);

                STCHECKRESULT(MI_DIVP_SetOutputPortAttr(i, &stDivpOutAttr));

                memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
                stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
                stBindInfo.stSrcChnPort.u32DevId = 0;
                stBindInfo.stSrcChnPort.u32ChnId = i;
                stBindInfo.stSrcChnPort.u32PortId = 0;

                stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
                stBindInfo.stDstChnPort.u32DevId = 0;
                stBindInfo.stDstChnPort.u32ChnId = i;
                stBindInfo.stDstChnPort.u32PortId = 0;
                stBindInfo.u32SrcFrmrate = 0;
                stBindInfo.u32DstFrmrate = 0;

                printf("%s %d divp(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
                STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
                g_bScaleDownByDivp[i] = TRUE;
            }
            else
            {
                memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
                STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
                memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
                STCHECKRESULT(MI_VDEC_GetChnAttr(i, &stVdecChnAttr));
                stVdecOutAttr.u16Width = MIN(stDispWndRect[i].u16PicW, stVdecChnAttr.u32PicWidth);
                stVdecOutAttr.u16Height = MIN(stDispWndRect[i].u16PicH, stVdecChnAttr.u32PicHeight);
                STCHECKRESULT(MI_VDEC_SetOutputPortAttr(i, &stVdecOutAttr));
            }
        }
    }

    // set disp port attribute
    ST_DispChnInfo_t stDispChnInfo;
    memset(&stDispChnInfo, 0, sizeof(ST_DispChnInfo_t));
    stDispChnInfo.InputPortNum = u32CurWndNum;

    for (i = 0; i < u32CurWndNum; i++)
    {
        stDispChnInfo.stInputPortAttr[i].u32Port = i;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16X =
            ALIGN_BACK(stDispWndRect[i].s32X, DISP_XPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Y =
            ALIGN_BACK(stDispWndRect[i].s32Y, DISP_YPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Width =
            ALIGN_BACK(stDispWndRect[i].u16PicW, DISP_WIDTH_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Height =
            ALIGN_BACK(stDispWndRect[i].u16PicH, DISP_HEIGHT_ALIGN);
        if(g_bScaleDownByDivp[i])
        {
            memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
            STCHECKRESULT(MI_DIVP_GetOutputPortAttr(i, &stDivpOutAttr));
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth = stDivpOutAttr.u32Width;
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight = stDivpOutAttr.u32Height;
        }
        else
        {
            memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
            STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth = stVdecOutAttr.u16Width;
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight = stVdecOutAttr.u16Height;
        }
    }
    STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER0, &stDispChnInfo));

    // start divp
    for (i = 0; i < u32CurWndNum; i++)
    {
        if(g_bScaleDownByDivp[i])
        {
            STCHECKRESULT(MI_DIVP_StartChn(i));
        }
        //STCHECKRESULT(MI_VDEC_StartChn(i));
    }

    // enable disp
    for (i = 0; i < stDispChnInfo.InputPortNum; i ++)
    {
        ExecFunc(MI_DISP_EnableInputPort(ST_DISP_LAYER0, i), MI_SUCCESS);
    }

    // 7, bind VDEC to DIVP
    for (i = 0; i < u32CurWndNum; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
        if(g_bScaleDownByDivp[i])
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d vdec(%d)->divp(%d) bind\n", __func__, __LINE__, i, i);
        }
        else
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d vdec(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
        }
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    // start vdec send thread
    for (i = 0; i < u32CurWndNum; i++)
    {
        g_stVdecThreadArgs[i].bRunFlag = TRUE;
    }

    pstCaseDesc[u32CaseIndex].u32ShowWndNum =
        pstCaseDesc[u32CaseIndex].stSubDesc[u32CurSubCaseIndex].u32WndNum;
    g_u32LastSubCaseIndex = g_u32CurSubCaseIndex;

    return 0;
}

MI_S32 ST_SubExit()
{
    MI_U32 u32WndNum = 0, u32ShowWndNum, u32WndNum_Disp =0;
    MI_S32 i = 0;
    ST_Sys_BindInfo_t stBindInfo;
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stCaseDesc);
    MI_BOOL bFlag = FALSE;
#ifdef PLAY_WITH_ARGS
    u32WndNum = g_u32DispWin;
    u32ShowWndNum = g_u32DispWin;
#else
    u32WndNum = pstCaseDesc[g_u32CaseIndex].stDesc.u32WndNum;
    u32ShowWndNum = pstCaseDesc[g_u32CaseIndex].u32ShowWndNum;
#endif
    u32WndNum_Disp = u32ShowWndNum;
    /************************************************
    step1:  stop send es stream
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        if (g_stVdecThreadArgs[i].pt)
            bFlag = TRUE;
    }

    if (bFlag == FALSE)
    {
        return MI_SUCCESS;
    }

    g_PushEsExit = TRUE;
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        pthread_join(g_stVdecThreadArgs[i].pt, NULL);
        g_stVdecThreadArgs[i].pt = 0;
    }

    /************************************************
    step2:  unbind VDEC to DIVP
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        if(g_bScaleDownByDivp[i])
        {
            memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;

            STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
            printf("%s %d vdec(%d)->divp(%d) unbind\n", __func__, __LINE__, i, i);
        }
    }
    if (pstCaseDesc[g_u32CaseIndex].bPIP)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = ST_DIVP_PIP_CHN;
        stBindInfo.stDstChnPort.u32PortId = 0;

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
        printf("%s %d vdec(0)->divp(%d) unbind\n", __func__, __LINE__, ST_DIVP_PIP_CHN);
    }

    /************************************************
    step4:  unbind DIVP to DISP
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
        if(g_bScaleDownByDivp[i])
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            printf("%s %d divp(%d)->disp(%d) unbind\n", __func__, __LINE__, i, i);
        }
        else
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            printf("%s %d vdec(%d)->disp(%d) unbind\n", __func__, __LINE__, i, i);
        }
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }
    if (pstCaseDesc[g_u32CaseIndex].bPIP)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = ST_DIVP_PIP_CHN;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = ST_DISP_PIP_PORT;
        stBindInfo.stDstChnPort.u32PortId = 0;
        printf("%s %d divp(%d)->disp(%d) unbind\n", __func__, __LINE__, ST_DIVP_PIP_CHN, ST_DISP_PIP_PORT);

        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    }

    /************************************************
    step5:  destroy vdec divp disp
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        STCHECKRESULT(MI_VDEC_StopChn(i));
        STCHECKRESULT(MI_VDEC_DestroyChn(i));

        if(g_bScaleDownByDivp[i])
        {
            STCHECKRESULT(MI_DIVP_StopChn(i));
            STCHECKRESULT(MI_DIVP_DestroyChn(i));
        }
    }

    if (pstCaseDesc[g_u32CaseIndex].bPIP)
    {
        STCHECKRESULT(MI_DIVP_StopChn(ST_DIVP_PIP_CHN));
        STCHECKRESULT(MI_DIVP_DestroyChn(ST_DIVP_PIP_CHN));
        ExecFunc(MI_DISP_DisableInputPort(ST_DISP_LAYER1, 0), MI_SUCCESS);
        ExecFunc(MI_DISP_DisableVideoLayer(ST_DISP_LAYER1), MI_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(ST_DISP_LAYER1, ST_DISP_DEV0), MI_SUCCESS);
    }

    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, u32WndNum_Disp)); //disp input port 0
    STCHECKRESULT(ST_Hdmi_DeInit(E_MI_HDMI_ID_0));
    STCHECKRESULT(ST_Fb_DeInit());

    return MI_SUCCESS;
}

MI_S32 _ST_CropZoom(MI_U32 chn, ST_Rect_t *pstCropRect, MI_BOOL cropDivp)
{
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_GetChnAttr(chn, &stVdecChnAttr));
    if(cropDivp)
    {
        MI_DIVP_ChnAttr_t stDivpChnAttr;
        memset(&stDivpChnAttr, 0, sizeof(stDivpChnAttr));
        STCHECKRESULT(MI_DIVP_GetChnAttr(chn, &stDivpChnAttr));

        if(stVdecChnAttr.u32PicWidth > DIVP_MAX_SIZE)
        {
            stDivpChnAttr.stCropRect.u16X = ALIGN_BACK(pstCropRect->s32X*DIVP_MAX_SIZE/stVdecChnAttr.u32PicWidth, DISP_WIDTH_ALIGN);
            stDivpChnAttr.stCropRect.u16Width = ALIGN_BACK(pstCropRect->u16PicW*DIVP_MAX_SIZE/stVdecChnAttr.u32PicWidth, DISP_WIDTH_ALIGN);
        }
        else
        {
            stDivpChnAttr.stCropRect.u16X = pstCropRect->s32X;
            stDivpChnAttr.stCropRect.u16Width = pstCropRect->u16PicW;
        }
        if(stVdecChnAttr.u32PicHeight > DIVP_MAX_SIZE)
        {
            stDivpChnAttr.stCropRect.u16Y = ALIGN_BACK(pstCropRect->s32Y*DIVP_MAX_SIZE/stVdecChnAttr.u32PicHeight, DISP_HEIGHT_ALIGN);
            stDivpChnAttr.stCropRect.u16Height = ALIGN_BACK(pstCropRect->u16PicH*DIVP_MAX_SIZE/stVdecChnAttr.u32PicHeight, DISP_HEIGHT_ALIGN);
        }
        else
        {
            stDivpChnAttr.stCropRect.u16Y = pstCropRect->s32Y;
            stDivpChnAttr.stCropRect.u16Height = pstCropRect->u16PicH;
        }
        STCHECKRESULT(MI_DIVP_SetChnAttr(chn, &stDivpChnAttr));
    }
    else
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;
        memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
        STCHECKRESULT(MI_DISP_GetInputPortAttr(ST_DISP_LAYER0, chn, &stInputPortAttr));
        MI_DISP_VidWinRect_t stWinRect;
        memset(&stWinRect, 0, sizeof(stWinRect));
        stWinRect.u16X      = ALIGN_BACK(pstCropRect->s32X*stInputPortAttr.u16SrcWidth/stVdecChnAttr.u32PicWidth, DISP_WIDTH_ALIGN);
        stWinRect.u16Y      = ALIGN_BACK(pstCropRect->s32Y*stInputPortAttr.u16SrcHeight/stVdecChnAttr.u32PicHeight, DISP_HEIGHT_ALIGN);
        stWinRect.u16Width  = ALIGN_BACK(pstCropRect->u16PicW*stInputPortAttr.u16SrcWidth/stVdecChnAttr.u32PicWidth, DISP_WIDTH_ALIGN);
        stWinRect.u16Height = ALIGN_BACK(pstCropRect->u16PicH*stInputPortAttr.u16SrcHeight/stVdecChnAttr.u32PicHeight, DISP_HEIGHT_ALIGN);
        STCHECKRESULT(MI_DISP_SetZoomInWindow(ST_DISP_LAYER0, chn, &stWinRect));
    }
}

#define MIN_ZOOM_WIDTH   192
#define MIN_ZOOM_HEIGHT  112

MI_S32 ST_ZoomChn(MI_U32 chn)
{
    ST_Rect_t stRect;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_S32 s32RandX = 0;
    MI_S32 s32RandY = 0;
    MI_S32 s32RandW = 0;
    MI_S32 s32RandH = 0;
    MI_U32 i = 0;
    MI_BOOL bCropByDivp = TRUE;

    memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_GetChnAttr(chn, &stVdecChnAttr));
    if(stVdecChnAttr.u32PicWidth * stVdecChnAttr.u32PicHeight > 2560*1440)
        bCropByDivp = FALSE;
    MI_U32 maxZoomTimes = MIN(stVdecChnAttr.u32PicWidth/MIN_ZOOM_WIDTH, stVdecChnAttr.u32PicHeight/MIN_ZOOM_HEIGHT);
    memset(&stRect, 0, sizeof(stRect));

    if(g_bScaleDownByDivp[chn])
    {
        printf("%s %d: not support this case now!!!\n", __func__, __LINE__);
    }
    else
    {
        if(bCropByDivp)
        {
            _ST_ChangeScaleDownFromVdecToDivp(chn);
        }
        for(i=0;i<50;i++)
        {
            s32RandW = rand() % maxZoomTimes;
            s32RandH = rand() % maxZoomTimes;
            if (s32RandH > 0 && s32RandW > 0)
            {
                s32RandX = rand() % maxZoomTimes;
                s32RandY = rand() % maxZoomTimes;
                if((s32RandX+s32RandW)*MIN_ZOOM_WIDTH < stVdecChnAttr.u32PicWidth && (s32RandY+s32RandH)*MIN_ZOOM_HEIGHT < stVdecChnAttr.u32PicHeight)
                {
                    stRect.s32X = s32RandX * MIN_ZOOM_WIDTH;
                    stRect.s32Y = s32RandY * MIN_ZOOM_HEIGHT;
                    stRect.u16PicW = s32RandW * MIN_ZOOM_WIDTH;
                    stRect.u16PicH = s32RandH * MIN_ZOOM_HEIGHT;
                    printf("%s %d: crop xywh(%d, %d, %d, %d) \n", __func__, __LINE__, stRect.s32X, stRect.s32Y, stRect.u16PicW, stRect.u16PicH);
                    _ST_CropZoom(chn, &stRect, bCropByDivp);
                    sleep(2);
                }
            }
        }
        if(bCropByDivp)
        {
            _ST_ChangeScaleDownFromDivpToVdec(chn);
        }
        else
        {
            MI_DISP_InputPortAttr_t stInputPortAttr;
            memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
            STCHECKRESULT(MI_DISP_GetInputPortAttr(ST_DISP_LAYER0, chn, &stInputPortAttr));
            MI_DISP_VidWinRect_t stWinRect;
            memset(&stWinRect, 0, sizeof(stWinRect));
            stWinRect.u16X = 0;
            stWinRect.u16Y = 0;
            stWinRect.u16Width = stInputPortAttr.u16SrcWidth;
            stWinRect.u16Height = stInputPortAttr.u16SrcHeight;
            STCHECKRESULT(MI_DISP_SetZoomInWindow(ST_DISP_LAYER0, chn, &stWinRect));
        }
    }
    return MI_SUCCESS;
}

void ST_SaveStreamToFile(MI_VENC_Stream_t *pstStream)
{
    MI_U32 i = 0;
    char szFileName[64];
    time_t now = 0;
	struct tm *tm = NULL;
    MI_U32 len = 0;
    int fd = 0;
    MI_VENC_Pack_t *pstPack = NULL;

    memset(szFileName, 0, sizeof(szFileName));

    now = time(NULL);
    tm = localtime(&now);
    len = 0;

    len += sprintf(szFileName + len, "%d_", tm->tm_year + 1900);
    len += sprintf(szFileName + len, "%02d_", tm->tm_mon);
    len += sprintf(szFileName + len, "%02d-", tm->tm_mday);
    len += sprintf(szFileName + len, "%02d_", tm->tm_hour);
    len += sprintf(szFileName + len, "%02d_", tm->tm_min);
    len += sprintf(szFileName + len, "%02d", tm->tm_sec);
    len += sprintf(szFileName + len, ".%s", "jpg");

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd <= 0)
	{
		ST_WARN("create file %s error\n", szFileName);
	}
	else
	{
		ST_DBG("open %s success\n", szFileName);
	}


    for (i = 0; i < pstStream->u32PackCount; i ++)
    {
        pstPack = &pstStream->pstPack[i];
        write(fd, pstPack->pu8Addr + pstPack->u32Offset, pstPack->u32Len - pstPack->u32Offset);
    }

    close(fd);
}

#if CAPTURE_FULL_SIZE
MI_S32 _ST_CreateCapture(MI_U8 u8ChnId)
{
    MI_U32 u32DevId = 0;
    ST_Sys_BindInfo_t stBindInfo;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_DIVP_ChnAttr_t stDivpChnAttr;

    memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_GetChnAttr(u8ChnId, &stVdecChnAttr));

    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = stVdecChnAttr.u32PicWidth;
    stDivpChnAttr.stCropRect.u16Height  = stVdecChnAttr.u32PicHeight;
    stDivpChnAttr.u32MaxWidth           = stVdecChnAttr.u32PicWidth;
    stDivpChnAttr.u32MaxHeight          = stVdecChnAttr.u32PicHeight;

    ExecFunc(MI_DIVP_CreateChn(ST_DIVP_CAP_CHN, &stDivpChnAttr), MI_SUCCESS);

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = stVdecChnAttr.u32PicWidth;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = stVdecChnAttr.u32PicHeight;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = stVdecChnAttr.u32PicWidth;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = stVdecChnAttr.u32PicHeight;
    stChnAttr.stVeAttr.stAttrJpeg.bByFrame = TRUE;
    stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;

    MI_VENC_CreateChn(u8ChnId, &stChnAttr);
    if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        MI_VENC_ParamJpeg_t stParamJpeg;

        memset(&stParamJpeg, 0, sizeof(stParamJpeg));
        MI_VENC_GetJpegParam(u8ChnId, &stParamJpeg);

        stParamJpeg.u32Qfactor = 50;
        MI_VENC_SetJpegParam(u8ChnId, &stParamJpeg);
    }

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = u8ChnId;
    stBindInfo.stSrcChnPort.u32PortId = 1; //for capture
    MI_SYS_SetChnOutputPortDepth(&stBindInfo.stSrcChnPort, 0, 3);

    MI_VENC_GetChnDevid(u8ChnId, &u32DevId);
    stBindInfo.stDstChnPort.u32DevId = u32DevId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32ChnId = u8ChnId;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    printf("%s %d vdec(%d)->venc(%d) bind\n", __func__, __LINE__,u8ChnId,u8ChnId);
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    return MI_SUCCESS;
}

MI_S32 _ST_DestoryCapture(MI_U8 u8ChnId)
{
    MI_U32 u32DevId = 0;
    ST_Sys_BindInfo_t stBindInfo;
    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = u8ChnId;
    stBindInfo.stSrcChnPort.u32PortId = 1; //for capture

    MI_VENC_GetChnDevid(u8ChnId, &u32DevId);
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = u32DevId;
    stBindInfo.stDstChnPort.u32ChnId = u8ChnId;
    stBindInfo.stDstChnPort.u32PortId = 0;

    printf("%s %d vdec(%d)->venc(%d) unbind\n", __func__, __LINE__,u8ChnId,u8ChnId);
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(MI_VENC_DestroyChn(u8ChnId));
    STCHECKRESULT(MI_DIVP_DestroyChn(ST_DIVP_CAP_CHN));

    return MI_SUCCESS;
}

MI_S32 _ST_CaptureJpeg(MI_U8 u8ChnId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;

    s32Ret = MI_VENC_StartRecvPic(u8ChnId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s[%d]: ST_Venc_StartChannel fail, 0x%x\n", __func__, __LINE__, s32Ret);
        return;
    }

    memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
    memset(&stPack, 0, sizeof(MI_VENC_Pack_t));
    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    while (1)
    {
#if 1
        s32Ret = MI_VENC_GetStream(u8ChnId, &stStream, 40);
        if (MI_SUCCESS == s32Ret)
        {
            printf("%s[%d]: ##########Start to write file!!!#####################\n",__func__, __LINE__);
            ST_SaveStreamToFile(&stStream);
            printf("%s[%d]: ##########End to write file!!!#####################\n",__func__, __LINE__);
            s32Ret = MI_VENC_ReleaseStream(u8ChnId, &stStream);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s[%d]: MI_VENC_ReleaseStream fail, ret:0x%x\n", __func__, __LINE__, s32Ret);
            }
            break;
        }
        else
            printf("%s[%d]: Continue!!!\n",__func__, __LINE__);
#endif
        usleep(200 * 1000);
    }

    s32Ret = MI_VENC_StopRecvPic(u8ChnId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s[%d]: MI_VENC_StopRecvPic fail, 0x%x\n", __func__, __LINE__, s32Ret);
    }
}
#else
MI_S32 _ST_CreateCapture(MI_U8 u8ChnId, MI_U32 width, MI_U32 height)
{
    MI_U32 u32DevId = 0;
    ST_Sys_BindInfo_t stBindInfo;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;

    memset(&stVdecChnAttr, 0, sizeof(stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_GetChnAttr(u8ChnId, &stVdecChnAttr));

    if(width > stVdecChnAttr.u32PicWidth || height > stVdecChnAttr.u32PicHeight || width < MIN_ZOOM_WIDTH || height < MIN_ZOOM_HEIGHT)
    {
        printf("%s %d: not support wh[%d, %d]\n", __func__, __LINE__, width, height);
    }
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = stVdecChnAttr.u32PicWidth;
    stDivpChnAttr.stCropRect.u16Height  = stVdecChnAttr.u32PicHeight;
    stDivpChnAttr.u32MaxWidth           = stVdecChnAttr.u32PicWidth;
    stDivpChnAttr.u32MaxHeight          = stVdecChnAttr.u32PicHeight;

    ExecFunc(MI_DIVP_CreateChn(ST_DIVP_CAP_CHN, &stDivpChnAttr), MI_SUCCESS);

    memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
    stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stDivpOutAttr.u32Width           = ALIGN_BACK(width, DIVP_OUT_WIDTH_ALIGN);
    stDivpOutAttr.u32Height          = height;

    STCHECKRESULT(MI_DIVP_SetOutputPortAttr(ST_DIVP_CAP_CHN, &stDivpOutAttr));

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = width;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = height;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = width;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = height;
    stChnAttr.stVeAttr.stAttrJpeg.bByFrame = TRUE;
    stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;

    MI_VENC_CreateChn(u8ChnId, &stChnAttr);
    if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        MI_VENC_ParamJpeg_t stParamJpeg;

        memset(&stParamJpeg, 0, sizeof(stParamJpeg));
        MI_VENC_GetJpegParam(u8ChnId, &stParamJpeg);

        stParamJpeg.u32Qfactor = 50;
        MI_VENC_SetJpegParam(u8ChnId, &stParamJpeg);
    }

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = ST_DIVP_CAP_CHN;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stBindInfo.stSrcChnPort, 0, 3);

    MI_VENC_GetChnDevid(u8ChnId, &u32DevId);
    stBindInfo.stDstChnPort.u32DevId = u32DevId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32ChnId = u8ChnId;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    printf("%s %d divp(%d)->venc(%d) bind\n", __func__, __LINE__,ST_DIVP_CAP_CHN,u8ChnId);
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    return MI_SUCCESS;
}

MI_S32 _ST_DestoryCapture(MI_U8 u8ChnId)
{
    MI_U32 u32DevId = 0;
    ST_Sys_BindInfo_t stBindInfo;
    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = ST_DIVP_CAP_CHN;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    MI_VENC_GetChnDevid(u8ChnId, &u32DevId);
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = u32DevId;
    stBindInfo.stDstChnPort.u32ChnId = u8ChnId;
    stBindInfo.stDstChnPort.u32PortId = 0;

    printf("%s %d divp(%d)->venc(%d) unbind\n", __func__, __LINE__,ST_DIVP_CAP_CHN,u8ChnId);

    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = u8ChnId;
    stBindInfo.stSrcChnPort.u32PortId = 1;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = ST_DIVP_CAP_CHN;
    stBindInfo.stDstChnPort.u32PortId = 0; //for capture

    printf("%s %d vdec(%d)->divp(%d) unbind\n", __func__, __LINE__,u8ChnId,ST_DIVP_CAP_CHN);
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(MI_VENC_DestroyChn(u8ChnId));

    STCHECKRESULT(MI_DIVP_StopChn(ST_DIVP_CAP_CHN));
    STCHECKRESULT(MI_DIVP_DestroyChn(ST_DIVP_CAP_CHN));

    return MI_SUCCESS;
}

MI_S32 _ST_CaptureJpeg(MI_U8 u8ChnId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;
    ST_Sys_BindInfo_t stBindInfo;

    s32Ret = MI_VENC_StartRecvPic(u8ChnId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s[%d]: ST_Venc_StartChannel fail, 0x%x\n", __func__, __LINE__,s32Ret);
        return;
    }

    ExecFunc(MI_DIVP_StartChn(ST_DIVP_CAP_CHN), MI_SUCCESS);

    // bind VDEC->DIVP
    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = u8ChnId;
    stBindInfo.stSrcChnPort.u32PortId = 1; //for capture

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = ST_DIVP_CAP_CHN;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;

    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    printf("%s %d vdec(%d)->divp(%d) bind\n", __func__, __LINE__,u8ChnId,ST_DIVP_CAP_CHN);

    memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
    memset(&stPack, 0, sizeof(MI_VENC_Pack_t));
    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    while (1)
    {
#if 1
        s32Ret = MI_VENC_GetStream(u8ChnId, &stStream, 40);
        if (MI_SUCCESS == s32Ret)
        {
            printf("%s[%d]: ##########Start to write file!!!#####################\n",__func__, __LINE__);
            ST_SaveStreamToFile(&stStream);
            printf("%s[%d]: ##########End to write file!!!#####################\n",__func__, __LINE__);
            s32Ret = MI_VENC_ReleaseStream(u8ChnId, &stStream);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s[%d]: MI_VENC_ReleaseStream fail, ret:0x%x\n", __func__, __LINE__,s32Ret);
            }
            break;
        }
        else
            printf("%s[%d]: Continue, ret = 0x%x!!!\n",__func__, __LINE__,s32Ret);
#endif
        usleep(200 * 1000);
    }

    s32Ret = MI_VENC_StopRecvPic(u8ChnId);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s[%d]: MI_VENC_StopRecvPic fail, 0x%x\n", __func__, __LINE__,s32Ret);
    }
}
#endif
MI_S32 ST_CaptureTest(MI_U8 chn)
{
    MI_U8 i = 0;
    MI_S32 ret = MI_SUCCESS;
#if CAPTURE_FULL_SIZE
    ret = _ST_CreateCapture(chn);
#else
    ret = _ST_CreateCapture(chn, 640, 360);
#endif
    if(MI_SUCCESS != ret)
    {
        printf("%s[%d]: _ST_CreateCapture fail\n", __func__, __LINE__);
        return 1;
    }
    for(i=0;i<10;i++)
    {
        _ST_CaptureJpeg(chn);
        sleep(2);
    }
    _ST_DestoryCapture(chn);
}

int autotest_sub(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_S32 s32CaseArray[5] = {1,3,4,5,6};
    MI_S32 s32RandNumber = 0;
    MI_S32 s32LastRandNumber = 0;
    MI_U32 u32CaseIndex = g_u32CaseIndex;

    while (1)
    {
        s32RandNumber = rand() % ARRAY_SIZE(s32CaseArray);
        if (s32LastRandNumber != s32RandNumber)
        {
            sleep(5);
            printf("Run test case %d...\n", s32CaseArray[s32RandNumber]);
            sleep(5);

            g_u32CurSubCaseIndex = s32CaseArray[s32RandNumber];

            if (pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming > 0)
            {
                ST_ChangeDisplayTiming(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming); //change timing
            }

            s32LastRandNumber = s32RandNumber;
        }
    }

    return MI_SUCCESS;
}

void ST_WaitSubCmd(void)
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    char szCmd[16];
    MI_U32 index = 0;
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 u32SubCaseSize = pstCaseDesc[u32CaseIndex].u32SubCaseNum;

    // autotest_sub();

    while (!g_subExit)
    {
        ST_CaseUsage();

        fgets((szCmd), (sizeof(szCmd) - 1), stdin);

        index = atoi(szCmd);

        if (index <= 0 || index > u32SubCaseSize)
        {
            continue;
        }

        g_u32CurSubCaseIndex = index - 1;

        if (pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming > 0)
        {
            ST_ChangeDisplayTiming(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming); //change timing
        }
        else
        {
            if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "exit", 4)))
            {
                ST_SubExit();
                return;
            }
            else if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "zoom", 4)))
            {
                ST_ZoomChn(0);
            }
            else if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "test region", 11)))
            {
                ST_TestRegion();
            }
            else if (0 == (strncmp(pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "Capture JPEG", 12)))
            {
                ST_CaptureTest(0);
            }
            else
            {
                if (pstCaseDesc[u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].u32WndNum > 0)
                {
                    ST_SplitWindow(); //switch screen
                }
            }
        }
    }
}

int ST_VdecH26X(ST_DispoutTiming_e eDispTiming)
{
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;
    MI_U32 u32CaseSize = ARRAY_SIZE(g_stCaseDesc);
    MI_U32 u32CaseIndex = g_u32CaseIndex;
    MI_U32 i = 0;
    ST_Rect_t stRect;
    MI_U32 u32Square = 0;
    ST_Rect_t stVdispOutRect;
    ST_Rect_t stDispWndRect[32] = {0};
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;
    MI_S32 s32Ret;

    if (u32CaseIndex < 0 || u32CaseIndex > u32CaseSize)
    {
        return;
    }

    MI_U32 u32SubCaseSize = pstCaseDesc[u32CaseIndex].u32SubCaseNum;
    MI_U32 u32WndNum = pstCaseDesc[u32CaseIndex].stDesc.u32WndNum;
    MI_U32 u32WndNum_Disp = 0;
    if(pstCaseDesc[u32CaseIndex].u32ShowWndNum == 0)
        u32WndNum_Disp = u32WndNum;
    else
        u32WndNum_Disp = pstCaseDesc[u32CaseIndex].u32ShowWndNum;

    stVdispOutRect.s32X     = 0;
    stVdispOutRect.s32Y     = 0;
#ifdef PLAY_WITH_ARGS
    u32WndNum = g_u32DispWin;
    u32WndNum_Disp = g_u32DispWin;
    STCHECKRESULT(ST_GetTimingInfo(dispTiming, &s32HdmiTiming, &s32DispTiming,
            &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));
#else
    STCHECKRESULT(ST_GetTimingInfo(eDispTiming, &s32HdmiTiming, &s32DispTiming,
            &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));
#endif
    // init hdmi
    STCHECKRESULT(ST_Hdmi_Init()); //Set hdmi outout 1080P
    MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, TRUE);

    if (u32WndNum <= 1)
    {
        u32Square = 1;
    }
    else if (u32WndNum <= 4)
    {
        u32Square = 2;
    }
    else if (u32WndNum <= 9)
    {
        u32Square = 3;
    }
    else if (u32WndNum <= 16)
    {
        u32Square = 4;
    }
    else if (u32WndNum <= 25)
    {
        u32Square = 5;
    }
    else if (u32WndNum <= 36)
    {
        u32Square = 6;
    }

    for (i = 0; i < u32WndNum; i++)
    {
        stDispWndRect[i].s32X    = ALIGN_BACK((stVdispOutRect.u16PicW / u32Square) * (i % u32Square), DISP_XPOS_ALIGN);
        stDispWndRect[i].s32Y    = ALIGN_BACK((stVdispOutRect.u16PicH / u32Square) * (i / u32Square), DISP_YPOS_ALIGN);
        stDispWndRect[i].u16PicW = ALIGN_BACK((stVdispOutRect.u16PicW / u32Square), DISP_WIDTH_ALIGN);
        stDispWndRect[i].u16PicH = ALIGN_BACK((stVdispOutRect.u16PicH / u32Square), DISP_HEIGHT_ALIGN);
    }
    /************************************************
    step2:  create VDEC chn
    *************************************************/
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN vdecChn = 0;
    MI_VDEC_OutputPortAttr_t stVdecOutAttr;
    MI_U16 u16TempW = 0;
    MI_U16 u16TempH = 0;
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        vdecChn = i;
        memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 1;
        stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
#ifdef PLAY_WITH_ARGS
        stVdecChnAttr.u32PicWidth   = g_u32VdecWidth;
        stVdecChnAttr.u32PicHeight  = g_u32VdecHeight;
        stVdecChnAttr.eCodecType    = videoType;
        if(g_u32RefFrame < 3)
            stVdecChnAttr.eDpbBufMode = g_u32RefFrame;
        else
        {
            stVdecChnAttr.eDpbBufMode  = 0;
            stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = g_u32RefFrame;
        }
#else
        stVdecChnAttr.u32PicWidth   = pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.u32MaxWidth;
        stVdecChnAttr.u32PicHeight  = pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.u32MaxHeight;
        stVdecChnAttr.eCodecType    = pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.eCodecType;
        stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_INPLACE_ONE_BUF;
#endif
        stVdecChnAttr.u32Priority   = 0;
        if(stVdecChnAttr.u32PicWidth*stVdecChnAttr.u32PicHeight > 1920*1088)
            stVdecChnAttr.u32BufSize = 2 * 1024 * 1024;
        else if(stVdecChnAttr.u32PicWidth*stVdecChnAttr.u32PicHeight > 720*576)
            stVdecChnAttr.u32BufSize = 1024 * 1024;
        else
            stVdecChnAttr.u32BufSize = 512 * 1024;

        s32Ret = MI_VDEC_CreateChn(vdecChn, &stVdecChnAttr);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_VDEC_CreateChn error, chn:%d, %X\n", vdecChn, s32Ret);
            return 1;
        }

        s32Ret = MI_VDEC_StartChn(vdecChn);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_VDEC_StartChn error, chn:%d, %X\n", vdecChn, s32Ret);
            return 1;
        }

        memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
        u16TempW = ALIGN_FRONT(stDispWndRect[i].u16PicW, VDEC_WIDTH_ALIGN);
        u16TempH = ALIGN_FRONT(stDispWndRect[i].u16PicH, VDEC_HEIGHT_ALIGN);
        if(_IS_OVER_VDEC_MAX_SCALEDOWN_TIMES(stVdecChnAttr.u32PicWidth, u16TempW) || _IS_OVER_VDEC_MAX_SCALEDOWN_TIMES(stVdecChnAttr.u32PicHeight, u16TempH))
        {
            printf("chn[%d]: special case, video wxh[%d,%d], scaler down to [%d,%d]\n",
                    vdecChn, stVdecChnAttr.u32PicWidth, stVdecChnAttr.u32PicHeight, u16TempW, u16TempH);
            g_bScaleDownByDivp[i] = TRUE;
            stVdecOutAttr.u16Width  = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN);
            stVdecOutAttr.u16Height = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN);
        }
        else
        {
            g_bScaleDownByDivp[i] = FALSE;
            stVdecOutAttr.u16Width  = u16TempW > stVdecChnAttr.u32PicWidth ? stVdecChnAttr.u32PicWidth : u16TempW;
            stVdecOutAttr.u16Height = u16TempH > stVdecChnAttr.u32PicHeight? stVdecChnAttr.u32PicHeight: u16TempH;
        }
        s32Ret = MI_VDEC_SetOutputPortAttr(vdecChn, &stVdecOutAttr);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_VDEC_SetOutputPortAttr error, chn:%d, %X\n", vdecChn, s32Ret);
            return 1;
        }

        printf("%s %d, start vdec chn %d success\n", __func__, __LINE__, vdecChn);
    }

    /************************************************
    step3:  create DIVP chn
    *************************************************/
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_DIVP_CHN divpChn = 0;

    for (i = 0; i < u32WndNum_Disp; i++)
    {
        if(g_bScaleDownByDivp[i])
        {
            memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
            STCHECKRESULT(MI_VDEC_GetChnAttr(i, &stVdecChnAttr));

            divpChn = i;
            memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
            stDivpChnAttr.bHorMirror            = FALSE;
            stDivpChnAttr.bVerMirror            = FALSE;
            stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
            stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
            stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
            stDivpChnAttr.stCropRect.u16X       = 0;
            stDivpChnAttr.stCropRect.u16Y       = 0;
            stDivpChnAttr.stCropRect.u16Width   = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //Vdec pic w
            stDivpChnAttr.stCropRect.u16Height  = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN); //Vdec pic h
            stDivpChnAttr.u32MaxWidth           = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //max size picture can pass
            stDivpChnAttr.u32MaxHeight          = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN);

            ExecFunc(MI_DIVP_CreateChn(divpChn, &stDivpChnAttr), MI_SUCCESS);
            ExecFunc(MI_DIVP_StartChn(divpChn), MI_SUCCESS);

            memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
            stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
            stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            stDivpOutAttr.u32Width           = ALIGN_BACK(stDispWndRect[i].u16PicW, DIVP_OUT_WIDTH_ALIGN);
            stDivpOutAttr.u32Height          = ALIGN_BACK(stDispWndRect[i].u16PicH, DISP_HEIGHT_ALIGN);

            STCHECKRESULT(MI_DIVP_SetOutputPortAttr(divpChn, &stDivpOutAttr));
        }
    }

    if (pstCaseDesc[u32CaseIndex].bPIP) // default: pip chn bind to chn0
    {
        divpChn = ST_DIVP_PIP_CHN;
        memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
        stDivpChnAttr.bHorMirror            = FALSE;
        stDivpChnAttr.bVerMirror            = FALSE;
        stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
        stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
        stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
        stDivpChnAttr.stCropRect.u16X       = 0;
        stDivpChnAttr.stCropRect.u16Y       = 0;
        if(g_bScaleDownByDivp[0])
        {
            memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
            STCHECKRESULT(MI_VDEC_GetChnAttr(0, &stVdecChnAttr));
            stDivpChnAttr.stCropRect.u16Width   = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //Vdec pic w
            stDivpChnAttr.stCropRect.u16Height  = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN); //Vdec pic h
            stDivpChnAttr.u32MaxWidth           = ALIGN_FRONT(stVdecChnAttr.u32PicWidth/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_WIDTH_ALIGN); //max size picture can pass
            stDivpChnAttr.u32MaxHeight          = ALIGN_FRONT(stVdecChnAttr.u32PicHeight/TWO_STEP_SCALEDOWN_VDEC_TIMES, VDEC_HEIGHT_ALIGN);
        }
        else
        {
            stDivpChnAttr.stCropRect.u16Width   = ALIGN_FRONT(stDispWndRect[0].u16PicW, VDEC_WIDTH_ALIGN); //Vdec pic w
            stDivpChnAttr.stCropRect.u16Height  = ALIGN_FRONT(stDispWndRect[0].u16PicH, VDEC_HEIGHT_ALIGN); //Vdec pic h
            stDivpChnAttr.u32MaxWidth           = ALIGN_FRONT(stDispWndRect[0].u16PicW, VDEC_WIDTH_ALIGN); //max size picture can pass
            stDivpChnAttr.u32MaxHeight          = ALIGN_FRONT(stDispWndRect[0].u16PicH, VDEC_HEIGHT_ALIGN);
        }
        ExecFunc(MI_DIVP_CreateChn(divpChn, &stDivpChnAttr), MI_SUCCESS);
        ExecFunc(MI_DIVP_StartChn(divpChn), MI_SUCCESS);

        memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
        stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
        stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stDivpOutAttr.u32Width  = ALIGN_BACK(stVdispOutRect.u16PicW / 3, DIVP_OUT_WIDTH_ALIGN);
        stDivpOutAttr.u32Height = ALIGN_BACK(stVdispOutRect.u16PicH / 3, DISP_HEIGHT_ALIGN);

        ST_DBG("DIVP PIP position, disp:%dx%d (%d,%d)\n", stVdispOutRect.u16PicW, stVdispOutRect.u16PicH,
            stDivpOutAttr.u32Width, stDivpOutAttr.u32Height);

        STCHECKRESULT(MI_DIVP_SetOutputPortAttr(divpChn, &stDivpOutAttr));
    }

    /************************************************
    step4:  start DISP device and chn
    *************************************************/
    printf("[%s][%d]timing:%d\n",__func__,__LINE__,s32DispTiming);

    ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, s32DispTiming);

    if (pstCaseDesc[u32CaseIndex].bPIP)
    {
        ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER1, s32DispTiming);
    }

    MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, FALSE);
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32HdmiTiming));
    e_curDispTiming = eDispTiming;

    ST_DispChnInfo_t stDispChnInfo;
    memset(&stDispChnInfo, 0, sizeof(ST_DispChnInfo_t));
    stDispChnInfo.InputPortNum = u32WndNum_Disp;

    for (i = 0; i < u32WndNum_Disp; i++)
    {
        stDispChnInfo.stInputPortAttr[i].u32Port = i;
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16X =
            ALIGN_BACK(stDispWndRect[i].s32X, DISP_XPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Y =
            ALIGN_BACK(stDispWndRect[i].s32Y, DISP_YPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Width =
            ALIGN_BACK(stDispWndRect[i].u16PicW, DISP_WIDTH_ALIGN);
        stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Height =
            ALIGN_BACK(stDispWndRect[i].u16PicH, DISP_HEIGHT_ALIGN);
        if(g_bScaleDownByDivp[i])
        {
            memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
            STCHECKRESULT(MI_DIVP_GetOutputPortAttr(i, &stDivpOutAttr));
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth = stDivpOutAttr.u32Width;
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight = stDivpOutAttr.u32Height;
        }
        else
        {
            memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
            STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth = stVdecOutAttr.u16Width;
            stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight = stVdecOutAttr.u16Height;
        }
    }
    STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER0, &stDispChnInfo));

    if (pstCaseDesc[u32CaseIndex].bPIP)
    {
        memset(&stDispChnInfo, 0, sizeof(ST_DispChnInfo_t));

        stDispChnInfo.InputPortNum = 1;
        stDispChnInfo.stInputPortAttr[0].u32Port = 0;//ST_DISP_PIP_PORT;
        stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X =
            ALIGN_BACK((stVdispOutRect.u16PicW / 3) * (8 % 3), DISP_XPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y =
            ALIGN_BACK((stVdispOutRect.u16PicH / 3) * (8 / 3), DISP_YPOS_ALIGN);
        stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width =
            ALIGN_BACK(stVdispOutRect.u16PicW / 3, DISP_WIDTH_ALIGN);
        stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height =
            ALIGN_BACK(stVdispOutRect.u16PicH / 3, DISP_HEIGHT_ALIGN);

        memset(&stDivpOutAttr, 0, sizeof(stDivpOutAttr));
        STCHECKRESULT(MI_DIVP_GetOutputPortAttr(ST_DIVP_PIP_CHN, &stDivpOutAttr));
        stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcWidth = stDivpOutAttr.u32Width;
        stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcHeight = stDivpOutAttr.u32Height;

        ST_DBG("DISP PIP position, disp:%dx%d (%d,%d,%d,%d), src:%dx%d\n", stVdispOutRect.u16PicW, stVdispOutRect.u16PicH,
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X,
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y,
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width,
            stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height,
            stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcWidth,
            stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcHeight);
        STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER1, &stDispChnInfo));
    }

    /************************************************
    step5:  bind VDEC to DIVP
    *************************************************/
    ST_Sys_BindInfo_t stBindInfo;

    for (i = 0; i < u32WndNum_Disp; i++)
    {
        if(g_bScaleDownByDivp[i])
        {
            memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

            printf("%s %d vdec(%d)->divp(%d) bind\n", __func__, __LINE__, i, i);
        }
    }

    if (pstCaseDesc[u32CaseIndex].bPIP)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = 0;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = ST_DIVP_PIP_CHN;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 0;
        stBindInfo.u32DstFrmrate = 0;

        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

        printf("%s %d vdec(0)->divp(%d) bind\n", __func__, __LINE__, ST_DIVP_PIP_CHN);
    }

    /************************************************
    step6:  bind DIVP to DISP
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
        if(g_bScaleDownByDivp[i])
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d divp(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
        }
        else
        {
            stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
            stBindInfo.stSrcChnPort.u32DevId = 0;
            stBindInfo.stSrcChnPort.u32ChnId = i;
            stBindInfo.stSrcChnPort.u32PortId = 0;

            stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
            stBindInfo.stDstChnPort.u32DevId = 0;
            stBindInfo.stDstChnPort.u32ChnId = i;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate = 0;
            stBindInfo.u32DstFrmrate = 0;

            printf("%s %d vdec(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
        }
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    if (pstCaseDesc[u32CaseIndex].bPIP)
    {
        memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));

        stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
        stBindInfo.stSrcChnPort.u32DevId = 0;
        stBindInfo.stSrcChnPort.u32ChnId = ST_DIVP_PIP_CHN;
        stBindInfo.stSrcChnPort.u32PortId = 0;

        stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stBindInfo.stDstChnPort.u32DevId = 0;
        stBindInfo.stDstChnPort.u32ChnId = ST_DISP_PIP_PORT;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate = 0;
        stBindInfo.u32DstFrmrate = 0;
        printf("%s %d: pip divp(%d)->disp(%d) bind\n", __func__, __LINE__, ST_DIVP_PIP_CHN, ST_DISP_PIP_PORT);

        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    /************************************************
    step9:  send es stream to vdec
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
#ifdef PLAY_WITH_ARGS
        g_stVdecThreadArgs[i].vdecChn = i;
        snprintf(g_stVdecThreadArgs[i].szFileName, sizeof(g_stVdecThreadArgs[i].szFileName) - 1, EsFileName);
#else
        g_stVdecThreadArgs[i].vdecChn = pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.u32Chn;
        snprintf(g_stVdecThreadArgs[i].szFileName, sizeof(g_stVdecThreadArgs[i].szFileName) - 1,
                pstCaseDesc[u32CaseIndex].stCaseArgs[i].uChnArg.stVdecChnArg.szFilePath);
#endif
        g_stVdecThreadArgs[i].bRunFlag = TRUE;
        g_PushEsExit = FALSE;
        pthread_create(&g_stVdecThreadArgs[i].pt, NULL, ST_VdecSendStream, (void *)&g_stVdecThreadArgs[i]);
    }

    g_u32LastSubCaseIndex = pstCaseDesc[u32CaseIndex].u32SubCaseNum - 1;
    g_u32CurSubCaseIndex = pstCaseDesc[u32CaseIndex].u32SubCaseNum - 1;
    pstCaseDesc[u32CaseIndex].u32ShowWndNum = u32WndNum_Disp;
}

void ST_DealCase(int argc, char **argv)
{
    MI_U32 u32Index = 0;
    MI_U32 u32SubIndex = 0;
    ST_CaseDesc_t *pstCaseDesc = g_stCaseDesc;

    if (argc != 3)
    {
        return;
    }

    u32Index = atoi(argv[1]);
    u32SubIndex = atoi(argv[2]);

    if (u32Index <= 0 || u32Index > ARRAY_SIZE(g_stCaseDesc))
    {
        printf("case index range (%d~%d)\n", 1, ARRAY_SIZE(g_stCaseDesc));
        return;
    }
    g_u32CaseIndex = u32Index - 1;

    if (u32SubIndex <= 0 || u32SubIndex > pstCaseDesc[g_u32CaseIndex].u32SubCaseNum)
    {
        printf("sub case index range (%d~%d)\n", 1, pstCaseDesc[g_u32CaseIndex].u32SubCaseNum);
        return;
    }

    g_u32LastSubCaseIndex = pstCaseDesc[g_u32CaseIndex].u32SubCaseNum - 1;
    pstCaseDesc[g_u32CaseIndex].u32ShowWndNum = pstCaseDesc[g_u32CaseIndex].stDesc.u32WndNum;

    printf("case index %d, sub case %d-%d\n", g_u32CaseIndex, g_u32CurSubCaseIndex, g_u32LastSubCaseIndex);

    ST_VdecH26X(g_stCaseDesc[g_u32CaseIndex].eDispoutTiming);

    g_u32CurSubCaseIndex = u32SubIndex - 1;

    if (pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming > 0)
    {
        ST_ChangeDisplayTiming(pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].eDispoutTiming); //change timing
    }
    else
    {
        if (0 == (strncmp(pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "exit", 4)))
        {
            ST_SubExit();
            return;
        }
        else if (0 == (strncmp(pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].szDesc, "zoom", 4)))
        {
            ST_ZoomChn(0);
        }
        else
        {
            if (pstCaseDesc[g_u32CaseIndex].stSubDesc[g_u32CurSubCaseIndex].u32WndNum > 0)
            {
                ST_SplitWindow(); //switch screen
            }
        }
    }

    ST_WaitSubCmd();
}

#define ENABLE_AUTOTEST 0

int autotest(int argc, char **argv)
{
    //MI_S32 s32CaseArray[18] = {0,1,2,3,5,7,10,12,15,16,21,24,28,30,34,36,37,38};
    MI_S32 s32RandNumber = 0;
    MI_S32 s32LastRandNumber = 0;

    STCHECKRESULT(ST_Sys_Init());

    while (1)
    {
        s32RandNumber = rand() % 39;
        if ((s32LastRandNumber != s32RandNumber) && (s32RandNumber != 25) && (s32RandNumber != 26))
        {
            sleep(2);
            printf("Run test case %d.......................................\n", s32RandNumber);
            sleep(1);
            g_u32CaseIndex = s32RandNumber;
            ST_VdecH26X(g_stCaseDesc[g_u32CaseIndex].eDispoutTiming);
            sleep(8);
            ST_SubExit();
            s32LastRandNumber = s32RandNumber;
        }
    }

    STCHECKRESULT(ST_Sys_Exit());
    return MI_SUCCESS;
}

void ST_VDEC_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit\n");

        ST_SubExit();

        sleep(1);

        ST_Sys_Exit();

        exit(0);
    }
}

int main(int argc, char **argv)
{
    MI_U32 u32WndNum = 4;
    MI_U32 u32Index = 0;
    MI_U32 u32Temp = 0;
    char szCmd[16];
    struct sigaction sigAction;
    int enterCount = 0;
#if ENABLE_AUTOTEST
    return autotest(argc, argv);
#endif

    sigAction.sa_handler = ST_VDEC_HandleSig;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = 0;
	sigaction(SIGINT, &sigAction, NULL);

    /************************************************
    step1:  init sys
    *************************************************/

    STCHECKRESULT(ST_Sys_Init());

#ifdef PLAY_WITH_ARGS
    char InputCmd[256];
    if(argc < 7)
    {
        printf("arg is less 7 !!!\n\n");
        printf("use case: ./prog_vdec filePath videoType VdecWidth VdecHeight RefFrame DispWin dispTiming !!!\n\n");
        printf("for example: ./prog_vdec /mnt/1080p.h265 1 1920 1080 1 4 4 !!!\n\n");
        STCHECKRESULT(ST_Sys_Exit());
        return -1;
    }
    if(argc >= 2)
    {
        memset(EsFileName,0,256);
        sprintf(EsFileName, "%s", argv[1]);
    }
    if (argc >= 3)
    {
        videoType = (MI_VDEC_CodecType_e)atoi(argv[2]);
        if(videoType < 0 || videoType > 2)
        {
            printf("%s[%d]: videoType[%d] is invaild!!!\n\n", __func__, __LINE__, videoType);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    if (argc >= 4)
    {
        g_u32VdecWidth = (MI_U32)atoi(argv[3]);
        if(g_u32VdecWidth < 32 || g_u32VdecWidth > 4096)
        {
            printf("%s[%d]: VdecWidth[%d] is invaild!!!\n\n", __func__, __LINE__, g_u32VdecWidth);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    if (argc >= 5)
    {
        g_u32VdecHeight = (MI_U32)atoi(argv[4]);
        if(g_u32VdecHeight < 32 || g_u32VdecHeight > 4094)
        {
            printf("%s[%d]: VdecHeight[%d] is invaild!!!\n\n", __func__, __LINE__, g_u32VdecHeight);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    if (argc >= 6)
    {
        g_u32RefFrame = (MI_VDEC_DPB_BufMode_e)atoi(argv[5]);
        if(g_u32RefFrame < 1 || g_u32RefFrame > 10)
        {
            printf("%s[%d]: RefFrame[%d] is invaild!!!\n\n", __func__, __LINE__, g_u32RefFrame);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    if (argc >= 7)
    {
        g_u32DispWin = (MI_U32)atoi(argv[6]);
        if(g_u32DispWin < 1 || g_u32DispWin > 64)
        {
            printf("%s[%d]: DispWin[%d] is invaild!!!\n\n", __func__, __LINE__, g_u32DispWin);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    if (argc >= 8)
    {
        dispTiming = (MI_U32)atoi(argv[7]);
        if(dispTiming < 1 || dispTiming > 12)
        {
            printf("%s[%d]: dispTiming[%d] is invaild!!!\n\n", __func__, __LINE__, dispTiming);
            STCHECKRESULT(ST_Sys_Exit());
            return -1;
        }
    }
    else
    {
        dispTiming = E_ST_TIMING_1080P_60;
    }
    ST_VdecH26X(dispTiming);
    while(1)
    {
        fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
        if(strncmp(InputCmd, "exit", 4) == 0)
        {
            printf("prepare to exit!\n\n");
            break;
        }
    }
#else
    ST_DealCase(argc, argv);

    while (!g_bExit)
    {
        ST_VdecUsage();
        fgets((szCmd), (sizeof(szCmd) - 1), stdin);

        u32Index = atoi(szCmd);

        printf("%s %d, u32Index:%d\n", __func__, __LINE__, u32Index);

        if (u32Index == 0)
        {
            // enter key
            if (enterCount ++ >= 1)
            {
                break;
            }
        }

        if (u32Index <= 0 || u32Index > ARRAY_SIZE(g_stCaseDesc))
        {
            continue;
        }

        g_u32CaseIndex = u32Index - 1;

        ST_VdecH26X(g_stCaseDesc[g_u32CaseIndex].eDispoutTiming);
        ST_WaitSubCmd();
    }

    printf("process exit normal\n");
#endif

    ST_SubExit();
    STCHECKRESULT(ST_Sys_Exit());

    return 0;
}
