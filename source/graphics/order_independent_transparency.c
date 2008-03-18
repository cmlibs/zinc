/*******************************************************************************
FILE : order_independent_transparency.c

LAST MODIFIED : 19 March 2008

DESCRIPTION :
Implements in NVIDIA and ATI hardware and Mesa software a depth sorting
algorithm where each depth layer is peeled off and then composited back
together from back to front at the end.  

HISTORY :
  Based on algorithm from NVIDIA using SGIS_shadow and NV_texture_shader to
  implement Rui Bastos's idea for getting the layers of z sorted
  by depth, and using the RGBA at each layer to re-order transparency
  correctly.
  Cass Everitt 3-28-01
  SAB. I got rid of the glh, c++ and nv_parse stuff.
  Uses ARB vertex and fragment programs.
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

#include <stdio.h>
#include <stdlib.h>

#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#include "graphics/order_independent_transparency.h"

#if ! defined GL_ARB_texture_rectangle
#   if defined GL_EXT_texture_rectangle
#      define GL_ARB_texture_rectangle GL_EXT_texture_rectangle
#   endif /* defined GL_EXT_texture_rectangle */
#endif /* ! defined GL_ARB_texture_rectangle */

static char *required_extensions_default[] = {"GL_ARB_texture_rectangle",
												  "GL_ARB_vertex_program",
												  "GL_ARB_fragment_program",
												  "GL_ARB_fragment_program_shadow",
												  "GL_ARB_depth_texture",
												  "GL_ARB_shadow"};
/* Mesa actually does the shadow test automatically but does not expose
the extension, and ATI on Linux has the extension but does not list it
in the extensions string. */
static char *required_extensions_mesa_ati[] = {"GL_ARB_texture_rectangle",
												  "GL_ARB_vertex_program",
												  "GL_ARB_fragment_program",
												  "GL_ARB_depth_texture"};
#if defined GL_ARB_texture_rectangle && defined GL_ARB_vertex_program \
   && defined GL_ARB_fragment_program && defined  GL_ARB_fragment_program_shadow \
   && defined GL_ARB_depth_texture && defined GL_ARB_shadow
#define ORDER_INDEPENDENT_CAPABLE
#endif

#if defined (ORDER_INDEPENDENT_CAPABLE)
struct Scene_viewer_order_independent_transparency_data
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
The private user data for this order independent transparency rendering pass.
==============================================================================*/
{
	GLuint ztex_texture_id;
	GLuint *rgba_layer_texture_id;

	int viewport_width;
	int viewport_height;

	int using_stencil_overlay;

	int number_of_layers;
	int maximum_number_of_layers;

	GLuint *zbuffer;	

	GLenum depth_format;

	enum Graphics_library_vendor_id vendor_id;

	struct Scene_viewer *scene_viewer;
};

static void set_texgen_planes(GLenum plane_type, float matrix[16])
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Sets all the texture generation coordinates to <plane_type> according to
the planes in the <matrix>.
==============================================================================*/
{
	int i;
	GLenum coord[] = {GL_S, GL_T, GL_R, GL_Q };

	ENTER(set_texgen_planes);
	for(i = 0; i < 4; i++)
	{
		glTexGenfv(coord[i], plane_type, matrix + 4 * i);
	}
	LEAVE;
} /* set_texgen_planes */

void eye_linear_texgen(float matrix[16])
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Sets all the texture generation coordinates to GL_EYE_LINEAR according to
the planes in the <matrix>.
==============================================================================*/
{
	ENTER(eye_linear_texgen);

	set_texgen_planes(GL_EYE_PLANE, matrix);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	LEAVE;
} /* eye_linear_texgen */

void obj_linear_texgen(float matrix[16])
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Sets all the texture generation coordinates to GL_OBJECT_LINEAR according to
the planes in the <matrix>.
==============================================================================*/
{
	ENTER(eye_linear_texgen);

	set_texgen_planes(GL_OBJECT_PLANE, matrix);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

	LEAVE;
} /* obj_linear_texgen */

static int order_independent_init_opengl(
	struct Scene_viewer_order_independent_transparency_data *data)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Initialises the order independent transparency extension.
