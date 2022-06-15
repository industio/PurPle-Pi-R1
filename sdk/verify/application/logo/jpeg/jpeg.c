

#include "jpeg.h"
#if defined(LOGO_WITH_JPEG)
#include <string.h>
#include <assert.h>
/*
 * compute image line size and bytes per pixel
 * from bits per pixel and width
 */
int bmp_ComputePitch(int bpp, MI_U32 width, MI_U32 *pitch, MI_BOOL does_round)
{
    int linesize;
    int bytespp = 1;

    if(bpp == 1)
        linesize = PIX2BYTES(width);
    else if(bpp <= 4)
        linesize = PIX2BYTES(width << 2);
    else if(bpp <= 8)
        linesize = width;
    else if(bpp <= 16) {
        linesize = width * 2;
        bytespp = 2;
    } else if(bpp <= 24) {
        linesize = width * 3;
        bytespp = 3;
    } else {
        linesize = width * 4;
        bytespp = 4;
    }

    /* rows are DWORD32 right aligned */
    if(does_round)
        *pitch = (linesize + 3) & -4;
    else
        *pitch = linesize;

    return bytespp;
}




/* Find the opaque pixel value corresponding to an RGB triple */
MI_U32 MapRGB(PixelFormat *format, MI_U8 r, MI_U8 g, MI_U8 b)
{
    MI_U32 pixv = (r >> format->Rloss) << format->Rshift
                  | (g >> format->Gloss) << format->Gshift
                  | (b >> format->Bloss) << format->Bshift
                  | format->Amask;
    return pixv;
}

/* Find the pixel value corresponding to an RGBA quadruple */
MI_U32 MapRGBA(PixelFormat *format, MI_U8 r, MI_U8 g, MI_U8 b, MI_U8 a)
{
    MI_U32 pixv = (r >> format->Rloss) << format->Rshift
                  | (g >> format->Gloss) << format->Gshift
                  | (b >> format->Bloss) << format->Bshift
                  | ((a >> format->Aloss) << format->Ashift & format->Amask);

    return pixv;

}

static inline MI_U8 *_mem_set_pixel(MI_U8 *dst, int bpp, MI_U32 pixel)
{
    switch(bpp) {
        case 1:
            *dst = pixel;
            break;

        case 2:
            *(MI_U16 *) dst = pixel;
            break;

        case 3:
            dst [0] = pixel;
            dst [1] = pixel >> 8;
            dst [2] = pixel >> 16;

            break;

        case 4:
            *(MI_U32 *) dst = pixel;
            break;
    }

    return dst + bpp;
}

/* This function compile a RGBA bitmap. */
void  CompileRGBABitmap(MI_U8 *dst_bits, MI_U32 dst_pitch,
                        const MI_U8 *src_bits, MI_U32 src_pitch,
                        MI_U32 w, MI_U32 h, PixelFormat *pixel_format, MI_U8 dst_bpp, MI_U8 src_bpp)
{
    MI_U32 x, y;
    MI_U8 *dst, *dst_line;
    const MI_U8 *src, *src_line;
    MI_U32 pixel;
    Color rgb;

    dst_line = dst_bits;
    src_line = src_bits;

    /* expand bits here. */
    for(y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;

        for(x = 0; x < w; x++) {
            rgb.r = *src++;
            rgb.g = *src++;
            rgb.b = *src++;

            if(src_bpp == 4) {
                rgb.a = *src;
                pixel = MapRGBA(pixel_format,
                                rgb.r, rgb.g, rgb.b, rgb.a);
                src++;
            } else {
                pixel = MapRGB(pixel_format, rgb.r, rgb.g, rgb.b);
            }

            dst = _mem_set_pixel(dst, dst_bpp, pixel);
        }

        src_line += src_pitch;
        dst_line += dst_pitch;
    }
}
#ifdef BYTE4
static void compilergba_bitmap_sl(HDC hdc, BYTE *bits, BYTE *alpha_mask, MYBITMAP *my_bmp)
{
    GAL_Color rgb;
    MI_U32 pixel;
    DWORD flags;
    PDC pdc;
    BYTE *src, *dst;
    int x = 0, w = 0;
    int bpp;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel(pdc->surface);
    dst = bits;
    src = my_bmp->bits;
    flags = my_bmp->flags;
    w = my_bmp->w;

    for(x = 0; x < w; x++) {
        if((flags & MYBMP_TYPE_MASK) == MYBMP_TYPE_BGR) {
            rgb.b = *src++;
            rgb.g = *src++;
            rgb.r = *src++;
        } else {
            rgb.r = *src++;
            rgb.g = *src++;
            rgb.b = *src++;
        }

        if(flags & MYBMP_RGBSIZE_4) {
            if(flags & MYBMP_ALPHA) {
                rgb.a = *src;

                if(alpha_mask) {
                    alpha_mask[x] = rgb.a;
                    pixel = GAL_MapRGB(pdc->surface->format,
                                       rgb.r, rgb.g, rgb.b);
                } else {
                    pixel = GAL_MapRGBA(pdc->surface->format,
                                        rgb.r, rgb.g, rgb.b, rgb.a);
                }
            } else {
                pixel = GAL_MapRGB(pdc->surface->format, rgb.r, rgb.g, rgb.b);
            }

            src++;
        } else {
            pixel = GAL_MapRGB(pdc->surface->format, rgb.r, rgb.g, rgb.b);
        }

        dst = _mem_set_pixel(dst, bpp, pixel);

    }

}
#endif

