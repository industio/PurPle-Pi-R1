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
#ifndef _FB_DEFS_H_
#define _FB_DEFS_H_

#include <stdio.h>

#define HAS_EX  1
#define HAS_GE  0
#define HAS_PD  1

#define LAYERCOUNT 32
#define CURSORLAYER 31
#define MAINLAYER 0


#define PRINTINFO 1
#if PRINTINFO
#define TRACER(f, ...) printf(f, ##__VA_ARGS__)
#define TERROR(f, ...) perror(f, ##__VA_ARGS__)
#else
#define TRACER(f, ...)
#define TERROR(f, ...)
#endif
#define DEBUG(op) TRACER("%s at %s %d\n", op, __FUNCTION__, __LINE__)
#define GUARD() DEBUG("GUARD");
#define FATALEXIT(code) do { DEBUG("FATAL"); } while(0)
#define ASSERT(exp) do { if (!(exp)) FATALEXIT(-1);  } while(0)
#define ASSERT_THROW(exp, code) do { if (!(exp)) { DEBUG("FAILE"); return code; } } while(0)

#define CHECK_IF(exp, dst, ret) if ((exp) == (dst)) { return ret; }
#define CHECK_IFNOT(exp, dst, ret) if ((exp) != (dst)) { return ret; }
#define CHECK_LOWER(exp, dst, ret) if ((exp) < (dst)) { return ret; }
#define CHECK_UPPER(exp, dst, ret) if ((exp) > (dst)) { return ret; }
#define CHECK_NOTLOWER(exp, dst, ret) if ((exp) >= (dst)) { return ret; }
#define CHECK_NOTUPPER(exp, dst, ret) if ((exp) <= (dst)) { return ret; }

#define DELETE_IF(P) if (P) { delete (P); (P) = nullptr; }

#define MIN(a, b) (a) > (b) ? (b) : (a)
#define MAX(a, b) (a) < (b) ? (b) : (a)

#define DONOTHING() (0)
#define Delay(usec) usleep(usec * 1000)
#define E_FB_NODEV -1
#define E_FB_INVAL -2
#define E_FB_NOMEM -3
#endif