==============================================================================*/
{
	int return_code;

	ENTER(order_independent_init_opengl);

	return_code = 1;
#if defined (REPORT_GL_ERRORS)
		{
			char message[200];
			GLenum error;
			while(GL_NO_ERROR!=(error = glGetError()))
			{
				strcpy(message,"order_independent_init_opengl A: GL ERROR ");
				strcat(message, (char *)gluErrorString(error));
				display_message(ERROR_MESSAGE, message);
			}
		}
#endif /* defined (REPORT_GL_ERRORS) */
	if (!data->ztex_texture_id)
	{
		glGenTextures(1, &data->ztex_texture_id);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->ztex_texture_id);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		if (Graphics_library_vendor_mesa == data->vendor_id)
		{
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
		}
#if defined (REPORT_GL_ERRORS)
		{
			char message[200];
			GLenum error;
			while(GL_NO_ERROR!=(error = glGetError()))
			{
				strcpy(message,"order_independent_init_opengl BA: GL ERROR ");
				strcat(message, (char *)gluErrorString(error));
				display_message(ERROR_MESSAGE, message);
			}
		}
#endif /* defined (REPORT_GL_ERRORS) */

	}

	LEAVE;

	return (return_code);
} /* order_independent_init_opengl */

static void render_scene_from_camera_view(int layer,
	struct Scene_viewer_rendering_data *rendering_data,
	struct Scene_viewer_order_independent_transparency_data *data,
	double *projection_matrix, double *modelview_matrix)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Draws one peeled layer of the scene.
==============================================================================*/
{
#if defined (OLD_CODE)
#define MAX_PROGRAM (20000)
	char vertex_program_string[MAX_PROGRAM], fragment_program_string[MAX_PROGRAM];
 	const GLubyte *error_msg;
	static GLuint vertex_program = 0, fragment_program = 0;
#endif /* defined (OLD_CODE) */

	ENTER(render_scene_from_camera_view);
	USE_PARAMETER(modelview_matrix);
	USE_PARAMETER(projection_matrix);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


#if defined (OLD_CODE)
	{
		FILE *program_file;
		int count;
		char *vertex_program_filename, first_peel_vertex[] = "first_peel.vp",
			peel_vertex[] = "peel.vp";
		char *fragment_program_filename, first_peel_fragment[] = "first_peel.fp",
			peel_fragment[] = "peel.fp";
		
		if (layer > 0)
		{
			vertex_program_filename = peel_vertex;
			fragment_program_filename = peel_fragment;
		}
		else
		{
			vertex_program_filename = first_peel_vertex;
			fragment_program_filename = first_peel_fragment;
		}

		if (program_file = fopen(vertex_program_filename, "r"))
		{
			count = fread(vertex_program_string, 1, MAX_PROGRAM - 1, program_file);
			vertex_program_string[count] = 0;
			if (count > MAX_PROGRAM - 2)
			{
				display_message(ERROR_MESSAGE, "Material_program_compile.  "
					"Short read on test.vp, need to increase MAX_PROGRAM.");
			}
			fclose (program_file);
		}
		else
		{
			display_message(ERROR_MESSAGE, "Material_program_compile.  "
				"Unable to open file test.vp.");
		}

		if (program_file = fopen(fragment_program_filename, "r"))
		{
			count = fread(fragment_program_string, 1, MAX_PROGRAM - 1, program_file);
			fragment_program_string[count] = 0;
			if (count > MAX_PROGRAM - 2)
			{
				display_message(ERROR_MESSAGE, "Material_program_compile.  "
					"Short read on test.fp, need to increase MAX_PROGRAM.");
			}
			fclose (program_file);
		}
		else
		{
			display_message(ERROR_MESSAGE, "Material_program_compile.  "
				"Unable to open file test.fp.");
		}
	}

	if (!vertex_program)
	{
		glGenProgramsARB(1, &vertex_program);
	}
		
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertex_program);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		strlen(vertex_program_string), vertex_program_string);
	error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

	
	if (!fragment_program)
	{
		glGenProgramsARB(1, &fragment_program);
	}
	
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragment_program);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		strlen(fragment_program_string), fragment_program_string);
	error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
	
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB,
		vertex_program);

	
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
		fragment_program);

#endif /* defined (OLD_CODE) */

	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1,
		data->viewport_width, data->viewport_height, 1.0, 1.0);

	if(layer > 0)
	{
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->ztex_texture_id);
		if (Graphics_library_vendor_ati == data->vendor_id)
		{
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
		}
		else if (Graphics_library_vendor_mesa == data->vendor_id)
		{
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);
		}

	}	
