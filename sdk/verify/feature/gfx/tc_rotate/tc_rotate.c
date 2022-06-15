#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mi_gfx.h>
#include <mi_gfx_datatype.h>
#include "../common/verify_gfx_type.h"
#include "../common/verify_gfx.h"
#include "../common/blitutil.h"
#include "convert_from_argb.h"


#define MAX_H ((unsigned int)1<<32-1)
#define MAX_W ((unsigned int)1<<32-1)
int __create_src_surface_I8(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *srcRect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;

    if((ret = _gfx_alloc_surface(srcSurf, data, "blitsrc")) < 0)
    {
        printf("%s %d fail\n", __FUNCTION__, __LINE__);
        return ret;
    }

    memset(*data, 1, srcSurf->u32Width * srcSurf->u32Height / 3);
    memset(*data + srcSurf->u32Width * srcSurf->u32Height / 3, 2, srcSurf->u32Width * srcSurf->u32Height / 3);
    memset(*data + srcSurf->u32Width * srcSurf->u32Height * 2 / 3, 3, srcSurf->u32Width * srcSurf->u32Height / 3);
    return 0;
}

int __create_src_surface_ARGB(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;
    MI_GFX_Rect_t rect = *Rect;
    MI_GFX_Rect_t *srcRect = &rect;
    
    if(srcSurf->eColorFmt==E_MI_GFX_FMT_I8
       ||srcSurf->eColorFmt==E_MI_GFX_FMT_I4
       ||srcSurf->eColorFmt==E_MI_GFX_FMT_I2)
    {
        if(__create_src_surface_I8(srcSurf,srcRect,data))
            return -1;
    
        MI_GFX_Palette_t stPalette;
        memset(&stPalette, 0, sizeof(stPalette));
        stPalette.u16PalStart =  1;
        stPalette.u16PalEnd = 3;
        
        stPalette.aunPalette[1].RGB.u8A = 0XFF;
        stPalette.aunPalette[1].RGB.u8R = 0XFF;
        stPalette.aunPalette[1].RGB.u8G = 0;
        stPalette.aunPalette[1].RGB.u8B = 0;
        
        stPalette.aunPalette[2].RGB.u8A = 0XFF;
        stPalette.aunPalette[2].RGB.u8R = 0;
        stPalette.aunPalette[2].RGB.u8G = 0XFF;
        stPalette.aunPalette[2].RGB.u8B = 0;
        
        stPalette.aunPalette[3].RGB.u8A = 0XFF;
        stPalette.aunPalette[3].RGB.u8R = 0;
        stPalette.aunPalette[3].RGB.u8G = 0;
        stPalette.aunPalette[3].RGB.u8B = 0;
        
        MI_GFX_SetPalette(E_MI_GFX_FMT_I8, &stPalette);
        return 0;
    }

    if((ret = _gfx_alloc_surface(srcSurf, data, "blendSrc")) < 0)
    {
        return ret;
    }
    srcRect->s32Xpos = 0;
    srcRect->s32Ypos = 0;
    srcRect->u32Width = srcSurf->u32Width / 6;
    srcRect->u32Height = srcSurf->u32Height / 3;
    color = 0XFFFF0000;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width / 6;
    srcRect->s32Ypos = srcSurf->u32Height / 3;

    color = 0X80FF0000;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width / 3;
    srcRect->s32Ypos = srcSurf->u32Height - srcSurf->u32Height / 3;

    color = 0X00800000;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);


    srcRect->s32Xpos = srcSurf->u32Width / 2;
    srcRect->s32Ypos = 0;

    color = 0X000000FF;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width / 2 + srcSurf->u32Width / 6;
    srcRect->s32Ypos = srcSurf->u32Height / 3;

    color = 0X800000FF;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width / 2 + srcSurf->u32Width / 3;
    srcRect->s32Ypos = srcSurf->u32Height - srcSurf->u32Height / 3;

    color = 0XFF000080;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);


    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}
