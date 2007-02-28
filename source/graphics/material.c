/*******************************************************************************
FILE : material.c

LAST MODIFIED : 6 October 2006

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
 * S. Blackett (s.blackett at auckland.ac.nz)
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "command/parser.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/io_stream.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
enum Material_program_type
/*******************************************************************************
LAST MODIFIED : 14 September 2005

DESCRIPTION :
Enumerates the main different types of vertex/fragment program for materials
==============================================================================*/
{
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
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 = 4,
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 = 8,
	/* If either bit is set then we are using a texture */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE = 12,
	/* If this bit is set then a colour texture will replace the lighting calculation for a colour,
		if it is not set then the texture will modulate the lighting calculation colour */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL = 16,
	/* Specifies the output dimension of the texture and therefore how it is applied.
		OUTPUT1 = grayscale, OUTPUT2 = grayscale and alpha, OUTPUT1 & OUTPUT2 = rgb
		!OUPUT1 & !OUTPUT2 = rgba. */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 = 32,
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 = 64,

	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a secondary or bump map texture. */
	MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_1 = 128,
	MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_2 = 256,
	MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE = 384,
   /* Specifies that the secondary texture is intended to be used as a bump map texture, modulating
		the per pixel value of the normal in the lighting calculation */
	MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP = 512,

	/* The colour value is used as the input, derived from the primary texture or the lighting. 
	   The specified number of input components (1, 2 or 3) are used directly from the input source */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 = 1024,
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 = 2048,

	/* Specify the outputs in the dependent texture lookup, either replacing the colour, alpha or both. */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR = 4096,
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA = 8192,

   /* Order independent transparency passes */
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_FIRST_LAYER = 16384,
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER = 32768
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

#if defined (OPENGL_API)
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
	GLuint vertex_program;
	GLuint fragment_program;
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */

	/* Display list which enables the correct state for this program */
	GLuint display_list;
#endif /* defined (OPENGL_API) */

	/* Flag indicating whether the program is compiled or not */
	int compiled;

	int access_count;
}; /* struct Material_program */

DECLARE_LIST_TYPES(Material_program);
PROTOTYPE_LIST_FUNCTIONS(Material_program);
FULL_DECLARE_INDEXED_LIST_TYPE(Material_program);

struct Graphical_material
/*******************************************************************************
LAST MODIFIED : 23 January 2004

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
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the texture for this material */
	struct Texture *texture;
	/* the texture used for adjusting normal calculations with this material */
	struct Texture *secondary_texture;
	/* spectrum used to render this material */
	struct Spectrum *spectrum;	
	/* callback if the spectrum changes */
	void *spectrum_manager_callback_id;
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
	struct MANAGER(Spectrum) *spectrum_manager;
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
#if defined (OPENGL_API)
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
		material_program->vertex_program = 0;
		material_program->fragment_program = 0;
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
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
LAST MODIFIED : 23 January 2004

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
		return_code = 1;
/* #define TESTING_PROGRAM_STRINGS */
/* #define DEBUG */
#if defined (TESTING_PROGRAM_STRINGS)
		/* If testing always recompile */
		material_program->compiled = 0;
