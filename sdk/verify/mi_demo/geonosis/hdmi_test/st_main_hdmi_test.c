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
#include <errno.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <linux/input.h>

#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_divp.h"
#include "mi_hdmi.h"
#include "st_fb.h"
#include "st_common.h"
#include "st_disp.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ao.h"

#define MOUSE_DEV_NAME   "/dev/input/mice"
void *ST_FBMouseUserProc(void *args);

typedef struct
{
    pthread_t pt;
    MI_VDEC_CHN vdecChn;
    char szFileName[64];
    MI_BOOL bRunFlag;
} VDEC_Thread_Args_t;

typedef struct
{
    MI_U32 u32WndNum;         // num of windows
    char szFilePath[64];      // h26x file path
    MI_VDEC_CodecType_e eCodecType; // vdec caode type
}ST_ES_DESC_t;

typedef struct
{
    MI_U8 DevID;         // num of windows
    MI_AUDIO_SampleRate_e eSamR;
    MI_S32 s32SetVolumeDb;
    int fdRd;
    pthread_t tid;
    MI_AUDIO_SoundMode_e soundMode;
    MI_BOOL runningFlag;
    char szFilePath[64];      // audio file path
}ST_Aud_DESC_t;

typedef enum
{
    FB_DRAW_BEGIN = 0,
    FB_DRAW_ING,
    FB_DRAW_END,
} FB_DRAW_E;

typedef struct
{
    MI_U8  u8BtnIndex;
    MI_U16 u16PosX;
    MI_U16 u16PosY;
    MI_U16 u16Width;
    MI_U16 u16Height;
    ST_DispoutTiming_e eTiming;
    MI_U8 u8TagFilePath[64];
    MI_U8 *u8DataBuf;
}FB_BUTTON_t;

static MI_BOOL g_PushEsExit = FALSE;
ST_DispoutTiming_e g_DispTimingLast;
static MI_BOOL bIsTimingChanging = FALSE;

static MI_U8 curBtnInx = 3;
#define FB_ENABLE    1
pthread_t g_mouse_user_pt;
VDEC_Thread_Args_t g_stVdecThreadArgs[MAX_CHN_NUM];
#define ADD_HEADER_ES

#define DISP_XPOS_ALIGN     2
#define DISP_YPOS_ALIGN     2
#define DISP_WIDTH_ALIGN    2
#define DISP_HEIGHT_ALIGN   2
#define VDEC_WIDTH_ALIGN    2
#define VDEC_HEIGHT_ALIGN   2
FILE *g_pStreamFile[32] = {NULL};

//Audio Var
#define MI_AUDIO_SAMPLE_PER_FRAME 1024
#define DMA_BUF_SIZE_8K (8000)
#define DMA_BUF_SIZE_16K    (16000)
#define DMA_BUF_SIZE_32K    (32000)
#define DMA_BUF_SIZE_48K    (48000)
typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int  dwSamplesPerSec;
    unsigned int  dwAvgBytesPerSec;
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

ST_ES_DESC_t g_stES_Desc =
{
    .u32WndNum = 4,
    .szFilePath = ST_D1_H265_25_FILE,
    .eCodecType = E_MI_VDEC_CODEC_TYPE_H265,
};

ST_Aud_DESC_t g_stAud_Desc =
{
    .DevID = 2,
    .eSamR = E_MI_AUDIO_SAMPLE_RATE_48000,
    .s32SetVolumeDb = -5,
    .fdRd = -1,
    .tid = 0,
    .soundMode = E_MI_AUDIO_SOUND_MODE_MONO,
    .runningFlag = TRUE,
    .szFilePath = "./48K_16bit_MONO_60s.wav",
};

