/*******************************************************************************
FILE : texturemap.h

LAST MODIFIED : 15 September 1997

DESCRIPTION :
==============================================================================*/
#if !defined (TEXTUREMAP_H)
#define TEXTUREMAP_H

#include "finite_element/finite_element.h"
#include "graphics/graphics_window.h"

/*
Global types
------------
*/
struct Image_buffer
/*******************************************************************************
LAST MODIFIED : 7 April 1997

DESCRIPTION :
==============================================================================*/
{
	short xsize,ysize,zsize;
	short **rbuf,**gbuf,**bbuf,**abuf;	
}; /* struct Image_buffer */

/*
Global functions
----------------
*/
int generate_textureimage_from_FE_element(
	struct Graphics_window *graphics_window,char *infile,char *outfile, 
	struct FE_element *element,double ximax[3],
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 15 September 1997

DESCRIPTION :
Creates texture map segment (range[0,ximax[3]]) SGI rgb image <out_image> from
<in_image> by interpolating and projecting FE element surface onto normalized
image space.
==============================================================================*/
#endif /* !defined (TEXTUREMAP_H) */
