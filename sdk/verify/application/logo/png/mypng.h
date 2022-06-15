#ifndef __MYPNG_H__
#define __MYPNG_H__
#include <stdio.h>
#include "../logo.h"


#if defined(LOGO_WITH_PNG)
int load_logo_png(char* frameBuffer,BITMAP* fb,FILE* fp);
#else
static inline int load_logo_png(char* frameBuffer,BITMAP* fb,FILE* fp){return -1;}
#endif
#endif