FB_BUTTON_t g_stFB_button[]=
{
    {
        .u8BtnIndex = 0,
        .u16PosX = 200,
        .u16PosY = 100,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_720P_50,
        .u8TagFilePath = "./ui_dat/720P_50.rgba",
    },
    {
        .u8BtnIndex = 1,
        .u16PosX = 400,
        .u16PosY = 100,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_720P_60,
        .u8TagFilePath = "./ui_dat/720P.rgba",
    },
    {
        .u8BtnIndex = 2,
        .u16PosX = 600,
        .u16PosY = 100,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1080P_50,
        .u8TagFilePath = "./ui_dat/1080P_50.rgba",  
    }, 
    {
        .u8BtnIndex = 3,
        .u16PosX = 800,
        .u16PosY = 100,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1080P_60,
        .u8TagFilePath = "./ui_dat/1080P.rgba",
    },
    {
        .u8BtnIndex = 4,
        .u16PosX = 200,
        .u16PosY = 250,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1600x1200_60,
        .u8TagFilePath = "./ui_dat/1600.rgba",
    },
    {
        .u8BtnIndex = 5,
        .u16PosX = 400,
        .u16PosY = 250,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1440x900_60,
        .u8TagFilePath = "./ui_dat/1440.rgba",
    },
    {
        .u8BtnIndex = 6,
        .u16PosX = 600,
        .u16PosY = 250,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1280x1024_60,
        .u8TagFilePath = "./ui_dat/1280x1024.rgba",
    },
    {
        .u8BtnIndex = 7,
        .u16PosX = 800,
        .u16PosY = 250,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1024x768_60,
        .u8TagFilePath = "./ui_dat/1024.rgba",
    },
    {
        .u8BtnIndex = 8,
        .u16PosX = 200,
        .u16PosY = 400,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1280x800_60,
        .u8TagFilePath = "./ui_dat/1280x800.rgba",
    },
    {
        .u8BtnIndex = 9,
        .u16PosX = 400,
        .u16PosY = 400,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1366x768_60,
        .u8TagFilePath = "./ui_dat/1366.rgba",
    },
    {
        .u8BtnIndex = 10,
        .u16PosX = 600,
        .u16PosY = 400,
        .u16Width = 100,
        .u16Height = 50,
        .eTiming = E_ST_TIMING_1680x1050_60,
        .u8TagFilePath = "./ui_dat/1680.rgba",
    },
};
static MI_U16 g_btn_nums = (sizeof(g_stFB_button)/sizeof(g_stFB_button[0]));

MI_S32 Hdmi_callback(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            printf("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            printf("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            printf("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}

void ST_FBVdecDeInit()
{
    ST_Sys_BindInfo_t stBindInfo;
    int i =0;
    g_PushEsExit = TRUE;
    for (i = 0; i < g_stES_Desc.u32WndNum ; i++)
    {
        pthread_join(g_stVdecThreadArgs[i].pt, NULL);
        g_stVdecThreadArgs[i].pt = 0;
    }
    for(i=0;i<g_stES_Desc.u32WndNum;i++)
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
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
        STCHECKRESULT(MI_VDEC_StopChn(i));
        STCHECKRESULT(MI_VDEC_DestroyChn(i));
    }
    ST_FB_Show(FALSE);
    if(bIsTimingChanging)
    {
        MI_HDMI_Stop(E_MI_HDMI_ID_0);
        for (i = 0; i < g_stES_Desc.u32WndNum; i++)
        {
            MI_DISP_DisableInputPort(ST_DISP_LAYER0, i);
        }
        MI_DISP_Disable(ST_DISP_DEV0);
	}
    else
    {
        ST_Hdmi_DeInit(E_MI_HDMI_ID_0);
        STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, g_stES_Desc.u32WndNum)); //disp input port 0
    }
    STCHECKRESULT(ST_Fb_DeInit());
}

void ST_FBUsage(void)
{
    printf("\t0: change to 720P\n");
    printf("\t1: change to 1080P\n");
    printf("\t2: change to 1600x1200\n");
}

