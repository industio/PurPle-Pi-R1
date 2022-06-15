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
#include "st_fb.h"
#include "st_common.h"
#include "st_disp.h"

#define MOUSE_AUTO_RUN      0
#define MOUSE_USER_MOVE     1

#define MOUSE_DEV_NAME   "/dev/input/mice"

typedef struct
{
    pthread_t ptVdec;
    ST_DispoutTiming_e enDispTiming;
    MI_VDEC_CHN vdecChn;
    char szFileName[64];
    MI_BOOL bRunFlag;
} FB_Thread_Args_t;

typedef enum
{
    FB_DRAW_BEGIN = 0,
    FB_DRAW_ING,
    FB_DRAW_END,
} FB_DRAW_E;

FB_Thread_Args_t g_st_fb_args;

pthread_t g_mouse_auto_pt;
static MI_BOOL g_mousemove_flag = TRUE;

pthread_t g_mouse_user_pt;

int ST_FBVdecInit(ST_DispoutTiming_e enDispTiming)
{
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN vdecChn = 0;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_DIVP_CHN divpChn = 0;
    ST_Sys_BindInfo_t stBindInfo;
    ST_Rect_t stVdispOutRect;
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;

    STCHECKRESULT(ST_GetTimingInfo(enDispTiming,
                    &s32HdmiTiming, &s32DispTiming, &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    // create VDEC
    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
    stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize    = 1 * 1024 * 1024;
    stVdecChnAttr.u32PicWidth   = 1920;
    stVdecChnAttr.u32PicHeight  = 1080;
    stVdecChnAttr.u32Priority   = 0;
    stVdecChnAttr.eCodecType    = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_INPLACE_ONE_BUF;
    ExecFunc(MI_VDEC_CreateChn(vdecChn, &stVdecChnAttr), MI_SUCCESS);
    ExecFunc(MI_VDEC_StartChn(vdecChn), MI_SUCCESS);

    MI_VDEC_OutputPortAttr_t stVdecOutAttr;
    stVdecOutAttr.u16Width = MIN(1920, stVdispOutRect.u16PicW);
    stVdecOutAttr.u16Height = MIN(1080, stVdispOutRect.u16PicH);
    ExecFunc(MI_VDEC_SetOutputPortAttr(vdecChn, &stVdecOutAttr), MI_SUCCESS);

    //
    ST_DispChnInfo_t stDispChnInfo;

    memset(&stDispChnInfo, 0, sizeof(ST_DispChnInfo_t));
    stDispChnInfo.InputPortNum = 1;

    stDispChnInfo.stInputPortAttr[0].u32Port = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = stVdispOutRect.u16PicW;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = stVdispOutRect.u16PicH;

    memset(&stVdecOutAttr, 0, sizeof(stVdecOutAttr));
    STCHECKRESULT(MI_VDEC_GetOutputPortAttr(vdecChn, &stVdecOutAttr));
    stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcWidth = stVdecOutAttr.u16Width;
    stDispChnInfo.stInputPortAttr[0].stAttr.u16SrcHeight = stVdecOutAttr.u16Height;
    STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER0, &stDispChnInfo));

    // bind VDEC->DISP
    memset(&stBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0; //only equal zero
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 0;
    stBindInfo.u32DstFrmrate = 0;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
}

void ST_FBUsage(void)
{
    printf("\t0: change to 720P\n");
    printf("\t1: change to 1080P\n");
    printf("\t2: change to 1600x1200\n");
}

void ST_FBDrawRect()
{
    MI_SYS_WindowRect_t Rect;

    Rect.u16X = 100;
    Rect.u16Y = 100;
    Rect.u16Width = 200;
    Rect.u16Height = 200;
    printf("1 R:%X,G:%X,B:%X\n", PIXEL8888RED(ARGB888_RED), PIXEL8888GREEN(ARGB888_RED), PIXEL8888BLUE(ARGB888_RED));
    ST_Fb_FillRect(&Rect, ARGB888_RED);

    Rect.u16X = 400;
    Rect.u16Y = 100;
    Rect.u16Width = 200;
    Rect.u16Height = 200;
    printf("2 R:%X,G:%X,B:%X\n", PIXEL8888RED(ARGB888_GREEN), PIXEL8888GREEN(ARGB888_GREEN), PIXEL8888BLUE(ARGB888_GREEN));
    ST_Fb_FillRect(&Rect, ARGB888_GREEN);

    Rect.u16X = 800;
    Rect.u16Y = 100;
    Rect.u16Width = 200;
    Rect.u16Height = 200;
    printf("3 R:%X,G:%X,B:%X\n", PIXEL8888RED(ARGB888_BLUE), PIXEL8888GREEN(ARGB888_BLUE), PIXEL8888BLUE(ARGB888_BLUE));
    ST_Fb_FillRect(&Rect, ARGB888_BLUE);

    Rect.u16X = 800;
    Rect.u16Y = 400;
    Rect.u16Width = 200;
    Rect.u16Height = 200;
    printf("4 R:%X,G:%X,B:%X\n", PIXEL8888RED(ARGB888_BLUE), PIXEL8888GREEN(ARGB888_BLUE), PIXEL8888BLUE(ARGB888_BLUE));
    ST_Fb_FillRect(&Rect, ARGB888_RED);

    //ST_Fb_FillCircle(600, 600, 200, ARGB888_RED);
}


int ST_FBChangeResolution(ST_DispoutTiming_e enDispTiming)
{
    MI_SYS_WindowRect_t Rect;
    ST_Rect_t stVdispOutRect;
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;

    STCHECKRESULT(ST_GetTimingInfo(enDispTiming,
                    &s32HdmiTiming, &s32DispTiming, &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    printf("%s %d disp timing:%d, hdmi timing:%d,w:%d,h:%d\n", __func__, __LINE__,
        s32DispTiming, s32HdmiTiming, stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);

    /*
    (1) stop HDMI
    (2) stop DISP and disable input port
    (3) start disp
    (4) start HDMI
    (5) set disp input port attribute and enable
    */

    // stop move cursor
    g_mousemove_flag = FALSE;

    // stop hdmi
    ExecFunc(MI_HDMI_Stop(E_MI_HDMI_ID_0), MI_SUCCESS);

    // stop disp dev and disable input port
    ExecFunc(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1), MI_SUCCESS);

    // start disp
    ExecFunc(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, s32DispTiming), MI_SUCCESS);

    // start HDMI
    ExecFunc(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32HdmiTiming), MI_SUCCESS);

    // change fb resolution
    ST_FB_ChangeResolution(stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);

    // redraw rect
    ST_FBDrawRect();
    // start move cursor
    g_mousemove_flag = TRUE;

    // set disp attribute
    ST_DispChnInfo_t stDispChnInfo;

    memset(&stDispChnInfo, 0, sizeof(ST_DispChnInfo_t));
    stDispChnInfo.InputPortNum = 1;
    stDispChnInfo.stInputPortAttr[0].u32Port = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = ALIGN_BACK(stVdispOutRect.u16PicW, 16);
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = ALIGN_BACK(stVdispOutRect.u16PicH, 2);
    STCHECKRESULT(ST_Disp_ChnInit(ST_DISP_LAYER0, &stDispChnInfo));
}

