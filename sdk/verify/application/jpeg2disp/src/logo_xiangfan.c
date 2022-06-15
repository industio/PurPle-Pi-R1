#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SUPPORT_JPEG 1
#define SUPPORT_PNG 0
#include <mi_common.h>
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include "mstarFb.h"
#include "sstardisp.h"
#include "bmp.h"
#if SUPPORT_JPEG
#include "jpeg.h"
#endif
#if SUPPORT_PNG
#include "mypng.h"
#endif
#include <mi_gfx.h>
#include <mi_gfx_datatype.h>
#include "verify_gfx_type.h"
#include "verify_gfx.h"
#include "blitutil.h"


#define TEST_GFX    1


struct fb_var_screeninfo vinfo = {0};
struct fb_fix_screeninfo finfo = {0};
MI_FB_DisplayLayerAttr_t g_stLayerInfo = {0};

//Start of frame buffer mem
static char *frameBuffer = NULL;

#define LOGO_FILE_RAW "/sstar_configs/logo.raw"
#define LOGO_FILE_JPG "./logo.jpg"
#define LOGO_FILE_PNG "/sstar_configs/logo.png"

#define LOGO_FILE LOGO_FILE_JPG

#define LOGO_SUFFIX_RAW ".raw"
#define LOGO_SUFFIX_JPEG ".jpg"
#define LOGO_SUFFIX_PNG ".png"

#define LOG() printf("%s %d \n",__FUNCTION__,__LINE__);
#if (SUPPORT_JPEG||SUPPORT_PNG)
static void syncFormat(BITMAP *bmp, struct fb_var_screeninfo *vinfo)
{
    MI_U32 Rmask;
    MI_U32 Gmask;
    MI_U32 Bmask;
    MI_U32 Amask;
    int i;

    Rmask = 0;

    for(i = 0; i < vinfo->red.length; ++i)
    {
        Rmask <<= 1;
        Rmask |= (0x00000001 << vinfo->red.offset);
    }

    Gmask = 0;

    for(i = 0; i < vinfo->green.length; ++i)
    {
        Gmask <<= 1;
        Gmask |= (0x00000001 << vinfo->green.offset);
    }

    Bmask = 0;

    for(i = 0; i < vinfo->blue.length; ++i)
    {
        Bmask <<= 1;
        Bmask |= (0x00000001 << vinfo->blue.offset);
    }

    Amask = 0;

    for(i = 0; i < vinfo->transp.length; ++i)
    {
        Amask <<= 1;
        Amask |= (0x00000001 << vinfo->transp.offset);
    }

    CompileFormat(&bmp->pxFmt, bmp->bmBitsPerPixel, Rmask, Gmask, Bmask, Amask);
}
#endif
#include <sched.h>
#include <pthread.h>
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
FILE *fp;

void *readTOPlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    LOG();

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");

    LOG();

    fseek(fp, 0, SEEK_SET);
    size_t n = fread(frameBuffer, (MI_U32)pos, 1, fp);
    //printf("%s %d %ld %lx \n", __FUNCTION__, __LINE__, n,(MI_U32)pos);

}

void *readBTMlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(1, &set);
    LOG();

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");

    LOG();

    fseek(fp, (MI_U32)pos, SEEK_SET);
    size_t n = fread(frameBuffer + (MI_U32)pos, (MI_U32)pos, 1, fp);
    //printf("%s %d %ld %lx \n", __FUNCTION__, __LINE__, n,(MI_U32)pos);

}