#if defined (DEBUG)
	else
	{
		/* So we can depth test even on the first path when debugging */
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->ztex_texture_id);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
		memset(data->zbuffer, 0, sizeof(GLuint) * data->viewport_width * data->viewport_height);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, data->depth_format, 
			data->viewport_width, data->viewport_height, 0, 
			GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data->zbuffer);

	}
#endif /* defined (DEBUG) */

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	glEnable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);

	Scene_viewer_call_next_renderer(rendering_data);

	if(layer < data->number_of_layers - 1)
	{
		/* copy the z buffer unless this is the last peeling pass */
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->ztex_texture_id);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0,
			data->viewport_width, data->viewport_height);
		glActiveTexture(GL_TEXTURE0);
	}

	/* copy the RGBA of the layer */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->rgba_layer_texture_id[layer]);
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0,
		data->viewport_width, data->viewport_height);


	LEAVE;

} /* render_scene_from_camera_view */

static void draw_sorted_transparency(
	struct Scene_viewer_order_independent_transparency_data *data,
	enum Scene_viewer_blending_mode blending_mode)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Draw a textured quad for each layer and blend them all together correctly.
==============================================================================*/
{
	int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, data->viewport_width, 0, data->viewport_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	if (data->using_stencil_overlay)
	{
		/* disable stencil buffer to get the overlay in the back plane */
		glDisable(GL_STENCIL_TEST);
	}

	switch(blending_mode)
	{
		default:
		case SCENE_VIEWER_BLEND_NORMAL:
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		} break;
		case SCENE_VIEWER_BLEND_NONE:
		{
			glDisable(GL_BLEND);
		} break;
#if defined GL_VERSION_1_4
		case SCENE_VIEWER_BLEND_TRUE_ALPHA:
		{
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
				GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		} break;
#endif /* defined GL_VERSION_1_4 */
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for(i = data->number_of_layers - 1 ; i >= 0 ; i--)
	{
		if (data->number_of_layers - 2 == i)
		{
			if (data->using_stencil_overlay)
			{
				glEnable(GL_STENCIL_TEST);
			}
		}

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->rgba_layer_texture_id[i]);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(0, data->viewport_height);
		glVertex2f(0, data->viewport_height);
		glTexCoord2f(data->viewport_width, data->viewport_height);
		glVertex2f(data->viewport_width, data->viewport_height);
		glTexCoord2f(data->viewport_width, 0);
		glVertex2f(data->viewport_width, 0);
		glEnd();
	}

	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	LEAVE;

}
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

int order_independent_capable(void)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Returns true if the current display is capable of order independent transparency.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	GLint alpha_bits, depth_bits;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */
	char **required_extensions;
	int return_code;
	unsigned int i, number_of_required_extensions;
	ENTER(order_independent_capable);

	enum Graphics_library_vendor_id vendor_id = Graphics_library_get_vendor_id();
	if ((Graphics_library_vendor_mesa == vendor_id) ||
		(Graphics_library_vendor_ati == vendor_id))
	{
		required_extensions = required_extensions_mesa_ati;
		number_of_required_extensions = (sizeof(required_extensions_mesa_ati) / sizeof (char *));
	}
	else
	{
		required_extensions = required_extensions_default;
		number_of_required_extensions = (sizeof(required_extensions_default) / sizeof (char *));
	}
#if defined (ORDER_INDEPENDENT_CAPABLE)
	return_code = 1;
	for (i = 0 ; i < number_of_required_extensions ; i++)
	{
		if (!Graphics_library_load_extension(required_extensions[i]))
		{
			return_code = 0;
		}
	}
	glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
	if ((depth_bits != 16) && (depth_bits != 24))
	{
		return_code = 0;
	}
	glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
	if (alpha_bits < 8)
	{
		return_code = 0;
	}
	if (!return_code)
	{
		display_message(ERROR_MESSAGE,
			"Order independent transparency not supported on this display\n"
			"It requries at least 8 alpha bits (detected %d), 16 or 24 bit depth buffer (detected %d) and "
			"these OpenGL extensions: ", alpha_bits, depth_bits);
		for (i = 0 ; i < number_of_required_extensions ; i++)
		{
			if (Graphics_library_load_extension(required_extensions[i]))
			{
				display_message(ERROR_MESSAGE,"%s: Available", required_extensions[i]);
			}
			else
			{
				display_message(ERROR_MESSAGE,"%s: Not Available", required_extensions[i]);
			}
		}
	}
