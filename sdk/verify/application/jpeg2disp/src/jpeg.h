#ifndef __JPEG_H__
#define __JPEG_H__
#include<stdio.h> 
#include <jerror.h>
#include "jpeglib.h"
#include <setjmp.h>
#include "bmp.h"

struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */

    jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

int load_JPEG_file(BITMAP *fb, BITMAP *logo, struct jpeg_decompress_struct *cinfo);
int init_JPEG_file(FILE *infile, BITMAP *logo, struct jpeg_decompress_struct *cinfo, struct my_error_mgr *jerr);
#endif
