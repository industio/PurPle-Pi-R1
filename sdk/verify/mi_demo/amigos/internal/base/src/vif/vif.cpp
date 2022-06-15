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
#include <assert.h>

#include <vector>
#include <string>

#include "mi_sensor.h"
#include "mi_vif.h"
#include "mi_sys.h"
#include "vif.h"

Vif::Vif()
{
}
Vif::~Vif()
{
}
void Vif::LoadDb()
{
    stVifInfo.intHdrType = GetIniInt(stModDesc.modKeyString,"HDR_TYPE");
    stVifInfo.intSensorId = GetIniInt(stModDesc.modKeyString,"SNR_ID");
    stVifInfo.intSensorRes = GetIniInt(stModDesc.modKeyString,"SNR_RES");
    stVifInfo.intWorkMode = GetIniInt(stModDesc.modKeyString, "WORK_MOD");
    stVifInfo.intEnableSnr = GetIniInt(stModDesc.modKeyString, "EN_SNR");
}
void Vif::Init()
{
    MI_SNR_PADInfo_t  stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_SNR_Res_t stRes;
    MI_U8 u8ResIndex =0;
    MI_U32 u32ResCount =0;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_PixelFormat_e ePixFormat;
    MI_VIF_DevAttr_t stDevAttr;
    MI_VIF_ChnPortAttr_t stChnPortAttr;
    MI_SYS_ChnPort_t stChnPort;

    memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    memset(&stRes, 0x0, sizeof(MI_SNR_Res_t));
    memset(&stDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
    memset(&stChnPortAttr, 0, sizeof(MI_VIF_ChnPortAttr_t));

    if (stVifInfo.intEnableSnr)
    {
        if(stVifInfo.intHdrType > 0)
            MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, TRUE);
        else
            MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, FALSE);
        
        MI_SNR_QueryResCount((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, &u32ResCount);
        for(u8ResIndex=0; u8ResIndex < u32ResCount; u8ResIndex++)
        {
            MI_SNR_GetRes((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, u8ResIndex, &stRes);
            printf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
            u8ResIndex,
            stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
            stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
            stRes.u32MaxFps,stRes.u32MinFps,
            stRes.strResDesc);
        }
        if(stVifInfo.intSensorRes >= (int)u32ResCount)
        {
            printf("choice err res %d > =cnt %d\n", stVifInfo.intSensorRes, u32ResCount);
            assert(0);
        }
        MI_SNR_SetRes((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, (MI_U32)stVifInfo.intSensorRes);
        MI_SNR_Enable((MI_SNR_PAD_ID_e)stVifInfo.intSensorId);
    }
    MI_SNR_GetPadInfo((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, &stPad0Info);
    MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)stVifInfo.intSensorId, 0, &stSnrPlane0Info);
    u32CapWidth = stSnrPlane0Info.stCapRect.u16Width;
    u32CapHeight = stSnrPlane0Info.stCapRect.u16Height;
    eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    switch (stPad0Info.eIntfMode)
    {
        case E_MI_VIF_MODE_MIPI:
        {
            if(stSnrPlane0Info.eBayerId == E_MI_SYS_PIXEL_BAYERID_MAX)
                ePixFormat = (MI_SYS_PixelFormat_e)stSnrPlane0Info.ePixel;
            else
                ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
        }
        break;
        default:
            assert(0);
    }
    stDevAttr.eIntfMode = stPad0Info.eIntfMode;
    stDevAttr.eWorkMode = (MI_VIF_WorkMode_e)stVifInfo.intWorkMode;
    stDevAttr.eHDRType = (MI_VIF_HDRType_e)stVifInfo.intHdrType;
    MI_VIF_SetDevAttr((MI_VIF_DEV)stModDesc.devId, &stDevAttr);
    MI_VIF_EnableDev((MI_VIF_DEV)stModDesc.devId);
    stChnPortAttr.stCapRect.u16X = 0;
    stChnPortAttr.stCapRect.u16Y = 0;
    stChnPortAttr.stCapRect.u16Width = u32CapWidth;
    stChnPortAttr.stCapRect.u16Height = u32CapHeight;
    stChnPortAttr.stDestSize.u16Width = u32CapWidth;
    stChnPortAttr.stDestSize.u16Height = u32CapHeight;
    stChnPortAttr.ePixFormat = ePixFormat;//E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stChnPortAttr.eFrameRate = eFrameRate;
    MI_VIF_SetChnPortAttr((MI_VIF_CHN)stModDesc.chnId, 0, &stChnPortAttr);
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = (MI_VIF_DEV)stModDesc.devId;
    stChnPort.u32ChnId = (MI_VIF_CHN)stModDesc.chnId;
    stChnPort.u32PortId = 0;
    MI_VIF_EnableChnPort((MI_VIF_CHN)stModDesc.chnId, 0);
}
void Vif::Deinit()
{
    MI_VIF_DisableChnPort((MI_VIF_CHN)stModDesc.chnId, 0);
    MI_VIF_DisableDev((MI_VIF_DEV)stModDesc.devId);
}

