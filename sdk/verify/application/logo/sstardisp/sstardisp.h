#ifndef __SSTARDISP__H__
#define __SSTARDISP__H__
#include "../config.h"
#include <mi_disp_datatype.h>

#ifdef MI_DISP_INTF_TYPE_HDMI
#include "mi_hdmi_datatype.h"
#include "mi_hdmi.h"
#endif
typedef struct stTimingArray_s
{
    char desc[50];
    MI_DISP_OutputTiming_e eOutputTiming;
#ifdef MI_DISP_INTF_TYPE_HDMI
    MI_HDMI_TimingType_e eHdmiTiming;
#else
    int eHdmiTiming;
#endif
    MI_U16 u16Width;
    MI_U16 u16Height;
}stTimingArray_t;
int sstar_disp_init(MI_DISP_PubAttr_t* pstDispPubAttr);
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr);
stTimingArray_t *getTimingByAlias(char* sTimingAlias);
void dumpTimingAlias(void);


#endif
