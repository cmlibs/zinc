/*******************************************************************************
FILE : texture.h

LAST MODIFIED : 27 September 1999

DESCRIPTION :
The data structures used for representing textures.
==============================================================================*/
#if !defined (TEXTURE_H)
#define TEXTURE_H

#include <stdio.h>
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

enum Texture_storage_type
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_UNDEFINED_STORAGE,
	TEXTURE_LUMINANCE,
	TEXTURE_LUMINANCE_ALPHA,
	TEXTURE_RGB,
	TEXTURE_RGBA,
	TEXTURE_ABGR,
	TEXTURE_DMBUFFER,
	TEXTURE_PBUFFER
}; /* enum Texture_storage_type */


enum Texture_wrap_type
/*******************************************************************************
LAST MODIFIED : 17 November 1994

DESCRIPTION :
What happens to a texture when the texture coordinates are outside [0,1].
==============================================================================*/
{
	TEXTURE_CLAMP_WRAP,
	TEXTURE_REPEAT_WRAP
}; /* enum Texture_wrap_type */

enum Texture_filter_type
/*******************************************************************************
LAST MODIFIED : 17 November 1994

DESCRIPTION :
What happens to when the screen and texture are at different resolutions.
==============================================================================*/
{
	TEXTURE_LINEAR_FILTER,
	TEXTURE_NEAREST_FILTER
}; /* enum Texture_filter_type */

enum Texture_combine_type
/*******************************************************************************
LAST MODIFIED : 17 November 1994

DESCRIPTION :
How the texture is combined with the material.
==============================================================================*/
{
	TEXTURE_BLEND,
	TEXTURE_DECAL,
	TEXTURE_MODULATE
}; /* enum Texture_combine_type */

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

int Texture_get_number_of_components_from_storage_type(
	enum Texture_storage_type storage);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Returns the number of bytes used per texel in the image.
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

