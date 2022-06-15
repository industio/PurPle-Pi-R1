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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "st_main_venc.h"

#define ASCII_COLOR_RED                          "\033[1;31m"
#define ASCII_COLOR_WHITE                        "\033[1;37m"
#define ASCII_COLOR_YELLOW                       "\033[1;33m"
#define ASCII_COLOR_BLUE                         "\033[1;36m"
#define ASCII_COLOR_GREEN                        "\033[1;32m"
#define ASCII_COLOR_END                          "\033[0m"

#define DBG_INFO(fmt, args...)     ({do{printf(ASCII_COLOR_WHITE"[APP INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#define DBG_ERR(fmt, args...)      ({do{printf(ASCII_COLOR_RED  "[APP ERR ]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})

#define MAX_VENC_NUM    16
#define VENC_PERFORMANCE_TEST 0

typedef struct
{
    MI_BOOL bStart;
    MI_VENC_CHN vencChn;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32Bitrate;
    MI_U32 u32Qp;
    MI_VENC_ModType_e eModType;
    MI_VENC_RcMode_e eRcMode;
    int vencFd;
    pthread_t ptGetEs;
    pthread_t ptFillYuv;
} VENC_ChnCtx_t;

typedef enum
{
    E_VENC_Test_NULL,
    E_VENC_Test_InsertUserData,
    E_VENC_Test_ROI,
    E_VENC_Test_IDR_Config,
    E_VENC_Test_Slice_Split,
    E_VENC_Test_Intra_Pred,
    E_VENC_Test_Inter_Pred,
    E_VENC_Test_Trans,
    E_VENC_Test_Entropy,
    E_VENC_Test_VUI,
    E_VENC_Test_LTR,
    E_VENC_Test_Crop,
    E_VENC_Test_Super_Frame,
    E_VENC_Test_RateControl_Priority,
    E_VENC_Test_RateControl,
    E_VENC_Test_Deblocking,
    E_VENC_Test_DropFrame_strategy,
    E_VENC_Test_MAX
} ExtendParamType_e;

#define SUB_VENC_INTERVAL       16

static int g_yuv_width = 1920;
static int g_yuv_height = 1080;
static int g_yuv_format = 0;
static char *g_file_path = NULL;

static MI_BOOL g_bExit = FALSE;
static ExtendParamType_e g_u32ParamIndex = 0;
// static MI_S32 g_skipFrame = 0;
// static MI_S32 g_dumpFrame = 60;

VENC_ChnCtx_t g_stVencChnCtx;

void ST_Venc_Help(const char *porgName)
{
    printf("%s [-options] source.yuv\n", porgName);
    printf(" -s <width> <height> .... source resolution.\n");
    printf(" -p <yuv format> ........ 0: nv12, 1: yuyv.\n");
    printf(" -h ..................... print this help\n");
    exit(0);
}

void ST_Venc_ParseOptions(int argc, char **argv)
{
    if (argc <= 1)
    {
        ST_Venc_Help(argv[0]);
    }

    int i = 0;
    for (i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help"))
        {
            ST_Venc_Help(argv[0]);
        }
        else if (!strcmp(argv[i], "-s"))
        {
            g_yuv_width = atoi(argv[++i]);    // NOLINT
            g_yuv_height = atoi(argv[++i]);   // NOLINT
        }
        else if (!strcmp(argv[i], "-p"))
        {
            g_yuv_format = atoi(argv[++i]);
        }
        else if (g_file_path == NULL)
        {
            g_file_path = argv[i];
        }
    }
}

void ST_VencUsage(void)
{
    MI_U32 type, rcMode, qp, bitrate;
    char str[32];
    printf("1)\t H264\n");
    printf("2)\t H265\n");
    printf("3)\t Jpeg\n");
    scanf("%s", str);
    type = atoi(str);
    if(type == 1 || type == 2)
    {
        printf("1)\t CBR\n");
        printf("2)\t VBR\n");
        printf("3)\t FixQP\n");
        scanf("%s", str);
        rcMode = atoi(str);
        if(type == 1)
        {
            g_stVencChnCtx.eModType = E_MI_VENC_MODTYPE_H264E;
            if(rcMode == 1)
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
            }
            else if (rcMode == 2)
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
            }
            else
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            }
        }
        else
        {
            g_stVencChnCtx.eModType = E_MI_VENC_MODTYPE_H265E;
            if(rcMode == 1)
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
            }
            else if (rcMode == 2)
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            }
            else
            {
                g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
            }
        }

        if(rcMode == 1 || 2)
        {
            printf("bitrate(Kbps):");
            scanf("%s", str);
            bitrate = atoi(str);
            g_stVencChnCtx.u32Bitrate = bitrate * 1024;
        }
        else
        {
            printf("qp:");
            scanf("%s", str);
            qp = atoi(str);
            g_stVencChnCtx.u32Qp = qp;
        }
    }
    else
    {
        g_stVencChnCtx.eModType = E_MI_VENC_MODTYPE_JPEGE;
        g_stVencChnCtx.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    }
}

