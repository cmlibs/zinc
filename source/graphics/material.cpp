/*******************************************************************************
FILE : material.c

LAST MODIFIED : 5 July 2007

DESCRIPTION :
The functions for manipulating graphical materials.

???RC Only OpenGL is supported now.

???RC 28 Nov 97.
		Two functions are now used for activating the graphical material:
- compile_Graphical_material puts the material into its own display list.
- execute_Graphical_material calls that display list.
		The separation into two functions was needed because OpenGL cannot start a
new list while one is being written to. As a consequence, you must precompile
all objects that are executed from within another display list. To make this job
easier, compile_Graphical_material is a list/manager iterator function.
		Future updates of OpenGL may overcome this limitation, in which case the
execute function can take over compiling as well. Furthermore, it is easy to
return to direct rendering, as described with these routines.
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
 * Shane Blackett (shane at blackett.co.nz)
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
extern "C" {
#include "api/cmiss_material.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_image.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/io_stream.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_module.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
}
#include "graphics/rendergl.hpp"
#include "graphics/material.hpp"
#include "graphics/spectrum.hpp"

/*
Module types
------------
*/
enum Material_program_type
/*****************************************************************************//**
@date LAST MODIFIED : 4 July 2007

Enumerates the main different types of vertex/fragment program for materials
==============================================================================*/
{
	/* This type is for Material_programs which have arbitrary specified strings
	 * rather than the program being generated based on this type value.
	 */
	MATERIAL_PROGRAM_SPECIFIED_STRINGS = 0,
	/* This first one is a standard Gouraud Shaded material, included here so
		that it can be peeled in order independent transparency */
	MATERIAL_PROGRAM_GOURAUD_SHADING = 1,
	MATERIAL_PROGRAM_PER_PIXEL_LIGHTING = 2,
	MATERIAL_PROGRAM_PER_PIXEL_TEXTURING = 10,
	MATERIAL_PROGRAM_BUMP_MAPPING = 770,
	MATERIAL_PROGRAM_BUMP_MAPPING_TEXTURING = 778,

	/* These classes modify the above programs and so must be bit independent */
	MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING = 1,
	MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING = 2,
	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a colour texture. */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 = (1<<2),
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 = (1<<3),
	/* If either bit is set then we are using a texture */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE = 12,
	/* If this bit is set then a colour texture will replace the lighting calculation for a colour,
		if it is not set then the texture will modulate the lighting calculation colour */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL = (1<<4),
	/* Specifies the output dimension of the texture and therefore how it is applied.
		OUTPUT1 = grayscale, OUTPUT2 = grayscale and alpha, OUTPUT1 & OUTPUT2 = rgb
		!OUPUT1 & !OUTPUT2 = rgba. */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 = (1<<5),
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 = (1<<6),

	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a second or bump map texture. */
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1 = (1<<7),
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2 = (1<<8),
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE = 384,
   /* Specifies that the second texture is intended to be used as a bump map texture, modulating
		the per pixel value of the normal in the lighting calculation */
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP = (1<<9),

	/* The colour value is used as the input, derived from the primary texture or the lighting.
	   Specify which input components are used directly from the input source,
		at most 3 of the input values can be used (a 3D texture lookup) */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 = (1<<10),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 = (1<<11),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 = (1<<12),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4 = (1<<13),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_INPUTS = (1<<10) + (1<<11) + (1<<12) + (1<<13),

	/* Specify the outputs in the dependent texture lookup, either replacing the colour, alpha or both. */
	/* These modes work by assuming that the dependent_texture inputs form
		the axes of a single 1D, 2D or 3D texture */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR = (1<<14),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA = (1<<15),
	/* This mode works by looking up each input component independently
		in a common 1D texture */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP = (1<<16),

	/* Assume that the texture contains an intensity followed by a 3 component
		normal vector.  This vector is used to light volume rendering by
		performing per pixel lighting using this normal. */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE = (1<<17),
	/* Calculate a normal by using a finite difference operator. */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL = (1<<18),
	/* Scale the alpha by the magnitude of the normal */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_SCALE_ALPHA = (1<<19),

   /* Order independent transparency passes */
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_FIRST_LAYER = (1<<20),
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER = (1<<21)
}; /* enum Material_program_type */

enum Material_program_shader_type
{
	MATERIAL_PROGRAM_SHADER_NONE,
	MATERIAL_PROGRAM_SHADER_ARB,
	MATERIAL_PROGRAM_SHADER_GLSL
};


/*****************************************************************************//**
@date LAST MODIFIED : 20 June 2008

Stores a display list which sets up the correct state for a particular
material state.  This allows vertex/fragment programs to be used per material
but shared between different materials with the same state.
==============================================================================*/
struct Material_program
{
	/*! Specifies the type of the Material Program
	 * These should be unique for each differing program as the materials
	 * will only generate one program for each value of type.
	 * As a special case, type == 0, specifies a predefined arbitrary string
	 * is used and so these should never be shared. */
	enum Material_program_type type;
#if defined (OPENGL_API)
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
	GLuint vertex_program;
	GLuint fragment_program;
	GLuint geometry_program;
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
	GLuint glsl_current_program;
	char *vertex_program_string;
	char *geometry_program_string;
	char *fragment_program_string;
	enum Material_program_shader_type shader_type;
	/*! Display list which enables the correct state for this program */
	GLuint display_list;
#endif /* defined (OPENGL_API) */

	/*! Flag indicating whether the program is compiled or not */
	int compiled;

	int access_count;
}; /* struct Material_program */

DECLARE_LIST_TYPES(Material_program);
PROTOTYPE_LIST_FUNCTIONS(Material_program);
FULL_DECLARE_INDEXED_LIST_TYPE(Material_program);

enum Material_program_uniform_type
{
	MATERIAL_PROGRAM_UNIFORM_TYPE_UNDEFINED,
	MATERIAL_PROGRAM_UNIFORM_TYPE_FLOAT
};

/*****************************************************************************//**
Store a uniform parameter value used by a Material_program.
These values are not stored in the program as a particular material may
use the same program with different values for these parameters.
Currently only the FLOAT type is implemented.
The object currently always stores the values in a double 4 array for simplicity.
==============================================================================*/
struct Material_program_uniform
{
	char *name;
	unsigned int number_of_defined_values;
	enum Material_program_uniform_type type;
	double values[4];
	int access_count;
}; /* struct Material_program_uniform */

DECLARE_LIST_TYPES(Material_program_uniform);
PROTOTYPE_LIST_FUNCTIONS(Material_program_uniform);
FULL_DECLARE_INDEXED_LIST_TYPE(Material_program_uniform);

/***************************************************************************//**
 * A structure for storing related object for the texture of a material.
 * Each material consists four of this (multitexture support).
 */
struct Material_image_texture
{
	struct Texture *texture;
	struct MANAGER(Computed_field) *manager;
	struct Computed_field *field;
	void *callback_id;
	struct Graphical_material *material;
};

struct Graphical_material
/*******************************************************************************
LAST MODIFIED : 23 January 2004

DESCRIPTION :
The properties of a material.
==============================================================================*/
{
	/* the name of the material */
	const char *name;
	/* the colour of the background light (so scattered that its incident
		direction is unknown, comes from all directions) reflected */
	struct Colour ambient;
	/* the colour of the directional light scattered in all directions */
	struct Colour diffuse;
	/* the colour of the light emitted */
	struct Colour emission;
	/* the colour of the directional light which is reflected in a preferred
		direction */
	struct Colour specular;
	/* the transparency */
	MATERIAL_PRECISION alpha;
	/* how sharp and bright the glinting is */
	MATERIAL_PRECISION shininess;
#if defined (OPENGL_API)
	GLuint display_list;

	GLuint brightness_texture_id;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the texture for this material */
	struct Material_image_texture image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	struct Material_image_texture second_image_texture;
	/* third stage multitexture */
	struct Material_image_texture third_image_texture;
	/* fourth stage multitexture */
	struct Material_image_texture fourth_image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	/* spectrum used to render this material */
	struct Spectrum *spectrum;
	/* callback if the spectrum changes */
	void *spectrum_manager_callback_id;
	/* the shared information for Graphical Materials, allowing them to share
	   Material_programs */
	struct Material_package *package;
	/* The normal calculated from the volume texture needs to be
		scaled similarly to how it is scaled into coordinate space,
		we do not take account of rotations or any other distortions.
		Four components as that is what ProgramEnvParameter4fvARB wants. */
	float lit_volume_normal_scaling[4];
	/* the graphics state program that represents this material */
	struct Material_program *program;
	/* user defined uniforms used by the program */
	LIST(Material_program_uniform) *program_uniforms;
	 int access_count, per_pixel_lighting_flag, bump_mapping_flag;
	/* this flag is for external API uses. If a material is set to be persistent
		 then this material will not be removed from the manager after destroy.
	 */
	bool is_managed_flag;

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Graphical_material) *manager;
	int manager_change_status;
}; /* struct Graphical_material */

FULL_DECLARE_INDEXED_LIST_TYPE(Graphical_material);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Graphical_material, Cmiss_graphics_module, void *);

struct Material_package
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Provide an opaque container for shared material information.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *material_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Graphical_material *default_material;
	struct Graphical_material *default_selected_material;
	struct LIST(Material_program) *material_program_list;
	struct Cmiss_region *root_region;
	int access_count;
}; /* struct Material_package */

/*
Module functions
----------------
*/

static struct Material_program *CREATE(Material_program)(enum Material_program_type type)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	struct Material_program *material_program;

	ENTER(CREATE(Material_program));

	if (ALLOCATE(material_program ,struct Material_program, 1))
	{
		material_program->type = type;
#if defined (OPENGL_API)
#if defined GL_ARB_vertex_program &&defined GL_ARB_fragment_program
		material_program->vertex_program = 0;
		material_program->fragment_program = 0;
		material_program->geometry_program = 0;
#endif
		material_program->shader_type=MATERIAL_PROGRAM_SHADER_NONE;
		material_program->glsl_current_program = 0;
		material_program->vertex_program_string = (char *)NULL;
		material_program->geometry_program_string = (char *)NULL;
		material_program->fragment_program_string = (char *)NULL;
		material_program->display_list = 0;
#endif /* defined (OPENGL_API) */
		material_program->compiled = 0;
		material_program->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_program).  Not enough memory");
	}
	LEAVE;

	return (material_program);
} /* CREATE(Material_program) */

/***************************************************************************//**
 * An alternative Material_program constructor that takes explicit program
 * strings.
 */
static struct Material_program *Material_program_create_from_program_strings(
	const char *vertex_program_string, const char *fragment_program_string,
	const char *geometry_program_string)
{
	struct Material_program *material_program;

	ENTER(Material_program_create_from_program_strings);

	if (material_program = CREATE(Material_program)(MATERIAL_PROGRAM_SPECIFIED_STRINGS))
	{
#if defined (OPENGL_API)
		material_program->vertex_program_string =
			duplicate_string(vertex_program_string);
		material_program->fragment_program_string =
			duplicate_string(fragment_program_string);
		if (geometry_program_string)
		{
			material_program->geometry_program_string =
				duplicate_string(geometry_program_string);
		}
#else /* defined (OPENGL_API) */
		USE_PARAMETER(vertex_program_string);
		USE_PARAMETER(fragment_program_string);
		USE_PARAMETER(geometry_program_string);
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_create_from_program_strings.  Not enough memory");
	}
	LEAVE;

	return (material_program);
} /* Material_program_create_from_program_strings */

static int DESTROY(Material_program)(struct Material_program **material_program_address)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Frees the memory for the material_program.
==============================================================================*/
{
	int return_code;
	struct Material_program *material_program;

	ENTER(DESTROY(Material_program));
	if (material_program_address &&
		(material_program = *material_program_address))
	{
		if (0==material_program->access_count)
		{
#if defined (OPENGL_API)
			if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
			{
				glUseProgram(0);
				if (material_program->vertex_program)
				{
					glDeleteShader(material_program->vertex_program);
				}
				if (material_program->fragment_program)
				{
					glDeleteShader(material_program->fragment_program);
				}
				if (material_program->geometry_program)
				{
					glDeleteShader(material_program->geometry_program);
				}
				if (material_program->glsl_current_program)
				{
					glDeleteProgram(material_program->glsl_current_program);
				}
			}
			else if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_ARB)
			{
				if (material_program->vertex_program)
				{
					glDeleteProgramsARB(1, &material_program->vertex_program);
				}
				if (material_program->fragment_program)
				{
					glDeleteProgramsARB(1, &material_program->fragment_program);
				}
			}
			if (material_program->display_list)
			{
				glDeleteLists(material_program->display_list, 1);
			}
			if (material_program->vertex_program_string)
			{
				DEALLOCATE(material_program->vertex_program_string);
				material_program->vertex_program_string = NULL;
			}
			if (material_program->geometry_program_string)
			{
				DEALLOCATE(material_program->geometry_program_string);
				material_program->geometry_program_string = NULL;
			}
			if (material_program->fragment_program_string)
			{
				DEALLOCATE(material_program->fragment_program_string);
				material_program->fragment_program_string = NULL;
			}
#endif /* defined (OPENGL_API) */
			DEALLOCATE(*material_program_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Material_program).  Material program has non-zero access count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_program).  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_program) */

DECLARE_OBJECT_FUNCTIONS(Material_program)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Material_program, type, \
	enum Material_program_type, compare_int)
DECLARE_INDEXED_LIST_FUNCTIONS(Material_program)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Material_program, type,
	enum Material_program_type, compare_int)

static struct Material_program_uniform *CREATE(Material_program_uniform)(char *name)
/*******************************************************************************
==============================================================================*/
{
	struct Material_program_uniform *uniform;

	ENTER(CREATE(Material_program_uniform));

	if (ALLOCATE(uniform, Material_program_uniform, 1))
	{
		uniform->name = duplicate_string(name);
		uniform->type = MATERIAL_PROGRAM_UNIFORM_TYPE_UNDEFINED;
		uniform->number_of_defined_values = 0;
		uniform->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_program).  Not enough memory");
	}
	LEAVE;

	return (uniform);
} /* CREATE(Material_program_uniform) */

static int DESTROY(Material_program_uniform)(struct Material_program_uniform **material_program_uniform_address)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Frees the memory for the material_program.
==============================================================================*/
{
	int return_code;
	Material_program_uniform *uniform;

	ENTER(DESTROY(Material_program_uniform));
	if (material_program_uniform_address &&
			(uniform = *material_program_uniform_address))
	{
		if (0==uniform->access_count)
		{
			if (uniform->name)
				DEALLOCATE(uniform->name)

			DEALLOCATE(*material_program_uniform_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Material_program_uniform).  Material program uniform has non-zero access count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_program_uniform).  Missing address");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_program_uniform) */

DECLARE_OBJECT_FUNCTIONS(Material_program_uniform)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Material_program_uniform, name, \
	char *, strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Material_program_uniform)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Material_program_uniform, name,
	char *, strcmp)

/* Misusing the double array here as the vector parser function gives us an 
 * array of doubles and I don't see the need to copy and pass floats.
 * It isn't called double_vector then because we are going to use it with Uniform?f
 */
static int Material_program_uniform_set_float_vector(Material_program_uniform *uniform,
	unsigned int number_of_values, double *values)
{
	int return_code;
	unsigned int i;
	if (uniform && (number_of_values <= 4))
	{
		uniform->type = MATERIAL_PROGRAM_UNIFORM_TYPE_FLOAT;
		uniform->number_of_defined_values = number_of_values;
		for (i = 0 ; i < number_of_values; i++)
		{
			uniform->values[i] = values[i];
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_uniform_set_float_vector.  Invalid arguments");
		return_code = 0;
	}
	return (return_code);
}

#if defined (OPENGL_API)
#  if defined (GL_VERSION_2_0)
static int Material_program_uniform_write_glsl_values(Material_program_uniform *uniform,
	void *material_void)
{
	int return_code;
	Graphical_material *material;
	if (uniform && (material = static_cast<Graphical_material*>(material_void)))
	{
		GLint location = glGetUniformLocation(material->program->glsl_current_program,
			uniform->name);
		if (location != (GLint)-1)
		{
			switch(uniform->type)
			{
				case MATERIAL_PROGRAM_UNIFORM_TYPE_FLOAT:
				{
					switch(uniform->number_of_defined_values)
					{
						case 1:
						{
							glUniform1f(location, uniform->values[0]);
						} break;
						case 2:
						{
							glUniform2f(location, uniform->values[0], uniform->values[1]);
						} break;
						case 3:
						{
							glUniform3f(location, uniform->values[0], uniform->values[1], uniform->values[2]);
						} break;
						case 4:
						{
							glUniform4f(location, uniform->values[0], uniform->values[1], uniform->values[2], uniform->values[3]);
						} break;
					}
				} break;
			}
		}

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_uniform_set_float_vector.  Invalid arguments");
		return_code = 0;
	}
	return (return_code);
}
#  endif // defined (GL_VERSION_2_0)
#endif // defined (OPENGL_API)

#if defined (OPENGL_API)
static int Material_program_compile(struct Material_program *material_program)
/*******************************************************************************
LAST MODIFIED : 4 July 2007

DESCRIPTION :
Compiles the material program objects.  These are separate objects so they can
be shared by multiple materials using the same program.
==============================================================================*/
{
	 int return_code;
	ENTER(Material_program_compile);
	if (material_program)
	{
#if defined (OPENGL_API)
		/* #define TESTING_PROGRAM_STRINGS */
/* #define WRITE_STRING */
/* #define DEBUG */
#if defined (DEBUG)
		const GLubyte *error_msg;
#endif /* defined (DEBUG) */
		return_code = 1;
#if defined (TESTING_PROGRAM_STRINGS)
		/* If testing always recompile */
		material_program->compiled = 0;
#endif /* defined (TESTING_PROGRAM_STRINGS) */
		if (!material_program->compiled)
		{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0

#if defined GL_ARB_fragment_program && defined GL_ARB_vertex_program
			 if (Graphics_library_check_extension(GL_ARB_fragment_program) &&
				  Graphics_library_check_extension(GL_ARB_vertex_program))
			 {
				 material_program->shader_type=MATERIAL_PROGRAM_SHADER_ARB;
			 }
#endif
#if defined (GL_VERSION_2_0)
			 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_ARB)
			 {
				 if (Graphics_library_check_extension(GL_shading_language))
				 {
					 material_program->shader_type=MATERIAL_PROGRAM_SHADER_GLSL;
				 }
			 }
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
			 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL ||
					 material_program->shader_type==MATERIAL_PROGRAM_SHADER_ARB)
			 {
#if defined DEBUG
				printf ("Compiling program type:%x\n", material_program->type);
#endif /* defined DEBUG */
#if defined GL_VERSION_2_0
				const char *vv, *ff, *gg;
#endif
#if ! defined (TESTING_PROGRAM_STRINGS)
				char *components_string, *fragment_program_string = NULL,
						*vertex_program_string = NULL, *geometry_program_string = NULL;
				enum Graphics_library_vendor_id vendor_id;
				const char *colour_texture_string[] = {"float", "vec2", "vec3", "vec4"};
				int colour_texture_dimension = 0, components_error, number_of_inputs,
					error;

				vendor_id = Graphics_library_get_vendor_id();
				error = 0;
				fragment_program_string = NULL;

				if (0 == material_program->type)
				{
					vertex_program_string = duplicate_string(material_program->vertex_program_string);
					fragment_program_string = duplicate_string(material_program->fragment_program_string);
					if (material_program->geometry_program_string)
					{
						geometry_program_string = duplicate_string(material_program->geometry_program_string);
					}
					if (vertex_program_string && strstr(vertex_program_string, "!!ARBvp"))
					{
						if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_ARB)
						{
							if (Graphics_library_check_extension(GL_ARB_fragment_program) &&
								Graphics_library_check_extension(GL_ARB_vertex_program))
							{
								material_program->shader_type=MATERIAL_PROGRAM_SHADER_ARB;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Program_string is written with ARB shading program\n"
									"but ARB shading program is not supported.\n");	
								vertex_program_string = NULL;
								fragment_program_string = NULL;
								geometry_program_string = NULL;
							}
						}
					}
					else if (vertex_program_string && strstr(vertex_program_string, "void main("))
					{
						if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
						{
							if (Graphics_library_check_extension(GL_shading_language))
							{
								material_program->shader_type=MATERIAL_PROGRAM_SHADER_GLSL;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Program_string is written with GLSL\n"
									"but GLSL is not supported.\n");
								vertex_program_string = NULL;
								fragment_program_string = NULL;
								geometry_program_string = NULL;
							}
						}
					}
				}

#if defined DEBUG
				if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
				{
					display_message(INFORMATION_MESSAGE,
							"OpenGL 2.0 or higher supported, GLSL supported\n");
				}
				else if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_ARB)
				{
					 display_message(INFORMATION_MESSAGE,
							"ARB shading program supported\n");
				}
