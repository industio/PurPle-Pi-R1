#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



#include <mi_gfx.h>
#include <mi_gfx_datatype.h>
#include "../common/verify_gfx_type.h"
#include "../common/verify_gfx.h"

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


int __create_src_surface_ARGB(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *srcRect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;

    if(srcSurf->eColorFmt == E_MI_GFX_FMT_I8
            || srcSurf->eColorFmt == E_MI_GFX_FMT_I4
            || srcSurf->eColorFmt == E_MI_GFX_FMT_I2)
    {
        if(__create_src_surface_I8(srcSurf, srcRect, data))
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

    if((ret = _gfx_alloc_surface(srcSurf, data, "blitsrc")) < 0)
    {
        return ret;
    }

    srcRect->s32Xpos = 0;
    srcRect->s32Ypos = 0;
    srcRect->u32Width = srcSurf->u32Width / 3;
    srcRect->u32Height = srcSurf->u32Height / 3;
    color = 0XFFFF0000;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width / 3;
    srcRect->s32Ypos = srcSurf->u32Height / 3;
    srcRect->u32Width = srcSurf->u32Width / 3;
    srcRect->u32Height = srcSurf->u32Height / 3;
    color = 0XFF00FF00;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);

    srcRect->s32Xpos = srcSurf->u32Width - srcSurf->u32Width / 3;
    srcRect->s32Ypos = srcSurf->u32Height - srcSurf->u32Height / 3;
    srcRect->u32Width = srcSurf->u32Width / 3;
    srcRect->u32Height = srcSurf->u32Height / 3;
    color = 0XFF0000FF;
    MI_GFX_QuickFill(srcSurf, srcRect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}

void __test_blit_mirror_H(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_HORIZONTAL;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}
void __test_blit_mirror_V(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_VERTICAL;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}

void __test_blit_mirror_HV(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_BOTH;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}
void __test_blit_color_ARGB8888(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));

    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}

void __test_blit_color_ARGB1555(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));

    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB1555;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}
void __test_blit_color_RGB565(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));

    dstSurf.eColorFmt = E_MI_GFX_FMT_RGB565;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);


}
void __test_blit_color_ARGB4444(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));

    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB4444;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
}

void __test_blit_color_I8(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{

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

    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));

    dstSurf.eColorFmt = E_MI_GFX_FMT_I8;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
}


void __test_blit_clip_SRC(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{

    srcRect.s32Xpos = srcSurf.u32Width / 4;
    srcRect.s32Ypos = srcSurf.u32Height / 4;
    srcRect.u32Width = srcSurf.u32Width / 2;
    srcRect.u32Height = srcSurf.u32Height / 2;

    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;


    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}

void __test_blit_clip_DST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    dstRect.s32Xpos = dstSurf.u32Width / 4;
    dstRect.s32Ypos = dstSurf.u32Height / 4;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height / 2;

    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}
void __test_blit_clip_DstClip(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{

    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos + dstRect.u32Width / 3;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos + dstRect.u32Height / 3;
    stOpt.stClipRect.u32Width = dstRect.u32Width / 2;
    stOpt.stClipRect.u32Height = dstRect.u32Height / 2;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}

void __test_blit_overlap(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                         MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{

    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);


    srcSurf = dstSurf;
    srcRect = dstRect;
    srcRect.u32Height = srcRect.u32Height - srcRect.u32Height / 4;
    srcRect.u32Width = srcRect.u32Width - srcRect.u32Width / 4;


    dstRect.s32Xpos = dstRect.u32Width / 4;
    dstRect.s32Ypos = dstRect.u32Height / 4;
    dstRect.u32Width = dstRect.u32Width - dstRect.s32Xpos;
    dstRect.u32Height = dstRect.u32Height - dstRect.s32Ypos;
    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width ;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);


    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

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

    if((ret = _gfx_alloc_surface(&dstSurf, &dstData, "blitdst")) < 0)
    {
        return ret;
    }

    _gfx_sink_surface(&srcSurf, srcData, "blitsrc");

    __test_blit_mirror_H(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_mirror_V(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_mirror_HV(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_color_ARGB8888(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_color_ARGB1555(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_color_RGB565(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_color_ARGB4444(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_color_I8(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_clip_SRC(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_clip_DST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_clip_DstClip(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blit_overlap(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);

    _gfx_free_surface(&dstSurf, dstData);

    return 0;
}
