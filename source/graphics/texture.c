/*******************************************************************************
FILE : texture.c

LAST MODIFIED : 16 April 2002

DESCRIPTION :
The functions for manipulating graphical textures.

???RC Only OpenGL is supported now.

???RC 12 February 1998.
		Two functions are now used for activating the texture:
- compile_Texture puts the texture into its own display list.
- execute_Texture calls that display list, or disables textures if a NULL
	argument is passed.
		The separation into two functions was needed because OpenGL cannot start a
new list while one is being written to. As a consequence, you must precompile
all objects that are executed from within another display list. To make this job
easier, compile_Texture is a list/manager iterator function.
		Future updates of OpenGL may overcome this limitation, in which case the
execute function can take over compiling as well. Furthermore, it is easy to
return to direct rendering, as described with these routines.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/image_utilities.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "general/object.h"
#if defined (DM_BUFFERS)
#include "three_d_drawing/ThreeDDraw.h"
#include "three_d_drawing/dm_interface.h"
#endif /* defined (DM_BUFFERS) */
#include "graphics/light.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"
#include "graphics/texture.h"
#include "three_d_drawing/movie_extensions.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Texture
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
The properties of a graphical texture.
???DB.  GL separates the texture from the texture environment (for performance
	reasons ?).  I do not think that this is "natural", especially because only
	certain environment types can be used with certain numbers of components.  The
	same index is used for environment and texture.
==============================================================================*/
{
	/* the name of the texture */
	char *name;
	/* texture dimension: 1,2 or 3, automatically determined from image files,
		 ie. 2 if it has one texel depth, 1 if it has one texel height */
	int dimension;
	/* the range of texture coordinates in model units */
	float depth, height, width;
	/* image distortion parameters in the physical size of the image, where 0,0
		 refers to the left, bottom corner and width, height is the right, top.
		 Only pure radial distortion in physical space is supported. Note that these
		 are just a convenient storage space for these values: the texture display
		 list does not need to be updated if these change. */
	float distortion_centre_x,distortion_centre_y,distortion_factor_k1;
	/* the name of the file the image was read from */
		/*???DB.  May get rid of this when able to "draw" the image in response to
			a list texture command */
		/*???RC.  Keep it for working out if we can reuse already loaded textures */
	char *image_file_name;
	/* file number pattern and ranges for 3-D textures */
	char *file_number_pattern;
	int start_file_number, stop_file_number, file_number_increment;
	enum Texture_storage_type storage;
	/* number of 8-bit bytes per component */
	int number_of_bytes_per_component;
	/* array of 4-byte row-aligned unsigned chars containing the texel data.  The
		 texels fill the image from left to right, bottom to top.  Each row of texel
		 information must be 4-byte aligned (end of row byte padded) */
		/*???DB.  OpenGL allows greater choice, but this will not be used */
	unsigned char *image;
	/* OpenGL requires the width and height of textures to be in powers of 2.
		Hence, only the original width x height contains useful image data */
	/* stored image size in texels */
	int depth_texels, height_texels, width_texels;
	/* original image size in texels */
	int original_depth_texels,original_height_texels,original_width_texels;
	/* cropping of image in file */
	int crop_bottom_margin,crop_height,crop_left_margin,crop_width;
	/* texture display options */
	enum Texture_combine_mode combine_mode;
	enum Texture_compression_mode compression_mode;
	enum Texture_filter_mode filter_mode;
	enum Texture_wrap_mode wrap_mode;
	struct Colour combine_colour;
	float combine_alpha;
	/* following controls how a texture is downsampled to fit texture hardware */
	enum Texture_resize_filter_mode resize_filter_mode;

	struct X3d_movie *movie;
#if defined (DM_BUFFERS)
	struct Dm_buffer *dmbuffer;
#endif /* defined (DM_BUFFERS) */

	struct MANAGER(Texture) *texture_manager;

#if defined (OPENGL_API)
	GLuint display_list;
	GLuint texture_id;
#endif /* defined (OPENGL_API) */
	/* flag to say if the display list is up to date */
	int display_list_current;
	/* the number of structures that point to this texture.  The texture
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Texture */

FULL_DECLARE_INDEXED_LIST_TYPE(Texture);

FULL_DECLARE_MANAGER_TYPE(Texture);

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Texture,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Texture)

int Texture_storage_type_get_number_of_components(
	enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the number of components used per texel for <storage> type.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_storage_type_get_number_of_components);
	switch (storage)
	{
		case TEXTURE_LUMINANCE:
		{
			return_code = 1;
		} break;
		case TEXTURE_LUMINANCE_ALPHA:
		{
			return_code = 2;
		} break;
		case TEXTURE_RGB:
		{
			return_code = 3;
		} break;
		case TEXTURE_RGBA:
		case TEXTURE_ABGR:
		{
			return_code = 4;
		} break;
		case TEXTURE_DMBUFFER:
		case TEXTURE_PBUFFER:
		{
			return_code = 4;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_storage_type_get_number_of_components.  "
				"Texture storage type unknown");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_number_of_bytes_per_texel_from_storage_type */

#if defined (OPENGL_API)
static int Texture_get_type_and_format_from_storage_type(
	enum Texture_storage_type storage,int number_of_bytes_per_component,
	GLenum *type,GLenum *format)
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_type_and_format_from_storage_type);
	return_code=1;
	switch (storage)
	{
		case TEXTURE_LUMINANCE:
		{
			*format=GL_LUMINANCE;
		} break;
		case TEXTURE_LUMINANCE_ALPHA:
		{
			*format=GL_LUMINANCE_ALPHA;
		} break;
		case TEXTURE_RGB:
		{
			*format=GL_RGB;
		} break;
		case TEXTURE_RGBA:
		{
			*format=GL_RGBA;
		} break;
		case TEXTURE_ABGR:
		{
#if defined (GL_EXT_abgr)
			*format=GL_ABGR_EXT;
#else /* defined (GL_EXT_abgr) */
			display_message(ERROR_MESSAGE,"Texture_get_type_and_format_from_storage_type.  abgr storage not supported in this compilation");
			return_code=0;
#endif /* defined (GL_EXT_abgr) */
		} break;
		case TEXTURE_DMBUFFER:
		case TEXTURE_PBUFFER:
		{
			*format=GL_RGBA;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_get_type_and_format_from_storage_type."
				"  Texture storage type unknown");
			return_code=0;
		} break;
	}
	if (return_code)
	{
		switch(number_of_bytes_per_component)
		{
			case 1:
			{
				*type=GL_UNSIGNED_BYTE;
			} break;
			case 2:
			{
				*type=GL_UNSIGNED_SHORT;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_type_and_format_from_storage_type."
					"  Must be one or two bytes per component");
				return_code=0;
			} break;
		}
	}
	LEAVE;

	return (return_code);
} /* Texture_get_type_and_format_from_storage_type */
#endif /* defined (OPENGL_API) */
	
#if defined (OPENGL_API)
static int Texture_get_hardware_storage_format(
	enum Texture_compression_mode compression_mode, int number_of_components)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_hardware_storage_format);
	return_code=1;
	switch (compression_mode)
	{
		case TEXTURE_UNCOMPRESSED:
		{
			return_code = number_of_components;
		} break;
		case TEXTURE_COMPRESSED_UNSPECIFIED:
		{
#if defined (GL_ARB_texture_compression)
			if (query_gl_extension("GL_ARB_texture_compression"))
			{
				switch (number_of_components)
				{
					case 3:
					{
						return_code = GL_COMPRESSED_RGB_ARB;
					} break;
					case 4:
					{
						return_code = GL_COMPRESSED_RGBA_ARB;
					} break;
					default:
					{
						display_message(WARNING_MESSAGE, "Texture_get_hardware_storage_format.  "
							"Texture compression not supported for this number of components.");
						return_code = number_of_components;
					} break;
				}
			}
			else
			{
#endif /* defined (GL_ARB_texture_compression) */
				display_message(WARNING_MESSAGE, "Texture_get_hardware_storage_format.  "
					"Texture compression not supported on this hardware.");
				return_code = number_of_components;
#if defined (GL_ARB_texture_compression)
			}
#endif /* defined (GL_ARB_texture_compression) */
		} break;
		default:
		{
			display_message(ERROR_MESSAGE, "Texture_get_hardware_storage_format.  "
				"Invalid texture compression, using uncompressed.");
			return_code = number_of_components;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_hardware_storage_format */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int direct_render_Texture_environment(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 February 2002

DESCRIPTION :
Directly outputs the commands that set the environment for this <texture>.
This is separated from the commands for the texture itself as these are the
only commands required when the texture is compiled and bound with the
GL_EXT_texture_object extension.
==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GLfloat values[4];
	GLint number_of_components;
	gtMatrix texture_coordinate_transform;
#endif /* defined (OPENGL_API) */

	ENTER(direct_render_Texture_environment);
	return_code=0;
	if (texture)
	{
		return_code=1;
#if defined (OPENGL_API)
		if (texture->width)
		{
			texture_coordinate_transform[0][0] = texture->original_width_texels/
				(texture->width_texels*texture->width);
		}
		else
		{
			texture_coordinate_transform[0][0] = 1.0;
		}
		texture_coordinate_transform[1][0]=0.;
		texture_coordinate_transform[2][0]=0.;
		texture_coordinate_transform[3][0]=0.;
		texture_coordinate_transform[0][1]=0.;
		if (texture->height)
		{
			texture_coordinate_transform[1][1] = texture->original_height_texels/
				(texture->height_texels*texture->height);
		}
		else
		{
			texture_coordinate_transform[1][1] = 1.0;
		}
		texture_coordinate_transform[2][1]=0.;
		texture_coordinate_transform[3][1]=0.;
		texture_coordinate_transform[0][2]=0.;
		texture_coordinate_transform[1][2]=0.;
		if (texture->depth)
		{
			texture_coordinate_transform[2][2] = texture->original_depth_texels/
				(texture->depth_texels*texture->depth);
		}
		else
		{
			texture_coordinate_transform[2][2] = 1.0;
		}
		texture_coordinate_transform[3][2]=0.;
		texture_coordinate_transform[0][3]=0.;
		texture_coordinate_transform[1][3]=0.;
		texture_coordinate_transform[2][3]=0.;
		texture_coordinate_transform[3][3]=1.;
		/* set the texture coordinate transformation */
		glMatrixMode(GL_TEXTURE);
		wrapperLoadCurrentMatrix(&texture_coordinate_transform);
		glMatrixMode(GL_MODELVIEW);
		number_of_components = Texture_storage_type_get_number_of_components(texture->storage);
		values[0]=(texture->combine_colour).red;
		values[1]=(texture->combine_colour).green;
		values[2]=(texture->combine_colour).blue;
		values[3]=texture->combine_alpha;
		/* specify how the texture is combined */
		switch (texture->combine_mode)
		{
			case TEXTURE_BLEND:
			{
				if (number_of_components<3)
				{
					glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
				}
				else
				{
					glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
				}
			} break;
			case TEXTURE_DECAL:
			{
				if (number_of_components<3)
				{
					glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
				}
				else
				{
					glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
				}
			} break;
			case TEXTURE_MODULATE:
			{
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			} break;
		}
		glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,values);
		switch (texture->dimension)
		{
			case 1:
			{
				glEnable(GL_TEXTURE_1D);
			} break;
			case 2:
			{
				glEnable(GL_TEXTURE_2D);
			} break;
			case 3:
			{
#if defined (GL_VERSION_1_2)
				glEnable(GL_TEXTURE_3D);
#else /* defined (GL_VERSION_1_2) */
				display_message(ERROR_MESSAGE,
					"direct_render_Texture_environment.  "
					"3D textures not supported in this version.");
				return_code=0;
#endif /* defined (GL_VERSION_1_2) */
			} break;
		}
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"direct_render_Texture_environment.  Missing texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Texture_environment */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Texture_get_hardware_reduction(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 21 February 2002

DESCRIPTION :
Queries the graphics hardware to determine how much the texture size must be
reduced to fit the available space. Returns the reduction factor which will be
a power of two, where 1 means no reduction, 2 means the size must be halved,
and 0 is returned without writing an error if there is no space for the texture.
The reduction factor applies equally in all texture dimensions.
==============================================================================*/
{
	int reduction;
#if defined (OPENGL_API)
	int return_code;
	GLenum format, type;
	GLint number_of_components, test_width, hardware_texture_format;
#endif /* defined (OPENGL_API) */

	ENTER(Texture_get_hardware_reduction);
	if (texture)
	{
#if defined (OPENGL_API)
		number_of_components =
			Texture_storage_type_get_number_of_components(texture->storage);
		Texture_get_type_and_format_from_storage_type(texture->storage,
			texture->number_of_bytes_per_component, &type, &format);
		hardware_texture_format = Texture_get_hardware_storage_format(
			texture->compression_mode, number_of_components);
		reduction = 1;
		switch (texture->dimension)
		{
			case 1:
			{
				do
				{
					glTexImage1D(GL_PROXY_TEXTURE_1D, (GLint)0, hardware_texture_format,
						(GLint)(texture->width_texels/reduction), (GLint)0,
						format, type, (GLvoid *)(texture->image));
					glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, (GLint)0,
						GL_TEXTURE_WIDTH, &test_width);
					if (0 == test_width)
					{
						reduction *= 2;
						return_code = (reduction < texture->width_texels);
					}
				}
				while ((test_width == 0) && return_code);
			} break;
			case 2:
			{
				do
				{
					glTexImage2D(GL_PROXY_TEXTURE_2D, (GLint)0, hardware_texture_format,
						(GLint)(texture->width_texels/reduction),
						(GLint)(texture->height_texels/reduction), (GLint)0,
						format, type, (GLvoid *)(texture->image));
					glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, (GLint)0,
						GL_TEXTURE_WIDTH, &test_width);
					if (0 == test_width)
					{
						reduction *= 2;
						return_code = (reduction < texture->width_texels) &&
							(reduction < texture->height_texels);
					}
				}
				while ((test_width == 0) && return_code);
			} break;
			case 3:
			{
#if defined (GL_VERSION_1_2)
				do
				{
					glTexImage3D(GL_PROXY_TEXTURE_3D, (GLint)0, hardware_texture_format,
						(GLint)(texture->width_texels/reduction),
						(GLint)(texture->height_texels/reduction),
						(GLint)(texture->depth_texels/reduction), (GLint)0,
						format, type, (GLvoid *)(texture->image));
					glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, (GLint)0,
						GL_TEXTURE_WIDTH, &test_width);
					if (0 == test_width)
					{
						reduction *= 2;
						return_code = (reduction < texture->width_texels) &&
							(reduction < texture->height_texels) &&
							(reduction < texture->depth_texels);
					}
				}
				while ((test_width == 0) && return_code);
#else /* defined (GL_VERSION_1_2) */
				display_message(ERROR_MESSAGE,
					"Texture_get_hardware_reduction.  "
					"3D textures not supported in this version.");
				return_code=0;
#endif /* defined (GL_VERSION_1_2) */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_hardware_reduction.  Invalid texture dimension");
				return_code = 0;
			} break;
		}
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"Texture_get_hardware_reduction.  Only implemented for OpenGL");
		reduction = 0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_hardware_reduction.  Missing texture");
		reduction = 0;
	}
	LEAVE;

	return (reduction);
} /* Texture_get_hardware_reduction */
#endif /* defined (OPENGL_API)*/

