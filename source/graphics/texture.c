/*******************************************************************************
FILE : texture.c

LAST MODIFIED : 22 January 2002

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
#include "general/image_utilities.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "three_d_drawing/dm_interface.h"
#include "graphics/light.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"
#include "graphics/texture.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "three_d_drawing/movie_extensions.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Texture
/*******************************************************************************
LAST MODIFIED : 11 October 2000

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
	/* the range of texture coordinates in model units */
	float height,width;
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
	enum Texture_storage_type storage;
	/* number of 8-bit bytes per component */
	int number_of_bytes_per_component;
	/* array of 4-byte words containing the texel data.  The texels fill the
		image from left to right, bottom to top.  Each row of texel information
		must be long word aligned (end of row byte padded) */
		/*???DB.  OpenGL allows greater choice, but this will not be used */
	long unsigned *image;
	/* OpenGL requires the width and height of textures to be in powers of 2.
		Hence, only the original width x height contains useful image data */
	/* stored image size in texels */
	int height_texels,width_texels;
	/* original image size in texels */
	int original_height_texels,original_width_texels;
	/* cropping of image in file */
	int crop_bottom_margin,crop_height,crop_left_margin,crop_width;
	/* texture display options */
	enum Texture_combine_mode combine_mode;
	enum Texture_filter_mode filter_mode;
	enum Texture_wrap_mode wrap_mode;
	struct Colour combine_colour;
	float combine_alpha;

	struct X3d_movie *movie;
	struct Dm_buffer *dmbuffer;

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

int Texture_get_number_of_components_from_storage_type(
	enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Returns the number of components used per texel in the image.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_number_of_components_from_storage_type);
	switch (storage)
	{
		case TEXTURE_UNDEFINED_STORAGE:
		default:
		{
			display_message(ERROR_MESSAGE,"Texture_get_number_of_components_from_storage_type."
				"  Texture storage type unknown");
			return_code = 0;
		} break;
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
		case TEXTURE_UNDEFINED_STORAGE:
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_get_type_and_format_from_storage_type."
				"  Texture storage type unknown");
			return_code=0;
		} break;
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
static int direct_render_Texture_environment(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

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
#if defined (OPENGL_API)
		texture_coordinate_transform[0][0]=texture->original_width_texels/
			(texture->width_texels*texture->width);
		texture_coordinate_transform[1][0]=0.;
		texture_coordinate_transform[2][0]=0.;
		texture_coordinate_transform[3][0]=0.;
		texture_coordinate_transform[0][1]=0.;
		texture_coordinate_transform[1][1]=texture->original_height_texels/
			(texture->height_texels*texture->height);
		texture_coordinate_transform[2][1]=0.;
		texture_coordinate_transform[3][1]=0.;
		texture_coordinate_transform[0][2]=0.;
		texture_coordinate_transform[1][2]=0.;
		texture_coordinate_transform[2][2]=1.;
		texture_coordinate_transform[3][2]=0.;
		texture_coordinate_transform[0][3]=0.;
		texture_coordinate_transform[1][3]=0.;
		texture_coordinate_transform[2][3]=0.;
		texture_coordinate_transform[3][3]=1.;
		/* set the texture coordinate transformation */
		glMatrixMode(GL_TEXTURE);
		wrapperLoadCurrentMatrix(&texture_coordinate_transform);
		glMatrixMode(GL_MODELVIEW);
		number_of_components = Texture_get_number_of_components_from_storage_type(texture->storage);
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
		glEnable(GL_TEXTURE_2D);
		return_code=1;
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
static int direct_render_Texture(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Directly outputs the commands setting up the <texture>.
==============================================================================*/
{
	int destination_row_width_bytes, i, j, k, number_of_bytes, scale, 
		source_row_width_bytes, total_size, return_code;
	unsigned char *image, *resized_image;
#if defined (OPENGL_API)
	GLenum format,type;
	GLfloat values[4], actual_width;
	GLint number_of_components;
#endif /* defined (OPENGL_API) */

	ENTER(direct_render_Texture);
	return_code=1;
	if (texture)
	{
#if defined (OPENGL_API)
		switch(texture->storage)
		{
			case TEXTURE_DMBUFFER:
			{
#if defined (SGI_DIGITAL_MEDIA)
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
#else /* defined (SGI_DIGITAL_MEDIA) */
				display_message(ERROR_MESSAGE,"direct_render_Texture."
					"  Texture has type DMBUFFER but DIGITAL_MEDIA unavailable");
				return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
			} break;
			case TEXTURE_PBUFFER:
			{
#if defined (SGI_DIGITAL_MEDIA)
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
#else /* defined (SGI_DIGITAL_MEDIA) */
				display_message(ERROR_MESSAGE,"direct_render_Texture."
					"  Texture has type PBUFFER but DIGITAL_MEDIA unavailable");
				return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
			} break;
			default:
			{
				/* make each row of the image start on a long word (4 byte) boundary */
				glPixelStorei(GL_UNPACK_ALIGNMENT,4);
				/* specify the components used for each texel and the storage for each
				component */
				number_of_components = Texture_get_number_of_components_from_storage_type(texture->storage);
				Texture_get_type_and_format_from_storage_type(texture->storage,
					texture->number_of_bytes_per_component, &type, &format);
				/* check to see if we can display an image of this size */
				glTexImage2D(GL_PROXY_TEXTURE_2D,(GLint)0,
					number_of_components,
					(GLint)(texture->width_texels),
					(GLint)(texture->height_texels),(GLint)0,
					format,type,(GLvoid *)(texture->image));
				glGetTexLevelParameterfv(GL_PROXY_TEXTURE_2D,(GLint)0,
					GL_TEXTURE_WIDTH,&actual_width);
				resized_image = (unsigned char *)NULL;
				scale = 1;
				while (return_code && (actual_width == 0))
				{
					scale *= 2;
					if ((scale < texture->width_texels) && 
						(scale < texture->height_texels))
					{
						glTexImage2D(GL_PROXY_TEXTURE_2D,(GLint)0,
							number_of_components,
							(GLint)(texture->width_texels/scale),
							(GLint)(texture->height_texels/scale),(GLint)0,
							format,type,(GLvoid *)(texture->image));
						glGetTexLevelParameterfv(GL_PROXY_TEXTURE_2D,(GLint)0,
							GL_TEXTURE_WIDTH,&actual_width);
					}
					else
					{
						display_message(ERROR_MESSAGE,"direct_render_Texture."
							"  Error trying to scale texture.");
						return_code=0;
					}
				}
				if (scale == 1)
				{
					glTexImage2D(GL_TEXTURE_2D,(GLint)0,
						number_of_components,
						(GLint)(texture->width_texels),
						(GLint)(texture->height_texels),(GLint)0,
						format,type,(GLvoid *)(texture->image));
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Image %s is too large for this display."
						"Reducing (%d,%d) to (%d,%d) for display only", texture->name,
						texture->width_texels, texture->height_texels, 
						texture->width_texels / scale, texture->height_texels / scale); 
					/* Make the appropriate image and load that */
					number_of_bytes = texture->number_of_bytes_per_component *
						number_of_components;
					/* ensure texture images are unsigned long row aligned */
					destination_row_width_bytes=
						4*((int)(((texture->width_texels/scale)*number_of_bytes+3)/4));
					source_row_width_bytes=
						4*((int)(((texture->width_texels)*number_of_bytes+3)/4));
					total_size = (texture->height_texels/scale)*destination_row_width_bytes*4;
					image = (unsigned char *)texture->image;
					if (ALLOCATE(resized_image, unsigned char, total_size))
					{
						for (i = 0 ; i < texture->height_texels/scale ; i++)
						{
							for (j = 0 ; j < texture->width_texels/scale ; j++)
							{
								for (k = 0 ; k < number_of_bytes ; k++)
								{
									resized_image[i * destination_row_width_bytes + j *
										number_of_bytes + k] = 
										image[i * scale * source_row_width_bytes +
											j * scale * number_of_bytes + k];
								}
							}
						}

						glTexImage2D(GL_TEXTURE_2D,(GLint)0,
							number_of_components,
							(GLint)(texture->width_texels/scale),
							(GLint)(texture->height_texels/scale),(GLint)0,
							format,type,(GLvoid *)(resized_image));
						DEALLOCATE(resized_image);
					}
					else
					{
						display_message(ERROR_MESSAGE,"direct_render_Texture."
							"  Unable to Allocate replacement memory space");
					}
				}
			} break;
		}
		switch (texture->wrap_mode)
		{
			case TEXTURE_CLAMP_WRAP:
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			} break;
			case TEXTURE_REPEAT_WRAP:
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
			} break;
		}
		switch (texture->filter_mode)
		{
			case TEXTURE_LINEAR_FILTER:
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			} break;
			case TEXTURE_NEAREST_FILTER:
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			} break;
		}
		values[0]=(texture->combine_colour).red;
		values[1]=(texture->combine_colour).green;
		values[2]=(texture->combine_colour).blue;
		values[3]=texture->combine_alpha;
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,values);
#endif /* defined (OPENGL_API) */
		return_code = direct_render_Texture_environment(texture);
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
LAST MODIFIED : 11 October 2000

