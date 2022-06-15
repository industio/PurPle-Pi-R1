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

#include "mi_sys.h"
#include "st_common.h"
#include "st_fb.h"
#include "mstarFb.h"

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

//Start of frame buffer mem
static MI_U8 *frameBuffer = NULL;
static MI_S32 g_fbFd = 0;
static MI_S32 g_screensize = 0;

#define CHECK_FB_INIT \
    do{ \
        if (frameBuffer == NULL) \
        { \
            ST_ERR("fb init fail\n"); \
            return; \
        } \
    } while(0)

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
            else if (var->transp.length == 1) //ARGB 1555
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
    int stride = 0;
    int *dest = NULL;
    int x = 0, y = 0;

    CHECK_FB_INIT;

    stride = finfo.line_length / bytesPerPixel;
    dest = (int *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

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
    int stride = 0;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 2);
    const int blue = (color & 0xff) >> 3;
    const short color16 = blue | (green << 5) | (red << (5 + 6));
    short *dest = NULL;
    int x = 0, y = 0;

    CHECK_FB_INIT;

    stride = finfo.line_length / bytesPerPixel;
    dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

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
 *draw point. the color format of Framebuffer is ARGB1555
 */
void drawPoint_rgb15 (int x, int y, int color)
{
    const int bytesPerPixel = 2;
    int stride = 0;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 3);
    const int blue = (color & 0xff) >> 3;
    const short color15 = blue | (green << 5) | (red << (5 + 5)) | 0x8000;
    short *dest = NULL;

    CHECK_FB_INIT;

    stride = finfo.line_length / bytesPerPixel;
    dest = (short *) (frameBuffer)
        + (y + vinfo.yoffset) * stride + (x + vinfo.xoffset);

    dest[0] = color15;
}

void drawLine_rgb15(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int ux = dx >0 ? 1 : -1;
    int uy = dy >0 ? 1 : -1;
    int dx2 = dx << 1;
    int dy2 = dy << 1;
    int e, x, y;

    if(abs(dx) > abs(dy))
    {
        e = -dx;
        x = x0;
        y = y0;
        for (x = x0; x < x1; x += ux)
        {
            // printf ("%d,%d, x1:%d\n",x, y, x1);
            drawPoint_rgb15(x, y, color);
            e = e + dy2;
            if (e > 0)
            {
                y += uy;
                e = e - dx2;
            }
        }
    }
    else
    {
        e = -dy;
        x = x0;
        y = y0;
        for (y = y0; y < y1; y += uy)
        {
            // printf ("%d,%d, y1:%d\n",x, y, y1);
            drawPoint_rgb15(x, y, color);
            e = e + dx2;
            if (e > 0)
            {
                x += ux;
                e= e - dy2;
            }
        }
    }
}

#if 0
void drawCircle_rgb15(int xc, int yc, int r, int color)
{
    int x, y, d;

    x = 0;
    y = r;

    d = 3 - 2 * r;

printf("ddddddddddddddddddddd, %d,%d - %d,%d\n", xc, yc, x, y);
    drawLine_rgb15(xc, yc, x, y, color);

    while(x < y)
    {
        if(d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * ( x - y ) + 10;
            y --;
        }
        x ++;

        drawLine_rgb15(xc, yc, x, y, color);
    }
}
#endif

void drawCircle_rgb15(int cx, int cy, int r, int color)
{
	int r2 = r * r + r;
	int x = 0, x2 = 0, dx2 = 1;
	int y = r, y2 = y*y, dy2 = 2*y - 1;
	int sum = r2;

	while(x <= y)
    {
        drawLine_rgb15(cx - y, cy + x, cx + y, cy + x, color);
        if (x) drawLine_rgb15(cx - y, cy - x, cx + y, cy - x, color);

		sum -= dx2;
		if (sum <= y2)
        {
			if (x != y)
            {
                drawLine_rgb15(cx - x, cy - y, cx + x, cy - y, color);
				if (y) drawLine_rgb15(cx - x, cy + y, cx + x, cy + y, color);
			} /* if */
			y--; y2 -= dy2; dy2 -= 2;
		} /* if */
		x++;
		x2 += dx2;
		dx2 += 2;
	} /* while */
} /* bh */


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
    const int alpha = (color>>24) ? 0x8000:0;
    const short color15 = blue | (green << 5) | (red << (5 + 5)) | alpha;
    short *dest = NULL;
    int x = 0, y = 0;

    CHECK_FB_INIT;

    dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);


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
    int stride = 0;
    const int red = (color & 0xff0000) >> (16 + 4);
    const int green = (color & 0xff00) >> (8 + 4);
    const int blue = (color & 0xff) >> 4;
    const short color16 = blue | (green << 4) | (red << (4+4)) |0xf000;
    short *dest = NULL;
    int x, y;

    stride = finfo.line_length / bytesPerPixel;
    dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dest[x] = color16;
        }
        dest += stride;
    }
}

