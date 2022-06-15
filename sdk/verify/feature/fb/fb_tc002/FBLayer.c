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
#include "FBLayer.h"
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "mstarFb.h"
#include "FBDefs.h"

#if HAS_GE
#include "mi_sys.h"
#include "mi_gfx_internal.h"
#endif

enum E_PAN_LOCATION
{
    EPL_UNKNOWN = 0,
    EPL_VERTICAL,
    EPL_HORIZONTAL,
};

static void dump_fb_fscreen_info(struct fb_fix_screeninfo finfo)
{
    TRACER("Fixed screen info:\n"
        "\tid: %s\n"
        "\tsmem_start: 0x%lx\n"
        "\tsmem_len: %d\n"
        "\ttype: %d\n"
        "\ttype_aux: %d\n"
        "\tvisual: %d\n"
        "\txpanstep: %d\n"
        "\typanstep: %d\n"
        "\tywrapstep: %d\n"
        "\tline_length: %d\n"
        "\tmmio_start: 0x%lx\n"
        "\tmmio_len: %d\n"
        "\taccel: %d\n"
        "\n",
        finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
        finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
        finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
        finfo.mmio_len, finfo.accel);
}

static void dump_fb_vscreen_info(struct fb_var_screeninfo vinfo)
{
    TRACER("Variable screen info:\n"
        "\txres: %d\n"
        "\tyres: %d\n"
        "\txres_virtual: %d\n"
        "\tyres_virtual: %d\n"
        "\txoffset: %d\n"
        "\tyoffset: %d\n"
        "\tbits_per_pixel: %d\n"
        "\tgrayscale: %d\n"
        "\tred: offset: %2d, length: %2d, msb_right: %2d\n"
        "\tgreen: offset: %2d, length: %2d, msb_right: %2d\n"
        "\tblue: offset: %2d, length: %2d, msb_right: %2d\n"
        "\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
        "\tnonstd: %d\n"
        "\tactivate: %d\n"
        "\theight: %d\n"
        "\twidth: %d\n"
        "\taccel_flags: 0x%x\n"
        "\tpixclock: %d\n"
        "\tleft_margin: %d\n"
        "\tright_margin: %d\n"
        "\tupper_margin: %d\n"
        "\tlower_margin: %d\n"
        "\thsync_len: %d\n"
        "\tvsync_len: %d\n"
        "\tsync: %d\n"
        "\tvmode: %d\n"
        "\n",
        vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
        vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
        vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
        vinfo.red.msb_right, vinfo.green.offset, vinfo.green.length,
        vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
        vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
        vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
        vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
        vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
        vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
        vinfo.sync, vinfo.vmode);
}

static FBColorFmt get_fb_color_format(struct fb_var_screeninfo vinfo)
{
    FBColorFmt fmt = E_FB_CLRFMT_INVALID;

    switch (vinfo.bits_per_pixel)
    {
    case 16:
    {
        if (vinfo.transp.length == 0)
            fmt = E_FB_CLRFMT_RGB565;
        else if (vinfo.transp.length == 1)
            fmt = E_FB_CLRFMT_ARGB1555;
        else if (vinfo.transp.length == 4)
            fmt = E_FB_CLRFMT_ARGB4444;
        break;
    }
    case 32:
        fmt = E_FB_CLRFMT_ARGB8888;
        break;
    default:
        break;
    }
    return fmt;
}

static inline uint32_t fb_argb8888_to_argb8888(uint32_t color)
{
    return color;
}

static inline uint32_t fb_argb8888_to_rgb565(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0x00ff0000) >> 19) << 11) |
        (((color & 0x0000ff00) >> 10) << 5) |
        (((color & 0x000000ff) >> 3))));
}

static inline uint32_t fb_argb8888_to_argb4444(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0xff000000) >> 28) << 12) |
        (((color & 0x00ff0000) >> 20) << 8) |
        (((color & 0x0000ff00) >> 12) << 4) |
        (((color & 0x000000ff) >> 4))));
}

static inline uint32_t fb_argb8888_to_argb1555(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0xff000000) >> 24) == 0 ? 0x0000 : 0x8000) |
        (((color & 0x00ff0000) >> 19) << 10) |
        (((color & 0x0000ff00) >> 11) << 5) |
        (((color & 0x000000ff) >> 5))));
}

static inline uint32_t fb_argb1555_to_argb8888(uint32_t color)
{
    return (((color & 0x8000) == 0 ? 0x00 : 0xff000000) |
        (((color & 0x7c00) >> 10) << 19) |
        (((color & 0x03e0) >> 5) << 11) |
        (((color & 0x001f) << 3)));
}

static inline uint32_t fb_argb1555_to_rgb565(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0x7c00) >> 10) << 11) |
        (((color & 0x03e0) >> 5) << 6) |
        (((color & 0x001f)))));
}

static inline uint32_t fb_argb1555_to_argb4444(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0x8000) == 0) ? 0x00 : 0xf000) |
        (((color & 0x7c00) >> 11) << 8) |
        (((color & 0x03e0) >> 6) << 4) |
        (((color & 0x001f) >> 1))));
}

static inline uint32_t fb_argb1555_to_argb1555(uint32_t color)
{
    return color;
}

static inline uint32_t fb_rgb565_to_argb8888(uint32_t color)
{
    return (0xffffffff & (
        (((color & 0xf800) >> 11) << 19) |
        (((color & 0x07e0) >> 5) << 10) |
        (((color & 0x001f) << 3))));
}

