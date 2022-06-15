
#include "jpeg.h"
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

    printf("%s %d %d %d %d\n", __FUNCTION__, __LINE__, logo->bmHeight, logo->bmWidth, logo->bmPitch);

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
        _load_bitmap_sl(fb, logo, 1);
        //fwrite(fb->bmBits, fb->bmPitch, 1, fp);
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
    //jpeg_calc_output_dimensions(cinfo);

    return 1;
}



