/*******************************************************************************
FILE : material.c

LAST MODIFIED : 5 August 2002

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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "command/parser.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
#define MATERIAL_SPECTRUM_ENABLED  (1)
#define MATERIAL_SPECTRUM_STARTED  (2)
#define MATERIAL_SPECTRUM_DIFFUSE  (4)
#define MATERIAL_SPECTRUM_AMBIENT  (8)
#define MATERIAL_SPECTRUM_SPECULAR (16)
#define MATERIAL_SPECTRUM_EMISSION (32)
#define MATERIAL_SPECTRUM_ALPHA    (64)

/*
Module types
------------
*/
enum Material_program_type
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Enumerates the main different types of vertex/fragment program for materials
==============================================================================*/
{
	MATERIAL_PROGRAM_PER_PIXEL_LIGHTING
}; /* enum Material_program_type */

struct Material_program
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Stores a display list which sets up the correct state for a particular
material state.  This allows vertex/fragment programs to be used per material
but shared between different materials with the same state.
==============================================================================*/
{
	enum Material_program_type type;

#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
	GLuint vertex_program;
	GLuint fragment_program;
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */

	/* Flag indicating whether the program is compiled or not */
	int compiled;
	/* Display list which enables the correct state for this program */
	GLuint display_list;

	int access_count;
}; /* struct Material_program */

DECLARE_LIST_TYPES(Material_program);
PROTOTYPE_LIST_FUNCTIONS(Material_program);
FULL_DECLARE_INDEXED_LIST_TYPE(Material_program);

struct Graphical_material
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
The properties of a material.
==============================================================================*/
{
	/* the name of the material */
	char *name;
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
#endif /* defined (OPENGL_API) */
	/* the spectrum_flag indicates to the direct render routine that
		the material is being used to represent a spectrum and so the
		commands generated from direct_render_Material can be 
		compacted, all the bits indicate what has been changed */
	int spectrum_flag;
	int spectrum_flag_previous;
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the texture for this material */
	struct Texture *texture;
	/* the shared information for Graphical Materials, allowing them to share
	   Material_programs */
	struct Material_package *package;
	/* the graphics state program that represents this material */
	struct Material_program *program;
	int access_count;
}; /* struct Graphical_material */

FULL_DECLARE_INDEXED_LIST_TYPE(Graphical_material);

FULL_DECLARE_MANAGER_TYPE(Graphical_material);

