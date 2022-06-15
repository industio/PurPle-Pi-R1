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

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_divp.h"

#include "divp.h"

Divp::Divp()
{
}
Divp::~Divp()
{
}
void Divp::LoadDb()
{
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapDivpOut;
    stDivpOutInfo_t stDivpOutInfo;

    stDivpInfo.intCropWidth = GetIniInt(stModDesc.modKeyString, "CROP_W");
    stDivpInfo.intCropHeight = GetIniInt(stModDesc.modKeyString, "CROP_H");
    stDivpInfo.intCropX = GetIniInt(stModDesc.modKeyString, "CROP_X");
    stDivpInfo.intCropY = GetIniInt(stModDesc.modKeyString, "CROP_Y");
    for (itMapDivpOut = mapModOutputInfo.begin(); itMapDivpOut != mapModOutputInfo.end(); itMapDivpOut++)
    {
        memset(&stDivpOutInfo, 0, sizeof(stDivpOutInfo_t));
        stDivpOutInfo.intDivpOutFmt = GetIniInt(itMapDivpOut->second.curIoKeyString, "VID_FMT");
        //printf("%s : VID_FMT %d\n", itDivpOut->second.curIoKeyString.c_str(), stDivpOutInfo.intDivpOutFmt);
        stDivpOutInfo.intDivputWidth = GetIniInt(itMapDivpOut->second.curIoKeyString, "VID_W");
        //printf("%s : VID_W %d\n", itDivpOut->second.curIoKeyString.c_str(), stDivpOutInfo.intDivputWidth);
        stDivpOutInfo.intDivpOutHeight = GetIniInt(itMapDivpOut->second.curIoKeyString, "VID_H");
        //printf("%s : VID_H %d\n", itDivpOut->second.curIoKeyString.c_str(), stDivpOutInfo.intDivpOutHeight);
        stDivpOutInfo.intPortId = itMapDivpOut->second.curPortId;
        vDivpOutInfo.push_back(stDivpOutInfo);
    }
}
void Divp::Init()
{
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stOutAttr;
    std::vector<stDivpOutInfo_t>::iterator itDivpOut;

    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = (MI_U16)stDivpInfo.intCropX;
    stDivpChnAttr.stCropRect.u16Y       = (MI_U16)stDivpInfo.intCropY;
    stDivpChnAttr.stCropRect.u16Width   = (MI_U16)stDivpInfo.intCropWidth;
    stDivpChnAttr.stCropRect.u16Height  = (MI_U16)stDivpInfo.intCropHeight;
    stDivpChnAttr.u32MaxWidth           = (MI_U16)stDivpInfo.intCropWidth;
    stDivpChnAttr.u32MaxHeight          = (MI_U16)stDivpInfo.intCropHeight;
    MI_DIVP_CreateChn((MI_DIVP_CHN)stModDesc.chnId, &stDivpChnAttr);

    for (itDivpOut = vDivpOutInfo.begin(); itDivpOut != vDivpOutInfo.end(); itDivpOut++)
    {            
        memset(&stOutAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
        stOutAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stOutAttr.ePixelFormat = (MI_SYS_PixelFormat_e)itDivpOut->intDivpOutFmt;
        stOutAttr.u32Width = (MI_U32)itDivpOut->intDivputWidth;
        stOutAttr.u32Height= (MI_U32)itDivpOut->intDivpOutHeight;
        MI_DIVP_SetOutputPortAttr((MI_DIVP_CHN)stModDesc.chnId, &stOutAttr);
        MI_DIVP_StartChn((MI_DIVP_CHN)stModDesc.chnId);
    }
}
void Divp::Deinit()
{
    vDivpOutInfo.clear();
    MI_DIVP_StopChn((MI_DIVP_CHN)stModDesc.chnId);
    MI_DIVP_DestroyChn((MI_DIVP_CHN)stModDesc.chnId);
}