void ST_FBDrawRectByPoint()
{
    MI_SYS_WindowRect_t Rect;
    int i = 0,j = 0,btnIndex = 0;
    int px = 0,py = 0,pointIndex = 0;
    MI_U8 *buf = NULL;
    MI_U32 _color = 0;
    MI_U8 selColVal = 0;

    for (btnIndex = 0; btnIndex < g_btn_nums; btnIndex++)
    {
        if(curBtnInx == btnIndex)
            selColVal = 100;  //add color when the button is selected
        else
            selColVal = 0;
        px = g_stFB_button[btnIndex].u16PosX;
        py = g_stFB_button[btnIndex].u16PosY;
        buf = g_stFB_button[btnIndex].u8DataBuf;
        pointIndex = 0;
        for (i = 0 ;i<g_stFB_button[btnIndex].u16Height;i++)
        {
            py++;
            px = g_stFB_button[btnIndex].u16PosX;       
            for(j=0;j<g_stFB_button[btnIndex].u16Width;j++)
            {
                 px++;
                 _color = ARGB2PIXEL8888(buf[pointIndex+0],buf[pointIndex+1]+selColVal,buf[pointIndex+2],buf[pointIndex+3]);
                 ST_Fb_FillPoint(px, py, _color);
                 pointIndex +=4;
            }  
          }
    }
}

