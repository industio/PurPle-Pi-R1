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

#include "iq.h"
#include <stdio.h>
#include "mi_isp.h"
#include "mi_iqserver.h"

Iq::Iq()
{
    intbUseIq = 0;
    iqSrvVidWid = 0;
    iqSrvVidHei = 0;
}
Iq::~Iq()
{
}
void Iq::Init()
{
}
void Iq::Deinit()
{
}
void Iq::Start()
{
    char *iqFilePath = NULL;
    unsigned int uintUsrKey = 0;
    std::string vpeStr;
    
    vpeStr = GetIniString(stModDesc.modKeyString, "VPE_KEY_STR");
    GetInstance(vpeStr)->GetModDesc(stVpeDesc);
    iqFilePath = GetIniString(stModDesc.modKeyString, "IQ_API_FILE");
    sleep(1);
    if (iqFilePath)
    {
        if(IspWaitReadyTimeout(1000) == 0)
        {
            uintUsrKey = GetIniUnsignedInt(stModDesc.modKeyString, "IQ_USR_KEY");
            MI_ISP_API_CmdLoadBinFile((MI_U32)stVpeDesc.chnId, iqFilePath,  uintUsrKey);
        }
    }
    intbUseIq = GetIniInt(stModDesc.modKeyString, "DO_OPEN_IQ_SRV");
    if (intbUseIq)
    {
        iqSrvVidWid = GetIniInt(stModDesc.modKeyString, "IQ_SRV_VID_W");
        iqSrvVidHei = GetIniInt(stModDesc.modKeyString, "IQ_SRV_VID_H");
        MI_IQSERVER_Open(iqSrvVidWid, iqSrvVidHei, (MI_S32)stVpeDesc.chnId);
    }

}
int Iq::IspWaitReadyTimeout(int time_ms)
{
    int delayCnt = time_ms;
    MI_ISP_IQ_PARAM_INIT_INFO_TYPE_t status;

    while(delayCnt)
    {
        MI_ISP_IQ_GetParaInitStatus((MI_U32)stVpeDesc.chnId, &status);
        if(status.stParaAPI.bFlag == 1){
            break;
        }
        delayCnt--;
        usleep(1000);
    }

    if(delayCnt <= 0){
        printf("wait isp ready timeout\n");
        return -1;
    }

    return 0;
}

void Iq::Stop()
{
    if (intbUseIq)
    {
        MI_IQSERVER_Close();
    }
}