#endif  /* defined DEBUG */

				if (MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING & material_program->type)
				{	
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {	 
							vertex_program_string = duplicate_string("//GOURAUDSHADING_VERTEX_SHADER\n"
								 "\n"
								 "varying vec4 NewCoord;\n"
								 "varying vec3 eyeNormal, lightVec, diffuse;\n"
								 );
					 }
					 else
					 {
							vertex_program_string = duplicate_string("!!ARBvp1.0\n"
								 "ATTRIB normal = vertex.normal;\n"
								 "ATTRIB position = vertex.position;\n"
								 "PARAM c0[4] = { state.matrix.mvp };\n"
								 "PARAM c1[4] = { state.matrix.modelview };\n"
								 "PARAM c2[4] = { state.matrix.modelview.invtrans };\n"
								 "PARAM eyeCameraPos = {0, 0, 0, 0};\n"
								 "PARAM eyeLightPos = state.light[0].position;\n"
								 "PARAM two = {2.0, 2.0, 2.0, 2.0};\n"
								 "PARAM m_one = {-1.0, -1.0, -1.0, -1.0};\n"
																											 );
					 }
					 if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE)
							& material_program->type)
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{	
								 append_string(&vertex_program_string, 
										"PARAM texture_scaling = program.env[0];\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string,
										"uniform vec4 texture_scaling;\n"
										, &error);
							}
					 }
					 
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							append_string(&vertex_program_string,
								 "\n"
								 "void main()\n"
								 "{\n"
								 "  vec4 eyeVertex, finalCol;\n"
								 "  float NdotHV;\n"
								 "  float Len, attenuation;\n"
								 "  eyeVertex = gl_ModelViewMatrix * gl_Vertex;\n"
								 "  eyeNormal = normalize(gl_NormalMatrix * gl_Normal);\n"
								 "  lightVec = gl_LightSource[0].position.xyz - eyeVertex.xyz;\n"
								 "  Len = length(lightVec);\n"
								 "  lightVec = normalize(lightVec);\n"
								 "  attenuation = 1.0 / (gl_LightSource[0].quadraticAttenuation * Len * Len + \n"
								 "    gl_LightSource[0].constantAttenuation +\n"
								 "    Len *  gl_LightSource[0].linearAttenuation);\n"
								 "  //NdotL = abs(dot(eyeNormal,lightVec));\n" 
								 "  diffuse = vec3(gl_LightSource[0].diffuse * gl_Color *attenuation);\n"
								 "  NdotHV = abs(dot(eyeNormal, gl_LightSource[0].halfVector.xyz));\n"
								 "  finalCol = gl_FrontLightProduct[0].ambient\n"
								 "    + pow(NdotHV, gl_FrontMaterial.shininess) * gl_LightSource[0].specular\n"
								 "    * gl_FrontMaterial.specular;\n"
								 "  finalCol = finalCol * attenuation + gl_FrontMaterial.emission\n"
								 "     + gl_FrontMaterial.ambient * gl_LightModel.ambient;\n"
								 "  finalCol.a = gl_LightSource[0].diffuse.a * gl_Color.a;\n"
								 , &error);
					 }
					 else
					 {
							append_string(&vertex_program_string, 
								 "TEMP eyeVertex;\n"
								 "TEMP eyeNormal;\n"
								 "TEMP temp_col;\n"
								 "TEMP temp_col2;\n"
								 "TEMP lightVec, Len, finalCol, attenuation, lightContrib;\n"
								 "\n"
								 "#Vertex position in eyespace\n"
								 "DP4 eyeVertex.x, c1[0], position;\n"
								 "DP4 eyeVertex.y, c1[1], position;\n"
								 "DP4 eyeVertex.z, c1[2], position;\n"
								 "\n"
								 "DP4 eyeNormal.x, c2[0], normal;\n"
								 "DP4 eyeNormal.y, c2[1], normal;\n"
								 "DP4 eyeNormal.z, c2[2], normal;\n"
								 "\n"
								 "DP3 eyeNormal.w, eyeNormal, eyeNormal;\n"
								 "RSQ eyeNormal.w, eyeNormal.w;\n"
								 "MUL eyeNormal.xyz, eyeNormal.w, eyeNormal;\n"
								 "\n"
								 "SUB lightVec, eyeLightPos, eyeVertex;\n"
								 "#MOV lightVec, state.light[0].position;\n"
								 "\n"
								 "#Normalize lightvec and viewvec.\n"
								 "DP3		Len.w, lightVec, lightVec;\n"
								 "RSQ		lightVec.w, Len.w;\n"
								 "MUL		lightVec.xyz, lightVec, lightVec.w;\n"
								 "\n"
								 "#Calculate attenuation.\n"
								 "MAD		attenuation, state.light[0].attenuation.z, Len.w, state.light[0].attenuation.x;\n"
								 "RCP		Len, lightVec.w;\n"
								 "MAD		attenuation, Len.w, state.light[0].attenuation.y, attenuation.x;\n"
								 "RCP		attenuation.x, attenuation.x;\n"
								 "\n"
								 "#Diffuse\n"
								 "DP3      lightContrib.x, eyeNormal, lightVec;\n"
								 "ABS      lightContrib.x, lightContrib.x;\n"
								 "\n"
								 "#Specular\n"
								 "#Phong:\n"
								 "#TEMP viewVec, reflVec;\n"
								 "#SUB viewVec, eyeCameraPos, eyeVertex;\n"
								 "#DP3 viewVec.w, viewVec, viewVec;\n"
								 "#RSQ viewVec.w, viewVec.w;\n"
								 "#MUL viewVec.xyz, viewVec.w, viewVec;\n"
								 "#DP3		reflVec, lightVec, eyeNormal;\n"
								 "#MUL		reflVec, reflVec, two;\n"
								 "#MAD		reflVec, reflVec.x, eyeNormal, -lightVec;\n"
								 "#DP3	lightContrib.y, reflVec, viewVec;\n"
								 "\n"
								 "DP3      lightContrib.y, eyeNormal, state.light[0].half;\n"
								 "#new_code: should use an absolute value for the specular dot product too.\n"
								 "ABS      lightContrib.y, lightContrib.y;\n"
								 "\n"
								 "MOV		lightContrib.w, state.material.shininess.x;\n"
								 "\n"
								 "#Accelerates lighting computations\n"
								 "LIT	lightContrib, lightContrib;\n"
								 "\n"
								 "#old_code: the old code used the vertex.colour only and not takinga account of\n"
								 "#old_code: the lighting\n"
								 "#MAD		finalCol, lightContrib.y, vertex.color, state.lightprod[0].ambient;\n"
								 "#MAD		finalCol, lightContrib.z, state.lightprod[0].specular, finalCol;\n"
								 "#new_code\n"
								 "MUL    temp_col,  state.light[0].diffuse, vertex.color;\n"
								 "MAD		finalCol, lightContrib.y, temp_col, state.lightprod[0].ambient;\n"
								 "MOV    temp_col2, state.lightprod[0].specular;\n"
								 "MAD		finalCol.xyz, lightContrib.z, temp_col2, finalCol;\n"
								 "\n"
								 "MAD		finalCol, finalCol, attenuation.x, state.material.emission;\n"
								 "#ADD		finalCol, finalCol, state.lightmodel.scenecolor;\n"
								 "\n"
								 "#old_code:\n"
								 "MAD		finalCol, state.material.ambient, state.lightmodel.ambient, finalCol;\n"
								 "#MAD		finalCol, vertex.color, state.lightmodel.ambient, finalCol;\n"
								 "\n"
								 "#old_code\n"
								 "#MOV finalCol.w, state.material.diffuse.w;\n"
								 "#new_code\n"
								 "MOV finalCol.w, temp_col.w;\n"
								 "\n"
								 , &error);
					 }
					
					 if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE)
							& material_program->type)
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{	
								 append_string(&vertex_program_string, 
										"MUL result.texcoord[0], texture_scaling, vertex.texcoord[0];\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string, 
										"gl_TexCoord[0] = texture_scaling * gl_MultiTexCoord0;\n"
										, &error);
							}
					 }
					
					 if(material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							append_string(&vertex_program_string,
								 "  gl_FrontColor = finalCol;\n"
								 "  gl_BackColor = finalCol;\n"
								 "  NewCoord = ftransform();\n"
								 "  gl_Position = NewCoord;\n"
								 "}\n"
								 , &error);
					 }
					 else
					 {
							append_string(&vertex_program_string, 
								 "DP4 result.texcoord[1].x, c0[0], position;\n"
								 "DP4 result.texcoord[1].y, c0[1], position;\n"
								 "DP4 result.texcoord[1].z, c0[2], position;\n"
								 "DP4 result.texcoord[1].w, c0[3], position;\n"
								 "\n"
								 "MOV result.color, finalCol;\n"
								 "MOV result.color.back, finalCol;\n"
								 "#MOV result.color.secondary,  {1, 1, 1, 1};\n"
								 "#MOV result.color.back.secondary,  {0, 0, 0, 0};\n"
								 "DP4 result.position.x, c0[0], position;\n"
								 "DP4 result.position.y, c0[1], position;\n"
								 "DP4 result.position.z, c0[2], position;\n"
								 "DP4 result.position.w, c0[3], position;\n"
								 "\n"
								"END\n"
								 , &error);
					 }

					 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
					 {
							/* Set the colour texture dimension here so that I can use it when
							 defining uniform variable in GLSL*/
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 & material_program->type)
							{
								 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 & material_program->type)
								 {
										colour_texture_dimension = 3;
								 }
								 else
								 {
										colour_texture_dimension = 1;
								 }								
							}
							else
							{
								 colour_texture_dimension = 2;
							}
					 }
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							fragment_program_string = duplicate_string("//fragment shader\n");
					 }
					 else
					 {
							fragment_program_string = duplicate_string("!!ARBfp1.0\n");
					 }

					 if (MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER & material_program->type)
					 {
							if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&fragment_program_string, 
										"#extension GL_ARB_texture_rectangle : enable\n"
										"uniform vec4 texturesize;\n"
										, &error);
								 if (Graphics_library_vendor_mesa != vendor_id)
								 {
										append_string(&fragment_program_string,
											 "uniform sampler2DRectShadow samplertex;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string,
											 "uniform sampler2DRect samplertex;\n"
											 , &error);
								 }
							}
							else
							{
								 if (Graphics_library_vendor_mesa != vendor_id)
								 {
										append_string(&fragment_program_string, 
											 "OPTION ARB_fragment_program_shadow;\n"
											 , &error);
								 }
								 append_string(&fragment_program_string, 
										"PARAM texturesize = program.env[1];\n"
										"TEMP tex4, kill, tex4coord;\n"
										, &error);
							}
					 }
					 
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
							{
								 char temp_string[100];
								 sprintf(temp_string,
										"uniform sampler%dD texture0;\n", colour_texture_dimension);
								 append_string(&fragment_program_string, 
										temp_string, &error);
							}
							append_string(&fragment_program_string, 
								 "varying vec4 NewCoord;\n"
								 "varying vec3 eyeNormal, lightVec, diffuse;\n"
								 "\n"
								 "void main()\n"
								 "{\n"
								 "  float perspective, texel, NdotL;\n"
								 "  vec4 tex4coord, eyespaceCoord, finalCol;\n"
								 "  perspective = float(1.0) / NewCoord.w;\n"
								 "  eyespaceCoord = NewCoord;\n"
								 "  eyespaceCoord = eyespaceCoord * perspective * 0.5 + 0.5;\n"
								 "  NdotL = dot(normalize(eyeNormal),normalize(lightVec));\n"
								 "  if (!gl_FrontFacing)\n"
								 "    NdotL = abs(NdotL);\n"
								 "  finalCol.xyz = vec3(NdotL * diffuse + gl_Color.xyz);\n"
								 "  finalCol.a = gl_Color.a;\n"
								 , &error);
					 }
					 else
					 {
							append_string(&fragment_program_string, 
								 "TEMP eyespaceCoord, perspective;\n"
								 "PARAM point_five = {0.5, 0.5, 0.5, 0.5};\n"
								 
								 "MOV      eyespaceCoord, fragment.texcoord[1];\n"
								 "RCP      perspective.w, eyespaceCoord.w;\n"
								 "MUL      eyespaceCoord, eyespaceCoord, perspective.w;\n"
								 "MAD      eyespaceCoord, eyespaceCoord, point_five, point_five;\n"
								 "MOV      eyespaceCoord.w, 1.0;\n"
								 , &error);
					 }

					 if (MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER & material_program->type)
					 {
							if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 if (Graphics_library_vendor_mesa == vendor_id)
								 {
										append_string(&fragment_program_string,
											 "  tex4coord = texturesize * eyespaceCoord.xyzx;\n"
											 "  tex4coord.z = gl_FragCoord.z;\n"
											 "  texel = eyespaceCoord.z;\n"
											 "  //texel = texture2DRect(samplertex, vec3(tex4coord)).x\n"
											 "  if (texel >= 0)\n"
											 "  {\n"
											 "    discard;\n"
											 "  }\n"
											 , &error);
								 }
								 else
								 {
										if (Graphics_library_vendor_ati == vendor_id)
										{
											 append_string(&fragment_program_string,
													"  tex4coord = texturesize * eyespaceCoord.xyzx;\n"
													"  tex4coord.z = gl_FragCoord.z -0.0001;\n"
													, &error);
										}
										else
										{
											 append_string(&fragment_program_string,
													"  tex4coord = texturesize * eyespaceCoord.xyzx;\n"
													, &error);
										}
										append_string(&fragment_program_string,
											 "  texel =  shadow2DRect(samplertex, vec3(tex4coord)).r;\n"
													"  if (texel<0.5)\n"
											 "  {\n"
											 "    discard;\n"
											 "  }\n"
											 , &error);
								 }
							}
							else
							{
								 if (Graphics_library_vendor_mesa == vendor_id)
								 {
										append_string(&fragment_program_string,
											 "MOV      tex4coord, eyespaceCoord.xyzx;\n"
											 "MUL      tex4coord, tex4coord, texturesize;\n"
											 "MOV     tex4coord.z, fragment.position.z;\n"
											 "TEX		tex4.x, tex4coord, texture[3], RECT;\n"
											 "MOV     kill, tex4.xxxx;\n"
											 "KIL     -kill;\n"
											 , &error);
								 }
								 else
								 {
										if (Graphics_library_vendor_ati == vendor_id)
										{
											 append_string(&fragment_program_string,
													"MOV      tex4coord, eyespaceCoord.xyzx;\n"
													"MUL      tex4coord, tex4coord, texturesize;\n"
													"\n"
													"MOV     tex4coord.z, fragment.position.z;\n"
													"ADD     tex4coord.z, tex4coord.z, -0.0001;\n" 
													, &error);
										}
										else
										{
											 /* Default is what we used to have which is for Nvidia */
											 append_string(&fragment_program_string,
													"MOV      tex4coord, eyespaceCoord.xyzx;\n"
													"MUL      tex4coord, tex4coord, texturesize;\n"
													"\n"
													"MOV      tex4.x, eyespaceCoord.zzzz;\n"
													, &error);
										}
										append_string(&fragment_program_string,
											 "TEX		tex4.x, tex4coord, texture[3], SHADOWRECT;\n"
											 "ADD      kill.x, tex4.x, -0.5;\n"
											 "KIL      kill.x;\n"
											 , &error);
								 }
							}
					 }
					 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
					 {
							char tex_string[100];
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 /* Load the colour texture */
								 sprintf(tex_string,
										"TEMP		tex;\n"
										"TEX		tex, fragment.texcoord[0], texture[0], %dD;\n",
										colour_texture_dimension);
								 append_string(&fragment_program_string, 
										tex_string, &error);
							}
							else
							{
								 /* Load the colour texture */
								 if (colour_texture_dimension > 1)
								 {
										sprintf(tex_string,
											 "  vec4 tex = texture%dD(texture0, %s(gl_TexCoord[0]));\n",
											 colour_texture_dimension, colour_texture_string[colour_texture_dimension - 1]);
								 }
								 append_string(&fragment_program_string, 
										tex_string, &error);
							}
					 }
					 else
					 {
							colour_texture_dimension = 0;
					 }
					 
					 if (!(MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type))
					 {
							if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
							{
								append_string(&fragment_program_string,
									 "  gl_FragColor = finalCol;\n"
									 , &error);
							}
							else
							{
								 /* Normal lighting, just use the fragment colour */
								 append_string(&fragment_program_string, 
										"MOV      result.color.xyzw, fragment.color.rgba;\n"
										, &error);
							}
					 }
					 else
					 {
							if (!(MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL & material_program->type))
							{
								 /* Modulate */
								 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 & material_program->type)
								 {
										if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
										{
											 /* RGB texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MUL		result.color.xyz, fragment.color, tex;\n"
														 "MOV	  result.color.w, fragment.color.w;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "  gl_FragColor.rgb = vec3(finalCol * tex);\n"
														 "  gl_FragColor.a = finalCol.a;\n"
														 , &error);
											 }
										}
										else
										{
											 /* grayscale texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MUL		result.color.xyz, fragment.color.xyz, tex.x;\n"
														 "MOV	  result.color.w, fragment.color.w;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "  gl_FragColor.rgb = finalCol.rgb * tex.x;\n"
														 "  gl_FragColor.a = finalCol.a;\n"
														 , &error);	
											 }
										}
								 }
								 else
								 {
										if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
										{
											 /* grayscale alpha texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MUL		result.color.xyz, fragment.color.xyz, tex.x;\n"
														 "MUL		result.color.w, fragment.color.w, tex.y;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "  gl_FragColor.rgb = finalCol.xyz * tex.x;\n"
														 "  gl_FragColor.a = finalCol.a * tex.y;\n"
														 , &error);
											 }
										}
										else
										{
											 /* RGBA texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MUL		result.color, fragment.color, tex;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "  gl_FragColor = finalCol * tex;\n"
														 , &error);
											 }
										}
								 }
							}
							else
							{
								 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 & material_program->type)
								 {
										if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
										{
											 /* RGB texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MOV		result.color.xyz, tex;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "	gl_FragColor.rgb = tex.xyz;\n"
														 , &error);
											 }
										}
										else
										{
											 /* grayscale texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MOV		result.color.xyz, tex.x;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "	gl_FragColor.rgb = tex.xxx;\n"
														 , &error);
											 }
										}
										if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
										{
											 append_string(&fragment_program_string, 
													"MOV		result.color.w, state.material.diffuse.w;\n"
													, &error);
										}
										else
										{
											 append_string(&fragment_program_string, 
													"  gl_FragColor.a = gl_FrontMaterial.diffuse.w;\n"
													, &error);
										}
								 }
								 else
								 {
										if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
										{
											 /* grayscale alpha texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MOV		result.color.xyz, tex.x;\n"
														 "MOV		result.color.w, tex.y;\n"
														 , &error);
											 }
											 else
											 {
													append_string(&fragment_program_string,
														 "  gl_FragColor.xyz = tex.x;\n"
														 "  gl_FragColor.w = tex.y;\n"
														 , &error);													
											 }
										}
										else
										{
											 /* RGBA texture */
											 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
											 {
													append_string(&fragment_program_string, 
														 "MOV		result.color, tex;\n"
													, &error);
											 }
											 else
											 {
													append_string(&fragment_program_string, 
														 "  gl_FragColor = tex;\n"
													, &error);
											 }
										}
								 }
							}
					 }
					 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							append_string(&fragment_program_string, 
								 "MOV		 result.depth.z, eyespaceCoord.z;\n"
								 "\n"
								 "END\n"
								 , &error);
					 }
					 else
					 {
							append_string(&fragment_program_string,
								 "  gl_FragDepth = eyespaceCoord.z;\n"
								 "}\n"
								 , &error);
					 }
				}
				else if (MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING & material_program->type)
				{
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							vertex_program_string = duplicate_string("//Vertex Shader\n"
																											 "#version 110\n");				
							append_string(&vertex_program_string,
								 "varying vec4 diffuse, ambientGlobal, ambient;\n"
								 , &error);
					 }
					 else
					 {
							vertex_program_string = duplicate_string("!!ARBvp1.0\n");
							append_string(&vertex_program_string, 
								 "ATTRIB normal = vertex.normal;\n"
								 "ATTRIB position = vertex.position;\n"
								 , &error);
					 }
					 if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE)
							& material_program->type)
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"PARAM texture_scaling = program.env[0];\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string, 
								 "uniform vec4 texture_scaling;\n"
								 , &error);
							} 
					 }
					 if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material_program->type)
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"ATTRIB tangent = vertex.texcoord[1];\n"
										, &error);
							}
					 }
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							append_string(&vertex_program_string, 
								 "\nvoid main()\n"
								 "{\n"
								 "  vec3 pos;\n"
								 , &error);
					 }
					 else
					 {
							append_string(&vertex_program_string, 
								 "PARAM c0[4] = { state.matrix.mvp };\n"
								 "PARAM c1[4] = { state.matrix.modelview };\n"
								 "PARAM eyeCameraPos = {0, 0, 0, 0};\n"
								 "PARAM eyeLightPos = state.light[0].position;\n"
								 
								 "TEMP eyeVertex;\n"
								 "TEMP viewVec;\n"
								 , &error);
					 }
					 if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material_program->type)
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"PARAM c3[4] = { state.matrix.modelview.inverse };\n"
										"TEMP lightVec;\n"
										"TEMP objectLight;\n"
										"TEMP cameraVec;\n"
										"TEMP objectCamera;\n"
										"TEMP binormal;\n"
										, &error);
							}
					 }
					 else
					 {
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)

							{
								 append_string(&vertex_program_string, 
										"PARAM c2[4] = { state.matrix.modelview.invtrans };\n"
										"TEMP eyeNormal;\n"
										, &error);
							}
					 }
					 
					 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					 {
							append_string(&vertex_program_string,
								 "  diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;\n"
								 "  ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;\n"
								 "  ambientGlobal = gl_LightModel.ambient * gl_FrontMaterial.ambient;\n"
								 "  vec4 ecPos = gl_ModelViewMatrix * gl_Vertex;\n"
								 "  vec3 aux = vec3(gl_LightSource[0].position - ecPos);\n"
								 , &error);
					 }
					 else
					 {
							append_string(&vertex_program_string, 
								 "#Vertex position in eyespace\n"
								 "DP4 eyeVertex.x, c1[0], position;\n"
								 "DP4 eyeVertex.y, c1[1], position;\n"
								 "DP4 eyeVertex.z, c1[2], position;\n"
								 , &error);		
					 }						

					if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE)
						& material_program->type)
					{
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"MUL result.texcoord[0], texture_scaling, vertex.texcoord[0];\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string, 
										"  gl_TexCoord[0] = texture_scaling * gl_MultiTexCoord0;\n"
										, &error);								 
							}
					}

					if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material_program->type)
					{
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"MUL binormal.xyz, tangent.zxyz, normal.yzxy;\n"
										"MAD binormal.xyz, tangent.yzxy, normal.zxyz, -binormal.xyzx;\n"
										
										"SUB lightVec, eyeLightPos, eyeVertex;\n"
										
										"DP3 objectLight.x, c3[0], lightVec;\n"
										"DP3 objectLight.y, c3[1], lightVec;\n"
										"DP3 objectLight.z, c3[2], lightVec;\n"
										
										"DP3 result.texcoord[1].x, tangent, objectLight;\n"
										"DP3 result.texcoord[1].y, binormal, objectLight;\n"
										"DP3 result.texcoord[1].z, normal, objectLight;\n"
										
										"SUB cameraVec, eyeCameraPos, eyeVertex;\n"
										"DP3 objectCamera.x, c3[0], cameraVec;\n"
										"DP3 objectCamera.y, c3[1], cameraVec;\n"
										"DP3 objectCamera.z, c3[2], cameraVec;\n"
										
										"DP3 result.texcoord[2].x, tangent, objectCamera;\n"
										"DP3 result.texcoord[2].y, binormal, objectCamera;\n"
										"DP3 result.texcoord[2].z, normal, objectCamera;\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string,
										"\n"
										"  vec3 binormal = vec3(gl_MultiTexCoord1.yzxy * gl_Normal.zxyz) - \n"
									  "         vec3(gl_MultiTexCoord1.zxyz * gl_Normal.yzxy);\n"
										"  vec3 temp = vec3(gl_MultiTexCoord1);\n"
										"  mat3 TBN_Matrix = gl_NormalMatrix * mat3(temp, binormal, gl_Normal);\n"
										"  gl_TexCoord[1].xyz = aux * TBN_Matrix; \n"
										"  gl_TexCoord[2].xyz = vec3(-ecPos) *TBN_Matrix; \n"
										, &error);
							}
					}
					else
					{
							if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&vertex_program_string, 
										"  gl_TexCoord[3].xyz = normalize(gl_NormalMatrix * gl_Normal);\n"
										"  gl_TexCoord[2].xyz = vec3(normalize(-ecPos));\n"
										"  gl_TexCoord[1].xyz = aux;\n"
										, &error);
							}
							else
							{
								 append_string(&vertex_program_string, 
										"DP4 eyeNormal.x, c2[0], normal;\n"
										"DP4 eyeNormal.y, c2[1], normal;\n"
										"DP4 eyeNormal.z, c2[2], normal;\n"
										"DP3 eyeNormal.w, eyeNormal, eyeNormal;\n"
										"RSQ eyeNormal.w, eyeNormal.w;\n"
										"MUL eyeNormal.xyz, eyeNormal.w, eyeNormal;\n"
										
										"SUB viewVec, eyeCameraPos, eyeVertex;\n"
										"DP3 viewVec.w, viewVec, viewVec;\n"
										"RSQ viewVec.w, viewVec.w;\n"
										"MUL viewVec.xyz, viewVec.w, viewVec;\n"
										
										"SUB result.texcoord[1], eyeLightPos, eyeVertex;\n"
										"MOV result.texcoord[2], viewVec;\n"
										"MOV result.texcoord[3], eyeNormal;\n"
										, &error);
							}
					}
							
					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
						 append_string(&vertex_program_string, 
								"  gl_FrontColor = gl_Color;\n"
								"  gl_BackColor = gl_Color;\n"
							  "  gl_FrontSecondaryColor = vec4(1.0);\n"
							  "  gl_BackSecondaryColor = vec4(0.0);\n"
								"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
								"}\n"
								, &error);



					}
					else
					{
						 append_string(&vertex_program_string, 
								"MOV result.color, vertex.color;\n"
								"MOV result.color.back, vertex.color;\n"
								"MOV result.color.secondary,  {1, 1, 1, 1};\n"
								"MOV result.color.back.secondary,  {0, 0, 0, 0};\n"
								"DP4 result.position.x, c0[0], position;\n"
								"DP4 result.position.y, c0[1], position;\n"
								"DP4 result.position.z, c0[2], position;\n"
								"DP4 result.position.w, c0[3], position;\n"
								"END\n"
								, &error);
					}
					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
						 fragment_program_string = duplicate_string("//fragment shader\n"
																												"#version 110\n");
						 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
						 {
								if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 & material_program->type)
								{
									 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 & material_program->type)
									 {
											append_string(&fragment_program_string,
												 "uniform sampler3D texture0;\n"
												 , &error);
									 }
									 else
									 {
											append_string(&fragment_program_string,
												 "uniform sampler1D texture0;\n"
												 , &error);											
									 }
								}
								else
								{
											append_string(&fragment_program_string,
												 "uniform sampler2D texture0;\n"
												 , &error);									 
								}	
						 }

						 if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE & material_program->type)
						 {
								/* Load the second texture using the same texture coordinates as the colour texture */
								if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1 & material_program->type)
								{
									 if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2 & material_program->type)
									 {
											append_string(&fragment_program_string,
												 "uniform sampler3D texture1;\n"
												 , &error);		 
									 }
									 else
									 {
											append_string(&fragment_program_string,
												 "uniform sampler1D texture1;\n"
												 , &error);	
									 }								
								}
								else
								{
											append_string(&fragment_program_string,
												 "uniform sampler2D texture1;\n"
												 , &error);	
								}
						 }

						 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP & material_program->type)
						 {
							 append_string(&fragment_program_string,
								 "uniform sampler1D texture1;\n"
								 , &error);
						 }	 
							 

						 if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 | 
									 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 | 
									 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 | 
									 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4) & material_program->type)
						 {
								if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR | 
											MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA) & material_program->type)
								{									 number_of_inputs = 0;
									 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1
											& material_program->type)
									 {
											number_of_inputs++;
									 }
									 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2
									 & material_program->type)
									 {
											number_of_inputs++;
									 }
									 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3
											& material_program->type)
									 {
											number_of_inputs++;
									 }
									 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4
									 & material_program->type)
									 {
											number_of_inputs++;
									 }
									 if (number_of_inputs > 0 && number_of_inputs < 4)
									 {
											char new_string[1000];
											sprintf(new_string,
												 "uniform sampler%1dD texture1;\n", number_of_inputs);
											append_string(&fragment_program_string,
												 new_string, &error);
									 }
								}
								append_string(&fragment_program_string,
									 "uniform vec4 lookup_offsets, lookup_scales;\n"
									 , &error);
						 }

						 if ((MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE |
									 MATERIAL_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL) &
								material_program->type)
						 {
								append_string(&fragment_program_string,
									 "uniform vec4 texture_scaling, normal_scaling;\n"
									 , &error);
						 }
						 append_string(&fragment_program_string,
								"varying vec4 diffuse, ambientGlobal, ambient;\n"
								"\n"
								"void main()\n"
								"{\n"
								"  vec4 color;\n"
								"  vec3 n, reflV, viewV, ldir;\n"
								"  float NdotL, NdotHV, len;\n"
								"  float att;\n"
								"\n"
								, &error);
					}
					else
					{
						 fragment_program_string = duplicate_string("!!ARBfp1.0\n");
						 append_string(&fragment_program_string, 
								"TEMP lightVec, viewVec, reflVec, normal, attenuation, Len, finalCol, lightContrib, reverse, tex, tex2;\n"
								"PARAM two = {2.0, 2.0, 2.0, 2.0};\n"
								"PARAM m_one = {-1.0, -1.0, -1.0, -1.0};\n"
								, &error);
					}

					if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4) & material_program->type)
					{
						 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
						 {
								append_string(&fragment_program_string, 	
									 "PARAM lookup_offsets = program.env[1];\n"
									 "PARAM lookup_scales = program.env[2];\n"
									 , &error);
						 }
					}

					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
						 append_string(&fragment_program_string,
								"  n = normalize(gl_TexCoord[3].xyz);\n"
								, &error);
					}
					else
					{
						 append_string(&fragment_program_string, 	
								"#Set up reverse vector based on secondary colour\n"
								"MAD      reverse, two, fragment.color.secondary.x, m_one;\n"
								, &error);
					}

					if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
					{
						int colour_texture_string_index = 0;
						char tex_string[100];
						if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
						{
							 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 & material_program->type)
							 {
									if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
									{
										 colour_texture_string_index = 2;
									}
									else
									{
										 colour_texture_string_index = 0;
									}
							 }
							 else
							 {
									if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
									{
										 colour_texture_string_index = 1;
									}
									else
									{
										 colour_texture_string_index = 3;
									}
							 }
						}
						/* Load the colour texture */
						if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 & material_program->type)
							{
								 /* RGB texture */
								 colour_texture_dimension = 3;

							}
							else
							{
								colour_texture_dimension = 1;
							}								

						}
						else
						{
							 colour_texture_dimension = 2;

						}
						if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
						{
							 sprintf(tex_string,
									"  %s tex = %s(texture%dD(texture0, %s(gl_TexCoord[0])));\n", 
									colour_texture_string[colour_texture_string_index],
									colour_texture_string[colour_texture_string_index],
								  colour_texture_dimension,
									colour_texture_string[colour_texture_dimension - 1]);	
						}
						else
						{	
 							 sprintf(tex_string,
									"TEX		tex, fragment.texcoord[0], texture[0], %dD;\n",
									colour_texture_dimension);
						}
						append_string(&fragment_program_string, 
							 tex_string, &error);
					}
					else
					{
						colour_texture_dimension = 0;
					}

					if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE & material_program->type)
					{
						/* Load the second texture using the same texture coordinates as the colour texture */
						if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1 & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2 & material_program->type)
							{
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										append_string(&fragment_program_string, 
											 "TEX		tex2, fragment.texcoord[0], texture[1], 3D;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string, 	
											 "  vec3 tex2 = texture3D(texture1, gl_TexCoord[0].xyz).rgb;\n"
											 , &error);
								 }
							}
							else
							{
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										append_string(&fragment_program_string, 
											 "TEX		tex2, fragment.texcoord[0], texture[1], 1D;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string, 	
											 "  vec3 tex2 = texture1D(texture1, gl_TexCoord[0].x).rgb;\n"
											 , &error);
								 }
							}								
						}
						else
						{
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 
										 "TEX		tex2, fragment.texcoord[0], texture[1], 2D;\n"
										 , &error);
							 }
							 else
							 {
									append_string(&fragment_program_string, 	
										 "  vec3 tex2 = texture2D(texture1, gl_TexCoord[0].xy).rgb;\n"
										 , &error);
							 }				
						}
					}
					if (!(MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL & material_program->type))
					{
						if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material_program->type)
						{
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 
										 "#Expand the range of the normal texture\n"
										 "MAD      normal, two, tex2, m_one;\n"
										 
										 "#Reverse the texture normal direction component if required\n"
										 "MUL      normal.z, reverse.z, normal.z;\n"
										 , &error);
							 }
							 else
							 {
									append_string(&fragment_program_string, 
										 "//Expand the range of the normal texture\n"
										 "  n = 2.0 * tex2 - 1.0;\n"		
										 "  n = normalize(n);\n"
										 "//Reverse the texture normal direction component if required\n"
										 " if (!gl_FrontFacing)\n"
										 "   n.z = -1.0 * n.z;\n"
										 , &error);
							 }
						}
						else
						{
							/* Normal is stored in texcoord[3] */
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 
										 "#Normalize the normal.\n"
										 "DP3		normal.w, fragment.texcoord[3], fragment.texcoord[3];\n"
										 "RSQ		normal.w, normal.w;\n"
										 "MUL		normal.xyz, fragment.texcoord[3], normal.w;\n"
										 
										 "#Reverse the normal if required\n"
										 "MUL      normal, reverse, normal;\n"
										 , &error);
							 }
						}
						/* Usual lighting calculations */
						if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
						{
							 append_string(&fragment_program_string,
									"  color = gl_Color * gl_LightModel.ambient + gl_FrontMaterial.emission;\n"
									"  len = length(vec3(gl_TexCoord[1]));\n"
									"  NdotL = (dot(n, normalize(gl_TexCoord[1].xyz)));\n"
									"  if (!gl_FrontFacing)\n"
									"    NdotL = abs(NdotL);\n"
									"\n"
									"  att = 1.0 / (gl_LightSource[0].constantAttenuation +\n"
									"    gl_LightSource[0].linearAttenuation * len +\n"
									"    gl_LightSource[0].quadraticAttenuation * len * len);\n"
									"  color += att * (diffuse *NdotL + ambient);\n"
									"\n"
									"  reflV = reflect(-normalize(gl_TexCoord[1].xyz), n);\n"
									"  NdotHV = max(dot(reflV, normalize(gl_TexCoord[2].xyz)),0.0);\n"
									"  color += att * gl_FrontMaterial.specular * gl_LightSource[0].specular *\n"
									"    pow(NdotHV, gl_FrontMaterial.shininess);\n"
									"  color.w = gl_FrontMaterial.diffuse.w;\n"
									, &error);
						}
						else
						{
							 append_string(&fragment_program_string, 
									"#Normalize lightvec and viewvec.\n"
									"DP3		Len.w, fragment.texcoord[1], fragment.texcoord[1];\n"
									"RSQ		lightVec.w, Len.w;\n"
									"MUL		lightVec.xyz, fragment.texcoord[1], lightVec.w;\n"
									
									"DP3		viewVec.w, fragment.texcoord[2], fragment.texcoord[2];\n"
									"RSQ		viewVec.w, viewVec.w;\n"
									"MUL		viewVec.xyz, fragment.texcoord[2], viewVec.w;\n"
									
									"#Calculate attenuation.\n"
									"MAD		attenuation, state.light[0].attenuation.z, Len.w, state.light[0].attenuation.x;\n"
									"RCP		Len, lightVec.w;\n"
									"MAD		attenuation, Len.w, state.light[0].attenuation.y, attenuation.x;\n"
									"RCP		attenuation.x, attenuation.x;\n"
									
									"#Diffuse\n"
									"DP3_SAT	   lightContrib.x, normal, lightVec;\n"
									"\n"
									"#Specular\n"
									"# Phong:\n"
									"DP3		reflVec, lightVec, normal;\n"
									"MUL		reflVec, reflVec, two;\n"
									"MAD		reflVec, reflVec, normal, -lightVec;\n"
									"\n"
									"DP3_SAT	lightContrib.y, reflVec, viewVec;\n"
									
									"MOV		lightContrib.w, state.material.shininess.x;\n"
									
									"#Accelerates lighting computations\n"
									"LIT	lightContrib, lightContrib;\n"
									
									"MAD		finalCol, lightContrib.y, fragment.color, state.lightprod[0].ambient;\n"
									"MAD		finalCol, lightContrib.z, state.lightprod[0].specular, finalCol;\n"
									"MAD		finalCol, finalCol, attenuation.x, state.material.emission;\n"
									
									"#Ambient lighting contribution;\n"
									"MAD		finalCol, fragment.color, state.lightmodel.ambient, finalCol;\n"
									
									"#Alpha value;\n"
									"MOV		finalCol.w, state.material.diffuse.w;\n"
									, &error);
						}

						/* The Ambient lighting contribution is using the fragment.color which
							is derived from the diffuse component rather than the ambient one.
							Should probably pass the ambient material colour through as a different
							colour */
						if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 & material_program->type)
							{
								if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
								{
									/* RGB texture */
									 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
									 {
											append_string(&fragment_program_string, 
												 "MUL		finalCol.xyz, finalCol, tex;\n"
												 , &error);
									 }
									 else
									 {
											append_string(&fragment_program_string,
												 "  color.xyz = color.xyz * tex.xyz;\n"
												 , &error);
									 }
								}
								else
								{
									 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
									 {
											/* grayscale texture */
											append_string(&fragment_program_string, 
												 "MUL		finalCol.xyz, finalCol.xyz, tex.x;\n"
												 , &error);
									 }
									 else
									 {
											/* float type tex */
											append_string(&fragment_program_string, 
												 "  color.xyz = color.xyz * tex;\n"
												 , &error);
									 }
								}
							}
							else
							{
								 if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
								 {
										if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
										{
											 /* grayscale alpha texture */
											 append_string(&fragment_program_string, 
													"MUL		finalCol.xyz, finalCol.xyz, tex.x;\n"
													"MUL		finalCol.w, finalCol.w, tex.y;\n"
													, &error);
										}
										else
										{
											append_string(&fragment_program_string,
												 "  color.xyz = color.xyz * tex.x;\n"
												 "  color.w = color.w * tex.y;\n"
												 , &error);
										}
								 }
								 else
								 {
										if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
										{
											 /* RGBA texture */
											 append_string(&fragment_program_string, 
													"MUL		finalCol, finalCol, tex;\n"
													, &error);
										}
										else
										{
											append_string(&fragment_program_string,
												 "  color = color * tex;\n"
												 , &error);
										}
								 }
							}
						}
					}
					else
					{
						/* No lighting calculations are required for a decal texture */
						if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
							{
								/* RGB texture */
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										append_string(&fragment_program_string, 
											 "MOV		finalCol.xyz, tex;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string, 
											 "  color.xyz = tex.xyz;\n"
											 , &error);										
								 }
							}
							else
							{
								/* grayscale texture */
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										append_string(&fragment_program_string, 
											 "MOV		finalCol.xyz, tex.x;\n"
											 , &error);
								 }
								 else
								 {
										/* float type tex */
										append_string(&fragment_program_string, 
											 "  color.xyz = vec3(tex);\n"
											 , &error);										
								 }
							}
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&fragment_program_string, 
										"MOV		finalCol.w, state.material.diffuse.w;\n"
										, &error);
							}
							else
							{
								 append_string(&fragment_program_string, 
										"  color.w = gl_FrontMaterial.diffuse.w;\n"
										, &error);
							}
						}
						else
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
							{
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										/* grayscale alpha texture */
										append_string(&fragment_program_string, 
											 "MOV		finalCol.xyz, tex.x;\n"
											 "MOV		finalCol.w, tex.y;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string, 
											 "  color.xyz = tex.xxx;\n"
											 "  color.w = tex.y;\n"
											 , &error);																				
								 }
							}
							else
							{
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
										/* RGBA texture */
										append_string(&fragment_program_string, 
											 "MOV		finalCol, tex;\n"
											 , &error);
								 }
								 else
								 {
										append_string(&fragment_program_string, 
											 "  color = tex;\n"
											 , &error);
								 }
							}
						}
					}
					if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4) & material_program->type)
					{
						if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR | 

							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA) & material_program->type)
						{
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 	
										 "TEMP dependentlookup;\n"
										 "TEMP offsetcolour;\n"
										 , &error);
							 }
							 components_string = (char *)NULL;
							 components_error = 0;
							 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1
									& material_program->type)
							 {
									append_string(&components_string, "r", &components_error);
							 }
							 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2
									& material_program->type)
							 {
									append_string(&components_string, "g", &components_error);
							 }
							 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3
									& material_program->type)
							 {
									append_string(&components_string, "b", &components_error);
							 }
							 if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4
									& material_program->type)
							 {
									append_string(&components_string, "a", &components_error);
							 }
							 number_of_inputs = strlen(components_string);
							 while (!components_error && (strlen(components_string) < 4))
							 {
								append_string(&components_string, "r", &components_error);
							 }
							 if (!components_error)
							 {
									char tex_string[1000];
									if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
									{
										 sprintf(tex_string,
												"#Offset and scale to counteract effect of linear interpolation\n"
												"#starting at the middle of the first texel and finishing in the\n"
												"#middle of the last texel\n"
												"MAD		offsetcolour, finalCol.%s, lookup_scales, lookup_offsets;\n"
												"TEX		dependentlookup, offsetcolour, texture[1], %1dD;\n",
												components_string, number_of_inputs);
									}
									else
									{
										 if (number_of_inputs == 1)
										 {
												sprintf(tex_string,
													 "  //Offset and scale to counteract effect of linear interpolation\n"
													 "  //starting at the middle of the first texel and finishing in the\n"
													 "  //middle of the last texel\n"
													 "  vec4  offsetcolour = color.%s * lookup_scales + lookup_offsets;\n"
													 "  vec4  dependentlookup = texture1D(texture1, float(offsetcolour));\n",
													 components_string);
										 }
										 else
										 {
												sprintf(tex_string,
													 "  //Offset and scale to counteract effect of linear interpolation\n"
													 "  //starting at the middle of the first texel and finishing in the\n"
													 "  //middle of the last texel\n"
													 "  vec4  offsetcolour = color.%s * lookup_scales + lookup_offsets;\n"
													 "  vec4  dependentlookup = texture%1dD(texture1, vec%1d(offsetcolour));\n",
													 components_string, number_of_inputs, number_of_inputs);
										 }
									}
									append_string(&fragment_program_string,
										 tex_string, &error);
							}
							if (components_string)
							{
								DEALLOCATE(components_string);

							}
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 switch ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR |
											 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA) 
										& material_program->type)
								 {
										case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR:
										{
											 /* Don't touch alpha */
											 append_string(&fragment_program_string,
													"MOV		finalCol.rgb, dependentlookup;\n"
													, &error);
										} break;
										case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA:
										{
											 append_string(&fragment_program_string,
													"MUL		finalCol.w, finalCol.w, dependentlookup.r;\n"
													, &error);
										} break;
										case (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR |
											 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA):
											 {
													append_string(&fragment_program_string,
														 "MOV		finalCol, dependentlookup;\n"
														 , &error);
											 } break;
								 }
							}
							else
							{
								 switch ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR |
											 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA) 
										& material_program->type)
								 {
										case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR:
										{
											 /* Don't touch alpha */
											 append_string(&fragment_program_string,
													" 	color.rgb = dependentlookup.rgb;\n"
													, &error);
										} break;
										case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA:
										{
											 append_string(&fragment_program_string,
													"  color.w = color.w * dependentlookup.r;\n"
													, &error);
										} break;
										case (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR |
											 MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA):
											 {
													append_string(&fragment_program_string,
														 "  color = dependentlookup;\n"
														 , &error);
											 } break;
											 
								 }
							}
						}
						else if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP & material_program->type)
						{
							 char tex_string[1000];
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									char lookup_one_component_string[] = 
										 "#Offset and scale to counteract effect of linear interpolation\n"
										 "#starting at the middle of the first texel and finishing in the\n"
										 "#middle of the last texel\n"
										 "MAD		offsetcolour, finalCol.%s, lookup_scales, lookup_offsets;\n"
										 "TEX		dependentlookup, offsetcolour, texture[1], 1D;\n"
										 "MOV		finalCol.%s, dependentlookup.r;\n";
									append_string(&fragment_program_string, 	
										 "TEMP dependentlookup;\n"
										 "TEMP offsetcolour;\n"
										 , &error);
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"rrrr", "r");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"gggg", "g");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"bbbb", "b");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"aaaa", "a");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
							 }
							 else
							 {
								  append_string(&fragment_program_string,
									   "  //Offset and scale to counteract effect of linear interpolation\n"
									   "  //starting at the middle of the first texel and finishing in the\n"
									   "  //middle of the last texel\n"
									   "  float  offsetcolour;\n"
										 "  vec4  dependentlookup;\n", &error);
									char lookup_one_component_string[] = 
										 "  offsetcolour = color.%s * lookup_scales.x + lookup_offsets.x;\n"
										 "  dependentlookup = texture1D(texture1, offsetcolour);\n"
										 "  color.%s = dependentlookup.r;\n";
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"r", "r");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"g", "g");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"b", "b");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
									if (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4
										 & material_program->type)
									{
										 sprintf(tex_string, lookup_one_component_string,
												"a", "a");
										 append_string(&fragment_program_string,
												tex_string, &error);
									}
							 }
						}
					}

					if ((MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE |
						MATERIAL_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL) &
						material_program->type)
					{
						/* I think with some rearrangement we could consolidate
							this with the per pixel lighting above assuming that we
							don't want to light using the fragment normals and
							then do this lighting too. */
						 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
						 {
								append_string(&fragment_program_string, 
									 "TEMP unlitColour;\n"
									 "MOV     unlitColour, finalCol;\n"
									 , &error);
						 }
						 else
						 {
								append_string(&fragment_program_string, 
									 "  vec4 unlitColour = color;\n"
									 , &error);		
						 }

						if (MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE &
							material_program->type)
						{
							 /* Normal comes from the texture */

							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {									
									append_string(&fragment_program_string, 
										 "#Expand the range of the normal texture\n"
										 /* We are assuming the normal is in .gba */
										 "MAD     normal, two, tex.gbaa, m_one;\n"
										 , &error);
							 }
							 else
							 {
									append_string(&fragment_program_string, 
										 "  n = 2 * tex.gbaa - 1.0;\n"
										 , &error);
							 }
						}
						else
						{
							char tex_string[1000];
							const char *component_labels[] = {"x", "y", "z"};
							int i;

							/* Normal is calculated from the red intensity,
								may want colour magnitude or alpha value. */
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&fragment_program_string,
										"#Calculate a finite difference normal based on the magnitude of texture components used.\n"
										"PARAM texture_scaling = program.env[0];\n"
										"PARAM normal_scaling = program.env[3];\n"
										"TEMP position_up, position_down, tex_up, tex_down;\n"
										"\n"
										, &error);
							}
							else
							{
								 append_string(&fragment_program_string,
										"//Calculate a finite difference normal based on the magnitude of texture components used.\n"
										"  vec4 position_up, position_down, tex_up, tex_down;\n"
										, &error);
							}
					
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 & material_program->type)
							{
								if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 & material_program->type)
								{
									colour_texture_dimension = 3;
								}
								else
								{
									colour_texture_dimension = 1;
								}								

							}
							else
							{
								colour_texture_dimension = 2;
							}

							for (i = 0 ; i < colour_texture_dimension ; i++)
							{
								 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
								 {
									 sprintf(tex_string,
											 "PARAM stencil_%sup = {%d, %d, %d, %d};\n"
											 "MAD      position_up, stencil_%sup, texture_scaling, fragment.texcoord[0];\n"
											 "TEX		tex_up, position_up, texture[0], %dD;\n"
											 "PARAM stencil_%sdown = {%d, %d, %d, %d};\n"
											 "MAD      position_down, stencil_%sdown, texture_scaling, fragment.texcoord[0];\n"
											 "TEX		tex_down, position_down, texture[0], %dD;\n",
											 component_labels[i],
											 (i==0),(i==1),(i==2),0,
											 component_labels[i], 
											 colour_texture_dimension,
											 component_labels[i],
											 -(i==0),-(i==1),-(i==2),0,
											 component_labels[i], 
											 colour_texture_dimension);
										append_string(&fragment_program_string,
											 tex_string, &error);
										
										switch ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1
													| MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2) & material_program->type)
										{
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1
													| MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2:
											 case 0:
											 {
													/* RGB or RGBA texture
														 Take the magnitude of the differences */
													sprintf(tex_string,
														 "SUB  tex_up, tex_up, tex_down;\n"
														 "DP3	tex_up.w, tex_up, tex_up;\n"
														 "RSQ  tex_up.w, tex_up.w;\n"
														 "RCP  normal.%s, tex_up.w;\n"
														 , component_labels[i]);
													append_string(&fragment_program_string,
														 tex_string, &error);
											 } break;
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1:
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2:
											 {
													/* Intensity or IntensityAlpha texture */
													sprintf(tex_string,
														 "SUB  normal.%s, tex_up.r, tex_down.r;\n"
														 , component_labels[i]);
													append_string(&fragment_program_string,
														 tex_string, &error);
											 } break;
										}
								 }
								 else
								 {
										sprintf(tex_string,
											 "  vec4 stencil_%sup = vec4(%d, %d, %d, %d);\n"
											 "  position_up = stencil_%sup * texture_scaling + gl_TexCoord[0];\n"
											 "  tex_up = texture%dD(texture0, vec%d(position_up));\n"
											 "  vec4 stencil_%sdown = vec4(%d, %d, %d, %d);\n"
											 "  position_down = stencil_%sdown * texture_scaling + gl_TexCoord[0];\n"
											 "  tex_down =  texture%dD(texture0, vec%d(position_down));\n",
											 component_labels[i],
											 (i==0),(i==1),(i==2),0,
											 component_labels[i], 
											 colour_texture_dimension,
											 colour_texture_dimension,
											 component_labels[i],
											 -(i==0),-(i==1),-(i==2),0,
											 component_labels[i], 
											 colour_texture_dimension,
											 colour_texture_dimension);

										append_string(&fragment_program_string,
											 tex_string, &error);
										
										switch ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1
													| MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2) & material_program->type)
										{
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1
													| MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2:
											 case 0:
											 {
													/* RGB or RGBA texture
														 Take the magnitude of the differences */
													sprintf(tex_string,
														 "  n.%s = sqrt(dot(vec3(tex_up - tex_down), vec3(tex_up - tex_down)));\n"
														 , component_labels[i]);
													append_string(&fragment_program_string,
														 tex_string, &error);
											 } break;
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1:
											 case MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2:
											 {
													/* Intensity or IntensityAlpha texture */
													sprintf(tex_string,
														 "  n.%s = tex_up.r - tex_down.r;\n"
														 , component_labels[i]);
													append_string(&fragment_program_string,
														 tex_string, &error);
											 } break;
										}
								 }
							}
							if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							{
								 append_string(&fragment_program_string,
										"MUL  normal, normal, normal_scaling;\n"
										, &error);
							}
							else
							{
								 append_string(&fragment_program_string,
										"  n = n * vec3(normal_scaling);\n"
										, &error);
							}
						}

						if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
						{
							 append_string(&fragment_program_string,
									/* Normalise the normal but keep the squared
										 magnitude so we can use it to scale the alpha */
									"TEMP  eyeNormal, normalMag;\n"
									"PARAM c2[4] = { state.matrix.modelview.invtrans };\n"
									"\n"
									"#Put the normal into eye point space\n"
									"DP4 eyeNormal.x, c2[0], normal;\n"
									"DP4 eyeNormal.y, c2[1], normal;\n"
									"DP4 eyeNormal.z, c2[2], normal;\n"
									"\n"
									"#Reverse the texture normal direction component if required\n"
									"MUL      eyeNormal, reverse, eyeNormal;\n"
									"\n"
									"#Normalize the normal.\n"
									"DP3		normalMag.w, eyeNormal, eyeNormal;\n"
									"RSQ		eyeNormal.w, normalMag.w;\n"
									"MUL		eyeNormal.xyz, eyeNormal, eyeNormal.w;\n"
									"\n"
									"#Normalize lightvec and viewvec.\n"
									"DP3		Len.w, fragment.texcoord[1], fragment.texcoord[1];\n"
									"RSQ		lightVec.w, Len.w;\n"
									"MUL		lightVec.xyz, fragment.texcoord[1], lightVec.w;\n"
									"DP3		viewVec.w, fragment.texcoord[2], fragment.texcoord[2];\n"
									"RSQ		viewVec.w, viewVec.w;\n"
									"MUL		viewVec.xyz, fragment.texcoord[2], viewVec.w;\n"
									"#Calculate attenuation.\n"
									"MAD		attenuation, state.light[0].attenuation.z, Len.w, state.light[0].attenuation.x;\n"
									"RCP		Len, lightVec.w;\n"
									"MAD		attenuation, Len.w, state.light[0].attenuation.y, attenuation.x;\n"
									"RCP		attenuation.x, attenuation.x;\n"
									"#Diffuse\n"
									"DP3_SAT	   lightContrib.x, eyeNormal, lightVec;\n"
									""
									"#Specular\n"
									"# Phong:\n"
									"DP3		reflVec, lightVec, eyeNormal;\n"
									"MUL		reflVec, reflVec, two;\n"
									"MAD		reflVec, reflVec, -eyeNormal, lightVec;\n"
									"\n"
									"DP3_SAT	lightContrib.y, reflVec, viewVec;\n"
									"MOV		lightContrib.w, state.material.shininess.x;\n"
									"#Accelerates lighting computations\n"
									"LIT	   lightContrib, lightContrib;\n"
									"MAD		finalCol, lightContrib.y, unlitColour, state.lightprod[0].ambient;\n"
									"MAD		finalCol, lightContrib.z, state.lightprod[0].specular, finalCol;\n"
									"MAD		finalCol, finalCol, attenuation.x, state.material.emission;\n"
									"#Ambient lighting contribution;\n"
									"MAD		finalCol, unlitColour, state.lightmodel.ambient, finalCol;\n"
									, &error);
						}
						else
						{
							 append_string(&fragment_program_string,
									/* Normalise the normal but keep the squared
										 magnitude so we can use it to scale the alpha */
									"  vec3 eyeNormal = gl_NormalMatrix * n;\n"
									"  if (!gl_FrontFacing)\n"
									"    eyeNormal.z = -1.0 * eyeNormal.z;\n"
									"  float normalMag = dot (eyeNormal, eyeNormal);\n"
									"  eyeNormal = normalize(eyeNormal);\n"
									"  len = length(vec3(gl_TexCoord[1]));\n"
									"  att = 1.0 / (gl_LightSource[0].constantAttenuation +\n"
									"    gl_LightSource[0].linearAttenuation * len +\n"
									"    gl_LightSource[0].quadraticAttenuation * len * len);\n"
									"  //Calculate attenuation.\n"
									"  NdotL = (dot(eyeNormal, normalize(gl_TexCoord[1].xyz)));\n"
									"  if (!gl_FrontFacing)\n"
									"    NdotL = abs(NdotL);\n"
									"  color += att * (diffuse *NdotL + ambient);\n"
									"  reflV = reflect(-normalize(gl_TexCoord[1].xyz), eyeNormal);\n"
									"  NdotHV = max(dot(reflV, normalize(gl_TexCoord[2].xyz)),0.0);\n"
									"  color += att * gl_FrontMaterial.specular * gl_LightSource[0].specular *\n"
									"    pow(NdotHV, gl_FrontMaterial.shininess);\n"
									, &error);
						}
						
						if (MATERIAL_PROGRAM_CLASS_LIT_VOLUME_SCALE_ALPHA &
							material_program->type)
						{
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 
										 "#Alpha value;\n"
										 "MUL		finalCol.w, unlitColour.w, normalMag.w;\n"
										 , &error);
							 }
							 else
							 {
									append_string(&fragment_program_string, 
										 "  //Alpha value;\n"
										 "  color.w = unlitColour.w * normalMag;\n"
										 , &error);
							 }
						}
						else
						{
							 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
							 {
									append_string(&fragment_program_string, 
										 "#Alpha value;\n"
										 "MOV		finalCol.w, unlitColour.w;\n"
										 , &error);
							 }
							 else
							 {
									append_string(&fragment_program_string, 
										 "//Alpha value;\n"
										 "	color.w = unlitColour.w;\n"
										 , &error);	
							 }
						}
					}
					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
						 append_string(&fragment_program_string,
								"  gl_FragColor = color;\n"
								"}"
								, &error);

					}
					else


					{
						 append_string(&fragment_program_string, 
								"MOV		result.color, finalCol;\n"
								"END"
								, &error);
					}
				}
				if (vertex_program_string && fragment_program_string)
				{
					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
#if defined (WRITE_STRING)
						FILE *program_file;
						if (vertex_program_string && (program_file = fopen("out.vert", "w")))
						{
							fprintf(program_file, "%s", vertex_program_string);
							fclose (program_file);
						}
						if (fragment_program_string && (program_file = fopen("out.frag", "w")))
						{
							fprintf(program_file, "%s", fragment_program_string);
							fclose (program_file);
						}
						if (geometry_program_string && (program_file = fopen("out.geo", "w")))
						{
							fprintf(program_file, "%s", geometry_program_string);
							fclose (program_file);
						}
#endif /* defined (WRITE_STRING) */
#if defined (GL_VERSION_2_0)
						material_program->vertex_program = glCreateShader(GL_VERTEX_SHADER);
						material_program->fragment_program = glCreateShader(GL_FRAGMENT_SHADER);
						if (geometry_program_string && Graphics_library_load_extension("GL_EXT_geometry_shader4"))
						{
							material_program->geometry_program = glCreateShader(GL_GEOMETRY_SHADER_EXT);
						}
#endif /* defined (GL_VERSION_2_0) */
					}
					else
					{
						if (!material_program->vertex_program)
						{
							glGenProgramsARB(1, &material_program->vertex_program);
						}
						glBindProgramARB(GL_VERTEX_PROGRAM_ARB, material_program->vertex_program);
						glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(vertex_program_string), vertex_program_string);
