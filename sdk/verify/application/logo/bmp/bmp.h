
#ifndef __BMP_H__
#define __BMP_H__
#include <stdio.h>
#include "../logo.h"


#if defined(LOGO_WITH_BMP)
int load_logo_bmp(char* frameBuffer,BITMAP* fb,FILE* fp);
#else
static inline int load_logo_bmp(char* frameBuffer,BITMAP* fb,FILE* fp){return -1;}
#endif
#endif

