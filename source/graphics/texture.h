/*******************************************************************************
FILE : texture.h

LAST MODIFIED : 15 March 2002

DESCRIPTION :
The data structures used for representing textures.
==============================================================================*/
#if !defined (TEXTURE_H)
#define TEXTURE_H

#include <stdio.h>
#include "general/enumerator.h"
#include "general/image_utilities.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"

/*
Global types
------------
*/

enum Texture_combine_mode
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
How the texture is combined with the material.
==============================================================================*/
{
	TEXTURE_BLEND,
	TEXTURE_DECAL,
	TEXTURE_MODULATE
}; /* enum Texture_combine_mode */

enum Texture_compression_mode
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Whether the texture is compressed.  Could add specific compression formats that
are explictly requested from the hardware.
==============================================================================*/
{
	TEXTURE_UNCOMPRESSED,
	TEXTURE_COMPRESSED_UNSPECIFIED /* Allow the hardware to choose the compression */
}; /* enum Texture_compression_mode */

enum Texture_filter_mode
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
What happens to when the screen and texture are at different resolutions.
==============================================================================*/
{
	TEXTURE_LINEAR_FILTER,
	TEXTURE_NEAREST_FILTER
}; /* enum Texture_filter_mode */

enum Texture_resize_filter_mode
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Controls how a texture is downsampled to fit texture hardware.
==============================================================================*/
{
	TEXTURE_RESIZE_LINEAR_FILTER,  /* high quality, but slow */
	TEXTURE_RESIZE_NEAREST_FILTER  /* fast but low quality */
}; /* enum Texture_resize_filter_mode */

enum Texture_wrap_mode
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
What happens to a texture when the texture coordinates are outside [0,1].
==============================================================================*/
{
	TEXTURE_CLAMP_WRAP,
	TEXTURE_REPEAT_WRAP
}; /* enum Texture_wrap_mode */

enum Texture_row_order 
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_TOP_TO_BOTTOM,
	TEXTURE_BOTTOM_TO_TOP
}; /* enum Texture_row_order */

struct Texture;
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
The contents of struct Texture are private.
==============================================================================*/

DECLARE_LIST_TYPES(Texture);

DECLARE_MANAGER_TYPES(Texture);

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_combine_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_compression_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_filter_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_resize_filter_mode);

int Texture_storage_type_is_user_selectable(
	enum Texture_storage_type storage, void *dummy);
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns true if <storage> is one of the basic storage types that users can
select, ie. not a digital media buffer.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_storage_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Texture_wrap_mode);

struct Texture *CREATE(Texture)(char *name);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a texture.  Adds the texture to the list
of all textures.
???DB.  Trimming name ?
???DB.  Check if it already exists ?  Retrieve if it does already exist ?
==============================================================================*/

int DESTROY(Texture)(struct Texture **texture_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the texture and sets <*texture_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Texture);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Texture);

PROTOTYPE_LIST_FUNCTIONS(Texture);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Texture,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Texture,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Texture);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Texture,name,char *);

int Texture_storage_type_get_number_of_components(
	enum Texture_storage_type storage);
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the number of components used per texel for <storage> type.
==============================================================================*/

int Texture_get_combine_alpha(struct Texture *texture,float *alpha);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the alpha value used for combining the texture.
==============================================================================*/

int Texture_set_combine_alpha(struct Texture *texture,float alpha);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the alpha value used for combining the texture.
==============================================================================*/

