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
/**
 *Conver color key value according to color format
 */
void convertColorKeyByFmt(MI_FB_ColorKey_t* colorkey)
{
        MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);
        MI_U8 red = colorkey->u8Red;
        MI_U8 green = colorkey->u8Green;
        MI_U8 blue = colorkey->u8Blue;
        switch (fmt)
        {
            case E_MI_FB_COLOR_FMT_RGB565:
            {
                colorkey->u8Red = (red >> 3)&(0x1f);
                colorkey->u8Green = (green >> 2)&(0x3f);
                colorkey->u8Blue = (blue >> 3)&(0x1f);
            }
            break;
            case E_MI_FB_COLOR_FMT_ARGB4444:
            {
                colorkey->u8Red = (red >> 4)&0xf;
                colorkey->u8Green = (green >> 4)&0xf;
                colorkey->u8Blue = (blue>>4)&0xf;
            }
            break;
            case E_MI_FB_COLOR_FMT_ARGB1555:
            {
                colorkey->u8Red = (red>>3) & 0x1f;
                colorkey->u8Green= (green >>3) & 0x1f;
                colorkey->u8Blue = (blue >>3) &0x1f;
            }
            break;
            default:
                printf("convertColorKeyByFmt colorfmt is %d\n",fmt);
            break;
        }
}

/**
 *Generate ColorKey channel
 */
