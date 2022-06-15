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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "mstarFb.h"

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

//Start of frame buffer mem
static char *frameBuffer = NULL;

/**
 *dump fix info of Framebuffer
 */
void printFixedInfo ()
{
    printf ("Fixed screen info:\n"
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

/**
 *dump var info of Framebuffer
 */
void printVariableInfo ()
{
    printf ("Variable screen info:\n"
            "\txres: %d\n"
            "\tyres: %d\n"
            "\txres_virtual: %d\n"
            "\tyres_virtual: %d\n"
            "\tyoffset: %d\n"
            "\txoffset: %d\n"
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
/**
 *get Color format fo framebuffer
 */
MI_FB_ColorFmt_e getFBColorFmt(struct fb_var_screeninfo *var)
{
    MI_FB_ColorFmt_e fmt = E_MI_FB_COLOR_FMT_INVALID;

    switch (var->bits_per_pixel)
    {
        case 16:
        {
            if (var->transp.length == 0) //RGB565
            {
                fmt = E_MI_FB_COLOR_FMT_RGB565;
            }
            else if (var->transp.length ==1) //ARGB 1555
            {
                fmt = E_MI_FB_COLOR_FMT_ARGB1555;
            }
            else if (var->transp.length == 4) //ARGB4444
            {
                fmt = E_MI_FB_COLOR_FMT_ARGB4444;
            }
        }
        break;
        //ARGB8888
        case 32:
        {
            fmt = E_MI_FB_COLOR_FMT_ARGB8888;
        }
        break;
        default:
            fmt = E_MI_FB_COLOR_FMT_INVALID;
            break;
    }
    return fmt;
}

/**
 * draw Rectangle. the colormat of Framebuffer is ARGB8888
 */
void drawRect_rgb32 (int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 4;
    const int stride = finfo.line_length / bytesPerPixel;

    int *dest = (int *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color;
        }
        dest += stride;
    }
}

/**
 * draw Rectangle. the colormat of Framebuffer is RGB565
 */
void drawRect_rgb16 (int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 2;
    const int stride = finfo.line_length / bytesPerPixel;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 2);
    const int blue = (color & 0xff) >> 3;
    const short color16 = blue | (green << 5) | (red << (5 + 6));

    short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color16;
        }
        dest += stride;
    }
}

/**
 *draw Rectangle. the color format of Framebuffer is ARGB1555
 */
void drawRect_rgb15 (int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 2;
    const int stride = finfo.line_length / bytesPerPixel;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 3);
    const int blue = (color & 0xff) >> 3;
    const short color15 = blue | (green << 5) | (red << (5 + 5)) | 0x8000;

    short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color15;
        }
        dest += stride;
    }
}

/**
 *draw Rectangle. the color format of Framebuffer is ARGB1444
 */
void  drawRect_rgb12(int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel =2;
    const int stride = finfo.line_length / bytesPerPixel;
    const int red = (color & 0xff0000) >> (16 + 4);
    const int green = (color & 0xff00) >> (8 + 4);
    const int blue = (color & 0xff) >> 4;
    const short color16 = blue | (green << 4) | (red << (4+4)) |0xf000;
    short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dest[x] = color16;
        }
        dest += stride;
    }
}
/**
 *draw Rectangle. accroding to Framebuffer format
 */
void drawRect (int x0, int y0, int width, int height, int color)
{
     MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);
     switch (fmt)
     {
         case E_MI_FB_COLOR_FMT_ARGB8888:
         {
            drawRect_rgb32(x0, y0, width, height, color);
         }
         break;
         case E_MI_FB_COLOR_FMT_RGB565:
         {
            drawRect_rgb16(x0, y0, width, height, color);
         }
         break;
         case E_MI_FB_COLOR_FMT_ARGB4444:
         {
            drawRect_rgb12(x0, y0, width, height, color);
         }
         break;
         case E_MI_FB_COLOR_FMT_ARGB1555:
         {
            drawRect_rgb15(x0, y0, width, height, color);
         }
         break;
         default:
             printf ("Warning: drawRect() not implemented for color Fmt %i\n",
                fmt);
     }
}
/**
 *Convert RGB chanel from ARGB1555 to ARGB8888
 */
