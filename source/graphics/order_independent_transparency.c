/*******************************************************************************
FILE : order_independent_transparency.c

LAST MODIFIED : 14 April 2003

DESCRIPTION :
Implements in NVIDIA hardware a depth sorting algorithm where each depth
layer is peeled off and then composited back together from back to front
at the end.  

HISTORY :
  Based on algorithm from NVIDIA using SGIS_shadow and NV_texture_shader to
  implement Rui Bastos's idea for getting the layers of z sorted
  by depth, and using the RGBA at each layer to re-order transparency
  correctly.
  Cass Everitt 3-28-01
  SAB. I got rid of the glh, c++ and nv_parse stuff.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"

#include "graphics/order_independent_transparency.h"

static char *required_extensions[] = {"GL_NV_register_combiners",
												  "GL_NV_register_combiners2",
												  "GL_NV_texture_rectangle",
												  "GL_NV_texture_shader",
												  "GL_SGIX_depth_texture",
												  "GL_SGIX_shadow"};
#if ! defined (WIN32_SYSTEM)
#if defined GL_NV_register_combiners && defined GL_NV_register_combiners2 \
   && defined GL_NV_texture_rectangle && defined GL_NV_texture_shader \
   && defined GL_SGIX_depth_texture && defined GL_SGIX_shadow
#define ORDER_INDEPENDENT_CAPABLE
#endif
#endif /* ! defined (WIN32_SYSTEM) */

#if defined (ORDER_INDEPENDENT_CAPABLE)
struct Scene_viewer_order_independent_transparency_data
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
The private user data for this order independent transparency rendering pass.
==============================================================================*/
{
	unsigned int ztex_texture_id;
	unsigned int simple_1x1_uhilo_texture_id;
	unsigned int *rgba_layer_texture_id;

	unsigned int quad_display_list;
	unsigned int geometry_display_list;
	unsigned int rc_highlight_adds_alpha_display_list;
	unsigned int rc_highlight_adds_alpha__peel_display_list;
	unsigned int rc_sum_display_list;
	unsigned int rc_sum__peel_display_list;
	unsigned int rc_composite_display_list;
	unsigned int rc_composite_alphaone_display_list;

	int viewport_width;
	int viewport_height;

	int using_stencil_overlay;

	int number_of_layers;
	int maximum_number_of_layers;

	GLuint *zbuffer;	

	GLenum depth_format;
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

static void texgen(int enable)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Enables or disables all the texture generation coordinates.
==============================================================================*/
{
	ENTER(texgen);
	if(enable)
	{
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	}

	LEAVE;
} /* texgen */

static void verify_shader_config(void)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Initialises the order independent transparency extension.
==============================================================================*/
{
	int consistent;

	ENTER(verify_shader_config);

	glActiveTexture( GL_TEXTURE0 );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, &consistent);
	if(consistent == GL_FALSE)
	{
		display_message(WARNING_MESSAGE,
			"verify_shader_config.  Shader stage 0 is inconsistent!\n");
	}

	glActiveTexture( GL_TEXTURE1 );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, &consistent);
	if(consistent == GL_FALSE)
	{
		display_message(WARNING_MESSAGE,
			"verify_shader_config.  Shader stage 1 is inconsistent!\n");
	}

	glActiveTexture( GL_TEXTURE2 );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, &consistent);
	if(consistent == GL_FALSE)
	{
		display_message(WARNING_MESSAGE,
			"verify_shader_config.  Shader stage 2 is inconsistent!\n");
	}

	glActiveTexture( GL_TEXTURE3 );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, &consistent);
	if(consistent == GL_FALSE)
	{
		display_message(WARNING_MESSAGE,
			"verify_shader_config.  Shader stage 3 is inconsistent!\n");
	}

	glActiveTexture( GL_TEXTURE0 );

	LEAVE;
} /* verify_shader_config */

static int order_independent_init_opengl(
	struct Scene_viewer_order_independent_transparency_data *data)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Initialises the order independent transparency extension.