#endif /* defined (TESTING_PROGRAM_STRINGS) */
		if (!material_program->compiled)
		{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
			if (Graphics_library_check_extension(GL_ARB_vertex_program) &&
				Graphics_library_check_extension(GL_ARB_fragment_program))
			{
#if ! defined (TESTING_PROGRAM_STRINGS)
				char *fragment_program_string, *vertex_program_string;
				int error;
#if defined (DEBUG)
				const GLubyte *error_msg;
#endif /* defined (DEBUG) */
				
				error = 0;
				if (MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING & material_program->type)
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
						"\n"
						"TEMP eyeVertex;\n"
						"TEMP eyeNormal;\n"
						"TEMP viewVec, lightVec, Len, reflVec, finalCol, attenuation, lightContrib;\n"
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
						"SUB viewVec, eyeCameraPos, eyeVertex;\n"
						"DP3 viewVec.w, viewVec, viewVec;\n"
						"RSQ viewVec.w, viewVec.w;\n"
						"MUL viewVec.xyz, viewVec.w, viewVec;\n"
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
						"DP3	   lightContrib.x, eyeNormal, lightVec;\n"
						"ABS      lightContrib.x, lightContrib.x;\n"
						"\n"
						"#Specular\n"
						"# Phong:\n"
						"#DP3		reflVec, lightVec, eyeNormal;\n"
						"#MUL		reflVec, reflVec, two;\n"
						"#MAD		reflVec, reflVec.x, eyeNormal, -lightVec;\n"
						"#DP3	lightContrib.y, reflVec, viewVec;\n"
						"\n"
						"DP3      lightContrib.y, eyeNormal, state.light[0].half;\n"
						"\n"
						"MOV		lightContrib.w, state.material.shininess.x;\n"
						"\n"
						"#Accelerates lighting computations\n"
						"LIT	lightContrib, lightContrib;\n"
						"\n"
						"MAD		finalCol, lightContrib.y, vertex.color, state.lightprod[0].ambient;\n"
						"MAD		finalCol, lightContrib.z, state.lightprod[0].specular, finalCol;\n"
						"MAD		finalCol, finalCol, attenuation.x, state.material.emission;\n"
						"#ADD		result.color.xyz, finalCol, state.lightmodel.scenecolor;\n"
						"\n"
						"MAD		finalCol, state.material.ambient, state.lightmodel.ambient, finalCol;\n"
						"\n"
						"MOV finalCol.w, state.material.diffuse.w;\n"
						"\n"
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
						"END\n");

					fragment_program_string = duplicate_string("!!ARBfp1.0\n");
					if (MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER & material_program->type)
					{
						append_string(&fragment_program_string, 
							"OPTION ARB_fragment_program_shadow;\n"
							"PARAM texturesize = program.env[0];\n"

							"TEMP tex4, kill, tex4coord;\n"
							"PARAM two = {2.0, 2.0, 2.0, 2.0};\n"
							"PARAM one = {1.0, 1.0, 1.0, 1.0};\n"
							"#PARAM minus_one = {-1.0, -1.0, -1.0, -1.0};\n"
							, &error);
					}

					append_string(&fragment_program_string, 
							"TEMP eyespaceCoord, perspective;\n"
							"PARAM point_five = {0.5, 0.5, 0.5, 0.5};\n"

							"MOV      eyespaceCoord, fragment.texcoord[1];\n"
							"RCP      perspective.w, eyespaceCoord.w;\n"
							"MUL      eyespaceCoord, eyespaceCoord, perspective.w;\n"
							"MAD      eyespaceCoord, eyespaceCoord, point_five, point_five;\n"
							, &error);
						
					if (MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER & material_program->type)
					{
						append_string(&fragment_program_string, 
							"#MOV      tex4coord, one;\n"
							"MOV      tex4coord, eyespaceCoord.xyzx;\n"
							"MUL      tex4coord, tex4coord, texturesize;\n"
							"#MUL      tex4coord, eyespaceCoord, two;\n"
							"#MAD      tex4coord, eyespaceCoord, point_five, point_five;\n"
							"\n"
							"MOV      tex4.x, eyespaceCoord.zzzz;\n"
							"TEX		tex4.x, tex4coord, texture[3], SHADOWRECT;\n"
							"\n"
							"#SUB      kill, eyespaceCoord.zzzz, tex4.rrrr;\n"
							"#CMP      kill, kill, minus_one, one;\n"
							"#MUL      kill, kill, small;\n"
							"#MOV      kill, eyespaceCoord.zzzz;\n"
							"\n"
							"ADD      kill.x, tex4.x, -0.5;\n"
							"KIL      kill.x;\n"
							, &error);
					}

					append_string(&fragment_program_string, 
						"MOV      result.color.xyzw, fragment.color.rgba;\n"
						"MOV		 result.depth.z, eyespaceCoord.z;\n"
						"\n"
						"#MOV      result.color.xyz, tex4coord.zzzz;\n"
						"#MOV      result.color.xyz, tex4.xxxx;\n"
						"\n"
						"END\n", &error);
				}
				else if (MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING & material_program->type)
				{
					vertex_program_string = duplicate_string("!!ARBvp1.0\n");
					append_string(&vertex_program_string, 
						"ATTRIB normal = vertex.normal;\n"
						"ATTRIB position = vertex.position;\n"
						, &error);
					if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE)
						& material_program->type)
					{
						append_string(&vertex_program_string, 
							"PARAM texture_scaling = program.env[0];\n"
							, &error);
					}
					if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material_program->type)
					{
						append_string(&vertex_program_string, 
							"ATTRIB tangent = vertex.texcoord[1];\n"
							, &error);
					}
					append_string(&vertex_program_string, 
						"PARAM c0[4] = { state.matrix.mvp };\n"
						"PARAM c1[4] = { state.matrix.modelview };\n"
						"PARAM eyeCameraPos = {0, 0, 0, 0};\n"
						"PARAM eyeLightPos = state.light[0].position;\n"

						"TEMP eyeVertex;\n"
						"TEMP viewVec;\n"
						, &error);
					if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material_program->type)
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
					else
					{
						append_string(&vertex_program_string, 
							"PARAM c2[4] = { state.matrix.modelview.invtrans };\n"
							"TEMP eyeNormal;\n"
							, &error);
					}

					append_string(&vertex_program_string, 
						"#Vertex position in eyespace\n"
						"DP4 eyeVertex.x, c1[0], position;\n"
						"DP4 eyeVertex.y, c1[1], position;\n"
						"DP4 eyeVertex.z, c1[2], position;\n"
						, &error);								

					if ((MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE | MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE)
						& material_program->type)
					{
						append_string(&vertex_program_string, 
							"MUL result.texcoord[0], texture_scaling, vertex.texcoord[0];\n"
							, &error);
					}

					if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material_program->type)
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
								
					fragment_program_string = duplicate_string("!!ARBfp1.0\n");
					append_string(&fragment_program_string, 
						"TEMP lightVec, viewVec, reflVec, normal, attenuation, Len, finalCol, lightContrib, reverse, tex, tex2;\n"
						"PARAM two = {2.0, 2.0, 2.0, 2.0};\n"
						"PARAM m_one = {-1.0, -1.0, -1.0, -1.0};\n"

						"#Set up reverse vector based on secondary colour\n"
						"MAD      reverse, two, fragment.color.secondary.x, m_one;\n"
						, &error);


					if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE & material_program->type)
					{
						/* Load the colour texture */
						if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 & material_program->type)
							{
								append_string(&fragment_program_string, 
									"TEX		tex, fragment.texcoord[0], texture[0], 3D;\n"
									, &error);
							}
							else
							{
								append_string(&fragment_program_string, 
									"TEX		tex, fragment.texcoord[0], texture[0], 1D;\n"
									, &error);
							}								
						}
						else
						{
							append_string(&fragment_program_string, 
								"TEX		tex, fragment.texcoord[0], texture[0], 2D;\n"
								, &error);
						}
					}

					if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE & material_program->type)
					{
						/* Load the secondary texture using the same texture coordinates as the colour texture */
						if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_1 & material_program->type)
						{
							if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_2 & material_program->type)
							{
								append_string(&fragment_program_string, 
									"TEX		tex2, fragment.texcoord[0], texture[1], 3D;\n"
									, &error);
							}
							else
							{
								append_string(&fragment_program_string, 
									"TEX		tex2, fragment.texcoord[0], texture[1], 1D;\n"
									, &error);
							}								
						}
						else
						{
							append_string(&fragment_program_string, 
								"TEX		tex2, fragment.texcoord[0], texture[1], 2D;\n"
								, &error);
						}
					}
					if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material_program->type)
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
						/* Normal is stored in texcoord[3] */
						append_string(&fragment_program_string, 
							"#Normalize the normal.\n"
							"DP3		normal.w, fragment.texcoord[3], fragment.texcoord[3];\n"
							"RSQ		normal.w, normal.w;\n"
							"MUL		normal.xyz, fragment.texcoord[3], normal.w;\n"

							"#Reverse the normal if required\n"
							"MUL      normal, reverse, normal;\n"
							, &error);
					}
					if (!(MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL & material_program->type))
					{
						/* Usual lighting calculations */
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
									append_string(&fragment_program_string, 
										"MUL		finalCol.xyz, finalCol, tex;\n"
										, &error);
								}
								else
								{
									/* grayscale texture */
									append_string(&fragment_program_string, 
										"MUL		finalCol.xyz, finalCol.xyz, tex.x;\n"
										, &error);
								}
							}
							else
							{
								if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
								{
									/* grayscale alpha texture */
									append_string(&fragment_program_string, 
										"MUL		finalCol.xyz, finalCol.xyz, tex.x;\n"
										"MUL		finalCol.w, finalCol.w, tex.y;\n"
										, &error);
								}
								else
								{
									/* RGBA texture */
									append_string(&fragment_program_string, 
										"MUL		finalCol, finalCol, tex;\n"
										, &error);
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
								append_string(&fragment_program_string, 
									"MOV		finalCol.xyz, tex;\n"
									, &error);
							}
							else
							{
								/* grayscale texture */
								append_string(&fragment_program_string, 
									"MOV		finalCol.xyz, tex.x;\n"
									, &error);
							}
							append_string(&fragment_program_string, 
								"MOV		finalCol.w, state.material.diffuse.w;\n"
								, &error);
						}
						else
						{
							if (MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 & material_program->type)
							{
								/* grayscale alpha texture */
								append_string(&fragment_program_string, 
									"MOV		finalCol.xyz, tex.x;\n"
									"MOV		finalCol.w, tex.y;\n"
									, &error);
							}
							else
							{
								/* RGBA texture */
								append_string(&fragment_program_string, 
									"MOV		finalCol, tex;\n"
									, &error);
							}
						}

					}
					if ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 | 
							MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2) & material_program->type)
					{
						append_string(&fragment_program_string, 	
							"TEMP dependentlookup;\n"
							, &error);
						switch ((MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 | 
								MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2) & material_program->type)
						{
							case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1:
							{
								append_string(&fragment_program_string,
									"TEX		dependentlookup, finalCol, texture[2], 1D;\n"
									, &error);
							} break;
							case MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2:
							{
								append_string(&fragment_program_string,
									"TEX		dependentlookup, finalCol, texture[2], 2D;\n"
									, &error);
							} break;
							case (MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 |
								MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2):
							{
								append_string(&fragment_program_string,
									"TEX		dependentlookup, finalCol, texture[2], 3D;\n"
									, &error);
							} break;
						}
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

					append_string(&fragment_program_string, 
						"MOV		result.color, finalCol;\n"
						"END"
						, &error);
				}

				if (!material_program->vertex_program)
				{
					glGenProgramsARB(1, &material_program->vertex_program);
				}
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, material_program->vertex_program);
				glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					strlen(vertex_program_string), vertex_program_string);
