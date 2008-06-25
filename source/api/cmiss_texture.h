/*******************************************************************************
FILE : cmiss_texture.h

LAST MODIFIED : 23 June 2008

DESCRIPTION :
The public interface to the Cmiss_texture object for rendering cmiss
scenes.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Shane Blackett shane at blackett.co.nz
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef __CMISS_TEXTURE_H__
#define __CMISS_TEXTURE_H__

#include "general/object.h"

/*
Global types
------------
*/

enum Cmiss_texture_combine_mode
/*******************************************************************************
LAST MODIFIED : 17 October 2007

DESCRIPTION :
Describes the blending of the texture with the texture constant colour and
the underlying fragment colour
==============================================================================*/
{
	CMISS_TEXTURE_COMBINE_BLEND,
	CMISS_TEXTURE_COMBINE_DECAL,
	CMISS_TEXTURE_COMBINE_MODULATE,
	CMISS_TEXTURE_COMBINE_ADD,
	CMISS_TEXTURE_COMBINE_ADD_SIGNED, /* Add the value and subtract 0.5 so the texture value
								 effectively ranges from -0.5 to 0.5 */
	CMISS_TEXTURE_COMBINE_MODULATE_SCALE_4, /* Multiply and then scale by 4, so that we can
										 scale down or up */
	CMISS_TEXTURE_COMBINE_BLEND_SCALE_4, /* Same as blend with a 4 * scaling */
	CMISS_TEXTURE_COMBINE_SUBTRACT,
	CMISS_TEXTURE_COMBINE_ADD_SCALE_4,
	CMISS_TEXTURE_COMBINE_SUBTRACT_SCALE_4,
	CMISS_TEXTURE_COMBINE_INVERT_ADD_SCALE_4,
	CMISS_TEXTURE_COMBINE_INVERT_SUBTRACT_SCALE_4
};

enum Cmiss_texture_compression_mode
/*******************************************************************************
LAST MODIFIED : 25 May 2007

DESCRIPTION :
Whether the texture is compressed.  Could add specific compression formats that
are explictly requested from the hardware.
==============================================================================*/
{
	CMISS_TEXTURE_COMPRESSION_UNCOMPRESSED,
	CMISS_TEXTURE_COMPRESSION_COMPRESSED_UNSPECIFIED /* Allow the hardware to choose the compression */
}; /* enum Texture_compression_mode */

enum Cmiss_texture_filter_mode
/*******************************************************************************
LAST MODIFIED : 28 March 2008

DESCRIPTION :
Specfiy how the graphics hardware rasterises the texture onto the screen.
==============================================================================*/
{
	CMISS_TEXTURE_FILTER_NEAREST,
	CMISS_TEXTURE_FILTER_LINEAR,
	CMISS_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	CMISS_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	CMISS_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
}; /* enum Cmiss_texture_filter_mode */

typedef struct Cmiss_texture *Cmiss_texture_id;

#define Cmiss_texture_manager manager_Cmiss_texture

struct Cmiss_texture_manager;

struct IO_stream_package;

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Returns the texture named name from the manager if it is defined.
 */
Cmiss_texture_id Cmiss_texture_manager_get_texture(
	struct Cmiss_texture_manager *manager, const char *name);

Cmiss_texture_id Cmiss_texture_manager_create_texture_from_file(
	struct Cmiss_texture_manager *manager, const char *name,
	struct IO_stream_package *io_stream_package, const char *filename);
/*******************************************************************************
LAST MODIFIED : 28 June 2007

DESCRIPTION :
Creates a texture <name> in <manager> if it can be read from <filename>.
==============================================================================*/

Cmiss_texture_id Cmiss_texture_manager_create_texture_from_file_with_parameters(
	struct Cmiss_texture_manager *manager, const char *name,
	struct IO_stream_package *io_stream_package, const char *filename,
	int specify_width, int specify_height,
	int specify_depth, int specify_format,
	int specify_number_of_bytes_per_component);
