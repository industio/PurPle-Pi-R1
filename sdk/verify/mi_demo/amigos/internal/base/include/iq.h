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

#ifndef __IQ_H__
#define __IQ_H__

#include "sys.h"

class Iq: public Sys
{
    public:
        Iq();
        virtual ~Iq();
    private:
        virtual void Init();
        virtual void Deinit();
        virtual void Start();
        virtual void Stop();
        int IspWaitReadyTimeout(int time_ms);
        stModDesc_t stVpeDesc;
        int intbUseIq;
        unsigned int iqSrvVidWid;
        unsigned int iqSrvVidHei;
};
#endif

