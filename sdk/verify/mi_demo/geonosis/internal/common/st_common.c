#include <stdio.h>

#include "mi_sys.h"
#include "mi_disp.h"

#include "st_common.h"

#define DEFAULT_MAX_PICW 1920
#define DEFAULT_MAX_PICH 1080

MI_S32 ST_Sys_Init(void)
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;
    STCHECKRESULT(MI_SYS_Init()); //Sys Init
    memset(&stVersion, 0x0, sizeof(MI_SYS_Version_t));
    STCHECKRESULT(MI_SYS_GetVersion(&stVersion));
    ST_INFO("(%d) u8Version:%s\n", __LINE__, stVersion.u8Version);
    STCHECKRESULT(MI_SYS_GetCurPts(&u64Pts));
    ST_INFO("(%d) u64Pts:0x%llx\n", __LINE__, u64Pts);

    u64Pts = 0xF1237890F1237890;
    STCHECKRESULT(MI_SYS_InitPtsBase(u64Pts));

    u64Pts = 0xE1237890E1237890;
    STCHECKRESULT(MI_SYS_SyncPts(u64Pts));

    return MI_SUCCESS;
}

MI_S32 ST_Sys_Exit(void)
{
    STCHECKRESULT(MI_SYS_Exit());

    return MI_SUCCESS;
}