#if defined (DEBUG)
				error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
#endif /* defined (DEBUG) */
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
#endif /* defined (DEBUG) */
				DEALLOCATE(fragment_program_string);
				
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

					if (!material_program->vertex_program)
					{
						glGenProgramsARB(1, &material_program->vertex_program);
					}

					glBindProgramARB(GL_VERTEX_PROGRAM_ARB, material_program->vertex_program);
					glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
						strlen(vertex_program_string), vertex_program_string);

					if (!material_program->fragment_program)
					{
						glGenProgramsARB(1, &material_program->fragment_program);
					}

					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, material_program->fragment_program);
					glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
						strlen(fragment_program_string), fragment_program_string);

					material_program->compiled = 1;
					
#endif /* ! defined (TESTING_PROGRAM_STRINGS) */
					
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
				"GL_ARB_vertex_program and GL_ARB_fragment_program extensions "
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

#if defined (GL_VERSION_1_3)
		if (material->spectrum)
		{
			if (Graphics_library_check_extension(GL_VERSION_1_3))
			{
				glActiveTexture(GL_TEXTURE2_ARB);
				Spectrum_execute_colour_lookup(material->spectrum);
				glActiveTexture(GL_TEXTURE0_ARB);
			}
		}
		if (material->secondary_texture)
		{
			/* I used to test for the GL_VERSION_1_3 when setting the texture
				and not here, but at that point the openGL may not have been
				initialised yet, instead check at compile time. */
			if (Graphics_library_check_extension(GL_VERSION_1_3))
			{
				glActiveTexture(GL_TEXTURE1_ARB);
				execute_Texture(material->secondary_texture);
				glActiveTexture(GL_TEXTURE0_ARB);
			}
		}
		else
		{
			/* If the secondary_texture has been set then we know the
				extension is available so we do not need to test above.
				When disabling however we are unsure whether any multitextures
				have been enabled and so have to check if the extension is available
				and if so then disable. */
			if (Graphics_library_check_extension(GL_VERSION_1_3))
			{
				glActiveTexture(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE2_ARB);
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glActiveTexture(GL_TEXTURE0_ARB);
			}
		}
