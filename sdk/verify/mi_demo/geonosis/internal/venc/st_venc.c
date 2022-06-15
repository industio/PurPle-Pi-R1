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
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "st_venc.h"
#include "st_common.h"

MI_S32 ST_Venc_CreateChannel(MI_VENC_CHN VencChn, MI_VENC_ChnAttr_t *pstAttr)
{
    MI_U32 u32DevId = -1;
    MI_SYS_ChnPort_t stChnOutputPort;

    if (pstAttr == NULL)
    {
        ST_ERR("invalid param\n");
        return -1;
    }

    ExecFunc(MI_VENC_CreateChn(VencChn, pstAttr), MI_SUCCESS);
    if (pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        MI_VENC_ParamJpeg_t stParamJpeg;

        memset(&stParamJpeg, 0, sizeof(stParamJpeg));
        ExecFunc(MI_VENC_GetJpegParam(VencChn, &stParamJpeg), MI_SUCCESS);

        ST_INFO("Get u32Qfactor:%d\n", stParamJpeg.u32Qfactor);

        stParamJpeg.u32Qfactor = 50;

        ExecFunc(MI_VENC_SetJpegParam(VencChn, &stParamJpeg), MI_SUCCESS);
    }

    ExecFunc(MI_VENC_GetChnDevid(VencChn, &u32DevId), MI_SUCCESS);

    memset(&stChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnOutputPort.u32DevId = u32DevId;
    stChnOutputPort.eModId = E_MI_MODULE_ID_VENC;
    stChnOutputPort.u32ChnId = VencChn;
    stChnOutputPort.u32PortId = 0;
    //This was set to (5, 10) and might be too big for kernel
    //ExecFunc(MI_SYS_SetChnOutputPortDepth(&stChnOutputPort, 5, 5), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_S32 ST_Venc_DestoryChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StopRecvPic(VencChn), MI_SUCCESS);
    ExecFunc(MI_VENC_DestroyChn(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_S32 ST_Venc_StartChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StartRecvPic(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_S32 ST_Venc_StopChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StopRecvPic(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