#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
 	display_message(ERROR_MESSAGE,
		"Order independent transparency not compiled into this executable.  \n"
		"It requries at least 8 alpha bits, 16 or 24 bit depth buffer and "
		"these OpenGL extensions: ");
	for (i = 0 ; (i < (sizeof(required_extensions) / sizeof (char *))) ; i++)
	{
		display_message(ERROR_MESSAGE,"%s ", required_extensions[i]);
	}
	return_code = 0;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;

	return (return_code);
} /* order_independent_capable */

struct Scene_viewer_order_independent_transparency_data *
   order_independent_initialise(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Initialises the order independent transparency extension.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	GLint alpha_bits, depth_bits;
	int return_code;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */
	struct Scene_viewer_order_independent_transparency_data *data;

	ENTER(order_independent_initialise);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	if (ALLOCATE(data, struct Scene_viewer_order_independent_transparency_data,
		1))
	{
		return_code = 1;

		data->ztex_texture_id = 0;
		data->rgba_layer_texture_id = (GLuint *)NULL;

		data->viewport_width = 0;
		data->viewport_height = 0;

		data->number_of_layers = 0;
		data->maximum_number_of_layers = 0;

		data->zbuffer = (GLuint *)NULL;

		data->scene_viewer = scene_viewer;

		data->vendor_id = Graphics_library_get_vendor_id();

		glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
		glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
		
		switch (depth_bits)
		{
			case 16:
			{
				data->depth_format = GL_DEPTH_COMPONENT16_ARB;
			} break;
			case 24:
			{
				data->depth_format = GL_DEPTH_COMPONENT24_ARB;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "order_independent_initialise.  "
					"Unsupported depth format for order independent transparency");
				return_code = 0;
			} break;
		}

		if (alpha_bits < 8)
		{
			display_message(ERROR_MESSAGE, "order_independent_initialise.  "
				"This extension requires alpha planes to work, alpha_bits = %d",
				alpha_bits);
			return_code = 0;
		}

		if (!return_code)
		{
			DEALLOCATE(data);
			data = (struct Scene_viewer_order_independent_transparency_data *)NULL;
		}
		else
		{
			order_independent_init_opengl(data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "order_independent_initialise.  "
				"Unable to allocate data structure\n");
		data = (struct Scene_viewer_order_independent_transparency_data *)NULL;
	}
#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
	USE_PARAMETER(scene_viewer);
	data = (struct Scene_viewer_order_independent_transparency_data *)NULL;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;

	return (data);
} /* order_independent_initialise */

int order_independent_reshape(
	struct Scene_viewer_order_independent_transparency_data *data,
	int width, int height, int layers, int using_stencil_overlay)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Initialises per rendering parts of this extension.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int i;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */
	int return_code;

	ENTER(order_independent_reshape);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	data->using_stencil_overlay = using_stencil_overlay;
	/* We need one more layer for the background */
	layers++;
	if (data->ztex_texture_id && data->zbuffer && (data->viewport_width == width)
		&& (data->viewport_height == height) && (layers <= data->maximum_number_of_layers))
	{
		data->number_of_layers = layers;
		return_code = 1;
	}
	else
	{
		return_code = 1;
		if (!data->ztex_texture_id)
		{
			glGenTextures(1, &data->ztex_texture_id);
		}

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->ztex_texture_id);
		
		if (REALLOCATE(data->zbuffer, data->zbuffer, GLuint, width * height))
		{
			memset(data->zbuffer, 0, sizeof(GLuint) * width * height);
			data->viewport_width = width;
			data->viewport_height = height;
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, data->depth_format, 
				width, height, 0, 
				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data->zbuffer);
		}
		else
		{
			display_message(ERROR_MESSAGE, "order_independent_reshape.  "
				"Unable to allocate ztex buffer\n");
			return_code = 0;
		}
		glActiveTexture(GL_TEXTURE0);

		if (return_code && (!data->rgba_layer_texture_id ||
			(layers > data->maximum_number_of_layers)))
		{
			if (REALLOCATE(data->rgba_layer_texture_id, data->rgba_layer_texture_id,
				GLuint, layers))
			{
				for (i = data->maximum_number_of_layers ; i < layers ; i++)
				{
					glGenTextures(1, &(data->rgba_layer_texture_id[i]));
				}
				data->maximum_number_of_layers = layers;
			}
			else
			{
				display_message(ERROR_MESSAGE, "order_independent_reshape.  "
					"Unable to allocate rgba layer ids\n");
				return_code = 0;
			}
		}

		if (return_code)
		{
			data->number_of_layers = layers;
			for (i = 0 ; i < data->maximum_number_of_layers ; i++)
			{
				glBindTexture(GL_TEXTURE_RECTANGLE_ARB, data->rgba_layer_texture_id[i]);
				glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, width, height, 0, 
					GL_RGBA, GL_UNSIGNED_BYTE, data->zbuffer);
				glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
	}
