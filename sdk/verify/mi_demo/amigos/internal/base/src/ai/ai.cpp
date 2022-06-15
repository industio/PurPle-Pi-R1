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

#include "mi_sys.h"
#include "mi_ai.h"
#include "ai.h"
#include <stdio.h>

Ai::Ai()
{
    
}
Ai::~Ai()
{
}
void Ai::LoadDb()
{
    stAiInfo.uintSampleRate = GetIniInt(stModDesc.modKeyString, "SAMPLE_RATE");
    stAiInfo.uintBitWidth = GetIniInt(stModDesc.modKeyString, "BIT_WIDTH");
    stAiInfo.uintWorkMode = GetIniInt(stModDesc.modKeyString, "WORK_MOD");
    stAiInfo.uintSoundMode = GetIniInt(stModDesc.modKeyString, "SND_MOD");
    stAiInfo.uintChannelCnt = GetIniInt(stModDesc.modKeyString, "CHN_CNT");
    stAiInfo.uintVolume = GetIniInt(stModDesc.modKeyString, "VOLUME");
}
void Ai::Init()
{
    MI_AUDIO_Attr_t stAiPubAttr;
    MI_SYS_ChnPort_t stChnOutputPort;

    //MI_AI_InitDev(NULL);
    memset(&stAiPubAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAiPubAttr.eSamplerate = (MI_AUDIO_SampleRate_e)stAiInfo.uintSampleRate;
    stAiPubAttr.eBitwidth = (MI_AUDIO_BitWidth_e)((stAiInfo.uintBitWidth == 16) ? E_MI_AUDIO_BIT_WIDTH_16: E_MI_AUDIO_BIT_WIDTH_24);
    stAiPubAttr.eWorkmode = (MI_AUDIO_Mode_e)stAiInfo.uintWorkMode;
    stAiPubAttr.eSoundmode = (MI_AUDIO_SoundMode_e)stAiInfo.uintSoundMode;
    stAiPubAttr.u32ChnCnt = stAiInfo.uintChannelCnt;
    stAiPubAttr.u32CodecChnCnt = 0; // useless
    stAiPubAttr.u32FrmNum = 6;  // useless
    stAiPubAttr.u32PtNumPerFrm = stAiPubAttr.eSamplerate / 32; // for aec
    stAiPubAttr.WorkModeSetting.stI2sConfig.bSyncClock = TRUE;
    stAiPubAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAiPubAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    /*printf("Audio sample rate %d, bit width %d, work mode %d, snd mode %d, chn %d\n",
            stAiInfo.uintSampleRate, stAiInfo.uintBitWidth, stAiInfo.uintWorkMode,
            stAiInfo.uintSoundMode, stAiInfo.uintChannelCnt);*/
    MI_AI_SetPubAttr((MI_AUDIO_DEV)stModDesc.devId, &stAiPubAttr);
    MI_AI_Enable((MI_AUDIO_DEV)stModDesc.devId);
    MI_AI_EnableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AI_CHN)stModDesc.chnId);
    MI_AI_SetVqeVolume((MI_AUDIO_DEV)stModDesc.devId, (MI_AI_CHN)stModDesc.chnId, stAiInfo.uintVolume);
    memset(&stChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnOutputPort.u32DevId = stModDesc.devId;
    stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32ChnId = stModDesc.chnId;
    stChnOutputPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stChnOutputPort, 5, 5);
}
void Ai::Deinit()
{
    MI_AI_DisableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AI_CHN)stModDesc.chnId);
    MI_AI_Disable((MI_AUDIO_DEV)stModDesc.devId);
    //MI_AI_DeInitDev();
}

