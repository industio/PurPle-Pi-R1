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
#ifndef _ONVIF_FUNCTION_H_
#define _ONVIF_FUNCTION_H_

#include <time.h>
#include "onvif_type.h"

#define ETH_NAME            "eth0"
#define ETH_NAME2           "wlan0"

#define ONVIF_LISTEN_PORT	3702
#define ONVIF_SERVICE_PORT	8089

int mst_GetMac(char *pInterface, unsigned char *pMac);

int mst_GetV4Ip(char *pInterface, unsigned int *ip);

int mst_GetV6Ip(char *pInterface, char *ip);

int mst_GetSysDataTime(onvif_system_date_time* tSysDataTime);

int mst_GetDefaultGateway(char *pInterface, char* szDefaultGateway );

int mst_netGetIp(char *pInterface, unsigned int *ip);

#endif

