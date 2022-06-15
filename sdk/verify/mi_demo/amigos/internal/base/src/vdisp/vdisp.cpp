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
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include <mi_vdisp_datatype.h>
#include <mi_vdisp.h>

#include "vdisp.h"
#include <stdio.h>

Vdisp::Vdisp()
{
}

Vdisp::~Vdisp()
{
}
void Vdisp::LoadDb()
{
    stVdispInputInfo_t stVdispInputInfo;
    stVdispOutputInfo_t stVdispOutputInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapVdispIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapVdispOut;

    for (itMapVdispIn = mapModInputInfo.begin(); itMapVdispIn != mapModInputInfo.end(); itMapVdispIn++)
    {
        memset(&stVdispInputInfo, 0, sizeof(stVdispInputInfo_t));
        stVdispInputInfo.intPortId = itMapVdispIn->second.curPortId;
        stVdispInputInfo.intChnId = GetIniInt(itMapVdispIn->second.curIoKeyString, "CHN");
        stVdispInputInfo.intFreeRun = GetIniInt(itMapVdispIn->second.curIoKeyString, "FREE_RUN");
        stVdispInputInfo.intVdispInX = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_X");
        stVdispInputInfo.intVdispInY = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_Y");
        
        stVdispInputInfo.intVdispInWidth = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_W");
        stVdispInputInfo.intVdispInHeight = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_H");
        vVdispInputInfo.push_back(stVdispInputInfo);
    }
    for (itMapVdispOut = mapModOutputInfo.begin(); itMapVdispOut != mapModOutputInfo.end(); itMapVdispOut++)
    {
        memset(&stVdispOutputInfo, 0, sizeof(stVdispOutputInfo_t));
        stVdispOutputInfo.intVdispOutFrameRate = itMapVdispOut->second.curFrmRate;
        stVdispOutputInfo.intVdispOutWidth = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_W");
        stVdispOutputInfo.intVdispOutHeight = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_H");
        stVdispOutputInfo.intVdispOutPts = GetIniInt(itMapVdispOut->second.curIoKeyString, "PTS");
        stVdispOutputInfo.intVdispOutFormat = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_FMT");
        stVdispOutputInfo.intVdispOutBkColor = GetIniInt(itMapVdispOut->second.curIoKeyString, "BK_COLOR");
        vVdispOutputInfo.push_back(stVdispOutputInfo);
    }
}
void Vdisp::Init()
{
    MI_VDISP_InputChnAttr_t stInputChnAttr;
    MI_VDISP_OutputPortAttr_t stOutputPortAttr;
    std::vector<stVdispInputInfo_t>::iterator itVdispIn;
    std::vector<stVdispOutputInfo_t>::iterator itVdispOut;

    MI_VDISP_Init();
    MI_VDISP_OpenDevice(stModDesc.devId);

    //set input port attr
    for (itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
        if(itVdispIn->intFreeRun == 1)
        {
            stInputChnAttr.s32IsFreeRun = TRUE;
        }
        else
        {
            stInputChnAttr.s32IsFreeRun = FALSE;
        }

        stInputChnAttr.u32OutX = ALIGN16_DOWN(itVdispIn->intVdispInX);
        stInputChnAttr.u32OutY = itVdispIn->intVdispInY;

        stInputChnAttr.u32OutWidth = itVdispIn->intVdispInWidth;
        stInputChnAttr.u32OutHeight = itVdispIn->intVdispInHeight;

        MI_VDISP_SetInputChannelAttr(stModDesc.devId, itVdispIn->intChnId, &stInputChnAttr);
        MI_VDISP_EnableInputChannel(stModDesc.devId, itVdispIn->intChnId);
    }

    for (itVdispOut = vVdispOutputInfo.begin(); itVdispOut != vVdispOutputInfo.end(); itVdispOut++)
    {
        
        memset(&stOutputPortAttr, 0, sizeof(MI_VDISP_OutputPortAttr_t));
        stOutputPortAttr.u32FrmRate = itVdispOut->intVdispOutFrameRate;
        stOutputPortAttr.u32Height = itVdispOut->intVdispOutHeight;
        stOutputPortAttr.u32Width = itVdispOut->intVdispOutWidth;
        stOutputPortAttr.u64pts = itVdispOut->intVdispOutPts;
        stOutputPortAttr.ePixelFormat = (MI_SYS_PixelFormat_e)itVdispOut->intVdispOutFormat;
        switch(itVdispOut->intVdispOutBkColor)
        {
            case 0:
                stOutputPortAttr.u32BgColor = YUYV_BLACK;
                break;
            case 1:
                stOutputPortAttr.u32BgColor = YUYV_WHITE;
                break;
            case 2:
                stOutputPortAttr.u32BgColor = YUYV_RED;
                break;
            case 3:
                stOutputPortAttr.u32BgColor = YUYV_GREEN;
                break;
            case 4:
                stOutputPortAttr.u32BgColor = YUYV_BLUE;
                break;
            default:
                stOutputPortAttr.u32BgColor = YUYV_BLACK;
                break;
        }

        MI_VDISP_SetOutputPortAttr(stModDesc.devId, 0, &stOutputPortAttr);
    }   
    MI_VDISP_StartDev(stModDesc.devId);
}
void Vdisp::Deinit()
{
    std::vector<stVdispInputInfo_t>::iterator itVdispIn;
    for(itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
        MI_VDISP_DisableInputChannel(stModDesc.devId, (MI_VDISP_CHN)(itVdispIn->intChnId));
    }
    MI_VDISP_StopDev(stModDesc.devId);
    MI_VDISP_CloseDevice(stModDesc.devId);
    MI_VDISP_Exit();
}
