/*******************************************************************************
FILE : texture.h

LAST MODIFIED : 12 October 2000

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
LAST MODIFIED : 29 June 2000

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_UNDEFINED_STORAGE,
	TEXTURE_TYPE_BEFORE_FIRST,
	TEXTURE_LUMINANCE,
	TEXTURE_LUMINANCE_ALPHA,
	TEXTURE_RGB,
	TEXTURE_RGBA,
	TEXTURE_ABGR,
	/* The last two types are special and cannot be normally selected */
	TEXTURE_TYPE_AFTER_LAST_NORMAL,
	TEXTURE_DMBUFFER,
	TEXTURE_PBUFFER,
	TEXTURE_TYPE_AFTER_LAST
}; /* enum Texture_storage_type */

enum Texture_combine_mode
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
How the texture is combined with the material.
==============================================================================*/
{
	TEXTURE_BLEND,
	TEXTURE_DECAL,
	TEXTURE_MODULATE,
	TEXTURE_COMBINE_MODE_INVALID
}; /* enum Texture_combine_mode */

enum Texture_filter_mode
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
What happens to when the screen and texture are at different resolutions.
==============================================================================*/
{
	TEXTURE_LINEAR_FILTER,
	TEXTURE_NEAREST_FILTER,
	TEXTURE_FILTER_MODE_INVALID
}; /* enum Texture_filter_mode */

enum Texture_wrap_mode
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
What happens to a texture when the texture coordinates are outside [0,1].
==============================================================================*/
{
	TEXTURE_CLAMP_WRAP,
	TEXTURE_REPEAT_WRAP,
	TEXTURE_WRAP_MODE_INVALID
}; /* enum Texture_wrap_mode */

enum Texture_row_order 
/*******************************************************************************
LAST MODIFIED : 4 May 2001

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_TOP_TO_BOTTOM,
	TEXTURE_BOTTOM_TO_TOP,
	TEXTURE_ROW_ORDER_INVALID
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

char *Texture_combine_mode_string(enum Texture_combine_mode combine_mode);
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the combine_mode,
eg. TEXTURE_DECAL = "decal". This string should match the command
used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Texture_combine_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_combine_modes.
Strings are obtained from function Texture_combine_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Texture_combine_mode Texture_combine_mode_from_string(
	char *combine_mode_string);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_combine_mode> described by <combine_mode_string>.
==============================================================================*/

char *Texture_filter_mode_string(enum Texture_filter_mode filter_mode);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the filter_mode,
eg. TEXTURE_LINEAR_FILTER = "linear_filter". This string should match the
command used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Texture_filter_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_filter_modes.
Strings are obtained from function Texture_filter_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Texture_filter_mode Texture_filter_mode_from_string(
	char *filter_mode_string);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_filter_mode> described by <filter_mode_string>.
==============================================================================*/

char *Texture_wrap_mode_string(enum Texture_wrap_mode wrap_mode);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the wrap_mode,
eg. TEXTURE_CLAMP_WRAP = "clamp_wrap". This string should match the command
used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Texture_wrap_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_wrap_modes. Strings are obtained from function Texture_wrap_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Texture_wrap_mode Texture_wrap_mode_from_string(char *wrap_mode_string);
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_wrap_mode> described by <wrap_mode_string>.
==============================================================================*/

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

char *Texture_storage_type_string(enum Texture_storage_type texture_storage_type);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Returns a pointer to a static string describing the texture storage, eg.
TEXTURE_STORAGE == "rgba". This string should match the command used
to create that type of texture. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Texture_storage_type_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Texture_storage_types - obtained from function Texture_storage_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Texture_storage_type Texture_storage_type_from_string(char *texture_type_string);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Returns the <Texture_storage_type> described by <texture_type_string>.
==============================================================================*/

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

int Texture_set_image(struct Texture *texture,unsigned long *image,
	enum Texture_storage_type storage,int number_of_bytes_per_component,
	int image_width,int image_height, enum Texture_row_order row_order,
	char *image_file_name,int	crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height,int perform_crop);
/*******************************************************************************
LAST MODIFIED : 5 September 2000

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value. Similarly, the four crop parameters should
be set to record any cropping already done on the image so that it may be
recorded with the texture - not however that if perform_crop is true then the
crop is performed as the image is put into the texture.
==============================================================================*/

int managed_Texture_set_image(struct Texture *texture,
	struct MANAGER(Texture) *texture_manager,unsigned long *image,
	enum Texture_storage_type storage,int number_of_bytes_per_component,
	int image_width,int image_height, enum Texture_row_order row_order,
	char *image_file_name,int	crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height,int perform_crop);
/*******************************************************************************
LAST MODIFIED : 25 January 2002

DESCRIPTION :
Does Texture_set_image on the managed <texture>.
Returns an error if <texture> is not managed.
==============================================================================*/


int Texture_set_image_file(struct Texture *texture,char *image_file_name,
	int specify_width,int specify_height,enum Raw_image_storage raw_image_storage,
	int crop_left_margin,int crop_bottom_margin,int crop_width,
	int crop_height,double radial_distortion_centre_x,
	double radial_distortion_centre_y,double radial_distortion_factor_k1);
/*******************************************************************************
LAST MODIFIED : 12 October 2000

DESCRIPTION :
Reads the image for <texture> from file <image_file_name> and then crops it
using <left_margin_texels>, <bottom_margin_texels>, <width_texels> and
<height_texels>.  If <width_texels> and <height_texels> are not both positive or
if <left_margin_texels> and <bottom_margin_texels> are not both non-negative or
if the cropping region is not contained in the image then no cropping is
performed.
<specify_width> and <specify_height> allow the width and height to be specified
for formats that do not have a header, eg. RAW and YUV.
==============================================================================*/

struct X3d_movie;

struct X3d_movie *Texture_get_movie(struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current X3d_movie from the texture.
==============================================================================*/

int Texture_set_movie(struct Texture *texture,struct X3d_movie *movie,
	struct User_interface *user_interface, char *image_file_name);
/*******************************************************************************
LAST MODIFIED : 9 September 1998

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
at its centre and the filter_mode used to determine whether the pixels are
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

#if defined (IMAGEMAGICK)
int Texture_write_to_file(struct Texture *texture, char *file_name);
#else /* defined (IMAGEMAGICK) */
int Texture_write_to_file(struct Texture *texture, char *file_name,
	enum Image_file_format file_format);
#endif /* defined (IMAGEMAGICK) */
/*******************************************************************************
LAST MODIFIED : 27 November 2001

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
