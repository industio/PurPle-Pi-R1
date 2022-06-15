#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"

#include "st_disp.h"

static MI_S32 _ST_Disp_DevInit(ST_DispDevInfo_t *pstDispDevInfo)
{
    MI_DISP_DEV DispDev = pstDispDevInfo->DispDev;
    MI_DISP_PubAttr_t stPubAttr;

    MI_DISP_LAYER DispLayer = pstDispDevInfo->DispLayer;
    MI_U32 u32Toleration = pstDispDevInfo->u32Toleration;
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    STDBG_ENTER();
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.u32BgColor = YUYV_BLACK;
    stPubAttr.eIntfSync = pstDispDevInfo->eTiming;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev, &stPubAttr), MI_SUCCESS);

    stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
    ExecFunc(MI_DISP_SetPubAttr(DispDev, &stPubAttr), MI_SUCCESS);

    ExecFunc(MI_DISP_Enable(DispDev), MI_SUCCESS);

    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width  = pstDispDevInfo->stLayerAttr.stVidLayerSize.u16Width;
    stLayerAttr.stVidLayerSize.u16Height = pstDispDevInfo->stLayerAttr.stVidLayerSize.u16Height;

    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
    stLayerAttr.stVidLayerDispWin.u16X      = pstDispDevInfo->stLayerAttr.stVidLayerDispWin.u16X;
    stLayerAttr.stVidLayerDispWin.u16Y      = pstDispDevInfo->stLayerAttr.stVidLayerDispWin.u16Y;
    stLayerAttr.stVidLayerDispWin.u16Width  = pstDispDevInfo->stLayerAttr.stVidLayerDispWin.u16Width;
    stLayerAttr.stVidLayerDispWin.u16Height = pstDispDevInfo->stLayerAttr.stVidLayerDispWin.u16Height;
    // must before set layer attribute

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_SUCCESS);
    printf("\n");
    printf("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width,
        stLayerAttr.stVidLayerSize.u16Height);
    printf("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y,
        stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    printf("\n");
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_SUCCESS);
    STDBG_LEAVE();

	return MI_SUCCESS;
}

MI_S32 ST_Disp_DevInit(MI_DISP_DEV dispDev, MI_DISP_LAYER DispLayer, MI_DISP_OutputTiming_e eTiming)
{
    ST_DispDevInfo_t stDispDevInfo;


    STDBG_ENTER();

    switch (eTiming)
    {
        case E_MI_DISP_OUTPUT_720P60:
        case E_MI_DISP_OUTPUT_720P50:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 720;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 720;
            break;
        case E_MI_DISP_OUTPUT_1080P60:
		case E_MI_DISP_OUTPUT_1080P50:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1920;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 1080;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1920;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 1080;
            break;
        case E_MI_DISP_OUTPUT_1024x768_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1024;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 768;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1024;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1280x1024_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 1024;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 1024;
            break;
        case E_MI_DISP_OUTPUT_1440x900_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1440;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 900;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1440;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 900;
            break;
        case E_MI_DISP_OUTPUT_1600x1200_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1600;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 1200;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1600;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 1200;
            break;
        case E_MI_DISP_OUTPUT_1280x800_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 800;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1280;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 800;
            break;
        case E_MI_DISP_OUTPUT_1366x768_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1366;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 768;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1366;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1680x1050_60:
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width = 1680;
            stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height = 1050;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y = 0;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width = 1680;
            stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height = 1050;
            break;
        default:
            printf("Unsupported timing!\n");
            return -1;
    }

    stDispDevInfo.eTiming   = eTiming;
    stDispDevInfo.DispDev   = dispDev;
    stDispDevInfo.DispLayer = DispLayer;
    printf("layer size(%d-%d) window(%d-%d-%d-%d)...\n",
        stDispDevInfo.stLayerAttr.stVidLayerSize.u16Width,
        stDispDevInfo.stLayerAttr.stVidLayerSize.u16Height,
        stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16X,
        stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Y,
        stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Width,
        stDispDevInfo.stLayerAttr.stVidLayerDispWin.u16Height);
    STCHECKRESULT(_ST_Disp_DevInit(&stDispDevInfo));

    STDBG_LEAVE();

    return MI_SUCCESS;
}

MI_S32 ST_Disp_ChnInit(MI_DISP_LAYER DispLayer, const ST_DispChnInfo_t *pstDispChnInfo)
{
    MI_U32 i = 0;
    MI_U32 u32InputPort = 0;
    MI_S32 InputPortNum = pstDispChnInfo->InputPortNum; //test use 0

    MI_DISP_InputPortAttr_t stInputPortAttr;

    STDBG_ENTER();
    for (i = 0; i < InputPortNum; i++)
    {
        memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

        u32InputPort = pstDispChnInfo->stInputPortAttr[i].u32Port;
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u32InputPort, &stInputPortAttr), MI_SUCCESS);
        stInputPortAttr.stDispWin.u16X      = pstDispChnInfo->stInputPortAttr[i].stAttr.stDispWin.u16X;
        stInputPortAttr.stDispWin.u16Y      = pstDispChnInfo->stInputPortAttr[i].stAttr.stDispWin.u16Y;
        stInputPortAttr.stDispWin.u16Width  = pstDispChnInfo->stInputPortAttr[i].stAttr.stDispWin.u16Width;
        stInputPortAttr.stDispWin.u16Height = pstDispChnInfo->stInputPortAttr[i].stAttr.stDispWin.u16Height;
        stInputPortAttr.u16SrcWidth = pstDispChnInfo->stInputPortAttr[i].stAttr.u16SrcWidth;
        stInputPortAttr.u16SrcHeight = pstDispChnInfo->stInputPortAttr[i].stAttr.u16SrcHeight;

        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u32InputPort, &stInputPortAttr), MI_SUCCESS);
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u32InputPort, &stInputPortAttr), MI_SUCCESS);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, u32InputPort), MI_SUCCESS);
        ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, u32InputPort, E_MI_DISP_SYNC_MODE_FREE_RUN), MI_SUCCESS);
    }
    STDBG_LEAVE();

	return MI_SUCCESS;
}

MI_S32 ST_Disp_DeInit(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer, MI_S32 s32InputPortNum)
{
    MI_S32 s32InputPort = 0;
    for (s32InputPort = 0; s32InputPort < s32InputPortNum; s32InputPort++)
    {
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, s32InputPort), MI_SUCCESS);
    }
    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_SUCCESS);
    ExecFunc(MI_DISP_Disable(DispDev), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_S32 ST_Disp_ShowStatus(MI_DISP_LAYER DispLayer, MI_S32 s32InputPortNum, MI_BOOL bIsShow)
{
    if (bIsShow)
    {
        ExecFunc(MI_DISP_ShowInputPort(DispLayer, s32InputPortNum), MI_SUCCESS);
    }
    else
    {
        ExecFunc(MI_DISP_HideInputPort(DispLayer, s32InputPortNum), MI_SUCCESS);

    }

    return MI_SUCCESS;
}
