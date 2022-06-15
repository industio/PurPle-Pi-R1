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

#ifndef __DIVP_H__
#define __DIVP_H__

#include "sys.h"

typedef struct stDivpInfo_s
{
    int intCropWidth;
    int intCropHeight;
    int intCropX;
    int intCropY;
}stDivpInfo_t;
typedef struct stDivpOutInfo_s
{
    int intPortId;
    int intDivpOutFmt;
    int intDivputWidth;
    int intDivpOutHeight;
}stDivpOutInfo_t;

class Divp: public Sys
{
    public:
        Divp();
        virtual ~Divp();
        void GetInfo(stDivpInfo_t &info, std::vector<stDivpOutInfo_t> &out)
        {
            info = stDivpInfo;
            out = vDivpOutInfo;
        }
        void UpdateInfo(stDivpInfo_t &info, std::vector<stDivpOutInfo_t> &out)
        {
            stDivpInfo = info;
            vDivpOutInfo = out;
        };
    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();

        stDivpInfo_t stDivpInfo;
        std::vector<stDivpOutInfo_t> vDivpOutInfo;
};
#endif

