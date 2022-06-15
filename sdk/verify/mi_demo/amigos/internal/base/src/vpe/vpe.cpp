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
#include <string.h>

#include <vector>
#include <string>
#include "mi_sys.h"
#include "mi_sensor.h"
#include "mi_vpe.h"
#include "vpe.h"

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct VPE_ChannelInfo_s
{
    MI_U16 u16VpeMaxW;
    MI_U16 u16VpeMaxH;
    MI_U16 u16VpeCropW;
    MI_U16 u16VpeCropH;
    MI_S32 u32X;
    MI_S32 u32Y;
    MI_SYS_PixelFormat_e eFormat;
    MI_VPE_RunningMode_e eRunningMode;
    MI_VPE_HDRType_e eHDRtype;
    MI_VPE_3DNR_Level_e e3DNRLevel;
    MI_BOOL bRotation;
    MI_VPE_SensorChannel_e eBindSensorId;
    MI_U32   u32ChnPortMode;
} VPE_ChannelInfo_t;

typedef struct VPE_PortInfo_s
{
    MI_VPE_CHANNEL DepVpeChannel;
    MI_U16 u16OutputWidth;                   // Width of target image
    MI_U16 u16OutputHeight;                  // Height of target image
    MI_SYS_PixelFormat_e  ePixelFormat;      // Pixel format of target image
    MI_SYS_CompressMode_e eCompressMode;     // Compression mode of the output
} VPE_PortInfo_t;

