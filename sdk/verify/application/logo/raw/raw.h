#ifndef __RAW_H__
#define __RAW_H__

#include<stdio.h> 
#include "../logo.h"

#if defined(LOGO_WITH_RAW)
int load_logo_raw(char* frameBuffer,BITMAP* fb,FILE* fp);
#else
static inline int load_logo_raw(char* frameBuffer,BITMAP* fb,FILE* fp){return -1;}
#endif

#endif