void ST_ChangeResolution(ST_DispoutTiming_e enDispTiming)
{
    MI_SYS_WindowRect_t Rect;
    ST_Rect_t stVdispOutRect;
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;

    if(enDispTiming == g_DispTimingLast)
    {
        return ;
    }
    bIsTimingChanging = TRUE;
    STCHECKRESULT(ST_GetTimingInfo(enDispTiming,
                    &s32HdmiTiming, &s32DispTiming, &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    printf("%s %d disp timing:%d, hdmi timing:%d,w:%d,h:%d\n", __func__, __LINE__,
        s32DispTiming, s32HdmiTiming, stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);

    ST_FBVdecDeInit();
    ST_FBVdecInit(enDispTiming);
#if (FB_ENABLE == 1)
    mouse_setrange(0, 0, stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);
    ST_FBDrawRectByPoint();
    ST_FB_Show(TRUE);
    ST_Fb_SetColorKey(0x0);
    ST_Fb_InitMouse(32, 32, 2, (MI_U8*)"./ui_dat/cursor-arrow.dat");
    ST_Fb_MouseSet(1, 1);
#endif
    bIsTimingChanging = FALSE;
    g_DispTimingLast = enDispTiming;
    return ;
}

/* Mouse button bits*/
#define WHEEL_UP    0x10
#define WHEEL_DOWN  0x08

#define BUTTON_L    0x01
#define BUTTON_M    0x02
#define BUTTON_R    0x03
#define SCALE       1 /* default scaling factor for acceleration */
#define THRESH      1 /* default threshhold for acceleration */

static int xpos; /* current x position of mouse */
static int ypos; /* current y position of mouse */
static int minx; /* minimum allowed x position */
static int maxx; /* maximum allowed x position */
static int miny; /* minimum allowed y position */
static int maxy; /* maximum allowed y position */
// static int buttons; /* current state of buttons */

MI_U8 ST_Click_Listen(int event)
{
    int i = 0;
    MI_U16 u16RangX=0,u16RangY=0;

    //printf("file=%s,xpos:%d,ypos:%d, button:0x%X\n",__FILE__, xpos, ypos, event);
    if(event == BUTTON_L)
    {
        for(i=0;i<g_btn_nums;i++)
        {
            u16RangX = g_stFB_button[i].u16PosX + g_stFB_button[i].u16Width;
            u16RangY =  g_stFB_button[i].u16PosY +  g_stFB_button[i].u16Height;
            if((xpos > g_stFB_button[i].u16PosX)
                && (xpos < u16RangX)
                && (ypos > g_stFB_button[i].u16PosY)
                && (ypos < u16RangY))
            {
                return g_stFB_button[i].u8BtnIndex;
            }
        }
    }
    return 0xFF;
}

static int IMPS2_Read(int fd, int *dx, int *dy, int *dz, int *bp)
{
    static unsigned char buf[5];
    static int buttons[7] = {0, 1, 3, 0, 2, 0, 0};// 1:left button, 2: mid button, 3: right button
    static int nbytes = 0;
    int n;

    while ((n = read (fd, &buf [nbytes], 4 - nbytes)))
    {
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        nbytes += n;

        if (nbytes == 4)
        {
            int wheel;
            // printf("[luther.gliethttp]: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
            if ((buf[0] & 0xc0) != 0)
            {
                buf[0] = buf[1];
                buf[1] = buf[2];
                buf[2] = buf[3];
                nbytes = 3;
                return -1;
            }

            /* FORM XFree86 4.0.1 */
            *bp = buttons[(buf[0] & 0x07)];
            *dx = (buf[0] & 0x10) ? buf[1] - 256 : buf[1];
            *dy = (buf[0] & 0x20) ? -(buf[2] - 256) : -buf[2];

            /* Is a wheel event? */
            if ((wheel = buf[3]) != 0)
            {
                if(wheel > 0x7f)
                {
                    *bp |= WHEEL_UP;
                }
                else
                {
                    *bp |= WHEEL_DOWN;
                }
            }

            *dz = 0;
            nbytes = 0;
            return 1;
        }
    }

    return 0;
}

void mouse_setrange (int newminx, int newminy, int newmaxx, int newmaxy)
{
    minx = newminx;
    miny = newminy;
    maxx = newmaxx;
    maxy = newmaxy;
}

int mouse_update(int dx, int dy, int dz)
{
    int sign;

    sign = 1;
    if (dx < 0)
    {
        sign = -1;
        dx = -dx;
    }

    if (dx > THRESH)
        dx = THRESH + (dx - THRESH) * SCALE;

    dx *= sign;
    xpos += dx;

    if( xpos < minx )
        xpos = minx;
    if( xpos > maxx )
        xpos = maxx;

    sign = 1;
    if (dy < 0)
    {
        sign = -1;
        dy = -dy;
    }

    if (dy > THRESH)
         dy = THRESH + (dy - THRESH) * SCALE;

    dy *= sign;
    ypos += dy;

    if (ypos < miny)
        ypos = miny;

    if (ypos > maxy)
        ypos = maxy;

    return 1;
}

void *ST_FBMouseUserProc(void *args)
{
    int fd,retval;
    fd_set readfds;
    struct input_event inputEv[64];
    int readlen = 0;
    int i = 0;
    unsigned char buf[32];
    int x_pos, y_pos;
    unsigned char imps2_param [] = {243, 200, 243, 100, 243, 80};
    int dx, dy, dz, button;
    FB_DRAW_E enDraw = FB_DRAW_END;
    int old_xpos, old_ypos;
    MI_SYS_WindowRect_t stRect;
    MI_U32 u32ColorIndex = 0;
    MI_U32 u32Color[] = {ARGB888_RED, ARGB888_GREEN, ARGB888_BLACK};
    int d_xpos, d_ypos = 0;

    do
    {
        fd = open(MOUSE_DEV_NAME, O_RDWR);
        if (fd < 0)
        {
            printf("can not open %s\n", MOUSE_DEV_NAME);
        }
        sleep (5);
    } while (fd < 0);

    printf("open %s success, fd:%d\n", MOUSE_DEV_NAME, fd);

    write(fd, imps2_param, sizeof (imps2_param));

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        retval = select(fd + 1, &readfds, NULL, NULL, NULL);
        if(retval == 0)
        {
            printf("Time out!\n");
            continue;
        }

        if(FD_ISSET(fd, &readfds))
        {
            IMPS2_Read(fd, &dx, &dy, &dz, &button);

            mouse_update(dx, dy, dz);

            if (button == 0x1)
            {
                // left button down
                if (enDraw == FB_DRAW_END)
                {
                    enDraw = FB_DRAW_BEGIN;
                    old_xpos = xpos;
                    old_ypos = ypos;
                }
                else if (enDraw == FB_DRAW_BEGIN)
                {
                    enDraw = FB_DRAW_ING;
                }
            }
            else
            {
                // left button up
                enDraw = FB_DRAW_END;
            }

            if (button == 0x3)
            {
                // right button down
                u32ColorIndex ++;
            }
            //printf("xpos:%d,ypos:%d, button:0x%X\n", xpos, ypos, button);
            ST_Fb_MouseSet(xpos, ypos);
            if(button == BUTTON_L)
            {
                int selBtnIndex =  ST_Click_Listen(button);
                if((selBtnIndex != curBtnInx) && (selBtnIndex != 0xFF))
                {
                    curBtnInx = selBtnIndex;
                    ST_ChangeResolution(g_stFB_button[selBtnIndex].eTiming);
                }
            }
            old_xpos = xpos;
            old_ypos = ypos;
        }
    }
    close(fd);

    return NULL;
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

    s32Ms = 10;
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
        //memset(&stChnStat, 0x0, sizeof(stChnStat));
        //MI_VDEC_GetChnStat(s32VoChannel, &stChnStat);

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


int ST_FBVdecInit(ST_DispoutTiming_e eDispTiming)
{
    MI_U32 i = 0;
    ST_Rect_t stRect;
    MI_U32 u32Square = 0;
    ST_Rect_t stVdispOutRect;
    ST_Rect_t stDispWndRect[32] = {0};
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;
    MI_S32 s32Ret;
    MI_U32 u32WndNum_Disp = g_stES_Desc.u32WndNum;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_U32 u32DispInputPort = 0;

    stVdispOutRect.s32X     = 0;
    stVdispOutRect.s32Y     = 0;

    STCHECKRESULT(ST_GetTimingInfo(eDispTiming, &s32HdmiTiming, &s32DispTiming,
            &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, TRUE);

    if (g_stES_Desc.u32WndNum <= 1)
    {
        u32Square = 1;
    }
    else if (g_stES_Desc.u32WndNum <= 4)
    {
        u32Square = 2;
    }
    else if (g_stES_Desc.u32WndNum <= 9)
    {
        u32Square = 3;
    }
    else if (g_stES_Desc.u32WndNum <= 16)
    {
        u32Square = 4;
    }
    else if (g_stES_Desc.u32WndNum <= 25)
    {
        u32Square = 5;
    }
    else if (g_stES_Desc.u32WndNum <= 36)
    {
        u32Square = 6;
    }

    for (i = 0; i < g_stES_Desc.u32WndNum; i++)
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
        stVdecChnAttr.u32PicWidth   = 720;
        stVdecChnAttr.u32PicHeight  = 576;
        stVdecChnAttr.eCodecType    = g_stES_Desc.eCodecType;
        stVdecChnAttr.u32Priority   = 0;
        stVdecChnAttr.eDpbBufMode   = E_MI_VDEC_DPB_MODE_INPLACE_ONE_BUF;
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

        stVdecOutAttr.u16Width  = u16TempW > stVdecChnAttr.u32PicWidth ? stVdecChnAttr.u32PicWidth : u16TempW;
        stVdecOutAttr.u16Height = u16TempH > stVdecChnAttr.u32PicHeight? stVdecChnAttr.u32PicHeight: u16TempH;
        s32Ret = MI_VDEC_SetOutputPortAttr(vdecChn, &stVdecOutAttr);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_VDEC_SetOutputPortAttr error, chn:%d, %X\n", vdecChn, s32Ret);
            return 1;
        }

        printf("%s %d, start vdec chn %d success\n", __func__, __LINE__, vdecChn);
    }


    /************************************************
    step4:  start DISP device and chn
    *************************************************/
    printf("[%s][%d]timing:%d\n",__func__,__LINE__,s32DispTiming);
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32HdmiTiming));
    if(bIsTimingChanging)
    {
        memset(&stPubAttr, 0, sizeof(stPubAttr));
        stPubAttr.u32BgColor = YUYV_BLACK;
        stPubAttr.eIntfSync = s32DispTiming;
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        ExecFunc(MI_DISP_SetPubAttr(ST_DISP_DEV0, &stPubAttr), MI_SUCCESS);
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        ExecFunc(MI_DISP_SetPubAttr(ST_DISP_DEV0, &stPubAttr), MI_SUCCESS);
        ExecFunc(MI_DISP_Enable(ST_DISP_DEV0), MI_SUCCESS);
    }
    else
    {
        ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, s32DispTiming);
    }

