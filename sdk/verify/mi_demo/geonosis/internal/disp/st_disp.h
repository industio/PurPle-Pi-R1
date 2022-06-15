#ifndef _ST_DISP_H
#define _ST_DISP_H

#include "mi_sys.h"
#include "mi_common_datatype.h"
#include "mi_disp_datatype.h"

#include "st_common.h"

#define ST_DISP_DEV0 0
#define ST_DISP_DEV1 1
#define DISP_MAX_CHN 1

#define ST_DISP_PIP_PORT 16

typedef enum
{
    ST_DISP_LAYER0 = 0,
    ST_DISP_LAYER1,         // for PIP
    ST_DISP_MAX
} ST_DispLayerId_e;
typedef struct ST_DispInfo_s
{
    /* Vo Layer attr*/
    MI_DISP_DEV DispDev;
    MI_DISP_Interface_e eIntfType;
    MI_DISP_LAYER DispLayer;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_OutputTiming_e eTiming;

    /* Others */
    MI_U32 u32Toleration;
} ST_DispDevInfo_t;

typedef struct ST_DispChnAttr_s
{
    MI_U32 u32Port;
    MI_DISP_InputPortAttr_t stAttr;
} ST_DispChnAttr_t;

typedef struct ST_DispChnInfo_s
{
    /* Vo Chn attr */
    MI_DISP_INPUTPORT InputPortNum;
    ST_DispChnAttr_t stInputPortAttr[MAX_CHANNEL_NUM];
} ST_DispChnInfo_t;

typedef struct ST_DataInfo_s
{
    MI_U16 u16PicWidth;
    MI_U16 u16PicHeight;
    MI_U16 u16Times;
    MI_U8 au8FileName[128];
} ST_DataInfo_t;

MI_S32 ST_Disp_DevInit(MI_DISP_DEV dispDev, MI_DISP_LAYER DispLayer, MI_DISP_OutputTiming_e eTiming);
MI_S32 ST_Disp_ChnInit(MI_DISP_LAYER DispLayer, const ST_DispChnInfo_t *pstDispChnInfo);

MI_S32 ST_Disp_DeInit(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer, MI_S32 s32InputPortNum);
MI_S32 ST_Disp_ShowStatus(MI_DISP_LAYER DispLayer, MI_S32 s32InputPortNum, MI_BOOL bIsShow);

#endif //_ST_DISP_H
