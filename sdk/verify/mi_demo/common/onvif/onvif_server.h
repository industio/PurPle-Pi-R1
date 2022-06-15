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
#ifndef __ONVIF_SERVER_H_
#define __ONVIF_SERVER_H_


#ifdef __cplusplus
extern "C"
{
#endif
void MST_ONVIF_Init();
void MST_ONVIF_StartTask();
void MST_ONVIF_StopTask();

#ifdef __cplusplus
}
#endif

#endif
