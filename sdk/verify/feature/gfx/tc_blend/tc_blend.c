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


int __create_src_surface(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;
    MI_GFX_Rect_t rect = *Rect;
    MI_GFX_Rect_t *srcRect = &rect;

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
int __refill_dst_surface(MI_GFX_Surface_t *Surf, MI_GFX_Rect_t *Rect)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;

//    if(Surf->eColorFmt == E_MI_GFX_FMT_I8
//            || Surf->eColorFmt == E_MI_GFX_FMT_I4
//            || Surf->eColorFmt == E_MI_GFX_FMT_I2)
//    {
//        memset(*data, 3, Surf->u32Width * Surf->u32Height / 3);
//        memset(*data + Surf->u32Width * Surf->u32Height / 3, 2, Surf->u32Width * Surf->u32Height / 3);
//        memset(*data + Surf->u32Width * Surf->u32Height * 2 / 3, 1, Surf->u32Width * Surf->u32Height / 3);
//
//        MI_GFX_Palette_t stPalette;
//        memset(&stPalette, 0, sizeof(stPalette));
//        stPalette.u16PalStart =  1;
//        stPalette.u16PalEnd = 3;
//
//        stPalette.aunPalette[1].RGB.u8A = 0XFF;
//        stPalette.aunPalette[1].RGB.u8R = 0XFF;
//        stPalette.aunPalette[1].RGB.u8G = 0;
//        stPalette.aunPalette[1].RGB.u8B = 0;
//
//        stPalette.aunPalette[2].RGB.u8A = 0XFF;
//        stPalette.aunPalette[2].RGB.u8R = 0;
//        stPalette.aunPalette[2].RGB.u8G = 0XFF;
//        stPalette.aunPalette[2].RGB.u8B = 0;
//
//        stPalette.aunPalette[3].RGB.u8A = 0XFF;
//        stPalette.aunPalette[3].RGB.u8R = 0;
//        stPalette.aunPalette[3].RGB.u8G = 0;
//        stPalette.aunPalette[3].RGB.u8B = 0;
//
//        MI_GFX_SetPalette(E_MI_GFX_FMT_I8, &stPalette);
//        return 0;
//    }

    Rect->s32Xpos = 0;
    Rect->s32Ypos = 0;
    Rect->u32Width = Surf->u32Width ;
    Rect->u32Height = Surf->u32Height / 2;
    color = 0X80FFFFFF;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);
    Rect->s32Xpos = 0;
    Rect->s32Ypos = Surf->u32Height / 2;
    Rect->u32Width = Surf->u32Width;
    Rect->u32Height = Surf->u32Height / 2;
    color = 0XFFFFFFFF;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;

}
int __refill_dst_surface_as_src(MI_GFX_Surface_t *Surf, MI_GFX_Rect_t *Rect)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;