#endif /* defined (GL_VERSION_1_3) */

		if (material->program)
		{
			Material_program_execute(material->program);
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
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
		}
		return_code=1;

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

static void Graphical_material_Spectrum_change(
	struct MANAGER_MESSAGE(Spectrum) *message, void *material_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(Graphical_material_Spectrum_change);
	if ((material = (struct Graphical_material *)material_void)
		&& message)
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Spectrum):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum):
			{
				if (IS_OBJECT_IN_LIST(Spectrum)(material->spectrum,
						message->changed_object_list))
				{
					material->compile_status = GRAPHICS_NOT_COMPILED;
				}
			} break;
			case MANAGER_CHANGE_ADD(Spectrum):
			case MANAGER_CHANGE_REMOVE(Spectrum):
			case MANAGER_CHANGE_IDENTIFIER(Spectrum):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_Spectrum_change.  Invalid argument(s)");
	}
	LEAVE;

} /* Graphical_material_Spectrum_change */

/*
Global functions
----------------
*/

struct Material_package *CREATE(Material_package)(
	struct MANAGER(Texture) *texture_manager,
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
			material->spectrum=(struct Spectrum *)NULL;
			material->spectrum_manager_callback_id=NULL;
			material->texture=(struct Texture *)NULL;
			material->secondary_texture=(struct Texture *)NULL;
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
			}
			if (material->texture)
			{
				DEACCESS(Texture)(&(material->texture));
			}
			if (material->secondary_texture)
			{
				DEACCESS(Texture)(&(material->secondary_texture));
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
			destination->package = source->package;
		}
		else
		{
			destination->package = (struct Material_package *)NULL;
		}
		REACCESS(Material_program)(&destination->program, source->program);
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
			}
		}
		REACCESS(Texture)(&(destination->texture), source->texture);
		REACCESS(Texture)(&(destination->secondary_texture), source->secondary_texture);
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
		/* Don't check run time availability yet as we may 
			not have initialised any openGL display yet */
		REACCESS(Spectrum)(&material->spectrum, spectrum);
		material->compile_status = GRAPHICS_NOT_COMPILED;
		return_code=1;