struct Material_package
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Provide an opaque container for shared material information.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *material_manager;
	struct MANAGER(Texture) *texture_manager;
	struct Graphical_material *default_material;
	struct Graphical_material *default_selected_material;
	struct LIST(Material_program) *material_program_list;
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
		material_program->vertex_program = 0;
		material_program->fragment_program = 0;
		material_program->display_list = 0;
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
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
			if (material_program->vertex_program)
			{
				glDeleteProgramsARB(1, &material_program->vertex_program);
			}
			if (material_program->fragment_program)
			{
				glDeleteProgramsARB(1, &material_program->fragment_program);
			}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
			if (material_program->display_list)
			{
				glDeleteLists(material_program->display_list, 1);
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

static int Material_program_compile(struct Material_program *material_program)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Material_program_compile);
	if (material_program)
	{
		return_code = 1;
		if (!material_program->compiled)
		{
			switch (material_program->type)
			{
				case MATERIAL_PROGRAM_PER_PIXEL_LIGHTING:
				{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
					if (query_gl_extension("GL_ARB_vertex_program") &&
						 query_gl_extension("GL_ARB_fragment_program"))
					{
						char vertex_program_string[] =
							"!!ARBvp1.0\n"
							"ATTRIB normal = vertex.normal;\n"
							"ATTRIB position = vertex.position;\n"
							"PARAM c0[4] = { state.matrix.mvp };\n"
							"PARAM c1[4] = { state.matrix.modelview };\n"
							"PARAM c2[4] = { state.matrix.modelview.invtrans };\n"
							"PARAM eyeCameraPos = {0, 0, 100, 0};\n"
							"PARAM eyeLightPos = state.light[0].position;\n"
							"TEMP eyeVertex;\n"
							"TEMP eyeNormal;\n"
							"\n"
							"#Vertex position in eyespace\n"
							"DP4 eyeVertex.x, c1[0], position;\n"
							"DP4 eyeVertex.y, c1[1], position;\n"
							"DP4 eyeVertex.z, c1[2], position;\n"
							"\n"
							"DP4 eyeNormal.x, c2[0], normal;\n"
							"DP4 eyeNormal.y, c2[1], normal;\n"
							"DP4 eyeNormal.z, c2[2], normal;\n"
							"DP3 eyeNormal.w, eyeNormal, eyeNormal;\n"
							"RSQ eyeNormal.w, eyeNormal.w;\n"
							"MUL eyeNormal.xyz, eyeNormal.w, eyeNormal;\n"
							"#MOV eyeNormal, normal;\n"
							"\n"
							"MOV result.texcoord[0], vertex.texcoord[0];\n"
							"SUB result.texcoord[1], eyeLightPos, eyeVertex;\n"
							"SUB result.texcoord[2], eyeCameraPos, eyeVertex;\n"
							"MOV result.texcoord[3], eyeNormal;\n"
							"MOV result.color, vertex.color;\n"
							"MOV result.color.back, vertex.color;\n"
							"MOV result.color.secondary,  {1, 1, 1, 1};\n"
							"MOV result.color.back.secondary,  {0, 0, 0, 0};\n"
							"DP4 result.position.x, c0[0], position;\n"
							"DP4 result.position.y, c0[1], position;\n"
							"DP4 result.position.z, c0[2], position;\n"
							"DP4 result.position.w, c0[3], position;\n"
							"\n"
							"END\n";
						char fragment_program_string[] =
							"!!ARBfp1.0\n"
							"TEMP lightVec, viewVec, reflVec, normal, attenuation, Len, finalCol, lightContrib, reverse, tex;\n"
							"PARAM two = {2.0, 2.0, 2.0, 2.0};\n"
							"PARAM m_one = {-1.0, -1.0, -1.0, -1.0};\n"
							"\n"
							"TEX		tex, fragment.texcoord[0], texture[0], 2D;\n"
							"\n"
							"#Normalize normal, lightvec and viewvec.\n"
							"DP3		Len.w, fragment.texcoord[1], fragment.texcoord[1];\n"
							"RSQ		lightVec.w, Len.w;\n"
							"MUL		lightVec.xyz, fragment.texcoord[1], lightVec.w;\n"
							"\n"
							"DP3		viewVec.w, fragment.texcoord[2], fragment.texcoord[2];\n"
							"RSQ		viewVec.w, viewVec.w;\n"
							"MUL		viewVec.xyz, fragment.texcoord[2], viewVec.w;\n"
							"\n"
							"DP3		normal.w, fragment.texcoord[3], fragment.texcoord[3];\n"
							"RSQ		normal.w, normal.w;\n"
							"MUL		normal.xyz, fragment.texcoord[3], normal.w;\n"
							"\n"
							"#Reverse the normal for the back faces based on the secondary colour\n"
							"MAD      reverse, two, fragment.color.secondary.x, m_one;\n"
							"MUL      normal, reverse, normal;\n"
							"\n"
							"#Calculate attenuation.\n"
							"MAD		attenuation, state.light[0].attenuation.z, Len.w, state.light[0].attenuation.x;\n"
							"RCP		Len, lightVec.w;\n"
							"MAD		attenuation, Len.w, state.light[0].attenuation.y, attenuation.x;\n"
							"RCP		attenuation.x, attenuation.x;\n"
							"\n"
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
							"\n"
							"# Blinn:\n"
							"#	ADD		reflVec, lightVec, viewVec;	# reflVec == Half-angle.\n"
							"#	DP3		reflVec.w, reflVec, reflVec;\n"
							"#	RSQ		reflVec.w, reflVec.w;\n"
							"#	MUL		reflVec.xyz, reflVec, reflVec.w;\n"
							"#	DP3		lightContrib.y, reflVec, normal;\n"
							"\n"
							"MOV		lightContrib.w, state.material.shininess.x;\n"
							"\n"
							"#Accelerates lighting computations\n"
							"LIT	lightContrib, lightContrib;\n"
							"\n"
							"MAD		finalCol, lightContrib.y, fragment.color, state.lightprod[0].ambient;\n"
							"\n"
							"# Enable this line for textured models\n"
							"#	MUL		finalCol, finalCol, tex;	# Texture?\n"
							"\n"
							"MAD		finalCol, lightContrib.z, state.lightprod[0].specular, finalCol;\n"
							"MAD		finalCol, finalCol, attenuation.x, state.material.emission;\n"
							"#MOV      finalCol, fragment.color.secondary;\n"
							"#MOV      finalCol, fragment.color;\n"
							"MOV      result.color.xyz, finalCol;\n"
							"ADD		result.color.xyz, finalCol, state.lightmodel.scenecolor;\n"
							"MOV		result.color.w, state.material.diffuse.w;\n"
							"END\n";

						if (!material_program->vertex_program)
						{
							glGenProgramsARB(1, &material_program->vertex_program);
							glBindProgramARB(GL_VERTEX_PROGRAM_ARB, material_program->vertex_program);
							glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
								strlen(vertex_program_string), vertex_program_string);
						}

						if (!material_program->fragment_program)
						{
							glGenProgramsARB(1, &material_program->fragment_program);
							glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, material_program->fragment_program);
							glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
								strlen(fragment_program_string), fragment_program_string);
						}

						if (!material_program->display_list)
						{
							material_program->display_list = glGenLists(/*number_of_lists*/1);
						}

						glNewList(material_program->display_list, GL_COMPILE);

						glEnable(GL_VERTEX_PROGRAM_ARB);
						glBindProgramNV(GL_VERTEX_PROGRAM_ARB,
							material_program->vertex_program);
					
						glEnable(GL_FRAGMENT_PROGRAM_ARB);
						glBindProgramNV(GL_FRAGMENT_PROGRAM_ARB,
							material_program->fragment_program);

						glEnable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);

						glEndList();
					}
					else
					{
						display_message(ERROR_MESSAGE, "Material_program_compile.  "
							"Support for PER_PIXEL_LIGHTING requires GL_ARB_vertex_program and GL_ARB_fragment_program extensions.");
					}