#if (1 ==TEST_GFX)
int __create_src_surface_ARGB(MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;

    if((ret = _gfx_alloc_surface(srcSurf, data, "blendSrc")) < 0) {
        return ret;
    }

    Rect->s32Xpos = 0;
    Rect->s32Ypos = 0;
    Rect->u32Width = srcSurf->u32Width;
    Rect->u32Height = srcSurf->u32Height;
    color = 0xFF000000;
    MI_GFX_QuickFill(srcSurf, Rect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}
int __test_rotate_ARGB_Normal(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    //dstSurf.u32Height = srcSurf.u32Width;
    //dstSurf.u32Width = srcSurf.u32Height;

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitNormal(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_ARGB_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCW(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_ARGB_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;


    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitHVFlip(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_ARGB_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCCW(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}
int __test_rotate_YUV420SP_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitHVFlip(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.h;
    dstUV.w = srcUV.w;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitHVFlip(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

#endif
int main(int argc, char **argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    BITMAP logo;
    BITMAP fb;
    MI_DISP_PubAttr_t stDispPubAttr = {0};
    struct timeval tv1 = {0, 0};
    struct timeval tv2 = {0, 0};
#if (1 ==TEST_GFX)
    MI_GFX_Surface_t srcSurf = {0};
    MI_GFX_Rect_t srcRect = {0};
    char *srcData = NULL;
    MI_GFX_Surface_t dstSurf = {0};
    MI_GFX_Rect_t dstRect = {0};
    char *dstData = NULL;
    MI_U32 u32Color = 0;
    MI_U16 u16Fence = 0;
#endif

    memset(&finfo, 0, sizeof(struct fb_fix_screeninfo));
    memset(&vinfo, 0, sizeof(struct fb_var_screeninfo));
    memset(&g_stLayerInfo, 0, sizeof(MI_FB_DisplayLayerAttr_t));

    gettimeofday(&tv1, NULL);

    //gettimeofday(&tv2, NULL);
    //printf("%s %d %ld \n \n \n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);
#if (1 ==TEST_GFX)
    MI_GFX_Open();
#endif

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;

    //system("/config/riu_w 0x101e 0d 2000");

    LOG();
    sstar_disp_init(&stDispPubAttr);
    //exit(0);

    gettimeofday(&tv2, NULL);
    printf("%s %d %ld\n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);

    /* Open the file for reading and writing */
    fbFd = open(devfile, O_RDWR);

    if(fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    int show = 0;

    if(ioctl(fbFd, FBIOSET_SHOW, &show) < 0)
    {
        return (0);
    }

    //get fb_fix_screeninfo
    if(ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(2);
    }

    //get fb_var_screeninfo
    if(ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(3);
    }
    //get FBIOGET_DISPLAYLAYER_ATTRIBUTES
    if(ioctl(fbFd, FBIOGET_DISPLAYLAYER_ATTRIBUTES, &g_stLayerInfo) == -1)
    {
        perror("3Error reading variable information");
        exit(3);
    }
#if (1 ==TEST_GFX)
    switch(g_stLayerInfo.eFbColorFmt)
    {
        case E_MI_FB_COLOR_FMT_RGB565:
            srcSurf.eColorFmt =E_MI_GFX_FMT_BGR565;
            break;
        case E_MI_FB_COLOR_FMT_ARGB4444:
            srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB4444;
            break;
        case E_MI_FB_COLOR_FMT_ARGB8888:
            srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB8888;
            break;
        case E_MI_FB_COLOR_FMT_ARGB1555:
            srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB1555;
            break;
        case E_MI_FB_COLOR_FMT_YUV422:
            srcSurf.eColorFmt =E_MI_GFX_FMT_YUV422;
            break;
        case E_MI_FB_COLOR_FMT_I8:
            srcSurf.eColorFmt =E_MI_GFX_FMT_I8;
            break;
        case E_MI_FB_COLOR_FMT_I4:
            srcSurf.eColorFmt =E_MI_GFX_FMT_I4;
            break;
        case E_MI_FB_COLOR_FMT_I2:
            srcSurf.eColorFmt =E_MI_GFX_FMT_I2;
            break;
        default:
            srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB8888;
            break;
    }
    srcSurf.phyAddr = 0;
    srcSurf.u32Width = vinfo.yres;//vinfo.xres;//假设图片是600X1024,所以这里是反的，只是用来测试。
    srcSurf.u32Height = vinfo.xres;//vinfo.yres;
    srcSurf.u32Stride = srcSurf.u32Width * getBpp(srcSurf.eColorFmt);
    srcRect.s32Xpos = 0;
    srcRect.s32Ypos =  0;
    srcRect.u32Width = srcSurf.u32Width;
    srcRect.u32Height = srcSurf.u32Height;
    printf("srcSurf.u32Width:%d ,srcSurf.u32Height:%d ,srcSurf.eColorFmt:%d ,srcSurf.u32Stride:%d \n",srcSurf.u32Width,srcSurf.u32Height,srcSurf.eColorFmt,srcSurf.u32Stride);
    __create_src_surface_ARGB(&srcSurf, &srcRect, &srcData);

    fb.bmPhyAddr = srcSurf.phyAddr;
    fb.bmWidth = srcRect.u32Width;
    fb.bmHeight = srcRect.u32Height;
    fb.bmPitch = srcSurf.u32Stride;
    fb.bmBitsPerPixel =  vinfo.bits_per_pixel;
    fb.bmBytesPerPixel = vinfo.bits_per_pixel / 8;
    printf("xres: %d,yres: %d\n",fb.bmWidth,fb.bmHeight);
    printf("xres: %d,yres: %d,,finfo.line_length: %d,g_stLayerInfo.eFbColorFmt: %d \n",vinfo.xres,vinfo.yres,finfo.line_length,g_stLayerInfo.eFbColorFmt);
    printf("fb.bmWidth: %d,fb.bmHeight: %d,fb.bmPitch: %d,fb.bmBitsPerPixel: %d,fb.bmBytesPerPixel: %d\n",fb.bmWidth,fb.bmHeight,fb.bmPitch,fb.bmBitsPerPixel,fb.bmBytesPerPixel);

#if (SUPPORT_JPEG||SUPPORT_PNG)
    syncFormat(&fb, &vinfo);
#endif
#if SUPPORT_JPEG
    if(strstr(LOGO_FILE, LOGO_SUFFIX_JPEG) != NULL)   //2 //JPEG
    {
        if((fp = fopen(LOGO_FILE, "r")) == NULL)
        {
            fprintf(stderr, "can't open %s\n", LOGO_FILE);
            return 0;
        }

        fb.bmBits = srcData;
        struct jpeg_decompress_struct  cinfo;
        struct my_error_mgr jerr;

        init_JPEG_file(fp, &logo, &cinfo, &jerr);

        load_JPEG_file(&fb, &logo, &cinfo);
    }

#endif
#if SUPPORT_PNG
    if(strstr(LOGO_FILE, LOGO_SUFFIX_PNG) != NULL)   //2 //PNG
    {
        if((fp = fopen(LOGO_FILE, "r")) == NULL)
        {
            fprintf(stderr, "can't open %s\n", LOGO_FILE);
            return 0;
        }

        fb.bmBits = frameBuffer;
        load_PNG_file(fp, &fb);
    }
#endif
    screensize = finfo.smem_len;
    frameBuffer = (char *) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);

    if(frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        exit(4);
    }
    dstSurf.eColorFmt = srcSurf.eColorFmt;
    dstSurf.phyAddr = finfo.smem_start;
    dstSurf.u32Width = vinfo.xres;
    dstSurf.u32Height = vinfo.yres;
    dstSurf.u32Stride = dstSurf.u32Width * getBpp(dstSurf.eColorFmt);
    dstRect.s32Xpos = 0;
    dstRect.s32Ypos =  0;
    dstRect.u32Width = dstSurf.u32Width;
    dstRect.u32Height = dstSurf.u32Height;
    dstData = frameBuffer;
    printf("dstSurf.u32Width:%d ,dstSurf.u32Height:%d ,dstSurf.eColorFmt:%d ,dstSurf.u32Stride:%d,dstRect.s32Xpos:%d,dstRect.s32Ypos:%d,dstRect.u32Width:%d,dstRect.u32Height:%d\n",
        dstSurf.u32Width,dstSurf.u32Height,dstSurf.eColorFmt,dstSurf.u32Stride,dstRect.s32Xpos,dstRect.s32Ypos,dstRect.u32Width,dstRect.u32Height);
    //__test_rotate_ARGB_90(srcSurf, srcRect, dstSurf, dstRect, dstData, FALSE);
    __test_rotate_ARGB_180(srcSurf, srcRect, dstSurf, dstRect, dstData, FALSE);

#else
    fb.bmPhyAddr = finfo.smem_start;
    fb.bmHeight = vinfo.yres;
    fb.bmWidth = vinfo.xres;
    fb.bmPitch = finfo.line_length;
    fb.bmBitsPerPixel =  vinfo.bits_per_pixel;
    fb.bmBytesPerPixel = vinfo.bits_per_pixel / 8;

    printf("fb.bmWidth: %d,fb.bmHeights: %d\n",fb.bmWidth,fb.bmHeight);
    printf("xres: %d,yres: %d,line_length: %d,eFbColorFmt: %d \n",vinfo.xres,vinfo.yres,finfo.line_length,g_stLayerInfo.eFbColorFmt);

#if (SUPPORT_JPEG||SUPPORT_PNG)
    syncFormat(&fb, &vinfo);
#endif

    screensize = finfo.smem_len;
    frameBuffer = (char *) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);

    if(frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        exit(4);
    }

#if SUPPORT_JPEG

    if(strstr(LOGO_FILE, LOGO_SUFFIX_JPEG) != NULL)   //2 //JPEG
    {
        if((fp = fopen(LOGO_FILE, "r")) == NULL)
        {
            fprintf(stderr, "can't open %s\n", LOGO_FILE);
            return 0;
        }

        fb.bmBits = frameBuffer;
        struct jpeg_decompress_struct  cinfo;
        struct my_error_mgr jerr;

        init_JPEG_file(fp, &logo, &cinfo, &jerr);

        load_JPEG_file(&fb, &logo, &cinfo);
    }

#endif
#if SUPPORT_PNG
    if(strstr(LOGO_FILE, LOGO_SUFFIX_PNG) != NULL)   //2 //PNG
    {
        if((fp = fopen(LOGO_FILE, "r")) == NULL)
        {
            fprintf(stderr, "can't open %s\n", LOGO_FILE);
            return 0;
        }

        fb.bmBits = frameBuffer;
        load_PNG_file(fp, &fb);
    }
#endif
#endif

SHOW:

    //Pandisplay
    if(ioctl(fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(5);
    }

/*
    while(1)
    {
	sleep(1);
    }
*/
    gettimeofday(&tv2, NULL);
    printf("%s %d %ld\n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);
#if (1 ==TEST_GFX)
    _gfx_free_surface(&srcSurf, srcData);
    MI_GFX_Close();
#endif
    /********************************************** for qt, don't deinit ****************************/
    sstar_disp_Deinit(&stDispPubAttr);
    //unmap buffer
    munmap(frameBuffer, screensize);
    fclose(fp);
    close(fbFd);
    return 0;
}