void ST_VencActionUsage()
{
    printf("1)\t Run\n");
    printf("2)\t SetParam\n");
}

void ST_VencExtendUsage()
{
    printf("1)\t Insert User Data\n");
    printf("2)\t ROI Config\n");
    printf("3)\t IDR Enable\n");
    printf("4)\t Slice Split\n");
    printf("5)\t Intra Pred\n");
    printf("6)\t Inter Pred\n");
    printf("7)\t Trans\n");
    printf("8)\t Entropy\n");
    printf("9)\t VUI\n");
    printf("10)\t LTR\n");
    printf("11)\t Crop\n");
    printf("12)\t Super Frame Config(Not supported)\n");
    printf("13)\t Rate Control Priority(Not Supported)\n");
    printf("14)\t Rate Control(need edit code)\n");
    printf("15)\t Deblocking\n");
    printf("16)\t Frame Lost Strategy(Not Supported)\n");
}

void ST_VENC_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit\n");

        g_bExit = TRUE;
        exit(0);
    }
}

void ST_VencSetParam(ExtendParamType_e index)
{
    MI_S32 s32Ret;
    MI_VENC_ChnAttr_t stChnAttr;

    s32Ret = MI_VENC_GetChnAttr(0, &stChnAttr);
    if(index == E_VENC_Test_InsertUserData)
    {
        s32Ret = MI_VENC_InsertUserData(0, _test_insert_data, sizeof(_test_insert_data));
        if(s32Ret)
            printf("%s %d, MI_VENC_InsertUserData error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_InsertUserData Success\n");
    }
    else if(index == E_VENC_Test_ROI)
    {
        g_set_roi = TRUE;
    }
    else if(index == E_VENC_Test_IDR_Config)
    {
        s32Ret = MI_VENC_EnableIdr(0, _test_idr_enable);
        if(s32Ret)
            printf("%s %d, MI_VENC_EnableIdr error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_EnableIdr Success\n");
        g_idr_config = TRUE;
    }
    else if(index == E_VENC_Test_Slice_Split)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264SliceSplit(0, &_test_split_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265SliceSplit(0, &_test_split_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetSliceSplit error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetSliceSplit Success\n");
    }
    else if(index == E_VENC_Test_Intra_Pred)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264IntraPred(0, &_test_intra_pred_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265IntraPred(0, &_test_intra_pred_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetIntraPred error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetIntraPred Success\n");
    }
    else if(index == E_VENC_Test_Intra_Pred)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264IntraPred(0, &_test_intra_pred_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265IntraPred(0, &_test_intra_pred_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetIntraPred error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetIntraPred Success\n");
    }
    else if(index == E_VENC_Test_Inter_Pred)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264InterPred(0, &_test_inter_pred_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265InterPred(0, &_test_inter_pred_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetInterPred error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetInterPred Success\n");
    }
    else if(index == E_VENC_Test_Trans)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264Trans(0, &_test_trans_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265Trans(0, &_test_trans_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetTrans error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetTrans Success\n");
    }
    else if(index == E_VENC_Test_Entropy)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264Entropy(0, &_test_entropy_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            printf("H.265 is not support SetEntropy\n");
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetEntropy error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetEntropy Success\n");
    }
    else if(index == E_VENC_Test_VUI)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264Vui(0, &_test_vui_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265Vui(0, &_test_vui_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetVui error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetVui Success\n");
    }
    else if(index == E_VENC_Test_LTR)
    {
        s32Ret = MI_VENC_SetRefParam(0, &_test_ltr);
        if(s32Ret)
            printf("%s %d, MI_VENC_SetRefParam error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetRefParam Success\n");
    }
    else if(index == E_VENC_Test_Crop)
    {
        s32Ret = MI_VENC_SetCrop(0, &_test_crop);
        if(s32Ret)
            printf("%s %d, MI_VENC_SetCrop error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetCrop Success\n");
    }
    else if(index == E_VENC_Test_Deblocking)
    {
        if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            s32Ret = MI_VENC_SetH264Dblk(0, &_test_dblk_264);
        }
        else if(stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            s32Ret = MI_VENC_SetH265Dblk(0, &_test_dblk_265);
        }

        if(s32Ret)
            printf("%s %d, MI_VENC_SetDblk error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetDblk Success\n");
    }
    else if(index == E_VENC_Test_Super_Frame)
    {
        s32Ret = MI_VENC_SetSuperFrameCfg(0, &_test_super_cfg);
        if(s32Ret)
            printf("%s %d, MI_VENC_SetSuperFrameCfg error, %X\n", __func__, __LINE__, s32Ret);
        else
            printf("MI_VENC_SetSuperFrameCfg Success\n");
    }
    else
    {
        printf("Not support\n");
    }

}

void ST_VencGetEsProc(void *args)
{
    VENC_ChnCtx_t *pCtx = (VENC_ChnCtx_t *)args;

    MI_SYS_ChnPort_t stVencChnInputPort;
    char szFileName[128];
    int fd = -1;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    MI_U32 u32DevId = 0;
    MI_S32 vencFd = -1;
    fd_set read_fds;

    MI_VENC_GetChnDevid(pCtx->vencChn, &u32DevId);

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnInputPort.u32DevId = u32DevId;
    stVencChnInputPort.u32ChnId = pCtx->vencChn;
    stVencChnInputPort.u32PortId = 0;

    memset(szFileName, 0, sizeof(szFileName));

    len = snprintf(szFileName, sizeof(szFileName) - 1, "venc_dev%d_chn%d_port%d_%dx%d.",
        stVencChnInputPort.u32DevId, stVencChnInputPort.u32ChnId, stVencChnInputPort.u32PortId,
        pCtx->u32Width, pCtx->u32Height);
    if (pCtx->eModType == E_MI_VENC_MODTYPE_H264E)
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "%s", "h264");
    }
    else if (pCtx->eModType == E_MI_VENC_MODTYPE_H265E)
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "%s", "h265");
    }
    else
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "%s", "mjpeg");
    }

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        printf("%s %d create %s error\n", __func__, __LINE__, szFileName);
        close(fd);
        return;
    }

    printf("%s %d create %s success\n", __func__, __LINE__, szFileName);

    vencFd = MI_VENC_GetFd(pCtx->vencChn);
    if(vencFd <= 0)
    {
        DBG_ERR("Unable to get FD:%d for chn:%2d\n", vencFd, pCtx->vencChn);
        close(fd);
        return;
    }
    else
    {
        printf("Venc Chn%2d FD:%d\n", pCtx->vencChn, vencFd);
    }

    while(pCtx->bStart)
    {
        struct timeval TimeoutVal;
        MI_VENC_ChnStat_t stStat;
        MI_VENC_Stream_t stStream;
        int i;
        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(vencFd, &read_fds);
        s32Ret = select(vencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            DBG_ERR("select failed\n");
            usleep(10 * 1000);
            continue;
        }
        else if (0 == s32Ret)
        {
            printf("select timeout\n");
            usleep(10 * 1000);
            continue;
        }
        else
        {
            if (FD_ISSET(vencFd, &read_fds))
            {
                #if (!VENC_PERFORMANCE_TEST)
                memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
                s32Ret = MI_VENC_Query(pCtx->vencChn, &stStat);
                if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
                {
                    DBG_ERR("%s %d, MI_VENC_Query error, %X\n", __func__, __LINE__, s32Ret);
                    usleep(10 * 1000);//sleep 10 ms
                    continue;
                }
                #endif

                memset(&stStream, 0, sizeof(MI_VENC_Stream_t));

                stStream.u32PackCount = 1;
                stStream.pstPack = (MI_VENC_Pack_t *)malloc(sizeof(MI_VENC_Pack_t) * stStream.u32PackCount);
                s32Ret = MI_VENC_GetStream(pCtx->vencChn, &stStream, 40);
                if (MI_SUCCESS == s32Ret)
                {
                #if (!VENC_PERFORMANCE_TEST)
                    printf("u32PackCount:%d, u32Seq:%d, offset:%d, len:%d, type:%d, pts:0x%llx\n", stStream.u32PackCount, stStream.u32Seq,
                        stStream.pstPack[0].u32Offset, stStream.pstPack[0].u32Len, stStream.pstPack[0].stDataType.eH264EType, stStream.pstPack[0].u64PTS);

                    if(pCtx->eModType == E_MI_VENC_MODTYPE_H264E)
                    {
                        printf("eRefType:%d\n", stStream.stH264Info.eRefType);
                    }
                    else if(pCtx->eModType == E_MI_VENC_MODTYPE_H265E)
                    {
                        printf("eRefType:%d\n", stStream.stH265Info.eRefType);
                    }

                    for (i = 0; i < stStream.u32PackCount; i ++)
                    {
                        write(fd, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,
                            stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
                    }
                #endif
                    MI_VENC_ReleaseStream(pCtx->vencChn, &stStream);
                }
                else
                {
                    DBG_ERR("%s %d, MI_VENC_GetStream error, %X\n", __func__, __LINE__, s32Ret);
                }
            }
        }
    }

    close(fd);
    s32Ret = MI_VENC_CloseFd(pCtx->vencChn);
    if(s32Ret != 0)
    {
        DBG_ERR("Chn%02d CloseFd error, Ret:%X\n", pCtx->vencChn, s32Ret);
    }
}