int Texture_get_combine_colour(struct Texture *texture,struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the colour to be combined with the texture in blending combine mode.
==============================================================================*/

int Texture_set_combine_colour(struct Texture *texture,struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the colour to be combined with the texture in blending combine mode.
==============================================================================*/

enum Texture_combine_mode Texture_get_combine_mode(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

int Texture_set_combine_mode(struct Texture *texture,
	enum Texture_combine_mode combine_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

enum Texture_compression_mode Texture_get_compression_mode(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Returns how the texture is compressed.
==============================================================================*/

int Texture_set_compression_mode(struct Texture *texture,
	enum Texture_compression_mode compression_mode);
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Sets how the texture is compressed.
==============================================================================*/

enum Texture_filter_mode Texture_get_filter_mode(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the texture filter: linear or nearest.
==============================================================================*/

int Texture_set_filter_mode(struct Texture *texture,
	enum Texture_filter_mode filter_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the texture filter: linear or nearest.
==============================================================================*/

enum Texture_resize_filter_mode Texture_get_resize_filter_mode(
	struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the texture filter: linear or nearest.
==============================================================================*/

int Texture_set_resize_filter_mode(struct Texture *texture,
	enum Texture_resize_filter_mode resize_filter_mode);
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Sets the texture filter: linear or nearest.
==============================================================================*/

int Texture_allocate_image(struct Texture *texture,
	int width, int height, int depth, enum Texture_storage_type storage,
	int number_of_bytes_per_component, char *source_name);
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
Establishes the texture image as <width>*<height>*<depth> with the storage and
number_of_components specified in the <storage_type>, and
<number_of_bytes_per_component> may currently be 1 or 2.
The allocated space is cleared to values of 0 = black.
Call Texture_set_image_block to add texel data.
The optional <source_name> is recorded as the texture's imagefile_name.
Crop and other parameters are cleared.
==============================================================================*/

struct Cmgui_image *Texture_get_image(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Creates and returns a Cmgui_image from the image in <texture>, usually for
writing. Depth planes are stored as subimages of the returned structure.
Up to the calling function to DESTROY the returned Cmgui_image.
==============================================================================*/

int Texture_set_image(struct Texture *texture,
	struct Cmgui_image *cmgui_image,
	char *image_file_name, char *file_number_pattern,
	int start_file_number, int stop_file_number, int file_number_increment,
	int	crop_left, int crop_bottom, int crop_width, int crop_height);
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Puts <cmgui_image> into <texture>. If the <cmgui_image> contains
more than one image, these are put together into a 3-D texture. All other
parameters are merely recorded with the texture to be able to reproduce the
command for it later. The exception is the crop parameters, which must
which are used to cut the image size if <crop_width> and <crop_height> are
positive. Cropping is not available in the depth direction.
==============================================================================*/

int Texture_set_image_block(struct Texture *texture,
	int left, int bottom, int width, int height, int depth_plane,
	int source_width_bytes, unsigned char *source_pixels);
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Over-writes a block of texels in <texture>. The block is one texel in depth
and is placed in the given <depth_plane> which ranges from 0 to one
less than depth_texels. The block is placed offset from the left by <left> and
from the bottom by <bottom> and is <width>*<height> in size. The block must be
wholly within the texture width_texels and height_texels.
Image data is taken from <source_pixels> which is expected to match the
texture's current storage type and number_of_bytes_per_component. Source pixels
are expected in rows from bottom to top, and within rows from left to right.
The <source_width_bytes> is the offset between row data in <source_pixels>
and must be at least width*number_of_components*number_of_bytes_per_component
in size.
==============================================================================*/

int Texture_add_image(struct Texture *texture,
	struct Cmgui_image *cmgui_image,
	int crop_left, int crop_bottom, int crop_width, int crop_height);
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
Adds <cmgui_image> into <texture> making a 3D image from 2D images.
==============================================================================*/

struct X3d_movie;
struct Graphics_buffer_package;

struct X3d_movie *Texture_get_movie(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current X3d_movie from the texture.
==============================================================================*/

int Texture_set_movie(struct Texture *texture,struct X3d_movie *movie,
	struct Graphics_buffer_package *graphics_buffer_package, char *image_file_name);
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value.
==============================================================================*/

int set_Texture_storage(struct Parse_state *state,void *enum_storage_void_ptr,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
A modifier function to set the texture storage type.
==============================================================================*/

int Texture_get_raw_pixel_values(struct Texture *texture,int x,int y,int z,
	unsigned char *values);
/*******************************************************************************
LAST MODIFIED : 21 March 2002

DESCRIPTION :
Returns the byte values in the texture at x,y,z.
==============================================================================*/

int Texture_get_pixel_values(struct Texture *texture,
	double x, double y, double z, double *values);
/*******************************************************************************
LAST MODIFIED : 21 March 2002

DESCRIPTION :
Returns the byte values in the texture using the texture coordinates relative
to the physical size.  Each texel is assumed to apply exactly
at its centre and the filter_mode used to determine whether the pixels are
interpolated or not.  When closer than half a texel to a boundary the colour 
is constant from the half texel location to the edge. 
==============================================================================*/

char *Texture_get_image_file_name(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the name of the file from which the current texture image was read.
Note: returned file_number_pattern may be NULL if not set yet.
User must not modify the returned value!
==============================================================================*/

char *Texture_get_file_number_pattern(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the file number pattern substituted in the image_file_name with the
image_file_number when 3-D image series are read in.
Note: returned file_number_pattern may be NULL if not set yet.
User must not modify the returned value!
==============================================================================*/

int Texture_get_image_series_file_numbers(struct Texture *texture,
	int *start_file_number, int *stop_file_number, int *file_number_increment);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the start, stop and increment of file_numbers which together with the
image_file_name and file_number_pattern determine the files read in for
3-D textures. The returned values are meaningless if the texture is not
three dimensional = depth_texels greater than 1.
==============================================================================*/

int Texture_uses_image_file_name(struct Texture *texture,
	void *image_file_name_void);
/*******************************************************************************
LAST MODIFIED : 5 March 1998

DESCRIPTION :
Returns true if <texture> contains the image from file <image_file_name>.
==============================================================================*/

int Texture_get_number_of_bytes_per_component(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Returns the number bytes in each component of the texture: 1 or 2.
==============================================================================*/

int Texture_get_number_of_components(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Returns the number of components used per texel in the texture: 1, 2, 3 or 4.
==============================================================================*/

int Texture_get_original_size(struct Texture *texture,
	int *original_width_texels, int *original_height_texels, 
	int *original_depth_texels);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the width, height and depth of the image from which the texture was
read. May differ from the dimensions of the texture which is in powers of 2.
==============================================================================*/

int Texture_get_physical_size(struct Texture *texture,float *width,
	float *height, float *depth);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the physical size in model coordinates of the original texture image.
==============================================================================*/

int Texture_set_physical_size(struct Texture *texture,
	float width, float height, float depth);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Sets the physical size in model coordinates of the original texture image. The
default is 1.0, so that texture coordinates in the range from 0 to 1 represent
real image data and not padding to make image sizes up to powers of 2.
==============================================================================*/

int Texture_get_distortion_info(struct Texture *texture,
	float *distortion_centre_x,float *distortion_centre_y,
	float *distortion_factor_k1);
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Returns the radial distortion information stored with the texture as
<centre_x,centre_y> with <factor_k1>, all in the physical space of the image,
from (0.0,0.0) to (texture->width,texture->height).
==============================================================================*/

int Texture_set_distortion_info(struct Texture *texture,
	float distortion_centre_x,float distortion_centre_y,
	float distortion_factor_k1);
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Records with the texture that the lens from which it was produced had radial
distortion centred at <centre_x,centre_y> with <factor_k1>, and hence is to be
considered as distorted with such values. These values are relative to the
physical size of the image, from (0.0,0.0) to (texture->width,texture->height).
==============================================================================*/

int Texture_get_size(struct Texture *texture,
	int *width_texels, int *height_texels, int *depth_texels);
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the dimensions of the texture image in powers of 2. The width, height
and depth returned are at least as large as those of the original image read
into the texture.
==============================================================================*/

enum Texture_wrap_mode Texture_get_wrap_mode(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns how textures coordinates outside [0,1] are handled.
==============================================================================*/

int Texture_set_wrap_mode(struct Texture *texture,
	enum Texture_wrap_mode wrap_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets how textures coordinates outside [0,1] are handled.
==============================================================================*/

int list_Texture(struct Texture *texture,void *dummy);
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Writes the properties of the <texture> to the command window.
==============================================================================*/

int list_Texture_commands(struct Texture *texture,void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Writes on the command window the command needed to recreate the <texture>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int compile_Texture(struct Texture *texture,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Texture list/manager iterator function.
Rebuilds the display_list for <texture> if it is not current. If <texture>
does not have a display list, first attempts to give it one. The display list
created here may be called using execute_Texture, below.
???RC Textures must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of textures is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Texture should just call direct_render_Texture.
==============================================================================*/

int execute_Texture(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Activates <texture> by calling its display list. If the display list is not
current, an error is reported.
If a NULL <texture> is supplied, textures are disabled.
???RC The behaviour of textures is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Texture.
==============================================================================*/

int set_Texture(struct Parse_state *state,void *texture_address_void,
	void *texture_manager_void);
/*******************************************************************************
LAST MODIFIED : 5 September 1996

DESCRIPTION :
Modifier function to set the texture from a command.
==============================================================================*/

#endif /* !defined (TEXTURE_H) */