#else /* defined (GL_VERSION_1_3) */
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_colour_lookup_spectrum.  "
			"OpenGL version 1.3 required for colour lookup spectrums and not compiled into this executable.");
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
LAST MODIFIED : 23 January 2004

DESCRIPTION :
==============================================================================*/
{
	char bump_mapping_flag, *current_token, normal_mode_flag, per_pixel_mode_flag;
	enum Spectrum_colour_components spectrum_colour_components;
	int dimension, process, return_code;
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
					bump_mapping_flag = 0;
					normal_mode_flag = 0;
					per_pixel_mode_flag = 0;
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table, "alpha",
						&(material_to_be_modified_copy->alpha), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "ambient",
						&(material_to_be_modified_copy->ambient), NULL,
						set_Colour);
					Option_table_add_char_flag_entry(option_table,
						"bump_mapping", &bump_mapping_flag);
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
					mode_option_table = CREATE(Option_table)();
					Option_table_add_char_flag_entry(mode_option_table,
						"normal_mode", &normal_mode_flag);
					Option_table_add_char_flag_entry(mode_option_table,
						"per_pixel_mode", &per_pixel_mode_flag);
					Option_table_add_suboption_table(option_table, mode_option_table);
					Option_table_add_entry(option_table, "secondary_texture",
						&(material_to_be_modified_copy->secondary_texture), 
						material_package->texture_manager,
						set_Texture);
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
							return_code = 0;
						}