void _load_bitmap_sl(BITMAP *fb, BITMAP *logo)
{
    MI_U8 *src_bits, *dst_bits;
    src_bits = logo->bmBits;
    dst_bits = fb->bmBits;

    switch(logo->bmBytesPerPixel) {

        case 3:
            CompileRGBABitmap(dst_bits, fb->bmPitch,
                              src_bits, logo->bmPitch,
                              fb->bmWidth, 1, &fb->pxFmt, fb->bmBytesPerPixel, logo->bmBytesPerPixel);
            break;
#ifdef BYTE4

        case 4:
            compilergba_bitmap_sl(info->hdc, dst_bits, my_bmp);
            break;
#endif

        default: {
            printf("%s %d %d unsupport\n", __FUNCTION__, __LINE__, logo->bmBytesPerPixel);
            assert(0);
        }
        break;
    }
}



static void my_error_exit(j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

int load_JPEG_file(BITMAP *fb, BITMAP *logo, struct jpeg_decompress_struct *cinfo)
{

    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */
    logo->bmHeight = cinfo->output_height;
    logo->bmWidth = cinfo->output_width;
    logo->bmBytesPerPixel = bmp_ComputePitch(cinfo->output_components * 8, logo->bmWidth, &logo->bmPitch, TRUE);

    printf("%s %d %d %d %d\n", __FUNCTION__, __LINE__, cinfo->output_width, cinfo->output_components, logo->bmPitch);

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo->output_width * cinfo->output_components;
    logo->bmPitch = row_stride;
    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo->mem->alloc_sarray)
             ((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo->output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    //FILE *fp = fopen("./rawargb", "w+");

    while(cinfo->output_scanline < cinfo->output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void) jpeg_read_scanlines(cinfo, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        logo->bmBits = buffer[0];
        _load_bitmap_sl(fb, logo);
        //fwrite(fb->bmBits, fb->bmPitch, 1, fp);
        //memcpy(fb->bmBits, buffer[0], row_stride);
        fb->bmBits += fb->bmPitch;

    }

    //fclose(fp);
    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */

    printf("%s %d %d %d %d\n", __FUNCTION__, __LINE__, logo->bmHeight, logo->bmWidth, logo->bmPitch);
    return 1;
}



int init_JPEG_file(FILE *infile, BITMAP *logo, struct jpeg_decompress_struct *cinfo, struct my_error_mgr *jerr)
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    //struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */


    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */


    fseek(infile, 0, SEEK_SET);

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo->err = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if(setjmp(jerr->setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(cinfo);
        fclose(infile);
        return 0;
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */
    cinfo->out_color_space = JCS_RGB;
    cinfo->quantize_colors = FALSE;
/*
    if(cinfo->comp_info != NULL) {
        cinfo->comp_info[0].h_samp_factor = 2;
        cinfo->comp_info[0].v_samp_factor = 2;
        cinfo->comp_info[1].h_samp_factor = 1;
        cinfo->comp_info[1].v_samp_factor = 2;
        cinfo->comp_info[2].h_samp_factor = 1;
        cinfo->comp_info[2].v_samp_factor = 2;
        
        printf("%s %d\n", __FUNCTION__, __LINE__);
    } else {
        printf("%s %d\n", __FUNCTION__, __LINE__);
    }
*/
    jpeg_calc_output_dimensions(cinfo);

    return 1;
}
int load_logo_jpeg(char* frameBuffer,BITMAP* fb,FILE* fp)
{
    
    BITMAP logo;
    fb->bmBits = frameBuffer;
    struct jpeg_decompress_struct  cinfo;
    struct my_error_mgr jerr;
    init_JPEG_file(fp, &logo, &cinfo, &jerr);
    load_JPEG_file(fb, &logo, &cinfo);

}

#endif