#if (FB_ENABLE == 1)
    ST_Fb_Init();
    sleep(1);
#endif

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
        
        memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
        STCHECKRESULT(MI_VDEC_GetOutputPortAttr(i, &stVdecOutAttr));
        stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth = stVdecOutAttr.u16Width;
        stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight = stVdecOutAttr.u16Height;
    }

    if(bIsTimingChanging)
    {
        for (i = 0; i < stDispChnInfo.InputPortNum; i++)
        {
            memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
            u32DispInputPort = stDispChnInfo.stInputPortAttr[i].u32Port;
            MI_DISP_GetInputPortAttr(ST_DISP_LAYER0, u32DispInputPort, &stInputPortAttr);
            stInputPortAttr.stDispWin.u16X      = stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16X;
            stInputPortAttr.stDispWin.u16Y      = stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Y;
            stInputPortAttr.stDispWin.u16Width  = stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Width;
            stInputPortAttr.stDispWin.u16Height = stDispChnInfo.stInputPortAttr[i].stAttr.stDispWin.u16Height;
            stInputPortAttr.u16SrcWidth = stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcWidth;
            stInputPortAttr.u16SrcHeight = stDispChnInfo.stInputPortAttr[i].stAttr.u16SrcHeight;
            MI_DISP_SetInputPortAttr(ST_DISP_LAYER0, u32DispInputPort, &stInputPortAttr);
            MI_DISP_GetInputPortAttr(ST_DISP_LAYER0, u32DispInputPort, &stInputPortAttr);
            MI_DISP_EnableInputPort(ST_DISP_LAYER0, u32DispInputPort);
        }
    }
    else
    {
	    STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER0, &stDispChnInfo));
    }
    MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, FALSE);