#if !defined (GL_VERSION_1_3)
						/* Don't check run time availability yet as we may 
							not have initialised any openGL display yet */
						if (material_to_be_modified_copy->secondary_texture)
						{
							display_message(ERROR_MESSAGE,
								"Bump mapping requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(Texture)(&material_to_be_modified_copy->secondary_texture);
							return_code = 0;
						}
						if (material_to_be_modified_copy->spectrum)
						{
							display_message(ERROR_MESSAGE,
								"A colour lookup spectrum requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(Spectrum)(&material_to_be_modified_copy->spectrum);
							return_code = 0;
						}
#endif /* defined (GL_VERSION_1_3) */
						if (material_to_be_modified_copy->spectrum)
						{
#if defined (GL_VERSION_1_3)
							if (!Graphics_library_check_extension(GL_VERSION_1_3))
							{
								display_message(ERROR_MESSAGE,
									"A colour lookup spectrum requires OpenGL version 1.3 or better which is "
									"not available with this OpenGL implementation.");
								return_code = 0;
							}
#else /* defined (GL_VERSION_1_3) */
							display_message(ERROR_MESSAGE,
								"A colour lookup spectrum requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							return_code = 0;
#endif /* defined (GL_VERSION_1_3) */
						}
						if (normal_mode_flag)
						{
							if (material_to_be_modified_copy->program)
							{
								DEACCESS(Material_program)(&material_to_be_modified_copy->program);
							}
						}
						else if (per_pixel_mode_flag || material_to_be_modified_copy->program)
						{
							enum Material_program_type type;

							type = MATERIAL_PROGRAM_PER_PIXEL_LIGHTING;

							if (material_to_be_modified_copy->texture)
							{
								Texture_get_dimension(material_to_be_modified_copy->texture, &dimension);
								switch (dimension)
								{
									case 1:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1;
									} break;
									case 2:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
									} break;
									case 3:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 |
											MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE, "Colour texture dimension %d not supported.",  
											dimension);
										return_code = 0;
									} break;
								}
								switch (Texture_get_number_of_components(material_to_be_modified_copy->texture))
								{
									case 1:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1;
									} break;
									case 2:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2;
									} break;
									case 3:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 |
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
								switch (Texture_get_combine_mode(material_to_be_modified_copy->texture))
								{
									case TEXTURE_DECAL:
									{
										type |= MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL;
									}
									default:
									{
										/* Do nothing as modulate is the default */
									}
								}
							}
							if (material_to_be_modified_copy->secondary_texture)
							{
								Texture_get_dimension(material_to_be_modified_copy->secondary_texture, &dimension);
								switch (dimension)
								{
									case 1:
									{
										type |= MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_1;
									} break;
									case 2:
									{
										type |= MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_2;
									} break;
									case 3:
									{
										type |= MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_1 |
											MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_2;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE, "Secondary texture dimension %d not supported.",  
											dimension);
										return_code = 0;
									} break;
								}
							}
							if (bump_mapping_flag)
							{
								if (material_to_be_modified_copy->secondary_texture)
								{
									type |= MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Bump mapping requires specification of a secondary texture containing a normal map.");
									return_code = 0;
								}
							}
							if (material_to_be_modified_copy->spectrum)
							{
								/* Cannot just rely on the COPY functions as when 
									first created this will be the actual object. */
								if (material_to_be_modified_copy->package && 
									(!material_to_be_modified_copy->spectrum_manager_callback_id))
								{
									material_to_be_modified_copy->spectrum_manager_callback_id=
										MANAGER_REGISTER(Spectrum)(Graphical_material_Spectrum_change,
										(void *)material_to_be_modified_copy, material_to_be_modified_copy->package->spectrum_manager);
								}

								switch (Spectrum_get_number_of_components(
									material_to_be_modified_copy->spectrum))
								{
									case 1:
									{
										type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1;
									} break;
									case 2:
									{
										type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2;
									} break;
									case 3:
									default:
									{
										type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 |
											MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2;
									} break;
								}
								spectrum_colour_components = Spectrum_get_colour_components(
									material_to_be_modified_copy->spectrum);
								if (spectrum_colour_components & SPECTRUM_COMPONENT_ALPHA) 
								{
									if (spectrum_colour_components == SPECTRUM_COMPONENT_ALPHA)
									{
										/* Alpha only */
										type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA;
									}
									else
									{
										/* Colour and alpha */
										type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR
											| MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA;
									}
								}
								else
								{
									/* Colour only */
									type |= MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR;
								}
							}
							else
							{
								if (material_to_be_modified_copy->package && 
									material_to_be_modified_copy->spectrum_manager_callback_id)
								{
									MANAGER_DEREGISTER(Spectrum)(
										material_to_be_modified_copy->spectrum_manager_callback_id,
										material_to_be_modified_copy->package->spectrum_manager);
								}
							}

							if (!material_to_be_modified_copy->program ||
								(material_to_be_modified_copy->program->type != type))
							{
								if (material_to_be_modified_copy->program)
								{
									DEACCESS(Material_program)(&material_to_be_modified_copy->program);
								}
								if (material_to_be_modified_copy->program = 
									FIND_BY_IDENTIFIER_IN_LIST(Material_program,type)(
									type, material_package->material_program_list))
								{
									ACCESS(Material_program)(material_to_be_modified_copy->program);
								}
								else
								{
									if (material_to_be_modified_copy->program = ACCESS(Material_program)(
										CREATE(Material_program)(type)))
									{
										ADD_OBJECT_TO_LIST(Material_program)(material_to_be_modified_copy->program,
											material_package->material_program_list);
									}
									else
									{
										return_code = 0;
									}
								}
							}
						}
						if (return_code)
						{
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
						else
						{
							if (material_to_be_modified)
							{
								DESTROY(Graphical_material)(&material_to_be_modified_copy);
							}
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
			else if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material->program->type)
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
		if (material->texture&&GET_NAME(Texture)(material->texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  texture : ");
			display_message(INFORMATION_MESSAGE, name);
			display_message(INFORMATION_MESSAGE, "\n");
			DEALLOCATE(name);
		}
		if (material->secondary_texture&&GET_NAME(Texture)(material->secondary_texture,&name))
		{
			display_message(INFORMATION_MESSAGE, "  secondary texture : ");
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
			else if (MATERIAL_PROGRAM_CLASS_SECONDARY_TEXTURE_BUMPMAP & material->program->type)
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
		if (material->texture&&GET_NAME(Texture)(material->texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," texture %s",name);
			DEALLOCATE(name);
		}
		if (material->secondary_texture&&GET_NAME(Texture)(material->secondary_texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," secondary_texture %s",name);
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
			if (material->secondary_texture)
			{
				compile_Texture(material->secondary_texture, NULL);
			}
			if (material->spectrum)
			{
				Spectrum_compile_colour_lookup(material->spectrum);
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
					if (material->program)
					{
						/* Load up the texture size into the vertex program environment */
						if (material->texture)
						{
							Texture_execute_vertex_program_environment(material->texture);
						}
						else if (material->secondary_texture)
						{
							Texture_execute_vertex_program_environment(material->secondary_texture);
						}
					}
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
#if defined (OPENGL_API)
	enum Material_program_type modified_type;
	struct Material_order_independent_transparency *data;
	struct Material_package *material_package;
	struct Material_program *unmodified_program;
#endif /* defined (OPENGL_API) */

	ENTER(compile_Graphical_material);

#if defined (OPENGL_API)
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
					Material_program,type)(modified_type, 
						material_package->material_program_list)))
				{
					if (material->program = ACCESS(Material_program)(
						CREATE(Material_program)(modified_type)))
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
			direct_render_Graphical_material(material);
			glEndList();

			material->program = unmodified_program;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Graphical_material.  Missing material");
		return_code=0;
	}
#else /* defined (OPENGL_API) */
	USE_PARAMETER(material);
	USE_PARAMETER(material_order_independent_data_void);
	display_message(ERROR_MESSAGE,
		"compile_Graphical_material.  Not defined for this graphics API");
	return_code = 0;
#endif /* defined (OPENGL_API) */
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