static inline uint32_t fb_rgb565_to_argb1555(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0xf800) >> 11) << 10) |
        (((color & 0x07e0) >> 6) << 5) |
        (((color & 0x001f)))));
}

static inline uint32_t fb_rgb565_to_argb4444(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0xf800) >> 12) << 8) |
        (((color & 0x07e0) >> 7) << 4) |
        (((color & 0x001f) >> 1))));
}

static inline uint32_t fb_rgb565_to_rgb565(uint32_t color)
{
    return color;
}

static inline uint32_t fb_argb4444_to_argb8888(uint32_t color)
{
    return (0xffffffff & (
        (((color & 0xf000) >> 12) << 28) |
        (((color & 0x0f00) >> 8) << 20) |
        (((color & 0x00f0) >> 4) << 12) |
        (((color & 0x000f) << 4))));
}

static inline uint32_t fb_argb4444_to_argb1555(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0xf000) == 0 ? 0x00 : 0x8000)) |
        (((color & 0x0f00) >> 8) << 11) |
        (((color & 0x00f0) >> 4) << 6) |
        (((color & 0x000f) << 1))));
}

static inline uint32_t fb_argb4444_to_rgb565(uint32_t color)
{
    return (0x0000ffff & (
        (((color & 0x0f00) >> 8) << 12) |
        (((color & 0x00f0) >> 4) << 7) |
        (((color & 0x000f) << 1))));
}

static inline uint32_t fb_argb4444_to_argb4444(uint32_t color)
{
    return color;
}

static void fb_convert_argb32_to_rgb16(uint16_t* dst, FBColorFmt fmt, uint32_t src)
{
    switch (fmt)
    {
    case E_FB_CLRFMT_RGB565:
        *dst = fb_argb8888_to_rgb565(src);
        break;
    case E_FB_CLRFMT_ARGB4444:
        *dst = fb_argb8888_to_argb4444(src);
        break;
    case E_FB_CLRFMT_ARGB1555:
        *dst = fb_argb8888_to_argb1555(src);
        break;
    default:
        *dst = 0;
        break;
    }
}

static void fb_fill_rect_u16(void* buff, int width,
    int height, int stride, uint16_t color)
{
    int x, y;
    uint16_t *dst = (uint16_t*)buff;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dst[x] = color;
        }
        dst += stride;
    }
}

static void fb_fill_rect_u32(void* buff, int width,
    int height, int stride, uint32_t color)
{
    int x, y;
    uint32_t* dst = (uint32_t*)buff;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dst[x] = color;
        }
        dst += stride;
    }
}

static void fb_draw_line_u16(void* buff, int stride,
    int x0, int y0, int x1, int y1, uint32_t color)
{
    uint16_t* dst = (uint16_t*)buff;
    if (x0 == x1 && y0 == y1) // dot
    {
        *(dst + y0 * stride + x0) = (uint16_t)color;
    }
    else if (x0 == x1) // vertical
    {
        int x = x0, y = MIN(y0, y1);
        int h = MAX(y0, y1);
        for (; y <= h; ++y)
        {
            *(dst + y * stride + x) = (uint16_t)color;
        }
    }
    else if (y0 == y1) // horizontal
    {
        int x = MIN(x0, x1), y = y0;
        int w = MAX(x0, x1);
        for (; x <= w; ++x)
        {
            *(dst + y * stride + x) = (uint16_t)color;
        }
    }
    else
    {
        float xx1 = x0, yy1 = y0;
        float xx2 = x1, yy2 = y1;

        float ratio = (yy2 - yy1) / (xx2 - xx1);
        float b = yy1 - (ratio * xx1);
        if (abs(ratio) <= 1) // tend to x-axis
        {
            int x = MIN(x0, x1), y = 0;
            int w = MAX(x0, x1);

            for (; x <= w; ++x)
            {
                y = b + ((float)x * ratio);
                *(dst + y * stride + x) = (uint16_t)color;
            }
        }
        else // tend to y-axis
        {
            int x = 0, y = MIN(y0, y1);
            int h = MAX(y0, y1);

            for (; y <= h; ++y)
            {
                x = b + ((float)y / ratio);
                *(dst + y * stride + x) = (uint16_t)color;
            }
        }
    }
}

static void fb_draw_line_u32(void* buff, int stride,
    int x0, int y0, int x1, int y1, uint32_t color)
{
    uint32_t* dst = (uint32_t*)buff;
    if (x0 == x1 && y0 == y1) // dot
    {
        *(dst + y0 * stride + x0) = (uint32_t)color;
    }
    else if (x0 == x1) // vertical
    {
        int x = x0, y = MIN(y0, y1);
        int h = MAX(y0, y1);
        for (; y <= h; ++y)
        {
            *(dst + y * stride + x) = (uint32_t)color;
        }
    }
    else if (y0 == y1) // horizontal
    {
        int x = MIN(x0, x1), y = y0;
        int w = MAX(x0, x1);
        for (; x <= w; ++x)
        {
            *(dst + y * stride + x) = (uint32_t)color;
        }
    }
    else
    {
        float xx1 = x0, yy1 = y0;
        float xx2 = x1, yy2 = y1;

        float ratio = (yy2 - yy1) / (xx2 - xx1);
        float b = yy1 - (ratio * xx1);
        if (abs(ratio) <= 1) // tend to x-axis
        {
            int x = MIN(x0, x1), y = 0;
            int w = MAX(x0, x1);

            for (; x <= w; ++x)
            {
                y = b + ((float)x * ratio);
                *(dst + y * stride + x) = (uint32_t)color;
            }
        }
        else // tend to y-axis
        {
            int x = 0, y = MIN(y0, y1);
            int h = MAX(y0, y1);

            for (; y <= h; ++y)
            {
                x = b + ((float)y / ratio);
                *(dst + y * stride + x) = (uint32_t)color;
            }
        }
    }
}