void generateClrKeyFromRGB888(MI_U8 u8Red,
    MI_U8 u8Green, MI_U8 u8Blue, MI_FB_ColorKey_t* result)
{
    MI_U32 fullClrKey = 0;
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);
    if (!result)
        return;
    result->u8Red = u8Red;
    result->u8Green = u8Green;
    result->u8Blue = u8Blue;
    convertColorKeyByFmt(result);
    switch (fmt)
    {
        case E_MI_FB_COLOR_FMT_RGB565:
        {
            fullClrKey = convert565To8888(result->u8Red,
                result->u8Green, result->u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB4444:
        {
            fullClrKey = convert4444To8888(result->u8Red,
                result->u8Green, result->u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB1555:
        {
            fullClrKey = convert1555To8888(result->u8Red,
                result->u8Green, result->u8Blue);
        }
        break;
        case E_MI_FB_COLOR_FMT_ARGB8888:
        {
            fullClrKey = (0xff<<24) | (result->u8Red<<16)
                            | (result->u8Green<<8) | (result->u8Blue);
        }
        break;
        default :
        {
            printf("generateClrKeyFromRGB888 invalid clrfmt=%d\n",fmt);
        }
        break;
    }
    result->u8Red = (fullClrKey >> 16) & 0xff;
    result->u8Green = (fullClrKey >> 8) & 0xff;
    result->u8Blue = fullClrKey & 0xff;
}
/**
 *Dump OSD layer info
 */
void printDisplayLayerAttr(MI_FB_DisplayLayerAttr_t displayerAttr)
{
    printf("displayerAttr info:\n"
        "\tu32Xpos: %d\n"
        "\tu32YPos: %d\n"
        "\tu32dstWidth: %d\n"
        "\tu32dstHeight: %d\n"
        "\tu32DisplayWidth :%d\n"
        "\tu32DisplayHeight :%d\n"
        "\tu32ScreenWidth :%d\n"
        "\tu32ScreenHeight :%d\n"
        "\tbPreMul :%d\n"
        "eFbColorFmt :%d\n"
        "eFbOutputColorSpace:%d\n"
        "eFbDestDisplayPlane:%d\n"
        "\n",
        displayerAttr.u32Xpos,displayerAttr.u32YPos,displayerAttr.u32dstWidth,displayerAttr.u32dstHeight,
        displayerAttr.u32DisplayWidth,displayerAttr.u32DisplayHeight,displayerAttr.u32ScreenWidth,displayerAttr.u32ScreenHeight,
        displayerAttr.bPreMul,displayerAttr.eFbColorFmt,displayerAttr.eFbOutputColorSpace,
        displayerAttr.eFbDestDisplayPlane
        );
}

int main (int argc, char **argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;

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
    //MI_FB_ColorFmt_e clrFmt = getFBColorFmt(&vinfo);

    /* Map the device to memory */
    frameBuffer =
        (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror ("Error: Failed to map framebuffer device to memory");
        exit (4);
    }
    printf ("Will draw 3 rectangles on the screen,\n"
            "they should be colored red, DarkGoldenrod and blue (in that order).\n");

    //draw red rectangle
    drawRect (vinfo.xres / 8, vinfo.yres / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xffff0000);
    //drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
    //         vinfo.xres / 4, vinfo.yres / 4, 0xff00ff00);

    //draw DarkGoldenrod rectangle
    drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
                vinfo.xres / 4, vinfo.yres / 4, 0xffb8860b);
    //draw blue rectanble
    drawRect (vinfo.xres * 5 / 8, vinfo.yres * 5 / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xff0000ff);

    sleep (5);
    printf (" Done.\n");

    //unmap buffer
    munmap (frameBuffer, screensize);
    //Pandisplay
    if (ioctl (fbFd, FBIOPAN_DISPLAY, &vinfo) == -1) {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(5);
    }

	//test FBIOGET_SHOW
	MI_BOOL bShown;
	if (ioctl(fbFd, FBIOGET_SHOW,&bShown)<0) {
		perror("Error: failed to FBIOGET_SHOW");
		exit(6);
	}
	printf("FBIOGET_SHOW result bShown is %d\n",bShown);

    //test FBIOSET_SHOW
    bShown  = FALSE;
    if (ioctl(fbFd, FBIOSET_SHOW,&bShown)<0) {
        perror("Error: failed to FBIOSET_SHOW");
        exit(7);
    }

    //test FBIOGET_SCREEN_LOCATION
    MI_FB_Rectangle_t dispRegion;
    if (ioctl(fbFd, FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
        perror("Error: failed to FBIOGET_SCREEN_LOCATION");
        exit(8);
    }
    printf("After FBIOGET_SCREEN_LOCATION dispRegoin:[%d,%d,%d,%d]\n",dispRegion.u16Xpos,dispRegion.u16Ypos,dispRegion.u16Width,dispRegion.u16Height);

    //test FBIOSET_SHOW to enable show
    bShown  = true;
    if (ioctl(fbFd, FBIOSET_SHOW,&bShown)<0) {
        perror("Error: failed to FBIOSET_SHOW");
        exit(7);
    }
    //test FBIOSET_SCREEN_LOCATION
    dispRegion.u16Xpos = 100;
    dispRegion.u16Ypos = 100;
    dispRegion.u16Width = vinfo.xres;
    dispRegion.u16Height = vinfo.yres;
    sleep(5);
    printf("After sleep 5s the OSD location was set as [%d,%d,%d,%d]\n",
        dispRegion.u16Xpos, dispRegion.u16Ypos,dispRegion.u16Width,dispRegion.u16Height);
    if (ioctl(fbFd, FBIOSET_SCREEN_LOCATION,&dispRegion) < 0) {
        perror("Error: failed to FBIOSET_SCREEN_LOCATION");
        exit(8);
    }
    if (ioctl(fbFd, FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
        perror("Error: failed to FBIOGET_SCREEN_LOCATION");
        exit(8);
    }
    printf("Exec FBIOSET_SCREEN_LOCATION the dispRegion is [%d,%d,%d,%d]\n",
        dispRegion.u16Xpos,dispRegion.u16Ypos,dispRegion.u16Width,dispRegion.u16Height);

    //test FBIOGET_COLORKEY
    MI_FB_ColorKey_t colorKeyInfo;
    if (ioctl(fbFd, FBIOGET_COLORKEY,&colorKeyInfo) < 0) {
        perror("Error: failed to FBIOGET_COLORKEY");
        exit(9);
    }
    printf("FBIOGET_COLORKEY colorkey Info Colorkey enable:%d,Colorkey RGB[0x%x,0x%x,0x%x]\n",
        colorKeyInfo.bKeyEnable,colorKeyInfo.u8Red,colorKeyInfo.u8Green,colorKeyInfo.u8Blue);
    colorKeyInfo.bKeyEnable = TRUE;
     //0xb8860b stand for DarkGoldenrod in RGB
    MI_U8 u8RedChanel = 0xb8;
    MI_U8 u8GreenChanel = 0x86;
    MI_U8 u8BlueChanel = 0x0b;
    //test FBIOSET_COLORKEY
    printf("After sleep 5s FBIOSET_COLORKEY enable colorkey the filter color[0x%x,0x%x,0x%x]\n",
        u8RedChanel,u8GreenChanel,u8BlueChanel);
    sleep(5);
    generateClrKeyFromRGB888(u8RedChanel, u8GreenChanel, u8BlueChanel,
        &colorKeyInfo);
    if (ioctl(fbFd, FBIOSET_COLORKEY, &colorKeyInfo) < 0) {
        perror("Error: failed to FBIOGET_COLORKEY");
        exit(10);
    }
    if (ioctl(fbFd, FBIOGET_COLORKEY, &colorKeyInfo) < 0) {
        perror("Error: failed to FBIOGET_COLORKEY");
        exit(9);
    }
     printf("Finally FBIOGET_COLORKEY colorkey Info Colorkey enable:%d,Colorkey RGB[0x%x,0x%x,0x%x]\n",
        colorKeyInfo.bKeyEnable,colorKeyInfo.u8Red,colorKeyInfo.u8Green,colorKeyInfo.u8Blue);

     //test FBIOGET_GLOBAL_ALPHA
     MI_FB_GlobalAlpha_t alphaInfo;
     if (ioctl(fbFd, FBIOGET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        exit(11);
     }
     printf("FBIOGET_GLOBAL_ALPHA alpha info: alpha blend enable=%d,Multialpha enable=%d,Global Alpha=%d,u8Alpha0=%d,u8Alpha1=%d\n",
        alphaInfo.bAlphaEnable,alphaInfo.bAlphaChannel,alphaInfo.u8GlobalAlpha,alphaInfo.u8Alpha0,alphaInfo.u8Alpha1);

     //test FBIOSET_GLOBAL_ALPHA. to DISABLE alpha blend
     alphaInfo.bAlphaEnable = FALSE;
     if (ioctl(fbFd, FBIOSET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        exit(12);
     }

     if (ioctl(fbFd, FBIOGET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        exit(11);
     }
     printf("After SET_GLOBAL_ALPHA step1 FBIOGET_GLOBAL_ALPHA alpha info: alpha blend enable=%d,Multialpha enable=%d,Global Alpha=%d,u8Alpha0=%d,u8Alpha1=%d\n",
        alphaInfo.bAlphaEnable,alphaInfo.bAlphaChannel,alphaInfo.u8GlobalAlpha,alphaInfo.u8Alpha0,alphaInfo.u8Alpha1);

     //FBIOSET_GLOBAL_ALPHA. enable alpha blend and enable multialpha
     alphaInfo.bAlphaEnable = TRUE;
     alphaInfo.bAlphaChannel= TRUE;
     alphaInfo.u8GlobalAlpha = 0x70;
     if (ioctl(fbFd, FBIOSET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        exit(12);
     }

     //FBIOGET_GLOBAL_ALPHA
      if (ioctl(fbFd, FBIOGET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        exit(11);
     }
     printf("After SET_GLOBAL_ALPHA step2222 FBIOGET_GLOBAL_ALPHA alpha info: alpha blend enable=%d,Multialpha enable=%d,Global Alpha=0x%x,u8Alpha0=%d,u8Alpha1=%d\n",
        alphaInfo.bAlphaEnable,alphaInfo.bAlphaChannel,alphaInfo.u8GlobalAlpha,alphaInfo.u8Alpha0,alphaInfo.u8Alpha1);

     //FBIOGET_DISPLAYLAYER_ATTRIBUTES
     MI_FB_DisplayLayerAttr_t displayerAttr;
    if (ioctl(fbFd, FBIOGET_DISPLAYLAYER_ATTRIBUTES, &displayerAttr) < 0) {
        perror("Error: failed to FBIOGET_DISPLAYLAYER_ATTRIBUTES");
        exit(13);
    }
    printf("FBIOGET_DISPLAYLAYER_ATTRIBUTES displayLayerAttr:\n");
    printDisplayLayerAttr(displayerAttr);

    close (fbFd);
    return 0;
}
