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
#define min(a,b) ((a) > (b) ? (b) : (a))
void __test_fill_ARGB8888(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                          char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;
    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_U32 color = 0xFFFF0000;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    color = 0x00FF0000;
    dstRect.s32Xpos = dstSurf.u32Width / 2;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);
}
void __test_fill_ARGB1555(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                          char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;
    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB1555;
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_U32 color = 0xFFFF0000;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    color = 0x00FF0000;
    dstRect.s32Xpos = dstSurf.u32Width / 2;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}
void __test_fill_ARGB4444(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                          char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;
    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB4444;
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_U32 color = 0xFFFF0000;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    color = 0x00FF0000;
    dstRect.s32Xpos = dstSurf.u32Width / 2;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}
void __test_fill_RGB565(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                        char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;
    dstSurf.eColorFmt = E_MI_GFX_FMT_RGB565;
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_U32 color = 0xFFFF0000;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    color = 0x00FF0000;
    dstRect.s32Xpos = dstSurf.u32Width / 2;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}
void __test_fill_I8(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                    char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;

    MI_GFX_Palette_t stPalette;
    memset(&stPalette, 0, sizeof(stPalette));
    stPalette.u16PalStart =  1;
    stPalette.u16PalEnd = 3;

    stPalette.aunPalette[1].RGB.u8A = 0XFF;
    stPalette.aunPalette[1].RGB.u8R = 0XFF;
    stPalette.aunPalette[1].RGB.u8G = 0;
    stPalette.aunPalette[1].RGB.u8B = 0;

    stPalette.aunPalette[2].RGB.u8A = 0;
    stPalette.aunPalette[2].RGB.u8R = 0XFF;
    stPalette.aunPalette[2].RGB.u8G = 0;
    stPalette.aunPalette[2].RGB.u8B = 0;

    stPalette.aunPalette[3].RGB.u8A = 0XFF;
    stPalette.aunPalette[3].RGB.u8R = 0;
    stPalette.aunPalette[3].RGB.u8G = 0;
    stPalette.aunPalette[3].RGB.u8B = 0;

    MI_GFX_SetPalette(E_MI_GFX_FMT_I8, &stPalette);
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    dstSurf.eColorFmt = E_MI_GFX_FMT_I8;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    MI_U32 color = 0xFFFF0001;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    color = 0x00FF0003;
    dstRect.s32Xpos = dstSurf.u32Width / 2;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width / 2;
    dstRect.u32Height = dstSurf.u32Height;
    MI_GFX_QuickFill(&dstSurf, &dstRect, color, &fence);

    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);

}

void __test_draw_LINE(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
                      char *dstData, MI_BOOL bSinkSurf)
{
    MI_U16 fence = 0;
    MI_GFX_Line_t stLine;
    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
    memset(&stLine, 0, sizeof(MI_GFX_Line_t));
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    stLine.bColorGradient = FALSE;
    stLine.stPointFrom.s16x = dstRect.s32Xpos;
    stLine.stPointFrom.s16y = dstRect.s32Ypos;
    stLine.stPointTo.s16x = dstRect.u32Width;
    stLine.stPointTo.s16y = dstRect.s32Ypos;
    stLine.u16Width = 5;
    stLine.u32ColorFrom = 0X80FF0000;
    stLine.u32ColorTo = 0X800000FF;
    MI_GFX_DrawLine(&dstSurf, &stLine, &fence);
    stLine.stPointFrom.s16x = dstRect.u32Width - stLine.u16Width;
    stLine.stPointFrom.s16y = dstRect.s32Ypos;
    stLine.stPointTo.s16x = dstRect.u32Width - stLine.u16Width;
    stLine.stPointTo.s16y = dstRect.u32Height;
    MI_GFX_DrawLine(&dstSurf, &stLine, &fence);
    stLine.stPointFrom.s16x = dstRect.u32Width;
    stLine.stPointFrom.s16y = dstRect.u32Height - stLine.u16Width;
    stLine.stPointTo.s16x = dstRect.s32Xpos;
    stLine.stPointTo.s16y = dstRect.u32Height - stLine.u16Width;
    MI_GFX_DrawLine(&dstSurf, &stLine, &fence);
    stLine.stPointFrom.s16x = dstRect.s32Xpos;
    stLine.stPointFrom.s16y = dstRect.u32Height;
    stLine.stPointTo.s16x = dstRect.s32Xpos;
    stLine.stPointTo.s16y = dstRect.s32Ypos;
    MI_GFX_DrawLine(&dstSurf, &stLine, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
}
//void __test_draw_OVAL(MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect,
//                      char *dstData, MI_BOOL bSinkSurf)
//{
//    MI_U16 fence = 0;
//    MI_GFX_Oval_t stOval;
//    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
//    memset(&stOval, 0, sizeof(MI_GFX_Oval_t));
//    clock_gettime(CLOCK_MONOTONIC, &ts1);
//    stOval.stDstRect.s32Xpos = dstRect.s32Xpos;
//    stOval.stDstRect.s32Ypos = dstRect.s32Ypos;
//    stOval.stDstRect.u32Width = dstRect.u32Width/2;
//    stOval.stDstRect.u32Height = dstRect.u32Height;
//    //stOval.stClipRect = ;
//    stOval.u16Width = 5;
//    stOval.u32Color = 0X80FF0000;
//    MI_GFX_DrawOval(&dstSurf, &stOval,&fence);
//
//    stOval.stDstRect.s32Xpos = dstRect.s32Xpos+dstRect.u32Width/2;
//    stOval.stDstRect.s32Ypos = dstRect.s32Ypos;
//    stOval.stDstRect.u32Width = min(dstRect.u32Width/2,dstRect.u32Height);
//    stOval.stDstRect.u32Height = min(dstRect.u32Width/2,dstRect.u32Height);
//    MI_GFX_DrawOval(&dstSurf, &stOval,&fence);
//
//    MI_GFX_WaitAllDone(FALSE, fence);
//    JDEC_PERF(ts1, ts2, 1);
//
//    if(bSinkSurf)
//        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);
//
//    memset(dstData, 0, dstSurf.u32Height * dstSurf.u32Stride);
//}
//

int main()
{
    MI_GFX_Surface_t dstSurf;
    MI_GFX_Rect_t dstRect;
    MI_S32 ret = 0;
    char *data = NULL;
    MI_SYS_Init();
    MI_GFX_Open();

    dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    dstSurf.phyAddr = 0;
    dstSurf.u32Height = 1080 > MAX_H ? MAX_H : 1080;
    dstSurf.u32Width = 1920 > MAX_W ? MAX_W : 1920 ;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos = 0;
    dstRect.u32Width = dstSurf.u32Width;
    dstRect.u32Height = dstSurf.u32Height;

    if((ret = _gfx_alloc_surface(&dstSurf, &data, "fillrect")) < 0)
    {
        return ret;
    }

    __test_fill_ARGB8888(dstSurf, dstRect, data, TRUE);
    __test_fill_ARGB1555(dstSurf, dstRect, data, TRUE);
    __test_fill_ARGB4444(dstSurf, dstRect, data, TRUE);
    __test_fill_RGB565(dstSurf, dstRect, data, TRUE);
    __test_fill_I8(dstSurf, dstRect, data, TRUE);
    __test_draw_LINE(dstSurf, dstRect, data, TRUE);
//    __test_draw_OVAL(dstSurf, dstRect, data, TRUE);

    _gfx_free_surface(&dstSurf, data);
    return 0;
}
