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

#include "mi_ao.h"
#include "ao.h"
#include <stdio.h>

Ao::Ao()
{
    
}
Ao::~Ao()
{
    printf("func: %s\n", __FUNCTION__);
}
void Ao::Init()
{
    printf("func: %s\n", __FUNCTION__);
    
}
void Ao::Deinit()
{
    printf("func: %s\n", __FUNCTION__);
}

