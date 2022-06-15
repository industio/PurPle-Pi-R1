#include <mi_gfx.h>
#include <mi_gfx_datatype.h>

#include <mi_sys.h>


// get bytesPerPixel each format
unsigned int getBpp(MI_GFX_ColorFmt_e eFmt);
int _gfx_alloc_surface(MI_GFX_Surface_t *pSurf, char **data,char  *surfName);
void _gfx_free_surface(MI_GFX_Surface_t *pSurf, char *data);
void _gfx_sink_surface(MI_GFX_Surface_t *pSurf, const char *data,const char  *name);