int __refill_dst_surface_ARGB(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;

    Rect->s32Xpos = 0;
    Rect->s32Ypos = 0;
    Rect->u32Width = srcSurf->u32Width ;
    Rect->u32Height = srcSurf->u32Height / 2;
    color = 0X80FFFFFF;
    MI_GFX_QuickFill(srcSurf, Rect, color, &fence);
    Rect->s32Xpos = 0;
    Rect->s32Ypos = srcSurf->u32Height / 2;
    Rect->u32Width = srcSurf->u32Width;
    Rect->u32Height = srcSurf->u32Height / 2;
    color = 0XFFFFFFFF;
    MI_GFX_QuickFill(srcSurf, Rect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;

}

int __create_dst_surface_ARGB(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;
    MI_GFX_Rect_t rect = *Rect;
    MI_GFX_Rect_t *srcRect = &rect;

    if((ret = _gfx_alloc_surface(srcSurf, data, "blendDst")) < 0)
    {
        return ret;
    }

    srcRect->s32Xpos = 0;
    srcRect->s32Ypos = 0;
    srcRect->u32Width = srcSurf->u32Width ;
    srcRect->u32Height = srcSurf->u32Height / 2;
    color = 0X80FFFFFF;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);
    srcRect->s32Xpos = 0;
    srcRect->s32Ypos = srcSurf->u32Height / 2;
    srcRect->u32Width = srcSurf->u32Width;
    srcRect->u32Height = srcSurf->u32Height / 2;
    color = 0XFFFFFFFF;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}
int __fill_surface_YUV420SP(MI_GFX_Surface_t *srcSurf, MI_GFX_Surface_t *dstSurf, char *srcData, char *dstData)
{

    int width;
    int height;

    const unsigned char *src_argb;
    int src_stride_argb;
    unsigned char *dst_y;
    int dst_stride_y;
    unsigned char *dst_uv;
    int dst_stride_uv;

    unsigned char *src_bgra = srcData;
    unsigned char *dst_argb = dstData;
    int src_stride_bgra = srcSurf->u32Stride;
    int dst_stride_argb = dstSurf->u32Stride;
    const unsigned char *shuffler = kShuffleMaskBGRAToARGB;
    width = dstSurf->u32Width;
    height = dstSurf->u32Height;

    ARGBShuffle(src_bgra, src_stride_bgra, dst_argb, dst_stride_argb, shuffler, width, height);
    MI_SYS_FlushInvCache(dstData, height * dstSurf->u32Stride);
    memcpy(srcData, dstData, height * dstSurf->u32Stride);

    width = dstSurf->u32Width;
    height = dstSurf->u32Height;
    src_argb = srcData;
    src_stride_argb = srcSurf->u32Stride;
    dst_y = dstData;
    dst_stride_y = dstSurf->u32Width;
    dst_uv = dstData + dst_stride_y * height;
    dst_stride_uv = dstSurf->u32Width ;

    ARGBToNV12(src_argb, src_stride_argb, dst_y, dst_stride_y, dst_uv, dst_stride_uv, width, height);
    MI_SYS_FlushInvCache(dstData, dstSurf->u32Height * dstSurf->u32Stride);
    memcpy(srcData, dstData, dstSurf->u32Height * dstSurf->u32Stride);
    MI_SYS_FlushInvCache(srcData, width * height * 3 / 2);
    return 0;
}