#if defined (OPENGL_API)
static unsigned char *Texture_get_resized_image(struct Texture *texture,
	int width_texels, int height_texels, int depth_texels,
	enum Texture_resize_filter_mode resize_filter)
/*******************************************************************************
LAST MODIFIED : 7 March 2002

DESCRIPTION :
Returns an allocated image space suitable for passing to texture commands and
containing the image in <texture> resized to:
<width_texels>*<height_texels>*<depth_texels>.
The <filter> mode controls how the texture image is resampled and varyies from
fast nearest pixel filtering to linear interpolation.
???RC Since this function is currently only called with a power of 2 reduction
factor obtained from Texture_get_hardware_reduction, only the case where the
original width, height and depth are integer multiples of the new values is
currently implemented for the expensive linear filter.
==============================================================================*/
{
	unsigned char *destination, *i_base, *image, *j_base, *source, *source2;
	float i_factor, j_factor, k_factor;
	int a, a_offset, b, b_offset, bytes_per_pixel, c, c_offset, depth_reduction,
		destination_row_width_bytes, height_reduction, i, i_offset, j, j_offset,
		k, k_offset, n, number_of_bytes_per_component, number_of_components,
		padding_bytes, return_code, source_row_width_bytes, width_reduction;
	unsigned long accumulator, number_of_accumulated_pixels;

	ENTER(Texture_get_resized_image);
	image = (unsigned char *)NULL;
	if (texture && (0 < width_texels) && (0 < height_texels) &&
		(0 < depth_texels))
	{
		return_code = 1;
		number_of_components =
			Texture_storage_type_get_number_of_components(texture->storage);
		number_of_bytes_per_component = texture->number_of_bytes_per_component;
		bytes_per_pixel = number_of_bytes_per_component * number_of_components;
		source_row_width_bytes = 4*((texture->width_texels*bytes_per_pixel + 3)/4);
		destination_row_width_bytes = 4*((width_texels*bytes_per_pixel + 3)/4);
		if (ALLOCATE(image, unsigned char,
			depth_texels*height_texels*destination_row_width_bytes))
		{
			switch (resize_filter)
			{
				case TEXTURE_RESIZE_LINEAR_FILTER:
				{
					width_reduction = texture->width_texels / width_texels;
					height_reduction = texture->height_texels / height_texels;
					depth_reduction = texture->depth_texels / depth_texels;
					if ((width_texels*width_reduction == texture->width_texels) &&
						(height_texels*height_reduction == texture->height_texels) &&
						(depth_texels*depth_reduction == texture->depth_texels))
					{
						number_of_accumulated_pixels =
							width_reduction*height_reduction*depth_reduction;
						i_offset = (depth_reduction - 1)*texture->height_texels*
							source_row_width_bytes;
						j_offset = (height_reduction - 1)*source_row_width_bytes;
						k_offset = width_reduction*bytes_per_pixel;
						a_offset = (texture->height_texels - height_reduction)*
							source_row_width_bytes;
						b_offset = source_row_width_bytes - width_reduction*bytes_per_pixel;
						c_offset = bytes_per_pixel;
						source = texture->image;
						destination = image;
						switch (number_of_bytes_per_component)
						{
							case 1:
							{
								for (i = 0; i < depth_texels; i++)
								{
									for (j = 0; j < height_texels; j++)
									{
										for (k = 0; k < width_texels; k++)
										{
											for (n = 0; n < number_of_components; n++)
											{
												source2 = source + n;
												accumulator = 0;
												for (a = 0; a < depth_reduction; a++)
												{
													for (b = 0; b < height_reduction; b++)
													{
														for (c = 0; c < width_reduction; c++)
														{
															accumulator += *source2;
															source2 += c_offset;
														}
														source2 += b_offset;
													}
													source2 += a_offset;
												}
												/* following performs integer rounding */
												accumulator =
													(accumulator + number_of_accumulated_pixels/2) /
													number_of_accumulated_pixels;
												*destination = (unsigned char)accumulator;
												destination++;
											}
											source += k_offset;
										}
										source += j_offset;
									}
									source += i_offset;
								}
							} break;
							case 2:
							{
								for (i = 0; i < depth_texels; i++)
								{
									for (j = 0; j < height_texels; j++)
									{
										for (k = 0; k < width_texels; k++)
										{
											for (n = 0; n < number_of_components; n++)
											{
												source2 = source + 2*n;
												accumulator = 0;
												for (a = 0; a < depth_reduction; a++)
												{
													for (b = 0; b < height_reduction; b++)
													{
														for (c = 0; c < width_reduction; c++)
														{
#if (1234==BYTE_ORDER)
															accumulator += *source2 + ((*(source2 + 1)) << 8);
#else /* (1234==BYTE_ORDER) */
															accumulator += (*source2 << 8) + (*(source2 + 1));
#endif /* (1234==BYTE_ORDER) */
															source2 += c_offset;
														}
														source2 += b_offset;
													}
													source2 += a_offset;
												}
												/* following performs integer rounding */
												accumulator =
													(accumulator + number_of_accumulated_pixels/2) /
													number_of_accumulated_pixels;
												*destination = (unsigned char)accumulator;
												destination++;
											}
											source += k_offset;
										}
										source += j_offset;
									}
									source += i_offset;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE, "Texture_get_resized_image.  "
									"Only 1 or 2 bytes per component supported");
								return_code = 0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Texture_get_resized_image.  "
							"Only integer downsampling supported for linear filter");
						return_code = 0;
					}
				} break;
				case TEXTURE_RESIZE_NEAREST_FILTER:
				{
					i_factor = texture->depth_texels / depth_texels;
					j_factor = texture->height_texels / height_texels;
					k_factor = texture->width_texels / width_texels;
					padding_bytes = (destination_row_width_bytes -
						width_texels*bytes_per_pixel);
					destination = image;
					for (i = 0; i < depth_texels; i++)
					{
						i_base = texture->image + (int)((0.5 + (float)i)*i_factor) *
							texture->height_texels*source_row_width_bytes;
						for (j = 0; j < height_texels; j++)
						{
							j_base = i_base + (int)((0.5 + (float)j)*j_factor) *
								source_row_width_bytes;
							for (k = 0; k < width_texels; k++)
							{
								source = j_base +
									(int)((0.5 + (float)k)*k_factor) * bytes_per_pixel;
								for (n = 0; n < bytes_per_pixel; n++)
								{
									*destination = *source;
									destination++;
									source++;
								}
							}
							if (0 < padding_bytes)
							{
								memset((void *)padding_bytes, 0, padding_bytes);
								destination += padding_bytes;
							}
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Texture_get_resized_image.  Unknown resize filter mode");
					return_code = 0;
				} break;
			}
			if (!return_code)
			{
				DEALLOCATE(image);
				image = (unsigned char *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_get_resized_image.  Not enough memory for image");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_resized_image.  Missing texture");
	}
	LEAVE;

	return (image);
} /* Texture_get_resized_image */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int direct_render_Texture(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 14 March 2002

DESCRIPTION :
Directly outputs the commands setting up the <texture>.
==============================================================================*/
{
	int  return_code;
#if defined (OPENGL_API)
	int hardware_storage_format, number_of_components,reduced_depth_texels,
		reduced_height_texels,reduced_width_texels,reduction;
	GLenum format, texture_target, type;
	GLfloat values[4];
	unsigned char *reduced_image;
#endif /* defined (OPENGL_API) */

	ENTER(direct_render_Texture);
	return_code = 1;
	if (texture)
	{
#if defined (OPENGL_API)
		switch (texture->dimension)
		{
			case 1:
			{
				texture_target = GL_TEXTURE_1D;
			} break;
			case 2:
			{
				texture_target = GL_TEXTURE_2D;
			} break;
			case 3:
			{
#if defined (GL_VERSION_1_2)
				texture_target = GL_TEXTURE_3D;
#else /* defined (GL_VERSION_1_2) */
				display_message(ERROR_MESSAGE,
					"direct_render_texture.  "
					"3D textures not supported in this version.");
				return_code=0;
#endif /* defined (GL_VERSION_1_2) */
			} break;
		}
		switch(texture->storage)
		{
			case TEXTURE_DMBUFFER:
			{
#if defined (SGI_DIGITAL_MEDIA)
				if (texture->dimension == 2)
				{
					Texture_get_type_and_format_from_storage_type(texture->storage,
						texture->number_of_bytes_per_component, &type, &format);
					glTexImage2D(GL_TEXTURE_2D,(GLint)0,
						GL_RGBA8_EXT,
						(GLint)(texture->width_texels),
						(GLint)(texture->height_texels),(GLint)0,
						format,type, NULL);
					glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
						texture->width_texels, texture->height_texels);
#if defined (DEBUG)
					glCopyTexImage2DEXT(GL_TEXTURE_2D, 0, GL_RGBA8_EXT, 0, 0,
						256, 256, 0);
					glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
						256, 256);
					{
						unsigned char test_pixels[262144];
						memset(test_pixels, 120, 262144);
						/*glTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0,
							256, 256,
							GL_RGBA, GL_UNSIGNED_BYTE, test_pixels);*/
						glTexImage2D(GL_TEXTURE_2D,(GLint)0,
							GL_RGBA8_EXT,
							256,
							256,(GLint)0,
							GL_RGBA,GL_UNSIGNED_BYTE, test_pixels);
					}
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE, "direct_render_Texture.  "
						"Only 2-D textures supported with DMBUFFER");
					return_code = 0;
				}
#else /* defined (SGI_DIGITAL_MEDIA) */
				display_message(ERROR_MESSAGE,"direct_render_Texture."
					"  Texture has type DMBUFFER but DIGITAL_MEDIA unavailable");
				return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
			} break;
			case TEXTURE_PBUFFER:
			{
#if defined (SGI_DIGITAL_MEDIA)
				if (texture->dimension == 2)
				{
					glPushAttrib(GL_PIXEL_MODE_BIT);
					Texture_get_type_and_format_from_storage_type(texture->storage,
						texture->number_of_bytes_per_component, &type, &format);
					glTexImage2D(GL_TEXTURE_2D,(GLint)0,
						GL_RGBA8_EXT,
						(GLint)(texture->width_texels),
						(GLint)(texture->height_texels),(GLint)0,
						format,type, NULL);
					glPixelTransferf(GL_ALPHA_BIAS, 1.0);
					glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
						texture->width_texels, texture->height_texels);
					glPopAttrib();
				}
				else
				{
					display_message(ERROR_MESSAGE, "direct_render_Texture.  "
						"Only 2-D textures supported with PBUFFER");
					return_code = 0;
				}
#else /* defined (SGI_DIGITAL_MEDIA) */
				display_message(ERROR_MESSAGE,"direct_render_Texture."
					"  Texture has type PBUFFER but DIGITAL_MEDIA unavailable");
				return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
			} break;
			default:
			{
				/* make each row of the image start on a 4-byte boundary */
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				Texture_get_type_and_format_from_storage_type(texture->storage,
					texture->number_of_bytes_per_component, &type, &format);
				number_of_components =
					Texture_storage_type_get_number_of_components(texture->storage);
				hardware_storage_format = Texture_get_hardware_storage_format
					(texture->compression_mode, number_of_components);
				if (0 < (reduction = Texture_get_hardware_reduction(texture)))
				{
					reduced_image = (unsigned char *)NULL;
					if (1 < reduction)
					{
						switch (texture->dimension)
						{
							case 1:
							{
								reduced_width_texels = texture->width_texels / reduction;
								reduced_height_texels = 1;
								reduced_depth_texels = 1;
								display_message(WARNING_MESSAGE,
									"1-D image %s is too large for this display.  "
									"Reducing width from %d to %d for display only",
									texture->name,
									texture->width_texels, reduced_width_texels); 
							} break;
							case 2:
							{
								reduced_width_texels = texture->width_texels / reduction;
								reduced_height_texels = texture->height_texels / reduction;
								reduced_depth_texels = 1;
								display_message(WARNING_MESSAGE,
									"Image %s is too large for this display.  "
									"Reducing (%d,%d) to (%d,%d) for display only",
									texture->name,
									texture->width_texels, texture->height_texels,
									reduced_width_texels, reduced_height_texels); 
							} break;
							case 3:
							{
								reduced_width_texels = texture->width_texels / reduction;
								reduced_height_texels = texture->height_texels / reduction;
								reduced_depth_texels = texture->depth_texels / reduction;
								display_message(WARNING_MESSAGE,
									"3-D image %s is too large for this display.  "
									"Reducing (%d,%d,%d) to (%d,%d,%d) for display only",
									texture->name,
									texture->width_texels, texture->height_texels,
									texture->depth_texels,
									reduced_width_texels, reduced_height_texels,
									reduced_depth_texels); 
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"direct_render_Texture.  Invalid texture dimension");
								return_code = 0;
							}
						}
						if (return_code)
						{
							if (!(reduced_image = Texture_get_resized_image(texture,
								reduced_width_texels, reduced_height_texels,
								reduced_depth_texels, texture->resize_filter_mode)))
							{
								display_message(ERROR_MESSAGE, "direct_render_Texture.  "
									"Could not reduce texture size for display");
								return_code = 0;
							}
						}
					}
					if (return_code)
					{
						switch (texture->dimension)
						{
							case 1:
							{
								if (reduced_image)
								{
									glTexImage1D(GL_TEXTURE_1D, (GLint)0,
										(GLint)hardware_storage_format,
										(GLint)reduced_width_texels, (GLint)0,
										format, type, (GLvoid *)reduced_image);
								}
								else
								{
									glTexImage1D(GL_TEXTURE_1D, (GLint)0,
										(GLint)hardware_storage_format,
										(GLint)(texture->width_texels), (GLint)0,
										format, type, (GLvoid *)(texture->image));
								}
							} break;
							case 2:
							{
								if (reduced_image)
								{
									glTexImage2D(GL_TEXTURE_2D, (GLint)0,
										(GLint)hardware_storage_format,
										(GLint)reduced_width_texels,
										(GLint)reduced_height_texels, (GLint)0,
										format, type, (GLvoid *)reduced_image);
								}
								else
								{
									glTexImage2D(GL_TEXTURE_2D, (GLint)0,
										(GLint)hardware_storage_format,
										(GLint)(texture->width_texels),
										(GLint)(texture->height_texels), (GLint)0,
										format, type, (GLvoid *)(texture->image));
								}
							} break;
							case 3:
							{
#if defined (GL_VERSION_1_2)
								if (reduced_image)
								{
									glTexImage3D(GL_TEXTURE_3D, (GLint)0,
										(GLint)hardware_storage_format,
										(GLint)reduced_width_texels,
										(GLint)reduced_height_texels,
										(GLint)reduced_depth_texels, (GLint)0,
										format, type, (GLvoid *)reduced_image);
								}
								else
								{
									glTexImage3D(GL_TEXTURE_3D, (GLint)0,	
										(GLint)hardware_storage_format,
										(GLint)(texture->width_texels),
										(GLint)(texture->height_texels),
										(GLint)(texture->depth_texels), (GLint)0,
										format, type, (GLvoid *)(texture->image));
								}
#else /* defined (GL_VERSION_1_2) */
								display_message(ERROR_MESSAGE,"direct_render_Texture.  "
								  "Not compiled with 3D texture support.");
								return_code=0;								
#endif /* defined (GL_VERSION_1_2) */
							} break;
						}
					}
					if (reduced_image)
					{
						DEALLOCATE(reduced_image);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Not enough hardware memory resources for texture %s",
						texture->name);
					return_code = 0;
				}
			} break;
		}
		switch (texture->wrap_mode)
		{
			case TEXTURE_CLAMP_WRAP:
			{
				glTexParameteri(texture_target,GL_TEXTURE_WRAP_S,GL_CLAMP);
				glTexParameteri(texture_target,GL_TEXTURE_WRAP_T,GL_CLAMP);
			} break;
			case TEXTURE_REPEAT_WRAP:
			{
				glTexParameteri(texture_target,GL_TEXTURE_WRAP_S,GL_REPEAT);
				glTexParameteri(texture_target,GL_TEXTURE_WRAP_T,GL_REPEAT);
			} break;
		}
		switch (texture->filter_mode)
		{
			case TEXTURE_LINEAR_FILTER:
			{
				glTexParameteri(texture_target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(texture_target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			} break;
			case TEXTURE_NEAREST_FILTER:
			{
				glTexParameteri(texture_target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(texture_target,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			} break;
		}
		values[0]=(texture->combine_colour).red;
		values[1]=(texture->combine_colour).green;
		values[2]=(texture->combine_colour).blue;
		values[3]=texture->combine_alpha;
		glTexParameterfv(texture_target,GL_TEXTURE_BORDER_COLOR,values);
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"direct_render_Texture.  Missing texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Texture */
#endif /* defined (OPENGL_API) */

int set_Texture_storage(struct Parse_state *state,void *enum_storage_void_ptr,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
A modifier function to set the texture storage type.
==============================================================================*/
{
	char *current_token, *storage_type_string;
	enum Texture_storage_type *storage_type_address, storage_type;
	int return_code;

	ENTER(set_Texture_storage);
	if (state && state->current_token && (!dummy_user_data))
	{
		if (storage_type_address=(enum Texture_storage_type *)enum_storage_void_ptr)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (STRING_TO_ENUMERATOR(Texture_storage_type)(
						state->current_token, &storage_type) &&
						Texture_storage_type_is_user_selectable(storage_type, (void *)NULL))
					{
						*storage_type_address = storage_type;
						return_code = shift_Parse_state(state, 1);
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE, "Invalid storage type %s",
							current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					/* write help */
					display_message(INFORMATION_MESSAGE, " ");
					storage_type = (enum Texture_storage_type)0;
					while ((storage_type_string =
						ENUMERATOR_STRING(Texture_storage_type)(storage_type)) &&
						Texture_storage_type_is_user_selectable(storage_type, (void *)NULL))
					{
						if (storage_type != (enum Texture_storage_type)0)
						{
							display_message(INFORMATION_MESSAGE,"|");
						}
						display_message(INFORMATION_MESSAGE, storage_type_string);
						if (storage_type == *storage_type_address)
						{
							display_message(INFORMATION_MESSAGE,"[%s]",
								storage_type_string);
						}
						storage_type++;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing texture storage type");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_storage.  Missing storage enum.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Texture_storage.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Texture_storage */

#if defined (SGI_MOVIE_FILE)
static int Texture_refresh(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Tells the texture it has changed, forcing it to send the manager message
MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER.
Don't want to copy the texture when we are doing intensive things like
playing movies.
???RC Review Manager Messages Here
==============================================================================*/
{
	int return_code;

	ENTER(Texture_refresh);
	if (texture)
	{
		/* display list is assumed to be current */
		if (texture->texture_manager&&IS_MANAGED(Texture)(texture,
			texture->texture_manager))
		{
			/*???RC these messages are meant to be sent before and after a change.
				Probably ok as can only have changed this scene before message sent */
			MANAGER_BEGIN_CHANGE(Texture)(texture->texture_manager,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Texture), texture);
			MANAGER_END_CHANGE(Texture)(texture->texture_manager);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Texture_refresh.  Texture not managed");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Texture_refresh.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_refresh */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (SGI_MOVIE_FILE)
static int Texture_movie_callback(struct X3d_movie *movie,
	int time, int time_scale, void *user_data)
/*******************************************************************************
LAST MODIFIED : 8 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Texture *texture;

	ENTER(Texture_movie_callback);
	USE_PARAMETER(time);
	USE_PARAMETER(time_scale);
	return_code=0;
	if (movie&&(texture=(struct Texture *)user_data))
	{
		return_code=1;
		switch(texture->storage)
		{
			case TEXTURE_DMBUFFER:
			case TEXTURE_PBUFFER:
			case TEXTURE_ABGR:
			{
				/* The buffers should be bound */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_movie_callback.  Invalid movie texture type");
				return_code=0;
			} break;
		}
		if (texture->display_list_current)
		{
			/* If something else has made the whole list invalid keep that state,
				otherwise we can set our special display_list_current state */
			texture->display_list_current=2;
		}
		Texture_refresh(texture);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_movie_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_movie_callback */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (SGI_MOVIE_FILE)
static int Texture_movie_destroy_callback(struct X3d_movie *movie,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 18 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Texture *texture;

	ENTER(Texture_movie_destroy_callback);
	return_code=0;
	if (movie&&(texture=(struct Texture *)user_data))
	{
		if (texture->movie == movie)
		{
			texture->movie = (struct X3d_movie *)NULL;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Texture_movie_destroy_callback.  "
				"The movie given isn't the current movie texture");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_movie_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_movie_destroy_callback */
#endif /* defined (SGI_MOVIE_FILE) */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_combine_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_combine_mode));
	switch (enumerator_value)
	{
		case TEXTURE_BLEND:
		{
			enumerator_string = "blend";
		} break;
		case TEXTURE_DECAL:
		{
			enumerator_string = "decal";
		} break;
		case TEXTURE_MODULATE:
		{
			enumerator_string = "modulate";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_combine_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_combine_mode)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_compression_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_compression_mode));
	switch (enumerator_value)
	{
		case TEXTURE_UNCOMPRESSED:
		{
			enumerator_string = "uncompressed";
		} break;
		case TEXTURE_COMPRESSED_UNSPECIFIED:
		{
			enumerator_string = "compressed_unspecified";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_compression_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_compression_mode)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_filter_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_filter_mode));
	switch (enumerator_value)
	{
		case TEXTURE_LINEAR_FILTER:
		{
			enumerator_string = "linear_filter";
		} break;
		case TEXTURE_NEAREST_FILTER:
		{
			enumerator_string = "nearest_filter";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_filter_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_filter_mode)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_resize_filter_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_resize_filter_mode));
	switch (enumerator_value)
	{
		case TEXTURE_RESIZE_LINEAR_FILTER:
		{
			enumerator_string = "resize_linear_filter";
		} break;
		case TEXTURE_RESIZE_NEAREST_FILTER:
		{
			enumerator_string = "resize_nearest_filter";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_resize_filter_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_resize_filter_mode)

int Texture_storage_type_is_user_selectable(
	enum Texture_storage_type storage, void *dummy)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns true if <storage> is one of the basic storage types that users can
select, ie. not a digital media buffer.
==============================================================================*/
{
	int return_code

	ENTER(Texture_storage_type_is_user_selectable);
	USE_PARAMETER(dummy);
	return_code =
		(TEXTURE_LUMINANCE == storage) ||
		(TEXTURE_LUMINANCE_ALPHA == storage) ||
		(TEXTURE_RGB == storage) ||
		(TEXTURE_RGBA == storage) ||
		(TEXTURE_ABGR == storage);
	LEAVE;

	return (return_code);
} /* Texture_storage_type_is_user_selectable */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_storage_type)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_storage_type));
	switch (enumerator_value)
	{
		case TEXTURE_LUMINANCE:
		{
			enumerator_string = "i";
		} break;
		case TEXTURE_LUMINANCE_ALPHA:
		{
			enumerator_string = "ia";
		} break;
		case TEXTURE_RGB:
		{
			enumerator_string = "rgb";
		} break;
		case TEXTURE_RGBA:
		{
			enumerator_string = "rgba";
		} break;
		case TEXTURE_ABGR:
		{
			enumerator_string = "abgr";
		} break;
		case TEXTURE_DMBUFFER:
		{
			enumerator_string = "dmbuffer";
		} break;
		case TEXTURE_PBUFFER:
		{
			enumerator_string = "pbuffer";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_storage_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_storage_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Texture_wrap_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Texture_wrap_mode));
	switch (enumerator_value)
	{
		case TEXTURE_CLAMP_WRAP:
		{
			enumerator_string = "clamp_wrap";
		} break;
		case TEXTURE_REPEAT_WRAP:
		{
			enumerator_string = "repeat_wrap";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Texture_wrap_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Texture_wrap_mode)

struct Texture *CREATE(Texture)(char *name)
/*******************************************************************************
LAST MODIFIED : 12 February 2002

DESCRIPTION :
Allocates memory and assigns fields for a texture.  Adds the texture to the list
of all textures.
???DB.  Trimming name ?
???DB.  Check if it already exists ?  Retrieve if it does already exist ?
==============================================================================*/
{
#if defined (OLD_CODE)
	static char *default_Texture_settings=
		"repeat nearest modulate colour 1 1 1 alpha 1";
	struct Parse_state *state;
#endif /* defined (OLD_CODE) */
	struct Texture *texture;

	ENTER(CREATE(Texture));
	/* allocate memory for structure */
	if (ALLOCATE(texture, struct Texture, 1)&&
		ALLOCATE(texture->image, unsigned char, 4))
	{
		if (name)
		{
			if (ALLOCATE(texture->name,char,strlen(name)+1))
			{
				strcpy(texture->name,name);
			}
		}
		else
		{
			if (ALLOCATE(texture->name,char,1))
			{
				*(texture->name)='\0';
			}
		}
		if (texture->name)
		{
			/* texture defaults to 1-D since it is a single texel */
			texture->dimension = 1;
			texture->depth=1.;
			texture->height=1.;
			texture->width=1.;
			/* assign image description fields */
			texture->image_file_name = (char *)NULL;
			/* file number pattern and ranges for 3-D textures */
			texture->file_number_pattern = (char *)NULL;
			texture->start_file_number = 0;
			texture->stop_file_number = 0;
			texture->file_number_increment = 0;

			texture->storage=TEXTURE_RGBA;
			texture->number_of_bytes_per_component=1;
			texture->depth_texels=1;
			texture->height_texels=1;
			texture->width_texels=1;
			texture->original_depth_texels=1;
			texture->original_height_texels=1;
			texture->original_width_texels=1;
			texture->distortion_centre_x=0.0;
			texture->distortion_centre_y=0.0;
			texture->distortion_factor_k1=0.0;
			(texture->image)[0]=0xFF;
			(texture->image)[1]=0xFF;
			(texture->image)[2]=0xFF;
			(texture->image)[3]=0xFF;
			texture->combine_mode=TEXTURE_DECAL;
			texture->compression_mode=TEXTURE_UNCOMPRESSED;
			texture->filter_mode=TEXTURE_NEAREST_FILTER;
			texture->resize_filter_mode=TEXTURE_RESIZE_NEAREST_FILTER;
			texture->wrap_mode=TEXTURE_REPEAT_WRAP;
			(texture->combine_colour).red=0.;
			(texture->combine_colour).green=0.;
			(texture->combine_colour).blue=0.;
			texture->combine_alpha=0.;
			texture->texture_manager = (struct MANAGER(Texture) *)NULL;
			texture->movie = (struct X3d_movie *)NULL;
#if defined (DM_BUFFERS)
			texture->dmbuffer = (struct Dm_buffer *)NULL;
#endif /* defined (DM_BUFFERS) */
#if defined (OPENGL_API)
			texture->display_list=0;
			texture->texture_id = 0;
#endif /* defined (OPENGL_API) */
			texture->display_list_current=0;
			texture->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Texture).  Insufficient memory for name");
			DEALLOCATE(texture->image);
			DEALLOCATE(texture);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Texture).  Insufficient memory for structure");
		DEALLOCATE(texture);
	}
	LEAVE;

	return (texture);
} /* CREATE(Texture) */

int DESTROY(Texture)(struct Texture **texture_address)
/*******************************************************************************
LAST MODIFIED : 7 February 2002

DESCRIPTION :
Frees the memory for the texture and sets <*texture_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Texture *texture;

	ENTER(DESTROY(Texture));
	if (texture_address)
	{
		if (texture= *texture_address)
		{
			if (texture->access_count<=0)
			{
				if(texture->movie)
				{
#if defined (SGI_MOVIE_FILE)
					switch(texture->storage)
					{
						case TEXTURE_PBUFFER:
						case TEXTURE_DMBUFFER:
						{
							if(texture->dmbuffer)
							{
								X3d_movie_unbind_from_dmbuffer(texture->movie);
							}
						} break;
						case TEXTURE_ABGR:
						{
							X3d_movie_unbind_from_image_buffer(texture->movie);
						} break;
					}
					X3d_movie_remove_callback(texture->movie,
						Texture_movie_callback, (void *)texture);
#else /* defined (SGI_MOVIE_FILE) */
					display_message(ERROR_MESSAGE,"DESTROY(Texture).  Movie unavailable but movie pointer found");
					return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
				}
#if defined (DM_BUFFERS)
				if(texture->dmbuffer)
				{
					DEACCESS(Dm_buffer)(&(texture->dmbuffer));
					return_code=0;
				}
#endif /* defined (DM_BUFFERS) */
#if defined (OPENGL_API)
				if (texture->display_list)
				{
					glDeleteLists(texture->display_list,1);
				}
				if (texture->texture_id)
				{
					glDeleteTextures(1, &(texture->texture_id));
				}
#endif /* defined (OPENGL_API) */
				DEALLOCATE(texture->name);
				if (texture->image_file_name)
				{
					DEALLOCATE(texture->image_file_name);
				}
				if (texture->file_number_pattern)
				{
					DEALLOCATE(texture->file_number_pattern);
				}
				DEALLOCATE(texture->image);
				DEALLOCATE(*texture_address);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Texture).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Texture) */

DECLARE_OBJECT_FUNCTIONS(Texture)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Texture)

DECLARE_INDEXED_LIST_FUNCTIONS(Texture)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Texture,name,char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Texture,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Texture,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Texture,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"MANAGER_COPY_WITH_IDENTIFIER(Texture,name).  Insufficient memory for name");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name)(destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Texture,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Texture,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Texture,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Texture,name)
{
	char *image_file_name;
	int image_size, number_of_components, return_code;
	unsigned char *destination_image;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name));
	if (source && destination)
	{
		if (source->image_file_name)
		{
			if (ALLOCATE(image_file_name,char,strlen(source->image_file_name)+1))
			{
				strcpy(image_file_name,source->image_file_name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name).  "
					"Insufficient memory for image file name");
				return_code=0;
			}
		}
		else
		{
			image_file_name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			number_of_components =
				Texture_storage_type_get_number_of_components(source->storage);
			image_size = source->depth_texels * source->height_texels * 4 *
				((source->width_texels * number_of_components *
					source->number_of_bytes_per_component + 3)/4);
			/*???RC Handling of access/deaccess/deallocate needs work here! */
			switch(source->storage)
			{
				case TEXTURE_DMBUFFER:
				case TEXTURE_PBUFFER:
				{
#if defined (SGI_DIGITAL_MEDIA)
					if (destination->image)
					{
						DEALLOCATE(destination->image)
					}
					ACCESS(Dm_buffer)(source->dmbuffer);
					if (destination->dmbuffer)
					{
						DEACCESS(Dm_buffer)(&(destination->dmbuffer));
					}
					destination->dmbuffer = source->dmbuffer;
#else /* defined (SGI_DIGITAL_MEDIA) */
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name)."
						"  Digital Media unavailable but source has type DM_BUFFER or PBUFFER");
					return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
				} break;
				default:
				{
	 				if ((0 < image_size) && REALLOCATE(destination_image,
						destination->image, unsigned char, image_size))
					{
						destination->image = destination_image;
						/* use memcpy to copy the image data - should be fastest method */
						memcpy((void *)destination->image,
							(void *)source->image, image_size);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name).  "
							"Insufficient memory for image");
						if (image_file_name)
						{
							DEALLOCATE(image_file_name);
						}
						return_code=0;
					}
				} break;
			}
		}
		if (return_code)
		{
			if (destination->image_file_name)
			{
				DEALLOCATE(destination->image_file_name);
			}
			destination->image_file_name=image_file_name;
			destination->height=source->height;
			destination->width=source->width;
			destination->distortion_centre_x=source->distortion_centre_x;
			destination->distortion_centre_y=source->distortion_centre_y;
			destination->distortion_factor_k1=source->distortion_factor_k1;
			destination->storage=source->storage;
			destination->number_of_bytes_per_component=source->number_of_bytes_per_component;
			destination->original_height_texels=source->original_height_texels;
			destination->original_width_texels=source->original_width_texels;
			destination->height_texels=source->height_texels;
			destination->width_texels=source->width_texels;
			destination->combine_mode=source->combine_mode;
			destination->compression_mode=source->compression_mode;
			destination->filter_mode=source->filter_mode;
			destination->wrap_mode=source->wrap_mode;
			(destination->combine_colour).red=(source->combine_colour).red;
			(destination->combine_colour).green=(source->combine_colour).green;
			(destination->combine_colour).blue=(source->combine_colour).blue;
			destination->combine_alpha=source->combine_alpha;
			/* flag destination display list as no longer current */
			destination->display_list_current=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Texture,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Texture,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Texture,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Texture,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Texture,name) */

