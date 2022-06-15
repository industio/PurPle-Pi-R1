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

#ifndef __UI_H__
#define __UI_H__

#include "sys.h"
typedef enum
{
    EN_UI_FUNC_DISP_TIME,
    EN_UI_FUNC_SHOW_DLA_RES,
    EN_UI_FUNC_MAX
}EN_UI_FUNCTION;

typedef struct stUiOsdPathInfo_s
{
    stModDesc_t pathModDesc;
    unsigned int uintPathOutPort;
    unsigned int uintPosX;
    unsigned int uintPosY;
    unsigned int uintLayer;
}stUiOsdPathInfo_t;
typedef struct stUiCoverPathInfo_s
{
    stModDesc_t pathModDesc;
    unsigned int uintPathOutPort;
    unsigned int uintPosX;
    unsigned int uintPosY;
    unsigned int uintWid;
    unsigned int uintHei;
    unsigned int uintColor;
    unsigned int uintLayer;
}stUiCoverPathInfo_t;

typedef struct stUiOsdInfo_s
{
    unsigned int uintHandle;
    unsigned int uintFmt;
    unsigned int uintWid;
    unsigned int uintHei;
    std::vector<stUiOsdPathInfo_t> vectPath;
}stUiOsdInfo_t;
typedef struct stUiCoverInfo_s
{
    unsigned int uintHandle;
    std::vector<stUiCoverPathInfo_t> vectPath;
}stUiCoverInfo_t;

class Ui: public Sys
{
    public:
        Ui();
        virtual ~Ui();
    private:
        virtual void Init();
        virtual void Deinit();
        virtual void Start();
        virtual void Stop();
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId);
        std::map<EN_UI_FUNCTION, std::vector<stUiOsdInfo_t>> mapUiInfo;
        std::vector<stUiCoverInfo_t> vectCoverInfo;
        std::vector<stFaceInfo_t> vectResultBk;
};
#endif