#if defined (DEBUG)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  Vertex Result: %s\n", error_msg);
#endif /* defined (DEBUG) */
#if defined (WRITE_STRING)
						FILE *program_file;
						if (vertex_program_string && (program_file = fopen("out.vp", "w")))
						{
							fprintf(program_file, "%s", vertex_program_string);
							fclose (program_file);
						}
#endif /* defined (WRITE_STRING) */
						DEALLOCATE(vertex_program_string);
						if (!material_program->fragment_program)
						{
							glGenProgramsARB(1, &material_program->fragment_program);
						}
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, material_program->fragment_program);
						glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(fragment_program_string), fragment_program_string);
#if defined (DEBUG)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  Fragment Result: %s\n", error_msg);
#endif /* defined (DEBUG) */
#if defined (WRITE_STRING)
						if (fragment_program_string && (program_file = fopen("out.fp", "w")))
						{
							fprintf(program_file, "%s", fragment_program_string);
							fclose (program_file);
						}				
#endif /* defined (WRITE_STRING) */
						DEALLOCATE(fragment_program_string);
					}
#else /* ! defined (TESTING_PROGRAM_STRINGS) */
#define MAX_PROGRAM (20000)
					char vertex_program_string[MAX_PROGRAM], fragment_program_string[MAX_PROGRAM];
					{
						FILE *program_file;
						int count;
						
						if (program_file = fopen("test.vp", "r"))
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
						
						if (program_file = fopen("test.fp", "r"))
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
					if (material_program->shader_type == MATERIAL_PROGRAM_SHADER_GLSL)
					{
#if defined (GL_VERSION_2_0)
						material_program->vertex_program = glCreateShader(GL_VERTEX_SHADER);
						material_program->fragment_program = glCreateShader(GL_FRAGMENT_SHADER);
						if (geometry_program_string && Graphics_library_load_extension("GL_EXT_geometry_shader4"))
						{
							material_program->geometry_program = glCreateShader(GL_GEOMETRY_SHADER_EXT);
						}
#endif /* defined (GL_VERSION_2_0) */		 
					}
					else
					{
						if (!material_program->vertex_program)
						{
							glGenProgramsARB(1, &material_program->vertex_program);
						}
						
						glBindProgramARB(GL_VERTEX_PROGRAM_ARB, material_program->vertex_program);
						glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(vertex_program_string), vertex_program_string);
#if defined (DEBUG)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  test.vp Vertex Result: %s", error_msg);
#endif /* defined (DEBUG) */
						
						if (!material_program->fragment_program)
						{
							glGenProgramsARB(1, &material_program->fragment_program);
						}
						
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, material_program->fragment_program);
						glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(fragment_program_string), fragment_program_string);
