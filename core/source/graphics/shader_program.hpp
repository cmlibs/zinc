#if !defined (SHADER_PROGRAM_H)
#define SHADER_PROGRAM_H

#include "opencmiss/zinc/zincconfigure.h"
#include "opencmiss/zinc/shader.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"


struct Render_graphics_opengl;
struct Texture;

enum Shader_program_type
/*****************************************************************************//**
@date LAST MODIFIED : 4 July 2007

Enumerates the main different types of vertex/fragment program for materials
==============================================================================*/
{
	/* This type is for Shader_programs which have arbitrary specified strings
	 * rather than the program being generated based on this type value.
	 */
	SHADER_PROGRAM_SPECIFIED_STRINGS = 0,
	/* This first one is a standard Gouraud Shaded material, included here so
		that it can be peeled in order independent transparency */
	SHADER_PROGRAM_GOURAUD_SHADING = 1,
	SHADER_PROGRAM_PER_PIXEL_LIGHTING = 2,
	SHADER_PROGRAM_PER_PIXEL_TEXTURING = 10,
	SHADER_PROGRAM_BUMP_MAPPING = 770,
	SHADER_PROGRAM_BUMP_MAPPING_TEXTURING = 778,

	/* These classes modify the above programs and so must be bit independent */
	SHADER_PROGRAM_CLASS_GOURAUD_SHADING = 1,
	SHADER_PROGRAM_CLASS_PER_PIXEL_LIGHTING = 2,
	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a colour texture. */
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE_1 = (1<<2),
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE_2 = (1<<3),
	/* If either bit is set then we are using a texture */
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE = 12,
	/* If this bit is set then a colour texture will replace the lighting calculation for a colour,
		if it is not set then the texture will modulate the lighting calculation colour */
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL = (1<<4),
	/* Specifies the output dimension of the texture and therefore how it is applied.
		OUTPUT1 = grayscale, OUTPUT2 = grayscale and alpha, OUTPUT1 & OUTPUT2 = rgb
		!OUPUT1 & !OUTPUT2 = rgba. */
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 = (1<<5),
	SHADER_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 = (1<<6),

	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a second or bump map texture. */
	SHADER_PROGRAM_CLASS_SECOND_TEXTURE_1 = (1<<7),
	SHADER_PROGRAM_CLASS_SECOND_TEXTURE_2 = (1<<8),
	SHADER_PROGRAM_CLASS_SECOND_TEXTURE = 384,
   /* Specifies that the second texture is intended to be used as a bump map texture, modulating
		the per pixel value of the normal in the lighting calculation */
	SHADER_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP = (1<<9),

	/* The colour value is used as the input, derived from the primary texture or the lighting.
	   Specify which input components are used directly from the input source,
		at most 3 of the input values can be used (a 3D texture lookup) */
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 = (1<<10),
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 = (1<<11),
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 = (1<<12),
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_4 = (1<<13),
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_INPUTS = (1<<10) + (1<<11) + (1<<12) + (1<<13),

	/* Specify the outputs in the dependent texture lookup, either replacing the colour, alpha or both. */
	/* These modes work by assuming that the dependent_texture inputs form
		the axes of a single 1D, 2D or 3D texture */
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR = (1<<14),
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA = (1<<15),
	/* This mode works by looking up each input component independently
		in a common 1D texture */
	SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP = (1<<16),

	/* Assume that the texture contains an intensity followed by a 3 component
		normal vector.  This vector is used to light volume rendering by
		performing per pixel lighting using this normal. */
	SHADER_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE = (1<<17),
	/* Calculate a normal by using a finite difference operator. */
	SHADER_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL = (1<<18),
	/* Scale the alpha by the magnitude of the normal */
	SHADER_PROGRAM_CLASS_LIT_VOLUME_SCALE_ALPHA = (1<<19),

   /* Order independent transparency passes */
	SHADER_PROGRAM_CLASS_ORDER_INDEPENDENT_FIRST_LAYER = (1<<20),
	SHADER_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER = (1<<21)
}; /* enum Shader_program_type */

enum Shader_program_shader_type
{
	SHADER_PROGRAM_SHADER_NONE,
	SHADER_PROGRAM_SHADER_ARB,
	SHADER_PROGRAM_SHADER_GLSL
};



/*****************************************************************************//**
@date LAST MODIFIED : 20 June 2008

Stores a display list which sets up the correct state for a particular
material state.  This allows vertex/fragment programs to be used per material
but shared between different materials with the same state.
==============================================================================*/
struct Shader_program
{
	/*! Specifies the type of the Material Program
	 * These should be unique for each differing program as the materials
	 * will only generate one program for each value of type.
	 * As a special case, type == 0, specifies a predefined arbitrary string
	 * is used and so these should never be shared. */
	enum Shader_program_type type;
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
	enum Shader_program_shader_type shader_type;
	/*! Display list which enables the correct state for this program */
	GLuint display_list;
#endif /* defined (OPENGL_API) */

	/*! Flag indicating whether the program is compiled or not */
	int compiled;

	int access_count;
}; /* struct Shader_program */

PROTOTYPE_OBJECT_FUNCTIONS(Shader_program);

DECLARE_LIST_TYPES(Shader_program);
PROTOTYPE_LIST_FUNCTIONS(Shader_program);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Shader_program,type,Shader_program_type);

struct Shader_program *CREATE(Shader_program)(enum Shader_program_type type);

/***************************************************************************//**
 * An alternative Shader_program constructor that takes explicit program
 * strings.
 */
struct Shader_program *Shader_program_create_from_program_strings(
	const char *vertex_program_string, const char *fragment_program_string,
	const char *geometry_program_string);

#if defined (OPENGL_API)
int Shader_program_compile(struct Shader_program *shader_program,
	Render_graphics_opengl *renderer);

int Shader_program_execute(struct Shader_program *shader_program,
	Render_graphics_opengl *renderer);

int Shader_program_execute_textures(struct Shader_program *shader_program,
	struct Texture *texture, struct Texture *second_texture,
	struct Texture *third_texture);

int Shader_program_execute_uniforms(struct Shader_program *program,
	cmzn_shaderuniforms_id uniforms);
#endif

int cmzn_shader_program_destroy(struct Shader_program **shader_program_address);

struct Shader_program *cmzn_shader_program_access(struct Shader_program *shader_program);

int Shader_program_set_vertex_string(Shader_program *shader_program_to_be_modified,
	const char *vertex_program_string);

int Shader_program_set_fragment_string(Shader_program *shader_program_to_be_modified,
	const char *fragment_program_string);

int Shader_program_set_geometry_string(Shader_program *shader_program_to_be_modified,
	const char *geometry_program_string);

#endif