MI_S32 ST_Sys_Bind(ST_Sys_BindInfo_t *pstBindInfo)
{
    ExecFunc(MI_SYS_BindChnPort(&pstBindInfo->stSrcChnPort, &pstBindInfo->stDstChnPort, \
        pstBindInfo->u32SrcFrmrate, pstBindInfo->u32DstFrmrate), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_S32 ST_Sys_UnBind(ST_Sys_BindInfo_t *pstBindInfo)
{
    ExecFunc(MI_SYS_UnBindChnPort(&pstBindInfo->stSrcChnPort, &pstBindInfo->stDstChnPort), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_U64 ST_Sys_GetPts(MI_U32 u32FrameRate)
{
    if (0 == u32FrameRate)
    {
        return (MI_U64)(-1);
    }

    return (MI_U64)(1000 / u32FrameRate);
}

MI_S32 ST_Get_VoRectBySplitMode(MI_S32 s32SplitMode, MI_S32 s32VoChnIndex, MI_U16 u16LayerW, MI_U16 u16LayerH, ST_Rect_t *pstRect)
{
    MI_U16 u16LayerWidth = u16LayerW;
    MI_U16 u16LayerHeight = u16LayerH;
    MI_S32 s32DivW = 0, s32DivH = 0;
    MI_U16 u16VoWidth = 0, u16VoHeight = 0;
    MI_S32 s32SupportChnNum = 0;

    switch (s32SplitMode)
    {
        case E_ST_SPLIT_MODE_ONE:
        {
            s32DivW = 1;
            s32DivH = 1;
            s32SupportChnNum = 1;
            if (0 != s32VoChnIndex)
            {
                ST_ERR("s32VoChnIndex err(%d)!!!\n", s32VoChnIndex);
                return -1;
            }
            else
            {
                pstRect->s32X = 0;
                pstRect->s32Y = 0;
                pstRect->u16PicW = u16LayerWidth;
                pstRect->u16PicH = u16LayerHeight;
            }
            break;
        }
        case E_ST_SPLIT_MODE_TWO:
        {
            s32DivW = 4;
            s32DivH = 4;
            s32SupportChnNum = 2;
            u16VoWidth = u16LayerWidth / 4;//align
            u16VoHeight = u16LayerHeight / 4;
            if (0 == s32VoChnIndex)
            {
                pstRect->s32X = 0;
                pstRect->s32Y = 0;
                pstRect->u16PicW = u16LayerWidth;
                pstRect->u16PicH = u16LayerHeight;
            }
            else if (1 == s32VoChnIndex)
            {
                pstRect->s32X = (s32DivW - 1) * u16VoWidth;
                pstRect->s32Y = (s32DivH - 1) * u16VoHeight;
                pstRect->u16PicW = u16VoWidth;
                pstRect->u16PicH = u16VoHeight;
            }
            break;
        }
        case E_ST_SPLIT_MODE_FOUR:
        {
            s32DivW = 2;
            s32DivH = 2;
            s32SupportChnNum = 4;
            u16VoWidth = u16LayerWidth / 2;//align
            u16VoHeight = u16LayerHeight / 2;
            if ((s32VoChnIndex < s32SupportChnNum) && (s32VoChnIndex >=0))
            {
                pstRect->s32X = (s32VoChnIndex % s32DivW) * u16VoWidth;
                pstRect->s32Y = (s32VoChnIndex / s32DivW) * u16VoHeight;
                pstRect->u16PicW = u16VoWidth;
                pstRect->u16PicH = u16VoHeight;
            }
            else
            {
                ST_ERR("s32VoChnIndex err(%d)!!!\n", s32VoChnIndex);
                return -1;
            }
            break;
        }
        case E_ST_SPLIT_MODE_NINE:
        {
            s32DivW = 3;
            s32DivH = 3;
            s32SupportChnNum = 9;
            u16VoWidth = u16LayerWidth / 3;//align
            u16VoHeight = u16LayerHeight / 3;
            if ((s32VoChnIndex < s32SupportChnNum) && (s32VoChnIndex >=0))
            {
                pstRect->s32X = (s32VoChnIndex % s32DivW) * u16VoWidth;
                pstRect->s32Y = (s32VoChnIndex / s32DivW) * u16VoHeight;
                pstRect->u16PicW = u16VoWidth;
                pstRect->u16PicH = u16VoHeight;
            }
            else
            {
                ST_ERR("s32VoChnIndex err(%d)!!!\n", s32VoChnIndex);
                return -1;
            }
            break;
        }
        case E_ST_SPLIT_MODE_NINE_EX: //1 big win + 5 litte win
        {
            s32DivW = 3;
            s32DivH = 3;
            s32SupportChnNum = 6;
            u16VoWidth = u16LayerWidth / 3;//align
            u16VoHeight = u16LayerHeight / 3;
            break;
        }
        default:
            ST_ERR("Unsupported split mode(%d)!!!\n", s32SplitMode);
            return -1;
    }

    return MI_SUCCESS;
}

MI_S32 ST_GetTimingInfo(MI_S32 s32ApTiming, MI_S32 *ps32HdmiTiming, MI_S32 *ps32DispTiming, MI_U16 *pu16DispW, MI_U16 *pu16DispH)
{
    switch (s32ApTiming)
    {
        case E_ST_TIMING_720P_50:
            *ps32DispTiming = E_MI_DISP_OUTPUT_720P50;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_720_50P;
            *pu16DispW = 1280;
            *pu16DispH = 720;
            break;
        case E_ST_TIMING_720P_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_720P60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_720_60P;
            *pu16DispW = 1280;
            *pu16DispH = 720;
            break;
        case E_ST_TIMING_1080P_50:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1080P50;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1080_50P;
            *pu16DispW = 1920;
            *pu16DispH = 1080;
            break;
        case E_ST_TIMING_1080P_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1080P60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1080_60P;
            *pu16DispW = 1920;
            *pu16DispH = 1080;
            break;
        case E_ST_TIMING_1600x1200_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1600x1200_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P;
            *pu16DispW = 1600;
            *pu16DispH = 1200;
            break;
        case E_ST_TIMING_1440x900_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1440x900_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1440x900_60P;
            *pu16DispW = 1440;
            *pu16DispH = 900;
            break;
        case E_ST_TIMING_1280x1024_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1280x1024_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P;
            *pu16DispW = 1280;
            *pu16DispH = 1024;
            break;
        case E_ST_TIMING_1024x768_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1024x768_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1024x768_60P;
            *pu16DispW = 1024;
            *pu16DispH = 768;
            break;
        case E_ST_TIMING_1280x800_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1280x800_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1280x800_60P;
            *pu16DispW = 1280;
            *pu16DispH = 800;
            break;
        case E_ST_TIMING_1366x768_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1366x768_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1366x768_60P;
            *pu16DispW = 1366;
            *pu16DispH = 768;
            break;
        case E_ST_TIMING_1680x1050_60:
            *ps32DispTiming = E_MI_DISP_OUTPUT_1680x1050_60;
            *ps32HdmiTiming = E_MI_HDMI_TIMING_1680x1050_60P;
            *pu16DispW = 1680;
            *pu16DispH = 1050;
            break;
        default:
            ST_WARN("Unspport Ap timing (%d)\n", s32ApTiming);
            return -1;
    }

    return MI_SUCCESS;
}
