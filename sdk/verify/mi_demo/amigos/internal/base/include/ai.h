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

#ifndef __AI_H__
#define __AI_H__

#include "sys.h"

typedef struct stAiInfo_s
{
    unsigned int uintSampleRate;
    unsigned int uintBitWidth;
    unsigned int uintWorkMode;
    unsigned int uintSoundMode;
    unsigned int uintChannelCnt;
    unsigned int uintVolume;
}stAiInfo_t;

class Ai: public Sys
{
    public:
        Ai();
        virtual ~Ai();
        void GetInfo(stAiInfo_t &info)
        {
            info = stAiInfo;
        }
        void UpdateInfo(stAiInfo_t &info)
        {
            stAiInfo = info;
        };

    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        stAiInfo_t stAiInfo;
};
#endif