DECLARE_MANAGER_FUNCTIONS(Texture)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Texture)

DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS( \
	Texture,name,char *,texture_manager)

#if defined (OLD_CODE)
	  /* Put the manager into scene */
DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Texture,name,char *)
#endif /* defined (OLD_CODE) */

int Texture_get_combine_alpha(struct Texture *texture,float *alpha)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the alpha value used for combining the texture.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_combine_alpha);
	if (texture&&alpha)
	{
		*alpha=texture->combine_alpha;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_combine_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_combine_alpha */

int Texture_set_combine_alpha(struct Texture *texture,float alpha)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the alpha value used for combining the texture.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_combine_alpha);
	if (texture&&(0.0<=alpha)&&(1.0>=alpha))
	{
		if (alpha != texture->combine_alpha)
		{
			texture->combine_alpha=alpha;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_combine_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_combine_alpha */

int Texture_get_combine_colour(struct Texture *texture,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the colour to be combined with the texture in blending combine mode.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_combine_colour);
	if (texture&&colour)
	{
		colour->red=texture->combine_colour.red;
		colour->green=texture->combine_colour.green;
		colour->blue=texture->combine_colour.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_combine_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_combine_colour */

int Texture_set_combine_colour(struct Texture *texture,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the colour to be combined with the texture in blending combine mode.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_combine_colour);
	if (texture&&colour)
	{
		texture->combine_colour.red=colour->red;
		texture->combine_colour.green=colour->green;
		texture->combine_colour.blue=colour->blue;
		/* display list needs to be compiled again */
		texture->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_combine_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_combine_colour */

enum Texture_combine_mode Texture_get_combine_mode(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/
{
	enum Texture_combine_mode combine_mode;

	ENTER(Texture_get_combine_mode);
	if (texture)
	{
		combine_mode = texture->combine_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_combine_mode.  Invalid argument(s)");
	}
	LEAVE;

	return (combine_mode);
} /* Texture_get_combine_mode */

int Texture_set_combine_mode(struct Texture *texture,
	enum Texture_combine_mode combine_mode)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Sets how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_combine_mode);
	if (texture&&((TEXTURE_BLEND==combine_mode)||
		(TEXTURE_DECAL==combine_mode)||(TEXTURE_MODULATE==combine_mode)))
	{
		if (combine_mode != texture->combine_mode)
		{
			texture->combine_mode = combine_mode;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_combine_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_combine_mode */

enum Texture_compression_mode Texture_get_compression_mode(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Returns how the texture is compressed.
==============================================================================*/
{
	enum Texture_compression_mode compression_mode;

	ENTER(Texture_get_compression_mode);
	if (texture)
	{
		compression_mode = texture->compression_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_compression_mode.  Invalid argument(s)");
	}
	LEAVE;

	return (compression_mode);
} /* Texture_get_compression_mode */

int Texture_set_compression_mode(struct Texture *texture,
	enum Texture_compression_mode compression_mode)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Sets how the texture is compressed.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_compression_mode);
	if (texture&&((TEXTURE_UNCOMPRESSED==compression_mode)||
		(TEXTURE_COMPRESSED_UNSPECIFIED==compression_mode)))
	{
		if (compression_mode != texture->compression_mode)
		{
			texture->compression_mode = compression_mode;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_compression_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_compression_mode */

enum Texture_filter_mode Texture_get_filter_mode(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the texture filter: linear or nearest.
==============================================================================*/
{
	enum Texture_filter_mode filter_mode;

	ENTER(Texture_get_filter_mode);
	if (texture)
	{
		filter_mode = texture->filter_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_filter_mode.  Invalid argument(s)");
	}
	LEAVE;

	return (filter_mode);
} /* Texture_get_filter_mode */

int Texture_set_filter_mode(struct Texture *texture,
	enum Texture_filter_mode filter_mode)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Sets the texture filter: linear or nearest.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_filter_mode);
	if (texture&&((TEXTURE_LINEAR_FILTER==filter_mode)||
		(TEXTURE_NEAREST_FILTER==filter_mode)))
	{
		if (filter_mode != texture->filter_mode)
		{
			texture->filter_mode = filter_mode;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_filter_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_filter_mode */

enum Texture_resize_filter_mode Texture_get_resize_filter_mode(
	struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the texture filter: linear or nearest.
==============================================================================*/
{
	enum Texture_resize_filter_mode resize_filter_mode;

	ENTER(Texture_get_resize_filter_mode);
	if (texture)
	{
		resize_filter_mode = texture->resize_filter_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_resize_filter_mode.  Invalid argument(s)");
	}
	LEAVE;

	return (resize_filter_mode);
} /* Texture_get_resize_filter_mode */

int Texture_set_resize_filter_mode(struct Texture *texture,
	enum Texture_resize_filter_mode resize_filter_mode)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Sets the texture filter: linear or nearest.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_resize_filter_mode);
	if (texture&&((TEXTURE_RESIZE_LINEAR_FILTER==resize_filter_mode)||
		(TEXTURE_RESIZE_NEAREST_FILTER==resize_filter_mode)))
	{
		if (resize_filter_mode != texture->resize_filter_mode)
		{
			texture->resize_filter_mode = resize_filter_mode;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_resize_filter_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_resize_filter_mode */

int Texture_allocate_image(struct Texture *texture,
	int width, int height, int depth, enum Texture_storage_type storage,
	int number_of_bytes_per_component, char *source_name)
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
{
	int bytes_per_pixel, dimension, padded_width_bytes, number_of_components,
		return_code, texture_depth, texture_height, texture_width;
	unsigned char *texture_image;

	ENTER(Texture_allocate_image);
	if (texture && (0 < width) && (0 < height) && (0 < depth) &&
		(0 < (number_of_components =
			Texture_storage_type_get_number_of_components(storage))) &&
		((1 == number_of_bytes_per_component) ||
			(2 == number_of_bytes_per_component)))
	{
		/* ensure width is a power of 2 */
		texture_width = 1;
		while (texture_width < width)
		{
			texture_width *= 2;
		}
		/* ensure height is a power of 2 */
		texture_height = 1;
		while (texture_height < height)
		{
			texture_height *= 2;
		}
		/* ensure depth is a power of 2 */
		texture_depth = 1;
		while (texture_depth < depth)
		{
			texture_depth *= 2;
		}
		if (1 < texture_depth)
		{
			dimension = 3;
		}
		else if (1 < texture_height)
		{
			dimension = 2;
		}
		else
		{
			dimension = 1;
		}
		if ((width != texture_width) ||
			(height != texture_height) ||
			(depth != texture_depth))
		{
			switch (dimension)
			{
				case 1:
				{
					display_message(WARNING_MESSAGE,
						"image width is not a power of 2.  "
						"Extending (%d) to (%d)", width, texture_width);
				} break;
				case 2:
				{
					display_message(WARNING_MESSAGE,
						"image width and/or height not powers of 2.  "
						"Extending (%d,%d) to (%d,%d)", width, height,
						texture_width, texture_height);
				} break;
				case 3:
				{
					display_message(WARNING_MESSAGE,
						"image width, height and/or depth not powers of 2.  "
						"Extending (%d,%d,%d) to (%d,%d,%d)", width, height, depth,
						texture_width, texture_height, texture_depth);
				} break;
			}
		}
		bytes_per_pixel = number_of_components * number_of_bytes_per_component;
		padded_width_bytes = 4*((texture_width*bytes_per_pixel + 3)/4);
		/* reallocate existing texture image to save effort */
		if (REALLOCATE(texture_image, texture->image, unsigned char,
			texture_depth*texture_height*padded_width_bytes))
		{
			texture->image = texture_image;
			/* fill the image with zeros */
			memset(texture_image, 0, texture_depth*texture_height*padded_width_bytes);
			/* assign values in the texture */
			texture->dimension = dimension;
			texture->storage = storage;
			texture->number_of_bytes_per_component =
				number_of_bytes_per_component;
			/* original size is intended to specify useful part of texture */
			texture->original_width_texels = width;
			texture->original_height_texels = height;
			texture->original_depth_texels = depth;
			texture->width_texels = texture_width;
			texture->height_texels = texture_height;
			texture->depth_texels = texture_depth;
			if (texture->image_file_name)
			{
				DEALLOCATE(texture->image_file_name);
			}
			if (source_name)
			{
				texture->image_file_name = duplicate_string(source_name);
			}
			else
			{
				texture->image_file_name = (char *)NULL;
			}
			texture->file_number_pattern = (char *)NULL;
			texture->start_file_number = 0;
			texture->stop_file_number = 0;
			texture->file_number_increment = 0;
			texture->crop_left_margin = 0;
			texture->crop_bottom_margin = 0;
			texture->crop_width = 0;
			texture->crop_height = 0;
			/* display list needs to be compiled again */
			texture->display_list_current = 0;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_allocate_image.  Could not reallocate texture image");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_allocate_image.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_allocate_image */

struct Cmgui_image *Texture_get_image(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Creates and returns a Cmgui_image from the image in <texture>, usually for
writing. Depth planes are stored as subimages of the returned structure.
Up to the calling function to DESTROY the returned Cmgui_image.
==============================================================================*/
{
	int bytes_per_pixel, i, number_of_components, return_code, width_bytes;
	unsigned char *source;
	struct Cmgui_image *cmgui_image, *next_cmgui_image;

	ENTER(Texture_get_image);
	cmgui_image = (struct Cmgui_image *)NULL;
	if (texture && (0 < (number_of_components =
		Texture_storage_type_get_number_of_components(texture->storage))) &&
		(0 < (bytes_per_pixel =
			number_of_components*texture->number_of_bytes_per_component)))
	{
		return_code = 1;
		width_bytes = 4*((texture->width_texels*bytes_per_pixel + 3)/4);
		source = texture->image;
		for (i = 0; (i < texture->original_depth_texels) && return_code; i++)
		{
			if (next_cmgui_image = Cmgui_image_constitute(
				texture->original_width_texels, texture->original_height_texels,
				number_of_components, texture->number_of_bytes_per_component,
				width_bytes, source))
			{
				if (cmgui_image)
				{
					/* following "swallows" next_cmgui_image */
					if (!Cmgui_image_append(cmgui_image, &next_cmgui_image))
					{
						display_message(ERROR_MESSAGE,
							"Texture_get_image.  Could not append image");
						return_code = 0;
					}
				}
				else
				{
					/* first image */
					cmgui_image = next_cmgui_image;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_image.  Could not constitute image");
				return_code = 0;
			}
			source += texture->height_texels*width_bytes;
		}
		if (!return_code)
		{
			if (cmgui_image)
			{
				DESTROY(Cmgui_image)(&cmgui_image);
				cmgui_image = (struct Cmgui_image *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Texture_get_image.  Invalid texture");
	}
	LEAVE;

	return (cmgui_image);
} /* Texture_get_image */

int Texture_set_image(struct Texture *texture,
	struct Cmgui_image *cmgui_image,
	char *image_file_name, char *file_number_pattern,
	int start_file_number, int stop_file_number, int file_number_increment,
	int	crop_left, int crop_bottom, int crop_width, int crop_height)
/*******************************************************************************
LAST MODIFIED : 14 March 2002

DESCRIPTION :
Puts <cmgui_image> into <texture>. If the <cmgui_image> contains
more than one image, these are put together into a 3-D texture. All other
parameters are merely recorded with the texture to be able to reproduce the
command for it later. The exception is the crop parameters, which must
which are used to cut the image size if <crop_width> and <crop_height> are
positive. Cropping is not available in the depth direction.
==============================================================================*/
{
	static unsigned char fill_byte = 0;
	enum Texture_storage_type storage;
	int bytes_per_pixel, dimension, padded_width_bytes,
		final_bottom, final_depth, final_height, final_left, final_width,
		image_height, image_width, k, number_of_bytes_per_component,
		number_of_components, number_of_images, image_padding_bytes,
		return_code, texture_depth, texture_height, texture_width;
	unsigned char *texture_image;
	unsigned char *destination;

	ENTER(Texture_set_image);
	if (texture && cmgui_image &&
		(0 < (image_width = Cmgui_image_get_width(cmgui_image))) &&
		(0 < (image_height = Cmgui_image_get_height(cmgui_image))) &&
		(0 < (number_of_components =
			Cmgui_image_get_number_of_components(cmgui_image))) &&
		(0 < (number_of_bytes_per_component =
			Cmgui_image_get_number_of_bytes_per_component(cmgui_image))) &&
		(0 < (number_of_images = Cmgui_image_get_number_of_images(cmgui_image))))
	{
		return_code = 1;
		if ((0 == crop_left) && (0 == crop_width) &&
			(0 == crop_bottom) && (0 == crop_height))
		{
			final_width = image_width;
			final_height = image_height;
			final_left = 0;
			final_bottom = 0;
		}
		else if ((0 <= crop_left) && (0 < crop_width) &&
			(crop_left + crop_width <= image_width) &&
			(0 <= crop_bottom) && (0 < crop_height) &&
			(crop_bottom + crop_height <= image_height))
		{
			final_width = crop_width;
			final_height = crop_height;
			final_left = crop_left;
			final_bottom = crop_bottom;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_set_image.  Invalid cropping parameters");
			return_code = 0;
		}
		final_depth = number_of_images;
		switch (number_of_components)
		{
			case 1:
			{
				storage = TEXTURE_LUMINANCE;
			} break;
			case 2:
			{
				storage = TEXTURE_LUMINANCE_ALPHA;
			} break;
			case 3:
			{
				storage = TEXTURE_RGB;
			} break;
			case 4:
			{
				storage = TEXTURE_RGBA;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_image.  Invalid number_of_components");
				return_code = 0;
			} break;
		}
		if (return_code)
		{
			/* ensure width is a power of 2 */
			texture_width = 1;
			while (texture_width < final_width)
			{
				texture_width *= 2;
			}
			/* ensure height is a power of 2 */
			texture_height = 1;
			while (texture_height < final_height)
			{
				texture_height *= 2;
			}
			/* ensure depth is a power of 2 */
			texture_depth = 1;
			while (texture_depth < final_depth)
			{
				texture_depth *= 2;
			}
			if (1 < texture_depth)
			{
				dimension = 3;
			}
			else if (1 < texture_height)
			{
				dimension = 2;
			}
			else
			{
				dimension = 1;
			}
			if ((final_width != texture_width) ||
				(final_height != texture_height) ||
				(final_depth != texture_depth))
			{
				switch (dimension)
				{
					case 1:
					{
						display_message(WARNING_MESSAGE,
							"image width is not a power of 2.  "
							"Extending (%d) to (%d)", final_width, texture_width);
					} break;
					case 2:
					{
						display_message(WARNING_MESSAGE,
							"image width and/or height not powers of 2.  "
							"Extending (%d,%d) to (%d,%d)",final_width, final_height,
							texture_width, texture_height);
					} break;
					case 3:
					{
						display_message(WARNING_MESSAGE,
							"image width, height and/or depth not powers of 2.  "
							"Extending (%d,%d,%d) to (%d,%d,%d)",
							final_width, final_height, final_depth,
							texture_width, texture_height, texture_depth);
					} break;
				}
			}
			/* ensure texture images are row aligned to 4-byte boundary */
			bytes_per_pixel = number_of_components * number_of_bytes_per_component;
			padded_width_bytes =
				4*((texture_width*bytes_per_pixel + 3)/4);
			image_padding_bytes =
				padded_width_bytes*(texture_height - final_height);
			if (ALLOCATE(texture_image, unsigned char,
				texture_depth*texture_height*padded_width_bytes))
			{
				destination = texture_image;
				for (k = 0; (k < final_depth) && return_code; k++)
				{
					/* fill image from bottom to top */
					return_code = Cmgui_image_dispatch(cmgui_image, /*image_number*/k,
						final_left, final_bottom, final_width, final_height,
						padded_width_bytes, /*number_of_fill_bytes*/1, &fill_byte,
						destination);
					destination += padded_width_bytes * final_height;
					if (0 < image_padding_bytes)
					{
						/* fill the padding rows of the image */
						memset(destination, fill_byte, image_padding_bytes);
						destination += image_padding_bytes;
					}
				}
				if (final_depth < texture_depth)
				{
					memset(destination, fill_byte, padded_width_bytes *
						texture_height * (texture_depth - final_depth));
				}

				/* assign values in the texture */
				texture->dimension = dimension;
				texture->storage = storage;
				texture->number_of_bytes_per_component =
					number_of_bytes_per_component;
				/* original size is intended to specify useful part of texture */
				texture->original_width_texels = final_width;
				texture->original_height_texels = final_height;
				texture->original_depth_texels = final_depth;
				texture->width_texels = texture_width;
				texture->height_texels = texture_height;
				texture->depth_texels = texture_depth;
				DEALLOCATE(texture->image);
				texture->image = texture_image;
				if (texture->image_file_name)
				{
					DEALLOCATE(texture->image_file_name);
				}
				if (image_file_name)
				{
					texture->image_file_name = duplicate_string(image_file_name);
				}
				else
				{
					texture->image_file_name = (char *)NULL;
				}
				if (texture->file_number_pattern)
				{
					DEALLOCATE(texture->file_number_pattern);
				}
				if (file_number_pattern)
				{
					texture->file_number_pattern =
						duplicate_string(file_number_pattern);
				}
				else
				{
					texture->file_number_pattern = (char *)NULL;
				}
				texture->start_file_number = start_file_number;
				texture->stop_file_number = stop_file_number;
				texture->file_number_increment = file_number_increment;
				texture->crop_left_margin = crop_left;
				texture->crop_bottom_margin = crop_bottom;
				texture->crop_width = crop_width;
				texture->crop_height = crop_height;
				/* display list needs to be compiled again */
				texture->display_list_current = 0;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_image.  Could not allocate texture image");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Texture_set_image.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_image */

int Texture_set_image_block(struct Texture *texture,
	int left, int bottom, int width, int height, int depth_plane,
	int source_width_bytes, unsigned char *source_pixels)
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
{
	int bytes_per_pixel, copy_width, i, number_of_components, return_code,
		width_bytes;
	unsigned char *destination, *source;

	ENTER(Texture_set_image_block);
	if (texture && (0 <= left) && (0 < width) &&
		(left + width <= texture->width_texels) &&
		(0 <= bottom) && (0 < height) &&
		(bottom + height <= texture->height_texels) &&
		(0 <= depth_plane) && (depth_plane < texture->depth_texels) &&
		(0 < (number_of_components =
			Texture_storage_type_get_number_of_components(texture->storage))) &&
		(0 < (bytes_per_pixel =
			number_of_components*texture->number_of_bytes_per_component)) &&
		(width*bytes_per_pixel <= source_width_bytes) &&
		source_pixels)
	{
		width_bytes = 4*((texture->width_texels*bytes_per_pixel + 3)/4);
		copy_width = width*bytes_per_pixel;
		destination = texture->image +
			(depth_plane*height + bottom)*width_bytes + left*bytes_per_pixel;
		source = source_pixels;
		for (i = 0; i < height; i++)
		{
			memcpy(destination, source, copy_width);
			destination += width_bytes;
			source += source_width_bytes;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_image_block.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_image_block */

struct X3d_movie *Texture_get_movie(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current X3d_movie from the texture.
==============================================================================*/
{
	struct X3d_movie *movie;
	ENTER(Texture_get_movie);
	if (texture)
	{
		movie=texture->movie;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Texture_get_movie.  Invalid argument");
		movie=(struct X3d_movie *)NULL;
	}
	LEAVE;

	return (movie);
} /* Texture_get_movie */

int Texture_set_movie(struct Texture *texture,struct X3d_movie *movie,
	struct User_interface *user_interface, char *image_file_name)
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	char *temp_file_name;
	unsigned char *texture_image;
	enum Dm_buffer_type dm_buffer_type;
	enum Texture_storage_type storage;
	int destination_row_width_bytes, dimension, i, image_height, image_width,
		number_of_components;
	long int texture_height,texture_width;
	struct Dm_buffer *dmbuffer;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(Texture_set_movie);
#if defined (SGI_MOVIE_FILE)
	if (texture&&movie&&image_file_name)
	{
		/* only 2-D movie textures supported */
		dimension = 2;
		X3d_movie_get_bounding_rectangle(movie, (int *)NULL, (int *)NULL,
			&image_width, &image_height);

		/* width and height must be powers of 2 */
		i=image_width;
		texture_width=1;
		while (i>1)
		{
			texture_width *= 2;
			i /= 2;
		}
		if (texture_width<image_width)
		{
			texture_width *= 2;
		}
		i=image_height;
		texture_height=1;
		while (i>1)
		{
			texture_height *= 2;
			i /= 2;
		}
		if (texture_height<image_height)
		{
			texture_height *= 2;
		}
		if ((image_width != texture_width)||(image_height != texture_height))
		{
			display_message(WARNING_MESSAGE,
				"movie width and/or height not powers of 2.  "
				"Extending (%d,%d) to (%d,%d)",image_width,image_height,texture_width,
				texture_height);
		}
#if defined (SGI_DIGITAL_MEDIA)
		/* If dm_buffers are available then use them, 
		 otherwise default to normal texture storage*/
		if(texture->dmbuffer)
		{
			DEACCESS(Dm_buffer)(&(texture->dmbuffer));
		}
		dmbuffer = CREATE(Dm_buffer)(texture_width, texture_height, /*depth_buffer_flag*/0,
			/*shared_display_buffer*/1, user_interface);
		if(dmbuffer && (ACCESS(Dm_buffer)(dmbuffer))
			&& (DM_BUFFER_INVALID_TYPE != (dm_buffer_type = 
			Dm_buffer_get_type(dmbuffer))))
		{
			texture->dmbuffer = dmbuffer;
			DEALLOCATE(texture->image);
			texture->image = (void *)NULL;

			X3d_movie_bind_to_dmbuffer(movie, 
				texture->dmbuffer);

			switch(dm_buffer_type)
			{
				case DM_BUFFER_DM_PBUFFER:
				{
					texture->storage = TEXTURE_DMBUFFER;
				} break;
				case DM_BUFFER_GLX_PBUFFER:
				{
					texture->storage = TEXTURE_PBUFFER;
				} break;
			}

			Dm_buffer_glx_make_current(texture->dmbuffer);
			glClearColor(0.0,0.0,0.0,1.0);
			glClearDepth(1.0);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			X3d_movie_render_to_glx(movie, 0, 15);
			X3dThreeDDrawingRemakeCurrent();

			X3d_movie_add_callback(movie, Texture_movie_callback, (void *)texture);
			X3d_movie_add_destroy_callback(movie,
				Texture_movie_destroy_callback, (void *)texture);

			if (ALLOCATE(temp_file_name,char,strlen(image_file_name)+1))
			{
				texture->dimension = dimension;
				texture->original_width_texels=image_width;
				texture->original_height_texels=image_height;
				texture->height_texels=texture_height;
				texture->width_texels=texture_width;
				strcpy(temp_file_name,image_file_name);
				texture->image_file_name=temp_file_name;
				texture->crop_left_margin=0;
				texture->crop_bottom_margin=0;
				texture->crop_width=image_width;
				texture->crop_height=image_width;
				/* display list needs to be compiled again */
				texture->display_list_current=0;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_movie.  Could not allocate image_file_name");
				return_code=0;
			}
		}
		else
#endif /* defined (SGI_DIGITAL_MEDIA) */
		{
			if (query_gl_extension("GL_EXT_abgr"))
			{
				texture->movie = movie;
				storage = TEXTURE_ABGR; /* This is the default format for frames from
																	 the movie library */
				number_of_components = 4;

				/* ensure texture images row aligned to 4-byte boundary */
				destination_row_width_bytes =
					4*((int)((texture_width*number_of_components+3)/4));
				if (ALLOCATE(texture_image, unsigned char,
					texture_height*destination_row_width_bytes))
				{
					X3d_movie_bind_to_image_buffer(movie, texture_image, 
						image_width, image_height, texture_width - image_width);

					memset((void *)texture_image,0x00,
						destination_row_width_bytes*texture_height);
					X3d_movie_render_to_image_buffer(movie, texture_image, 
						image_width, image_height, texture_width - image_width,
						0, 15);

					X3d_movie_add_callback(movie, Texture_movie_callback, (void *)texture);
					X3d_movie_add_destroy_callback(movie,
						Texture_movie_destroy_callback, (void *)texture);

					if (ALLOCATE(temp_file_name,char,strlen(image_file_name)+1))
					{
						strcpy(temp_file_name,image_file_name);
						/* assign values */
						texture->dimension = dimension;
						texture->storage = storage;
						/* original size is intended to specify useful part of texture */
						texture->original_width_texels=image_width;
						texture->original_height_texels=image_height;
						texture->height_texels=texture_height;
						texture->width_texels=texture_width;
						DEALLOCATE(texture->image);
						texture->image=texture_image;
						if (texture->image_file_name)
						{
							DEALLOCATE(texture->image_file_name);
						}
						texture->image_file_name=temp_file_name;
						texture->crop_left_margin=0;
						texture->crop_bottom_margin=0;
						texture->crop_width=image_width;
						texture->crop_height=image_width;
						/* display list needs to be compiled again */
						texture->display_list_current=0;
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Texture_set_movie.  Could not allocate image_file_name");
						DEALLOCATE(texture_image);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Texture_set_movie.  Could not allocate texture image");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_movie.  ABGR format not supported in this OpenGL");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_movie.  Invalid argument(s)");
		return_code=0;
	}
#else /* defined (SGI_MOVIE_FILE) */
	USE_PARAMETER(texture);
	USE_PARAMETER(movie);
	USE_PARAMETER(user_interface);
	USE_PARAMETER(image_file_name);
	display_message(ERROR_MESSAGE,
		"Texture_set_movie.  Movie textures not available");
	return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
	LEAVE;

	return (return_code);
} /* Texture_set_movie */

int Texture_get_raw_pixel_values(struct Texture *texture,int x,int y,int z,
	unsigned char *values)
/*******************************************************************************
LAST MODIFIED : 21 March 2002

DESCRIPTION :
Returns the byte values in the texture at x,y,z.
==============================================================================*/
{
	int i,number_of_bytes, return_code,row_width_bytes;
	unsigned char *pixel_ptr;

	ENTER(Texture_get_raw_pixel_values);
	if (texture&&(0<=x)&&(x<texture->original_width_texels)&&
		(0<=y)&&(y<texture->original_height_texels)&&
		(0<=z)&&(z<texture->original_depth_texels)&&values)
	{
		number_of_bytes = Texture_storage_type_get_number_of_components(texture->storage)
			* texture->number_of_bytes_per_component;
		row_width_bytes=
			((int)(texture->width_texels*number_of_bytes+3)/4)*4;
		pixel_ptr=(unsigned char *)(texture->image);
		pixel_ptr += ((z*texture->height_texels + y)*row_width_bytes+x*number_of_bytes);
		for (i=0;i<number_of_bytes;i++)
		{
			values[i]=pixel_ptr[i];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_raw_pixel_values.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_raw_pixel_values */

int Texture_get_pixel_values(struct Texture *texture,
	double x, double y, double z, double *values)
/*******************************************************************************
LAST MODIFIED : 16 April 2002

DESCRIPTION :
Returns the byte values in the texture using the texture coordinates relative
to the physical size.  Each texel is assumed to apply exactly
at its centre and the filter_mode used to determine whether the pixels are
interpolated or not.  When closer than half a texel to a boundary the colour 
is constant from the half texel location to the edge. 
==============================================================================*/
{
	double local_xi[3], max_v, pos[3], weight, weight_i, weight_j, weight_k, v;
	int bytes_per_pixel, component_max, dimension, high_offset[3], i, j, k,
		low_offset[3], max_i, max_j, max_k, n, number_of_bytes_per_component,
		number_of_components, offset, offset_i, offset_j, offset_k, return_code,
		row_width_bytes, size[3], v_i, x_i, y_i, z_i;
	unsigned char *pixel_ptr;
	unsigned short short_value;

	ENTER(Texture_get_pixel_values);
	if (texture && values)
	{
		return_code = 1;
		number_of_components =
			Texture_storage_type_get_number_of_components(texture->storage);
		number_of_bytes_per_component = texture->number_of_bytes_per_component;
		bytes_per_pixel = number_of_components*number_of_bytes_per_component;
		row_width_bytes=
			((int)(texture->width_texels*bytes_per_pixel+3)/4)*4;

		/* make x, y and z range from 0.0 to 1.0 over full texture size */
		x *= ((double)texture->original_width_texels /
			(double)texture->width_texels) / texture->width;
		y *= ((double)texture->original_height_texels /
			(double)texture->height_texels) / texture->height;
		z *= ((double)texture->original_depth_texels /
			(double)texture->depth_texels) / texture->depth;
		switch (texture->wrap_mode)
		{
			case TEXTURE_CLAMP_WRAP:
			{
				if (x < 0.0)
				{
					x = 0.0;
				}
				else if (x > 1.0)
				{
					x = 1.0;
				}
				if (y < 0.0)
				{
					y = 0.0;
				}
				else if (y > 1.0)
				{
					y = 1.0;
				}
				if (z < 0.0)
				{
					z = 0.0;
				}
				else if (z > 1.0)
				{
					z = 1.0;
				}
			} break;
			case TEXTURE_REPEAT_WRAP:
			{
				x -= floor(x);
				y -= floor(y);
				z -= floor(z);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_pixel_values.  Unknown wrap type");
				return_code = 0;
			} break;
		}
		if (2 == number_of_bytes_per_component)
		{
			component_max = 65535;
		}
		else
		{
			component_max = 255;
		}
		x *= (double)(texture->width_texels);
		y *= (double)(texture->height_texels);
		z *= (double)(texture->depth_texels);
		switch (texture->filter_mode)
		{
			case TEXTURE_LINEAR_FILTER:
			{
				dimension = texture->dimension;
				pos[0] = x;
				pos[1] = y;
				pos[2] = z;
				size[0] = texture->width_texels;
				size[1] = texture->height_texels;
				size[2] = texture->depth_texels;
				offset = bytes_per_pixel;
				for (i = 0; i < dimension; i++)
				{
					max_v = (double)size[i] - 0.5;
					v = pos[i];
					if ((0.5 <= v) && (v < max_v))
					{
						v_i = (int)(v - 0.5);
						local_xi[i] = v - 0.5 - (double)v_i;
						low_offset[i] = v_i*offset;
						high_offset[i] = (v_i + 1)*offset;
					}
					else
					{
						/* result depends on the wrap mode */
						low_offset[i] = (size[i] - 1)*offset;
						high_offset[i] = 0;
						switch (texture->wrap_mode)
						{
							case TEXTURE_CLAMP_WRAP:
							{
								if (v < 0.5)
								{
									local_xi[i] = 1.0;
								}
								else
								{
									local_xi[i] = 0.0;
								}
							} break;
							case TEXTURE_REPEAT_WRAP:
							{
								if (v < 0.5)
								{
									local_xi[i] = v + 0.5;
								}
								else
								{
									local_xi[i] = v - max_v;
								}
							} break;
						}
					}
					offset *= size[i];
				}

				max_i = 2;
				if (1 < dimension)
				{
					max_j = 2;
				}
				else
				{
					max_j = 1;
				}
				if (2 < dimension)
				{
					max_k = 2;
				}
				else
				{
					max_k = 1;
				}

				for (i = 0; i < number_of_components; i++)
				{
					values[i] = 0.0;
				}

				for (k = 0; k < max_k; k++)
				{
					if (2 < dimension)
					{
						if (0 == k)
						{
							weight_k = (1.0 - local_xi[2]);
							offset_k = low_offset[2];
						}
						else
						{
							weight_k = local_xi[2];
							offset_k = high_offset[2];
						}
					}
					else
					{
						weight_k = 1.0;
						offset_k = 0;
					}
					for (j = 0; j < max_j; j++)
					{
						if (1 < dimension)
						{
							if (0 == j)
							{
								weight_j = weight_k*(1.0 - local_xi[1]);
								offset_j = offset_k + low_offset[1];
							}
							else
							{
								weight_j = weight_k*local_xi[1];
								offset_j = offset_k + high_offset[1];
							}
						}
						else
						{
							weight_j = 1.0;
							offset_j = 0;
						}
						for (i = 0; i < max_i; i++)
						{
							if (0 == i)
							{
								weight_i = weight_j*(1.0 - local_xi[0]);
								offset_i = offset_j + low_offset[0];
							}
							else
							{
								weight_i = weight_j*local_xi[0];
								offset_i = offset_j + high_offset[0];
							}
							pixel_ptr = texture->image + offset_i;
							weight = weight_i / component_max;
							for (n = 0; n < number_of_components; n++)
							{
								if (2 == number_of_bytes_per_component)
								{
#if (1234==BYTE_ORDER)
									short_value =
										(((unsigned short)(*(pixel_ptr + 1))) << 8) - (*pixel_ptr);
#else /* (1234==BYTE_ORDER) */
									short_value = 
										(((unsigned short)(*pixel_ptr)) << 8) - (*(pixel_ptr + 1));
#endif /* (1234==BYTE_ORDER) */
									values[n] += (double)short_value * weight;
								}
								else
								{
									values[n] += (double)(*pixel_ptr) * weight;
								}
								pixel_ptr += number_of_bytes_per_component;
							}
						}
					}
				}
			} break;
			case TEXTURE_NEAREST_FILTER:
			{
				x_i = (int)x;
				y_i = (int)y;
				z_i = (int)z;
				if (TEXTURE_CLAMP_WRAP == texture->wrap_mode)
				{
					/* fix problem of value being exactly on upper boundary */
					if (x_i == texture->width_texels)
					{
						x_i--;
					}
					if (y_i == texture->height_texels)
					{
						y_i--;
					}
					if (z_i == texture->depth_texels)
					{
						z_i--;
					}
				}
				offset = (z_i*texture->height_texels + y_i)*row_width_bytes +
					x_i*bytes_per_pixel;
				pixel_ptr = texture->image + offset;
				for (n = 0; n < number_of_components; n++)
				{
					if (2 == number_of_bytes_per_component)
					{
#if (1234==BYTE_ORDER)
						short_value =
							(((unsigned short)(*(pixel_ptr + 1))) << 8) - (*pixel_ptr);
#else /* (1234==BYTE_ORDER) */
						short_value = 
							(((unsigned short)(*pixel_ptr)) << 8) - (*(pixel_ptr + 1));
#endif /* (1234==BYTE_ORDER) */
						values[n] = (double)short_value / component_max;
					}
					else
					{
						values[n] = (double)(*pixel_ptr) / component_max;
					}
					pixel_ptr += number_of_bytes_per_component;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_pixel_values.  Unknown filter type");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_pixel_values.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_pixel_values */

char *Texture_get_image_file_name(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the name of the file from which the current texture image was read.
Note: returned file_number_pattern may be NULL if not set yet.
User must not modify the returned value!
==============================================================================*/
{
	char *image_file_name;

	ENTER(Texture_get_image_file_name);
	if (texture)
	{
		image_file_name = texture->image_file_name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_image_file_name.  Missing texture");
		image_file_name = (char *)NULL;
	}
	LEAVE;

	return (image_file_name);
} /* Texture_get_image_file_name */

char *Texture_get_file_number_pattern(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the file number pattern substituted in the image_file_name with the
image_file_number when 3-D image series are read in.
Note: returned file_number_pattern may be NULL if not set yet.
User must not modify the returned value!
==============================================================================*/
{
	char *file_number_pattern;

	ENTER(Texture_get_file_number_pattern);
	if (texture)
	{
		file_number_pattern = texture->file_number_pattern;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_file_number_pattern.  Missing texture");
		file_number_pattern = (char *)NULL;
	}
	LEAVE;

	return (file_number_pattern);
} /* Texture_get_file_number_pattern */

int Texture_get_file_number_series(struct Texture *texture,
	int *start_file_number, int *stop_file_number, int *file_number_increment)
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Returns the start, stop and increment of file_numbers which together with the
image_file_name and file_number_pattern determine the files read in for
3-D textures. The returned values are meaningless if the texture is not
three dimensional = depth_texels greater than 1.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_file_number_series);
	if (texture && start_file_number && stop_file_number && file_number_increment)
	{
		*start_file_number = texture->start_file_number;
		*stop_file_number = texture->stop_file_number;
		*file_number_increment = texture->file_number_increment;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_file_number_series.  Missing texture");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_file_number_series */

int Texture_uses_image_file_name(struct Texture *texture,
	void *image_file_name_void)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Returns true if <texture> contains the image from file <image_file_name>.
==============================================================================*/
{
	char *image_file_name;
	int return_code;

	ENTER(Texture_uses_image_file_name);
	if (texture&&(image_file_name=(char *)image_file_name_void))
	{
		if (texture->image_file_name)
		{
			return_code=(0==strcmp(texture->image_file_name,image_file_name));
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_uses_image_file_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_uses_image_file_name */

int Texture_get_number_of_bytes_per_component(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Returns the number bytes in each component of the texture: 1 or 2.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_number_of_bytes_per_component);
	if (texture)
	{
		return_code = texture->number_of_bytes_per_component;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_number_of_bytes_per_component.  Missing texture");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_number_of_bytes_per_component */

int Texture_get_number_of_components(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Returns the number of components used per texel in the texture: 1, 2, 3 or 4.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_number_of_components);
	if (texture)
	{
		return_code =
			Texture_storage_type_get_number_of_components(texture->storage);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_number_of_components.  Missing texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_number_of_components */

int Texture_get_original_size(struct Texture *texture,
	int *original_width_texels, int *original_height_texels, 
	int *original_depth_texels)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the width, height and depth of the image from which the texture was
read. May differ from the dimensions of the texture which is in powers of 2.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_original_size);
	if (texture && original_width_texels && original_height_texels &&
		original_depth_texels)
	{
		*original_width_texels = texture->original_width_texels;
		*original_height_texels = texture->original_height_texels;
		*original_depth_texels = texture->original_depth_texels;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_original_size.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_original_size */

int Texture_get_physical_size(struct Texture *texture,float *width,
	float *height, float *depth)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the physical size in model coordinates of the original texture image.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_physical_size);
	if (texture && width && height && depth)
	{
		*width = texture->width;
		*height = texture->height;
		*depth = texture->depth;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_physical_size.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_physical_size */

int Texture_set_physical_size(struct Texture *texture,
	float width, float height, float depth)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Sets the physical size in model coordinates of the original texture image. The
default is 1.0, so that texture coordinates in the range from 0 to 1 represent
real image data and not padding to make image sizes up to powers of 2.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_physical_size);
	if (texture)
	{
		if ((width != texture->width) || (height != texture->height) ||
			(depth != texture->depth))
		{
			texture->width = width;
			texture->height = height;
			texture->depth = depth;
			/* display list needs to be compiled again */
			texture->display_list_current = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_physical_size.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_physical_size */

int Texture_get_distortion_info(struct Texture *texture,
	float *distortion_centre_x,float *distortion_centre_y,
	float *distortion_factor_k1)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Returns the radial distortion information stored with the texture as
<centre_x,centre_y> with <factor_k1>, all in the physical space of the image,
from (0.0,0.0) to (texture->width,texture->height).
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_distortion_info);
	if (texture&&distortion_centre_x&&distortion_centre_y&&distortion_factor_k1)
	{
		*distortion_centre_x = texture->distortion_centre_x;
		*distortion_centre_y = texture->distortion_centre_y;
		*distortion_factor_k1 = texture->distortion_factor_k1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_distortion_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_distortion_info */

int Texture_set_distortion_info(struct Texture *texture,
	float distortion_centre_x,float distortion_centre_y,
	float distortion_factor_k1)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Records with the texture that the lens from which it was produced had radial
distortion centred at <centre_x,centre_y> with <factor_k1>, and hence is to be
considered as distorted with such values. These values are relative to the
physical size of the image, from (0.0,0.0) to (texture->width,texture->height).
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_distortion_info);
	if (texture)
	{
		texture->distortion_centre_x = distortion_centre_x;
		texture->distortion_centre_y = distortion_centre_y;
		texture->distortion_factor_k1 = distortion_factor_k1;
		/* texture display list DOES NOT need to be compiled. Distortion info
			 is merely made available to users of the texture for a consistent look
			 if it decides to correct it */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_distortion_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_distortion_info */

int Texture_get_size(struct Texture *texture,
	int *width_texels, int *height_texels, int *depth_texels)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Returns the dimensions of the texture image in powers of 2. The width, height
and depth returned are at least as large as those of the original image read
into the texture.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_size);
	if (texture && width_texels && height_texels && depth_texels)
	{
		*width_texels = texture->width_texels;
		*height_texels = texture->height_texels;
		*depth_texels = texture->depth_texels;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Texture_get_size.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_size */

enum Texture_wrap_mode Texture_get_wrap_mode(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns how textures coordinates outside [0,1] are handled.
==============================================================================*/
{
	enum Texture_wrap_mode wrap_mode;

	ENTER(Texture_get_wrap_mode);
	if (texture)
	{
		wrap_mode = texture->wrap_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_wrap_mode.  Invalid argument(s)");
	}
	LEAVE;

	return (wrap_mode);
} /* Texture_get_wrap_mode */

int Texture_set_wrap_mode(struct Texture *texture,
	enum Texture_wrap_mode wrap_mode)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Sets how textures coordinates outside [0,1] are handled.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_wrap_mode);
	if (texture&&((TEXTURE_CLAMP_WRAP==wrap_mode)||
		(TEXTURE_REPEAT_WRAP==wrap_mode)))
	{
		if (wrap_mode != texture->wrap_mode)
		{
			texture->wrap_mode = wrap_mode;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_wrap_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_wrap_mode */

int list_Texture(struct Texture *texture,void *dummy)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Writes the properties of the <texture> to the command window.
==============================================================================*/
{
	int return_code;

	ENTER(list_Texture);
	USE_PARAMETER(dummy);
	if (texture)
	{
		return_code=1;
		/* write the name */
		display_message(INFORMATION_MESSAGE,"texture : %s\n",texture->name);
		/* write the texture size in model units */
		display_message(INFORMATION_MESSAGE,
			"  size in model units.  width = %g, height = %g, depth = %g\n",
			texture->width, texture->height, texture->depth);
		/* write the radial distortion parameters in physical space */
		display_message(INFORMATION_MESSAGE,"  radial distortion in model space.  "
			"centre = %g,%g  factor_k1 = %g\n",texture->distortion_centre_x,
			texture->distortion_centre_y,texture->distortion_factor_k1);
		/* write the name of the image file */
		if (texture->image_file_name)
		{
			display_message(INFORMATION_MESSAGE, "  image file name : %s\n",
				texture->image_file_name);
		}
		/* write the file_number_series if more than one texel deep */
		if (1 < texture->depth_texels)
		{
			display_message(INFORMATION_MESSAGE,
				"  file number pattern : %s\n", texture->file_number_pattern);
			display_message(INFORMATION_MESSAGE,
				"  file number series : start = %d, stop = %d, increment = %d\n",
				texture->start_file_number,
				texture->stop_file_number,
				texture->file_number_increment);
 		}
		/* write the original image size */
		display_message(INFORMATION_MESSAGE,
			"  original width (texels) = %d, original height (texels) = %d, "
			"original depth (texels) = %d\n", texture->original_width_texels,
			texture->original_height_texels, texture->original_depth_texels);
		/* write the image components */
		switch (texture->storage)
		{
			case TEXTURE_LUMINANCE:
			{
				display_message(INFORMATION_MESSAGE,"  components : intensity\n");
			} break;
			case TEXTURE_LUMINANCE_ALPHA:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : intensity, alpha\n");
			} break;
			case TEXTURE_RGB:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : red, green, blue\n");
			} break;
			case TEXTURE_RGBA:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : red, green, blue, alpha\n");
			} break;
			case TEXTURE_ABGR:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : alpha, blue, green, red\n");
			} break;
			case TEXTURE_DMBUFFER:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : Using SGI Digital Media Buffer\n");
			} break;
			case TEXTURE_PBUFFER:
			{
				display_message(INFORMATION_MESSAGE,
					"  components : Using SGI GLX PBuffer\n");
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"list_Texture.  Undefined or unknown storage type");
				return_code=0;
			} break;
		}
		display_message(INFORMATION_MESSAGE,
			"  bytes_per_component : %d\n", texture->number_of_bytes_per_component);
		/* write the image size */
		display_message(INFORMATION_MESSAGE,
			"  width (texels) = %d, height (texels) = %d, depth (texels) = %d\n",
			texture->width_texels, texture->height_texels, texture->depth_texels);
		/* write the type of wrapping */
		display_message(INFORMATION_MESSAGE,
			"  wrap : %s\n",ENUMERATOR_STRING(Texture_wrap_mode)(texture->wrap_mode));
		/* write the type of magnification/minification filter */
		display_message(INFORMATION_MESSAGE,
			"  magnification/minification filter : %s\n",
			ENUMERATOR_STRING(Texture_filter_mode)(texture->filter_mode));
		/* write the type of resize filter */
		display_message(INFORMATION_MESSAGE, "  resize filter : %s\n",
			ENUMERATOR_STRING(Texture_resize_filter_mode)(
				texture->resize_filter_mode));
		/* write the combine type */
		display_message(INFORMATION_MESSAGE,"  combine type : %s\n",
			ENUMERATOR_STRING(Texture_combine_mode)(texture->combine_mode));
		/* write the compression type */
		display_message(INFORMATION_MESSAGE,"  compression type : %s\n",
			ENUMERATOR_STRING(Texture_compression_mode)(texture->compression_mode));
		/* write the colour */
		display_message(INFORMATION_MESSAGE,
			"  colour : red = %.3g, green = %.3g, blue = %.3g\n",
			(texture->combine_colour).red,(texture->combine_colour).green,
			(texture->combine_colour).blue);
		display_message(INFORMATION_MESSAGE,
			"  alpha = %.3g\n",texture->combine_alpha);
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Texture.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Texture */

int list_Texture_commands(struct Texture *texture,void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Writes on the command window the command needed to recreate the <texture>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/
{
	char *command_prefix,*name;
	int return_code;

	ENTER(list_Texture_commands);
	if (texture&&(command_prefix=(char *)command_prefix_void))
	{
		display_message(INFORMATION_MESSAGE,command_prefix);
		if (name=duplicate_string(texture->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,name);
			DEALLOCATE(name);
		}
		/* write the name of the image file, if any */
		if (texture->image_file_name)
		{
			display_message(INFORMATION_MESSAGE," image %s",texture->image_file_name);
		}
		/* file number_series if more than one texel deep */
		if (1 < texture->depth_texels)
		{
			display_message(INFORMATION_MESSAGE,
				" number_pattern %s number_series %d %d %d",
				texture->file_number_pattern,
				texture->start_file_number,
				texture->stop_file_number,
				texture->file_number_increment);
 		}
		if (texture->movie)
		{
			display_message(INFORMATION_MESSAGE," movie");
		}
#if defined (DEBUG)
		/*???debug*/if (texture->image_file_name&&texture->movie)
		{
			printf("Texture %s has image_file_name and movie!!\n",texture->name);
		}
#endif /* defined (DEBUG) */
		/* write the texture size in model units */
		display_message(INFORMATION_MESSAGE," width %g height %g depth %g",
			texture->width, texture->height, texture->depth);
		/* write the radial distortion parameters in physical space */
		display_message(INFORMATION_MESSAGE," distortion %g %g %g",
			texture->distortion_centre_x,texture->distortion_centre_y,
			texture->distortion_factor_k1);
		/* write the colour, alpha */
		display_message(INFORMATION_MESSAGE," colour %g %g %g",
			(texture->combine_colour).red,(texture->combine_colour).green,
			(texture->combine_colour).blue);
		display_message(INFORMATION_MESSAGE," alpha %g",texture->combine_alpha);
		/* write the combine type */
		display_message(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(Texture_combine_mode)(texture->combine_mode));
		/* write the compression type */
		display_message(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(Texture_compression_mode)(texture->compression_mode));
		/* write the type of magnification/minification filter */
		display_message(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(Texture_filter_mode)(texture->filter_mode));
		/* write the type of resize filter */
		display_message(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(Texture_resize_filter_mode)(
				texture->resize_filter_mode));
		/* write the type of wrapping */
		display_message(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(Texture_wrap_mode)(texture->wrap_mode));
		display_message(INFORMATION_MESSAGE,";\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Texture_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Texture_commands */

int compile_Texture(struct Texture *texture,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2002

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
{
	int return_code;
#if defined (OPENGL_API)
	GLboolean resident;
	GLenum texture_target;
	unsigned int old_texture_id;
#endif /* defined (OPENGL_API) */

	ENTER(compile_Texture);
	if (texture && (!dummy_void))
	{
		if ((texture->display_list_current == 1) 
#if defined (OPENGL_API)
			&& texture->texture_id 
			&&glAreTexturesResident(1, &texture->texture_id, &resident)
#endif /* defined (OPENGL_API) */
				)
		{
			return_code = 1;
		}
		else
		{
#if defined (OPENGL_API)
			switch (texture->dimension)
			{
				case 1:
				{
					texture_target = GL_TEXTURE_1D;
				} break;
				case 2:
				{
					texture_target = GL_TEXTURE_2D;
				} break;
				case 3:
				{
#if defined (GL_VERSION_1_2)
					texture_target = GL_TEXTURE_3D;
#else /* defined (GL_VERSION_1_2) */
					display_message(ERROR_MESSAGE,
						"compile_Texture.  "
						"3D textures not supported in this version.");
					return_code=0;
#endif /* defined (GL_VERSION_1_2) */
				} break;
			}
			if (texture->display_list||(texture->display_list=glGenLists(1)))
			{
				if(texture->display_list_current == 2)
				{
					switch(texture->storage)
					{
						case TEXTURE_DMBUFFER:
						{
							/* If copied by reference we don't need to do anything */
#if defined (DEBUG)
							Dm_buffer_glx_make_read_current(texture->dmbuffer);
							glBindTextureEXT(GL_TEXTURE_2D, texture->texture_id);
							glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
							glCopyTexImage2DEXT(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,
								256, 256, 0);
							glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
								texture->original_width_texels, texture->original_height_texels);
							Texture_get_type_and_format_from_storage_type(texture->storage,
								&type, &format);
							glTexImage2D(GL_TEXTURE_2D,(GLint)0,
								GL_RGBA8_EXT,
								(GLint)(texture->width_texels),
								(GLint)(texture->height_texels),(GLint)0,
								format,type, NULL);
							glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
								256, 256);
							{
								unsigned char test_pixels[262144];
								
								glReadPixels(0, 0, 256, 256,
									GL_RGBA, GL_UNSIGNED_BYTE,
									(void*)test_pixels);
								glTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0,
									256, 256,
									GL_RGBA, GL_UNSIGNED_BYTE, test_pixels);
							}
							glRasterPos3i(1,1,1);
							glCopyPixels(0,0, 256,256, GL_COLOR);
							{
								static int counter = 100;
								unsigned char test_pixels[262144];

								counter += 7;
								memset(test_pixels, counter % 255, 262144);
								glTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0,
									256, 256,
									GL_RGBA, GL_UNSIGNED_BYTE, test_pixels);
							}
							X3dThreeDDrawingRemakeCurrent();
#endif /* defined (DEBUG) */
						} break;
						case TEXTURE_PBUFFER:
						{
#if defined (SGI_DIGITAL_MEDIA)
							glPushAttrib(GL_PIXEL_MODE_BIT);
							glBindTextureEXT(GL_TEXTURE_2D, texture->texture_id);
							Dm_buffer_glx_make_read_current(texture->dmbuffer);
							glPixelTransferf(GL_ALPHA_BIAS, 1.0);  /* This is required
																					cause the OCTANE seems to return alpha of zero from render
																					to buffer */
							glCopyTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
								texture->width_texels, texture->height_texels);
							{
								/* This code is should be totally unnecessary but
									it seems that the texture stuff is broken */
								unsigned char test_pixels[262144];
								
								glReadPixels(0, 0, 256, 256,
									GL_RGBA, GL_UNSIGNED_BYTE,
									(void*)test_pixels);
								glTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0,
									256, 256,
									GL_RGBA, GL_UNSIGNED_BYTE, test_pixels);
							}
							X3dThreeDDrawingRemakeCurrent();
							glPopAttrib();
#else /* defined (SGI_DIGITAL_MEDIA) */
							display_message(ERROR_MESSAGE,
								"compile_Texture.  PBUFFER not supported");
							return_code=0;								
#endif /* defined (SGI_DIGITAL_MEDIA) */
						} break;
						default:
						{
							glBindTexture(texture_target, texture->texture_id);
#if defined (EXT_subtexture)
							Texture_get_type_and_format_from_storage_type(texture->storage,
								&type, &format);
							switch (texture->dimension)
							{
								case 1:
								{
									glTexSubImage1DEXT(texture_target, 0, 0, 0,
										(GLint)(texture->width_texels),
										(GLint)(texture->height_texels),
										format, type, (GLvoid *)(texture->image));
								} break;
								case 2:
								{
									glTexSubImage2DEXT(texture_target, 0, 0, 0,
										(GLint)(texture->width_texels),
										(GLint)(texture->height_texels),
										format, type, (GLvoid *)(texture->image));
								} break;
								case 3:
								{
									glTexSubImage3DEXT(texture_target, 0, 0, 0,
										(GLint)(texture->width_texels),
										(GLint)(texture->height_texels),
										(GLint)(texture->depth_texels),
										format, type, (GLvoid *)(texture->image));
								} break;
							}
#else /* defined (EXT_subtexture) */
							direct_render_Texture(texture);
#endif /* defined (EXT_subtexture) */
						} break;
					}
				}
				else
				{
					/* SAB Mirage Octanes are not allowing the texture to be reread into
						the bound texture so I am going to destroy them every time they
						need to be reconstructed.  Further, a texture in both the window 
						window background and loaded onto an object doesn't get updated
						correctly still, now I'm making sure the new texture_id is at
						least recently new */
					old_texture_id = texture->texture_id;
					glGenTextures(1, &(texture->texture_id));
					if (old_texture_id)
					{
						glDeleteTextures(1, &(old_texture_id));
					}
					glBindTexture(texture_target, texture->texture_id);
#if defined (DM_BUFFERS)
					if(texture->storage==TEXTURE_DMBUFFER || 
						texture->storage==TEXTURE_PBUFFER)
					{
						Dm_buffer_glx_make_read_current(texture->dmbuffer);				
						direct_render_Texture(texture);
						X3dThreeDDrawingRemakeCurrent();
					}
					else
					{
#endif /* defined (DM_BUFFERS) */
						direct_render_Texture(texture);
#if defined (DM_BUFFERS)
					}
#endif /* defined (DM_BUFFERS) */
					glNewList(texture->display_list,GL_COMPILE);
					glBindTexture(texture_target, texture->texture_id);
					/* As we have bound the texture we only need the 
						environment in the display list */
					direct_render_Texture_environment(texture);

					glEndList();
				}
				texture->display_list_current=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Texture.  Could not generate display list");
				return_code=0;
			}
#else /* defined (OPENGL_API) */
			display_message(ERROR_MESSAGE,
				"compile_Texture.  Not defined for this API");
			return_code=0;
#endif /* defined (OPENGL_API) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Texture.  Missing texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Texture */

int execute_Texture(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 14 March 2002

DESCRIPTION :
Activates <texture> by calling its display list. If the display list is not
current, an error is reported.
If a NULL <texture> is supplied, textures are disabled.
???RC The behaviour of textures is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Texture.
==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GLboolean resident;
#endif /* defined (OPENGL_API) */

	ENTER(execute_Texture);
	return_code=0;
	if (texture)
	{
		if (texture->display_list_current)
		{
#if defined (OPENGL_API)
			glCallList(texture->display_list);
			/* SAB There seems to be some problem with losing a bound texture,
				this tries to correct this */
			/* If using texture_objects then we need to check that the texture
				is still resident */
			glAreTexturesResident(1, &texture->texture_id, &resident);
			if (GL_TRUE != resident)
			{
#if defined (DM_BUFFERS)
				/* Reload the texture */
				if(texture->storage==TEXTURE_DMBUFFER || 
					texture->storage==TEXTURE_PBUFFER)
				{
					Dm_buffer_glx_make_read_current(texture->dmbuffer);				
					direct_render_Texture(texture);
					X3dThreeDDrawingRemakeCurrent();
				}
				else
#endif /* defined (DM_BUFFERS) */
				{
					direct_render_Texture(texture);
				}
			}
			return_code=1;
#else /* defined (OPENGL_API) */
			display_message(ERROR_MESSAGE,
				"execute_Texture.  Not defined for this API");
			return_code=0;
#endif /* defined (OPENGL_API) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_Texture.  Display list not current");
			return_code=0;
		}
	}
	else
	{
#if defined (OPENGL_API)
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
#if defined (GL_VERSION_1_2)
		glDisable(GL_TEXTURE_3D);
#endif /* defined (GL_VERSION_1_2) */
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* execute_Texture */

int set_Texture(struct Parse_state *state,void *texture_address_void,
	void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the texture from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(Texture) *texture_manager;
	struct Texture *temp_texture,**texture_address;

	ENTER(set_Texture);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((texture_address=(struct Texture **)texture_address_void)&&
					(texture_manager=(struct MANAGER(Texture) *)texture_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*texture_address)
						{
							DEACCESS(Texture)(texture_address);
							*texture_address=(struct Texture *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(
							current_token,texture_manager))
						{
							if (*texture_address!=temp_texture)
							{
								DEACCESS(Texture)(texture_address);
								*texture_address=ACCESS(Texture)(temp_texture);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Texture.  Texture does not exist");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Texture.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," TEXTURE_NAME|none");
				if (texture_address=(struct Texture **)texture_address_void)
				{
					if (temp_texture= *texture_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_texture->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Texture.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* set_Texture */