//    if(Surf->eColorFmt == E_MI_GFX_FMT_I8
//            || Surf->eColorFmt == E_MI_GFX_FMT_I4
//            || Surf->eColorFmt == E_MI_GFX_FMT_I2)
//    {
//        memset(*data, 1, Surf->u32Width * Surf->u32Height / 3);
//        memset(*data + Surf->u32Width * Surf->u32Height / 3, 2, Surf->u32Width * Surf->u32Height / 3);
//        memset(*data + Surf->u32Width * Surf->u32Height * 2 / 3, 3, Surf->u32Width * Surf->u32Height / 3);
//
//        MI_GFX_Palette_t stPalette;
//        memset(&stPalette, 0, sizeof(stPalette));
//        stPalette.u16PalStart =  1;
//        stPalette.u16PalEnd = 3;
//
//        stPalette.aunPalette[1].RGB.u8A = 0XFF;
//        stPalette.aunPalette[1].RGB.u8R = 0XFF;
//        stPalette.aunPalette[1].RGB.u8G = 0;
//        stPalette.aunPalette[1].RGB.u8B = 0;
//
//        stPalette.aunPalette[2].RGB.u8A = 0XFF;
//        stPalette.aunPalette[2].RGB.u8R = 0;
//        stPalette.aunPalette[2].RGB.u8G = 0XFF;
//        stPalette.aunPalette[2].RGB.u8B = 0;
//
//        stPalette.aunPalette[3].RGB.u8A = 0XFF;
//        stPalette.aunPalette[3].RGB.u8R = 0;
//        stPalette.aunPalette[3].RGB.u8G = 0;
//        stPalette.aunPalette[3].RGB.u8B = 0;
//
//        MI_GFX_SetPalette(E_MI_GFX_FMT_I8, &stPalette);
//        return 0;
//    }

    Rect->s32Xpos = 0;
    Rect->s32Ypos = 0;
    Rect->u32Width = Surf->u32Width / 6;
    Rect->u32Height = Surf->u32Height / 3;
    color = 0XFFFF0000;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);

    Rect->s32Xpos = Surf->u32Width / 6;
    Rect->s32Ypos = Surf->u32Height / 3;

    color = 0X80FF0000;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);

    Rect->s32Xpos = Surf->u32Width / 3;
    Rect->s32Ypos = Surf->u32Height - Surf->u32Height / 3;

    color = 0X00800000;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);


    Rect->s32Xpos = Surf->u32Width / 2;
    Rect->s32Ypos = 0;

    color = 0X000000FF;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);

    Rect->s32Xpos = Surf->u32Width / 2 + Surf->u32Width / 6;
    Rect->s32Ypos = Surf->u32Height / 3;

    color = 0X800000FF;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);

    Rect->s32Xpos = Surf->u32Width / 2 + Surf->u32Width / 3;
    Rect->s32Ypos = Surf->u32Height - Surf->u32Height / 3;

    color = 0XFF000080;
    MI_GFX_QuickFill(Surf, Rect, color, &fence);


    MI_GFX_WaitAllDone(FALSE, fence);

    return 0;

}

int __create_dst_surface(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data)
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

void __test_blend_colorkey_SRC_RGB_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_RGB_OP_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_SRC_RGB_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_RGB_OP_NOT_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_SRC_ALPHA_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_ALPHA_OP_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_SRC_ALPHA_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_ALPHA_OP_NOT_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_SRC_ARGB_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_ARGB_OP_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_SRC_ARGB_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.stSrcColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stSrcColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stSrcColorKeyInfo.eCKeyOp = E_MI_GFX_ARGB_OP_NOT_EQUAL;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stSrcColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
void __test_blend_colorkey_DST_RGB_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;

    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_RGB_OP_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_DST_RGB_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);


    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;


    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_RGB_OP_NOT_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_DST_ALPHA_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);


    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;


    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_ALPHA_OP_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_DST_ALPHA_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);


    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;


    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_ALPHA_OP_NOT_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_DST_ARGB_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);


    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;


    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_ARGB_OP_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

void __test_blend_colorkey_DST_ARGB_NOT_EQUAL(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Rect_t oriSrcRect = srcRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;
    __refill_dst_surface_as_src(&dstSurf, &dstRect);
    __refill_dst_surface(&srcSurf, &srcRect);
    clock_gettime(CLOCK_MONOTONIC, &ts1);


    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = oriDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = oriDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = oriDstRect.u32Width;
    stOpt.stClipRect.u32Height = oriDstRect.u32Height;


    stOpt.stDstColorKeyInfo.bEnColorKey = TRUE;
    stOpt.stDstColorKeyInfo.eCKeyFmt = srcSurf.eColorFmt;
    stOpt.stDstColorKeyInfo.eCKeyOp = E_MI_GFX_ARGB_OP_NOT_EQUAL;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorStart = 0X80FF0000;
    stOpt.stDstColorKeyInfo.stCKeyVal.u32ColorEnd = 0X80FF0000;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &oriSrcRect, &dstSurf, &oriDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface_as_src(&dstSurf, &oriDstRect);
}