void *ST_VencFillYUVProc(void *args)
{
    VENC_ChnCtx_t *pCtx = (VENC_ChnCtx_t *)args;

    MI_SYS_ChnPort_t stVencChnInputPort;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    FILE *pFile = NULL;
    MI_U32 u32FrameSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32FilePos = 0;
    struct stat st;
    MI_U32 u32DevId = 0;
    MI_U8* u8YBuff = NULL;
    MI_U8* u8CBuff = NULL;

    memset(&stVencChnInputPort, 0, sizeof(MI_SYS_ChnPort_t));
    MI_VENC_GetChnDevid(pCtx->vencChn, &u32DevId);

    printf("%s %d, chn:%d, dev:%d\n", __func__, __LINE__, pCtx->vencChn, u32DevId);

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnInputPort.u32DevId = u32DevId;
    stVencChnInputPort.u32ChnId = pCtx->vencChn;
    stVencChnInputPort.u32PortId = 0;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.eFormat = (g_yuv_format == 0) ? E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 : E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.u16Width = g_yuv_width;
    stBufConf.stFrameCfg.u16Height = g_yuv_height;

    if ((s32Ret = stat(g_file_path, &st)) != 0)
    {
        printf("stat %s error:%x\n", g_file_path, s32Ret);
    }

    pFile = fopen(g_file_path, "rb");
    if (pFile == NULL)
    {
        printf("%s %d, open %s error\n", __func__, __LINE__, g_file_path);
        return NULL;
    }

    printf("open %s success, total size:%lld bytes\n", g_file_path, (long long)st.st_size);

    if(g_yuv_format == 0)
    {
        u32YSize = g_yuv_width * g_yuv_height;
        u32FrameSize = (u32YSize >> 1) + u32YSize;
    }
    else
    {
        u32YSize = g_yuv_width * g_yuv_height << 1;
        u32FrameSize = u32YSize;
    }

    printf("%s %d, chn:%d u32YSize:%d, u32FrameSize:%d\n", __func__, __LINE__, stVencChnInputPort.u32ChnId,
        u32YSize, u32FrameSize);

#if VENC_PERFORMANCE_TEST
    if(g_yuv_format == 0)
    {
        u8YBuff = malloc(u32YSize);
        u8CBuff = malloc(u32YSize >> 1);
        if (0 >= fread(u8YBuff, 1, u32YSize, pFile))
        {
            return NULL;
        }

        if (0 >= fread(u8CBuff, 1, u32YSize >> 1, pFile))
        {
            return NULL;
        }
    }
    else
    {
        u8YBuff = malloc(u32YSize);
        if (0 >= fread(u8YBuff, 1, u32YSize, pFile))
        {
            return NULL;
        }
    }
#endif

    while (pCtx->bStart)
    {

        memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

        u32FilePos = ftell(pFile);
        if ((st.st_size - u32FilePos) < u32FrameSize)
        {
            fseek(pFile, 0L, SEEK_SET);
        }

        s32Ret = MI_SYS_ChnInputPortGetBuf(&stVencChnInputPort, &stBufConf, &stBufInfo, &hHandle, 1000);
        if(MI_SUCCESS == s32Ret)
        {

        #if VENC_PERFORMANCE_TEST
            memcpy(stBufInfo.stFrameData.pVirAddr[0], u8YBuff, u32YSize);
            if(g_yuv_format == 0)
            {
                memcpy(stBufInfo.stFrameData.pVirAddr[1], u8CBuff, u32YSize >> 1);
            }
        #else

            if (0 >= fread(stBufInfo.stFrameData.pVirAddr[0], 1, u32YSize, pFile))
            {
                fseek(pFile, 0, SEEK_SET);

                stBufInfo.bEndOfStream = TRUE;
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                if (MI_SUCCESS != s32Ret)
                {
                    printf("%s %d, MI_SYS_ChnInputPortPutBuf error, %X\n", __func__, __LINE__, s32Ret);
                }
                continue;
            }

            if(g_yuv_format == 0)
            {
                if (0 >= fread(stBufInfo.stFrameData.pVirAddr[1], 1, u32YSize >> 1, pFile))
                {
                    fseek(pFile, 0, SEEK_SET);

                    stBufInfo.bEndOfStream = TRUE;
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                    if (MI_SUCCESS != s32Ret)
                    {
                        printf("%s %d, MI_SYS_ChnInputPortPutBuf error, %X\n", __func__, __LINE__, s32Ret);
                    }
                    continue;
                }
            }
        #endif

            s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s %d, MI_SYS_ChnInputPortPutBuf error, %X\n", __func__, __LINE__, s32Ret);
            }
        }
#if 0
        else
        {
            printf("%s %d, MI_SYS_ChnInputPortGetBuf error, chn:%d, %X\n",
                __func__, __LINE__, pCtx->vencChn, s32Ret);
        }
#endif
    }

    fclose(pFile);

    return NULL;
}

