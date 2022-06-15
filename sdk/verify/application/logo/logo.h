#ifndef __LOGO_H__
#define __LOGO_H__
#include "config.h"

#include <mi_common.h>



typedef struct _Color {
    /**
     * The red component of a RGBA quarter.
     */
    MI_U8 r;
    /**
     * The green component of a RGBA quarter.
     */
    MI_U8 g;
    /**
     * The blue component of a RGBA quarter.
     */
    MI_U8 b;
    /**
     * The alpha component of a RGBA quarter.
     */
    MI_U8 a;
} Color;

typedef struct PixelFormat {
    MI_U8  Rloss;
    MI_U8  Gloss;
    MI_U8  Bloss;
    MI_U8  Aloss;
    MI_U8  Rshift;
    MI_U8  Gshift;
    MI_U8  Bshift;
    MI_U8  Ashift;
    MI_U32 Rmask;
    MI_U32 Gmask;
    MI_U32 Bmask;
    MI_U32 Amask;

    /* Alpha value information (per-surface alpha) */
    MI_U8 alpha;
} PixelFormat;


/** Expanded device-dependent bitmap structure. */
struct _BITMAP {
    MI_U32  bmWidth;
    /** The height of the bitmap */
    MI_U32  bmHeight;
    /** The pitch of the bitmap */
    MI_U32  bmPitch;
    /** The bits of the bitmap */
    char  *bmBits;
    /** The bytes per piexel. */
    MI_U8   bmBytesPerPixel;

    MI_U8   bmBitsPerPixel;

    MI_U32  bmPhyAddr;

    PixelFormat pxFmt;
};
typedef struct _BITMAP BITMAP;

#define PIX2BYTES(n)    (((n)+7)/8)

#endif