static MI_S32 Vpe_CreateChannel(MI_VPE_CHANNEL VpeChannel, VPE_ChannelInfo_t *pstChannelInfo)
{
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_VPE_ChannelPara_t stChannelVpeParam;

    memset(&stChannelVpeAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
    memset(&stChannelVpeParam, 0x00, sizeof(MI_VPE_ChannelPara_t));


    stChannelVpeAttr.u16MaxW = pstChannelInfo->u16VpeMaxW;
    stChannelVpeAttr.u16MaxH = pstChannelInfo->u16VpeMaxH;
    stChannelVpeAttr.bNrEn= FALSE;
    stChannelVpeAttr.bEdgeEn= FALSE;
    stChannelVpeAttr.bEsEn= FALSE;
    stChannelVpeAttr.bContrastEn= FALSE;
    stChannelVpeAttr.bUvInvert= FALSE;
    stChannelVpeAttr.ePixFmt = pstChannelInfo->eFormat;
    stChannelVpeAttr.eRunningMode = pstChannelInfo->eRunningMode;
    stChannelVpeAttr.bRotation = pstChannelInfo->bRotation;
    stChannelVpeAttr.eHDRType  = pstChannelInfo->eHDRtype;
    stChannelVpeAttr.eSensorBindId = pstChannelInfo->eBindSensorId;
    stChannelVpeAttr.u32ChnPortMode = pstChannelInfo->u32ChnPortMode;
    MI_VPE_CreateChannel(VpeChannel, &stChannelVpeAttr);
    stChannelVpeParam.eHDRType = pstChannelInfo->eHDRtype;
    stChannelVpeParam.e3DNRLevel = pstChannelInfo->e3DNRLevel;
    MI_VPE_SetChannelParam(VpeChannel, &stChannelVpeParam);

    return MI_SUCCESS;
}

static MI_S32 Vpe_DestroyChannel(MI_VPE_CHANNEL VpeChannel)
{
    MI_VPE_DestroyChannel(VpeChannel);
    return MI_SUCCESS;
}

static MI_S32 Vpe_StartPort(MI_VPE_PORT VpePort, VPE_PortInfo_t *pstPortInfo)
{
    MI_VPE_PortMode_t stVpeMode;
    MI_SYS_WindowRect_t stCrop;

    printf("############........Vpe_StartPort......%d  %d....\n", pstPortInfo->DepVpeChannel, VpePort);

    memset(&stVpeMode, 0, sizeof(stVpeMode));
    memset(&stCrop, 0, sizeof(MI_SYS_WindowRect_t));
    MI_VPE_SetPortCrop(pstPortInfo->DepVpeChannel, VpePort, &stCrop);
    MI_VPE_GetPortMode(pstPortInfo->DepVpeChannel, VpePort, &stVpeMode);
    stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stVpeMode.ePixelFormat = pstPortInfo->ePixelFormat;
    stVpeMode.u16Width = pstPortInfo->u16OutputWidth;
    stVpeMode.u16Height= pstPortInfo->u16OutputHeight;
    MI_VPE_SetPortMode(pstPortInfo->DepVpeChannel, VpePort, &stVpeMode);
    MI_VPE_EnablePort(pstPortInfo->DepVpeChannel, VpePort);

    return MI_SUCCESS;
}

static MI_S32 Vpe_StopPort(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort)
{
    MI_VPE_DisablePort(VpeChannel, VpePort);
    return MI_SUCCESS;
}

Vpe::Vpe()
{
}
Vpe::~Vpe()
{
}
void Vpe::LoadDb()
{
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapVpeOut;
    stVpeOutInfo_t stVpeOut;

    stVpeInfo.intHdrType = GetIniInt(stModDesc.modKeyString, "HDR_TYPE");
    stVpeInfo.intbUseSnrFmt = GetIniInt(stModDesc.modKeyString, "IS_USE_SNR_FMT");
    stVpeInfo.intRotation= GetIniInt(stModDesc.modKeyString, "ROT");
    stVpeInfo.int3dNrLevel= GetIniInt(stModDesc.modKeyString, "3DNR_LV");
    stVpeInfo.intRunningMode = GetIniInt(stModDesc.modKeyString, "R_MOD");
    stVpeInfo.intChnPortMode = GetIniInt(stModDesc.modKeyString, "CHN_PORT_MOD");
    stVpeInfo.intSensorId = GetIniInt(stModDesc.modKeyString, "SNR_ID");
    stVpeInfo.intVidWidth= GetIniInt(stModDesc.modKeyString, "VID_W");
    stVpeInfo.intVidHeight = GetIniInt(stModDesc.modKeyString, "VID_H");
    if (stVpeInfo.intbUseSnrFmt == 0)
    {
        stVpeInfo.intInputFmt = GetIniInt(stModDesc.modKeyString, "VID_FMT");
    }
    for (itMapVpeOut = mapModOutputInfo.begin(); itMapVpeOut != mapModOutputInfo.end(); itMapVpeOut++)
    {
        memset(&stVpeOut, 0, sizeof(stVpeOutInfo_t));
        stVpeOut.intVpeOutFmt = GetIniInt(itMapVpeOut->second.curIoKeyString, "VID_FMT");
        //printf("%s : VID_FMT %d\n", itVpeOut->second.curIoKeyString.c_str(), stVpeOut.intVpeOutFmt);
        stVpeOut.intVpeOutWidth= GetIniInt(itMapVpeOut->second.curIoKeyString, "VID_W");
        //printf("%s : VID_W %d\n", itVpeOut->second.curIoKeyString.c_str(), stVpeOut.intVpeOutWidth);
        stVpeOut.intVpeOutHeight= GetIniInt(itMapVpeOut->second.curIoKeyString, "VID_H");
        //printf("%s : VID_H %d\n", itVpeOut->second.curIoKeyString.c_str(), stVpeOut.intVpeOutHeight);
        stVpeOut.intPortId = itMapVpeOut->second.curPortId;
        vVpeOutInfo.push_back(stVpeOut);
    }
}
void Vpe::Init()
{
    VPE_ChannelInfo_t stVpeChannelInfo;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_SNR_PADInfo_t  stPad0Info;
    MI_BOOL bMirror = FALSE, bFlip = FALSE;
    std::vector<stVpeOutInfo_t>::iterator itVpeOut;
    VPE_PortInfo_t stVpePortInfo;
    MI_U16 u16Width;
    MI_U16 u16Height;
    MI_SYS_WindowRect_t stCrop;

    stVpeInfo.intHdrType = GetIniInt(stModDesc.modKeyString, "HDR_TYPE");
    stVpeInfo.intbUseSnrFmt = GetIniInt(stModDesc.modKeyString, "IS_USE_SNR_FMT");
    stVpeInfo.intRotation= GetIniInt(stModDesc.modKeyString, "ROT");
    stVpeInfo.int3dNrLevel= GetIniInt(stModDesc.modKeyString, "3DNR_LV");
    stVpeInfo.intRunningMode = GetIniInt(stModDesc.modKeyString, "R_MOD");
    stVpeInfo.intChnPortMode = GetIniInt(stModDesc.modKeyString, "CHN_PORT_MOD");
    stVpeInfo.intSensorId = GetIniInt(stModDesc.modKeyString, "SNR_ID");
    stVpeInfo.intVidWidth= GetIniInt(stModDesc.modKeyString, "VID_W");
    stVpeInfo.intVidHeight = GetIniInt(stModDesc.modKeyString, "VID_H");

    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));

    if (stVpeInfo.intbUseSnrFmt)
    {
        MI_SNR_GetPadInfo((MI_SNR_PAD_ID_e)stVpeInfo.intSensorId, &stPad0Info);
        MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)stVpeInfo.intSensorId, 0, &stSnrPlane0Info);
        switch (stPad0Info.eIntfMode)
        {
            case E_MI_VIF_MODE_MIPI:
            {
                if(stSnrPlane0Info.eBayerId == E_MI_SYS_PIXEL_BAYERID_MAX)
                    stVpeInfo.intInputFmt = (int)stSnrPlane0Info.ePixel;
                else
                    stVpeInfo.intInputFmt = (int)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
            }
            break;
            default:
                assert(0);
        }
        u16Width = stSnrPlane0Info.stCapRect.u16Width;
        u16Height= stSnrPlane0Info.stCapRect.u16Height;
        printf("lane num %d\n", stPad0Info.unIntfAttr.stMipiAttr.u32LaneNum);
        printf("w  %d\n", u16Width);
        printf("w  %d\n", u16Height);
        printf("fmt  %d\n", stVpeInfo.intInputFmt);

    }
    else
    {
        stVpeInfo.intInputFmt = GetIniInt(stModDesc.modKeyString, "VID_FMT");
        u16Width = stVpeInfo.intVidWidth;
        u16Height = stVpeInfo.intVidHeight;
    }
    switch((MI_SYS_Rotate_e)stVpeInfo.intRotation)
    {
        case E_MI_SYS_ROTATE_NONE:
            bMirror= FALSE;
            bFlip = FALSE;
            break;
        case E_MI_SYS_ROTATE_90:
            bMirror = FALSE;
            bFlip = TRUE;
            break;
        case E_MI_SYS_ROTATE_180:
            bMirror = TRUE;
            bFlip = TRUE;
            break;
        case E_MI_SYS_ROTATE_270:
            bMirror = TRUE;
            bFlip = FALSE;
            break;
        default:
            bMirror= FALSE;
            bFlip = FALSE;
            break;
    }
    MI_SNR_SetOrien((MI_SNR_PAD_ID_e)stVpeInfo.intSensorId, bMirror, bFlip);
    memset(&stVpeChannelInfo, 0, sizeof(VPE_ChannelInfo_t));
    stVpeChannelInfo.u16VpeMaxW = u16Width;
    stVpeChannelInfo.u16VpeMaxH = u16Height;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 0;
    stVpeChannelInfo.u16VpeCropH = 0;
    stVpeChannelInfo.eRunningMode = (MI_VPE_RunningMode_e)stVpeInfo.intRunningMode;
    if(stVpeChannelInfo.eRunningMode == E_MI_VPE_RUN_DVR_MODE)
    {
        stVpeChannelInfo.eBindSensorId = E_MI_VPE_SENSOR_INVALID;
    }
    else
    {
        stVpeChannelInfo.eBindSensorId = (MI_VPE_SensorChannel_e)(stVpeInfo.intSensorId + 1);
    }
    stVpeChannelInfo.eFormat = (MI_SYS_PixelFormat_e)stVpeInfo.intInputFmt;
    stVpeChannelInfo.e3DNRLevel = (MI_VPE_3DNR_Level_e)stVpeInfo.int3dNrLevel;
    stVpeChannelInfo.eHDRtype = (MI_VPE_HDRType_e)stVpeInfo.intHdrType;
    stVpeChannelInfo.bRotation = FALSE;
    stVpeChannelInfo.u32ChnPortMode = stVpeInfo.intChnPortMode;
    Vpe_CreateChannel((MI_VPE_CHANNEL)stModDesc.chnId, &stVpeChannelInfo);
    MI_VPE_SetChannelRotation((MI_VPE_CHANNEL)stModDesc.chnId, (MI_SYS_Rotate_e)stVpeInfo.intRotation);
    memset(&stCrop, 0, sizeof(MI_SYS_WindowRect_t));
    MI_VPE_SetChannelCrop((MI_VPE_CHANNEL)stModDesc.chnId, &stCrop);
    MI_VPE_StartChannel ((MI_VPE_CHANNEL)stModDesc.chnId);
    for(itVpeOut = vVpeOutInfo.begin(); itVpeOut != vVpeOutInfo.end(); itVpeOut++)
    {
        memset(&stVpePortInfo, 0, sizeof(VPE_PortInfo_t));
        stVpePortInfo.DepVpeChannel = (MI_VPE_CHANNEL)stModDesc.chnId;
        stVpePortInfo.ePixelFormat = (MI_SYS_PixelFormat_e)itVpeOut->intVpeOutFmt;
        stVpePortInfo.u16OutputWidth = (MI_U16)itVpeOut->intVpeOutWidth;
        stVpePortInfo.u16OutputHeight = (MI_U16)itVpeOut->intVpeOutHeight;
        stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        Vpe_StartPort(itVpeOut->intPortId, &stVpePortInfo);
    }
}

void Vpe::Deinit()
{
    std::vector<stVpeOutInfo_t>::iterator itVpeOutInfo;
    for(itVpeOutInfo = vVpeOutInfo.begin(); itVpeOutInfo != vVpeOutInfo.end(); itVpeOutInfo++)
    {
        Vpe_StopPort((MI_VPE_CHANNEL)stModDesc.chnId, itVpeOutInfo->intPortId);
    }
    MI_VPE_StopChannel((MI_VPE_CHANNEL)stModDesc.chnId);
    Vpe_DestroyChannel((MI_VPE_CHANNEL)stModDesc.chnId);
}