/*******************************************************************************
LAST MODIFIED : 28 June 2007

DESCRIPTION :
Creates a texture <name> in <manager> if it can be read from <filename>.
The extra parameters are supplied to the image reader and will only be 
used if the image format specified in <filename> does not override them.
They will normally only be used for formats such as .gray or .rgb which
only contain raw data. The <specify_format> values are defined in
Texture.idl.  Supplying zero for any of these parameters ensures that 
particular parameter will be ignored.
==============================================================================*/

int Cmiss_texture_get_graphics_storage_size(Cmiss_texture_id texture);
/*******************************************************************************
LAST MODIFIED : 25 May 2007

DESCRIPTION :
Returns the amount of graphics memory used to store the texture.
If the texture is compressed then this parameter will be requested directly
from the graphics card, so if it hasn't been rendered yet will be undefined.
If the texture is not compressed then it is calculated from the texture
parameters.
==============================================================================*/

enum Cmiss_texture_combine_mode Cmiss_texture_get_combine_mode(
   Cmiss_texture_id texture);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Returns how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

int Cmiss_texture_set_combine_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_combine_mode combine_mode);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Sets how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

enum Cmiss_texture_compression_mode Cmiss_texture_get_compression_mode(
   Cmiss_texture_id texture);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
==============================================================================*/

int Cmiss_texture_set_compression_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_compression_mode compression_mode);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Indicate to the graphics hardware how you would like the texture stored in
graphics memory.
==============================================================================*/

enum Cmiss_texture_filter_mode Cmiss_texture_get_filter_mode(
   Cmiss_texture_id texture);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
==============================================================================*/

int Cmiss_texture_set_filter_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_filter_mode filter_mode);
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Specfiy how the graphics hardware rasterises the texture onto the screen.
==============================================================================*/

int Cmiss_texture_get_original_texel_sizes(Cmiss_texture_id texture, 
   unsigned int *dimension, unsigned int **original_texel_sizes);
/*******************************************************************************
LAST MODIFIED : 25 May 2007

DESCRIPTION :
Returns the original texel sizes of the texture.  These may have been
subsequently modified by cmgui such as to support platforms which require
each size to be a power of two.
==============================================================================*/

int Cmiss_texture_get_rendered_texel_sizes(Cmiss_texture_id texture,
	unsigned int *dimension, unsigned int **sizes);
/*******************************************************************************
LAST MODIFIED : 29 June 2007

DESCRIPTION :
Returns the rendered texel sizes of the texture.  These indicate what sizes
were actually loaded into OpenGL and until the texture is rendered will be
zero.
==============================================================================*/
int Cmiss_texture_get_texture_coordinate_sizes(Cmiss_texture_id texture, 
   unsigned int *dimension, double **texture_coordinate_sizes);
/*******************************************************************************
LAST MODIFIED : 25 May 2007

DESCRIPTION :
Returns the texture coordinates sizes of the texture.  When rendered the
texture will be rendered mapping the texture coordinates [0,0,0] to the bottom
left of the texture and
[textureCoordinateWidth, textureCoordinateHeight, textureCoordinateDepth] to
the top right of the texture.
==============================================================================*/

int Cmiss_texture_set_texture_coordinate_sizes(Cmiss_texture_id texture, 
   unsigned int dimension, double *texture_coordinate_sizes);
/*******************************************************************************
LAST MODIFIED : 25 May 2007

DESCRIPTION :
Returns the texture coordinates sizes of the texture.  When rendered the
texture will be rendered mapping the texture coordinates [0,0,0] to the bottom
left of the texture and
[textureCoordinateWidth, textureCoordinateHeight, textureCoordinateDepth] to
the top right of the texture.
==============================================================================*/

/*****************************************************************************//**
@date LAST MODIFIED : 1 August 2007

Gets the number of components used per texel in the texture.

@param texture The texture object. 
@return Number of components used per texel, 1, 2, 3 or 4.
*//*==============================================================================*/
int Cmiss_texture_get_number_of_components(Cmiss_texture_id texture);