#if defined (DEBUG)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  test.fp Fragment Result: %s", error_msg);
#endif /* defined (DEBUG) */
					}
					material_program->compiled = 1;
#endif /* ! defined (TESTING_PROGRAM_STRINGS) */			
					if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
					{
						GLint vertexShaderCompiled, fragmentShaderCompiled, geometryShaderCompiled;
						
						material_program->glsl_current_program = glCreateProgram();
						
						vv = vertex_program_string;
						glShaderSource(material_program->vertex_program,1, &vv, NULL);
						glCompileShader(material_program->vertex_program);
						glGetShaderiv(material_program->vertex_program, GL_COMPILE_STATUS, &vertexShaderCompiled);
						glAttachShader(material_program->glsl_current_program,material_program->vertex_program);
						DEALLOCATE(vertex_program_string);
						if (material_program->geometry_program)
						{
							gg = geometry_program_string;
							glShaderSource(material_program->geometry_program,1, &gg, NULL);
							glCompileShader(material_program->geometry_program);
							glProgramParameteriEXT(material_program->glsl_current_program, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
							glProgramParameteriEXT(material_program->glsl_current_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
							glGetShaderiv(material_program->geometry_program, GL_COMPILE_STATUS, &geometryShaderCompiled);
							glAttachShader(material_program->glsl_current_program,material_program->geometry_program);
							int geom_ouput_max_vertices = 0;
							glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &geom_ouput_max_vertices);
							glProgramParameteriEXT(material_program->glsl_current_program, GL_GEOMETRY_VERTICES_OUT_EXT, geom_ouput_max_vertices);
							DEALLOCATE(geometry_program_string);
						}
						else
						{
							geometryShaderCompiled = 1;
						}

						ff = fragment_program_string;
						glShaderSource(material_program->fragment_program,1, &ff, NULL);
						glCompileShader(material_program->fragment_program);
						glGetShaderiv(material_program->fragment_program, GL_COMPILE_STATUS, &fragmentShaderCompiled);
						glAttachShader(material_program->glsl_current_program,material_program->fragment_program);
						DEALLOCATE(fragment_program_string);

#if !defined (DEBUG)
						// If DEBUG is defined always write the program info, otherwise
						// write the program info only when one of the shaders fails to compile.
						if (!vertexShaderCompiled || !fragmentShaderCompiled || !geometryShaderCompiled)
#endif // !defined (DEBUG)
						{
							int infologLength = 0;
							int charsWritten  = 0;
							char *infoLog;
							glGetShaderiv(material_program->vertex_program, GL_INFO_LOG_LENGTH,&infologLength);
							if (infologLength > 0)
							{
								infoLog = (char *)malloc(infologLength);
								glGetShaderInfoLog(material_program->vertex_program,
									infologLength, &charsWritten, infoLog);
								display_message(INFORMATION_MESSAGE,"Vertex program info:\n%s\n",infoLog);
								free(infoLog);
							}
							glGetShaderiv(material_program->geometry_program, GL_INFO_LOG_LENGTH,&infologLength);
							if (infologLength > 0)
							{
								infoLog = (char *)malloc(infologLength);
								glGetShaderInfoLog(material_program->geometry_program,
									infologLength, &charsWritten, infoLog);
								display_message(INFORMATION_MESSAGE,"Geometry program info:\n%s\n",infoLog);
								free(infoLog);
							}
							glGetShaderiv(material_program->fragment_program, GL_INFO_LOG_LENGTH,&infologLength);
							if (infologLength > 0)
							{
								infoLog = (char *)malloc(infologLength);
								glGetShaderInfoLog(material_program->fragment_program, 
									infologLength, &charsWritten, infoLog);
								display_message(INFORMATION_MESSAGE,"Fragment program info:\n%s\n",infoLog);
								free(infoLog);
							}
						}
						
						if (!material_program->display_list)
						{
							material_program->display_list = glGenLists(1);
						}
						glNewList(material_program->display_list, GL_COMPILE);
						glLinkProgram(material_program->glsl_current_program);
						glUseProgram(material_program->glsl_current_program);
						glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
						glEndList();
					}
					else
					{
						if (!material_program->display_list)
						{
							material_program->display_list = glGenLists(/*number_of_lists*/1);
						}
						
						glNewList(material_program->display_list, GL_COMPILE);
						
						glEnable(GL_VERTEX_PROGRAM_ARB);
						glBindProgramARB(GL_VERTEX_PROGRAM_ARB,
							material_program->vertex_program);
						
						glEnable(GL_FRAGMENT_PROGRAM_ARB);
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
							material_program->fragment_program);
						
						glEnable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
						
						glEndList();
					}
				}
				material_program->compiled = 1;
			 }
			 else
			 {
				 display_message(ERROR_MESSAGE, "Support for per pixel lighting and "
					 "bump mapping requires the "
					 "GL_ARB_vertex_program and GL_ARB_fragment_program extensions "
					 "which are not available in this OpenGL implementation.");
			 }
