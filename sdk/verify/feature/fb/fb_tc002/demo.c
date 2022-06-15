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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include "FBLayer.h"
#include "FBDefs.h"

#define MAIN_LAYER_ID 0

int main()
{
    int nImgBytes = 72*72*2;
    uint32_t nColor = 0;
    FBRect dst = { 0, 0, 300, 200 };

    FBPoint pt0 = { 100 , 200 };
    FBPoint pt1 = { 300, 200 };
    FBPoint pt2 = { 300, 400 };
    FBPoint pt3 = { 100, 400 };
    FBPoint pt4 = { 100, 200 };

    FBPoint rect[5] = {pt0, pt1, pt2, pt3, pt4};

    FBRect rect1 = { 100, 200, 200, 200 };

    FILE* fp = fopen("bitmap", "rb");
    if (fp == NULL) {
        printf("Can not find raw data file bitmap in current directory!!\n");
        return 0;
    }
    uint8_t buff[72*72*2] = {0};
    fread(buff, 1, nImgBytes, fp);
    fclose(fp);

    FBRect imgdst0 = { 0, 0, 32, 32 };
    FBRect imgdst1 = { 100, 400, 50, 50 };
    FBRect imgdst2 = { 150, 400, 100, 50 };
    FBRect imgdst3 = { 250, 400, 50, 100 };
    FBRect imgdst4 = { 300, 400, 72, 72 };
    FBRect imgsrc0 = { 10, 10, 50, 50 };
    FBRect imgsrc1 = { 0, 0, 36, 36 };
    FBImage img = { 72, 72, buff,
        E_FB_CLRFMT_ARGB1555 };

    FBLayerInfo_t fbLayerInfo;
    memset(&fbLayerInfo, 0, sizeof(FBLayerInfo_t));
    initFBLayerInfo(MAIN_LAYER_ID, &fbLayerInfo);
    while (1)
    {
        nColor = rand() % 0xFFFFFFFF;
        beginDrawOnFBLayer(false, true, &fbLayerInfo);
        {
            fillRectOnFBLayer(NULL,nColor, &fbLayerInfo);
            fillRectOnFBLayer(&rect1, 0, &fbLayerInfo);
            drawLinesOnFBLayer(rect, 5, 0x00FFFFFF, 1, &fbLayerInfo);
#if HAS_GE
            stretchBlitImgByGE("bitmap", 72, 72, 144,
                E_FB_CLRFMT_ARGB1555, &fbLayerInfo);
#else
            strechRectOnFBLayer(&imgdst0, NULL, &img, &fbLayerInfo);
#endif
        }
        flipOperationOnFBLayer(&fbLayerInfo);
        endDrawOnFBLayer(&fbLayerInfo);
        //usleep to idle
        usleep(1000000);
    }
    return 0;
}
