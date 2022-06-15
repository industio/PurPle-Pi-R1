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

#define SUPPORT_JPEG 0
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

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

//Start of frame buffer mem
static char *frameBuffer = NULL;

#define LOGO_FILE_RAW "/sstar_configs/logo.raw"
#define LOGO_FILE_JPG "./logo.jpg"
#define LOGO_FILE_PNG "/sstar_configs/logo.png"

#define LOGO_FILE LOGO_FILE_JPG

#define LOGO_SUFFIX_RAW ".raw"
#define LOGO_SUFFIX_JPEG ".jpg"
#define LOGO_SUFFIX_PNG ".png"

#define LOG() //printf("%s %d \n",__FUNCTION__,__LINE__);
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


int main(int argc, char **argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    BITMAP logo;
    BITMAP fb;
    MI_DISP_PubAttr_t stDispPubAttr;
    struct timeval tv1 = {0, 0};
    struct timeval tv2 = {0, 0};

    gettimeofday(&tv1, NULL);

    //gettimeofday(&tv2, NULL);
    //printf("%s %d %ld \n \n \n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;

    //system("/config/riu_w 0x101e 0d 2000");

    LOG();
    sstar_disp_init(&stDispPubAttr);

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
    
    fb.bmPhyAddr = finfo.smem_start;
    fb.bmHeight = vinfo.yres;
    fb.bmWidth = vinfo.xres;
    fb.bmPitch = finfo.line_length;
    fb.bmBitsPerPixel =  vinfo.bits_per_pixel;
    fb.bmBytesPerPixel = vinfo.bits_per_pixel / 8;

    printf("xres: %d,yres: %d\n",fb.bmWidth,fb.bmHeight);

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

SHOW:

    //Pandisplay
    if(ioctl(fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(5);
    }

    gettimeofday(&tv2, NULL);
    printf("%s %d %ld\n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);
    while(1);
    //unmap buffer
    munmap(frameBuffer, screensize);

    fclose(fp);
    close(fbFd);
    return 0;
}