#else /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
			 display_message(ERROR_MESSAGE, "Support for per pixel lighting and "
				 "bump mapping requires the "
				 "GL_ARB_vertex_program and GL_ARB_fragment_program extensions or GL_VERSION_2_0 "
				 "which were not compiled into this version.");
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
		}
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"Material_program_compile.  Not defined for this graphics API.");
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_compile.  Missing material_program");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Material_program_compile */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Material_program_execute(struct Material_program *material_program)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Material_program_execute);
	if (material_program)
	{
		if (material_program->compiled)
		{
			if (material_program->display_list)
			{
				glCallList(material_program->display_list);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Material_program_execute.  Display list not current");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_execute.  Missing material_program object.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Material_program_execute */
#endif /* defined (OPENGL_API) */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphical_material,name,const char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphical_material)

#if defined (OPENGL_API)
static int direct_render_Graphical_material(struct Graphical_material *material,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Directly outputs the graphics library commands for activating <material>.
???RC Only supports OpenGL.
SAB Spectrums make extensive use of this function as they operate by copying the
normal material, modifying it according to the settings and then calling this
routine.  I expected this to make spectrums much slower as we are no longer
using the glColorMaterial function, didn't really seem to change.  However on
some other OPENGL implementations this could have a significant effect.  I was
going to try and get this routine to keep record of the current colour and use
the glColorMaterial function if it can however if this is done you must make
sure that when this routine is compiled into different display list the correct
material results.
==============================================================================*/
{
	GLfloat values[4];
#if defined (GL_VERSION_2_0)
	GLint loc1 = -1;
#endif /* defined (GL_VERSION_2_0) */
	int return_code;

	ENTER(direct_render_Graphical_material);
	if (material)
	{
		values[0]=(material->diffuse).red;
		values[1]=(material->diffuse).green;
		values[2]=(material->diffuse).blue;
		values[3]=material->alpha;
		/* use diffuse colour for lines, which are unlit */
		glColor4fv(values);
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,values);
		values[0]=(material->ambient).red;
		values[1]=(material->ambient).green;
		values[2]=(material->ambient).blue;
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,values);
		values[0]=(material->emission).red;
		values[1]=(material->emission).green;
		values[2]=(material->emission).blue;
		values[3]=1.0;
		glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,values);
		values[0]=(material->specular).red;
		values[1]=(material->specular).green;
		values[2]=(material->specular).blue;
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,values);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,(material->shininess)*128.);

		if (material->image_texture.texture)
		{
			renderer->Texture_execute(material->image_texture.texture);
		}

#if defined (GL_VERSION_1_3)
		if (Graphics_library_check_extension(GL_VERSION_1_3))
		{
			/* I used to test for the GL_VERSION_1_3 when setting the texture
				and not here, but at that point the openGL may not have been
				initialised yet, instead check here at display list compile time. */
			if (material->second_image_texture.texture)
			{
				glActiveTexture(GL_TEXTURE1);
				renderer->Texture_execute(material->second_image_texture.texture);
				glActiveTexture(GL_TEXTURE0);
			}
			else if (material->spectrum)
			{
				glActiveTexture(GL_TEXTURE1);
				Spectrum_execute_colour_lookup(material->spectrum, renderer);
				glActiveTexture(GL_TEXTURE0);
			}
			else
			{
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glActiveTexture(GL_TEXTURE0);
			}
			/* The colour_lookup_spectrum is specified as the third texture
				so far, so can't have both a spectrum and an explicit third texture
				at the moment. */
			if (material->third_image_texture.texture)
			{
				glActiveTexture(GL_TEXTURE2);
				renderer->Texture_execute(material->third_image_texture.texture);
				glActiveTexture(GL_TEXTURE0);
			}
			else
			{
				glActiveTexture(GL_TEXTURE2);
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glActiveTexture(GL_TEXTURE0);
			}
			if (material->fourth_image_texture.texture)
			{
				glActiveTexture(GL_TEXTURE3);
				renderer->Texture_execute(material->fourth_image_texture.texture);
				glActiveTexture(GL_TEXTURE0);
			}
			else
			{
				glActiveTexture(GL_TEXTURE3);
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glActiveTexture(GL_TEXTURE0);
			}
		}
#endif /* defined (GL_VERSION_1_3) */

		if (material->program)
		{
			 Material_program_execute(material->program);
#if defined (GL_VERSION_2_0)
			 if (material->program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
			 {
					if (material->program->glsl_current_program)
					{
						 if (material->image_texture.texture)
						 {
								Texture_execute_vertex_program_environment(material->image_texture.texture,
									 material->program->glsl_current_program);
						 }
						 if (material->second_image_texture.texture)
						 {
								Texture_execute_vertex_program_environment(material->second_image_texture.texture,
									 material->program->glsl_current_program);
								
						 }
						 if (material->third_image_texture.texture)
						 {
								Texture_execute_vertex_program_environment(material->third_image_texture.texture,
									 material->program->glsl_current_program);
						 }
						 if (glIsProgram(material->program->glsl_current_program))
						 {
								loc1 = glGetUniformLocation(material->program->glsl_current_program,"texture2");
								if (loc1 != (GLint)-1)
									 glUniform1i(loc1,2);
								loc1 = glGetUniformLocation(material->program->glsl_current_program,"texture1");
								if (loc1 != (GLint)-1)
									 glUniform1i(loc1,1);
								loc1 = glGetUniformLocation(material->program->glsl_current_program,"texture0");
								if (loc1 != (GLint)-1)
									 glUniform1i(loc1, 0);
								if (material->program_uniforms)
									FOR_EACH_OBJECT_IN_LIST(Material_program_uniform)(
											Material_program_uniform_write_glsl_values, material,
											material->program_uniforms);
						 }
					}
			 }
#endif /* defined (GL_VERSION_2_0) */
			if (material->image_texture.texture)
			{
#if defined(GL_VERSION_2_0) || defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
				 /* Adjust the scaling by the ratio from the original texel
					size to the actually rendered texture size so that
					we are independent of texture reduction. */
				 float normal_scaling[4];
				 unsigned int original_dimension, *original_sizes,
						rendered_dimension, *rendered_sizes;
				 if (Cmiss_texture_get_original_texel_sizes(material->image_texture.texture,
							 &original_dimension, &original_sizes) &&
						Cmiss_texture_get_rendered_texel_sizes(material->image_texture.texture,
							 &rendered_dimension, &rendered_sizes))
				 {
						if ((original_dimension > 0) && (rendered_dimension > 0)
							 && (original_sizes[0] > 0))
						{
							 normal_scaling[0] = (float)rendered_sizes[0] / 
									(float)original_sizes[0] *
									material->lit_volume_normal_scaling[0];
						}
						else
						{
							 normal_scaling[0] = 1.0;
						}
						if ((original_dimension > 1) && (rendered_dimension > 1)
							 && (original_sizes[1] > 0))
						{
							 normal_scaling[1] = (float)rendered_sizes[1] / 
									(float)original_sizes[1] *
									material->lit_volume_normal_scaling[1];
						}
						else
						{
							 normal_scaling[1] = 1.0;
						}
						if ((original_dimension > 2) && (rendered_dimension > 2)
							 && (original_sizes[2] > 0))
						{
							 normal_scaling[2] = (float)rendered_sizes[2] / 
									(float)original_sizes[2] *
									material->lit_volume_normal_scaling[2];
						}
						else
						{
							normal_scaling[2] = 1.0;
						}
						normal_scaling[3] = 0.0;
						if (material->program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
						{
							if (glIsProgram(material->program->glsl_current_program))
							{
								loc1 = glGetUniformLocation(material->program->glsl_current_program,"normal_scaling");
								if (loc1 != (GLint)-1)
									glUniform4f(loc1, normal_scaling[0], normal_scaling[1],
										normal_scaling[2], normal_scaling[3]);
							}
						}
						else if (material->program->shader_type==MATERIAL_PROGRAM_SHADER_ARB)
						{
							glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3,
								normal_scaling);
						}
						DEALLOCATE(original_sizes);
						DEALLOCATE(rendered_sizes);
				 }
#endif /* defined(GL_VERSION_2_0) || defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
			}
		}
		else
		{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
			 if (Graphics_library_check_extension(GL_ARB_vertex_program) &&
				 Graphics_library_check_extension(GL_ARB_fragment_program))
			 {
				 glDisable(GL_VERTEX_PROGRAM_ARB);
				 glDisable(GL_FRAGMENT_PROGRAM_ARB);
				 glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
			 }
#endif
#if defined (GL_VERSION_2_0)
			 if (Graphics_library_check_extension(GL_shading_language))
			 {
						 glUseProgram(0);
						 glDisable(GL_VERTEX_PROGRAM_TWO_SIDE);
			 }
#endif
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"direct_render_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Graphical_material */
#endif /* defined (OPENGL_API) */

static void Graphical_material_Spectrum_change(
	struct MANAGER_MESSAGE(Spectrum) *message, void *material_void)
{
	struct Graphical_material *material;

	ENTER(Graphical_material_Spectrum_change);
	if (message && (material = (struct Graphical_material *)material_void))
	{
		int change = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Spectrum)(message, material->spectrum);
		if (change & MANAGER_CHANGE_RESULT(Spectrum))
		{
			material->compile_status = GRAPHICS_NOT_COMPILED;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_Spectrum_change.  Invalid argument(s)");
	}
	LEAVE;
}

/*
Global functions
----------------
*/

struct Material_package *CREATE(Material_package)(
	struct Cmiss_region *root_region,
	struct MANAGER(Spectrum) *spectrum_manager)
/*******************************************************************************
LAST MODIFIED : 20 May 2005

DESCRIPTION :
Create a shared information container for Materials.
==============================================================================*/
{
	struct Material_package *material_package;
	struct Colour colour;
	struct Material_definition
	{
		MATERIAL_PRECISION ambient[3];
		MATERIAL_PRECISION diffuse[3];
		MATERIAL_PRECISION emission[3];
		MATERIAL_PRECISION specular[3];
		MATERIAL_PRECISION alpha;
		MATERIAL_PRECISION shininess;
	}
	default_material = {
		/*ambient*/ { 1.00, 1.00, 1.00},
		/*diffuse*/ { 1.00, 1.00, 1.00},
		/*emission*/{ 0.00, 0.00, 0.00},
		/*specular*/{ 0.00, 0.00, 0.00},
		/*alpha*/1.0,
		/*shininess*/0.0},
	default_selected = {
		/*ambient*/ { 1.00, 0.20, 0.00},
		/*diffuse*/ { 1.00, 0.20, 0.00},
		/*emission*/{ 0.00, 0.00, 0.00},
		/*specular*/{ 0.00, 0.00, 0.00},
		/*alpha*/1.0,
		/*shininess*/0.0};

	ENTER(CREATE(Material_package));

	if (ALLOCATE(material_package ,struct Material_package, 1))
	{
		material_package->material_manager = CREATE(MANAGER(Graphical_material))();
		material_package->default_material = (struct Graphical_material *)NULL;
		material_package->default_selected_material = (struct Graphical_material *)NULL;
		material_package->material_program_list = CREATE(LIST(Material_program))();
		material_package->spectrum_manager = spectrum_manager;
		material_package->root_region = root_region;
		material_package->access_count = 0;

		/* command/cmiss.c overrides the ambient and diffuse colours of the
			default material to be the "foreground" colour. */
		material_package->default_material = ACCESS(Graphical_material)(
			CREATE(Graphical_material)("default"));
		colour.red   = default_material.ambient[0];
		colour.green = default_material.ambient[1];
		colour.blue  = default_material.ambient[2];
		Graphical_material_set_ambient(material_package->default_material, &colour);
		colour.red   = default_material.diffuse[0];
		colour.green = default_material.diffuse[1];
		colour.blue  = default_material.diffuse[2];
		Graphical_material_set_diffuse(material_package->default_material, &colour);
		colour.red   = default_material.emission[0];
		colour.green = default_material.emission[1];
		colour.blue  = default_material.emission[2];
		Graphical_material_set_emission(material_package->default_material, &colour);
		colour.red   = default_material.specular[0];
		colour.green = default_material.specular[1];
		colour.blue  = default_material.specular[2];
		Graphical_material_set_specular(material_package->default_material, &colour);
		Graphical_material_set_alpha(material_package->default_material,
			default_material.alpha);
		Graphical_material_set_shininess(material_package->default_material,
			default_material.shininess);
		Material_package_manage_material(material_package,
			material_package->default_material);

		material_package->default_selected_material = ACCESS(Graphical_material)(
			CREATE(Graphical_material)("default_selected"));
		colour.red   = default_selected.ambient[0];
		colour.green = default_selected.ambient[1];
		colour.blue  = default_selected.ambient[2];
		Graphical_material_set_ambient(material_package->default_selected_material, &colour);
		colour.red   = default_selected.diffuse[0];
		colour.green = default_selected.diffuse[1];
		colour.blue  = default_selected.diffuse[2];
		Graphical_material_set_diffuse(material_package->default_selected_material, &colour);
		colour.red   = default_selected.emission[0];
		colour.green = default_selected.emission[1];
		colour.blue  = default_selected.emission[2];
		Graphical_material_set_emission(material_package->default_selected_material, &colour);
		colour.red   = default_selected.specular[0];
		colour.green = default_selected.specular[1];
		colour.blue  = default_selected.specular[2];
		Graphical_material_set_specular(material_package->default_selected_material, &colour);
		Graphical_material_set_alpha(material_package->default_selected_material,
			default_selected.alpha);
		Graphical_material_set_shininess(material_package->default_selected_material,
			default_selected.shininess);
		Material_package_manage_material(material_package,
			material_package->default_selected_material);

		/* Reset the access count to zero so as these materials are owned by the package
			and so should not stop it destroying.  Correspondingly the materials must not
			DEACCESS the package when the package is being destroyed. */
		material_package->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_package).  Not enough memory");
	}
	LEAVE;

	return (material_package);
} /* CREATE(Material_package) */

static int Graphical_material_remove_package_if_matching(struct Graphical_material *material,
	void *material_package_void)
/*******************************************************************************
LAST MODIFIED : 25 November 2003

DESCRIPTION :
Iterator function to guarantee that no materials will reference the Material
package after it has been destroyed.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_remove_package_if_matching);
	if (material && material_package_void)
	{
		if (material->package == (struct Material_package *)material_package_void)
		{
			material->package = (struct Material_package *)NULL;
		}
		return_code = 1;
	}
	else
	{
 		display_message(ERROR_MESSAGE,
			"Graphical_material_remove_package_if_matching.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_remove_package_if_matching */

int DESTROY(Material_package)(struct Material_package **material_package_address)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Frees the memory for the material_package.
==============================================================================*/
{
	int return_code;
	struct Material_package *material_package;

	ENTER(DESTROY(Material));
	if (material_package_address &&
		(material_package = *material_package_address))
	{
		if (0==material_package->access_count)
		{

			if (material_package->default_material)
			{
				DEACCESS(Graphical_material)(&material_package->default_material);
			}
			if (material_package->default_selected_material)
			{
				DEACCESS(Graphical_material)(&material_package->default_selected_material);
			}
			if (material_package->root_region)
			{
				DEACCESS(Cmiss_region)(&material_package->root_region);
			}

			DESTROY(LIST(Material_program))(&material_package->material_program_list);
			/* Make sure each material no longer points at this package */
			FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
				Graphical_material_remove_package_if_matching, (void *)material_package,
				material_package->material_manager);
			DESTROY(MANAGER(Graphical_material))(&material_package->material_manager);
			DEALLOCATE(*material_package_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Material_package).  Material_package has non-zero access count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_package).  Missing material package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_package) */

DECLARE_OBJECT_FUNCTIONS(Material_package)

int Material_package_manage_material(struct Material_package *material_package,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Material_package_manage_material);
	if (material_package && material_package->material_manager && material)
	{
		if (material->package)
		{
			display_message(ERROR_MESSAGE,
				"Material_package_manage_material.  This material is already being managed");
		}
		if (return_code = ADD_OBJECT_TO_MANAGER(Graphical_material)(
			material, material_package->material_manager))
		{
			Cmiss_material_set_attribute_integer(material, CMISS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);
			/* Cannot ACCESS the package as the package is
				accessing each material through the MANAGER */
			material->package = material_package;
		}
	}
	else
	{
 		display_message(ERROR_MESSAGE,
			"Material_package_manage_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Material_package_manage_material */

struct Graphical_material *Material_package_get_default_material(
	struct Material_package *material_package)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default material object.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(Material_package_get_default_material);
	if (material_package)
	{
		material = material_package->default_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_package_get_default_material.  Invalid argument(s)");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* Material_package_get_default_material */

struct Graphical_material *Material_package_get_default_selected_material(
	struct Material_package *material_package)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default_selected material object.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(Material_package_get_default_selected_material);
	if (material_package)
	{
		material = material_package->default_selected_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_package_get_default_selected_material.  Invalid argument(s)");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* Material_package_get_default_selected_material */

struct MANAGER(Graphical_material) *Material_package_get_material_manager(
	struct Material_package *material_package)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the material manager.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *material_manager;

	ENTER(Material_package_material_manager);
	if (material_package)
	{
		material_manager = material_package->material_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_package_get_material_manager.  Invalid argument(s)");
		material_manager = (struct MANAGER(Graphical_material) *)NULL;
	}
	LEAVE;

	return (material_manager);
} /* Material_package_get_default_selected_material */

struct Graphical_material *CREATE(Graphical_material)(const char *name)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Allocates memory and assigns fields for a material.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(CREATE(Graphical_material));
	if (name)
	{
		/* allocate memory for structure */
		if (ALLOCATE(material,struct Graphical_material,1)&&
			ALLOCATE(material->name,char,strlen(name)+1))
		{
			strcpy((char *)material->name,name);
			material->access_count=0;
			material->per_pixel_lighting_flag = 0;
			material->bump_mapping_flag = 0;
			(material->ambient).red=1;
			(material->ambient).green=1;
			(material->ambient).blue=1;
			(material->diffuse).red=1;
			(material->diffuse).green=1;
			(material->diffuse).blue=1;
			material->alpha=1;
			(material->emission).red=0;
			(material->emission).green=0;
			(material->emission).blue=0;
			(material->specular).red=0;
			(material->specular).green=0;
			(material->specular).blue=0;
			material->shininess=0;
			material->spectrum=(struct Spectrum *)NULL;
			material->spectrum_manager_callback_id=NULL;
			(material->image_texture).texture=(struct Texture *)NULL;
			(material->image_texture).manager = NULL;
			(material->image_texture).field  = NULL;
			(material->image_texture).callback_id = NULL;
			(material->image_texture).material = material;
			(material->second_image_texture).texture=(struct Texture *)NULL;
			(material->second_image_texture).manager = NULL;
			(material->second_image_texture).field  = NULL;
			(material->second_image_texture).callback_id = NULL;
			(material->second_image_texture).material = material;
			(material->third_image_texture).texture=(struct Texture *)NULL;
			(material->third_image_texture).manager = NULL;
			(material->third_image_texture).field  = NULL;
			(material->third_image_texture).callback_id = NULL;
			(material->third_image_texture).material = material;
			(material->fourth_image_texture).texture=(struct Texture *)NULL;
			(material->fourth_image_texture).manager = NULL;
			(material->fourth_image_texture).field  = NULL;
			(material->fourth_image_texture).callback_id = NULL;
			(material->fourth_image_texture).material = material;
			material->package = (struct Material_package *)NULL;
			material->lit_volume_normal_scaling[0] = 1.0;
			material->lit_volume_normal_scaling[1] = 1.0;
			material->lit_volume_normal_scaling[2] = 1.0;
			material->lit_volume_normal_scaling[3] = 1.0;
			material->program = (struct Material_program *)NULL;
			material->program_uniforms = (LIST(Material_program_uniform) *)NULL;
			material->is_managed_flag = false;
			material->manager = (struct MANAGER(Graphical_material) *)NULL;
			material->manager_change_status = MANAGER_CHANGE_NONE(Graphical_material);
#if defined (OPENGL_API)
			material->display_list=0;
			material->brightness_texture_id=0;
#endif /* defined (OPENGL_API) */
			material->compile_status = GRAPHICS_NOT_COMPILED;
		}
		else
		{
			if (material)
			{
				DEALLOCATE(material);
			}
			display_message(ERROR_MESSAGE,
				"CREATE(Graphical_material).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphical_material).  Missing name");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* CREATE(Graphical_material) */

/*******************************************************************************
 * Reset the material_image_texture to hold NULL object.
 *
 * @param image_texture  Pointer to the image_texture object to be reset.
 * @return  1 if successfully reset, otherwise 0.
 */
int Material_image_texture_reset(struct Material_image_texture *image_texture)
{
	int return_code = 1;
	if (image_texture)
	{
		if (image_texture->texture)
			DEACCESS(Texture)(&(image_texture->texture));

		if (image_texture->manager &&
				image_texture->callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(image_texture->callback_id,
				image_texture->manager);
			image_texture->callback_id = NULL;
		}
		if (image_texture->field)
		{
			Cmiss_field_destroy(&(image_texture->field));
		}
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,"Material_image_texture_reset.  Invalid argument");
	}

	return return_code;
}

int DESTROY(Graphical_material)(struct Graphical_material **material_address)
/*******************************************************************************
LAST MODIFIED : 3 August 1998

DESCRIPTION :
Frees the memory for the material and sets <*material_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Graphical_material *material;

	ENTER(DESTROY(Graphical_material));
	if (material_address&&(material= *material_address))
	{
		if (0==material->access_count)
		{
			DEALLOCATE(material->name);
#if defined (OPENGL_API)
			if (material->display_list)
			{
				glDeleteLists(material->display_list, 1);
			}
#endif /* defined (OPENGL_API) */
			if (material->spectrum)
			{
				DEACCESS(Spectrum)(&(material->spectrum));
			}
			if (material->package &&
				material->spectrum_manager_callback_id)
			{
				MANAGER_DEREGISTER(Spectrum)(
					material->spectrum_manager_callback_id,
					material->package->spectrum_manager);
				material->spectrum_manager_callback_id=NULL;
			}
			Material_image_texture_reset(&(material->image_texture));
			Material_image_texture_reset(&(material->second_image_texture));
			Material_image_texture_reset(&(material->third_image_texture));
			Material_image_texture_reset(&(material->fourth_image_texture));
			if (material->program)
			{
				DEACCESS(Material_program)(&(material->program));
			}
			if (material->program_uniforms)
			{
				DESTROY(LIST(Material_program_uniform))(&material->program_uniforms);
			}
			DEALLOCATE(*material_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Graphical_material).  Graphical_material %s has non-zero access count",
				material->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphical_material).  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphical_material) */

/***************************************************************************//**
 * Something has changed in the regional computed field manager.
 * Check if the field being used is changed, if so update the material.
 */
static void Material_image_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *material_image_texture_void)
{
	struct Material_image_texture *image_texture =
		(struct Material_image_texture *)material_image_texture_void;
	if (message && image_texture)
	{
		int change = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Computed_field)(
				message, image_texture->field);
		if (change & MANAGER_CHANGE_RESULT(Computed_field))
		{
			if (image_texture->material->compile_status != GRAPHICS_NOT_COMPILED)
			{
				image_texture->material->compile_status = CHILD_GRAPHICS_NOT_COMPILED;
			}
			if (image_texture->material->manager)
				MANAGER_BEGIN_CACHE(Graphical_material)(image_texture->material->manager);
			REACCESS(Texture)(&(image_texture->texture),
				Computed_field_get_texture(image_texture->field));
			MANAGED_OBJECT_CHANGE(Graphical_material)(image_texture->material,
				MANAGER_CHANGE_DEPENDENCY(Graphical_material));
			if (image_texture->material->manager)
				MANAGER_END_CACHE(Graphical_material)(image_texture->material->manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_image_field_change.  Invalid argument(s)");
	}
}

/***************************************************************************//**
 * Set the field and update all the related objects in material_image_texture.
 * This will also create a callback for computed field.
 */
int Material_image_texture_set_field(struct Material_image_texture *image_texture,
	struct Computed_field *field)
{
	int return_code = 0;
	if (image_texture)
	{
		return_code = 1;
		if (image_texture->field)
		{
			DEACCESS(Computed_field)(&(image_texture->field));
			image_texture->field=(struct Computed_field *)NULL;
			if (image_texture->manager &&	image_texture->callback_id)
			{
				MANAGER_DEREGISTER(Computed_field)(image_texture->callback_id,
						image_texture->manager);
				image_texture->callback_id = NULL;
			}
			if (image_texture->texture)
				DEACCESS(Texture)(&(image_texture->texture));
		}
		if (field)
		{
			struct Cmiss_region *temp_region = Computed_field_get_region(field);
			MANAGER(Computed_field) *field_manager =
				Cmiss_region_get_Computed_field_manager(temp_region);
			if (field_manager)
			{
				image_texture->callback_id=
					MANAGER_REGISTER(Computed_field)(Material_image_field_change,
						(void *)image_texture, field_manager);
				image_texture->manager = field_manager;
				image_texture->field = ACCESS(Computed_field)(field);
				image_texture->texture = ACCESS(Texture)(Computed_field_get_texture(image_texture->field));
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "Material_image_texture_set_field.  Missing Material_image_texture");
		return_code = 0;
	}

	return return_code;
}

DECLARE_OBJECT_FUNCTIONS(Graphical_material)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphical_material)
DECLARE_INDEXED_LIST_FUNCTIONS(Graphical_material)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphical_material,name,
	const char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Graphical_material,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Graphical_material,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name));
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
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Insufficient memory");
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
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Graphical_material,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		(destination->diffuse).red=(source->diffuse).red;
		(destination->diffuse).green=(source->diffuse).green;
		(destination->diffuse).blue=(source->diffuse).blue;
		(destination->ambient).red=(source->ambient).red;
		(destination->ambient).green=(source->ambient).green;
		(destination->ambient).blue=(source->ambient).blue;
		(destination->emission).red=(source->emission).red;
		(destination->emission).green=(source->emission).green;
		(destination->emission).blue=(source->emission).blue;
		(destination->specular).red=(source->specular).red;
		(destination->specular).green=(source->specular).green;
		(destination->specular).blue=(source->specular).blue;
		destination->shininess=source->shininess;
		destination->alpha=source->alpha;
		if (source->package)
		{
			destination->package = source->package;
		}
		else
		{
			destination->package = (struct Material_package *)NULL;
		}
		REACCESS(Material_program)(&destination->program, source->program);
		destination->lit_volume_normal_scaling[0] =
			source->lit_volume_normal_scaling[0];
		destination->lit_volume_normal_scaling[1] =
			source->lit_volume_normal_scaling[1];
		destination->lit_volume_normal_scaling[2] =
			source->lit_volume_normal_scaling[2];
		destination->lit_volume_normal_scaling[3] =
			source->lit_volume_normal_scaling[3];
		REACCESS(Spectrum)(&(destination->spectrum), source->spectrum);
		if (destination->spectrum)
		{
			if (destination->package &&
				(!destination->spectrum_manager_callback_id))
			{
				destination->spectrum_manager_callback_id=
					MANAGER_REGISTER(Spectrum)(Graphical_material_Spectrum_change,
						(void *)destination, destination->package->spectrum_manager);
			}
		}
		else
		{
			if (destination->package &&
				destination->spectrum_manager_callback_id)
			{
				MANAGER_DEREGISTER(Spectrum)(
					destination->spectrum_manager_callback_id,
					destination->package->spectrum_manager);
				destination->spectrum_manager_callback_id=NULL;
			}
		}
		Material_image_texture_set_field(&(destination->image_texture), source->image_texture.field);
		Material_image_texture_set_field(&(destination->second_image_texture), source->second_image_texture.field);
		Material_image_texture_set_field(&(destination->third_image_texture), source->third_image_texture.field);
		Material_image_texture_set_field(&(destination->fourth_image_texture), source->fourth_image_texture.field);
		if (source->program_uniforms)
		{
			if (destination->program_uniforms)
			{
				REMOVE_ALL_OBJECTS_FROM_LIST(Material_program_uniform)(destination->program_uniforms);
			}
			else
			{
				destination->program_uniforms = CREATE(LIST(Material_program_uniform))();
			}
			COPY_LIST(Material_program_uniform)(destination->program_uniforms,
				source->program_uniforms);
		}
		else
		{
			if (destination->program_uniforms)
			{
				DESTROY(LIST(Material_program_uniform))(&destination->program_uniforms);
			}
		}
		/* flag destination display list as no longer current */
		destination->compile_status = GRAPHICS_NOT_COMPILED;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphical_material,name,const char *)
{
	char *destination_name = NULL;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Graphical_material,name));
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
			"MANAGER_COPY_IDENTIFIER(Graphical_material,name).  Insufficient memory");
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
			"MANAGER_COPY_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Graphical_material,name) */

DECLARE_MANAGER_FUNCTIONS(Graphical_material, manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Graphical_material,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS( \
	Graphical_material, name, const char *, manager)

DECLARE_MANAGER_OWNER_FUNCTIONS(Graphical_material, struct Cmiss_graphics_module)

int Material_manager_set_owner(struct MANAGER(Graphical_material) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Graphical_material)(manager, graphics_module);
}

