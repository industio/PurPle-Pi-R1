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

int main(int argc,char** argv)
{
    const char *devfile = "/dev/fb0";
    int fbFd = 0;
    unsigned int osdWidth = 1920;
    unsigned int osdHeight = 1080;
    //Icon width 44, heigth 56, ARGB8888
    int nIconBytes = 44*56*4;
    int nIconPitch = 44;
    int reqPosX = 0;
    int reqPosY = 0;
    FILE* fp = NULL;
    unsigned char buff[44*56*4] = {0};
    MI_FB_CursorAttr_t stCursorAttr;
    memset(&stCursorAttr, 0, sizeof(MI_FB_CursorAttr_t));
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
        exit(2);
    }
    printFixedInfo ();

    //get fb_var_screeninfo
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
        exit(3);
    }
    printVariableInfo ();
    osdWidth = vinfo.xres;
    osdHeight = vinfo.yres;

    //read icon data
    fp = fopen("cursor.raw", "rb");
    if (fp == NULL) {
        printf("Can not find cursor.raw in current file!!\n");
        exit(0);
    }
    fread(buff, 1, nIconBytes, fp);
    fclose(fp);
    //set curosr Icon && set positon
    stCursorAttr.stCursorImageInfo.u32Width = 44;
    stCursorAttr.stCursorImageInfo.u32Height = 56;
    stCursorAttr.stCursorImageInfo.u32Pitch = nIconPitch;
    stCursorAttr.stCursorImageInfo.eColorFmt = E_MI_FB_COLOR_FMT_ARGB8888;
    stCursorAttr.stCursorImageInfo.data = buff;
    stCursorAttr.u32HotSpotX = 18;
    stCursorAttr.u32HotSpotY = 9;
    stCursorAttr.u32XPos = 100;
    stCursorAttr.u32YPos = 100;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_ICON
        | E_MI_FB_CURSOR_ATTR_MASK_SHOW | E_MI_FB_CURSOR_ATTR_MASK_POS;
    if (ioctl(fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
        perror ("Error FBIOSET_CURSOR_ATTRIBUTE");
        exit(4);
    }
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_COLORKEY;
    stCursorAttr.stColorKey.bKeyEnable = TRUE;
    stCursorAttr.stColorKey.u8Red = 0xff;
    stCursorAttr.stColorKey.u8Green = 0xff;
    stCursorAttr.stColorKey.u8Blue = 0xff;
    printf("After sleep 5s it will enable colorkey [%x,%x,%x]\n",stCursorAttr.stColorKey.u8Red,
        stCursorAttr.stColorKey.u8Green,stCursorAttr.stColorKey.u8Blue);
    sleep(5);
    if (ioctl(fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
        perror ("Error FBIOSET_CURSOR_ATTRIBUTE");
        exit(4);
    }
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_COLORKEY;
    stCursorAttr.stColorKey.bKeyEnable = FALSE;
    printf("After sleep 5s it will disable colorkey [%x,%x,%x]\n");
    sleep(5);
    if (ioctl(fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
        perror ("Error FBIOSET_CURSOR_ATTRIBUTE");
        exit(4);
    }
    while (1)
    {
        reqPosX = rand() % osdWidth;
        reqPosY = rand() % osdHeight;
        stCursorAttr.u32XPos = reqPosX;
        stCursorAttr.u32YPos = reqPosY;
        stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_POS;
        if (ioctl(fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
            perror ("Error FBIOSET_CURSOR_ATTRIBUTE");
            exit(4);
        }
    }
    close(fbFd);
    exit(0);
}