/// Csrc
void __test_blend_COEF_CSRC(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                            MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

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

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * Aconst + Cdst * (1 - Aconst)
void __test_blend_COEF_CONST_SRC(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0x80000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_COLORALPHA | E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
///  Csrc * Asrc + Cdst * (1 - Asrc)
void __test_blend_COEF_ASRC_CSRC(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * Adst + Cdst * (1 - Adst)
void __test_blend_COEF_ADST_CSRC(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_DESTALPHA;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_INVDESTALPHA;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Cdst
void __test_blend_COEF_CDST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                            MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * (1 - Aconst) + Cdst * Aconst
void __test_blend_COEF_CONST_DST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xC0000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_COLORALPHA;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_SRCALPHA;

    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * (1 - Asrc) + Cdst * Asrc
void __test_blend_COEF_ASRC_CDST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0x10000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_SRCALPHA;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
///  Csrc * (1 - Adst) + Cdst * Adst
void __test_blend_COEF_ADST_CDST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xC0000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_DST_PREMULTIPLY;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_INVDESTALPHA;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * Aconst
void __test_blend_COEF_CSRC_CONST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                  MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0x80000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_SRC_PREMULTCOLOR;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}
/// Csrc * (1-Aconst)
void __test_blend_COEF_CSRC_INVCONST(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                                     MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    MI_GFX_Rect_t oriDstRect = dstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stOpt.stClipRect.s32Xpos = dstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = dstRect.s32Ypos;
    stOpt.stClipRect.u32Width = dstRect.u32Width;
    stOpt.stClipRect.u32Height = dstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xC0000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_COLORALPHA;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;

    MI_GFX_BitBlit(&srcSurf, &srcRect, &dstSurf, &dstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    __refill_dst_surface(&dstSurf, &oriDstRect);
}

int main()
{
#define SURFACE_COLOR_FMT E_MI_GFX_FMT_ARGB1555

    MI_S32 ret = 0;
    MI_GFX_Surface_t srcSurf;
    MI_GFX_Rect_t srcRect;
    char *srcData;

    MI_SYS_Init();
    MI_GFX_Open();
    srcSurf.eColorFmt = SURFACE_COLOR_FMT;
    srcSurf.phyAddr = 0;
    srcSurf.u32Height = 1080 > MAX_H ? MAX_H : 1080;
    srcSurf.u32Width = 1920 > MAX_W ? MAX_W : 1920 ;
    srcSurf.u32Stride = srcSurf.u32Width * getBpp(srcSurf.eColorFmt);
    __create_src_surface(&srcSurf, &srcRect, &srcData);

    MI_GFX_Surface_t dstSurf;
    MI_GFX_Rect_t dstRect;
    char *dstData;
    dstSurf.eColorFmt = SURFACE_COLOR_FMT;
    dstSurf.phyAddr = 0;
    dstSurf.u32Height = 1080 > MAX_H ? MAX_H : 1080;
    dstSurf.u32Width = 1920 > MAX_W ? MAX_W : 1920 ;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos =  0;
    dstRect.u32Height = dstSurf.u32Height;
    dstRect.u32Width = dstSurf.u32Width;
    srcRect = dstRect;
    __create_dst_surface(&dstSurf, &dstRect, &dstData);

    _gfx_sink_surface(&dstSurf, dstData, "blenddst");
    _gfx_sink_surface(&srcSurf, srcData, "blendsrc");

    __test_blend_colorkey_SRC_RGB_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_SRC_RGB_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_SRC_ALPHA_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_SRC_ALPHA_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_SRC_ARGB_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_SRC_ARGB_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_RGB_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_RGB_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_ALPHA_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_ALPHA_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_ARGB_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_colorkey_DST_ARGB_NOT_EQUAL(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);

    _gfx_free_surface(&srcSurf, srcData);
    _gfx_free_surface(&dstSurf, dstData);
    __create_src_surface(&srcSurf, &srcRect, &srcData);
    __create_dst_surface(&dstSurf, &dstRect, &dstData);

    __test_blend_COEF_CSRC(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_CONST_SRC(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_ASRC_CSRC(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_ADST_CSRC(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_CDST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_CONST_DST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_ASRC_CDST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_ADST_CDST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_CSRC_CONST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);
    __test_blend_COEF_CSRC_INVCONST(srcSurf, srcRect, dstSurf, dstRect, dstData, TRUE);


    _gfx_free_surface(&srcSurf, srcData);
    _gfx_free_surface(&dstSurf, dstData);
    //MI_GFX_Close();
    //MI_SYS_Exit();

    return 0;
}