int ST_VencStart(void)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_CHN VencChn = 0;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_SYS_ChnPort_t stVencChnOutputPort;

    {
        memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
        memset(&stVencChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));

        if (E_MI_VENC_MODTYPE_H264E == g_stVencChnCtx.eModType)
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = true;

            ST_DBG("u32PicWidth:%d, u32PicHeight:%d\n", stChnAttr.stVeAttr.stAttrH264e.u32PicWidth,
                stChnAttr.stVeAttr.stAttrH264e.u32PicHeight);

            if(g_stVencChnCtx.eRcMode == E_MI_VENC_RC_MODE_H264FIXQP)
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = g_stVencChnCtx.u32Qp;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = g_stVencChnCtx.u32Qp;
            }
            else if(g_stVencChnCtx.eRcMode == E_MI_VENC_RC_MODE_H264CBR)
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = g_stVencChnCtx.u32Bitrate;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
            }
            else
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = g_stVencChnCtx.u32Bitrate;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp = 48;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp = 12;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
            }
        }
        else if (E_MI_VENC_MODTYPE_H265E == g_stVencChnCtx.eModType)
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = true;

            if(g_stVencChnCtx.eRcMode == E_MI_VENC_RC_MODE_H265FIXQP)
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp = g_stVencChnCtx.u32Qp;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp = g_stVencChnCtx.u32Qp;
            }
            else if(g_stVencChnCtx.eRcMode == E_MI_VENC_RC_MODE_H265CBR)
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = g_stVencChnCtx.u32Bitrate;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
            }
            else
            {
                stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = g_stVencChnCtx.u32Bitrate;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp = 48;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp = 12;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = 30;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            }
        }
        else if (E_MI_VENC_MODTYPE_JPEGE == g_stVencChnCtx.eModType)
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = g_stVencChnCtx.u32Width;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = g_stVencChnCtx.u32Height;
            stChnAttr.stVeAttr.stAttrJpeg.bByFrame = true;
        }

        s32Ret = MI_VENC_CreateChn(VencChn, &stChnAttr);
        if (MI_SUCCESS != s32Ret)
        {
            printf("%s %d, MI_VENC_CreateChn %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
        }
        printf("%s %d MI_VENC_CreateChn, vencChn:%d\n", __func__, __LINE__, VencChn);

        if (E_MI_VENC_MODTYPE_JPEGE == g_stVencChnCtx.eModType)
        {
            MI_VENC_ParamJpeg_t stParamJpeg;

            memset(&stParamJpeg, 0, sizeof(stParamJpeg));
            s32Ret = MI_VENC_GetJpegParam(VencChn, &stParamJpeg);
            if(s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
            printf("Get Qf:%d\n", stParamJpeg.u32Qfactor);

            stParamJpeg.u32Qfactor = 20;
            s32Ret = MI_VENC_SetJpegParam(VencChn, &stParamJpeg);
            if(s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
        }

        s32Ret = MI_VENC_SetMaxStreamCnt(VencChn, 4);
        if(MI_SUCCESS != s32Ret)
        {
            printf("%s %d, MI_VENC_SetMaxStreamCnt %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
        }

        ST_VencSetParam(g_u32ParamIndex);

        s32Ret = MI_VENC_StartRecvPic(VencChn);
        if (MI_SUCCESS != s32Ret)
        {
            printf("%s %d, MI_VENC_StartRecvPic %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
        }

#if 0
        memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
        s32Ret = MI_VENC_GetChnAttr(VencChn, &stChnAttr);
        if (MI_VENC_OK != s32Ret)
        {
            printf("%s %d, MI_VENC_GetChnAttr %d error, %X\n", __func__, __LINE__, VencChn, s32Ret);
        }

        printf("%s %d, u32MaxPicHeight:%d, u32MaxPicWidth:%d\n", __func__, __LINE__,
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight, stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth);
#endif

        if(g_set_roi)
        {
            s32Ret = MI_VENC_SetRoiCfg(0, &_test_roi_cfg);
            if(s32Ret)
                printf("%s %d, MI_VENC_SetRoiCfg error, %X\n", __func__, __LINE__, s32Ret);
            else
                printf("MI_VENC_SetRoiCfg Success\n");
        }

        g_stVencChnCtx.vencChn = VencChn;
        g_stVencChnCtx.vencFd = MI_VENC_GetFd(VencChn);

        printf("%s %d, venc:%d\n", __func__, __LINE__, VencChn);

        // get es stream
        pthread_create(&g_stVencChnCtx.ptGetEs, NULL, (void*)ST_VencGetEsProc, (void *)&g_stVencChnCtx);

        // put yuv data to venc
        pthread_create(&g_stVencChnCtx.ptFillYuv, NULL, (void*)ST_VencFillYUVProc, (void *)&g_stVencChnCtx);

    }

    return 0;
}

int ST_VencEnd(void)
{
    MI_VENC_CHN VencChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret = MI_VENC_StopRecvPic(VencChn);
    if(s32Ret)
        printf("%s %d, MI_VENC_StopRecvPic error, %X\n", __func__, __LINE__, s32Ret);
    else
        printf("MI_VENC_StopRecvPic Success\n");

    s32Ret = MI_VENC_DestroyChn(VencChn);
    if(s32Ret)
        printf("%s %d, MI_VENC_DestroyChn error, %X\n", __func__, __LINE__, s32Ret);
    else
        printf("MI_VENC_DestroyChn Success\n");

    return 0;
}


int main(int argc, char **argv)
{
    ST_Venc_ParseOptions(argc, argv);

    printf("source yuv info:  format=%d,width=%d,height=%d,file=%s\n",
        g_yuv_format, g_yuv_width, g_yuv_height, g_file_path);

    struct sigaction sigAction;
    char szCmd[16];
    MI_U32 u32Index = 0;

    sigAction.sa_handler = ST_VENC_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);

    ExecFunc(MI_SYS_Init(), MI_SUCCESS);
    memset(&g_stVencChnCtx, 0, sizeof(VENC_ChnCtx_t));
    g_stVencChnCtx.u32Width = g_yuv_width;
    g_stVencChnCtx.u32Height = g_yuv_height;

    ST_VencUsage();

    while (!g_bExit)
    {
        ST_VencActionUsage();
        scanf("%s", szCmd);
        u32Index = atoi(szCmd);
        if (u32Index == 2)
        {
            // Extend Test
            ST_VencExtendUsage();
            scanf("%s", szCmd);
            u32Index = atoi(szCmd);
            if(u32Index > 0 && u32Index < E_VENC_Test_MAX)
                g_u32ParamIndex = u32Index;
        }
        else if (u32Index == 1 && !g_stVencChnCtx.bStart)
        {
            g_stVencChnCtx.bStart = TRUE;
            ST_VencStart();
        }
        else
        {
            g_stVencChnCtx.bStart = FALSE;
            break;
        }
    }

    pthread_join(g_stVencChnCtx.ptGetEs, NULL);
    pthread_join(g_stVencChnCtx.ptFillYuv, NULL);

    ST_VencEnd();
    ExecFunc(MI_SYS_Exit(), MI_SUCCESS);

    return 0;
}

