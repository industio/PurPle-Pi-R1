#ifndef _ST_FB_H
#define _ST_FB_H

#include "mstarFb.h"

#define RGB2PIXEL1555(r,g,b)	\
	((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3) | 0x8000)

#if 0
#define PIXEL1555BLUE(pixelval)		((pixelval) & 0x1f)
#define PIXEL1555GREEN(pixelval)	(((pixelval) >> 5) & 0x1f)
#define PIXEL1555RED(pixelval)		(((pixelval) >> 10) & 0x1f)
#endif
#define PIXEL1555RED(pixelval)		(((pixelval) >> 7) & 0xf8)
#define PIXEL1555GREEN(pixelval)		(((pixelval) >> 2) & 0xf8)
#define PIXEL1555BLUE(pixelval)			(((pixelval) << 3) & 0xf8)

#define ARGB1555_BLACK  RGB2PIXEL1555(0,0,0)
#define ARGB1555_RED    RGB2PIXEL1555(255,0,0)


#define ARGB2PIXEL8888(a,r,g,b)	\
	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define PIXEL8888ALPHA(pixelval)	(((pixelval) >> 24) & 0xff)
#define PIXEL8888RED(pixelval)  	(((pixelval) >> 16) & 0xff)
#define PIXEL8888GREEN(pixelval)	(((pixelval) >> 8) & 0xff)
#define PIXEL8888BLUE(pixelval) 	((pixelval) & 0xff)

#define ARGB888_BLACK   ARGB2PIXEL8888(128,0,0,0)
#define ARGB888_RED     ARGB2PIXEL8888(128,255,0,0)
#define ARGB888_GREEN   ARGB2PIXEL8888(128,0,255,0)
#define ARGB888_BLUE    ARGB2PIXEL8888(128,0,0,255)

MI_S32 ST_Fb_Init();
MI_S32 ST_Fb_SetColorKey();
MI_S32 ST_Fb_FillRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal);
MI_S32 ST_Fb_DrawRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal);
MI_S32 ST_Fb_FillPoint(MI_U16 x, MI_U16 y, MI_U32 u32ColorVal);
MI_S32 ST_Fb_FillLine(MI_U16 x0, MI_U16 y0, MI_U16 x1, MI_U16 y1, MI_U32 u32ColorVal);
MI_S32 ST_Fb_FillCircle(MI_U16 x, MI_U16 y, MI_U16 r, MI_U32 u32ColorVal);
MI_S32 ST_Fb_DeInit();
MI_S32 ST_Fb_GetColorKey(MI_U32 *pu32ColorKeyVal);
MI_S32 ST_Fb_InitMouse(MI_S32 s32MousePicW, MI_S32 s32MousePicH, MI_S32 s32BytePerPixel, MI_U8 *pu8MouseFile);
MI_S32 ST_Fb_MouseSet(MI_U32 u32X, MI_U32 u32Y);
void ST_FB_Show(MI_BOOL bShown);
void ST_FB_GetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo);
void ST_FB_SetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo);
void ST_FB_ChangeResolution(int width, int height);
void ST_Fb_SetColorFmt(MI_FB_ColorFmt_e enFormat);

#endif//_ST_FB_H
