
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mi_sys.h>
#include <mi_sys_datatype.h>


#include "verify_gfx.h"

unsigned int getBpp(MI_GFX_ColorFmt_e eFmt)
{
    switch(eFmt) {
        case E_MI_GFX_FMT_I8:
            return 1;

        case E_MI_GFX_FMT_RGB565:
        case E_MI_GFX_FMT_ARGB1555:
        case E_MI_GFX_FMT_ARGB4444:
            return 2;

        case E_MI_GFX_FMT_ARGB8888:
            return 4;

        default:
            return -1;
    }
}

int _gfx_alloc_surface(MI_GFX_Surface_t *pSurf, char **data, char  *name)
{
    FILE *fp = NULL;
    char surfName[128] = {0};
    snprintf(surfName, sizeof(surfName), "#%s", name);

    if(MI_SUCCESS != MI_SYS_MMA_Alloc(surfName,
                                      pSurf->u32Height * pSurf->u32Stride, &pSurf->phyAddr)) {
        printf("MI_SYS_MMA_Alloc fail\n");
        return -1;
    }

    if(MI_SUCCESS != MI_SYS_Mmap(pSurf->phyAddr,
                                 pSurf->u32Height * pSurf->u32Stride, (void **)data, FALSE)) {
        printf("MI_SYS_Mmap fail\n");
        return -1;
    }

    memset(*data, 0, pSurf->u32Height * pSurf->u32Stride);
    return 0;
}
void _gfx_free_surface(MI_GFX_Surface_t *pSurf, char *data)
{
    MI_SYS_Munmap(data, pSurf->u32Height * pSurf->u32Stride);
    MI_SYS_MMA_Free(pSurf->phyAddr);
}
void _gfx_sink_surface(MI_GFX_Surface_t *pSurf,const char *data,const char  *name)
{
    FILE *fp = NULL;
    char sinkName[128] = {0};
    snprintf(sinkName, sizeof(sinkName), "%s_%dx%d.raw", name, pSurf->u32Width, pSurf->u32Height);
    fp = fopen(sinkName, "w+");

    if(fp == NULL) {
        fprintf(stderr, "fp == NULL\n");
    } else {
        const char *p = data;
        long n = pSurf->u32Height * pSurf->u32Stride;

        do {
            long n0 = fwrite(p, 1, n, fp);
            n = n -n0;
            p = data+n0;
        } while(n > 0);

        fclose(fp);
    }

}