#if (FB_ENABLE == 1)
    //fb size should match disp size,otherwise cursor xpos,ypos will be wrong
    ST_FB_ChangeResolution(stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);
#endif

    ST_Sys_BindInfo_t stBindInfo;

    /************************************************
    step6:  bind VDEC to DISP
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
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

            printf("%s %d vdec(%d)->disp(%d) bind\n", __func__, __LINE__, i, i);
            STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    }

    /************************************************
    step9:  send es stream to vdec
    *************************************************/
    for (i = 0; i < u32WndNum_Disp; i++)
    {
        g_stVdecThreadArgs[i].vdecChn = i;
        snprintf(g_stVdecThreadArgs[i].szFileName, sizeof(g_stVdecThreadArgs[i].szFileName) - 1,g_stES_Desc.szFilePath);
        g_stVdecThreadArgs[i].bRunFlag = TRUE;
        g_PushEsExit = FALSE;
        pthread_create(&g_stVdecThreadArgs[i].pt, NULL, ST_VdecSendStream, (void *)&g_stVdecThreadArgs[i]);
    }
    return 0;
}

int ST_AudioDeInit()
{
    g_stAud_Desc.runningFlag = FALSE;
    pthread_join(g_stAud_Desc.tid, NULL);
    /* disable ao channel of */
    ExecFunc(MI_AO_DisableChn(g_stAud_Desc.DevID, 0), MI_SUCCESS);
    /* disable ao device */
    ExecFunc(MI_AO_Disable(g_stAud_Desc.DevID), MI_SUCCESS);
    close(g_stAud_Desc.fdRd);
	return 0;
}

void ST_HandleSig(MI_S32 signo)
{
    int i = 0;
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit\n");
        ST_FBVdecDeInit();
        ST_AudioDeInit();
        ST_Sys_Exit();
        for (i = 0; i < g_btn_nums; i++)
            free(g_stFB_button[i].u8DataBuf);
        exit(0);
    }
}