DESCRIPTION :
A modifier function to set the texture storage type.
==============================================================================*/
{
	char *current_token;
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
					if (TEXTURE_UNDEFINED_STORAGE != (storage_type = 
						Texture_storage_type_from_string(state->current_token)))
					{
						*storage_type_address = storage_type;
						return_code=shift_Parse_state(state,1);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_Texture_storage.  Invalid storage type.");
						return_code=0;				
					}
				}
				else
				{
					/* write help */
					storage_type=TEXTURE_TYPE_BEFORE_FIRST;
					storage_type++;
					display_message(INFORMATION_MESSAGE," ");
					while (storage_type<TEXTURE_TYPE_AFTER_LAST_NORMAL)
					{
						display_message(INFORMATION_MESSAGE,
							Texture_storage_type_string(storage_type));
						if (storage_type == *storage_type_address)
						{
							display_message(INFORMATION_MESSAGE,"[%s]",
								Texture_storage_type_string(storage_type));
						}
						storage_type++;
						if (storage_type<TEXTURE_TYPE_AFTER_LAST_NORMAL)
						{
							display_message(INFORMATION_MESSAGE,"|");
						}
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

char *Texture_combine_mode_string(enum Texture_combine_mode combine_mode)
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the combine_mode,
eg. TEXTURE_DECAL = "decal". This string should match the command
used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Texture_combine_mode_string);
	switch (combine_mode)
	{
		case TEXTURE_BLEND:
		{
			return_string="blend";
		} break;
		case TEXTURE_DECAL:
		{
			return_string="decal";
		} break;
		case TEXTURE_MODULATE:
		{
			return_string="modulate";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_combine_mode_string.  Unknown combine_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Texture_combine_mode_string */

char **Texture_combine_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 10 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_combine_modes.
Strings are obtained from function Texture_combine_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;

	ENTER(Texture_combine_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=3;
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			valid_strings[0]=Texture_combine_mode_string(TEXTURE_BLEND);
			valid_strings[1]=Texture_combine_mode_string(TEXTURE_DECAL);
			valid_strings[2]=Texture_combine_mode_string(TEXTURE_MODULATE);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_combine_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_combine_mode_get_valid_strings.  Invalid argument(s)");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Texture_combine_mode_get_valid_strings */

enum Texture_combine_mode Texture_combine_mode_from_string(
	char *combine_mode_string)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_combine_mode> described by <combine_mode_string>.
==============================================================================*/
{
	enum Texture_combine_mode combine_mode;

	ENTER(Texture_combine_mode_from_string);
	if (combine_mode_string)
	{
		if (fuzzy_string_compare_same_length(combine_mode_string,
			Texture_combine_mode_string(TEXTURE_BLEND)))
		{
			combine_mode = TEXTURE_BLEND;
		}
		else if (fuzzy_string_compare_same_length(combine_mode_string,
			Texture_combine_mode_string(TEXTURE_DECAL)))
		{
			combine_mode = TEXTURE_DECAL;
		}
		else if (fuzzy_string_compare_same_length(combine_mode_string,
			Texture_combine_mode_string(TEXTURE_MODULATE)))
		{
			combine_mode = TEXTURE_MODULATE;
		}
		else
		{
			combine_mode = TEXTURE_COMBINE_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_combine_mode_from_string.  Invalid argument");
		combine_mode = TEXTURE_COMBINE_MODE_INVALID;
	}
	LEAVE;

	return (combine_mode);
} /* Texture_combine_mode_from_string */

char *Texture_filter_mode_string(enum Texture_filter_mode filter_mode)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the filter_mode,
eg. TEXTURE_LINEAR_FILTER = "linear_filter". This string should match the
command used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Texture_filter_mode_string);
	switch (filter_mode)
	{
		case TEXTURE_LINEAR_FILTER:
		{
			return_string="linear_filter";
		} break;
		case TEXTURE_NEAREST_FILTER:
		{
			return_string="nearest_filter";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_filter_mode_string.  Unknown filter_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Texture_filter_mode_string */

char **Texture_filter_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_filter_modes.
Strings are obtained from function Texture_filter_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;

	ENTER(Texture_filter_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=2;
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			valid_strings[0]=Texture_filter_mode_string(TEXTURE_LINEAR_FILTER);
			valid_strings[1]=Texture_filter_mode_string(TEXTURE_NEAREST_FILTER);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_filter_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_filter_mode_get_valid_strings.  Invalid argument(s)");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Texture_filter_mode_get_valid_strings */

enum Texture_filter_mode Texture_filter_mode_from_string(
	char *filter_mode_string)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_filter_mode> described by <filter_mode_string>.
==============================================================================*/
{
	enum Texture_filter_mode filter_mode;

	ENTER(Texture_filter_mode_from_string);
	if (filter_mode_string)
	{
		if (fuzzy_string_compare_same_length(filter_mode_string,
			Texture_filter_mode_string(TEXTURE_LINEAR_FILTER)))
		{
			filter_mode = TEXTURE_LINEAR_FILTER;
		}
		else if (fuzzy_string_compare_same_length(filter_mode_string,
			Texture_filter_mode_string(TEXTURE_NEAREST_FILTER)))
		{
			filter_mode = TEXTURE_NEAREST_FILTER;
		}
		else
		{
			filter_mode = TEXTURE_FILTER_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_filter_mode_from_string.  Invalid argument");
		filter_mode = TEXTURE_FILTER_MODE_INVALID;
	}
	LEAVE;

	return (filter_mode);
} /* Texture_filter_mode_from_string */

char *Texture_wrap_mode_string(enum Texture_wrap_mode wrap_mode)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns a pointer to a static string describing the wrap_mode,
eg. TEXTURE_CLAMP_WRAP = "clamp_wrap". This string should match the command
used to set the type. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Texture_wrap_mode_string);
	switch (wrap_mode)
	{
		case TEXTURE_CLAMP_WRAP:
		{
			return_string="clamp_wrap";
		} break;
		case TEXTURE_REPEAT_WRAP:
		{
			return_string="repeat_wrap";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_wrap_mode_string.  Unknown wrap_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Texture_wrap_mode_string */

char **Texture_wrap_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Texture_wrap_modes. Strings are obtained from function Texture_wrap_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;

	ENTER(Texture_wrap_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=2;
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			valid_strings[0]=Texture_wrap_mode_string(TEXTURE_CLAMP_WRAP);
			valid_strings[1]=Texture_wrap_mode_string(TEXTURE_REPEAT_WRAP);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_wrap_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_wrap_mode_get_valid_strings.  Invalid argument(s)");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Texture_wrap_mode_get_valid_strings */

enum Texture_wrap_mode Texture_wrap_mode_from_string(char *wrap_mode_string)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

DESCRIPTION :
Returns the <Texture_wrap_mode> described by <wrap_mode_string>.
==============================================================================*/
{
	enum Texture_wrap_mode wrap_mode;

	ENTER(Texture_wrap_mode_from_string);
	if (wrap_mode_string)
	{
		if (fuzzy_string_compare_same_length(wrap_mode_string,
			Texture_wrap_mode_string(TEXTURE_CLAMP_WRAP)))
		{
			wrap_mode = TEXTURE_CLAMP_WRAP;
		}
		else if (fuzzy_string_compare_same_length(wrap_mode_string,
			Texture_wrap_mode_string(TEXTURE_REPEAT_WRAP)))
		{
			wrap_mode = TEXTURE_REPEAT_WRAP;
		}
		else
		{
			wrap_mode = TEXTURE_WRAP_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_wrap_mode_from_string.  Invalid argument");
		wrap_mode = TEXTURE_WRAP_MODE_INVALID;
	}
	LEAVE;

	return (wrap_mode);
} /* Texture_wrap_mode_from_string */

struct Texture *CREATE(Texture)(char *name)
/*******************************************************************************
LAST MODIFIED : 27 September 1999

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
	if (ALLOCATE(texture,struct Texture,1)&&
		ALLOCATE(texture->image,unsigned long,4))
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
			texture->height=1.;
			texture->width=1.;
			/* assign image description fields */
			texture->image_file_name=(char *)NULL;
			texture->storage=TEXTURE_RGBA;
			texture->number_of_bytes_per_component=1;
			texture->height_texels=1;
			texture->width_texels=1;
			texture->original_height_texels=1;
			texture->original_width_texels=1;
			texture->distortion_centre_x=0.0;
			texture->distortion_centre_y=0.0;
			texture->distortion_factor_k1=0.0;
			(texture->image)[0]=0xFFFFFFFF;
			(texture->image)[1]=0xFFFFFFFF;
			(texture->image)[2]=0xFFFFFFFF;
			(texture->image)[3]=0xFFFFFFFF;
			texture->combine_mode=TEXTURE_DECAL;
			texture->filter_mode=TEXTURE_NEAREST_FILTER;
			texture->wrap_mode=TEXTURE_REPEAT_WRAP;
			(texture->combine_colour).red=0.;
			(texture->combine_colour).green=0.;
			(texture->combine_colour).blue=0.;
			texture->combine_alpha=0.;
			texture->texture_manager = (struct MANAGER(Texture) *)NULL;
			texture->movie = (struct X3d_movie *)NULL;
			texture->dmbuffer = (struct Dm_buffer *)NULL;
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
LAST MODIFIED : 20 April 1998

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
				if(texture->dmbuffer)
				{
#if defined (SGI_DIGITAL_MEDIA)
					DEACCESS(Dm_buffer)(&(texture->dmbuffer));
#else /* defined (SGI_DIGITAL_MEDIA) */
					display_message(ERROR_MESSAGE,"DESTROY(Texture).  Digital Media unavailable but dm_buffer pointer found");
					return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
				}
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
				DEALLOCATE(texture->image_file_name);
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
	int image_size,number_of_components, return_code;
	long unsigned *destination_image;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name));
	/* check arguments */
	if (source&&destination)
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
				Texture_get_number_of_components_from_storage_type(source->storage);
			image_size=(source->height_texels)*
				((int)((source->width_texels)*number_of_components*
				source->number_of_bytes_per_component+3)/4);
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
	 				if ((0<image_size)&&REALLOCATE(destination_image,destination->image,
		 				unsigned long,image_size))
					{
						destination->image=destination_image;
						/* use memcpy to copy the image data - should be fastest method */
						memcpy((void *)destination->image,
							(void *)source->image,image_size*4);
#if defined (PREVIOUS_CODE_TO_KEEP)
						source_image=source->image;
						while (image_size>0)
						{
							*destination_image= *source_image;
							destination_image++;
							source_image++;
							image_size--;
						}
#endif /* defined (PREVIOUS_CODE_TO_KEEP) */
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

char *Texture_storage_type_string(enum Texture_storage_type texture_storage_type)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Returns a pointer to a static string describing the texture storage, eg.
TEXTURE_STORAGE == "rgba". This string should match the command used
to create that type of texture. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Texture_storage_type_string);
	switch (texture_storage_type)
	{
		case TEXTURE_LUMINANCE:
		{
			return_string="i";
		} break;
		case TEXTURE_LUMINANCE_ALPHA:
		{
			return_string="ia";
		} break;
		case TEXTURE_RGB:
		{
			return_string="rgb";
		} break;
		case TEXTURE_RGBA:
		{
			return_string="rgba";
		} break;
		case TEXTURE_ABGR:
		{
			return_string="abgr";
		} break;
		case TEXTURE_DMBUFFER:
		{
			return_string="dmbuffer";
		} break;
		case TEXTURE_PBUFFER:
		{
			return_string="pbuffer";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_storage_type_string.  Unknown texture_type");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Texture_storage_type_string */

char **Texture_storage_type_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Texture_storage_types - obtained from function Texture_storage_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Texture_storage_type texture_type;
	int i;

	ENTER(Texture_storage_type_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		texture_type=TEXTURE_TYPE_BEFORE_FIRST;
		texture_type++;
		while (texture_type<TEXTURE_TYPE_AFTER_LAST_NORMAL)
		{
			(*number_of_valid_strings)++;
			texture_type++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			texture_type=TEXTURE_TYPE_BEFORE_FIRST;
			texture_type++;
			i=0;
			while (texture_type<TEXTURE_TYPE_AFTER_LAST_NORMAL)
			{
				valid_strings[i]=Texture_storage_type_string(texture_type);
				i++;
				texture_type++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Texture_storage_type_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_storage_type_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Texture_storage_type_get_valid_strings */

enum Texture_storage_type Texture_storage_type_from_string(char *texture_type_string)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the <Texture_storage_type> described by <texture_type_string>.
==============================================================================*/
{
	enum Texture_storage_type texture_type;

	ENTER(Texture_storage_type_from_string);
	if (texture_type_string)
	{
		texture_type=TEXTURE_TYPE_BEFORE_FIRST;
		texture_type++;
		while ((texture_type<TEXTURE_TYPE_AFTER_LAST_NORMAL)&&
			(!fuzzy_string_compare_same_length(texture_type_string,
			Texture_storage_type_string(texture_type))))
		{
			texture_type++;
		}
		if (TEXTURE_TYPE_AFTER_LAST_NORMAL==texture_type)
		{
			texture_type++;
			while ((texture_type<TEXTURE_TYPE_AFTER_LAST)&&
				(!fuzzy_string_compare_same_length(texture_type_string,
				Texture_storage_type_string(texture_type))))
			{
				texture_type++;
			}
			if (TEXTURE_TYPE_AFTER_LAST==texture_type)
			{
				texture_type=TEXTURE_UNDEFINED_STORAGE;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_storage_type_from_string.  Invalid argument");
		texture_type=TEXTURE_UNDEFINED_STORAGE;
	}
	LEAVE;

	return (texture_type);
} /* Texture_storage_type_from_string */

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
		combine_mode = TEXTURE_COMBINE_MODE_INVALID;
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
		filter_mode = TEXTURE_FILTER_MODE_INVALID;
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

int Texture_set_image(struct Texture *texture,unsigned long *image,
	enum Texture_storage_type storage,int number_of_bytes_per_component,
	int image_width,int image_height, enum Texture_row_order row_order,
	char *image_file_name,int	crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height,int perform_crop)
/*******************************************************************************
LAST MODIFIED : 5 September 2000

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value. Similarly, the four crop parameters should
be set to record any cropping already done on the image so that it may be
recorded with the texture - note however that if perform_crop is true then the
crop is performed as the image is put into the texture.
==============================================================================*/
{
	char *temp_file_name;
	int copy_row_width_bytes,destination_row_width_bytes,final_bottom_margin,
		final_height,final_left_margin,final_width,i,j,number_of_bytes,
		source_row_width_bytes,return_code;
	long int texture_height,texture_width;
	long unsigned *texture_image;
	unsigned char *source,*destination,*destination_white_space;

	ENTER(Texture_set_image);
	if (texture&&image&&image_file_name)
	{
		return_code=1;

		number_of_bytes = number_of_bytes_per_component *
			Texture_get_number_of_components_from_storage_type(storage);

		if (perform_crop)
		{
			if ((0<=crop_left_margin)&&(0<=crop_bottom_margin)&&
				(0<crop_width)&&(0<crop_height)&&
				(crop_left_margin+crop_width<=image_width)&&
				(crop_bottom_margin+crop_height<=image_height))
			{
				final_width = crop_width;
				final_height = crop_height;
				final_left_margin = crop_left_margin;
				final_bottom_margin = crop_bottom_margin;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_image.  Invalid cropping parameters");
				return_code=0;
			}
		}
		else
		{
			final_width = image_width;
			final_height = image_height;
			final_left_margin = 0;
			final_bottom_margin = 0;
		}
		if (return_code)
		{
			/* width and height must be powers of 2 */
			i=final_width;
			texture_width=1;
			while (i>1)
			{
				texture_width *= 2;
				i /= 2;
			}
			if (texture_width<final_width)
			{
				texture_width *= 2;
			}
			i=final_height;
			texture_height=1;
			while (i>1)
			{
				texture_height *= 2;
				i /= 2;
			}
			if (texture_height<final_height)
			{
				texture_height *= 2;
			}
			if ((final_width != texture_width)||(final_height != texture_height))
			{
				display_message(WARNING_MESSAGE,
					"image width and/or height not powers of 2.  "
					"Extending (%d,%d) to (%d,%d)",final_width,final_height,texture_width,
					texture_height);
			}
			/* ensure texture images are unsigned long row aligned */
			destination_row_width_bytes=
				4*((int)((texture_width*number_of_bytes+3)/4));
			source_row_width_bytes=image_width*number_of_bytes;
			copy_row_width_bytes=final_width*number_of_bytes;
			/*???RC was allocating long ints as if they were bytes! */
			if (ALLOCATE(texture_image,unsigned long,
				texture_height*destination_row_width_bytes/4))
			{
				/* transform image into texturing format */
				destination=(unsigned char *)texture_image;
				destination_white_space=
					(unsigned char *)texture_image + source_row_width_bytes;
				if (TEXTURE_TOP_TO_BOTTOM == row_order)
				{
					source_row_width_bytes *= -1;
					source = (unsigned char *)image +
						((final_bottom_margin + image_height - 1)*image_width+
						final_left_margin)*number_of_bytes;
				}
				else
				{
					source = (unsigned char *)image +
						(final_bottom_margin*image_width+final_left_margin)*number_of_bytes;
				}
				for (j=0;j<final_height;j++)
				{
					memcpy((void *)destination,(void *)source,copy_row_width_bytes);
					destination += destination_row_width_bytes;
					source += source_row_width_bytes;
					/* fill the space on the right with black pixels */
					memset((void *)destination_white_space,0x00,
						destination_row_width_bytes-copy_row_width_bytes);
					destination_white_space += destination_row_width_bytes;
				}
				/* fill the space on the top with black pixels */
				for (j=final_height;j<texture_height;j++)
				{
					memset((void *)destination,0x00,destination_row_width_bytes);
					destination += destination_row_width_bytes;
				}
				if (ALLOCATE(temp_file_name,char,strlen(image_file_name)+1))
				{
					strcpy(temp_file_name,image_file_name);
					/* assign values */
					texture->storage=storage;
					texture->number_of_bytes_per_component=number_of_bytes_per_component;
					/* original size is intended to specify useful part of texture */
					texture->original_width_texels=final_width;
					texture->original_height_texels=final_height;
					texture->height_texels=texture_height;
					texture->width_texels=texture_width;
					DEALLOCATE(texture->image);
					texture->image=texture_image;
					if (texture->image_file_name)
					{
						DEALLOCATE(texture->image_file_name);
					}
					texture->image_file_name=temp_file_name;
					texture->crop_left_margin=crop_left_margin;
					texture->crop_bottom_margin=crop_bottom_margin;
					texture->crop_width=crop_width;
					texture->crop_height=crop_height;
					/* display list needs to be compiled again */
					texture->display_list_current=0;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Texture_set_image.  Could not allocate image_file_name");
					DEALLOCATE(texture_image);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_image.  Could not allocate texture image");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_image */

int Texture_set_image_file(struct Texture *texture,char *image_file_name,
	int specify_width,int specify_height, enum Raw_image_storage raw_image_storage,
	int crop_left_margin,int crop_bottom_margin,int crop_width,
	int crop_height,double radial_distortion_centre_x,
	double radial_distortion_centre_y,double radial_distortion_factor_k1)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

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
{
	enum Texture_storage_type storage;
	int original_height_texels,original_width_texels,perform_crop,return_code;
	int number_of_components,number_of_bytes_per_component;
	long image_width,image_height;
	long unsigned *image;

	ENTER(Texture_set_image_file);
	if (texture&&image_file_name)
	{
		image_height = specify_height;
		image_width = specify_width;
#if defined (IMAGEMAGICK)
		/*???RC Remove <raw_image_storage> once non-ImageMagick code removed */
		USE_PARAMETER(raw_image_storage);
		if (read_image_file(image_file_name,&number_of_components,
			&number_of_bytes_per_component,&image_height,&image_width,
			&image))
#else /* defined (IMAGEMAGICK) */
		if (read_image_file(image_file_name,&number_of_components,
			&number_of_bytes_per_component,&image_height,&image_width,
			raw_image_storage,&image))
#endif /* defined (IMAGEMAGICK) */
		{
			return_code=1;
			switch(number_of_components)
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
						"Texture_set_image_file.  Invalid argument(s)");
					return_code=0;
				} break;
			}
			original_width_texels=image_width;
			original_height_texels=image_height;
			if (image&&(0.0 != radial_distortion_factor_k1))
			{
				if (!undistort_image(&image, number_of_components,
					number_of_bytes_per_component,
					original_width_texels,original_height_texels,
					radial_distortion_centre_x,radial_distortion_centre_y,
					radial_distortion_factor_k1))
				{
					display_message(ERROR_MESSAGE,
						"Texture_set_image_file.  Could not correct radial distortion");
					DEALLOCATE(image);
				}
			}
			if (image)
			{
				perform_crop=(0<crop_width)&&(0<crop_height);
				if (!Texture_set_image(texture,image,storage,
					number_of_bytes_per_component,
					original_width_texels,original_height_texels,
					TEXTURE_TOP_TO_BOTTOM, image_file_name,
					crop_left_margin,crop_bottom_margin,crop_width,crop_height,
					perform_crop))
				{
					return_code=0;
				}
				DEALLOCATE(image);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"Texture_set_image_file.  Failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not read image file '%s'",
				image_file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_set_image_file */

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
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Puts the <image> in the texture. The image is left unchanged by this function.
The <image_file_name> is specified purely so that it may be recorded with the
texture, and must be given a value.
==============================================================================*/
{
	int return_code;
#if defined (SGI_MOVIE_FILE)
	char *temp_file_name;
	enum Dm_buffer_type dm_buffer_type;
	enum Texture_storage_type storage;
	int destination_row_width_bytes,i,image_height,image_width,
		number_of_components;
	long int texture_height,texture_width;
	long unsigned *texture_image;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(Texture_set_movie);
#if defined (SGI_MOVIE_FILE)
	if (texture&&movie&&image_file_name)
	{
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
		if((texture->dmbuffer = ACCESS(Dm_buffer)(
			CREATE(Dm_buffer)(texture_width, texture_height, /*depth_buffer_flag*/0,
			  /*shared_display_buffer*/1, user_interface)))
			&& (DM_BUFFER_INVALID_TYPE != (dm_buffer_type = 
			Dm_buffer_get_type(texture->dmbuffer))))
		{
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

			X3d_movie_add_callback(movie, user_interface->application_context,
				Texture_movie_callback, (void *)texture);
			X3d_movie_add_destroy_callback(movie,
				Texture_movie_destroy_callback, (void *)texture);

			if (ALLOCATE(temp_file_name,char,strlen(image_file_name)+1))
			{
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
			if(query_gl_extension("GL_EXT_abgr"))
			{
				texture->movie = movie;
				storage = TEXTURE_ABGR; /* This is the default format for frames from
													the movie library */
				number_of_components = 4;

				/* ensure texture images are unsigned long row aligned */
				destination_row_width_bytes=
					4*((int)((texture_width*number_of_components+3)/4));
				if (ALLOCATE(texture_image,unsigned long,
					texture_height*destination_row_width_bytes))
				{
					X3d_movie_bind_to_image_buffer(movie, texture_image, 
						image_width, image_height, texture_width - image_width);

					memset((void *)texture_image,0x00,
						4*destination_row_width_bytes*texture_height);
					X3d_movie_render_to_image_buffer(movie, texture_image, 
						image_width, image_height, texture_width - image_width,
						0, 15);

					X3d_movie_add_callback(movie, user_interface->application_context,
						Texture_movie_callback, (void *)texture);
					X3d_movie_add_destroy_callback(movie,
						Texture_movie_destroy_callback, (void *)texture);

					if (ALLOCATE(temp_file_name,char,strlen(image_file_name)+1))
					{
						strcpy(temp_file_name,image_file_name);
						/* assign values */
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

int Texture_get_raw_pixel_values(struct Texture *texture,int x,int y,
	unsigned char *values)
/*******************************************************************************
LAST MODIFIED : 13 April 1999

DESCRIPTION :
Returns the byte values in the texture at x,y.
==============================================================================*/
{
	int i,number_of_bytes, return_code,row_width_bytes;
	unsigned char *pixel_ptr;

	ENTER(Texture_get_raw_pixel_values);
	if (texture&&(0<=x)&&(x<texture->original_width_texels)&&
		(0<=y)&&(y<texture->original_height_texels)&&values)
	{
		number_of_bytes = Texture_get_number_of_components_from_storage_type(texture->storage)
			* texture->number_of_bytes_per_component;
		row_width_bytes=
			((int)(texture->width_texels*number_of_bytes+3)/4)*4;
		pixel_ptr=(unsigned char *)(texture->image);
		pixel_ptr += (y*row_width_bytes+x*number_of_bytes);
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
	double x, double y, double *values)
/*******************************************************************************
LAST MODIFIED : 13 April 1999

DESCRIPTION :
Returns the byte values in the texture using the texture coordinates relative
to the physical size.  Each texel is assumed to apply exactly
at its centre and the filter_mode used to determine whether the pixels are
interpolated or not.  When closer than half a texel to a boundary the colour 
is constant from the half texel location to the edge. 
==============================================================================*/
{
	double max_x, max_y, local_xi1, local_xi2, weight;
	int bytes_per_component,i,j,number_of_bytes,number_of_components,return_code,
		row_width_bytes,x_i, y_i, pixela, pixelb, pixelc, pixeld;
	unsigned char *pixel_ptr;

	ENTER(Texture_get_pixel_values);
	if (texture && values)
	{
		number_of_components = Texture_get_number_of_components_from_storage_type(texture->storage);
		bytes_per_component = texture->number_of_bytes_per_component;
		number_of_bytes = number_of_components * bytes_per_component;
		row_width_bytes=
			((int)(texture->width_texels*number_of_bytes+3)/4)*4;

		x /= texture->width;
		y /= texture->height;

		switch(texture->wrap_mode)
		{
			case TEXTURE_CLAMP_WRAP:
			{
				if(x < 0.0)
				{
					x = 0.0;
				}
				if(x > 1.0)
				{
					x = 1.0;
				}
				if(y < 0.0)
				{
					y = 0.0;
				}
				if(y > 1.0)
				{
					y = 1.0;
				}
			} break;
			case TEXTURE_REPEAT_WRAP:
			{
				x -= floor(x);
				y -= floor(y);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_pixel_values.  Unknown wrap type");
				return_code=0;
			}
		}

		x *= (double)(texture->original_width_texels);
		y *= (double)(texture->original_height_texels);
		max_x = (double)texture->original_width_texels - 0.5;
		max_y = (double)texture->original_width_texels - 0.5;

		switch (texture->filter_mode)
		{
			case TEXTURE_LINEAR_FILTER:
			{
				if ((x > 0.5) && (x < max_x))
				{
					x_i = (int)(x-0.5);
					local_xi1 = x - 0.5 - (double)x_i;
					
					pixela = x_i*number_of_bytes;
					pixelb = pixela + number_of_bytes;
				}
				else
				{
					if (x < 0.5)
					{
						local_xi1 = 0;
						pixela = 0;
						pixelb = 0;
					}
					else
					{
						local_xi1 = 0;
						pixela = (texture->original_width_texels-1)
							*number_of_bytes;
						pixelb = pixela;
					}
				}
				if ((y > 0.5) && (y < max_y))
				{
					y_i = (int)(y-0.5);
					local_xi2 = y - 0.5 - (double)y_i;

					pixela += y_i*row_width_bytes;
					pixelb += y_i*row_width_bytes;
					pixelc = pixela + row_width_bytes;
					pixeld = pixelb + row_width_bytes;
				}
				else
				{
					if (y < 0.5)
					{
						local_xi2 = 0;
						pixelc = pixela;
						pixeld = pixelb;
					}
					else
					{
						local_xi2 = 0;
						pixela += (texture->original_height_texels-1)
							* row_width_bytes;
						pixelb += (texture->original_height_texels-1)
							* row_width_bytes;
						pixelc = pixela;
						pixeld = pixelb;
					}
				}

				for (i=0;i<number_of_components;i++)
				{
					values[i]=0.0;
				}

				pixel_ptr=(unsigned char *)(texture->image) + pixela;
				weight = (1.0 - local_xi1)*(1.0 - local_xi2) / 255.0;
				for (j=0;j<bytes_per_component;j++)
				{
					for (i=0;i<number_of_components;i++)
					{
						values[i]+=(double)pixel_ptr[i*bytes_per_component+j] * weight;
					}
					weight /= 255.0;
				}

				pixel_ptr=(unsigned char *)(texture->image) + pixelb;
				weight = local_xi1*(1.0 - local_xi2) / 255.0;
				for (j=0;j<bytes_per_component;j++)
				{
					for (i=0;i<number_of_components;i++)
					{
						values[i]+=(double)pixel_ptr[i*bytes_per_component+j] * weight;
					}
					weight /= 255.0;
				}

				pixel_ptr=(unsigned char *)(texture->image) + pixelc;
				weight = (1.0 - local_xi1)*local_xi2 / 255.0;
				for (j=0;j<bytes_per_component;j++)
				{
					for (i=0;i<number_of_components;i++)
					{
						values[i]+=(double)pixel_ptr[i*bytes_per_component+j] * weight;
					}
					weight /= 255.0;
				}

				pixel_ptr=(unsigned char *)(texture->image) + pixeld;
				weight = local_xi1*local_xi2 / 255.0;
				for (j=0;j<bytes_per_component;j++)
				{
					for (i=0;i<number_of_components;i++)
					{
						values[i]+=(double)pixel_ptr[i*bytes_per_component+j] * weight;
					}
					weight /= 255.0;
				}
		
				return_code=1;
			} break;
			case TEXTURE_NEAREST_FILTER:
			{
				x_i = (int)x;
				y_i = (int)y;

				pixela = x_i * number_of_bytes + y_i * row_width_bytes;

				pixel_ptr=(unsigned char *)(texture->image) + pixela;
				for (i=0;i<number_of_components;i++)
				{
					values[i]=(double)pixel_ptr[i*bytes_per_component] / 255.0;
				}
				weight = 255.0;
				for (j=1;j<bytes_per_component;j++)
				{
					weight *= 255.0;
					for (i=0;i<number_of_components;i++)
					{
						values[i]+=(double)pixel_ptr[i*bytes_per_component+j] / weight;
					}
				}
				return_code=1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_pixel_values.  Unknown filter type");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_pixel_values.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_pixel_values */

char *Texture_get_image_file_name(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 13 February 1998

DESCRIPTION :
Returns the name of the file from which the current texture image was read.
==============================================================================*/
{
	char *return_file_name;

	ENTER(Texture_get_image_file_name);
	if (texture)
	{
		return_file_name=texture->image_file_name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_image_file_name.  Missing texture");
		return_file_name=(char *)NULL;
	}
	LEAVE;

	return (return_file_name);
} /* Texture_get_image_file_name */

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
} /* Texture_get_image_file_name */

int Texture_get_number_of_components(struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Returns the number of 8-bit components used per texel (1,2,3 or 4) in the image.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_number_of_components);
	if (texture)
	{
		return_code = Texture_get_number_of_components_from_storage_type(texture->storage);
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
	int *original_width_texels,int *original_height_texels)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the width and height of the image from which the texture was read. May
differ from the dimensions of the texture which is in powers of 2.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_original_size);
	if (texture&&original_width_texels&&original_height_texels)
	{
		*original_width_texels=texture->original_width_texels;
		*original_height_texels=texture->original_height_texels;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_original_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_original_size */

int Texture_get_physical_size(struct Texture *texture,float *width,
	float *height)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Returns the physical size in model coordinates of the original texture image.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_physical_size);
	if (texture&&width&&height)
	{
		*width=texture->width;
		*height=texture->height;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_physical_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_physical_size */

int Texture_set_physical_size(struct Texture *texture,float width,float height)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the physical size in model coordinates of the original texture image. The
default is 1.0, so that texture coordinates in the range from 0 to 1 represent
real image data and not padding to make image sizes up to powers of 2.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_set_physical_size);
	if (texture&&(0.0!=width)&&(0.0!=height))
	{
		if ((width != texture->width)||(height != texture->height))
		{
			texture->width=width;
			texture->height=height;
			/* display list needs to be compiled again */
			texture->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_set_physical_size.  Invalid argument(s)");
		return_code=0;
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
	int *width_texels,int *height_texels)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the dimensions of the texture image in powers of 2. The width and height
returned are at least as large as those of the original image read into the
texture.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_get_size);
	if (texture&&width_texels&&height_texels)
	{
		*width_texels=texture->width_texels;
		*height_texels=texture->height_texels;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Texture_get_size.  Invalid argument(s)");
		return_code=0;
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
		wrap_mode = TEXTURE_WRAP_MODE_INVALID;
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

#if defined (IMAGEMAGICK)
int Texture_write_to_file(struct Texture *texture, char *file_name)
#else /* defined (IMAGEMAGICK) */
int Texture_write_to_file(struct Texture *texture, char *file_name,
	enum Image_file_format file_format)
#endif /* defined (IMAGEMAGICK) */
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Writes the image stored in the texture to a file.
==============================================================================*/
{
	int return_code;

	ENTER(Texture_write_to_file);
	if (texture&&file_name)
	{
#if defined (IMAGEMAGICK)
		return_code = write_image_file(file_name, 
			Texture_get_number_of_components_from_storage_type(texture->storage),
			texture->number_of_bytes_per_component,
			texture->original_height_texels,texture->original_width_texels, 
			(texture->width_texels)-(texture->original_width_texels),
			texture->image);
#else /* defined (IMAGEMAGICK) */
		switch (file_format)
		{
			case RGB_FILE_FORMAT:
			{
				return_code = write_rgb_image_file(file_name, 
					Texture_get_number_of_components_from_storage_type(texture->storage),
					texture->number_of_bytes_per_component,
					texture->original_height_texels,texture->original_width_texels, 
					(texture->width_texels)-(texture->original_width_texels),
					texture->image);
			} break;
			case TIFF_FILE_FORMAT:
			{
				return_code = write_tiff_image_file(file_name, 
					Texture_get_number_of_components_from_storage_type(texture->storage),
					texture->number_of_bytes_per_component,
					texture->original_height_texels,texture->original_width_texels, 
					(texture->width_texels)-(texture->original_width_texels),
					TIFF_PACK_BITS_COMPRESSION,texture->image);
			} break;
			case POSTSCRIPT_FILE_FORMAT:
			{
				/*???SAB.  The postscript routines require user_interface at the moment
					but I don't want to impose that requirement on this routine */
				display_message(ERROR_MESSAGE,
					"Texture_write_to_file.  Postscript file format not implemented");
				return_code=0;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_write_to_file.  Unknown file format");
				return_code=0;
			}
		}
#endif /* defined (IMAGEMAGICK) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_write_to_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_write_to_file */

int list_Texture(struct Texture *texture,void *dummy)
/*******************************************************************************
LAST MODIFIED : 11 October 2000

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
			"  size in model units.  height = %g, width = %g\n",
			texture->height,texture->width);
		/* write the radial distortion parameters in physical space */
		display_message(INFORMATION_MESSAGE,"  radial distortion in model space.  "
			"centre = %g,%g  factor_k1 = %g\n",texture->distortion_centre_x,
			texture->distortion_centre_y,texture->distortion_factor_k1);
		/* write the name of the image file */
		display_message(INFORMATION_MESSAGE,"  image file name : ");
		if (texture->image_file_name)
		{
			display_message(INFORMATION_MESSAGE,"%s\n",texture->image_file_name);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"<NONE>\n");
		}
		/* write the original image size */
		display_message(INFORMATION_MESSAGE,
			"  original width (texels) = %d, original height (texels) "
			"= %d\n",texture->original_width_texels,texture->original_height_texels);
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
			"  width (texels) = %d, height (texels) = %d\n",
			texture->width_texels,texture->height_texels);
		/* write the type of wrapping */
		display_message(INFORMATION_MESSAGE,
			"  wrap : %s\n",Texture_wrap_mode_string(texture->wrap_mode));
		/* write the type of magnification/minification filter */
		display_message(INFORMATION_MESSAGE,
			"  magnification/minification filter : %s\n",
			Texture_filter_mode_string(texture->filter_mode));
		/* write the combine type */
		display_message(INFORMATION_MESSAGE,"  combine type : %s\n",
			Texture_combine_mode_string(texture->combine_mode));
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
LAST MODIFIED : 22 January 2002

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
		if (texture->movie)
		{
			display_message(INFORMATION_MESSAGE," movie");
		}
		/*???debug*/if (texture->image_file_name&&texture->movie)
		{
			printf("Texture %s has image_file_name and movie!!\n",texture->name);
		}
		/* write the texture size in model units */
		display_message(INFORMATION_MESSAGE," height %g width %g",
			texture->height,texture->width);
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
			Texture_combine_mode_string(texture->combine_mode));
		/* write the type of magnification/minification filter */
		display_message(INFORMATION_MESSAGE," %s",
			Texture_filter_mode_string(texture->filter_mode));
		/* write the type of wrapping */
		display_message(INFORMATION_MESSAGE," %s",
			Texture_wrap_mode_string(texture->wrap_mode));
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
LAST MODIFIED : 25 November 1999

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
	unsigned int old_texture_id;
#endif /* defined (OPENGL_API) */

	ENTER(compile_Texture);
	if (texture && (!dummy_void))
	{
		if (texture->display_list_current == 1)
		{
			return_code=1;
		}
		else
		{
#if defined (OPENGL_API)
			if (texture->display_list||(texture->display_list=glGenLists(1)))
			{
				if(/* 0 &&    SAB Digifarmers are occasionally getting 
						            the wrong texture loaded up so can disable
					               texture_objects here */
					query_gl_extension("GL_EXT_texture_object"))
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
								glBindTexture(GL_TEXTURE_2D, texture->texture_id);
#if defined (EXT_subtexture)
								Texture_get_type_and_format_from_storage_type(texture->storage,
									&type, &format);
								glTexSubImage2DEXT(GL_TEXTURE_2D, 0, 0, 0,
									(GLint)(texture->width_texels), (GLint)(texture->height_texels),
									format, type, (GLvoid *)(texture->image));
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
						glBindTexture(GL_TEXTURE_2D, texture->texture_id);
						if(texture->storage==TEXTURE_DMBUFFER || 
							texture->storage==TEXTURE_PBUFFER)
						{
							Dm_buffer_glx_make_read_current(texture->dmbuffer);				
							direct_render_Texture(texture);
							X3dThreeDDrawingRemakeCurrent();
						}
						else
						{
							direct_render_Texture(texture);
						}
						glNewList(texture->display_list,GL_COMPILE);
						glBindTexture(GL_TEXTURE_2D, texture->texture_id);
						/* As we have bound the texture we only need the 
							environment in the display list */
						direct_render_Texture_environment(texture);

						glEndList();
					}
				}
				else
				{
					glNewList(texture->display_list,GL_COMPILE);
					direct_render_Texture(texture);
					glEndList();
				}
#if defined (OLD_CODE) /* ! defined (GL_EXT_texture_object) */
				glNewList(texture->display_list,GL_COMPILE);
				direct_render_Texture(texture);
				glEndList();
#endif /* defined (OLD_CODE) */
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
LAST MODIFIED : 25 November 1999

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
	int resident;
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
			glGetTexParameteriv(GL_TEXTURE_2D,
				GL_TEXTURE_RESIDENT,
				&resident);
			if(GL_TRUE != resident)
			{
				/* Reload the texture */
				if(texture->storage==TEXTURE_DMBUFFER || 
					texture->storage==TEXTURE_PBUFFER)
				{
					Dm_buffer_glx_make_read_current(texture->dmbuffer);				
					direct_render_Texture(texture);
					X3dThreeDDrawingRemakeCurrent();
				}
				else
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
		glDisable(GL_TEXTURE_2D);
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