typedef  uint32_t(*fb_convert_func_t)(uint32_t);
static fb_convert_func_t fb_select_convert_func(int16_t src, int16_t dst)
{
    switch (src)
    {
    case E_FB_CLRFMT_RGB565:
        switch (dst)
        {
        case E_FB_CLRFMT_RGB565: return fb_rgb565_to_rgb565;
        case E_FB_CLRFMT_ARGB4444: return fb_rgb565_to_argb4444;
        case E_FB_CLRFMT_ARGB8888: return fb_rgb565_to_argb8888;
        case E_FB_CLRFMT_ARGB1555: return fb_rgb565_to_argb1555;
        }
        break;
    case E_FB_CLRFMT_ARGB4444:
        switch (dst)
        {
        case E_FB_CLRFMT_RGB565: return fb_argb4444_to_rgb565;
        case E_FB_CLRFMT_ARGB4444: return fb_argb4444_to_argb4444;
        case E_FB_CLRFMT_ARGB8888: return fb_argb4444_to_argb8888;
        case E_FB_CLRFMT_ARGB1555: return fb_argb4444_to_argb1555;
        }
        break;
    case E_FB_CLRFMT_ARGB8888:
        switch (dst)
        {
        case E_FB_CLRFMT_RGB565: return fb_argb8888_to_rgb565;
        case E_FB_CLRFMT_ARGB4444: return fb_argb8888_to_argb4444;
        case E_FB_CLRFMT_ARGB8888: return fb_argb8888_to_argb8888;
        case E_FB_CLRFMT_ARGB1555: return fb_argb8888_to_argb1555;
        }
        break;
        case E_FB_CLRFMT_ARGB1555:
        switch (dst)
        {
        case E_FB_CLRFMT_RGB565: return fb_argb1555_to_rgb565;
        case E_FB_CLRFMT_ARGB4444: return fb_argb1555_to_argb4444;
        case E_FB_CLRFMT_ARGB8888: return fb_argb1555_to_argb8888;
        case E_FB_CLRFMT_ARGB1555: return fb_argb1555_to_argb1555;
        }
        break;
    }
    return NULL;
}

static void fb_strech_rect_u16Src_u16Dst(
    void* buff0, void* buff1,
    int cx0, int cy0, int cx1, int cy1,
    int stride0, int stride1,
    fb_convert_func_t convert_func)
{
    uint16_t* dst = (uint16_t*)buff0;
    uint16_t* src = (uint16_t*)buff1;

    float x_step = cx0 == cx1 ? 0 : ((float)cx1) / ((float)(cx0 - cx1));
    float y_step = cy0 == cy1 ? 0 : ((float)cy1) / ((float)(cy0 - cy1));
    int x = 0, y = 0;
    int x_nail = 0;
    int y_nail = 0;
    int x_op = x_step < 0 ? -1 : 1;
    int y_op = y_step < 0 ? -1 : 1;
    for (y = 0; y < cy1 && y + y_nail < cy0; ++y)
    {
        x_nail = 0;
        for (x = 0; x < cx1 && x + x_nail < cx0; ++x)
        {
            if (x_step == 0)
                dst[x] = (uint16_t)convert_func((uint32_t)src[x]);
            else
            {
                uint16_t pixel = (uint16_t)convert_func((uint32_t)src[x]);
                dst[x + x_nail] = pixel;
                while (x + 1 == (int)ceil(x_step * (x_nail + x_op)))
                {
                    if (x_step > 0)
                    {
                        x_nail++;
                        dst[x + x_nail] = pixel;
                     }
                    else
                        x_nail--;
                }
            }
        }
        if (y_step == 0)
        {
            dst += stride0;
            src += stride1;
        }
        else if (y_step > 0)
        {
            dst += stride0;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                src += stride1;
            else
            {
                y--;
                y_nail++;
            }
        }
        else
        {
            src += stride1;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                dst += stride0;
            else
                y_nail--;
        }
    }
}

static void fb_strech_rect_u32Src_u16Dst(
    void* buff0, void* buff1,
    int cx0, int cy0, int cx1, int cy1,
    int stride0, int stride1,
    fb_convert_func_t convert_func)
{
    uint16_t* dst = (uint16_t*)buff0;
    uint32_t* src = (uint32_t*)buff1;

    float x_step = cx0 == cx1 ? 0 : ((float)cx1) / ((float)(cx0 - cx1));
    float y_step = cy0 == cy1 ? 0 : ((float)cy1) / ((float)(cy0 - cy1));
    int x = 0, y = 0;
    int x_nail = 0;
    int y_nail = 0;
    int x_op = x_step < 0 ? -1 : 1;
    int y_op = y_step < 0 ? -1 : 1;
    for (y = 0; y < cy1 && y + y_nail < cy0; ++y)
    {
        x_nail = 0;
        for (x = 0; x < cx1 && x + x_nail < cx0; ++x)
        {
            if (x_step == 0)
                dst[x] = (uint16_t)convert_func((uint32_t)src[x]);
            else
            {
                uint16_t pixel = (uint16_t)convert_func((uint32_t)src[x]);
                dst[x + x_nail] = pixel;
                while (x + 1 == (int)ceil(x_step * (x_nail + x_op)))
                {
                    if (x_step > 0)
                    {
                        x_nail++;
                        dst[x + x_nail] = pixel;
                    }
                    else
                        x_nail--;
                }
            }
        }
        if (y_step == 0)
        {
            dst += stride0;
            src += stride1;
        }
        else if (y_step > 0)
        {
            dst += stride0;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                src += stride1;
            else
            {
                y--;
                y_nail++;
            }
        }
        else
        {
            src += stride1;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                dst += stride0;
            else
                y_nail--;
        }
    }
}