int Texture_get_combine_mode(struct Texture *texture,
	enum Texture_combine_type *combine_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

int Texture_set_combine_mode(struct Texture *texture,
	enum Texture_combine_type combine_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/

int Texture_get_filter_mode(struct Texture *texture,
	enum Texture_filter_type *filter_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the texture filter: linear or nearest.
==============================================================================*/

int Texture_set_filter_mode(struct Texture *texture,
	enum Texture_filter_type filter_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the texture filter: linear or nearest.
==============================================================================*/

int Texture_set_image(struct Texture *texture,unsigned long *image,
	enum Texture_storage_type storage,int number_of_bytes_per_component,
	int image_width,int image_height,
	char *image_file_name,int	crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value. Similarly, the four crop parameters should
be set to record any cropping already done on the image so that it may be
recorded with the texture.
==============================================================================*/

int Texture_set_image_file(struct Texture *texture,char *image_file_name,
	int left_margin_texels,int bottom_margin_texels,int width_texels,
	int height_texels,double radial_distortion_centre_x,
	double radial_distortion_centre_y,double radial_distortion_factor_k1);
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Reads the image for <texture> from file <image_file_name> and then crops it
using <left_margin_texels>, <bottom_margin_texels>, <width_texels> and
<height_texels>.  If <width_texels> and <height_texels> are not both positive or
if <left_margin_texels> and <bottom_margin_texels> are not both non-negative or
if the cropping region is not contained in the image then no cropping is
performed.
==============================================================================*/

struct X3d_movie;

int Texture_set_movie(struct Texture *texture,struct X3d_movie *movie,
	struct User_interface *user_interface, char *image_file_name);
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value.
==============================================================================*/

int set_Texture_image(struct Parse_state *state,void *texture_void,
	void *set_file_name_option_table_void);
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Modifier function to set the texture image from a command.
==============================================================================*/

int set_Texture_wrap_repeat(struct Parse_state *state,void *texture_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture wrap type to repeat.
==============================================================================*/

int set_Texture_wrap_clamp(struct Parse_state *state,void *texture_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture wrap type to clamp.
==============================================================================*/

int set_Texture_filter_nearest(struct Parse_state *state,
	void *texture_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture magnification/minification filter to
nearest.
==============================================================================*/

int set_Texture_filter_linear(struct Parse_state *state,
	void *texture_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture magnification/minification filter to
linear.
==============================================================================*/

int set_Texture_combine_blend(struct Parse_state *state,
	void *texture_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture combine type to blend.
==============================================================================*/

int set_Texture_combine_decal(struct Parse_state *state,
	void *texture_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture combine type to decal.
==============================================================================*/

int set_Texture_combine_modulate(struct Parse_state *state,
	void *texture_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 August 1996

DESCRIPTION :
A modifier function to set the texture combine type to modulate.
==============================================================================*/

int Texture_get_raw_pixel_values(struct Texture *texture,int x,int y,
	unsigned char *values);
/*******************************************************************************
LAST MODIFIED : 13 April 1999

DESCRIPTION :
Returns the byte values in the texture at x,y.
==============================================================================*/

int Texture_get_pixel_values(struct Texture *texture,
	double x, double y, double *values);
/*******************************************************************************
LAST MODIFIED : 13 April 1999

DESCRIPTION :
Returns the byte values in the texture using the texture coordinates relative
to the physical size.  Each texel is assumed to apply exactly
at its centre and the filter_type used to determine whether the pixels are
interpolated or not.  When closer than half a texel to a boundary the colour 
is constant from the half texel location to the edge. 
==============================================================================*/

char *Texture_get_image_file_name(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the name of the file from which the current texture image was read.
==============================================================================*/

int Texture_uses_image_file_name(struct Texture *texture,
	void *image_file_name_void);
/*******************************************************************************
LAST MODIFIED : 5 March 1998

DESCRIPTION :
Returns true if <texture> contains the image from file <image_file_name>.
==============================================================================*/

int Texture_get_number_of_components(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the number of 8-bit components used per texel (1,2,3 or 4) in the image.
==============================================================================*/

int Texture_get_original_size(struct Texture *texture,
	int *original_width_texels,int *original_height_texels);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the width and height of the image from which the texture was read. May
differ from the dimensions of the texture which is in powers of 2.
==============================================================================*/

int Texture_get_physical_size(struct Texture *texture,
	float *width,float *height);
/*******************************************************************************
LAST MODIFIED : 5 March 1998

DESCRIPTION :
Returns the physical size in model coordinates of the original texture image.
==============================================================================*/

int Texture_set_physical_size(struct Texture *texture,
	float width,float height);
/*******************************************************************************
LAST MODIFIED : 5 March 1998

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
	int *width_texels,int *height_texels);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the dimensions of the texture image in powers of 2. The width and height
returned are at least as large as those of the original image read into the
texture.
==============================================================================*/

int Texture_get_wrap_mode(struct Texture *texture,
	enum Texture_wrap_type *wrap_mode);
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns how textures coordinates outside [0,1] are handled.
==============================================================================*/

int Texture_set_wrap_mode(struct Texture *texture,
	enum Texture_wrap_type wrap_mode);
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

#if defined (OLD_CODE)
int activate_Texture(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 19 November 1994

DESCRIPTION :
Activates the <texture> as part of the rendering loop.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int set_Texture(struct Parse_state *state,void *texture_address_void,
	void *texture_manager_void);
/*******************************************************************************
LAST MODIFIED : 5 September 1996

DESCRIPTION :
Modifier function to set the texture from a command.
==============================================================================*/

int Texture_write_to_file(struct Texture *texture, char *file_name,
	enum Image_file_format file_format, enum Image_orientation orientation);
/*******************************************************************************
LAST MODIFIED : 28 May 1999

DESCRIPTION :
Writes the image stored in the texture to a file.
==============================================================================*/

#if defined (OLD_CODE)
#if defined (OPENGL_API)
/*???DB.  This should not be here.  Textures should not have to know about
	scenes.  OPENGL_API is not the ideal flag, but I want to prevent scenes
	having to be included in unemap.  I tried shifting it to scene, but then
	knowledge of DM_buffers and Textures is needed.  What should be done is:
	- move DM_buffers into there own module (?)
	- extend texture by adding functions for getting and setting DM_buffers
	- move set_Texture_image_from_scene to scene_viewer
	*/
struct Scene;

int set_Texture_image_from_scene(struct Texture *texture, struct Scene *scene,
	enum Texture_storage_type storage,int image_width,int image_height,
	char *image_file_name,int crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Creates the image in the format given by attempting to render it into the
texture.
==============================================================================*/
#endif /* defined (OPENGL_API) */
#endif /* defined (OLD_CODE) */
#endif /* !defined (TEXTURE_H) */
