#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_hdmi.h"

#include "st_common.h"
#include "st_hdmi.h"


static MI_S32 _gs32HdmiRunnig = FALSE;

static MI_S32 Hdmi_callback_impl(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            printf("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            printf("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            printf("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Hdmi_Init(void)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;

    STDBG_ENTER();
    stInitParam.pCallBackArgs = NULL;
    stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;

    ExecFunc(MI_HDMI_Init(&stInitParam), MI_SUCCESS);
    ExecFunc(MI_HDMI_Open(eHdmi), MI_SUCCESS);
    STDBG_LEAVE();

	return MI_SUCCESS;
}

MI_S32 ST_Hdmi_DeInit(MI_HDMI_DeviceId_e eHdmi)
{
    STDBG_ENTER();
    ExecFunc(MI_HDMI_Stop(eHdmi), MI_SUCCESS);
    ExecFunc(MI_HDMI_Close(eHdmi), MI_SUCCESS);
    ExecFunc(MI_HDMI_DeInit(), MI_SUCCESS);
    STDBG_LEAVE();

    return MI_SUCCESS;
}

/*
 * Default: HDMI MODE, YUV444, NoID(color depth)
 */
MI_S32 ST_Hdmi_Start(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_TimingType_e eTimingType)
{
    MI_HDMI_Attr_t stAttr;

    STDBG_ENTER();
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
    stAttr.stAudioAttr.bEnableAudio = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel = 0;
    stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
    stAttr.stVideoAttr.bEnableVideo = TRUE;
    stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
    stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
    stAttr.stVideoAttr.eTimingType = eTimingType;
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    ExecFunc(MI_HDMI_SetAttr(eHdmi, &stAttr), MI_SUCCESS);

    ExecFunc(MI_HDMI_Start(eHdmi), MI_SUCCESS);
    STDBG_LEAVE();

    return MI_SUCCESS;
}

MI_S32 ST_Hdmi_SetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_TimingType_e eTimingType)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_HDMI_Attr_t stAttr;

    STDBG_ENTER();
    ExecFunc(MI_HDMI_SetAvMute(eHdmi, TRUE), MI_SUCCESS);
    ExecFunc(MI_HDMI_GetAttr(eHdmi, &stAttr), MI_SUCCESS);

    stAttr.stVideoAttr.eTimingType = eTimingType;

    ExecFunc(MI_HDMI_SetAttr(eHdmi, &stAttr), MI_SUCCESS);
    ExecFunc(MI_HDMI_SetAvMute(eHdmi, FALSE), MI_SUCCESS);
    STDBG_LEAVE();

    return MI_SUCCESS;
}

MI_S32 ST_Hdmi_GetEdid(MI_HDMI_DeviceId_e eHdmi, MI_U8 *pu8Data, MI_U8 *u8Len)
{
    MI_HDMI_Edid_t stEdid;
    MI_S32 i = 0;

    STDBG_ENTER();
    ExecFunc(MI_HDMI_ForceGetEdid(eHdmi, &stEdid), MI_SUCCESS);
    for (i = 0; i < stEdid.u32Edidlength; i++)
    {
        printf("[%x] ", stEdid.au8Edid[i]);
    }
    printf("\n");
    memcpy(pu8Data, &stEdid.au8Edid, stEdid.u32Edidlength);
    *u8Len = stEdid.u32Edidlength;
    STDBG_LEAVE();

    return MI_SUCCESS;
}

MI_S32 ST_Hdmi_GetSinkInfo(MI_HDMI_DeviceId_e eHdmi)
{
    MI_HDMI_SinkInfo_t stSinkInfo;
    MI_S32 s32Ret = MI_SUCCESS;

    STDBG_ENTER();
    ExecFunc(MI_HDMI_GetSinkInfo(eHdmi, &stSinkInfo), MI_SUCCESS);
    STDBG_LEAVE();

    return MI_SUCCESS;
}
