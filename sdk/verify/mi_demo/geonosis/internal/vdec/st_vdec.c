#include <stdio.h>

#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "st_common.h"

MI_S32 ST_Vdec_CreateChannel(MI_S32 s32VdecChannel, MI_VDEC_ChnAttr_t *PstVdecChnAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    STCHECKRESULT(MI_VDEC_CreateChn(s32VdecChannel, PstVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(s32VdecChannel));

    return MI_SUCCESS;
}
