#ifndef _ST_HDMI_H
#define _ST_HDMI_H

#include "mi_hdmi.h"

/* call Init & Start */
MI_S32 ST_Hdmi_Init(void);
MI_S32 ST_Hdmi_DeInit(MI_HDMI_DeviceId_e eHdmi);
MI_S32 ST_Hdmi_Start(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_TimingType_e eTimingType);
MI_S32 ST_Hdmi_SetAttr(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_TimingType_e eTimingType);

MI_S32 ST_Hdmi_GetEdid(MI_HDMI_DeviceId_e eHdmi, MI_U8 *pu8Data, MI_U8 *u8Len);
MI_S32 ST_Hdmi_GetSinkInfo(MI_HDMI_DeviceId_e eHdmi);

#endif //_ST_HDMI_H