void loadPicData()
{
#define picDataLen (20000)  //(r,g,b,a) 4 * Width * Height    
    FILE *fp = NULL;
    int btnIndex = 0;
    MI_U8 *u8TmpBuf;
    for (btnIndex = 0; btnIndex < g_btn_nums; btnIndex++)
    {        
        u8TmpBuf = (MI_U8*)malloc(picDataLen),
        memset(u8TmpBuf, 0x0, picDataLen);
        fp = fopen( g_stFB_button[btnIndex].u8TagFilePath, "rb");
        if (fp)
        {
           fread(u8TmpBuf,1, picDataLen, fp);
           g_stFB_button[btnIndex].u8DataBuf = u8TmpBuf;
           fclose(fp);
        }
    }
}

void *ST_AudioSendStream(void *args)
{
    MI_S32 s32RetSendStatus = 0;
    MI_U32 u32DmaBufSize = 0;
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    // file read/write
    MI_U8 au8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];
    MI_S32 s32RdSize;
    MI_S32 s32NeedSize;
    WaveFileHeader_t stWavHeaderInput;
    MI_AUDIO_Frame_t stAoSendFrame;
    
    if ( NULL == strstr((const char*)g_stAud_Desc.szFilePath, ".wav") )
    {
        printf("should input wav file\n");
        return 1;
    }
    
    /* open file  to read data and write data */
    g_stAud_Desc.fdRd = open((const char *)g_stAud_Desc.szFilePath, O_RDONLY, 0666);
    if(g_stAud_Desc.fdRd <= 0)
    {
        printf("Open input file failed:%s \n", g_stAud_Desc.szFilePath);
        printf("MI_AO error:%s", strerror(errno));
        return 1;
    }
    
    // read input wav file
    s32RdSize = read(g_stAud_Desc.fdRd, &stWavHeaderInput, sizeof(WaveFileHeader_t));
    
    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.u32ChnCnt = stWavHeaderInput.wave.wChannels;
    stSetAttr.eSoundmode = g_stAud_Desc.soundMode;
    stSetAttr.eSamplerate = g_stAud_Desc.eSamR;    
    
    /* set ao public attr*/
    ExecFunc(MI_AO_SetPubAttr(g_stAud_Desc.DevID, &stSetAttr), MI_SUCCESS);
    
    /* get ao device*/
    ExecFunc(MI_AO_GetPubAttr(g_stAud_Desc.DevID, &stGetAttr), MI_SUCCESS);
    
    /* enable ao device */
    ExecFunc(MI_AO_Enable(g_stAud_Desc.DevID), MI_SUCCESS);
    
    /* enable ao channel of device*/
    ExecFunc(MI_AO_EnableChn(g_stAud_Desc.DevID, 0), MI_SUCCESS);
    
    ExecFunc(MI_AO_SetVolume(g_stAud_Desc.DevID, g_stAud_Desc.s32SetVolumeDb), MI_SUCCESS);
    
    s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 2 * (stSetAttr.u32ChnCnt);
    if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_8K;;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_16K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_32K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSetAttr.eSamplerate)
    {
        u32DmaBufSize = DMA_BUF_SIZE_48K;
    }
    
    if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO)
    {
        if (s32NeedSize > (u32DmaBufSize / 4))
        {
            s32NeedSize = u32DmaBufSize / 4;
        }
    }
    else if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO)
    {
        if (s32NeedSize > (u32DmaBufSize / 8))
        {
            s32NeedSize = u32DmaBufSize / 8;
        }
    }
    
    printf("----------- s32NeedSize[%d] -------------\n", s32NeedSize);
    if( (g_stAud_Desc.fdRd >0))
    {
        while(g_stAud_Desc.runningFlag)
        {
            if(bIsTimingChanging)
            {
                MI_AO_ClearChnBuf(g_stAud_Desc.DevID, 0);
                usleep(100000);
                continue;
            }

            s32RdSize = read(g_stAud_Desc.fdRd, &au8TempBuf, s32NeedSize);
            if(s32RdSize != s32NeedSize)
            {
                lseek(g_stAud_Desc.fdRd, sizeof(WaveFileHeader_t), SEEK_SET);
                s32RdSize = read(g_stAud_Desc.fdRd, &au8TempBuf, s32NeedSize);
            }
            //read data and send to AO module
            stAoSendFrame.u32Len = s32RdSize;
            stAoSendFrame.apVirAddr[0] = au8TempBuf;
            stAoSendFrame.apVirAddr[1] = NULL;
    
            do{
                s32RetSendStatus = MI_AO_SendFrame(g_stAud_Desc.DevID, 0, &stAoSendFrame, 128);
            }while(s32RetSendStatus == MI_AO_ERR_NOBUF);
    
            if(s32RetSendStatus != MI_SUCCESS)
            {
                printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
            }
        }
    }
    return 0;
}