void *ST_FBVdecProc(void *args)
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
    FB_Thread_Args_t *pArgs = (FB_Thread_Args_t *)args;

    MI_S32 readfd = 0, s32ReadCnt = 0;
    MI_U8 *pu8Buf = NULL;
    MI_S32 s32Len = 0;
    MI_U32 u32FrameLen = 0;
    MI_U64 u64Pts = 0;
    MI_U8 au8Header[16] = {0};
    MI_U32 u32Pos = 0;
    MI_VDEC_ChnStat_t stChnStat;
    MI_VDEC_VideoStream_t stVdecStream;

    char tname[32];
    memset(tname, 0, 32);

    ST_FBVdecInit(pArgs->enDispTiming);

    vdecChn = 0;
    snprintf(tname, 32, "push_t_%u", vdecChn);
    prctl(PR_SET_NAME, tname);

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = vdecChn;
    stChnPort.u32PortId = 0;

    readfd = open(pArgs->szFileName,  O_RDONLY, 0); //ES
    if (0 > readfd)
    {
        printf("Open %s failed!\n", pArgs->szFileName);
        return NULL;
    }

    // s32Ms = _stTestParam.stChannelInfo[s32VoChannel].s32PushDataMs;
    // bVdecChnEnable = _stTestParam.stChannelInfo[0].bVdecChnEnable;

    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufConf.u64TargetPts = 0;
    pu8Buf = malloc(NALU_PACKET_SIZE);

    s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s %d, MI_SYS_SetChnOutputPortDepth error, %X\n", __func__, __LINE__, s32Ret);
        return NULL;
    }

    //stChnPort.eModId = E_MI_MODULE_ID_DIVP;
    //STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5));

    s32Ms = 33;
    printf("----------------------%d------------------\n", stChnPort.u32ChnId);
    while (1)
    {
        usleep(s32Ms * 1000);//33

        if (pArgs->bRunFlag == FALSE)
        {
            continue;
        }

        memset(au8Header, 0, 16);
        u32Pos = lseek(readfd, 0L, SEEK_CUR);
        s32Len = read(readfd, au8Header, 16);
        if(s32Len <= 0)
        {
            lseek(readfd, 0, SEEK_SET);
            continue;
        }

        u32FrameLen = MI_U32VALUE(au8Header, 4);
        // printf("vdecChn:%d, u32FrameLen:%d\n", vdecChn, u32FrameLen);
        if(u32FrameLen > NALU_PACKET_SIZE)
        {
            lseek(readfd, 0, SEEK_SET);
            continue;
        }
        s32Len = read(readfd, pu8Buf, u32FrameLen);
        if(s32Len <= 0)
        {
            lseek(readfd, 0, SEEK_SET);
            continue;
        }

        stVdecStream.pu8Addr = pu8Buf;
        stVdecStream.u32Len = s32Len;
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;
        if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
        {
            printf("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", vdecChn, s32Ret);
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

    printf("End----------------------%d------------------End\n", stChnPort.u32ChnId);

    return NULL;
}

/* Mouse button bits*/
#define WHEEL_UP    0x10
#define WHEEL_DOWN  0x08

#define BUTTON_L    0x04
#define BUTTON_M    0x02
#define BUTTON_R    0x01
#define SCALE       1 /* default scaling factor for acceleration */
#define THRESH      1 /* default threshhold for acceleration */

static int xpos; /* current x position of mouse */
static int ypos; /* current y position of mouse */
static int minx; /* minimum allowed x position */
static int maxx; /* maximum allowed x position */
static int miny; /* minimum allowed y position */
static int maxy; /* maximum allowed y position */
// static int buttons; /* current state of buttons */

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
    int r;
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

            if (enDraw == FB_DRAW_BEGIN ||
                enDraw == FB_DRAW_ING)
            {
                // ST_Fb_FillPoint(xpos, ypos, ARGB888_RED);
                // ST_Fb_FillLine(ARGB888_RED);
                // stRect.u16X = xpos;
                // stRect.u16Y = ypos - 10;
                // stRect.u16Width = 20;
                // stRect.u16Height = 20;
                // ST_Fb_FillRect(&stRect, u32Color[u32ColorIndex % ARRAY_SIZE(u32Color)]);

                d_xpos = abs(xpos - old_xpos);
                d_ypos = abs(ypos - old_ypos);
                if (d_xpos > 0)
                {
                    for (i = MIN(xpos, old_xpos); i < MAX(xpos, old_xpos); i ++)
                    {
                        ST_Fb_FillCircle(i, ypos, 10, u32Color[u32ColorIndex % ARRAY_SIZE(u32Color)]);
                    }
                }

                if (d_ypos > 0)
                {
                    for (i = MIN(ypos, old_ypos); i < MAX(ypos, old_ypos); i ++)
                    {
                        ST_Fb_FillCircle(xpos, i, 10, u32Color[u32ColorIndex % ARRAY_SIZE(u32Color)]);
                    }
                }
            }

            // printf("xpos:%d,ypos:%d, button:0x%X\n", xpos, ypos, button);
            ST_Fb_MouseSet(xpos, ypos);
            old_xpos = xpos;
            old_ypos = ypos;
        }
    }

    close(fd);

    return NULL;
}