static void fb_strech_rect_u16Src_u32Dst(
    void* buff0, void* buff1,
    int cx0, int cy0, int cx1, int cy1,
    int stride0, int stride1,
    fb_convert_func_t convert_func)
{
    uint32_t* dst = (uint32_t*)buff0;
    uint16_t* src = (uint16_t*)buff1;

    float x_step = cx0 == cx1 ? 0 : ((float)cx1) / ((float)(cx0 - cx1));
    float y_step = cy0 == cy1 ? 0 : ((float)cy1) / ((float)(cy0 - cy1));
    int x = 0, y = 0;
    int x_nail = 0;
    int y_nail = 0;
    int x_op = x_step < 0 ? -1 : 1;
    int y_op = y_step < 0 ? -1 : 1;
    for (y = 0; y < cy1 && y + y_nail < cy0; ++y)
    {
        x_nail = 0;
        for (x = 0; x < cx1 && x + x_nail < cx0; ++x)
        {
            if (x_step == 0)
                dst[x] = (uint32_t)convert_func((uint32_t)src[x]);
            else
            {
                uint32_t pixel = (uint32_t)convert_func((uint32_t)src[x]);
                dst[x + x_nail] = pixel;
                while (x + 1 == (int)ceil(x_step * (x_nail + x_op)))
                {
                    if (x_step > 0)
                    {
                        x_nail++;
                        dst[x + x_nail] = pixel;
                    }
                    else
                        x_nail--;
                }
            }
        }
        if (y_step == 0)
        {
            dst += stride0;
            src += stride1;
        }
        else if (y_step > 0)
        {
            dst += stride0;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                src += stride1;
            else
            {
                y--;
                y_nail++;
            }
        }
        else
        {
            src += stride1;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                dst += stride0;
            else
                y_nail--;
        }
    }
}

static void fb_strech_rect_u32Src_u32Dst(
    void* buff0, void* buff1,
    int cx0, int cy0, int cx1, int cy1,
    int stride0, int stride1,
    fb_convert_func_t convert_func)
{
    uint32_t* dst = (uint32_t*)buff0;
    uint32_t* src = (uint32_t*)buff1;

    float x_step = cx0 == cx1 ? 0 : ((float)cx1) / ((float)(cx0 - cx1));
    float y_step = cy0 == cy1 ? 0 : ((float)cy1) / ((float)(cy0 - cy1));
    int x = 0, y = 0;
    int x_nail = 0;
    int y_nail = 0;
    int x_op = x_step < 0 ? -1 : 1;
    int y_op = y_step < 0 ? -1 : 1;
    for (y = 0; y < cy1 && y + y_nail < cy0; ++y)
    {
        x_nail = 0;
        for (x = 0; x < cx1 && x + x_nail < cx0; ++x)
        {
            if (x_step == 0)
                dst[x] = (uint32_t)convert_func((uint32_t)src[x]);
            else
            {
                uint32_t pixel = (uint32_t)convert_func((uint32_t)src[x]);
                dst[x + x_nail] = pixel;
                while (x + 1 == (int)ceil(x_step * (x_nail + x_op)))
                {
                    if (x_step > 0)
                    {
                        x_nail++;
                        dst[x + x_nail] = pixel;
                    }
                    else
                        x_nail--;
                }
            }
        }
        if (y_step == 0)
        {
            dst += stride0;
            src += stride1;
        }
        else if (y_step > 0)
        {
            dst += stride0;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                src += stride1;
            else
            {
                y--;
                y_nail++;
            }
        }
        else
        {
            src += stride1;
            if (y + 1 != (int)ceil(y_step * (y_nail + y_op)))
                dst += stride0;
            else
               y_nail--;
        }
    }
}

#if HAS_GE
static MI_GFX_ColorFmt_e fb_select_gfx_fmt(FBColorFmt fmt)
{
    switch (fmt)
    {
    case E_FB_CLRFMT_RGB565: return E_MI_GFX_FMT_RGB565;
    case E_FB_CLRFMT_ARGB4444: return E_MI_GFX_FMT_ARGB4444;
    case E_FB_CLRFMT_ARGB8888: return E_MI_GFX_FMT_ARGB8888;
    case E_FB_CLRFMT_ARGB1555: return E_MI_GFX_FMT_ARGB1555;
    case E_FB_CLRFMT_YUV422: return E_MI_GFX_FMT_YUV422;
    case E_FB_CLRFMT_INVALID: break;
    default: break;
    }
    return ((MI_GFX_ColorFmt_e)0);
}
#endif