MI_BOOL checkResReady()
{
    int i = 0;
    if(-1 == access("./ui_dat/cursor-arrow.dat",R_OK))
    {
        ST_WARN("can't find cursor resource file ./ui_dat/cursor-arrow.dat\n");
        return FALSE;
    }
    if(-1 == access(g_stES_Desc.szFilePath,R_OK))
    {
        ST_WARN("can't find %s\n",g_stES_Desc.szFilePath);
        return FALSE;
    }
    if(-1 == access(g_stAud_Desc.szFilePath,R_OK))
    {
        ST_WARN("can't find %s\n",g_stAud_Desc.szFilePath);
        return FALSE;
    }
    for(i=0;i<g_btn_nums;i++)
    {
        if(-1 == access((const char *)g_stFB_button[i].u8TagFilePath,R_OK))
        {
            ST_WARN("can't find resource file %s\n",g_stFB_button[i].u8TagFilePath);
            return FALSE;
        }
    }
    return TRUE;
}

MI_S32 main(MI_S32 argc, char **argv)
{
    struct sigaction sigAction;
    sigAction.sa_handler = ST_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);

    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;
    ST_Rect_t stVdispOutRect;
    char ch = 0;
    MI_HDMI_InitParam_t stInitParam;

    ST_DispoutTiming_e enDispTimingCur = E_ST_TIMING_1080P_60;
    STCHECKRESULT(ST_GetTimingInfo(enDispTimingCur,
                    &s32HdmiTiming, &s32DispTiming, &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    ST_Sys_Init();
#if (FB_ENABLE == 1)
    if(!checkResReady())
        return 0;
    loadPicData();
#endif
    memset(&stInitParam, 0x0, sizeof(MI_HDMI_InitParam_t));
    stInitParam.pCallBackArgs = NULL;
    stInitParam.pfnHdmiEventCallback = Hdmi_callback;
    MI_HDMI_Init(&stInitParam);
    MI_HDMI_Open(E_MI_HDMI_ID_0);
    ST_FBVdecInit(enDispTimingCur);
#if (FB_ENABLE == 1)
    mouse_setrange(0, 0, stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);
    ST_FBDrawRectByPoint();
    ST_FB_Show(TRUE);
    ST_Fb_SetColorKey(0x0);
    ST_Fb_InitMouse(32, 32, 2, (MI_U8*)"./ui_dat/cursor-arrow.dat");
    ST_Fb_MouseSet(1, 1);
    pthread_create(&g_mouse_user_pt, NULL, ST_FBMouseUserProc, NULL);
#endif
    pthread_create(&g_stAud_Desc.tid, NULL, ST_AudioSendStream, NULL);

    ST_FBUsage();

    while (1)
    {
        ch = getchar();

        if(10 == ch)
        {
            ST_FBUsage();
            continue;
        }
        getchar();

        switch (ch)
        {
            case '0':
            {
                enDispTimingCur = E_ST_TIMING_720P_60;
            }
            break;

            case '1':
            {
                enDispTimingCur = E_ST_TIMING_1080P_60;
            }
            break;

            case '2':
            {
                enDispTimingCur = E_ST_TIMING_1600x1200_60;
            }
            break;
        }

        ST_ChangeResolution(enDispTimingCur);

        g_DispTimingLast = enDispTimingCur;
    }

    return 0;
}