#else /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
					display_message(ERROR_MESSAGE, "Material_program_compile.  "
						"Support for PER_PIXEL_LIGHTING was not compiled into this executable.");
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */

				} break;
				default:
				{
					/* Do nothing */
				};
			
			}
			material_program->compiled = 1;
		}
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

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphical_material,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphical_material)

int direct_render_Graphical_material(struct Graphical_material *material)
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
#if defined (OPENGL_API)
	GLfloat values[4];
#endif /* defined (OPENGL_API) */
	int return_code;

	ENTER(direct_render_Graphical_material);
	if (material)
	{
#if defined (OPENGL_API)
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

		if (material->texture)
		{
			execute_Texture(material->texture);
		}
		if (material->program)
		{
			Material_program_execute(material->program);
		}
		else
		{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
			glDisable(GL_VERTEX_PROGRAM_ARB);
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
			glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
		}
		return_code=1;

		material->spectrum_flag = MATERIAL_SPECTRUM_STARTED |
			(MATERIAL_SPECTRUM_ENABLED & material->spectrum_flag);
		/* Indicates first render has been done */
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"direct_render_Graphical_material.  Not defined for this API");
		return_code=0;
#endif /* defined (OPENGL_API) */
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

/*
Global functions
----------------
*/

struct Material_package *CREATE(Material_package)(struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

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
		material_package->texture_manager = texture_manager;
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_package).  Not enough memory");
	}
	LEAVE;

	return (material_package);
} /* CREATE(Material_package) */

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
			DESTROY(LIST(Material_program))(&material_package->material_program_list);
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
			REACCESS(Material_package)(&material->package, material_package);
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

struct Graphical_material *CREATE(Graphical_material)(char *name)
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
			strcpy(material->name,name);
			material->access_count=0;
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
			material->texture=(struct Texture *)NULL;
			material->spectrum_flag=0;
			material->spectrum_flag_previous=0;
			material->package = (struct Material_package *)NULL;
			material->program = (struct Material_program *)NULL;
#if defined (OPENGL_API)
			material->display_list=0;
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
			if (material->texture)
			{
				DEACCESS(Texture)(&(material->texture));
			}
			if (material->package)
			{
				DEACCESS(Material_package)(&(material->package));
			}
			if (material->program)
			{
				DEACCESS(Material_program)(&(material->program));
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

DECLARE_OBJECT_FUNCTIONS(Graphical_material)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphical_material)
DECLARE_INDEXED_LIST_FUNCTIONS(Graphical_material)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphical_material,name,
	char *,strcmp)
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
			destination->package = ACCESS(Material_package)(source->package);
		}
		else
		{
			destination->package = (struct Material_package *)NULL;
		}
		if (source->program)
		{
			destination->program = ACCESS(Material_program)(source->program);
		}
		else
		{
			destination->program = (struct Material_program *)NULL;
		}
		REACCESS(Texture)(&(destination->texture), source->texture);
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

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphical_material,name,char *)
{
	char *destination_name;
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

DECLARE_MANAGER_FUNCTIONS(Graphical_material)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Graphical_material)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Graphical_material,name,char *)