int initFBLayerInfo(uint8_t nId, FBLayerInfo_t* pFbLayerInfo)
{
    char devfile[20] = { 0 };
    snprintf(devfile, sizeof(devfile), "/dev/fb%d", nId);
    pFbLayerInfo->fb_device_fd = open(devfile, O_RDWR);
    ASSERT_THROW(pFbLayerInfo->fb_device_fd !=-1, E_FB_NODEV);

    ASSERT_THROW(ioctl(pFbLayerInfo->fb_device_fd,
        FBIOGET_FSCREENINFO, &pFbLayerInfo->fb_finfo) !=-1, E_FB_INVAL);
    ASSERT_THROW(ioctl(pFbLayerInfo->fb_device_fd,
        FBIOGET_VSCREENINFO, &pFbLayerInfo->fb_vinfo) !=-1, E_FB_INVAL);

    pFbLayerInfo->fb_vinfo.xoffset = pFbLayerInfo->fb_vinfo.yoffset = 0;
    dump_fb_fscreen_info(pFbLayerInfo->fb_finfo);
    dump_fb_vscreen_info(pFbLayerInfo->fb_vinfo);
    pFbLayerInfo->p_fb_screenMmapAddr = (char*)mmap(0, pFbLayerInfo->fb_finfo.smem_len,
        PROT_READ | PROT_WRITE, MAP_SHARED, pFbLayerInfo->fb_device_fd, 0);
    ASSERT_THROW(pFbLayerInfo->p_fb_screenMmapAddr != MAP_FAILED, E_FB_NOMEM);

    pFbLayerInfo->e_fb_clr_fmt = get_fb_color_format(pFbLayerInfo->fb_vinfo);
    pFbLayerInfo->n_fb_bytePerPixel =
        (pFbLayerInfo->e_fb_clr_fmt == E_FB_CLRFMT_ARGB8888) ? 4 : 2;

    //default set double buffer
    pFbLayerInfo->n_fb_screen_count = 2;
    pFbLayerInfo->n_fb_front_screen = 0;
    pFbLayerInfo->n_fb_back_screen  = 1;
    pFbLayerInfo->n_fb_layerId = nId;
    pFbLayerInfo->p_fb_screen[0] = (char*)(pFbLayerInfo->p_fb_screenMmapAddr) +
        (pFbLayerInfo->fb_vinfo.yoffset) * pFbLayerInfo->fb_finfo.line_length +
        (pFbLayerInfo->fb_vinfo.xoffset) *  pFbLayerInfo->n_fb_bytePerPixel;
    //Physical addr used by GE
    pFbLayerInfo->p_fb_screenPhyAddr[0] = (char*)(pFbLayerInfo->fb_finfo.smem_start);
#if HAS_PD
    if (pFbLayerInfo->fb_vinfo.yres * 2 <= pFbLayerInfo->fb_vinfo.yres_virtual)
    {
        //Vertical double buffer
        pFbLayerInfo->p_fb_screen[1] = (char*)(pFbLayerInfo->p_fb_screenMmapAddr) +
            (pFbLayerInfo->fb_vinfo.yoffset + pFbLayerInfo->fb_vinfo.yres) * pFbLayerInfo->fb_finfo.line_length +
            (pFbLayerInfo->fb_vinfo.xoffset) * pFbLayerInfo->n_fb_bytePerPixel;
        pFbLayerInfo->p_fb_screenPhyAddr[1] = (char*)(pFbLayerInfo->fb_finfo.smem_start) +
            (pFbLayerInfo->fb_vinfo.yoffset + pFbLayerInfo->fb_vinfo.yres) * pFbLayerInfo->fb_finfo.line_length +
            (pFbLayerInfo->fb_vinfo.xoffset) * pFbLayerInfo->n_fb_bytePerPixel;
        TRACER("[ FB user module ] epl vertical\n");
    }
    else //single buffer
#endif
    {
        pFbLayerInfo->p_fb_screen[1] = pFbLayerInfo->p_fb_screen[0];
        pFbLayerInfo->p_fb_screenPhyAddr[1] = pFbLayerInfo->p_fb_screenPhyAddr[0];
        pFbLayerInfo->n_fb_screen_count = 1;
        pFbLayerInfo->n_fb_back_screen = 0;
        TRACER("[ FB user module ] signal buffer\n");
    }
    TRACER("[_FB_ScreenPhyAddr] p_fb_screenPhyAddr[0]=%p,p_fb_screenPhyAddr[1]=%p\n",
        pFbLayerInfo->p_fb_screenPhyAddr[0],pFbLayerInfo->p_fb_screenPhyAddr[1]);
    TRACER("[_FB_p_fb_screen] pFbLayerInfo->p_fb_screen[0]=%p,pFbLayerInfo->p_fb_screen[1]=%p\n",
        pFbLayerInfo->p_fb_screen[0],pFbLayerInfo->p_fb_screen[1]);
#if HAS_GE
    MI_GFX_Open();
#endif
    pFbLayerInfo->b_fb_ready_flag = true;
    pFbLayerInfo->n_fb_layerId = nId;
    return 0;
}

