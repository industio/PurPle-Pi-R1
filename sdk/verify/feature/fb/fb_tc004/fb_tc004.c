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

int main(int argc,char** argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    int maxSupportOSDWidth = 1280;
    int maxSupportOSDHeight = 720;
    int retval = 0;

    /* Open the file for reading and writing */
    fbFd = open (devfile, O_RDWR);
    if (fbFd == -1)
    {
        perror ("Error: cannot open framebuffer device");
        retval = ENOENT;
        goto out;
    }
    //get fb_fix_screeninfo
    if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror ("Error reading fixed information");
        retval = EINVAL;
        goto out;
    }
    printFixedInfo ();

    //get fb_var_screeninfo
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
        retval = EINVAL;
        goto out;
    }
    printVariableInfo ();

    /* Figure out the size of the screen in bytes */
    screensize = finfo.smem_len;
    maxSupportOSDWidth = vinfo.xres;
    maxSupportOSDHeight =vinfo.yres;
    /* Map the device to memory */
    frameBuffer =
        (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror ("Error: Failed to map framebuffer device to memory");
        retval = EINVAL;
        goto out;
    }
    printf ("Will draw 3 rectangles on the screen,\n"
            "they should be colored red, Green and blue (in that order).\n");

    //draw red rectangle
    drawRect (vinfo.xres / 8, vinfo.yres / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xffff0000);
    //draw Green rectangle
    drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xff00ff00);

    //draw DarkGoldenrod rectangle
    //drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
    //            vinfo.xres / 4, vinfo.yres / 4, 0xffb8860b);
    //draw blue rectanble
    drawRect (vinfo.xres * 5 / 8, vinfo.yres * 5 / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xff0000ff);

    sleep (5);
    printf (" Done.\n");

    //Pandisplay
    if (ioctl (fbFd, FBIOPAN_DISPLAY, &vinfo) == -1) {
        perror("Error: failed to FBIOPAN_DISPLAY");
        retval = EINVAL;
        goto out;
    }
    printf("After 5s sleep, it'll Switch resolution form[%d,%d] to [720,576]\n",vinfo.xres,vinfo.yres);
    sleep(5);
    vinfo.xres = 720;
    vinfo.yres = 576;

    //FBIOPUT_VSCREENINFO will call pandisplay
    if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vinfo)) {
        perror("Error: failed to FBIOPUT_VSCREENINFO");
        retval = EINVAL;
        goto out;
    }
    //FBIOSET_SHOW as flase in order to avoid garbage
    MI_BOOL bShown = FALSE;
    if (ioctl(fbFd, FBIOSET_SHOW, &bShown)) {
        perror("Error: failed to FBIOSET_SHOW");
        retval = EINVAL;
        goto out;
    }
    //retrive fixinfo and varinfo after FBIOPUT_VSCREENINFO
    //get fb_fix_screeninfo
    if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror ("Error reading fixed information");
        retval = EINVAL;
        goto out;
    }
    printFixedInfo ();
    //get fb_var_screeninfo
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
        retval = EINVAL;
        goto out;
    }
    printVariableInfo ();
    //set vinfo.yoffset as maxSupportOSDHeight in order to avoid overlap when
    //switch resolution from bigger to smaller
    vinfo.yoffset = maxSupportOSDHeight;

    //redraw rectangle according to new resolution
    //draw red rectangle
    drawRect (vinfo.xres / 8, vinfo.yres / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xffff0000);
    //draw DarkGoldenrod rectangle
    drawRect (vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
                vinfo.xres / 4, vinfo.yres / 4, 0xffb8860b);
    //draw blue rectanble
    drawRect (vinfo.xres * 5 / 8, vinfo.yres * 5 / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xff0000ff);
    sleep(5);
    //Pandisplay
    if (ioctl (fbFd, FBIOPAN_DISPLAY, &vinfo) == -1) {
        perror("Error: failed to FBIOPAN_DISPLAY");
        retval = EINVAL;
        goto out;
    }
    printf("After 5s sleep, it'll Switch resolution form[%d,%d] to [1280,720]\n",vinfo.xres,vinfo.yres);
    sleep(5);
    vinfo.xres = 1280;
    vinfo.yres = 720;
    vinfo.yoffset = 0;
    //FBIOSET_SHOW as flase in order to avoid garbage
    if (ioctl(fbFd, FBIOSET_SHOW, &bShown)) {
        perror("Error: failed to FBIOSET_SHOW");
        retval = EINVAL;
        goto out;
    }
    //FBIOPUT_VSCREENINFO will call pandisplay
    if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vinfo)) {
        perror("Error: failed to FBIOPUT_VSCREENINFO");
        retval = EINVAL;
        goto out;
    }
    //retrive fixinfo and varinfo after FBIOPUT_VSCREENINFO
    //get fb_fix_screeninfo
    if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror ("Error reading fixed information");
        retval = EINVAL;
        goto out;
    }
    printFixedInfo ();
    //get fb_var_screeninfo
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
        retval = EINVAL;
        goto out;
    }
    printVariableInfo ();
out:
    //unmap buffer
    if (frameBuffer != NULL) {
        munmap (frameBuffer, screensize);
        frameBuffer = NULL;
    }
    if ( fbFd != 0) {
        close (fbFd);
        fbFd = 0;
    }
    return retval;
}