const char *Graphical_material_name(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/
{
	const char *return_name;
	static char error_string[]="ERROR";
	static char no_name_string[]="NO NAME";

	ENTER(Graphical_material_name);
	if (material)
	{
		if (material->name)
		{
			return_name=material->name;
		}
		else
		{
			return_name=no_name_string;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_name.  Missing material");
		return_name=error_string;
	}
	LEAVE;

	return (return_name);
} /* Graphical_material_name */

/***************************************************************************//**
 * Broadcast changes in the graphical material to be propagated to objects that
 * uses it through manager that owns it.
 *
 * @param material  Modified Graphical_material to be broadcast.
 * @return 1 on success, 0 on failure
 */
int Graphical_material_changed(struct Graphical_material *material)
{
	int return_code;

	ENTER(Graphical_material_changed);
	if (material)
	{
		return_code = MANAGED_OBJECT_CHANGE(Graphical_material)(material,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Graphical_material));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_changed.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Graphical_material_get_ambient(struct Graphical_material *material,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the ambient colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_ambient);
	if (material&&ambient)
	{
		ambient->red=material->ambient.red;
		ambient->green=material->ambient.green;
		ambient->blue=material->ambient.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_ambient */

int Graphical_material_set_ambient(struct Graphical_material *material,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the ambient colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_ambient);
	if (material&&ambient)
	{
		material->ambient.red=ambient->red;
		material->ambient.green=ambient->green;
		material->ambient.blue=ambient->blue;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_ambient */

int Graphical_material_get_diffuse(struct Graphical_material *material,
	struct Colour *diffuse)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the diffuse colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_diffuse);
	if (material&&diffuse)
	{
		diffuse->red=material->diffuse.red;
		diffuse->green=material->diffuse.green;
		diffuse->blue=material->diffuse.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_diffuse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_diffuse */

int Graphical_material_set_diffuse(struct Graphical_material *material,
	struct Colour *diffuse)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the diffuse colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_diffuse);
	if (material&&diffuse)
	{
		material->diffuse.red=diffuse->red;
		material->diffuse.green=diffuse->green;
		material->diffuse.blue=diffuse->blue;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_diffuse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_diffuse */

int Graphical_material_get_emission(struct Graphical_material *material,
	struct Colour *emission)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the emission colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_emission);
	if (material&&emission)
	{
		emission->red=material->emission.red;
		emission->green=material->emission.green;
		emission->blue=material->emission.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_emission.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_emission */

int Graphical_material_set_emission(struct Graphical_material *material,
	struct Colour *emission)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the emission colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_emission);
	if (material&&emission)
	{
		material->emission.red=emission->red;
		material->emission.green=emission->green;
		material->emission.blue=emission->blue;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_emission.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_emission */

int Graphical_material_get_specular(struct Graphical_material *material,
	struct Colour *specular)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the specular colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_specular);
	if (material&&specular)
	{
		specular->red=material->specular.red;
		specular->green=material->specular.green;
		specular->blue=material->specular.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_specular.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_specular */

int Graphical_material_set_specular(struct Graphical_material *material,
	struct Colour *specular)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the specular colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_specular);
	if (material&&specular)
	{
		material->specular.red=specular->red;
		material->specular.green=specular->green;
		material->specular.blue=specular->blue;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_specular.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_specular */

int Graphical_material_get_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION *alpha)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the alpha value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_alpha);
	if (material&&alpha)
	{
		*alpha=material->alpha;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_alpha */

int Graphical_material_set_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION alpha)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the alpha value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_alpha);
	if (material&&(0.0 <= alpha)&&(1.0 >= alpha))
	{
		material->alpha=alpha;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_alpha */

int Graphical_material_get_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION *shininess)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the shininess value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_shininess);
	if (material&&shininess)
	{
		*shininess=material->shininess;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_shininess.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_shininess */

int Graphical_material_set_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION shininess)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the shininess value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_shininess);
	if (material&&(0.0 <= shininess)&&(1.0 >= shininess))
	{
		material->shininess=shininess;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_shininess.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_shininess */

int Graphical_material_get_bump_mapping_flag(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for bump_mapping.
==============================================================================*/
{
	 int return_code;

	 ENTER(Graphical_material_get_bump_mapping_flag);
	 if (material)
	 {
			return_code = material->bump_mapping_flag;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Graphical_material_get_bump_mapping_flag.  Invalid argument(s)");
			return_code=0;
	 }
	LEAVE;

	return (return_code);
}

int Graphical_material_get_per_pixel_lighting_flag(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for per_pixel_lighting.
==============================================================================*/
{
	 int return_code;

	 ENTER(Graphical_material_get_per_pixel_lighting_flag);
	 if (material)
	 {
			return_code = material->per_pixel_lighting_flag;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Graphical_material_get_per_pixel_flag.  Invalid argument(s)");
			return_code=0;
	 }
	LEAVE;

	return (return_code);
}

struct Computed_field *Cmiss_material_get_first_image_field(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Computed_field *image_field;

	ENTER(Cmiss_material_get_first_image_field);
	if (material)
	{
		image_field=material->image_texture.field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_get_first_image_field.  Missing material");
		image_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (image_field);
} /* Cmiss_material_get_first_image_field */

struct Computed_field *Cmiss_material_get_second_image_field(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Computed_field *image_field;

	ENTER(Cmiss_material_get_second_image_field);
	if (material)
	{
		image_field=material->second_image_texture.field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_get_second_image_field.  Missing material");
		image_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (image_field);
} /* Cmiss_material_get_second_image_field */

struct Computed_field *Cmiss_material_get_third_image_field(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Computed_field *image_field;

	ENTER(Cmiss_material_get_third_image_field);
	if (material)
	{
		image_field=material->third_image_texture.field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_get_third_image_field.  Missing material");
		image_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (image_field);
} /* Cmiss_material_get_third_image_field */

struct Computed_field *Cmiss_material_get_fourth_image_field(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Computed_field *image_field;

	ENTER(Cmiss_material_get_fourth_image_field);
	if (material)
	{
		image_field=material->fourth_image_texture.field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_get_fourth_image_field.  Missing material");
		image_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (image_field);
} /* Cmiss_material_get_fourth_image_field */

Cmiss_field_id  Cmiss_material_get_image_field(Cmiss_material_id material,
	enum Cmiss_material_image_field_identifier identifier)
{
	Cmiss_field_id image_field = NULL;

	ENTER(Cmiss_material_get_image_field);
	if (material)
	{
		switch(identifier)
		{
			case CMISS_MATERIAL_FIRST_IMAGE_FIELD:
			{
				image_field = Cmiss_material_get_first_image_field(material);
			} break;
			case CMISS_MATERIAL_SECOND_IMAGE_FIELD:
			{
				image_field = Cmiss_material_get_second_image_field(material);
			} break;
			case CMISS_MATERIAL_THIRD_IMAGE_FIELD:
			{
				image_field = Cmiss_material_get_third_image_field(material);
			} break;
			case CMISS_MATERIAL_FOURTH_IMAGE_FIELD:
			{
				image_field = Cmiss_material_get_fourth_image_field(material);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_material_get_image_field.  Invalid image field has been specified");
				image_field = NULL;
			} break;
		}
		if (image_field)
		{
			Cmiss_field_access(image_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_get_image_field.  Missing material");
		image_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (image_field);
} /* Cmiss_material_get_fourth_image_field */

struct Texture *Graphical_material_get_texture(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Texture *texture;

	ENTER(Graphical_material_get_texture);
	if (material)
	{
		texture=material->image_texture.texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_texture.  Missing material");
		texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Graphical_material_get_texture */

struct Texture *Graphical_material_get_second_texture(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the second texture of the material.
==============================================================================*/
{
	struct Texture *texture;

	ENTER(Graphical_material_get_second_texture);
	if (material)
	{
		texture=material->second_image_texture.texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_second_texture.  Missing material");
		texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Graphical_material_get_second_texture */

struct Texture *Graphical_material_get_third_texture(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the third texture of the material.
==============================================================================*/
{
	struct Texture *texture;

	ENTER(Graphical_material_get_third_texture);
	if (material)
	{
		texture=material->third_image_texture.texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_third_texture.  Missing material");
		texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Graphical_material_get_second_texture */

struct Texture *Graphical_material_get_fourth_texture(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the fourth texture of the material.
==============================================================================*/
{
	struct Texture *texture;

	ENTER(Graphical_material_get_fourth_texture);
	if (material)
	{
		texture=material->fourth_image_texture.texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_fourth_texture.  Missing material");
		texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Graphical_material_get_fourth_texture */

int Graphical_material_set_colour_lookup_spectrum(struct Graphical_material *material,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Sets the spectrum member of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_colour_lookup_spectrum);
	if (material)
	{
#if defined (GL_VERSION_1_3)
		if (Graphics_library_tentative_check_extension(GL_VERSION_1_3))
		{
			REACCESS(Spectrum)(&material->spectrum, spectrum);
			material->compile_status = GRAPHICS_NOT_COMPILED;
			Graphical_material_changed(material);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"A colour lookup spectrum requires OpenGL version 1.3 or better which is "
				"not available on this display.");
			return_code = 0;
		}
#else /* defined (GL_VERSION_1_3) */
		USE_PARAMETER(spectrum);
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_colour_lookup_spectrum.  "
			"OpenGL version 1.3 required for colour lookup spectrums and not compiled into this executable.");
		return_code=0;
#endif /* defined (GL_VERSION_1_3) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_colour_lookup_spectrum.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_colour_lookup_spectrum */

struct Spectrum *Graphical_material_get_colour_lookup_spectrum(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Returns the spectrum member of the material.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(Graphical_material_get_colour_lookup_spectrum);
	if (material)
	{
		spectrum=material->spectrum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_colour_lookup_spectrum.  Missing material");
		spectrum=(struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* Graphical_material_get_colour_lookup_spectrum */

int Cmiss_material_set_first_image_field(Cmiss_material_id material,
		Cmiss_field_id field)
{
	int return_code;

	ENTER(Cmiss_material_set_first_image_field);
	if (material)
	{
		return_code = Material_image_texture_set_field(&(material->image_texture), field);
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_set_first_image_field.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_material_set_second_image_field(Cmiss_material_id material,
		Cmiss_field_id field)
{
	int return_code;

	ENTER(Graphical_material_set_second_image_field);
	if (material)
	{
		return_code = Material_image_texture_set_field(&(material->second_image_texture), field);
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_set_second_image_field.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_material_set_third_image_field(Cmiss_material_id material,
		Cmiss_field_id field)
{
	int return_code;

	ENTER(Graphical_material_set_third_image_field);
	if (material)
	{
		return_code = Material_image_texture_set_field(&(material->third_image_texture), field);
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_set_third_image_field.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_material_set_fourth_image_field(Cmiss_material_id material,
		Cmiss_field_id field)
{
	int return_code;

	ENTER(Graphical_material_set_fourth_image_field);
	if (material)
	{
		return_code = Material_image_texture_set_field(&(material->fourth_image_texture), field);
		material->compile_status = GRAPHICS_NOT_COMPILED;
		Graphical_material_changed(material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_set_fourth_image_field.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
int Cmiss_material_set_image_field(Cmiss_material_id material,
		enum Cmiss_material_image_field_identifier identifier, Cmiss_field_id field)
{
	int return_code = 0;
	ENTER(Cmiss_material_set_image_field);
	if (material)
	{
		switch(identifier)
		{
			case CMISS_MATERIAL_FIRST_IMAGE_FIELD:
			{
				return_code = Cmiss_material_set_first_image_field(material, field);
			} break;
			case CMISS_MATERIAL_SECOND_IMAGE_FIELD:
			{
				return_code = Cmiss_material_set_second_image_field(material, field);
			} break;
			case CMISS_MATERIAL_THIRD_IMAGE_FIELD:
			{
				return_code = Cmiss_material_set_third_image_field(material, field);
			} break;
			case CMISS_MATERIAL_FOURTH_IMAGE_FIELD:
			{
				return_code = Cmiss_material_set_fourth_image_field(material, field);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_material_set_image_field.  Invalid image field has been specified");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_material_set_image_field.  Missing material");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_material_set_image_field */

int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *material_package_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Shifted from command/cmiss.c now that there is a material package.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/
{
	const char *current_token;
	int material_is_new,return_code;
	struct Graphical_material *material;
	struct Material_package *material_package;

	ENTER(gfx_create_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (material_package=(struct Material_package *)material_package_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					/* if there is an existing material of that name, just modify it */
					if (!(material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						current_token,material_package->material_manager)))
					{
						if (material=CREATE(Graphical_material)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(material,
								material_package->default_material);
						}
						material_is_new=1;
					}
					else
					{
						material_is_new=0;
					}
					if (material)
					{
						shift_Parse_state(state,1);
						if (state->current_token)
						{
							return_code=modify_Graphical_material(state,(void *)material,
								material_package_void);
						}
						else
						{
							return_code=1;
						}
						if (material_is_new)
						{
							Cmiss_material_set_attribute_integer(material, CMISS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);
							ADD_OBJECT_TO_MANAGER(Graphical_material)(material,
								material_package->material_manager);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_material.  Error creating material");
						return_code=0;
					}
				}
				else
				{
					return_code=modify_Graphical_material(state,(void *)NULL,
						material_package_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_material.  Missing material_package_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_material */

int set_material_program_type_texture_mode(struct Graphical_material *material_to_be_modified,
	 int *type, int return_code)
{
	 int dimension;

	 ENTER(set_material_program_type_texture_mode);
	 if (material_to_be_modified->image_texture.texture)
	 {
			Texture_get_dimension(material_to_be_modified->image_texture.texture, &dimension);
			switch (dimension)
			{
				 case 1:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1;
				 } break;
				 case 2:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
				 } break;
				 case 3:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 |
							 MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
				 } break;
				 default:
				 {
						display_message(ERROR_MESSAGE, "Colour texture dimension %d not supported.",
							 dimension);
						return_code = 0;
				 } break;
			}
			switch (Texture_get_number_of_components(material_to_be_modified->image_texture.texture))
			{
				 case 1:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1;
				 } break;
				 case 2:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2;
				 } break;
				 case 3:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 |
							 MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2;
				 } break;
				 case 4:
				 {
						/* Do nothing as zero in these bits indicates rgba */
				 } break;
				 default:
				 {
						display_message(ERROR_MESSAGE, "Colour texture output dimension not supported.");
						return_code = 0;
				 } break;
			}
			switch (Texture_get_combine_mode(material_to_be_modified->image_texture.texture))
			{
				 case TEXTURE_DECAL:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL;
				 }
				 default:
				 {
						/* Do nothing as modulate is the default */
				 }
			}
	 }
	 LEAVE;

	 return return_code;
}

int set_material_program_type_second_texture(struct Graphical_material *material_to_be_modified,
	 int *type, int return_code)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program second texture type
for using the vertex and fragment program. This and following
functions are orginally from the modify_graphical_materil.
==============================================================================*/
{
	 int dimension;

	 ENTER(set_material_program_type_second_texture);
	 if (material_to_be_modified->second_image_texture.texture)
	 {
			Texture_get_dimension(material_to_be_modified->second_image_texture.texture, &dimension);
			switch (dimension)
			{
				 case 1:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1;
				 } break;
				 case 2:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2;
				 } break;
				 case 3:
				 {
						*type |= MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1 |
							 MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2;
				 } break;
				 default:
				 {
						display_message(ERROR_MESSAGE, "Second texture dimension %d not supported.",
							 dimension);
						return_code = 0;
				 } break;
			}
	 }
	 LEAVE;

	 return return_code;
}

int set_material_program_type_bump_mapping(struct Graphical_material *material_to_be_modified,
	 int *type, int return_code)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program bump mapping type
for using the vertex and fragment program. This and following
functions are orginally from the modify_graphical_materil.
==============================================================================*/
{
	 ENTER(set_material_program_type_bump_mapping);
	 if (material_to_be_modified->second_image_texture.texture)
	 {
			*type |= MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP;
			material_to_be_modified->bump_mapping_flag = 1;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Bump mapping requires specification of a second texture containing a normal map.");
			material_to_be_modified->bump_mapping_flag = 0;
			return_code = 0;
	 }
	 LEAVE;

	 return return_code;
}

int set_material_program_type_spectrum(struct Graphical_material *material_to_be_modified,
	 int *type, int red_flag, int green_flag, int blue_flag,
	 int alpha_flag, int return_code)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program spectrum type
for using the vertex and fragment program. This and following
functions are orginally from the modify_graphical_materil.
==============================================================================*/
{
	 enum Spectrum_colour_components spectrum_colour_components;
	 int number_of_spectrum_components;

	 ENTER(set_material_program_type_spectrum);
	 if (material_to_be_modified->spectrum)
	 {
			/* Cannot just rely on the COPY functions as when
				 first created this will be the actual object. */
			if (material_to_be_modified->package &&
				 (!material_to_be_modified->spectrum_manager_callback_id))
			{
				 material_to_be_modified->spectrum_manager_callback_id=
						MANAGER_REGISTER(Spectrum)(Graphical_material_Spectrum_change,
							 (void *)material_to_be_modified, material_to_be_modified->package->spectrum_manager);
			}

			/* Specify the input colours */
			if (red_flag)
			{
				 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1;
			}
			if (green_flag)
			{
				 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2;
			}
			if (blue_flag)
			{
				 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3;
			}
			if (alpha_flag)
			{
				 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4;
			}

			number_of_spectrum_components =
				 Spectrum_get_number_of_components(material_to_be_modified->spectrum);
			if ((alpha_flag + blue_flag +
						green_flag + red_flag) ==
				 number_of_spectrum_components)
			{
				 spectrum_colour_components = Spectrum_get_colour_components(
						material_to_be_modified->spectrum);
				 if (spectrum_colour_components & SPECTRUM_COMPONENT_ALPHA)
				 {
						if (spectrum_colour_components == SPECTRUM_COMPONENT_ALPHA)
						{
							 /* Alpha only */
							 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA;
						}
						else
						{
							 /* Colour and alpha */
							 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR
									| MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA;
						}
				 }
				 else
				 {
						/* Colour only */
						*type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR;
				 }
			}
			else if (1 == number_of_spectrum_components)
			{
				 /* Lookup each component specified in the 1D spectra only */
				 *type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP;
			}
	 }
	 else
	 {
			if (material_to_be_modified->package &&
				 material_to_be_modified->spectrum_manager_callback_id)
			{
				 MANAGER_DEREGISTER(Spectrum)(
						material_to_be_modified->spectrum_manager_callback_id,
						material_to_be_modified->package->spectrum_manager);
				 material_to_be_modified->spectrum_manager_callback_id=NULL;
			}
	 }
	 LEAVE;

	 return return_code;
}

int material_update_material_program(struct Graphical_material *material_to_be_modified,
	 struct Material_package *material_package, enum Material_program_type type, int return_code)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Check the material program and renew it if necessary.
==============================================================================*/
{
	 ENTER(material_update_material_program);
	 if (!material_to_be_modified->program ||
			(material_to_be_modified->program->type != type))
	 {
			if (material_to_be_modified->program)
			{
				 DEACCESS(Material_program)(&material_to_be_modified->program);
			}
			if (material_to_be_modified->program =
				 FIND_BY_IDENTIFIER_IN_LIST(Material_program,type)(
						type, material_package->material_program_list))
			{
				 ACCESS(Material_program)(material_to_be_modified->program);
			}
			else
			{
				 if (material_to_be_modified->program = ACCESS(Material_program)(
								CREATE(Material_program)(type)))
				 {
						ADD_OBJECT_TO_LIST(Material_program)(material_to_be_modified->program,
							 material_package->material_program_list);
				 }
				 else
				 {
						return_code = 0;
				 }
			}
	 }
	 LEAVE;

	 return return_code;
}

int set_material_program_type(struct Graphical_material *material_to_be_modified,
	 int bump_mapping_flag, int colour_lookup_red_flag, int colour_lookup_green_flag,
	 int colour_lookup_blue_flag,  int colour_lookup_alpha_flag,
	 int lit_volume_intensity_normal_texture_flag, int lit_volume_finite_difference_normal_flag,
	 int lit_volume_scale_alpha_flag, int return_code)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program type for using the vertex
and fragment program. This and following functions are orginally
from the modify_graphical_material.
NOTE: I use the pointer to the material_package from the material.
==============================================================================*/
{
	 int type;

	 ENTER(set_material_program_type);

	 type = MATERIAL_PROGRAM_PER_PIXEL_LIGHTING;
	 material_to_be_modified->per_pixel_lighting_flag = 1;

	 return_code = set_material_program_type_texture_mode(material_to_be_modified,
			&type, return_code);
	 return_code = set_material_program_type_second_texture(material_to_be_modified,
			&type, return_code);
	 if (bump_mapping_flag)
	 {
			return_code = set_material_program_type_bump_mapping(material_to_be_modified,
				 &type, return_code);
	 }
	 else
	 {
			material_to_be_modified->bump_mapping_flag = 0;
	 }

	 return_code = set_material_program_type_spectrum(material_to_be_modified,
			&type, colour_lookup_red_flag, colour_lookup_green_flag, colour_lookup_blue_flag,
			colour_lookup_alpha_flag, return_code);

	 if (lit_volume_intensity_normal_texture_flag)
	 {
			type |= MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE;
	 }
	 if (lit_volume_finite_difference_normal_flag)
	 {
			type |= MATERIAL_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL;
	 }
	 if (lit_volume_scale_alpha_flag)
	 {
			type |= MATERIAL_PROGRAM_CLASS_LIT_VOLUME_SCALE_ALPHA;
	 }

	 material_to_be_modified->compile_status = GRAPHICS_NOT_COMPILED;

	 return_code = material_update_material_program(material_to_be_modified,
			material_to_be_modified->package, (Material_program_type)type, return_code);

	 return return_code;
}

int material_copy_bump_mapping_and_per_pixel_lighting_flag(struct Graphical_material *material,
	 struct Graphical_material *material_to_be_modified)
/******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION : This function will set the bump mapping and per
pixel_lighting_flag of the material_to_be_modified to be the same as
the one in material, it is used for setting up the GUI.
==============================================================================*/
{
	 int return_code;
	 if (material && material_to_be_modified)
	 {
			material_to_be_modified->bump_mapping_flag =
				 material->bump_mapping_flag;
			material_to_be_modified->per_pixel_lighting_flag =
				 material->per_pixel_lighting_flag;
			return_code = 1;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "material_deaccess_material_program.  Missing material_program");
			return_code = 0;
	 }

	 return return_code;
}

/**************************************************************************//**
 * Sets the material to use a #Material_program with user specified strings
 * for the vertex_program and fragment_program.
 */
int Material_set_material_program_strings(struct Graphical_material *material_to_be_modified,
	char *vertex_program_string, char *fragment_program_string, char *geometry_program_string)
{
	int return_code;
	struct Material_program *old_program;

	ENTER(Material_set_material_program_strings);

	if (old_program = material_to_be_modified->program)
	{
#if defined (OPENGL_API)
		if (!vertex_program_string)
		{
			vertex_program_string = old_program->vertex_program_string;
		}
		if (!fragment_program_string)
		{
			fragment_program_string = old_program->fragment_program_string;
		}
		if (!geometry_program_string)
		{
			geometry_program_string = old_program->geometry_program_string;
		}
#endif /* defined (OPENGL_API) */
	}
	if (material_to_be_modified->program = ACCESS(Material_program)(
		Material_program_create_from_program_strings(
		vertex_program_string, fragment_program_string, geometry_program_string)))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	if (old_program)
	{
		DEACCESS(Material_program)(&old_program);
	}
	LEAVE;

	return return_code;
}

#if defined (WX_USER_INTERFACE)
int material_deaccess_material_program(struct Graphical_material *material_to_be_modified)
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : This function is to allow the material editor to
deaccess the material program from the material.
==============================================================================*/
{
	 int return_code;
	 ENTER(material_deaccess_material_program);

	 if (material_to_be_modified->program)
	 {
			DEACCESS(Material_program)(&material_to_be_modified->program);
			material_to_be_modified->compile_status = GRAPHICS_NOT_COMPILED;
			material_to_be_modified->per_pixel_lighting_flag = 0;
			material_to_be_modified->bump_mapping_flag = 0;
			return_code = 1;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "material_deaccess_material_program.  Missing material_program");
			return_code = 0;
	 }
	 LEAVE;

	 return return_code;
}
#endif /* (WX_USER_INTERFACE) */

int set_Material_image_texture(struct Parse_state *state,void *material_image_texture_void,
		void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct Cmiss_region *root_region = NULL;
	struct Computed_field *temp_field = NULL;
	struct Material_image_texture *image_texture;

	ENTER(set_Material_image_field);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				image_texture=(struct Material_image_texture *)material_image_texture_void;
				root_region = (struct Cmiss_region *)root_region_void;
				if (image_texture	&& root_region)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (image_texture->field)
						{
							Material_image_texture_set_field(image_texture,	NULL);
						}
						return_code=1;
					}
					else
					{
						struct Cmiss_region *region = NULL;
						char *region_path = NULL, *field_name = NULL;
						if (Cmiss_region_get_partial_region_path(root_region,
							current_token, &region, &region_path, &field_name))
						{
							Cmiss_field_module *field_module = Cmiss_region_get_field_module(region);
							if (field_name && (strlen(field_name) > 0) &&
								(strchr(field_name, CMISS_REGION_PATH_SEPARATOR_CHAR)	== NULL))
							{
								temp_field = Cmiss_field_module_find_field_by_name(field_module,
									field_name);
								if (temp_field &&
										!Computed_field_is_image_type(temp_field,NULL))
								{
									DEACCESS(Computed_field)(&temp_field);
									display_message(ERROR_MESSAGE,
										"set_Material_image_field.  Field specify does not contain image "
										"information.");
									return_code=0;
								}
							}
							else
							{
								if (field_name)
								{
									display_message(ERROR_MESSAGE,
										"set_Material_image_texture:  Invalid region path or texture field name '%s'", field_name);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_Material_image_texture:  Missing texture field name or name matches child region '%s'", current_token);
								}
								display_parse_state_location(state);
								return_code = 0;
							}
							Cmiss_field_module_destroy(&field_module);
						}
						if (region_path)
							DEALLOCATE(region_path);
						if (field_name)
							DEALLOCATE(field_name);
						if (temp_field)
						{
							Material_image_texture_set_field(image_texture,	temp_field);
							return_code=1;
							DEACCESS(Computed_field)(&temp_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Material_image_field.  Image field does not exist");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Material_image_field.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," IMAGE_NAME|none");
				image_texture=(struct Material_image_texture *)material_image_texture_void;
				if (image_texture)
				{
					temp_field= image_texture->field;
					if (temp_field)
					{
						char *temp_name = Cmiss_field_get_name(temp_field);
						display_message(INFORMATION_MESSAGE,"[%s]",temp_name);
						DEALLOCATE(temp_name);
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
			display_message(ERROR_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Material_image_field.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* set_Material_image_field */


int modify_Graphical_material(struct Parse_state *state,void *material_void,
	void *material_package_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2007

DESCRIPTION :
==============================================================================*/
{
	const char *current_token;
	char bump_mapping_flag, colour_lookup_red_flag, colour_lookup_green_flag,
		colour_lookup_blue_flag, colour_lookup_alpha_flag,
		lit_volume_finite_difference_normal_flag,
		lit_volume_intensity_normal_texture_flag, lit_volume_scale_alpha_flag,
		normal_mode_flag, per_pixel_mode_flag,
		*fragment_program_string, *vertex_program_string, *geometry_program_string,
		*uniform_name;
	/*	enum Spectrum_colour_components spectrum_colour_components; */
	int /*dimension,*/ lit_volume_normal_scaling_number,
		number_of_spectrum_components, number_of_uniform_values,
		process, return_code;
	struct Graphical_material *material_to_be_modified,
		*material_to_be_modified_copy;
	struct Material_package *material_package;
	struct Option_table *help_option_table, *option_table, *mode_option_table;
	double *uniform_values;

	ENTER(modify_Graphical_material);
	if (state)
	{
		if (material_package = (struct Material_package *)material_package_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (material_to_be_modified=(struct Graphical_material *)material_void)
				{
					if (IS_MANAGED(Graphical_material)(material_to_be_modified,
						material_package->material_manager))
					{
						if (material_to_be_modified_copy=CREATE(Graphical_material)("copy"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
								material_to_be_modified_copy,material_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create material copy");
							return_code=0;
						}
					}
					else
					{
						material_to_be_modified_copy=material_to_be_modified;
						material_to_be_modified=(struct Graphical_material *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (material_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Graphical_material,name)(current_token,
							material_package->material_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (material_to_be_modified_copy=CREATE(Graphical_material)(
									"copy"))
								{
									MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
										material_to_be_modified_copy,material_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"modify_Graphical_material.  Could not create material copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (material_to_be_modified=CREATE(Graphical_material)("help"))
						{
							if (material_package->default_material)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
									material_to_be_modified,material_package->default_material);
							}
							help_option_table = CREATE(Option_table)();
							Option_table_add_entry(help_option_table, "MATERIAL_NAME",
								material_to_be_modified, material_package_void,
								modify_Graphical_material);
							return_code=Option_table_parse(help_option_table, state);
							DESTROY(Option_table)(&help_option_table);
							DESTROY(Graphical_material)(&material_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create dummy material");
							return_code=0;
						}
					}
				}
				if (process)
				{
					bump_mapping_flag = 0;
					normal_mode_flag = 0;
					per_pixel_mode_flag = 0;
					colour_lookup_red_flag = 0;
					colour_lookup_green_flag = 0;
					colour_lookup_blue_flag = 0;
					colour_lookup_alpha_flag = 0;
					lit_volume_intensity_normal_texture_flag = 0;
					lit_volume_finite_difference_normal_flag = 0;
					lit_volume_scale_alpha_flag = 0;
					vertex_program_string = (char *)NULL;
					geometry_program_string = (char *)NULL;
					fragment_program_string = (char *)NULL;
					uniform_name = (char *)NULL;
					number_of_uniform_values = 0;
					uniform_values = (double *)NULL;
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
						"The material controls how pixels will be rendered on the "
						"screen.  The initial mode of operation reflects the "
						"standard gouraud shading model usual within OpenGL, "
						"this is called <normal_mode>. "
						"This has rgb colour values for <diffuse>, <ambient> "
						"<emission> and <specular> colours.  The <ambient> "
						"colour is the unlit colour, the <diffuse> colour "
						"interacts with the light and is the lit surface, "
						"the <specular> colour is used for the glossy "
						"highlights, the spread of which is specified by the"
						"<shininess>.  The <alpha> controls the transparency "
						"of the material.  See a/a1."
						"If a <texture> is specified then this is combined with "
						"the calculated gouraud colour according to the textures "
						"rendering mode and the texture coordinates.  "
						"Additional support is provided for more accurate "
						"rendering using phong shading calculated at every pixel, "
						"called <per_pixel_lighting>.  This is implemented using "
						"the ARB_vertex_program and ARB_fragment_program OpenGL "
						"extensions.  The program only implements a single point "
						"light so you should set the default light to a point light "
						"at the position you want.  See a/per_pixel_lighting.  "
						"<bump_mapping> uses a <secondary_texture> to specify "
						"a perturbation to the normal, giving a smooth model "
						"a much more detailed surface appearance.  "
						"Also demonstrated in a/per_pixel_lighting.  "
						"A <colour_lookup_spectrum> takes the calculated colour values "
						"and further modifies them by using the specified subset "
						"of colour values specified by <colour_lookup_red>, "
						"<colour_lookup_green>, "
						"<colour_lookup_blue> and <colour_lookup_alpha>, as inputs "
						"to a 1, 2 or 3 component spectrum.  Depending on the "
						"spectrum this will override either the rgb colour, "
						"the alpha value or both rgb and alpha.  "
						"If the number of input components used in the spectrum "
						"matches the number of components specified then a texture "
						"of this dimension will be used and evaluated for each tensor "
						"product combination.  If only a 1 component spectrum is "
						"specified then it will be applied independently to each input "
						"component specified.  This spectrum "
						"can be modified to quickly change the appearance of a "
						"large volume dataset, see example a/indexed_volume.  "
						"A lit volume uses a per voxel normal to calculate the "
						"phong lighting model at each pixel.  The normal can "
						"be specified by encoding it into the blue, green and "
						"alpha channels of an input texture (the red channel "
						"being the intensity) <lit_volume_intensity_normal_texture>."
						"Alternatively it can be estimated on the fly by applying "
						"a finite difference operator to the pixel intensities. "
						"<lit_volume_finite_difference_normal>."
						"A <lit_volume_normal_scaling> can be applied, modifying "
						"the estimated normal by scaling it.  The normal is used "
						"as if it is a coordinate normal and so the texture "
						"coordinates must line up with the geometrical coordinates. "
						"The <lit_volume_normal_scaling> can be used to account for "
						"when the texture coordinates are not equally matched to "
						"the geometrical coordinates. "
						"The magnitude of the <lit_volume_normal_scaling> will only "
						"affect the optional following parameter, "
						"<lit_volume_scale_alpha>, scaling the alpha attenuation. "
						"Optionally with either normal, the magnitude of that normal"
						"can multiply the calculated alpha value, "
						"<lit_volume_scale_alpha> making those pixels with small "
						"gradients more transparent.  See a/volume_render. "
						"<secondary_texture>, <third_texture> and <fourth_texture> will "
						"be blended with the first <texture> according to the multitexture "
						"rules controlled by each textures combine mode (such as modulate or add). "
						"Specifying a <vertex_program_string> and <fragment_program_string> allows "
						"any arbitrary program to be loaded, overriding the one that is generated automatically, "
						"no checks for consistency with textures or inputs are made.  Both must be specified together. "
						"Optional <geometry_program_string> is also available, which is only available on "
						"relatively newer hardware, it must be used with <vertex_program_string> and "
						"<fragment_program_string> and only works with surface at the moment. Specifying "
						"a <uniform_name> and <uniform_value> allows any uniform qualified variables in"
						"any arbitrary program to be set. At the moment only float type is supported.");

					Option_table_add_entry(option_table, "alpha",
						&(material_to_be_modified_copy->alpha), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "ambient",
						&(material_to_be_modified_copy->ambient), NULL,
						set_Colour);
					Option_table_add_char_flag_entry(option_table,
						"bump_mapping", &bump_mapping_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_alpha", &colour_lookup_alpha_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_blue", &colour_lookup_blue_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_green", &colour_lookup_green_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_red", &colour_lookup_red_flag);
					Option_table_add_entry(option_table, "colour_lookup_spectrum",
						&(material_to_be_modified_copy->spectrum),
						material_package->spectrum_manager,
						set_Spectrum);
					Option_table_add_entry(option_table, "diffuse",
						&(material_to_be_modified_copy->diffuse), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "emission",
						&(material_to_be_modified_copy->emission), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "fourth_texture",
						&(material_to_be_modified_copy->fourth_image_texture),
						material_package->root_region,
						set_Material_image_texture);
					Option_table_add_name_entry(option_table, "fragment_program_string",
						&fragment_program_string);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_intensity_normal_texture",
						&lit_volume_intensity_normal_texture_flag);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_finite_difference_normal",
						&lit_volume_finite_difference_normal_flag);
					lit_volume_normal_scaling_number = 3;
					Option_table_add_float_vector_entry(option_table,
						"lit_volume_normal_scaling",
						material_to_be_modified_copy->lit_volume_normal_scaling,
						&lit_volume_normal_scaling_number);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_scale_alpha",
						&lit_volume_scale_alpha_flag);
					mode_option_table = CREATE(Option_table)();
					Option_table_add_char_flag_entry(mode_option_table,
						"normal_mode", &normal_mode_flag);
					Option_table_add_char_flag_entry(mode_option_table,
						"per_pixel_mode", &per_pixel_mode_flag);
					Option_table_add_suboption_table(option_table, mode_option_table);
					Option_table_add_entry(option_table, "secondary_texture",
						&(material_to_be_modified_copy->second_image_texture),
						material_package->root_region,
						set_Material_image_texture);
					Option_table_add_entry(option_table, "shininess",
						&(material_to_be_modified_copy->shininess), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "specular",
						&(material_to_be_modified_copy->specular), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "texture",
						&(material_to_be_modified_copy->image_texture),
						material_package->root_region,
						set_Material_image_texture);
					Option_table_add_entry(option_table, "third_texture",
						&(material_to_be_modified_copy->third_image_texture),
						material_package->root_region,
						set_Material_image_texture);
					Option_table_add_name_entry(option_table, "vertex_program_string",
						&vertex_program_string);
					Option_table_add_name_entry(option_table, "geometry_program_string",
						&geometry_program_string);
					Option_table_add_name_entry(option_table, "uniform_name",
						&uniform_name);
					Option_table_add_variable_length_double_vector_entry(option_table,
						"uniform_values", &number_of_uniform_values, &uniform_values);
					if (return_code=Option_table_multi_parse(option_table, state))
					{
						if (normal_mode_flag + per_pixel_mode_flag > 1)
						{
							display_message(ERROR_MESSAGE,
								"Specify only one of normal_mode/per_pixel_mode.");
							return_code = 0;
						}
						if (fragment_program_string || vertex_program_string)
						{
							if (normal_mode_flag)
							{
								display_message(ERROR_MESSAGE,
									"vertex_program_string and fragment_program_string"
									"imply per_pixel_mode and cannot be used with normal_mode.");
								return_code = 0;
							}
							per_pixel_mode_flag = 1;
						}
						if ((!material_to_be_modified_copy->program ||
							(0 != material_to_be_modified_copy->program->type))
							&& ((fragment_program_string && !vertex_program_string) ||
							(vertex_program_string && !fragment_program_string)))
						{
							display_message(ERROR_MESSAGE,
								"If you specify one of vertex_program_string or "
								"fragment_program_string you must specify both.");
							return_code = 0;
						}
						if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
								colour_lookup_green_flag + colour_lookup_red_flag) > 0)
						{
							if (!material_to_be_modified_copy->spectrum)
							{
								display_message(ERROR_MESSAGE,
									"If you specify a colour lookup colour you must also specify a colour_lookup_spectrum.");
								return_code = 0;
							}
							else
							{
								number_of_spectrum_components =
									Spectrum_get_number_of_components(material_to_be_modified_copy->spectrum);
								if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
										colour_lookup_green_flag + colour_lookup_red_flag) ==
									number_of_spectrum_components)
								{
									/* OK */
								}
								else if (1 == number_of_spectrum_components)
								{
									/* Lookup each component specified in the 1D spectra only */
									/* Also OK */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Either your spectrum should have 1 component, or the number of components must match the number of colour lookups specfied (colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag or colour_lookup_red_flag).");
									return_code = 0;
								}
							}
							if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
									colour_lookup_green_flag + colour_lookup_red_flag) > 3)
							{
								display_message(ERROR_MESSAGE,
									"A maximum of three colours (3 of colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag or colour_lookup_red_flag) can be used as input to a colour_lookup_spectrum.");
								return_code = 0;
							}
						}
						else if (material_to_be_modified_copy->spectrum &&
							(!(material_to_be_modified_copy->program) ||
							!(material_to_be_modified_copy->program->type &
								MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_INPUTS)))
						{
							display_message(ERROR_MESSAGE,
								"If you specify a colour_lookup_spectrum you must also specify the input colours. (1 to 3 of colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag and colour_lookup_red_flag)");
							return_code = 0;
						}
						/* Don't check run time availability yet as we may
							not have initialised any openGL display yet */
						if (material_to_be_modified_copy->second_image_texture.texture)
						{
#if defined (GL_VERSION_1_3)
							if (!Graphics_library_tentative_check_extension(GL_VERSION_1_3))
							{
								display_message(ERROR_MESSAGE,
									"Multitexture requires OpenGL version 1.3 or better which is "
									"not available on this display.");
								DEACCESS(Texture)(&material_to_be_modified_copy->second_image_texture.texture);
								return_code = 0;
							}
#else /* defined (GL_VERSION_1_3) */
							display_message(ERROR_MESSAGE,
								"Multitexture requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(Texture)(&material_to_be_modified_copy->second_image_texture.texture);
							return_code = 0;
#endif /* defined (GL_VERSION_1_3) */
						}
						if (material_to_be_modified_copy->spectrum)
						{
#if defined (GL_VERSION_1_3)
							if (!Graphics_library_tentative_check_extension(GL_VERSION_1_3))
							{
								display_message(ERROR_MESSAGE,
									"A colour lookup spectrum requires OpenGL version 1.3 or better which is "
									"not available on this display.");
								DEACCESS(Spectrum)(&material_to_be_modified_copy->spectrum);
								return_code = 0;
							}
#else /* defined (GL_VERSION_1_3) */
							display_message(ERROR_MESSAGE,
								"A colour lookup spectrum requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(Spectrum)(&material_to_be_modified_copy->spectrum);
							return_code = 0;
#endif /* defined (GL_VERSION_1_3) */
						}
						if (normal_mode_flag)
						{
							if (material_to_be_modified_copy->program)
							{
								DEACCESS(Material_program)(&material_to_be_modified_copy->program);
								material_to_be_modified->per_pixel_lighting_flag = 0;
								material_to_be_modified->bump_mapping_flag = 0;
							}
						}
						else if (per_pixel_mode_flag || material_to_be_modified_copy->program)
						{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
							return_code = 0;
#if defined GL_ARB_fragment_program && defined GL_ARB_vertex_program
							if (Graphics_library_tentative_check_extension(GL_ARB_fragment_program) &&
									Graphics_library_tentative_check_extension(GL_ARB_vertex_program))
							{
								return_code = 1;
							}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
#if defined (GL_VERSION_2_0)
							if (Graphics_library_tentative_check_extension(GL_shading_language))
							{
								return_code = 1;
							}
#endif // defined (GL_VERSION_2_0)
							if (return_code)
							{
								if (vertex_program_string && fragment_program_string)
								{
									return_code = Material_set_material_program_strings(
										material_to_be_modified_copy, vertex_program_string,
										fragment_program_string, geometry_program_string);
								}
								else if (material_to_be_modified_copy->program &&
										(0 == material_to_be_modified_copy->program->type))
								{
									/* Do nothing as we just keep the existing program */
								}
								else
								{
									return_code = set_material_program_type(material_to_be_modified_copy,
										bump_mapping_flag, colour_lookup_red_flag,colour_lookup_green_flag,
										colour_lookup_blue_flag, colour_lookup_alpha_flag,
										lit_volume_intensity_normal_texture_flag, lit_volume_finite_difference_normal_flag,
										lit_volume_scale_alpha_flag, return_code);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Program or shader based materials require GL_ARB_vertex_program"
									" and GL_ARB_fragment_program extensions or OpenGL 2.0 shading language support"
									" which is not available on this display.");
							}
#else // defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
							display_message(ERROR_MESSAGE,
								"Program or shader based materials require GL_ARB_vertex_program"
								" and GL_ARB_fragment_program extensions or OpenGL 2.0 shading language support"
								" which was not compiled into this program.");
							return_code = 0;
#endif // defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
						}
						if (uniform_name && number_of_uniform_values && uniform_values)
						{
							Material_program_uniform *uniform;
							if (!material_to_be_modified_copy->program_uniforms)
							{
								material_to_be_modified_copy->program_uniforms = CREATE(LIST(Material_program_uniform))();
							}
							if (!(uniform = FIND_BY_IDENTIFIER_IN_LIST(Material_program_uniform,name)
								(uniform_name, material_to_be_modified_copy->program_uniforms)))
							{
								uniform = CREATE(Material_program_uniform)(uniform_name);
								ADD_OBJECT_TO_LIST(Material_program_uniform)(uniform,
									material_to_be_modified_copy->program_uniforms);
							}
							if (uniform)
							{
								Material_program_uniform_set_float_vector(uniform,
									number_of_uniform_values, uniform_values);
							}
						}
						if (uniform_values)
						{
							DEALLOCATE(uniform_values);
						}
						if (return_code)
						{
							if (material_to_be_modified)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
									material_to_be_modified,material_to_be_modified_copy,
									material_package->material_manager);
								material_copy_bump_mapping_and_per_pixel_lighting_flag(
									 material_to_be_modified_copy, material_to_be_modified);
								DESTROY(Graphical_material)(&material_to_be_modified_copy);
							}
							else
							{
								material_to_be_modified=material_to_be_modified_copy;
							}
						}
						else
						{
							if (material_to_be_modified)
							{
								DESTROY(Graphical_material)(&material_to_be_modified_copy);
							}
						}
					}
					if (vertex_program_string)
					{
						DEALLOCATE(vertex_program_string);
					}
					if (fragment_program_string)
					{
						DEALLOCATE(fragment_program_string);
					}
					if (geometry_program_string)
					{
						DEALLOCATE(geometry_program_string);
					}
					if (uniform_name)
					{
						DEALLOCATE(uniform_name);
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			else
			{
				if (material_void)
				{
					display_message(ERROR_MESSAGE,"Missing material modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing material name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"modify_Graphical_material.  Missing modify_graphical_material_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphical_material */

int list_Graphical_material(struct Graphical_material *material,void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Writes the properties of the <material> to the command window.
==============================================================================*/
{
	char line[80],*name;
	int return_code;

	ENTER(list_Graphical_material);
	USE_PARAMETER(dummy);
	/* check the arguments */
	if (material)
	{
		display_message(INFORMATION_MESSAGE,"material : ");
		display_message(INFORMATION_MESSAGE,material->name);
		display_message(INFORMATION_MESSAGE,"\n");
		sprintf(line,"  access count = %i\n",material->access_count);
		if (material->program)
		{
			if (MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING & material->program->type)
			{
				display_message(INFORMATION_MESSAGE,"  Standard Gouraud Shading (program)\n");
			}
			else if (MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING & material->program->type)
			{
				display_message(INFORMATION_MESSAGE,"  Per Pixel Shading\n");
			}
			else if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material->program->type)
			{
				display_message(INFORMATION_MESSAGE,"  Per Pixel Bump map Shading\n");
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  Standard Gouraud Shading\n");
		}
 		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  diffuse  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->diffuse).red,(material->diffuse).green,
			(material->diffuse).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  ambient  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->ambient).red,(material->ambient).green,
			(material->ambient).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  alpha = %.3g\n",material->alpha);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  emission  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->emission).red,(material->emission).green,
			(material->emission).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  specular  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->specular).red,(material->specular).green,
			(material->specular).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  shininess = %.3g\n",material->shininess);
		display_message(INFORMATION_MESSAGE,line);
		if (material->image_texture.texture&&GET_NAME(Texture)(material->image_texture.texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  texture : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		if (material->second_image_texture.texture&&GET_NAME(Texture)(material->second_image_texture.texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  second texture : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		if (material->third_image_texture.texture&&GET_NAME(Texture)(material->third_image_texture.texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  third texture : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		if (material->fourth_image_texture.texture&&GET_NAME(Texture)(material->fourth_image_texture.texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  fourth texture : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		if (material->spectrum&&GET_NAME(Spectrum)(material->spectrum,&name))
		{
			display_message(INFORMATION_MESSAGE, "  colour lookup spectrum : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphical_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphical_material */

int list_Graphical_material_commands(struct Graphical_material *material,
	 void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 15 August 2007

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/
{
	char *command_prefix,line[100],*name;
	int return_code;

	ENTER(list_Graphical_material_commands);
	if (material&&(command_prefix=(char *)command_prefix_void))
	{
		display_message(INFORMATION_MESSAGE,command_prefix);
		if (name=duplicate_string(material->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,name);
			DEALLOCATE(name);
		}
		if (material->program)
		{
			if (MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING & material->program->type)
			{
				display_message(INFORMATION_MESSAGE," normal_mode");
			}
			else if (MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING & material->program->type)
			{
				display_message(INFORMATION_MESSAGE," per_pixel_mode");
			}
			else if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material->program->type)
			{
				display_message(INFORMATION_MESSAGE," per_pixel_mode bump_mapping");
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE," normal_mode");
		}
		sprintf(line," ambient %g %g %g",
			(material->ambient).red,(material->ambient).green,
			(material->ambient).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," diffuse %g %g %g",
			(material->diffuse).red,(material->diffuse).green,
			(material->diffuse).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," emission %g %g %g",
			(material->emission).red,(material->emission).green,
			(material->emission).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," specular %g %g %g",
			(material->specular).red,(material->specular).green,
			(material->specular).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," alpha %g",material->alpha);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," shininess %g",material->shininess);
		display_message(INFORMATION_MESSAGE,line);
		if (material->image_texture.texture&&GET_NAME(Texture)(material->image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," texture %s",name);
			DEALLOCATE(name);
		}
		if (material->second_image_texture.texture&&GET_NAME(Texture)(material->second_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," secondary_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->third_image_texture.texture&&GET_NAME(Texture)(material->third_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," third_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->fourth_image_texture.texture&&GET_NAME(Texture)(material->fourth_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," fourth_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->spectrum&&GET_NAME(Spectrum)(material->spectrum,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," colour_lookup_spectrum %s",name);
			DEALLOCATE(name);
		}
		display_message(INFORMATION_MESSAGE,";\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphical_material_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphical_material_commands */

int write_Graphical_material_commands_to_comfile(struct Graphical_material *material,
	 void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 15 August 2007

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/
{
	char *command_prefix,line[100],*name;
	int return_code;

	ENTER(write_Graphical_material_commands_to_comfile);
	if (material&&(command_prefix=(char *)command_prefix_void))
	{
		 write_message_to_file(INFORMATION_MESSAGE,command_prefix);
		if (name=duplicate_string(material->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE,name);
			DEALLOCATE(name);
		}
		if (material->program)
		{
			if (MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING & material->program->type)
			{
				 write_message_to_file(INFORMATION_MESSAGE," normal_mode");
			}
			else if (MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING & material->program->type)
			{
				write_message_to_file(INFORMATION_MESSAGE," per_pixel_mode");
			}
			else if (MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP & material->program->type)
			{
				write_message_to_file(INFORMATION_MESSAGE," per_pixel_mode bump_mapping");
			}
		}
		else
		{
			 write_message_to_file(INFORMATION_MESSAGE," normal_mode");
		}
		sprintf(line," ambient %g %g %g",
			(material->ambient).red,(material->ambient).green,
			(material->ambient).blue);
		write_message_to_file(INFORMATION_MESSAGE,line);
		sprintf(line," diffuse %g %g %g",
			(material->diffuse).red,(material->diffuse).green,
			(material->diffuse).blue);
		write_message_to_file(INFORMATION_MESSAGE,line);
		sprintf(line," emission %g %g %g",
			(material->emission).red,(material->emission).green,
			(material->emission).blue);
		write_message_to_file(INFORMATION_MESSAGE,line);
		sprintf(line," specular %g %g %g",
			(material->specular).red,(material->specular).green,
			(material->specular).blue);
		write_message_to_file	(INFORMATION_MESSAGE,line);
		sprintf(line," alpha %g",material->alpha);
		write_message_to_file	(INFORMATION_MESSAGE,line);
		sprintf(line," shininess %g",material->shininess);
		write_message_to_file(INFORMATION_MESSAGE,line);
		if (material->image_texture.texture&&GET_NAME(Texture)(material->image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE," texture %s",name);
			DEALLOCATE(name);
		}
		if (material->second_image_texture.texture&&GET_NAME(Texture)(material->second_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE," secondary_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->third_image_texture.texture&&GET_NAME(Texture)(material->third_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE," third_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->fourth_image_texture.texture&&GET_NAME(Texture)(material->fourth_image_texture.texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE," fourth_texture %s",name);
			DEALLOCATE(name);
		}
		if (material->spectrum&&GET_NAME(Spectrum)(material->spectrum,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE," colour_lookup_spectrum %s",name);
			DEALLOCATE(name);
		}
		write_message_to_file(INFORMATION_MESSAGE,";\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphical_material_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_Graphical_material_commands_to_comfile */


/* int list_Graphical_material_commands(struct Graphical_material *material, */
/* 	void *command_prefix_void) */
/* ****************************************************************************** */
/* LAST MODIFIED : 15 August 2007 */

/* DESCRIPTION : */
/* List on the command window the command needed to recreate the <material>. */
/* The command is started with the string pointed to by <command_prefix>. */
/* ==============================================================================*/
/* { */
/* 	 int return_code; */

/* 	 ENTER(list_Graphical_material_commands); */
/* 	 Process_list_command_class *list_message = */
/* 			new Process_list_command_class(); */
/* 	 return_code = process_list_or_write_Graphical_material_commands( */
/* 			material,command_prefix_void, list_message); */
/* 	 LEAVE; */

/* 	return (return_code); */
/* }  write_Graphical_material_commands_to_comfile */

/* int write_Graphical_material_commands_to_comfile(struct Graphical_material *material, */
/* 	void *command_prefix_void) */
/* ****************************************************************************** */
/* LAST MODIFIED : 15 August 2007 */

/* DESCRIPTION : */
/* Writes on the command window the command needed to recreate the <material>. */
/* The command is started with the string pointed to by <command_prefix>. */
/* ==============================================================================*/
/* { */
/* 	 int return_code; */

/* 	 ENTER(write_Graphical_material_commands_to_comfile); */
/* 	 Process_write_command_class *write_message = */
/* 			new Process_write_command_class(); */
/* 	 return_code = process_list_or_write_Graphical_material_commands( */
/* 			material,command_prefix_void, write_message); */
/* 	 LEAVE; */

/* 	 return (return_code); */
/* } write_Graphical_material_commands_to_comfile */

int file_read_Graphical_material_name(struct IO_stream *stream,
	struct Graphical_material **material_address,
	struct MANAGER(Graphical_material) *graphical_material_manager)
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a material name from a <file>.  Searchs the list of all materials for one
with the specified name.  If one is not found a new one is created with the
specified name and the default properties.
==============================================================================*/
{
	char *material_name;
	int return_code;
	struct Graphical_material *material;

	ENTER(file_read_Graphical_material_name);
	/* check the arguments */
	if (stream&&material_address)
	{
		if (IO_stream_read_string(stream,"s",&material_name))
		{
			/*???DB.  Should this read function be in another module ? */
			/* either find an existing material of that name, use no material if the
				 name is "none", or make a material of the given name */
			if ((material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
				material_name,graphical_material_manager))||
				fuzzy_string_compare_same_length(material_name,"NONE"))
			{
				*material_address=material;
				return_code=1;
			}
			else
			{
				if (material=CREATE(Graphical_material)(material_name))
				{
					Cmiss_material_set_attribute_integer(material, CMISS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);
					if (ADD_OBJECT_TO_MANAGER(Graphical_material)(material,
						graphical_material_manager))
					{
						*material_address=material;
						return_code=1;
					}
					else
					{
						DESTROY(Graphical_material)(&material);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_read_Graphical_material_name.  Could not create material");
					return_code=0;
				}
			}
			DEALLOCATE(material_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"file_read_Graphical_material_name.  Error reading material name strin");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_read_Graphical_material_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_Graphical_material_name */

#if defined (OPENGL_API)
int Material_compile_members_opengl(Graphical_material *material,
	Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(Material_compile_members_opengl);
	if (material)
	{
		return_code = 1;
		if (GRAPHICS_COMPILED != material->compile_status)
		{
			/* must compile texture before opening material display list */
			if (material->image_texture.texture)
			{
				renderer->Texture_compile(material->image_texture.texture);
			}
#if defined (GL_VERSION_1_3)
			if (Graphics_library_check_extension(GL_VERSION_1_3))
			{
				/* It probably isn't necessary to activate the texture
					unit only to compile the texture, but seems a discipline
					that may help avoid graphics driver problems? */
				if (material->second_image_texture.texture)
				{
					glActiveTexture(GL_TEXTURE1);
					renderer->allow_texture_tiling = 0;
					renderer->Texture_compile(material->second_image_texture.texture);
					glActiveTexture(GL_TEXTURE0);
				}
				if (material->third_image_texture.texture)
				{
					glActiveTexture(GL_TEXTURE2);
					renderer->allow_texture_tiling = 0;
					renderer->Texture_compile(material->third_image_texture.texture);
					glActiveTexture(GL_TEXTURE0);
				}
				if (material->fourth_image_texture.texture)
				{
					glActiveTexture(GL_TEXTURE3);
					renderer->allow_texture_tiling = 0;
					renderer->Texture_compile(material->fourth_image_texture.texture);
					glActiveTexture(GL_TEXTURE0);
				}
				if (material->spectrum)
				{
					glActiveTexture(GL_TEXTURE1);
					Spectrum_compile_colour_lookup(material->spectrum,
						renderer);
					glActiveTexture(GL_TEXTURE0);
				}
			}
#endif /* defined (GL_VERSION_1_3) */

			if (material->program)
			{
				Material_program_compile(material->program);
			}
		}
		else
		{
			if (renderer->allow_texture_tiling && material->image_texture.texture)
			{
				renderer->Texture_compile(material->image_texture.texture);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_compile_members_opengl.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Material_compile_members_opengl */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int Material_compile_opengl_display_list(Graphical_material *material,
	Callback_base< Graphical_material* > *execute_function,
	Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(Material_compile_opengl_display_list);
	USE_PARAMETER(renderer);
	if (material)
	{
		return_code = 1;
		if (GRAPHICS_NOT_COMPILED == material->compile_status)
		{
			if (material->display_list || (material->display_list = glGenLists(1)))
			{
				glNewList(material->display_list,GL_COMPILE);
				return_code = (*execute_function)(material);
				glEndList();
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Material_compile_opengl_display_list.  Could not generate display list");
				return_code = 0;
			}
		}
		if (return_code)
		{
			material->compile_status = GRAPHICS_COMPILED;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_compile_opengl_display_list.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Material_compile_opengl_display_list */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int compile_Graphical_material_for_order_independent_transparency(
	struct Graphical_material *material,
	void *material_order_independent_data_void)
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Recompile each of the <materials> which have already been compiled so that they
will work with order_independent_transparency.
==============================================================================*/
{
	int return_code;
	int modified_type;
	int dimension;
	struct Material_order_independent_transparency *data;
	struct Material_package *material_package;
	struct Material_program *unmodified_program;

	ENTER(compile_Graphical_material_for_order_independent_transparency);

	if (material && (data = (struct Material_order_independent_transparency *)
			material_order_independent_data_void))
	{
		return_code = 1;
		material_package = material->package;
		/* Only do the materials that have been compiled already as the scene
			is compiled so presumably uncompiled materials are not used. */
		if ((GRAPHICS_COMPILED == material->compile_status) &&
			material->display_list)
		{
			unmodified_program = material->program;
			if (material->program)
			{
				modified_type = material->program->type;
			}
			else
			{
				modified_type = MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING;
				if (material->image_texture.texture)
				{
					/* Texture should have already been compiled when the
						scene was compiled before rendering. */
					if (material->image_texture.texture)
					{
						data->renderer->Texture_compile(material->image_texture.texture);
					}
					Texture_get_dimension(material->image_texture.texture, &dimension);
					switch (dimension)
					{
						case 1:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1;
						} break;
						case 2:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
						} break;
						case 3:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 |
								MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "Colour texture dimension %d not supported.",
								dimension);
							return_code = 0;
						} break;
					}
					switch (Texture_get_number_of_components(material->image_texture.texture))
					{
						case 1:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1;
						} break;
						case 2:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2;
						} break;
						case 3:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 |
								MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2;
						} break;
						case 4:
						{
							/* Do nothing as zero in these bits indicates rgba */
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "Colour texture output dimension not supported.");
							return_code = 0;
						} break;
					}
					switch (Texture_get_combine_mode(material->image_texture.texture))
					{
						case TEXTURE_DECAL:
						{
							modified_type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL;
						}
						default:
						{
							/* Do nothing as modulate is the default */
						}
					}
				}
			}

			if (data->layer == 1)
			{
				/* The first layer does not peel */
				modified_type |= MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_FIRST_LAYER;
			}
			else if (data->layer > 1)
			{
				/* The rest of the layers should peel */
				modified_type |= MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER;
			}
			/*
			else
			{
			      Reset the material to its original state.  Could
					try to avoid this compile if we are about to render
					with order_independent_transparency again but need more
					compilation states then.
			} */

			if (modified_type != MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING)
			{
				if (!(material->program = FIND_BY_IDENTIFIER_IN_LIST(
					Material_program,type)((Material_program_type)modified_type,
						material_package->material_program_list)))
				{
					if (material->program = ACCESS(Material_program)(
						CREATE(Material_program)((Material_program_type)modified_type)))
					{
						ADD_OBJECT_TO_LIST(Material_program)(material->program,
							material_package->material_program_list);
					}
					else
					{
						return_code = 0;
					}
				}
				if (!material->program->compiled)
				{
					Material_program_compile(material->program);
				}
			}

			glNewList(material->display_list,GL_COMPILE);
			if (material->program && 
				material->program->shader_type == MATERIAL_PROGRAM_SHADER_ARB)
			{
				 if (material->image_texture.texture)
				 {
						Texture_execute_vertex_program_environment(material->image_texture.texture,
							 0);
				 }
			}
			direct_render_Graphical_material(material,data->renderer);
			if (material->program && 
				material->program->shader_type == MATERIAL_PROGRAM_SHADER_GLSL)
			{
				 GLint loc1;
				 if (data && data->renderer && data->renderer->graphics_buffer)
				 {
						if (glIsProgram(material->program->glsl_current_program))
						{
							 loc1 = glGetUniformLocation((GLuint)material->program->glsl_current_program,"texturesize");
							 if (loc1>-1)
							 {
								 glUniform4f(loc1,Graphics_buffer_get_width(data->renderer->graphics_buffer),
									 Graphics_buffer_get_height(data->renderer->graphics_buffer), 1.0, 1.0);
							 }
							 loc1 = glGetUniformLocation(material->program->glsl_current_program,"samplertex");
							 if (loc1 != (GLint)-1)
							 {
									glUniform1i(loc1, 3);
							 }
						}
				 }
			}
			glEndList();
			material->program = unmodified_program;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Graphical_material_for_order_independent_transparency.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Graphical_material_for_order_independent_transparency */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int Material_render_opengl(Graphical_material *material,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 19 November 2007

DESCRIPTION :
Rebuilds the display_list for <material> if it is not current. If <material>
does not have a display list, first attempts to give it one. The display list
created here may be called using execute_Graphical_material, below.
If <texture_tiling> is not NULL then if the material uses a primary texture
and this texture is larger than can be compiled into a single texture on
the current graphics hardware, then it can be tiled into several textures
and information about the tile boundaries is returned in Texture_tiling
structure and should be used to compile any graphics objects.
???RC Graphical_materials must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Graphical_material should just call direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(Material_render_opengl);
	if (material)
	{
		if (material->program)
		{
			/* Load up the texture scaling into the vertex program
				environment and the texel size into the fragment
				program environment. */
			if (material->image_texture.texture)
			{
				Texture_execute_vertex_program_environment(material->image_texture.texture,
					0);
			}
			else if (material->second_image_texture.texture)
			{
				Texture_execute_vertex_program_environment(material->second_image_texture.texture,
					0);
			}
			else if (material->third_image_texture.texture)
			{
				Texture_execute_vertex_program_environment(material->third_image_texture.texture,
					0);
			}
			else if (material->fourth_image_texture.texture)
			{
				Texture_execute_vertex_program_environment(material->fourth_image_texture.texture,
					0);
			}
		}
#if defined GL_ARB_fragment_program || defined GL_VERSION_2_0
		if (material->spectrum && (
			material->program->shader_type == MATERIAL_PROGRAM_SHADER_ARB ||
			material->program->shader_type == MATERIAL_PROGRAM_SHADER_GLSL))
		{
			int i, lookup_dimensions, *lookup_sizes;
			float values[4];

			Spectrum_get_colour_lookup_sizes(material->spectrum,
				&lookup_dimensions, &lookup_sizes);
			/* Set the offsets = 0.5 / size */
			for (i = 0 ; i < lookup_dimensions ; i++)
			{
				values[i] = 0.5 / ((double)lookup_sizes[i]);
			}
			for (; i < 4 ; i++)
			{
				values[i] = 0.0;
			}
			if (material->program->shader_type == MATERIAL_PROGRAM_SHADER_ARB)
			{
				glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1,
					values[0], values[1], values[2], values[3]);
			}
			else
			{
				GLint loc1=-1;
				if (glIsProgram(material->program->glsl_current_program))
				{
					loc1 = glGetUniformLocation(material->program->glsl_current_program,"lookup_offsets");
					if (loc1 != (GLint)-1)
					{
						glUniform4f(loc1, values[0], values[1],
							values[2], values[3]);
					}
				}
			}
			/* Set the scales = (size - 1) / (size) */
			for (i = 0 ; i < lookup_dimensions ; i++)
			{
				values[i] = ((double)(lookup_sizes[i] - 1)) / ((double)lookup_sizes[i]);
			}
			for (; i < 4 ; i++)
			{
				values[i] = 1.0;
			}
			if (material->program->shader_type == MATERIAL_PROGRAM_SHADER_ARB)
			{
				glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2,
					values[0], values[1], values[2], values[3]);
			}
			else
			{
				GLint loc2=-1;
				if (glIsProgram(material->program->glsl_current_program))
				{
					loc2 = glGetUniformLocation(material->program->glsl_current_program,"lookup_scales");
					if (loc2 != (GLint)-1)
					{
						glUniform4f(loc2, values[0], values[1],
							values[2], values[3]);
					}
				}
			}
			DEALLOCATE(lookup_sizes);
		}
#endif /* defined GL_ARB_fragment_program */
		return_code = direct_render_Graphical_material(material, renderer);
	}
	else
	{
		return_code = renderer->Texture_execute((struct Texture *)NULL);
	}
	LEAVE;

	return (return_code);
} /* Material_render_opengl */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int Material_execute_opengl_display_list(Graphical_material *material,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
Activates <material> by calling its display list. If the display list is not
current, an error is reported.
Passing a NULL material will deactivate any textures or material parameters
that get set up with materials.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(execute_Graphical_material);
	if (material)
	{
		if (GRAPHICS_COMPILED == material->compile_status)
		{
			glCallList(material->display_list);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_Graphical_material.  Display list not current");
			return_code = 0;
		}
	}
	else
	{
#if defined (GL_VERSION_2_0)
		if (Graphics_library_check_extension(GL_shading_language))
		{
			glUseProgram(0);
			glDisable(GL_VERTEX_PROGRAM_TWO_SIDE);
		}
#endif
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
		if (Graphics_library_check_extension(GL_ARB_vertex_program) &&
			Graphics_library_check_extension(GL_ARB_fragment_program))
		{
			glDisable(GL_VERTEX_PROGRAM_ARB);
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
			glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
		}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
		return_code = renderer->Texture_execute((struct Texture *)NULL);
	}
	LEAVE;

	return (return_code);
} /* execute_Graphical_material */
#endif /* defined (OPENGL_API) */

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the material from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Graphical_material *temp_material,**material_address;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(set_Graphical_material);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((material_address=
					(struct Graphical_material **)material_address_void)&&
					(graphical_material_manager=(struct MANAGER(Graphical_material) *)
					graphical_material_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*material_address)
						{
							DEACCESS(Graphical_material)(material_address);
							*material_address=(struct Graphical_material *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,
							name)(current_token,graphical_material_manager))
						{
							if (*material_address!=temp_material)
							{
								DEACCESS(Graphical_material)(material_address);
								*material_address=ACCESS(Graphical_material)(temp_material);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Graphical_material.  Invalid argument(s).  material_address %p.  material_manager %p",
						material_address_void,graphical_material_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MATERIAL_NAME|none");
				/* if possible, then write the name */
				if (material_address=
					(struct Graphical_material **)material_address_void)
				{
					if (temp_material= *material_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_material->name);
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
			display_message(WARNING_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphical_material */

int Option_table_add_set_Material_entry(
	struct Option_table *option_table, const char *token,
	struct Graphical_material **material, struct Material_package *material_package)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <material> is selected from
the <material_package> by name.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_double_vector_with_help_entry);
	if (option_table && token)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)material,
			(void *)material_package->material_manager, set_Graphical_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_double_vector_with_help_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_double_vector_with_help_entry */

int Cmiss_material_set_texture(
	Graphical_material *material, Texture *texture)
{
	int return_code = 0;

	ENTER(Cmiss_material_set_texture);
	if (material)
	{
		REACCESS(Texture)(&(material->image_texture.texture), texture);
		if (material->manager)
		{
			Graphical_material_changed(material);
			return_code = 1;
		}
	}
	LEAVE;

	return return_code;
}

struct Graphical_material *Cmiss_material_access(struct Graphical_material *material)
{
	ENTER(Cmiss_material_access);
	if (material)
	{
		material->access_count++;
	}

	return material;
}

int Cmiss_material_destroy(Graphical_material **material_address)
{
	int return_code = 0;
	struct Graphical_material *material;

	ENTER(Cmiss_material_destroy);
	if (material_address && (material = *material_address))
	{
		(material->access_count)--;
		if (material->access_count <= 0)
		{
			return_code = DESTROY(Graphical_material)(material_address);
		}
		else if ((!material->is_managed_flag) && (material->manager) &&
			((1 == material->access_count) || ((2 == material->access_count) &&
				(MANAGER_CHANGE_NONE(Graphical_material) != material->manager_change_status))))
		{
			return_code = REMOVE_OBJECT_FROM_MANAGER(Graphical_material)(material, material->manager);
		}
		else
		{
			return_code = 1;
		}
		*material_address = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return return_code;
}

int Cmiss_material_get_attribute_integer(Cmiss_material_id material,
	enum Cmiss_material_attribute_id attribute_id)
{
	int value = 0;
	if (material)
	{
		switch (attribute_id)
		{
		case CMISS_MATERIAL_ATTRIBUTE_IS_MANAGED:
			value = (int)material->is_managed_flag;
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_material_get_attribute_integer.  Invalid attribute");
			break;
		}
	}
	return value;
}

int Cmiss_material_set_attribute_integer(Cmiss_material_id material,
	enum Cmiss_material_attribute_id attribute_id, int value)
{
	int return_code = 0;
	if (material)
	{
		return_code = 1;
		int old_value = Cmiss_material_get_attribute_integer(material, attribute_id);
		enum MANAGER_CHANGE(Graphical_material) change =
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Graphical_material);
		switch (attribute_id)
		{
		case CMISS_MATERIAL_ATTRIBUTE_IS_MANAGED:
			material->is_managed_flag = (value != 0);
			change = MANAGER_CHANGE_NOT_RESULT(Graphical_material);
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_material_set_attribute_integer.  Invalid attribute");
			return_code = 0;
			break;
		}
		if (Cmiss_material_get_attribute_integer(material, attribute_id) != old_value)
		{
			MANAGED_OBJECT_CHANGE(Graphical_material)(material, change);
		}
	}
	return return_code;
}

int Cmiss_material_set_name(
	Graphical_material *material, const char *name)
{
	int return_code = 0;

	ENTER(Cmiss_material_set_name);
	if (material && material->manager && name)
	{
		return_code = MANAGER_MODIFY_IDENTIFIER(Graphical_material, name)
			(material, name, material->manager);
	}
	LEAVE;

	return return_code;
}

int Cmiss_material_execute_command(struct Graphical_material *material, const char *command_string)
{
	int return_code = 0;
	ENTER(Cmiss_material_execute_command);
	if (material && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		if (state)
		{
			struct Cmiss_graphics_module *graphics_module =
				MANAGER_GET_OWNER(Graphical_material)(material->manager);
			if(graphics_module)
			{
				struct Material_package *package =
					Cmiss_graphics_module_get_material_package(graphics_module);
				if (package)
				{
					return_code=modify_Graphical_material(state,(void *)material,
						(void *)package);
					DEACCESS(Material_package)(&package);
				}
			}
			destroy_Parse_state(&state);
		}
	}

	return return_code;
}