int beginDrawOnFBLayer(bool bErase, bool waitForReady, FBLayerInfo_t* pFbLayerInfo)
{
    int ret = -1;
    int y = 0;
    int frontbufferSeq = pFbLayerInfo->n_fb_front_screen;
    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
    char* frontbufferVaddr = pFbLayerInfo->p_fb_screen[frontbufferSeq];
    char* backbufferVaddr = pFbLayerInfo->p_fb_screen[backbufferSeq];
#if HAS_GE
    MI_U16 fence = 0;
    MI_GFX_Surface_t dst;
    MI_GFX_Surface_t src;
    MI_GFX_Rect_t dstRect;
    MI_GFX_Rect_t srcRect;
    memset(&src, 0, sizeof(MI_GFX_Surface_t));
    memset(&dst, 0, sizeof(MI_GFX_Surface_t));
    memset(&dstRect, 0, sizeof(MI_GFX_Rect_t));
    memset(&srcRect, 0, sizeof(MI_GFX_Rect_t));
 #endif
    if (!waitForReady)
        CHECK_IFNOT(pFbLayerInfo->b_fb_ready_flag, true, -1)
    pFbLayerInfo->b_fb_ready_flag = false;
    if (!bErase) {
        if (pFbLayerInfo->n_fb_screen_count > 1)
        {
#if HAS_GE
            dst.phyAddr = (MI_U32)(pFbLayerInfo->p_fb_screenPhyAddr[backbufferSeq]);
            dst.eColorFmt = fb_select_gfx_fmt(pFbLayerInfo->e_fb_clr_fmt);
            dst.u32Width = pFbLayerInfo->fb_vinfo.xres;
            dst.u32Height = pFbLayerInfo->fb_vinfo.yres;
            dst.u32Stride = pFbLayerInfo->fb_finfo.line_length;

            src.phyAddr = (MI_U32)(pFbLayerInfo->p_fb_screenPhyAddr[frontbufferSeq]);
            src.eColorFmt = dst.eColorFmt;
            src.u32Width =  dst.u32Width;
            src.u32Height = dst.u32Height;
            src.u32Stride = dst.u32Stride;
            if (0 == MI_GFX_BitBlit(&src, &srcRect, &dst, &dstRect, NULL, &fence))
                ret = (0 == MI_GFX_WaitAllDone(FALSE, fence) ? 0 : -1);
            if (ret != 0) {
                pFbLayerInfo->b_fb_ready_flag = true;
                return -1;
            }
#else
            for (; y < pFbLayerInfo->fb_vinfo.yres; y++)
            {
                memcpy(backbufferVaddr + y * pFbLayerInfo->fb_finfo.line_length,
                    frontbufferVaddr + y * pFbLayerInfo->fb_finfo.line_length,
                    pFbLayerInfo->fb_vinfo.xres * pFbLayerInfo->n_fb_bytePerPixel);
            }
#endif
            }
        }
        else {
            for(y = 0; y < pFbLayerInfo->fb_vinfo.yres; y++)
            {
                memset(backbufferVaddr + y * pFbLayerInfo->fb_finfo.line_length,
                    0,pFbLayerInfo->fb_vinfo.xres * pFbLayerInfo->n_fb_bytePerPixel);
            }
        }
        TRACER("[ FB user module ] %s\n", __FUNCTION__);
        return 0;
 }
int flipOperationOnFBLayer(FBLayerInfo_t* pFbLayerInfo)
{
    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
    struct fb_var_screeninfo vinfo = pFbLayerInfo->fb_vinfo;
#if HAS_GE
    MI_GFX_WaitAllDone(TRUE, 0);
#endif
#if HAS_PD
    if (pFbLayerInfo->n_fb_screen_count == 2)
    {
        vinfo.yoffset = (backbufferSeq==1) ? (pFbLayerInfo->fb_vinfo.yoffset + pFbLayerInfo->fb_vinfo.yres) : pFbLayerInfo->fb_vinfo.yoffset;
        TRACER("[ FB user module ] %s xoffset %d yoffset %d\n", __FUNCTION__, vinfo.xoffset, vinfo.yoffset);
    }
#endif
    if (0 != ioctl(pFbLayerInfo->fb_device_fd, FBIOPAN_DISPLAY, &vinfo))
    {
        TERROR("[ FB user module ] pan display failed");
        return -1;
    }
    return 0;
}

int endDrawOnFBLayer(FBLayerInfo_t* pFbLayerInfo)
{
    pFbLayerInfo->n_fb_back_screen = pFbLayerInfo->n_fb_front_screen;
    pFbLayerInfo->n_fb_front_screen = (pFbLayerInfo->n_fb_front_screen+1) % (pFbLayerInfo->n_fb_screen_count);
    pFbLayerInfo->b_fb_ready_flag  = true;
    TRACER("[ FB user module ] %s\n", __FUNCTION__);
    return 0;
}

int strechRectOnFBLayer(FBRect* pDstRect, FBRect* pSrcRect,
    FBImage* oImage, FBLayerInfo_t* pFbLayerInfo)
{
    CHECK_IF(pFbLayerInfo->e_fb_clr_fmt, E_FB_CLRFMT_INVALID, -1)
    CHECK_IF(oImage, NULL, -1)
    CHECK_IF(oImage->f, E_FB_CLRFMT_INVALID, -1)

    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
    const int nBytsPerPixelSrc =
        oImage->f == E_FB_CLRFMT_ARGB8888 ? 4 : 2;
    int x0 = pDstRect ? pDstRect->l : 0; //dst x offset
    int y0 = pDstRect ? pDstRect->t : 0; //dst y offset
    uint32_t width = pDstRect ? pDstRect->w : pFbLayerInfo->fb_vinfo.xres; //dst width
    uint32_t height = pDstRect ? pDstRect->h : pFbLayerInfo->fb_vinfo.yres; //dst height

    int x = pSrcRect ? pSrcRect->l: 0; //src  x offset
    int y = pSrcRect ? pSrcRect->t : 0; //src y offst
    uint32_t cx = pSrcRect ? pSrcRect->w : oImage->w; //src width
    uint32_t cy = pSrcRect ? pSrcRect->h : oImage->h; //src height
    const int nPitch = pFbLayerInfo->fb_finfo.line_length / pFbLayerInfo->n_fb_bytePerPixel;
    char* dst = pFbLayerInfo->p_fb_screen[backbufferSeq] +
        y0 * pFbLayerInfo->fb_finfo.line_length +
        x0 * pFbLayerInfo->n_fb_bytePerPixel;

    int nPitchSrc = oImage->w;
    char* src = (char*)(oImage->p) +
        y * oImage->w * nBytsPerPixelSrc +
		x * nBytsPerPixelSrc;
    fb_convert_func_t convert_func = fb_select_convert_func(oImage->f, pFbLayerInfo->e_fb_clr_fmt);
    if (pFbLayerInfo->n_fb_bytePerPixel == 2)
    {
        if (nBytsPerPixelSrc ==2 )
            fb_strech_rect_u16Src_u16Dst(dst, src, width, height, cx, cy, nPitch,nPitchSrc,convert_func);
        else
            fb_strech_rect_u32Src_u16Dst(dst, src, width, height, cx, cy, nPitch,nPitchSrc,convert_func);
    }
    else
    {
        if (nBytsPerPixelSrc ==2 )
            fb_strech_rect_u16Src_u32Dst(dst, src, width, height, cx, cy, nPitch,nPitchSrc,convert_func);
         else
            fb_strech_rect_u32Src_u32Dst(dst, src, width, height, cx, cy, nPitch,nPitchSrc,convert_func);
    }
    return 0;
}