==============================================================================*/
{
	int return_code;

	ENTER(order_independent_init_opengl);

	return_code = 1;
	if (!data->ztex_texture_id)
	{
		glGenTextures(1, &data->ztex_texture_id);
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->ztex_texture_id);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_COMPARE_SGIX, GL_TRUE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_COMPARE_OPERATOR_SGIX,
			GL_TEXTURE_LEQUAL_R_SGIX);
	}

	if (!data->simple_1x1_uhilo_texture_id)
	{
		GLushort texel[] = {0, 0};
		 
		 glGenTextures(1, &data->simple_1x1_uhilo_texture_id);
		 glBindTexture(GL_TEXTURE_2D, data->simple_1x1_uhilo_texture_id);
		 glTexImage2D(GL_TEXTURE_2D, 0, GL_HILO_NV, 1, 1, 0, GL_HILO_NV, GL_UNSIGNED_SHORT, texel);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	 }
	
	 if (!data->rc_composite_display_list)
	 {
		 data->rc_composite_display_list = glGenLists(1);
		 glNewList(data->rc_composite_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "out.rgb = tex0;                                              \n"
			 "out.a = tex0;                                                \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_PER_STAGE_CONSTANTS_NV);
		glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE0, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE0, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glEndList();
	 }

	 if (!data->rc_composite_alphaone_display_list)
	 {
		 data->rc_composite_alphaone_display_list = glGenLists(1);
		 glNewList(data->rc_composite_alphaone_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "out.rgb = tex0;                                              \n"
			 "out.a = 0.0;                                                 \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_PER_STAGE_CONSTANTS_NV);
		glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE0, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glEndList();
	}

	 if (!data->rc_sum__peel_display_list)
	 {
		 data->rc_sum__peel_display_list = glGenLists(1);
		 glNewList(data->rc_sum__peel_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "{ rgb { spare0 = unsigned_invert(tex3) * col0.a; } }         \n"
			 "out.rgb = col0 + col1;                                       \n"
			 "out.a = spare0.b;                                            \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE3, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_PRIMARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_PER_STAGE_CONSTANTS_NV);
		glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glEndList();
	} 

	 if (!data->rc_sum_display_list)
	 {
		 data->rc_sum_display_list = glGenLists(1);
		 glNewList(data->rc_sum_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "out.rgb = col0 + col1;                                       \n"
			 "out.a = col0;                                                \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_PER_STAGE_CONSTANTS_NV);
		glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glEndList();
	} 

	 if (!data->rc_highlight_adds_alpha_display_list)
	 {
		 GLfloat const0[4] = {.2, .35, .2, 0};
		 GLfloat const1[4] = {0, 0, 1, 1};

		 data->rc_highlight_adds_alpha_display_list = glGenLists(1);
		 glNewList(data->rc_highlight_adds_alpha_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "const0 = (.2, .35, .2, 0);                                   \n"
			 "const1 = (0, 0, 1, 1);                                       \n"
			 "{ rgb { spare1 = col1 . const0; } }                          \n"
			 "{ alpha { discard = const1 * spare1.b; discard = const1.b * col0; col0 = sum(); } }\n"
			 "out.rgb = col0 + col1;                                       \n"
			 "out.a = col0;                                                \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, const0);
		glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV, const1);
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 2);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SECONDARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_CONSTANT_COLOR0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_CONSTANT_COLOR1_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_SPARE1_NV, GL_SIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_CONSTANT_COLOR1_NV, GL_SIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_PRIMARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerOutputNV(GL_COMBINER1_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_PRIMARY_COLOR_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_PER_STAGE_CONSTANTS_NV);
		glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		glEndList();
	}

	 if (!data->rc_highlight_adds_alpha__peel_display_list)
	 {
		 GLfloat const0[4] = {.2, .35, .2, 0};
		 GLfloat const1[4] = {0, 0, 1, 1};

		 data->rc_highlight_adds_alpha__peel_display_list = glGenLists(1);
		 glNewList(data->rc_highlight_adds_alpha__peel_display_list, GL_COMPILE);
#if defined (OLD_CODE)
		 nvparse(
			 "!!RC1.0                                                      \n"
			 "const0 = (.2, .35, .2, 0);                                   \n"
			 "const1 = (0, 0, 1, 1);                                       \n"
			 "{ rgb { spare1 = col1 . const0; } }                          \n"
			 "{ alpha { discard = const1 * spare1.b; discard = const1.b * col0; col0 = sum(); } }\n"
			 "{ rgb { spare0 = unsigned_invert(tex3) * col0.a; } }         \n"
			 "out.rgb = col0 + col1;                                       \n"
			 "out.a = spare0.b;                                            \n"
			 );
		 nvparse_print_errors(stderr);
#endif /* defined (OLD_CODE) */
		glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, const0);
		glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV, const1);
		glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 3);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SECONDARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_CONSTANT_COLOR0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_CONSTANT_COLOR1_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_SPARE1_NV, GL_SIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_CONSTANT_COLOR1_NV, GL_SIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_PRIMARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerOutputNV(GL_COMBINER1_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_PRIMARY_COLOR_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE3, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_B_NV, GL_PRIMARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
		glCombinerInputNV(GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerInputNV(GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glCombinerOutputNV(GL_COMBINER2_NV, GL_RGB, GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glCombinerInputNV(GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerInputNV(GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glCombinerOutputNV(GL_COMBINER2_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//glDisable(GL_PER_STAGE_CONSTANTS_NV);
		//glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
		glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_FALSE, GL_UNSIGNED_INVERT_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_FALSE, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
		glEndList();
	} 

	LEAVE;

	return (return_code);
} /* order_independent_init_opengl */

static void render_scene_from_camera_view(int layer,
	struct Scene_viewer_rendering_data *rendering_data,
	struct Scene_viewer_order_independent_transparency_data *data,
	double *projection_matrix, double *modelview_matrix)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Draws one peeled layer of the scene.
==============================================================================*/
{
	float identity_matrix[16] = {1, 0, 0, 0,
										  0, 1, 0, 0,
										  0, 0, 1, 0,
										  0, 0, 0, 1};
	float blue_alpha_matrix[16] = {0, 0, 0, 0,
											 0, 0, 0, 0,
											 0, 0, 0, 0,
											 0, 0, 1, 0};


	ENTER(render_scene_from_camera_view);
	USE_PARAMETER(modelview_matrix);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);

	glPushMatrix();

	glLoadIdentity();
	glOrtho(0, data->viewport_width, 0, data->viewport_height, -1, 1);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	glLoadIdentity();

	/* set up texture shaders for per-pixel z/w calculation
		-- this more closely matches per-pixel r/q used for shadow mapping */
	glEnable(GL_TEXTURE_SHADER_NV);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, data->simple_1x1_uhilo_texture_id);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	eye_linear_texgen(identity_matrix);
	texgen(/*true*/1);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0, 0, .5f);
	glScalef(0, 0, .5f);
	glMultMatrixd(projection_matrix);
	glMatrixMode(GL_MODELVIEW);

	glActiveTexture(GL_TEXTURE2);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);
	glPushMatrix();
	glLoadIdentity();
	eye_linear_texgen(identity_matrix);
	texgen(/*true*/1);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMultMatrixf(blue_alpha_matrix);
	glMultMatrixd(projection_matrix);
	glMatrixMode(GL_MODELVIEW);
		
	glActiveTexture(GL_TEXTURE3);
	glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_RECTANGLE_NV);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);

	glActiveTexture(GL_TEXTURE0);

	if(layer > 0)
	{
		glActiveTexture(GL_TEXTURE3);
		glPushMatrix();
		glLoadIdentity();
		eye_linear_texgen(identity_matrix);
		glPopMatrix();
		texgen(/*true*/1);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(data->viewport_width, data->viewport_height, 1);
		glTranslatef(.5,.5,.5);
		glScalef(.5,.5,.5);
		glMultMatrixd(projection_matrix);

		glMatrixMode(GL_MODELVIEW);
		
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->ztex_texture_id);
		glEnable(GL_TEXTURE_RECTANGLE_NV);
		
		glActiveTexture(GL_TEXTURE0);
		glAlphaFunc(GL_GREATER, 0);
		glEnable(GL_ALPHA_TEST);
	}	

	verify_shader_config();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	if(layer > 0)
	{
		glCallList(data->rc_highlight_adds_alpha__peel_display_list);
	}
	else
	{
		glCallList(data->rc_highlight_adds_alpha_display_list);
	}

	Scene_viewer_call_next_renderer(rendering_data);

	if(layer > 0)
	{
		glActiveTexture(GL_TEXTURE3);
		glDisable(GL_TEXTURE_RECTANGLE_NV);
		texgen(/*false*/0);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_ALPHA_TEST);
	}

	glDisable(GL_TEXTURE_SHADER_NV);

	if(layer < data->number_of_layers - 1)
	{
		/* copy the z buffer unless this is the last peeling pass */
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->ztex_texture_id);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0,
			data->viewport_width, data->viewport_height);
	}

	/* copy the RGBA of the layer */
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->rgba_layer_texture_id[layer]);
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0,
		data->viewport_width, data->viewport_height);

	LEAVE;

} /* render_scene_from_camera_view */
 