char *Graphical_material_name(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/
{
	char *return_name;
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
		material->spectrum_flag |= MATERIAL_SPECTRUM_AMBIENT;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
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
		material->spectrum_flag |= MATERIAL_SPECTRUM_DIFFUSE;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
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
		material->spectrum_flag |= MATERIAL_SPECTRUM_EMISSION;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
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
		material->spectrum_flag |= MATERIAL_SPECTRUM_SPECULAR;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
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
		material->spectrum_flag |= MATERIAL_SPECTRUM_ALPHA;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
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
		texture=material->texture;
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

int Graphical_material_set_texture(struct Graphical_material *material,
	struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the texture member of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_texture);
	if (material)
	{
		if (texture)
		{
			ACCESS(Texture)(texture);
		}
		if (material->texture)
		{
			DEACCESS(Texture)(&material->texture);
		}
		material->texture=texture;
		/* display list needs to be compiled again */
		material->compile_status = GRAPHICS_NOT_COMPILED;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_texture.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_texture */

int Graphical_material_uses_texture_in_list(struct Graphical_material *material,
	void *texture_list_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Returns true if the <material> uses a texture in the <texture_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(Texture) *texture_list;

	ENTER(Graphical_material_uses_texture);
	if (material && (texture_list = (struct LIST(Texture) *)texture_list_void))
	{
		return_code = material->texture &&
			IS_OBJECT_IN_LIST(Texture)(material->texture, texture_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_uses_texture.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_uses_texture */

int Graphical_material_Texture_change(struct Graphical_material *material,
	void *texture_change_data_void)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
If the <material> uses a texture in the <changed_texture_list>, marks the
material compile_status as CHILD_GRAPHICS_NOT_COMPILED and adds the material
to the <changed_material_list>.
???RC Currently managed by Scene. This function should be replaced once messages
go directly from texture to material.
==============================================================================*/
{
	int return_code;
	struct Graphical_material_Texture_change_data *texture_change_data;

	ENTER(Graphical_material_Texture_change);
	if (material && (texture_change_data =
		(struct Graphical_material_Texture_change_data *)texture_change_data_void))
	{
		if (material->texture && IS_OBJECT_IN_LIST(Texture)(material->texture,
			texture_change_data->changed_texture_list))
		{
			if (material->compile_status != GRAPHICS_NOT_COMPILED)
			{
				material->compile_status = CHILD_GRAPHICS_NOT_COMPILED;
			}
			ADD_OBJECT_TO_LIST(Graphical_material)(material,
				texture_change_data->changed_material_list);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_Texture_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_Texture_change */

int Graphical_material_set_spectrum_flag(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the spectrum_flag of a material giving it the cue to intelligently issue
commands from direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_spectrum_flag);
	if (material)
	{
		material->spectrum_flag |= MATERIAL_SPECTRUM_ENABLED;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_spectrum_flag.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_spectrum_flag */

int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *material_package_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Shifted from command/cmiss.c now that there is a material package.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/
{
	char *current_token;
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

int modify_Graphical_material(struct Parse_state *state,void *material_void,
	void *material_package_void)
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
==============================================================================*/
{
	char *current_token, normal_mode_flag, per_pixel_mode_flag;
	int process,return_code;
	struct Graphical_material *material_to_be_modified,
		*material_to_be_modified_copy;
	struct Material_package *material_package;
	struct Option_table *help_option_table, *option_table, *mode_option_table;

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
					normal_mode_flag = 0;
					per_pixel_mode_flag = 0;
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table, "alpha",
						&(material_to_be_modified_copy->alpha), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "ambient",
						&(material_to_be_modified_copy->ambient), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "diffuse",
						&(material_to_be_modified_copy->diffuse), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "emission",
						&(material_to_be_modified_copy->emission), NULL,
						set_Colour);
					mode_option_table = CREATE(Option_table)();
					Option_table_add_char_flag_entry(mode_option_table,
						"normal_mode", &normal_mode_flag);
					Option_table_add_char_flag_entry(mode_option_table,
						"per_pixel_mode", &per_pixel_mode_flag);
					Option_table_add_suboption_table(option_table, mode_option_table);
					Option_table_add_entry(option_table, "shininess",
						&(material_to_be_modified_copy->shininess), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "specular",
						&(material_to_be_modified_copy->specular), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "texture",
						&(material_to_be_modified_copy->texture), 
						material_package->texture_manager,
						set_Texture);
					if (return_code=Option_table_multi_parse(option_table, state))
					{
						if (normal_mode_flag + per_pixel_mode_flag > 1)
						{
							display_message(ERROR_MESSAGE,
								"Specify only one of normal_mode/per_pixel_mode.");
						}
						if (normal_mode_flag)
						{
							if (material_to_be_modified_copy->program)
							{
								DEACCESS(Material_program)(&material_to_be_modified_copy->program);
							}
						}
						else if (per_pixel_mode_flag)
						{
							if (!(material_to_be_modified_copy->program = 
								FIND_BY_IDENTIFIER_IN_LIST(Material_program,type)(
									MATERIAL_PROGRAM_PER_PIXEL_LIGHTING, material_package->material_program_list)))
							{
								material_to_be_modified_copy->program = CREATE(Material_program)
									(MATERIAL_PROGRAM_PER_PIXEL_LIGHTING);
								ADD_OBJECT_TO_LIST(Material_program)(material_to_be_modified_copy->program,
									material_package->material_program_list);
							}
						}
						if (material_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
								material_to_be_modified,material_to_be_modified_copy,
								material_package->material_manager);
							DESTROY(Graphical_material)(&material_to_be_modified_copy);
						}
						else
						{
							material_to_be_modified=material_to_be_modified_copy;
						}
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
	char line[80],*texture_name;
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
		if (material->texture&&GET_NAME(Texture)(material->texture,&texture_name))
		{
			display_message(INFORMATION_MESSAGE,"  texture : ");
			display_message(INFORMATION_MESSAGE,texture_name);
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(texture_name);
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
LAST MODIFIED : 22 January 2002

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
		if (material->texture&&GET_NAME(Texture)(material->texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," texture %s",name);
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

int file_read_Graphical_material_name(FILE *file,
	struct Graphical_material **material_address,
	struct MANAGER(Graphical_material) *graphical_material_manager)
/*******************************************************************************
LAST MODIFIED : 20 June 1996

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
	if (file&&material_address)
	{
		if (read_string(file,"s",&material_name))
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

int compile_Graphical_material(struct Graphical_material *material,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2002

DESCRIPTION :
Graphical_material list/manager iterator function.
Rebuilds the display_list for <material> if it is not current. If <material>
does not have a display list, first attempts to give it one. The display list
created here may be called using execute_Graphical_material, below.
???RC Graphical_materials must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Graphical_material should just call direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(compile_Graphical_material);
	USE_PARAMETER(dummy_void);
	if (material)
	{
		return_code = 1;
		if (GRAPHICS_COMPILED != material->compile_status)
		{
			/* must compile texture before opening material display list */
			if (material->texture)
			{
				compile_Texture(material->texture, NULL);
			}
			if (material->program)
			{
				Material_program_compile(material->program);
			}
			if (GRAPHICS_NOT_COMPILED == material->compile_status)
			{
#if defined (OPENGL_API)
				if (material->display_list || (material->display_list = glGenLists(1)))
				{
					glNewList(material->display_list,GL_COMPILE);
					direct_render_Graphical_material(material);
					glEndList();
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"compile_Graphical_material.  Could not generate display list");
					return_code = 0;
				}
#else /* defined (OPENGL_API) */
				display_message(ERROR_MESSAGE,
					"compile_Graphical_material.  Not defined for this API");
				return_code = 0;
#endif /* defined (OPENGL_API) */
			}
			if (return_code)
			{
				material->compile_status = GRAPHICS_COMPILED;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Graphical_material */

int execute_Graphical_material(struct Graphical_material *material)
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
#if defined (OPENGL_API)
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
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"execute_Graphical_material.  Not defined for this API");
		return_code = 0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
#if defined (OPENGL_API)
		/* turn off any texture */
		return_code = execute_Texture((struct Texture *)NULL);
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"execute_Graphical_material.  Not defined for this API");
		return_code = 0;
#endif /* defined (OPENGL_API) */
	}
	LEAVE;

	return (return_code);
} /* execute_Graphical_material */

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the material from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	char *current_token;
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
	struct Option_table *option_table, char *token,
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