int fillRectOnFBLayer(FBRect * pDstRect,
    FBCOLOR nColor, FBLayerInfo_t * pFbLayerInfo)
{
    CHECK_IF(pFbLayerInfo->e_fb_clr_fmt, E_FB_CLRFMT_INVALID, -1)

    int x0 = pDstRect ? pDstRect->l : 0;
	int y0 = pDstRect ? pDstRect->t : 0;
	uint32_t width = pDstRect ? pDstRect->w : pFbLayerInfo->fb_vinfo.xres;
	uint32_t height = pDstRect ? pDstRect->h : pFbLayerInfo->fb_vinfo.yres;
    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
 #if HAS_GE
    MI_U16 fence = 0;
    MI_GFX_Surface_t dst;
    MI_GFX_Rect_t dstRect = {x0, y0, width, height};
    memset(&dst, 0, sizeof(MI_GFX_Surface_t));
    dst.phyAddr = (MI_U32)(pFbLayerInfo->p_fb_screenPhyAddr[backbufferSeq]);
    dst.eColorFmt = fb_select_gfx_fmt(pFbLayerInfo->e_fb_clr_fmt);
    dst.u32Width = pFbLayerInfo->fb_vinfo.xres;
    dst.u32Height = pFbLayerInfo->fb_vinfo.yres;
    dst.u32Stride = pFbLayerInfo->fb_finfo.line_length;
    if (0 == MI_GFX_QuickFill(&dst, &dstRect, nColor, &fence))
        return 0 == MI_GFX_WaitAllDone(FALSE, fence) ? 0 : -1;
    else
        return -1;
 #else
    uint32_t color32 = nColor;
    uint16_t color16 = 0;
    char* pDst = pFbLayerInfo->p_fb_screen[backbufferSeq] +
        y0 * pFbLayerInfo->fb_finfo.line_length + x0 * pFbLayerInfo->n_fb_bytePerPixel;
    int nPitch =  pFbLayerInfo->fb_finfo.line_length / pFbLayerInfo->n_fb_bytePerPixel;
    pFbLayerInfo->n_fb_bytePerPixel == 2 ?
        fb_convert_argb32_to_rgb16(&color16, pFbLayerInfo->e_fb_clr_fmt, nColor),
        fb_fill_rect_u16(pDst, width, height, nPitch, color16):
        fb_fill_rect_u32(pDst, width, height, nPitch, color32);
 #endif
    return 0;
}

int drawLineOnFBLayer(FBPoint* oPoint1, FBPoint* oPoint2,
    FBCOLOR nColor, int nWidth, FBLayerInfo_t* pFbLayerInfo)
{
    CHECK_IF(pFbLayerInfo->e_fb_clr_fmt, E_FB_CLRFMT_INVALID, -1)

    const int nPitch = pFbLayerInfo->fb_finfo.line_length / pFbLayerInfo->n_fb_bytePerPixel;
    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
    char* pDst = pFbLayerInfo->p_fb_screen[backbufferSeq];

    uint16_t color16 = 0;
    pFbLayerInfo->n_fb_bytePerPixel == 2 ?
        fb_convert_argb32_to_rgb16(&color16, pFbLayerInfo->e_fb_clr_fmt, nColor),
        fb_draw_line_u16(pDst, nPitch, oPoint1->x, oPoint1->y, oPoint2->x, oPoint2->y, color16):
        fb_draw_line_u32(pDst, nPitch, oPoint1->x, oPoint1->y, oPoint2->x, oPoint2->y, color16);
    return 0;
}

int drawLinesOnFBLayer(FBPoint* pPoints, size_t nCount,
    FBCOLOR nColor, int nWidth, FBLayerInfo_t* pFbLayerInfo)
{
    CHECK_IF(pPoints, NULL, -1);
    int i = 0;
    for (i = 1; i < nCount; ++i)
    {
        drawLineOnFBLayer(&pPoints[i - 1], &pPoints[i], nColor, nWidth, pFbLayerInfo);
    }
    return 0;
}