static void draw_sorted_transparency(
	struct Scene_viewer_order_independent_transparency_data *data,
	enum Scene_viewer_blending_mode blending_mode)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

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

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_REGISTER_COMBINERS_NV);
 	glDisable(GL_BLEND);

	/* The last layer contains the background without an alpha channel */
	glCallList(data->rc_composite_alphaone_display_list);

	glEnable(GL_TEXTURE_RECTANGLE_NV);
	if (data->using_stencil_overlay)
	{
		/* disable stencil buffer to get the overlay in the back plane */
		glDisable(GL_STENCIL_TEST);
	}

	for(i = data->number_of_layers-1 ; i >= 0 ; i--)
	{
		if ((data->number_of_layers-2) == i)
		{
			glCallList(data->rc_composite_display_list);
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
			if (data->using_stencil_overlay)
			{
				/* disable stencil buffer to get the overlay back*/
				glEnable(GL_STENCIL_TEST);
			}
		}

		glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->rgba_layer_texture_id[i]);
		
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

	glDisable(GL_TEXTURE_RECTANGLE_NV);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_REGISTER_COMBINERS_NV);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	LEAVE;

}
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

int order_independent_capable(void)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Returns true if the current display is capable of order independent transparency.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int alpha_bits, depth_bits;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */
	int return_code;
	unsigned int i;
	ENTER(order_independent_capable);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	return_code = 1;
	for (i = 0 ; return_code &&
		(i < (sizeof(required_extensions) / sizeof (char *))) ; i++)
	{
		if (!query_gl_extension(required_extensions[i]))
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
			"It requries at least 8 alpha bits, 16 or 24 bit depth buffer and "
			"these OpenGL extensions: ");
		for (i = 0 ; (i < (sizeof(required_extensions) / sizeof (char *))) ; i++)
		{
			display_message(ERROR_MESSAGE,"%s ", required_extensions[i]);
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
   order_independent_initialise(void)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Initialises the order independent transparency extension.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int alpha_bits, depth_bits, return_code;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */
	struct Scene_viewer_order_independent_transparency_data *data;

	ENTER(order_independent_initialise);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	if (ALLOCATE(data, struct Scene_viewer_order_independent_transparency_data,
		1))
	{
		return_code = 1;

		data->simple_1x1_uhilo_texture_id = 0;
		data->ztex_texture_id = 0;
		data->rgba_layer_texture_id = (unsigned int *)NULL;

		data->quad_display_list = 0;
		data->geometry_display_list = 0;
		data->rc_highlight_adds_alpha_display_list = 0;
		data->rc_highlight_adds_alpha__peel_display_list = 0;
		data->rc_sum_display_list = 0;
		data->rc_sum__peel_display_list = 0;
		data->rc_composite_display_list = 0;
		data->rc_composite_alphaone_display_list = 0;

		data->viewport_width = 0;
		data->viewport_height = 0;

		data->number_of_layers = 0;
		data->maximum_number_of_layers = 0;

		data->zbuffer = (GLuint *)NULL;

		glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
		glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
		
		switch (depth_bits)
		{
			case 16:
			{
				data->depth_format = GL_DEPTH_COMPONENT16_SGIX;
			} break;
			case 24:
			{
				data->depth_format = GL_DEPTH_COMPONENT24_SGIX;
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

		order_independent_init_opengl(data);
	}
	else
	{
		display_message(ERROR_MESSAGE, "order_independent_initialise.  "
				"Unable to allocate data structure\n");
		data = (struct Scene_viewer_order_independent_transparency_data *)NULL;
	}
#else /* defined (ORDER_INDEPENDENT_CAPABLE) */
	data = (struct Scene_viewer_order_independent_transparency_data *)NULL;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	LEAVE;

	return (data);
} /* order_independent_initialise */

int order_independent_reshape(
	struct Scene_viewer_order_independent_transparency_data *data,
	int width, int height, int layers, int using_stencil_overlay)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

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

		glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->ztex_texture_id);
		
		if (REALLOCATE(data->zbuffer, data->zbuffer, GLuint, width * height))
		{
			data->viewport_width = width;
			data->viewport_height = height;
			glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, data->depth_format, 
				width, height, 0, 
				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data->zbuffer);
		}
		else
		{
			display_message(ERROR_MESSAGE, "order_independent_reshape.  "
				"Unable to allocate ztex buffer\n");
			return_code = 0;
		}

		if (return_code && (!data->rgba_layer_texture_id ||
			(layers > data->maximum_number_of_layers)))
		{
			if (REALLOCATE(data->rgba_layer_texture_id, data->rgba_layer_texture_id,
				unsigned int, layers))
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
				glBindTexture(GL_TEXTURE_RECTANGLE_NV, data->rgba_layer_texture_id[i]);
				glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, width, height, 0, 
					GL_RGBA, GL_UNSIGNED_BYTE, data->zbuffer);
				glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Actually preforms the rendering pass.
