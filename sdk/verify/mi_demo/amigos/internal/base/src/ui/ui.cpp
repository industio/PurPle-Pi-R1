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

#include "ui.h"

#include "mi_common.h"
#include "mi_rgn.h"

#include "st_rgn.h"

#include <stdio.h>

#define SWITCH_MI_MOD_TO_RGN(mi, rgn) do{  \
    switch (mi) \
    {   \
        case E_MI_MODULE_ID_VPE:    \
        {   \
            rgn = E_MI_RGN_MODID_VPE;   \
        }   \
        break;  \
        case E_MI_MODULE_ID_DIVP:   \
        {   \
            rgn = E_MI_RGN_MODID_DIVP;  \
        }   \
        break;  \
        case E_MI_MODULE_ID_LDC:    \
        {   \
            rgn = E_MI_RGN_MODID_LDC;   \
        }   \
        break;  \
        default:    \
            assert(0);  \
    }   \
}while(0)
#define I4_RED      (1)
#define I4_GREEN    (2)
#define I4_BLUE     (3)
#define I4_BLACK    (12)

Ui::Ui()
{
}
Ui::~Ui()
{
}
void Ui::Init()
{
    ST_OSD_Init();

}
void Ui::Deinit()
{
    ST_OSD_Deinit();
}
void Ui::Start()
{
    unsigned int uintOsdCnt = 0;
    unsigned int uintCoverCnt = 0;
    unsigned int uintAttachCnt = 0;
    unsigned int uintUiFunction = 0;
    char *pSet = NULL;
    unsigned int i = 0, j = 0;
    char strBuf[30];
    std::string strTempString;
    std::string strTempPort;
    int tmpPos = 0;
    stUiOsdInfo_t stUiOsdInfo;
    stUiOsdPathInfo_t stUiOsdPath;
    stUiCoverInfo_t stUiCoverInfo;
    stUiCoverPathInfo_t stCoverOsdPath;
    MI_RGN_Attr_t stRgnAttr;
    MI_RGN_ChnPort_t stChnPort;
    MI_RGN_ChnPortParam_t stChnAttr;
    std::map<unsigned int, stModInputInfo_t>::iterator itUiIn;

    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    memset(&stChnAttr, 0, sizeof(MI_RGN_ChnPortParam_t));
    uintOsdCnt = GetIniUnsignedInt(stModDesc.modKeyString, "OSD_CNT");
    for (i = 0; i < uintOsdCnt; i++)
    {
        memset(strBuf, 0, 30);
        sprintf(strBuf, "OSD_%d", i);
        pSet = GetIniString(stModDesc.modKeyString, strBuf);
        stUiOsdInfo.uintHandle = GetIniUnsignedInt(pSet, "HANDLE");
        stUiOsdInfo.uintFmt = GetIniUnsignedInt(pSet, "FMT");
        stUiOsdInfo.uintWid = GetIniUnsignedInt(pSet, "WID");
        stUiOsdInfo.uintHei = GetIniUnsignedInt(pSet, "HEI");
        uintAttachCnt = GetIniUnsignedInt(pSet, "ATTACH_CNT");
        uintUiFunction = GetIniUnsignedInt(pSet, "FUNCTION");
        stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
        stRgnAttr.stOsdInitParam.ePixelFmt = (MI_RGN_PixelFormat_e)stUiOsdInfo.uintFmt;
        stRgnAttr.stOsdInitParam.stSize.u32Width = (MI_U32)stUiOsdInfo.uintWid;
        stRgnAttr.stOsdInitParam.stSize.u32Height = (MI_U32)stUiOsdInfo.uintHei;
        ST_OSD_Create((MI_RGN_HANDLE)stUiOsdInfo.uintHandle, &stRgnAttr);
        for (j = 0; j < uintAttachCnt; j++)
        {
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d", j);
            strTempString = GetIniString(pSet, strBuf);
            strTempPort = strTempString;
            tmpPos = strTempString.find_first_of(':');
            strTempPort.erase(0, tmpPos + 1);
            stUiOsdPath.pathModDesc.modKeyString = strTempString.erase(tmpPos, strTempString.size() - tmpPos);
            GetInstance(stUiOsdPath.pathModDesc.modKeyString)->GetModDesc(stUiOsdPath.pathModDesc);
            strTempString.erase();
            strTempString = GetIniString(stUiOsdPath.pathModDesc.modKeyString, strTempPort.c_str());
            stUiOsdPath.uintPathOutPort = GetIniUnsignedInt(strTempString.c_str(), "PORT");
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_X", j);
            stUiOsdPath.uintPosX = GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_Y", j);
            stUiOsdPath.uintPosY = GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_LAYER", j);
            stUiOsdPath.uintLayer =  GetIniUnsignedInt(pSet, strBuf);
            stUiOsdInfo.vectPath.push_back(stUiOsdPath);
            SWITCH_MI_MOD_TO_RGN(stUiOsdPath.pathModDesc.modId, stChnPort.eModId);
            stChnPort.s32ChnId = (MI_S32)stUiOsdPath.pathModDesc.chnId;
            stChnPort.s32OutputPortId = (MI_S32)stUiOsdPath.uintPathOutPort;
            stChnPort.s32DevId = (MI_S32)stUiOsdPath.pathModDesc.devId;
            stChnAttr.bShow = TRUE;
            stChnAttr.stPoint.u32X = stUiOsdPath.uintPosX;
            stChnAttr.stPoint.u32Y = stUiOsdPath.uintPosY;
            stChnAttr.unPara.stOsdChnPort.u32Layer = stUiOsdPath.uintLayer;
            stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
            stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
            stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
            MI_RGN_AttachToChn((MI_RGN_HANDLE)stUiOsdInfo.uintHandle, &stChnPort, &stChnAttr);
        }
        mapUiInfo[(EN_UI_FUNCTION)uintUiFunction].push_back(stUiOsdInfo);
    }
    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    memset(&stChnAttr, 0, sizeof(MI_RGN_ChnPortParam_t));
    uintCoverCnt = GetIniUnsignedInt(stModDesc.modKeyString, "COVER_CNT");
    for (i = 0; i < uintCoverCnt; i++)
    {
        memset(strBuf, 0, 30);
        sprintf(strBuf, "COVER_%d", i);
        pSet = GetIniString(stModDesc.modKeyString, strBuf);
        stUiCoverInfo.uintHandle = GetIniUnsignedInt(pSet, "HANDLE");
        uintAttachCnt = GetIniUnsignedInt(pSet, "ATTACH_CNT");
        stRgnAttr.eType = E_MI_RGN_TYPE_COVER;
        MI_RGN_Create(stUiCoverInfo.uintHandle, &stRgnAttr);
        for (j = 0; j < uintAttachCnt; j++)
        {
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d", j);
            strTempString = GetIniString(pSet, strBuf);
            strTempPort = strTempString;
            tmpPos = strTempString.find_first_of(':');
            strTempPort.erase(0, tmpPos + 1);
            stCoverOsdPath.pathModDesc.modKeyString = strTempString.erase(tmpPos, strTempString.size() - tmpPos);
            GetInstance(stCoverOsdPath.pathModDesc.modKeyString)->GetModDesc(stCoverOsdPath.pathModDesc);
            strTempString.erase();
            strTempString = GetIniString(stCoverOsdPath.pathModDesc.modKeyString, strTempPort.c_str());
            stCoverOsdPath.uintPathOutPort = GetIniUnsignedInt(strTempString.c_str(), "PORT");
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_X", j);
            stCoverOsdPath.uintPosX = GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_Y", j);
            stCoverOsdPath.uintPosY = GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_LAYER", j);
            stCoverOsdPath.uintLayer =  GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_W", j);
            stCoverOsdPath.uintWid =  GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_H", j);
            stCoverOsdPath.uintHei =  GetIniUnsignedInt(pSet, strBuf);
            memset(strBuf, 0, 30);
            sprintf(strBuf, "ATTACH_PATH%d_POS_COLOR", j);
            stCoverOsdPath.uintColor =  GetIniUnsignedInt(pSet, strBuf);
            stUiCoverInfo.vectPath.push_back(stCoverOsdPath);
            SWITCH_MI_MOD_TO_RGN(stCoverOsdPath.pathModDesc.modId, stChnPort.eModId);
            stChnPort.s32ChnId = (MI_S32)stCoverOsdPath.pathModDesc.chnId;
            stChnPort.s32OutputPortId = (MI_S32)stCoverOsdPath.uintPathOutPort;
            stChnPort.s32DevId = (MI_S32)stCoverOsdPath.pathModDesc.devId;
            stChnAttr.bShow = TRUE;
            stChnAttr.stPoint.u32X = (MI_U32)stCoverOsdPath.uintPosX;
            stChnAttr.stPoint.u32Y = (MI_U32)stCoverOsdPath.uintPosY;
            stChnAttr.unPara.stCoverChnPort.u32Layer = (MI_U32)stCoverOsdPath.uintLayer;
            stChnAttr.unPara.stCoverChnPort.stSize.u32Width = (MI_U32)stCoverOsdPath.uintWid;
            stChnAttr.unPara.stCoverChnPort.stSize.u32Height = (MI_U32)stCoverOsdPath.uintHei;
            stChnAttr.unPara.stCoverChnPort.u32Color = (MI_U32)stCoverOsdPath.uintColor;
            MI_RGN_AttachToChn((MI_RGN_HANDLE)stUiCoverInfo.uintHandle, &stChnPort, &stChnAttr);
        }
        vectCoverInfo.push_back(stUiCoverInfo);
    }
    for (itUiIn = mapModInputInfo.begin(); itUiIn != mapModInputInfo.end(); itUiIn++)
    {
        CreateReceiver(itUiIn->second.curPortId, DataReceiver, NULL, NULL, this);
        StartReceiver(itUiIn->second.curPortId);
    }

}
void Ui::Stop()
{
    std::map<EN_UI_FUNCTION, std::vector<stUiOsdInfo_t>>::iterator itMapUiInfo;
    std::vector<stUiOsdInfo_t>::iterator itVectUiInfo;
    std::vector<stUiCoverInfo_t>::iterator itVectCoverInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itUiIn;

    for (itUiIn = mapModInputInfo.begin(); itUiIn != mapModInputInfo.end(); itUiIn++)
    {
        StopReceiver(itUiIn->second.curPortId);
        DestroyReceiver(itUiIn->second.curPortId);
    }
    for (itMapUiInfo = mapUiInfo.begin(); itMapUiInfo != mapUiInfo.end(); ++itMapUiInfo)
    {
        for (itVectUiInfo = itMapUiInfo->second.begin(); itVectUiInfo != itMapUiInfo->second.end(); ++itVectUiInfo)
        {
            ST_OSD_Destroy((MI_RGN_HANDLE)itVectUiInfo->uintHandle);
            itVectUiInfo->vectPath.clear();
        }
    }
    for (itVectCoverInfo = vectCoverInfo.begin(); itVectCoverInfo != vectCoverInfo.end(); ++itVectCoverInfo)
    {
        MI_RGN_Destroy((MI_RGN_HANDLE)itVectCoverInfo->uintHandle);
        itVectCoverInfo->vectPath.clear();
    }
    mapUiInfo.clear();
    vectCoverInfo.clear();
}
void Ui::BindBlock(stModInputInfo_t &stIn)
{
}
void Ui::UnBindBlock(stModInputInfo_t &stIn)
{
}
#define ASCII_WID 32
#define ASCII_HEI 32
void Ui::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    std::vector<stUiOsdInfo_t> *pDlaUiOsdInfo;
    std::vector<stUiOsdInfo_t>::iterator it;
    stFaceInfo_t *pRes;
    unsigned int intResCnt;
    MI_S32 s32Ret = 0;
    MI_RGN_CanvasInfo_t *pCanvasInfo;
    std::vector<stFaceInfo_t>::iterator itVectRes;
    ST_Rect_T stRectArea;
    ST_Point_T stTextPoint;
    unsigned int nameLen = 0;
    MI_U32 u32Color = 0;
    Ui *pInstance = NULL;

    //printf("UI get data size %d\n", dataSize);
    assert(dataSize % sizeof(stFaceInfo_t) == 0);
    pInstance = (Ui *)pUsrData;
    pDlaUiOsdInfo = (std::vector<stUiOsdInfo_t> *)&(pInstance->mapUiInfo[EN_UI_FUNC_SHOW_DLA_RES]);
    pRes = (stFaceInfo_t *)pData;
    intResCnt = dataSize / sizeof(stFaceInfo_t);
    //printf("Rescnt %d\n", intResCnt);

    for (it = pDlaUiOsdInfo->begin(); it != pDlaUiOsdInfo->end(); ++it)
    {
        if (it->uintFmt != E_MI_RGN_PIXEL_FORMAT_I4)
        {
            printf("Only support I4!\n");
            continue;
        }
        s32Ret = ST_OSD_GetCanvasInfo((MI_RGN_HANDLE)it->uintHandle, &pCanvasInfo);
        if (s32Ret == MI_RGN_OK)
        {
            //printf("Handler %d\n", it->uintHandle);
            //printf("W %d\n", it->uintWid);
            //printf("H %d\n", it->uintHei);
            for (itVectRes = pInstance->vectResultBk.begin(); itVectRes != pInstance->vectResultBk.end(); itVectRes++)
            {
                nameLen = strlen(itVectRes->faceName);
                stRectArea.u16PicW = itVectRes->faceW * it->uintWid / itVectRes->winWid;
                stRectArea.u16PicH = itVectRes->faceH * it->uintHei / itVectRes->winHei;
                stRectArea.u32X = itVectRes->xPos * it->uintWid / itVectRes->winWid;
                stRectArea.u32Y = itVectRes->yPos * it->uintHei / itVectRes->winHei;
                if (stRectArea.u32X + stRectArea.u16PicW < it->uintWid
                    && stRectArea.u32Y + stRectArea.u16PicH < it->uintHei)
                {
                    ST_OSD_ClearRectFast((MI_RGN_HANDLE)it->uintHandle, stRectArea);
                    if(nameLen && (stRectArea.u32Y > ASCII_HEI)
                        && (it->uintWid > nameLen * ASCII_WID + stRectArea.u32X))
                    {
                        stRectArea.u32Y -= ASCII_HEI;
                        stRectArea.u16PicW = nameLen * ASCII_WID;
                        stRectArea.u16PicH = ASCII_HEI;
                        ST_OSD_ClearRectFast((MI_RGN_HANDLE)it->uintHandle, stRectArea);
                    }
                }
            } 
            for (unsigned int i = 0; i < intResCnt; i++)
            {
                nameLen = strlen(pRes[i].faceName);
                stRectArea.u16PicW = (pRes[i].faceW * it->uintWid) / pRes[i].winWid;
                stRectArea.u16PicH = (pRes[i].faceH * it->uintHei) / pRes[i].winHei;
                stRectArea.u32X = (pRes[i].xPos * it->uintWid) / pRes[i].winWid;
                stRectArea.u32Y = (pRes[i].yPos * it->uintHei) / pRes[i].winHei;
                u32Color = (!strstr(pRes[i].faceName, "unknown") && nameLen)?(I4_GREEN):(I4_RED);
                if (stRectArea.u32X + stRectArea.u16PicW < it->uintWid
                    && stRectArea.u32Y + stRectArea.u16PicH < it->uintHei)
                {
                    //printf("cnt %d rect w %d h %d x %d y %d\n", i, stRectArea.u16PicW, stRectArea.u16PicH, stRectArea.u32X, stRectArea.u32Y);
                    //printf("Name %s\n", pRes[i].faceName);
                    //printf("Face W %d\n", pRes[i].faceW);
                    //printf("Face H %d\n", pRes[i].faceH);
                    //printf("Face X %d\n", pRes[i].xPos);
                    //printf("Face Y %d\n", pRes[i].yPos);
                    //printf("window W %d\n", pRes[i].winWid);
                    //printf("window H %d\n", pRes[i].winHei);
                    //printf("dst win W %d\n", it->uintWid);
                    //printf("dst win H %d\n", it->uintHei);
                    ST_OSD_DrawRectFast((MI_RGN_HANDLE)it->uintHandle, stRectArea, 4, u32Color);
                    if(nameLen && (stRectArea.u32Y > ASCII_HEI)
                        && (it->uintWid > nameLen * ASCII_WID + stRectArea.u32X))
                    {
                        stTextPoint.u32X = stRectArea.u32X;
                        stTextPoint.u32Y = stRectArea.u32Y - ASCII_HEI;
                        //printf("Draw %s\n", pRes[i].faceName);
                        ST_OSD_DrawText((MI_RGN_HANDLE)it->uintHandle, stTextPoint, pRes[i].faceName, u32Color, DMF_Font_Size_32x32);
                    }
                }
            }
            ST_OSD_Update((MI_RGN_HANDLE)it->uintHandle);
        }
    }
    pInstance->vectResultBk.clear();
    for (unsigned int i = 0; i < intResCnt; i++)
    {
        pInstance->vectResultBk.push_back(pRes[i]);
    }

}