int Cmiss_texture_get_number_of_bytes_per_component(Cmiss_texture_id texture);
/*******************************************************************************
LAST MODIFIED : 5 October 2007

DESCRIPTION :
Returns the number of bytes used per components per texel in the texture:
1 = 8 bit image, 2 = 16 bit image.
==============================================================================*/

int Cmiss_texture_write_to_file(Cmiss_texture_id texture, 
   const char *filename);
/*******************************************************************************
LAST MODIFIED : 27 June 2007

DESCRIPTION :
Writes the <texture> to file <filename>.
I think it is best to write a separate function if you want to write a 
3D texture to a file sequence rather than handle it with this function.
==============================================================================*/

int Cmiss_texture_set_pixels(Cmiss_texture_id texture,
	int width, int height, int depth,
	int number_of_components, int number_of_bytes_per_component,
	int source_width_bytes, unsigned char *source_pixels);
/*******************************************************************************
LAST MODIFIED : 4 December 2007

DESCRIPTION :
Sets the texture data to the specified <width>, <height> and <depth>, with
<number_of_components> where 1=luminance, 2=LuminanceA, 3=RGB, 4=RGBA, and
<number_of_bytes_per_component> which may be 1 or 2.
Data for the image is taken from <source_pixels> which has <source_width_bytes>
of at least <width>*<number_of_components>*<number_of_bytes_per_component>.
The source_pixels are stored in rows from the bottom to top and from left to
right in each row. Pixel colours are interleaved, eg. RGBARGBARGBA...
==============================================================================*/

int Cmiss_texture_get_pixels(Cmiss_texture_id texture,
	unsigned int left, unsigned int bottom, unsigned int depth_start,
	int width, int height, int depth,
	unsigned int padded_width_bytes, unsigned int number_of_fill_bytes, 
	unsigned char *fill_bytes,
	int components, unsigned char *destination_pixels);
/*******************************************************************************
LAST MODIFIED : 4 December 2007

DESCRIPTION :
Fills <destination_pixels> with all or part of <texture>.
The <left>, <bottom>, <depth_start> and  <width>, <height>, <depth> specify the part of <cmgui_image> output and must be wholly within its bounds.
Image data is ordered from the bottom row to the top, and within each row from
the left to the right, and from the front to back.
If <components> is > 0, the specified components are output at each pixel, 
otherwise all the number_of_components components of the image are output at each pixel.
Pixel values relate to components by:
  1 -> I    = Intensity;
  2 -> IA   = Intensity Alpha;
  3 -> RGB  = Red Green Blue;
  4 -> RGBA = Red Green Blue Alpha;
  5 -> BGR  = Blue Green Red

If <padded_width_bytes> is zero, image data for subsequent rows follows exactly
after the right-most pixel of the row below. If a positive number is specified,
which must be greater than <width>*number_of_components*
number_of_bytes_per_component in <cmgui_image>, each
row of the output image will take up the specified number of bytes, with
pixels beyond the extracted image <width> undefined.
If <number_of_fill_bytes> is positive, the <fill_bytes> are repeatedly output
to fill the padded row; the cycle of outputting <fill_bytes> starts at the
left of the image to make a more consitent output if more than one colour is
specified in them -- it makes no difference if <number_of_fill_bytes> is 1 or
equal to the number_of_components.
<destination_pixels> must be large enough to take the greater of
<depth> *(<padded_width_bytes> or <width>)*
<height>*number_of_components*number_of_bytes_per_component in the image.
==============================================================================*/

int Cmiss_texture_set_property(Cmiss_texture_id texture,
	const char *property, const char *value);
/*******************************************************************************
LAST MODIFIED : 25 October 2007

DESCRIPTION :
Sets the <property> and <value> string for the <texture>.
==============================================================================*/

char *Cmiss_texture_get_property(Cmiss_texture_id texture,
	const char *property);
/*******************************************************************************
LAST MODIFIED : 25 October 2007

DESCRIPTION :
If the <property> is defined for the <texture>, then a copy is returned (and
should be DEALLOCATED when finished with).
==============================================================================*/

#endif /* __CMISS_TEXTURE_H__ */