#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
	USE_PARAMETER(data);
	USE_PARAMETER(width);
	USE_PARAMETER(height);
	USE_PARAMETER(layers);
	USE_PARAMETER(using_stencil_overlay);
	return_code = 0;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;

	return (return_code);
} /* order_independent_reshape */

void order_independent_display(struct Scene_viewer_rendering_data *rendering_data,
	struct Scene_viewer_order_independent_transparency_data *data,
	double *projection_matrix, double *modelview_matrix,
	enum Scene_viewer_blending_mode blending_mode)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Actually preforms the rendering pass.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int layer;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	ENTER(order_independent_display);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	/* glViewport(0, 0, data->viewport_width, data->viewport_height); */

	/* Copy the image that is already drawn into the back layer */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,
		data->rgba_layer_texture_id[data->number_of_layers - 1]);
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0,
		data->viewport_width, data->viewport_height);

	/* Always create the textures with this, use the blending mode
		to composite the images together */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);

	USE_PARAMETER(rendering_data);
	USE_PARAMETER(data);
	USE_PARAMETER(projection_matrix);
	USE_PARAMETER(modelview_matrix);
	
	for(layer = 0; layer < data->number_of_layers - 1 ; layer++) 
	{
		/* Recompile the materials for order_independent_transaprency */
		{
			struct Material_order_independent_transparency material_data;
			
			material_data.layer = layer + 1;
			material_data.graphics_buffer = 
				Scene_viewer_get_graphics_buffer(data->scene_viewer);
			
			Scene_for_each_material(Scene_viewer_get_scene(data->scene_viewer),
				compile_Graphical_material_for_order_independent_transparency,
				(void *)&material_data);
		}

		render_scene_from_camera_view(layer, rendering_data, data,
			projection_matrix, modelview_matrix);
	}

	draw_sorted_transparency(data, blending_mode);

	/* Recompile the materials back to the original state */
	{
		struct Material_order_independent_transparency material_data;
	 
		material_data.layer = 0;
		
		Scene_for_each_material(Scene_viewer_get_scene(data->scene_viewer),
			compile_Graphical_material_for_order_independent_transparency,
			(void *)&material_data);
	}

#if defined (DEBUG)
	{
		FILE *out;
		printf("Writing depth.raw -geometry %dx%d -depth 32\n", data->viewport_width, data->viewport_height);
		glReadPixels(0, 0, data->viewport_width, data->viewport_height, GL_DEPTH_COMPONENT,
			GL_UNSIGNED_INT, data->zbuffer);
		out = fopen("depth.raw", "w");
		fwrite(data->zbuffer, sizeof(GLuint), data->viewport_width * data->viewport_height,
			out);
		fclose(out);
	}
#endif /* defined (DEBUG) */

#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
	USE_PARAMETER(rendering_data);
	USE_PARAMETER(data);
	USE_PARAMETER(projection_matrix);
	USE_PARAMETER(modelview_matrix);
	USE_PARAMETER(blending_mode);
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;
} /* order_independent_display */

int order_independent_finalise(
	struct Scene_viewer_order_independent_transparency_data **data_address)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Frees the memory associated with the <data_address> and sets <data_address> to NULL.
==============================================================================*/
{
	int return_code;
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int i;
	struct Scene_viewer_order_independent_transparency_data *data;	
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	ENTER(order_independent_finalise);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	if (data_address && (data = *data_address))
	{
		DEALLOCATE(data->zbuffer);
		for (i = 0 ; i < data->maximum_number_of_layers ; i++)
		{
			glDeleteTextures(1, &data->rgba_layer_texture_id[i]);
		}
		DEALLOCATE(data->rgba_layer_texture_id);

		if (data->ztex_texture_id)
		{
			glDeleteTextures(1, &data->ztex_texture_id);
		}

		DEALLOCATE(*data_address);
		*data_address = (struct Scene_viewer_order_independent_transparency_data *)NULL;

		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
	USE_PARAMETER(data_address);
	return_code = 0;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;

	return (return_code);
} /* order_independent_finalise */