void drawPoint (int x, int y, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
        case E_MI_FB_COLOR_FMT_ARGB1555:
         {
            drawPoint_rgb15(x, y, color);
         }
         break;
         default:
             printf ("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

void drawLine (int x0, int y0, int x1, int y1, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
        case E_MI_FB_COLOR_FMT_ARGB1555:
         {
            drawLine_rgb15(x0, y0, x1, y1, color);
         }
         break;
         default:
             printf ("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

void drawCircle(int xc, int yc, int r, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
        case E_MI_FB_COLOR_FMT_ARGB1555:
         {
            drawCircle_rgb15(xc, yc, r, color);
         }
         break;
         default:
             printf ("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

/**
 *draw Rectangle. accroding to Framebuffer format
 */
void drawRect (int x0, int y0, int width, int height, int color)
{
     MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);
     printf ("%s[%d]: Fmt = %d, xywh[%d,%d,%d,%d] !!!!!!!!!!!!!\n", __FUNCTION__, __LINE__, fmt, x0, y0, width, height);
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
             printf ("%s[%d]: drawRect() not implemented for color Fmt %d\n",
                    __FUNCTION__, __LINE__, fmt);
     }
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

MI_S32 ST_Fb_Init(void)
{
    const MI_U8 *devfile = "/dev/fb0";
    MI_BOOL bShown;

    /* Open the file for reading and writing */
    g_fbFd = open (devfile, O_RDWR);
    if (g_fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(0);
    }

    //get fb_fix_screeninfo
    if (ioctl(g_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(0);
    }
    printFixedInfo();

    //get fb_var_screeninfo
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(0);
    }
    printVariableInfo();

    /* Figure out the size of the screen in bytes */
    g_screensize = finfo.smem_len;

    /* Map the device to memory */
    frameBuffer =
        (char *) mmap(0, g_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        exit(0);
    }

    // clear framebuffer
    //drawRect(0, 0, vinfo.xres, vinfo.yres, ARGB888_BLUE);
    // memset(frameBuffer, ARGB1555_RED, g_screensize);
#if 0
    if (ioctl(g_fbFd, FBIOGET_SHOW,&bShown)<0) {
        perror("Error: failed to FBIOGET_SHOW");
        exit(0);
    }
    printf("FBIOGET_SHOW result bShown is %d\n",bShown);

    //test FBIOSET_SHOW
    bShown  = TRUE;
    if (ioctl(g_fbFd, FBIOSET_SHOW,&bShown)<0) {
        perror("Error: failed to FBIOSET_SHOW");
        exit(0);
    }
#endif

    return MI_SUCCESS;
}

void ST_Fb_SetColorFmt(MI_FB_ColorFmt_e enFormat)
{
    if (g_fbFd < 0)
    {
        return;
    }

    MI_FB_DisplayLayerAttr_t stDisplayAttr;

    // set color format
    memset(&stDisplayAttr, 0, sizeof(MI_FB_DisplayLayerAttr_t));
    if (ioctl(g_fbFd, FBIOGET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOGET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }

    printf("%s %d, u32Xpos:%d,u32YPos:%d,u32dstWidth:%d,u32dstHeight:%d,u32DisplayWidth:%d,u32DisplayHeight:%d,u32ScreenWidth:%d,u32ScreenHeight:%d\n", __func__, __LINE__,
        stDisplayAttr.u32Xpos, stDisplayAttr.u32YPos, stDisplayAttr.u32dstWidth, stDisplayAttr.u32dstHeight,
        stDisplayAttr.u32DisplayWidth, stDisplayAttr.u32DisplayHeight, stDisplayAttr.u32ScreenWidth, stDisplayAttr.u32ScreenHeight);

    if (enFormat == E_MI_FB_COLOR_FMT_YUV422)
    {
        printf("not support this format, set default format, ARGB1555\n");
        enFormat = E_MI_FB_COLOR_FMT_ARGB1555;
    }
    stDisplayAttr.u32SetAttrMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB;
    stDisplayAttr.eFbColorFmt = enFormat;
    if (ioctl(g_fbFd, FBIOSET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOSET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(0);
    }
    printVariableInfo();
}

MI_S32 ST_Fb_DeInit()
{
    if (frameBuffer != NULL)
    {
        munmap (frameBuffer, g_screensize);
        frameBuffer = NULL;
    }

    if (g_fbFd > 0)
    {
        close(g_fbFd);
        g_fbFd = 0;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal)
{
    drawRect((MI_S32)pRect->u16X, (MI_S32)pRect->u16Y, (MI_S32)pRect->u16Width, (MI_S32)pRect->u16Height, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_DrawRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal)
{
#if 0
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
     {
         case E_MI_FB_COLOR_FMT_ARGB1555:
         {
            drawLine_rgb15(pRect->u16X, pRect->u16Y,
                pRect->u16X + pRect->u16Width, pRect->u16Y, u32ColorVal);

            drawLine_rgb15(pRect->u16X + pRect->u16Width, pRect->u16Y,
                pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height, u32ColorVal);

            printf("dddddddddd\n");
            drawLine_rgb15(pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height,
                pRect->u16X, pRect->u16Y + pRect->u16Height, u32ColorVal);
            printf("fffff\n");
            drawLine_rgb15(pRect->u16X, pRect->u16Y + pRect->u16Height,
                pRect->u16X, pRect->u16Y, u32ColorVal);
         }
         break;
         default:
             printf ("Warning: drawRect() not implemented for color Fmt %i\n",
                fmt);
     }
#endif

    drawLine(pRect->u16X, pRect->u16Y,
                pRect->u16X + pRect->u16Width, pRect->u16Y, u32ColorVal);

    drawLine(pRect->u16X + pRect->u16Width, pRect->u16Y,
                pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height, u32ColorVal);

    drawLine(pRect->u16X, pRect->u16Y + pRect->u16Height,
            pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height,u32ColorVal);

    drawLine(pRect->u16X, pRect->u16Y,
            pRect->u16X, pRect->u16Y + pRect->u16Height,u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillLine(MI_U16 x0, MI_U16 y0, MI_U16 x1, MI_U16 y1, MI_U32 u32ColorVal)
{
    drawLine(x0, y0, x1, y1, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillPoint(MI_U16 x, MI_U16 y, MI_U32 u32ColorVal)
{
    drawPoint(x, y, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillCircle(MI_U16 x, MI_U16 y, MI_U16 r, MI_U32 u32ColorVal)
{
    drawCircle(x, y, r, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_GetColorKey(MI_U32 *pu32ColorKeyVal)
{
    MI_FB_ColorKey_t colorKeyInfo;
    if (ioctl(g_fbFd, FBIOGET_COLORKEY,&colorKeyInfo) < 0) {
        ST_ERR("Error: failed to FBIOGET_COLORKEY\n");
        exit(0);
    }
    *pu32ColorKeyVal = (colorKeyInfo.bKeyEnable << 24)|(colorKeyInfo.u8Red << 16)|(colorKeyInfo.u8Green << 8)|(colorKeyInfo.u8Blue);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_SetColorKey(MI_U32 u32ColorKeyVal)
{
    MI_FB_ColorKey_t colorKeyInfo;
    if (ioctl(g_fbFd, FBIOGET_COLORKEY,&colorKeyInfo) < 0) {
        ST_ERR("Error: failed to FBIOGET_COLORKEY\n");
        exit(0);
    }

    colorKeyInfo.bKeyEnable = TRUE;
    colorKeyInfo.u8Red = PIXEL8888RED(u32ColorKeyVal);
    colorKeyInfo.u8Green = PIXEL8888GREEN(u32ColorKeyVal);
    colorKeyInfo.u8Blue = PIXEL8888BLUE(u32ColorKeyVal);

    //convertColorKeyByFmt(&colorKeyInfo);
    if (ioctl(g_fbFd, FBIOSET_COLORKEY, &colorKeyInfo) < 0) {
        ST_ERR("Error: failed to FBIOGET_COLORKEY");
        exit(0);
    }

    return MI_SUCCESS;
}

MI_S32 ST_Fb_InitMouse(MI_S32 s32MousePicW, MI_S32 s32MousePicH, MI_S32 s32BytePerPixel, MI_U8 *pu8MouseFile)
{
    MI_FB_CursorAttr_t stCursorAttr;
    FILE *fp = NULL;
    MI_U8 *pbuff = NULL;
    pbuff = malloc(s32BytePerPixel * s32MousePicW * s32MousePicW);
    if (!g_fbFd)
    {
        ST_ERR("Please init fb first.\n");
        return -1;
    }
    fp = fopen(pu8MouseFile, "rb");
    if (fp)
    {
        fread(pbuff, 1, s32BytePerPixel * s32MousePicW * s32MousePicW, fp);
        fclose(fp);
    }
    //set curosr Icon && set positon
    stCursorAttr.stCursorImageInfo.u32Width = s32MousePicW;
    stCursorAttr.stCursorImageInfo.u32Height = s32MousePicH;
    stCursorAttr.stCursorImageInfo.u32Pitch = s32MousePicW;
    stCursorAttr.stCursorImageInfo.eColorFmt = E_MI_FB_COLOR_FMT_ARGB1555;
    stCursorAttr.stCursorImageInfo.data = pbuff;
    stCursorAttr.u32HotSpotX = 18;
    stCursorAttr.u32HotSpotY = 9;
    stCursorAttr.u32XPos = 100;
    stCursorAttr.u32YPos = 100;
    stCursorAttr.stAlpha.bAlphaEnable = TRUE;
    stCursorAttr.stAlpha.u8Alpha0 = 0;
    stCursorAttr.stAlpha.u8Alpha1 = 0xff;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_ICON | E_MI_FB_CURSOR_ATTR_MASK_ALPHA
            | E_MI_FB_CURSOR_ATTR_MASK_SHOW | E_MI_FB_CURSOR_ATTR_MASK_POS;
    if (ioctl(g_fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
        ST_ERR("Error FBIOSET_CURSOR_ATTRIBUTE\n");
        exit(0);
    }

    if (pbuff)
    {
        free(pbuff);
    }

    return MI_SUCCESS;
}

MI_S32 ST_Fb_MouseSet(MI_U32 u32X, MI_U32 u32Y)
{
    MI_FB_CursorAttr_t stCursorAttr;

    stCursorAttr.u32XPos = u32X;
    stCursorAttr.u32YPos = u32Y;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_POS;
    if (ioctl(g_fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
        ST_ERR("Error FBIOSET_CURSOR_ATTRIBUTE\n");
        exit(0);
    }

    return MI_SUCCESS;
}

void ST_FB_Show(MI_BOOL bShow)
{
    if (g_fbFd < 0)
    {
        return;
    }

    if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(0);
    }

    if (ioctl(g_fbFd, FBIOSET_SHOW, &bShow)<0)
    {
        perror("Error: failed to FBIOSET_SHOW");
    }

    printf("%s fb\n", bShow ? "show" : "hide");
}

void ST_FB_SetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo)
{
    if (g_fbFd < 0)
    {
        return;
    }

    if (pstAlphaInfo == NULL)
    {
        return;
    }

    if (ioctl(g_fbFd, FBIOSET_GLOBAL_ALPHA, pstAlphaInfo) < 0)
    {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
    }
}

void ST_FB_GetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo)
{
    if (g_fbFd < 0)
    {
        return;
    }

    if (pstAlphaInfo == NULL)
    {
        return;
    }

    if (ioctl(g_fbFd, FBIOGET_GLOBAL_ALPHA, pstAlphaInfo) < 0)
    {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
    }
}

void ST_FB_ChangeResolution(int width, int height)
{
    if (g_fbFd < 0)
    {
        return;
    }

    printf("%s %d, width:%d,height:%d\n", __func__, __LINE__, width, height);
    MI_FB_DisplayLayerAttr_t stDisplayAttr;
    memset(&stDisplayAttr, 0, sizeof(MI_FB_DisplayLayerAttr_t));
    stDisplayAttr.u32SetAttrMask =  E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE |
        E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE | E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE
        | E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS;
    stDisplayAttr.u32Xpos = 0;
    stDisplayAttr.u32YPos = 0;
    stDisplayAttr.u32dstWidth = width;
    stDisplayAttr.u32dstHeight = height;
    stDisplayAttr.u32DisplayWidth = width;
    stDisplayAttr.u32DisplayHeight = height;
    stDisplayAttr.u32ScreenWidth = width;
    stDisplayAttr.u32ScreenHeight = height;
    //E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE operaton will change
    //var info and fix.line_lingth, so Need Retrive fixinfo and varinfo
    if (ioctl(g_fbFd, FBIOSET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOSET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }

    // get fb_fix_screeninfo
    if (ioctl(g_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(0);
    }
    // printFixedInfo();

    // get fb_var_screeninfo
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(0);
    }
    // printVariableInfo();

    // clear framebuffer
    //drawRect(0, 0, vinfo.xres, vinfo.yres, ARGB888_BLUE);
    // memset(frameBuffer, 0x0, finfo.smem_len);

    //Pandisplay
    if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(0);
    }
}

