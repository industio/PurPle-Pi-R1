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
#ifndef _FB_LAYER_H_
#define _FB_LAYER_H_
#include <linux/fb.h>
#include <stdbool.h>
#include "FBTypes.h"

typedef enum
{
    E_FBM_GET_LOCATION,
    E_FBM_SET_LOCATION,

    E_FBM_GET_SHOWN,
    E_FBM_SET_SHOWN,

    E_FBM_GET_GLOBAL_ALPHA,
    E_FBM_SET_GLOBAL_ALPHA,

    E_FBM_GET_COLORKEY,
    E_FBM_SET_COLORKEY,

    E_FBM_GET_LAYER_ATTRIBUTE,
    E_FBM_SET_LAYER_ATTRIBUTE,
} FBMetric;

typedef struct
{
    bool enable;
    bool channel;
    uint8_t alpha0;
    uint8_t alpha1;
    uint8_t global_alpha;
    uint8_t reserved;
} FBGlobalAlpha;

typedef struct
{
    bool enable;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} FBColorKey;

typedef enum
{
    E_FB_CLRSPACE_RGB = 0,
    E_FB_CLRSPACE_YUV = 1,
} FBColorSpace;

typedef enum
{
    E_FB_DISP_PLANE_IP0 = 0,
    E_FB_DISP_PLANE_MIXER2VE = 1,
    E_FB_DISP_PLANE_OP0 = 2,
    E_FB_DISP_PLANE_VOP = 3,
    E_FB_DISP_PLANE_IP1 = 4,
    E_FB_DISP_PLANE_IP_MAIN = 5,
    E_FB_DISP_PLANE_IP_SUB = 6,
    E_FB_DISP_PLANE_MIXER2OP = 7,
    E_FB_DISP_PLANE_VOP_SUB = 8,
    E_FB_DISP_PLANE_FRC = 9,
    E_FB_DISP_PLANE_VE = 10,
    E_FB_DISP_PLANE_OP1 = 11,
    E_FB_DISP_PLANE_MIXER2OP1 = 12,
    E_FB_DISP_PLANE_DIP = 13,
    E_FB_DISP_PLANE_GOPScaling = 14,
    E_FB_DISP_PLANE_BYPASS = 15,
    E_FB_DISP_PLANE_OP_DUAL_RATE = 16,
}FBDisplayPlane;

typedef struct
{
    uint32_t xpos;
    uint32_t ypos;
    uint32_t dst_width;
    uint32_t dst_height;
    uint32_t disp_width;
    uint32_t disp_height;
    uint32_t screen_width;
    uint32_t screen_height;
    bool premulti;
    FBColorFmt colorfmt;
    FBColorSpace output_colorspace;
    FBDisplayPlane dest_disp_plane;
    uint32_t attr_mask;
}FBLayerAttr;

typedef struct
{
    //device fbdevice Id
    uint8_t fb_device_fd;
    //framebuffer varinfo
    struct fb_var_screeninfo fb_vinfo;
    //framebuffer varinfo
    struct fb_fix_screeninfo fb_finfo;
    //Store Virtual address of front buffer and back buffer
    char* p_fb_screen[2];
    //Store Physical address of front buffer and back buffer
    char* p_fb_screenPhyAddr[2];
    //Store Virtual address of framebuffer via mmap
    char* p_fb_screenMmapAddr;
    //Fb color format
    FBColorFmt e_fb_clr_fmt;
    //bytes per pixel
    uint8_t n_fb_bytePerPixel;
    //Used to flag whether fb is ready
    bool b_fb_ready_flag;
    //front buffer sequence of framebuffer
    uint8_t n_fb_front_screen;
    //back buffer sequence of framebuffer
    uint8_t n_fb_back_screen;
    //buffer count of Framebuffer
    uint8_t n_fb_screen_count;
    //FB Layer Id
    uint8_t n_fb_layerId;
} FBLayerInfo_t;

typedef uint32_t FBCOLOR;

int initFBLayerInfo(uint8_t nId, FBLayerInfo_t* pFbLayerInfo);

int beginDrawOnFBLayer(bool bErase, bool waitforReady,
    FBLayerInfo_t* pFbLayerInfo);
int flipOperationOnFBLayer(FBLayerInfo_t* pFbLayerInfo);
int endDrawOnFBLayer(FBLayerInfo_t* pFbLayerInfo);
//Draw Operation
int strechRectOnFBLayer(FBRect* pDstRect, FBRect* pSrcRect,
    FBImage* oImage, FBLayerInfo_t* pFbLayerInfo);
int  fillRectOnFBLayer(FBRect* pDstRect, FBCOLOR nColor,
    FBLayerInfo_t* pFbLayerInfo);
int drawLineOnFBLayer(FBPoint* oPoint1, FBPoint* oPoint2,
    FBCOLOR nColor, int nWidth, FBLayerInfo_t* pFbLayerInfo);
int drawLinesOnFBLayer(FBPoint* pPoints, size_t nCount,
    FBCOLOR nColor, int nWidth, FBLayerInfo_t* pFbLayerInfo);
int stretchBlitImgByGE(const char* fileName, int srcWidth, int srcHeight,
    int srcStride, FBColorFmt srcFmt, FBLayerInfo_t* pFbLayerInfo);
//Ioctl operation
int ioctlOnFBLayer(FBMetric nCmd, void *pArgs,
    FBLayerInfo_t* pFbLayerInfo);
#endif