int stretchBlitImgByGE(const char* fileName,
    int srcWidth, int srcHeight,int srcStride,
    FBColorFmt srcFmt, FBLayerInfo_t* pFbLayerInfo)
{
#if HAS_GE
    MI_PHY phyAddr_src = 0x0;
    MI_U32 virAddr_src = 0x0;
    int bytePerPixel = (srcFmt == E_FB_CLRFMT_ARGB8888) ? 4 : 2;
    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    MI_U16 u16TargetFence = 0;
    MI_S32 ret = 0;
    FILE* fp = NULL;
    int backbufferSeq = pFbLayerInfo->n_fb_back_screen;
    TRACER("[ stretchBlitImgByGE ] %s srcHeight=%d,srcStride=%d\n", __FUNCTION__,srcHeight,srcStride);
    ret = MI_SYS_MMA_Alloc("mma_heap_name0", srcHeight * srcStride, &phyAddr_src);
    if (ret != MI_SUCCESS) {
        TERROR("[ FB user module ] MI_SYS_MMA_Alloc failed");
        ret = -1;
        goto cleanup;
    }
    TRACER("[ stretchBlitImgByGE ] %s MI_SYS_MMA_Alloc success phyAddr_src=0x%lx", __FUNCTION__,phyAddr_src);
    ret = MI_SYS_Mmap(phyAddr_src, srcHeight * srcStride, (void**)&virAddr_src, FALSE);
    if (ret != MI_SUCCESS) {
        TERROR("[ FB user module ] MI_SYS_Mmap failed");
        ret = -1;
        goto cleanup;
    }
    memset((void*)virAddr_src, 0x0,  srcHeight * srcStride);
    //fill src with image data
    fp = fopen(fileName, "rb");
    if (fp == NULL) {
        TRACER("Can not open file:%s please check it!!\n",fileName);
        ret = -1;
        goto cleanup;
    }
    fread((void*)virAddr_src, 1, srcHeight * srcStride, fp);
    fclose(fp);
    //GE blit source factor
    stSrc.eColorFmt = fb_select_gfx_fmt(srcFmt);
    stSrc.u32Width = srcWidth;
    stSrc.u32Height = srcHeight;
    stSrc.u32Stride = srcStride;
    stSrc.phyAddr = phyAddr_src;

    stSrcRect.s32Xpos = 0;
    stSrcRect.s32Ypos = 0;
    stSrcRect.u32Width = srcWidth;
    stSrcRect.u32Height = srcHeight;

    stDst.eColorFmt = fb_select_gfx_fmt(pFbLayerInfo->e_fb_clr_fmt);
    stDst.u32Width = pFbLayerInfo->fb_vinfo.xres;
    stDst.u32Height = pFbLayerInfo->fb_vinfo.yres;
    stDst.u32Stride = pFbLayerInfo->fb_finfo.line_length;
    stDst.phyAddr = (MI_U32) (pFbLayerInfo->p_fb_screenPhyAddr[backbufferSeq]);

    stDstRect.s32Xpos = 0;
    stDstRect.s32Ypos = 0;
    stDstRect.u32Width = srcWidth;
    stDstRect.u32Height = srcHeight;
    TRACER("StrechBlitImgByGE BlitFactor: srcRect[%d,%d,%d,%d],dstRect[%d,%d,%d,%d]\n",
        stSrcRect.s32Xpos,stSrcRect.s32Ypos,stSrcRect.u32Width,stSrcRect.u32Height,stDstRect.s32Xpos,stDstRect.s32Ypos,stDstRect.u32Width,stDstRect.u32Height);
    ret = MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, NULL, &u16TargetFence);
    if (ret != MI_SUCCESS) {
        TERROR("[ FB user module ]  StrechBlitImgByGE MI_GFX_WaitAllDone failed");
        ret = -1;
        goto cleanup;
    }
cleanup:
    if (virAddr_src!=0x0) {
        MI_SYS_Munmap((void*)virAddr_src, srcHeight * srcStride);
    }
    if (phyAddr_src!=0x0) {
        MI_SYS_MMA_Free(phyAddr_src);
    }
    return ret;
#else
    return 0;
#endif
}
int ioctlOnFBLayer(FBMetric nCmd, void *pArgs, FBLayerInfo_t* pFbLayerInfo)
{
    CHECK_IF(pArgs, NULL, -1);
    int fbcmd = 0;
    switch (nCmd)
    {
        case E_FBM_GET_LOCATION: fbcmd = FBIOGET_SCREEN_LOCATION; break;
        case E_FBM_SET_LOCATION: fbcmd = FBIOSET_SCREEN_LOCATION; break;
        case E_FBM_GET_SHOWN: fbcmd = FBIOGET_SHOW; break;
        case E_FBM_SET_SHOWN: fbcmd = FBIOSET_SHOW; break;
        case E_FBM_GET_GLOBAL_ALPHA: fbcmd = FBIOGET_GLOBAL_ALPHA; break;
        case E_FBM_SET_GLOBAL_ALPHA: fbcmd = FBIOSET_GLOBAL_ALPHA; break;
        case E_FBM_GET_COLORKEY: fbcmd = FBIOGET_COLORKEY; break;
        case E_FBM_SET_COLORKEY: fbcmd = FBIOSET_COLORKEY; break;
        case E_FBM_GET_LAYER_ATTRIBUTE: fbcmd = FBIOSET_DISPLAYLAYER_ATTRIBUTES; break;
        case E_FBM_SET_LAYER_ATTRIBUTE: fbcmd = FBIOSET_DISPLAYLAYER_ATTRIBUTES; break;
        default: return -1;
    }
    CHECK_IFNOT(ioctl(pFbLayerInfo->fb_device_fd, fbcmd, pArgs), 0, -1);
    return 0;
}
