
#include "bmp.h"
#include <stdio.h>
#include <assert.h>
/*
 * compute image line size and bytes per pixel
 * from bits per pixel and width
 */
int bmp_ComputePitch(int bpp, MI_U32 width, MI_U32 *pitch, MI_BOOL does_round)
{
    int linesize;
    int bytespp = 1;

    if(bpp == 1)
        linesize = PIX2BYTES(width);
    else if(bpp <= 4)
        linesize = PIX2BYTES(width << 2);
    else if(bpp <= 8)
        linesize = width;
    else if(bpp <= 16) {
        linesize = width * 2;
        bytespp = 2;
    } else if(bpp <= 24) {
        linesize = width * 3;
        bytespp = 3;
    } else {
        linesize = width * 4;
        bytespp = 4;
    }

    /* rows are DWORD32 right aligned */
    if(does_round)
        *pitch = (linesize + 3) & -4;
    else
        *pitch = linesize;

    return bytespp;
}




/* Find the opaque pixel value corresponding to an RGB triple */
MI_U32 MapRGB(PixelFormat *format, MI_U8 r, MI_U8 g, MI_U8 b)
{
    MI_U32 pixv = (r >> format->Rloss) << format->Rshift
                  | (g >> format->Gloss) << format->Gshift
                  | (b >> format->Bloss) << format->Bshift
                  | format->Amask;
    return pixv;
}

/* Find the pixel value corresponding to an RGBA quadruple */
MI_U32 MapRGBA(PixelFormat *format, MI_U8 r, MI_U8 g, MI_U8 b, MI_U8 a)
{
    MI_U32 pixv = (r >> format->Rloss) << format->Rshift
                  | (g >> format->Gloss) << format->Gshift
                  | (b >> format->Bloss) << format->Bshift
                  | ((a >> format->Aloss) << format->Ashift & format->Amask);

    return pixv;

}

static inline MI_U8 *_mem_set_pixel(MI_U8 *dst, int bpp, MI_U32 pixel)
{
    switch(bpp) {
        case 1:
            *dst = pixel;
            break;

        case 2:
            *(MI_U16 *) dst = pixel;
            break;

        case 3:
            dst [0] = pixel;
            dst [1] = pixel >> 8;
            dst [2] = pixel >> 16;

            break;

        case 4:
            *(MI_U32 *) dst = pixel;
            break;
    }

    return dst + bpp;
}

/* This function compile a RGBA bitmap. */
void  CompileRGBABitmap(MI_U8 *dst_bits, MI_U32 dst_pitch,
                        const MI_U8 *src_bits, MI_U32 src_pitch,
                        MI_U32 w, MI_U32 h, PixelFormat *pixel_format, MI_U8 dst_bpp, MI_U8 src_bpp)
{
    MI_U32 x, y;
    MI_U8 *dst, *dst_line;
    const MI_U8 *src, *src_line;
    MI_U32 pixel;
    Color rgb;

    dst_line = dst_bits;
    src_line = src_bits;

    /* expand bits here. */
    for(y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;

        for(x = 0; x < w; x++) {
            rgb.r = *src++;
            rgb.g = *src++;
            rgb.b = *src++;

            if(src_bpp == 4) {
                rgb.a = *src;
                pixel = MapRGBA(pixel_format,
                                rgb.r, rgb.g, rgb.b, rgb.a);
                src++;
            } else {
                pixel = MapRGB(pixel_format, rgb.r, rgb.g, rgb.b);
            }

            dst = _mem_set_pixel(dst, dst_bpp, pixel);
        }

        src_line += src_pitch;
        dst_line += dst_pitch;
    }
}
#ifdef BYTE4
static void compilergba_bitmap_sl(HDC hdc, BYTE *bits, BYTE *alpha_mask, MYBITMAP *my_bmp)
{
    GAL_Color rgb;
    MI_U32 pixel;
    DWORD flags;
    PDC pdc;
    BYTE *src, *dst;
    int x = 0, w = 0;
    int bpp;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel(pdc->surface);
    dst = bits;
    src = my_bmp->bits;
    flags = my_bmp->flags;
    w = my_bmp->w;

    for(x = 0; x < w; x++) {
        if((flags & MYBMP_TYPE_MASK) == MYBMP_TYPE_BGR) {
            rgb.b = *src++;
            rgb.g = *src++;
            rgb.r = *src++;
        } else {
            rgb.r = *src++;
            rgb.g = *src++;
            rgb.b = *src++;
        }

        if(flags & MYBMP_RGBSIZE_4) {
            if(flags & MYBMP_ALPHA) {
                rgb.a = *src;

                if(alpha_mask) {
                    alpha_mask[x] = rgb.a;
                    pixel = GAL_MapRGB(pdc->surface->format,
                                       rgb.r, rgb.g, rgb.b);
                } else {
                    pixel = GAL_MapRGBA(pdc->surface->format,
                                        rgb.r, rgb.g, rgb.b, rgb.a);
                }
            } else {
                pixel = GAL_MapRGB(pdc->surface->format, rgb.r, rgb.g, rgb.b);
            }

            src++;
        } else {
            pixel = GAL_MapRGB(pdc->surface->format, rgb.r, rgb.g, rgb.b);
        }

        dst = _mem_set_pixel(dst, bpp, pixel);

    }

}
#endif