static MI_U32 convert1555To8888(MI_U8 u8Red, MI_U8 u8Green, MI_U8 u8Blue)
{
    MI_U32 fullclr = 0;
    MI_U16 clr = (0x1 << 15) | ((u8Red & 0x1f) << 10)
        | ((u8Green&0x1f) << 5) | (u8Blue & 0x1f);
    //amplify
    fullclr = ((clr>>10)&0x1f)<<3; //R Component
    fullclr |= ((clr>>12)&0x7);
    fullclr = fullclr << 8;
    fullclr |= ((clr>>5)&0x1f)<<3;      // G Component
    fullclr |= ((clr>>7)&0x7);
    fullclr = fullclr << 8;
    fullclr |= (clr&0x1f)<<3;           // B
    fullclr |= ((clr>>2)&0x7);
    return fullclr;
}
/**
 *Convert RGB565 chanel from RGB565 to ARGB8888
 */
static MI_U32 convert565To8888(MI_U8 u8Red, MI_U8 u8Green, MI_U8 u8Blue)
{
    MI_U32 fullclr = 0;
    MI_U16 clr = ((u8Red & 0x1f) << 11) | ((u8Green & 0x3f)<<5)
        | (u8Blue & 0x1f);
    //amplify to ARGB8888
    fullclr = ((clr >> 11)&0x1f) << 3; //R
    fullclr |= ((clr >> 13)&0x7);
    fullclr = fullclr << 8;
    fullclr |= ((clr >> 5)&0x3f) << 2; //G
    fullclr |= ((clr >> 9) &0x3);
    fullclr = fullclr << 8;
    fullclr |= (clr&0x1f) << 3; //B
    fullclr |= ((clr>>2)&0x7);
    return fullclr;
}
/**
 *Convert ARGB4444 chanel from ARGB4444 to ARGB8888
 */
static MI_U32 convert4444To8888(MI_U8 u8Red, MI_U8 u8Green, MI_U8 u8Blue)
{
    MI_U32 fullclr = 0;
    MI_U16 clr = ((u8Red & 0xf) << 8) | ((u8Green & 0xf) << 4)
        |((u8Blue & 0xf));
    //amplify to ARGB8888
    fullclr = ((clr >> 8) & 0xf) << 4; //R
    fullclr |= ((clr >> 8) & 0xf);
    fullclr = fullclr << 8;
    fullclr |= ((clr >> 4) & 0xf) << 4; //G
    fullclr |= ((clr >> 4) & 0xf);
    fullclr = fullclr << 8;
    fullclr |= (clr & 0xf) << 4; //B
    fullclr |= (clr & 0xf);
    return fullclr;
}