int __test_rotate_ARGB_0(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                         MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface src, dst;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width*getBpp(dstSurf.eColorFmt);

    src.eGFXcolorFmt = srcSurf.eColorFmt;
    src.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    src.h = srcSurf.u32Height;
    src.w = srcSurf.u32Width;
    src.pitch = src.w * src.BytesPerPixel;
    src.phy_addr = srcSurf.phyAddr;

    dst.eGFXcolorFmt = dstSurf.eColorFmt;
    dst.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dst.h = src.h;
    dst.w = src.w;
    dst.pitch = dst.w * dst.BytesPerPixel;
    dst.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = src.h;
    srcClip.right = src.w;
    SstarBlitNormal(&src, &dst, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_ARGB_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface src, dst;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width*getBpp(dstSurf.eColorFmt);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    
    src.eGFXcolorFmt = srcSurf.eColorFmt;
    src.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    src.h = srcSurf.u32Height;
    src.w = srcSurf.u32Width;
    src.pitch = src.w * src.BytesPerPixel;
    src.phy_addr = srcSurf.phyAddr;

    dst.eGFXcolorFmt = dstSurf.eColorFmt;
    dst.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dst.h = dstSurf.u32Height;
    dst.w = dstSurf.u32Width;
    dst.pitch = dst.w * dst.BytesPerPixel;
    dst.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = src.h;
    srcClip.right = src.w;
    SstarBlitCW(&src, &dst, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_ARGB_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface src, dst;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width*getBpp(dstSurf.eColorFmt);

    src.eGFXcolorFmt = srcSurf.eColorFmt;
    src.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    src.h = srcSurf.u32Height;
    src.w = srcSurf.u32Width;
    src.pitch = src.w * src.BytesPerPixel;
    src.phy_addr = srcSurf.phyAddr;

    dst.eGFXcolorFmt = dstSurf.eColorFmt;
    dst.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dst.h = src.h;
    dst.w = src.w;
    dst.pitch = dst.w * dst.BytesPerPixel;
    dst.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = src.h;
    srcClip.right = src.w;
    SstarBlitHVFlip(&src, &dst, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_ARGB_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface src, dst;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width*getBpp(dstSurf.eColorFmt);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    src.eGFXcolorFmt = srcSurf.eColorFmt;
    src.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    src.h = srcSurf.u32Height;
    src.w = srcSurf.u32Width;
    src.pitch = src.w * src.BytesPerPixel;
    src.phy_addr = srcSurf.phyAddr;

    dst.eGFXcolorFmt = dstSurf.eColorFmt;
    dst.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dst.h = dstSurf.u32Height;
    dst.w = dstSurf.u32Width;
    dst.pitch = dst.w * dst.BytesPerPixel;
    dst.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = src.h;
    srcClip.right = src.w;
    SstarBlitCCW(&src, &dst, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_YUV420SP_0(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                             MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitNormal(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.h;
    dstUV.w = srcUV.w;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitNormal(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_YUV420SP_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitHVFlip(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.h;
    dstUV.w = srcUV.w;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitHVFlip(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}


int main()
{
    MI_S32 ret = 0;
    MI_GFX_Surface_t srcSurf;
    MI_GFX_Rect_t srcRect;
    char *srcData;

    MI_SYS_Init();
    MI_GFX_Open();
    srcSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    srcSurf.phyAddr = 0;
    srcSurf.u32Height = 1080 > MAX_H ? MAX_H : 1080;
    srcSurf.u32Width = 1920 > MAX_W ? MAX_W : 1920 ;
    srcSurf.u32Stride = srcSurf.u32Width * getBpp(srcSurf.eColorFmt);
    __create_src_surface_ARGB(&srcSurf, &srcRect, &srcData);

    MI_GFX_Surface_t dstSurf;
    MI_GFX_Rect_t dstRect;
    char *dstData;
    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    dstSurf.phyAddr = 0;
    dstSurf.u32Height = 1080 > MAX_H ? MAX_H : 1080;
    dstSurf.u32Width = 1920 > MAX_W ? MAX_W : 1920 ;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos =  0;
    dstRect.u32Height = dstSurf.u32Height;
    dstRect.u32Width = dstSurf.u32Width;
    srcRect = dstRect;
    __create_dst_surface_ARGB(&dstSurf, &dstRect, &dstData);

    _gfx_sink_surface(&srcSurf, srcData, "blendsrc_ARGB8888");
    _gfx_sink_surface(&dstSurf, dstData, "blenddst_ARGB8888");
    
    __test_rotate_ARGB_0(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_ARGB_90(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_ARGB_180(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_ARGB_270(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);

    __fill_surface_YUV420SP(&srcSurf, &dstSurf, srcData, dstData);
    _gfx_sink_surface(&srcSurf, srcData, "blendsrc_YVU420SP");

    __test_rotate_YUV420SP_0(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_YUV420SP_90(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_YUV420SP_180(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_rotate_YUV420SP_270(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);

    _gfx_free_surface(&srcSurf, srcData);
    _gfx_free_surface(&dstSurf, dstData);
    return 0;
}