==============================================================================*/
{
#if defined (ORDER_INDEPENDENT_CAPABLE)
	int layer;
#endif /* defined (ORDER_INDEPENDENT_CAPABLE) */

	ENTER(order_independent_display);

#if defined (ORDER_INDEPENDENT_CAPABLE)
	//	glViewport(0, 0, data->viewport_width, data->viewport_height);

	/* Copy the image that is already drawn into the back layer */
	glBindTexture(GL_TEXTURE_RECTANGLE_NV,
		data->rgba_layer_texture_id[data->number_of_layers - 1]);
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0,
		data->viewport_width, data->viewport_height);

	/* Always create the textures with this, use the blending mode
		to composite the images together */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	glEnable(GL_REGISTER_COMBINERS_NV);

	for(layer = 0; layer < data->number_of_layers - 1 ; layer++) 
	{
		render_scene_from_camera_view(layer, rendering_data, data,
			projection_matrix, modelview_matrix);
	}
	draw_sorted_transparency(data, blending_mode);

	// glCallList(data->rc_sum_display_list);
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
LAST MODIFIED : 14 April 2003

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
		if (data->simple_1x1_uhilo_texture_id)
		{
			glDeleteTextures(1, &data->simple_1x1_uhilo_texture_id);
		}

		if (data->rc_composite_display_list)
		{
			glDeleteLists(data->rc_composite_display_list, 1);
		}
		if (data->rc_composite_alphaone_display_list)
		{
			glDeleteLists(data->rc_composite_alphaone_display_list, 1);
		}
		if (data->rc_sum__peel_display_list)
		{
			glDeleteLists(data->rc_sum__peel_display_list, 1);
		}
		if (data->rc_sum_display_list)
		{
			glDeleteLists(data->rc_sum_display_list, 1);
		}
		if (data->rc_highlight_adds_alpha_display_list)
		{
			glDeleteLists(data->rc_highlight_adds_alpha_display_list, 1);
		}
		if (data->rc_highlight_adds_alpha__peel_display_list)
		{
			glDeleteLists(data->rc_highlight_adds_alpha__peel_display_list, 1);
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