int main(int argc,char** argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    MI_FB_ColorFmt_e efmt = E_MI_FB_COLOR_FMT_INVALID;
    MI_U32 fullClrKey = 0;
    /* Open the file for reading and writing */
    fbFd = open (devfile, O_RDWR);
    if (fbFd == -1)
    {
        perror ("Error: cannot open framebuffer device");
        exit (1);
    }
    //get fb_fix_screeninfo
    if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror ("Error reading fixed information");
        exit (2);
    }
    printFixedInfo ();

    //get fb_var_screeninfo
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
        exit (3);
    }
    printVariableInfo ();

    /* Figure out the size of the screen in bytes */
    screensize = finfo.smem_len;

    /* Map the device to memory */
    frameBuffer =
        (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror ("Error: Failed to map framebuffer device to memory");
        exit (4);
    }
    printf ("Will draw 1 rectangles on the screen,\n"
            "they should be colored Purple (in that order).\n");
     //draw Purple rectanble
     //0xff9933fa stand for colored Purple as RGB888 format
    drawRect (vinfo.xres / 2, vinfo.yres  / 2,
             vinfo.xres / 4, vinfo.yres / 4, 0xff9933fa);

    sleep (5);
    printf (" Done.\n");
    //unmap buffer
    munmap (frameBuffer, screensize);
    //Pandisplay
    if (ioctl (fbFd, FBIOPAN_DISPLAY, &vinfo) == -1) {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(5);
    }
    //test FBIOSET_COLORKEY
    MI_FB_ColorKey_t colorKeyInfo;
    colorKeyInfo.bKeyEnable = TRUE;
    colorKeyInfo.u8Red = 0x99;
    colorKeyInfo.u8Green = 0x33;
    colorKeyInfo.u8Blue = 0xfa;
    efmt = getFBColorFmt(&vinfo);
    /**
      *Convert colorkey value from RGB88 to current format
      *In order to avoid tolerance result from different format
      */
    switch (efmt)
    {
        case E_MI_FB_COLOR_FMT_RGB565:
        {
            colorKeyInfo.u8Red =  (colorKeyInfo.u8Red >> 3)&(0x1f);
            colorKeyInfo.u8Green = (colorKeyInfo.u8Green >> 2)&(0x3f);
            colorKeyInfo.u8Blue = (colorKeyInfo.u8Blue >> 3)&(0x1f);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB4444:
        {
            colorKeyInfo.u8Red = (colorKeyInfo.u8Red >> 4)&(0xf);
            colorKeyInfo.u8Green = (colorKeyInfo.u8Green >> 4)&(0xf);
            colorKeyInfo.u8Blue = (colorKeyInfo.u8Blue >> 4)&(0xf);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB1555:
        {
             colorKeyInfo.u8Red =  (colorKeyInfo.u8Red >> 3)&(0x1f);
            colorKeyInfo.u8Green = (colorKeyInfo.u8Green >> 3)&(0x1f);
            colorKeyInfo.u8Blue = (colorKeyInfo.u8Blue >> 3)&(0x1f);
        }
        break;
        default:
            printf("convertColorKeyByFmt colorfmt is %d\n",efmt);
            break;
    }
    /*
     *Convert color key expressed as current format to ARGB888
     *Because GOP hw can only support ARGB888 format
     */
     switch (efmt)
     {
        case E_MI_FB_COLOR_FMT_RGB565:
        {
            fullClrKey = convert565To8888(colorKeyInfo.u8Red,
                colorKeyInfo.u8Green, colorKeyInfo.u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB4444:
        {
            fullClrKey = convert4444To8888(colorKeyInfo.u8Red,
                colorKeyInfo.u8Green, colorKeyInfo.u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB1555:
        {
            fullClrKey = convert1555To8888(colorKeyInfo.u8Red,
                colorKeyInfo.u8Green, colorKeyInfo.u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB8888:
        {
            fullClrKey = (0xff<<24) | (colorKeyInfo.u8Red<<16)
                            | (colorKeyInfo.u8Red<<8) | (colorKeyInfo.u8Red);
        }
        break;
        default :
        {
            printf("generateClrKeyFromRGB888 invalid clrfmt=%d\n",efmt);
        }
        break;
     }
     /**
      *Set RGB chanel according to fullClrKey
      */
    colorKeyInfo.u8Red = (fullClrKey >> 16) & 0xff;
    colorKeyInfo.u8Green = (fullClrKey >> 8) & 0xff;
    colorKeyInfo.u8Blue = fullClrKey & 0xff;
    printf("After sleep 5s FBIOSET_COLORKEY enable colorkey the filter color[0x%x,0x%x,0x%x]\n",
        colorKeyInfo.u8Red,colorKeyInfo.u8Green,colorKeyInfo.u8Blue);
    sleep(5);
    if (ioctl(fbFd, FBIOSET_COLORKEY, &colorKeyInfo) < 0) {
        perror("Error: failed to FBIOGET_COLORKEY");
        exit(10);
    }
    close (fbFd);
    return 0;
}