void *ST_FBMouseMoveProc(void *args)
{
    MI_FB_GlobalAlpha_t stAlphaInfo;
    MI_U32 count = 1;
    MI_U8 u8GlobalAlpha = 0, u8Space = 0x10;

    while (1)
    {
        if (g_mousemove_flag == FALSE)
        {
            usleep(1 * 1000 * 1000);
            continue;
        }

        if (count ++ % 100 == 0)
        {
            u8GlobalAlpha = u8GlobalAlpha + u8Space;
            if (u8GlobalAlpha > 0xFF)
            {
                u8GlobalAlpha = 0;
            }
            memset(&stAlphaInfo, 0, sizeof(MI_FB_GlobalAlpha_t));
            ST_FB_GetAlphaInfo(&stAlphaInfo);
            stAlphaInfo.bAlphaEnable = TRUE;
            stAlphaInfo.bAlphaChannel= TRUE;
            stAlphaInfo.u8GlobalAlpha = u8GlobalAlpha;
            ST_FB_SetAlphaInfo(&stAlphaInfo);
            // printf("%s %d\n", __func__, __LINE__);
        }

        ST_Fb_MouseSet(rand() % 1000, rand() % 1000);

        usleep(100 * 1000);
    }
}

MI_S32 main(MI_S32 argc, MI_U8 **argv)
{
    ST_Rect_t stVdispOutRect;
    MI_S32 s32HdmiTiming = 0, s32DispTiming = 0;
    char ch = 0;
    ST_DispoutTiming_e enDispTimingCur = E_ST_TIMING_1080P_60;//E_ST_TIMING_1080P_60;
    ST_DispoutTiming_e enDispTimingLast = enDispTimingCur;

    ST_Sys_Init();

    STCHECKRESULT(ST_GetTimingInfo(enDispTimingCur,
                    &s32HdmiTiming, &s32DispTiming, &stVdispOutRect.u16PicW, &stVdispOutRect.u16PicH));

    ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, s32DispTiming);

    // init hdmi
    STCHECKRESULT(ST_Hdmi_Init());
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, s32HdmiTiming));

    ST_Fb_Init();

    sleep(1);
    //ST_FBChangeResolution(enDispTimingCur);

	// after change resolution, fb default size is 1280x720, if DISP init at 1024x768,
    // set color format will fail
    //ST_Fb_SetColorFmt(E_MI_FB_COLOR_FMT_ARGB1555);

    mouse_setrange(0, 0, stVdispOutRect.u16PicW, stVdispOutRect.u16PicH);

    memset(&g_st_fb_args, 0, sizeof(FB_Thread_Args_t));
    g_st_fb_args.enDispTiming = enDispTimingCur;
    g_st_fb_args.bRunFlag = TRUE;
    snprintf(g_st_fb_args.szFileName, sizeof(g_st_fb_args.szFileName) - 1, ST_1080P_H264_25_FILE);
    pthread_create(&g_st_fb_args.ptVdec, NULL, ST_FBVdecProc, &g_st_fb_args);

    ST_FBDrawRect();

    ST_FB_Show(TRUE);

    ST_Fb_SetColorKey(0x0);

    MI_FB_GlobalAlpha_t stAlphaInfo;

    memset(&stAlphaInfo, 0, sizeof(MI_FB_GlobalAlpha_t));
    ST_FB_GetAlphaInfo(&stAlphaInfo);
    printf("FBIOGET_GLOBAL_ALPHA alpha info: alpha blend enable=%d,Multialpha enable=%d,Global Alpha=%d,u8Alpha0=%d,u8Alpha1=%d\n",
        stAlphaInfo.bAlphaEnable,stAlphaInfo.bAlphaChannel,stAlphaInfo.u8GlobalAlpha,stAlphaInfo.u8Alpha0,stAlphaInfo.u8Alpha1);
    stAlphaInfo.bAlphaEnable = TRUE;
    stAlphaInfo.bAlphaChannel= TRUE;
    stAlphaInfo.u8GlobalAlpha = 0x70;
    ST_FB_SetAlphaInfo(&stAlphaInfo);

    ST_Fb_InitMouse(32, 32, 2, "./cursor-arrow.dat");
    ST_Fb_MouseSet(1, 1);

#if MOUSE_AUTO_RUN
    pthread_create(&g_mouse_auto_pt, NULL, ST_FBMouseMoveProc, NULL);
#endif

#if MOUSE_USER_MOVE
    pthread_create(&g_mouse_user_pt, NULL, ST_FBMouseUserProc, NULL);
#endif

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

        if (enDispTimingCur == enDispTimingLast)
        {
            printf("same timing, do nothing\n");
            continue;
        }

        ST_FBChangeResolution(enDispTimingCur);

        enDispTimingLast = enDispTimingCur;
    }

    return 0;
}