void _load_bitmap_sl(BITMAP *fb, BITMAP *logo, int y)
{
    MI_U8 *src_bits, *dst_bits;
    src_bits = logo->bmBits;
    dst_bits = fb->bmBits + fb->bmPitch * y;

    switch(logo->bmBytesPerPixel) {

        case 3:
            CompileRGBABitmap(dst_bits, fb->bmPitch,
                              src_bits, logo->bmPitch,
                              fb->bmWidth, 1, &fb->pxFmt, fb->bmBytesPerPixel, logo->bmBytesPerPixel);
            break;
#ifdef BYTE4

        case 4:
            compilergba_bitmap_sl(info->hdc, dst_bits, my_bmp);
            break;
#endif

        default: {
            printf("%s %d %d unsupport\n", __FUNCTION__, __LINE__, logo->bmBytesPerPixel);
            assert(0);
        }
        break;
    }
}


void CompileFormat(PixelFormat *format, int bpp,
                   MI_U32 Rmask, MI_U32 Gmask, MI_U32 Bmask, MI_U32 Amask)
{
    MI_U32 mask;

    format->alpha = 0xFF;

    switch(bpp) {
        case 1:
        case 4:
        case 8: {
            assert(0);
            printf("%s %d unsupport\n", __FUNCTION__, __LINE__);
        }

        default:
            format->Rshift = 0;
            format->Rloss = 8;

            if(Rmask) {
                for(mask = Rmask; !(mask & 0x01); mask >>= 1)
                    ++format->Rshift;

                for(; (mask & 0x01); mask >>= 1)
                    --format->Rloss;
            }

            format->Gshift = 0;
            format->Gloss = 8;

            if(Gmask) {
                for(mask = Gmask; !(mask & 0x01); mask >>= 1)
                    ++format->Gshift;

                for(; (mask & 0x01); mask >>= 1)
                    --format->Gloss;
            }

            format->Bshift = 0;
            format->Bloss = 8;

            if(Bmask) {
                for(mask = Bmask; !(mask & 0x01); mask >>= 1)
                    ++format->Bshift;

                for(; (mask & 0x01); mask >>= 1)
                    --format->Bloss;
            }

            format->Ashift = 0;
            format->Aloss = 8;

            if(Amask) {
                for(mask = Amask; !(mask & 0x01); mask >>= 1)
                    ++format->Ashift;

                for(; (mask & 0x01); mask >>= 1)
                    --format->Aloss;
            }

            format->Rmask = Rmask;
            format->Gmask = Gmask;
            format->Bmask = Bmask;
            format->Amask = Amask;
            break;
    }

    /* Calculate some standard bitmasks, if necessary
     * Note:  This could conflict with an alpha mask, if given.
     */
    if((bpp > 8) && !format->Rmask && !format->Gmask && !format->Bmask) {
        /* R-G-B */
        if(bpp > 24)
            bpp = 24;

        format->Rloss = 8 - (bpp / 3);
        format->Gloss = 8 - (bpp / 3) - (bpp % 3);
        format->Bloss = 8 - (bpp / 3);
        format->Rshift = ((bpp / 3) + (bpp % 3)) + (bpp / 3);
        format->Gshift = (bpp / 3);
        format->Bshift = 0;
        format->Rmask = ((0xFF >> format->Rloss) << format->Rshift);
        format->Gmask = ((0xFF >> format->Gloss) << format->Gshift);
        format->Bmask = ((0xFF >> format->Bloss) << format->Bshift);

    }
}

