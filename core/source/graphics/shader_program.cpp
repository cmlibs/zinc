#include "opencmiss/zinc/zincconfigure.h"
#include "opencmiss/zinc/status.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/shader_program.hpp"
#include "graphics/graphics_library.h"
#include "graphics/texture.h"
#include "three_d_drawing/abstract_graphics_buffer.h"
#include "graphics/render_gl.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/indexed_list_stl_private.hpp"

/*
Module types
------------
*/
FULL_DECLARE_INDEXED_LIST_TYPE(Material_program_uniform);

FULL_DECLARE_INDEXED_LIST_TYPE(Material_program);


#if defined (OPENGL_API)
int Material_program_compile(struct Material_program *material_program,
	Render_graphics_opengl *renderer)
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
/* #define DEBUG_CODE */
#if defined (DEBUG_CODE)
		const GLubyte *error_msg;
#endif /* defined (DEBUG_CODE) */
		return_code = 1;
#if defined (TESTING_PROGRAM_STRINGS)
		/* If testing always recompile */
		material_program->compiled = 0;
#endif /* defined (TESTING_PROGRAM_STRINGS) */
		if (!material_program->compiled)
		{
#if (defined GL_ARB_vertex_program && defined GL_ARB_fragment_program) || defined GL_VERSION_2_0
#if defined (GL_VERSION_2_0)
			 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_ARB)
			 {
				 if (Graphics_library_check_extension(GL_shading_language))
				 {
					 material_program->shader_type=MATERIAL_PROGRAM_SHADER_GLSL;
				 }
			 }
#endif
#if defined GL_ARB_fragment_program && defined GL_ARB_vertex_program
			 if (material_program->shader_type!=MATERIAL_PROGRAM_SHADER_GLSL)
			 {
				 if (Graphics_library_check_extension(GL_ARB_fragment_program) &&
				 Graphics_library_check_extension(GL_ARB_vertex_program))
				 {
					 material_program->shader_type=MATERIAL_PROGRAM_SHADER_ARB;
				 }
			 }
#endif
			 if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL ||
					 material_program->shader_type==MATERIAL_PROGRAM_SHADER_ARB)
			 {
#if defined DEBUG_CODE
				printf ("Compiling program type:%x\n", material_program->type);
#endif /* defined DEBUG_CODE */
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

#if defined DEBUG_CODE
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
#endif  /* defined DEBUG_CODE */

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
								 "  vec3 aux = gl_LightSource[0].position.xyz - ecPos.xyz;\n"
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
										"  gl_TexCoord[3].xyz = normalize((gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal,0.0)).xyz);\n"
										"  gl_TexCoord[2].xyz = vec3(normalize(-ecPos.xyz));\n"
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
										 "  if (!gl_FrontFacing)\n"
										 "    n.z = -1.0 * n.z;\n"
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
							 number_of_inputs = static_cast<int>(strlen(components_string));
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
							static_cast<GLsizei>(strlen(vertex_program_string)), vertex_program_string);
#if defined (DEBUG_CODE)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  Vertex Result: %s\n", error_msg);
#endif /* defined (DEBUG_CODE) */
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
							static_cast<GLsizei>(strlen(fragment_program_string)), fragment_program_string);
#if defined (DEBUG_CODE)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  Fragment Result: %s\n", error_msg);
#endif /* defined (DEBUG_CODE) */
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
#if defined (DEBUG_CODE)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  test.vp Vertex Result: %s", error_msg);
#endif /* defined (DEBUG_CODE) */

						if (!material_program->fragment_program)
						{
							glGenProgramsARB(1, &material_program->fragment_program);
						}

						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, material_program->fragment_program);
						glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(fragment_program_string), fragment_program_string);
#if defined (DEBUG_CODE)
						error_msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
						display_message(WARNING_MESSAGE,
							"Material_program_compile.  test.fp Fragment Result: %s", error_msg);
#endif /* defined (DEBUG_CODE) */
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

#if !defined (DEBUG_CODE)
						// If DEBUG_CODE is defined always write the program info, otherwise
						// write the program info only when one of the shaders fails to compile.
						if (!vertexShaderCompiled || !fragmentShaderCompiled || !geometryShaderCompiled)
#endif // !defined (DEBUG_CODE)
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
							if (material_program->geometry_program)
							{
								glGetShaderiv(material_program->geometry_program, GL_INFO_LOG_LENGTH,&infologLength);
								if (infologLength > 0)
								{
									infoLog = (char *)malloc(infologLength);
									glGetShaderInfoLog(material_program->geometry_program,
										infologLength, &charsWritten, infoLog);
									display_message(INFORMATION_MESSAGE,"Geometry program info:\n%s\n",infoLog);
									free(infoLog);
								}
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
						if (renderer->use_display_list)
						{
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
					}
					else
					{
						if (renderer->use_display_list)
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

int Material_program_execute(struct Material_program *material_program,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	if (material_program)
	{
		if (material_program->compiled)
		{
			if (renderer->use_display_list)
			{
				if (material_program->display_list)
				{
					glCallList(material_program->display_list);
				}
			}
			else
			{
				if (material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL)
				{
					if (material_program->glsl_current_program)
					{
						GLint linked= 0;
						glGetProgramiv(material_program->glsl_current_program, GL_LINK_STATUS, &linked);
						if (linked)
						{
							glUseProgram(material_program->glsl_current_program);
						}
						else
						{
							glLinkProgram(material_program->glsl_current_program);
							glUseProgram(material_program->glsl_current_program);
						}
						glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
					}
				}
				else
				{
					if (material_program->vertex_program && material_program->fragment_program)
					{
						glEnable(GL_VERTEX_PROGRAM_ARB);
						glBindProgramARB(GL_VERTEX_PROGRAM_ARB,
								material_program->vertex_program);

						glEnable(GL_FRAGMENT_PROGRAM_ARB);
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
								material_program->fragment_program);
						glEnable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
					}
				}
			}
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Material_program_execute.  Missing material_program object.");
		return_code = 0;
	}

	return (return_code);
} /* Material_program_execute */

int Material_program_execute_textures(struct Material_program *material_program,
	struct Texture *texture, struct Texture *second_texture,
	struct Texture *third_texture)
{
#if defined (GL_VERSION_2_0)
	if (material_program && material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL &&
			material_program->glsl_current_program)
	{
		 if (texture)
		 {
			Texture_execute_vertex_program_environment(texture,
					material_program->glsl_current_program);
		 }
		 if (second_texture)
		 {
			Texture_execute_vertex_program_environment(second_texture,
					material_program->glsl_current_program);

		 }
		 if (third_texture)
		 {
			Texture_execute_vertex_program_environment(third_texture,
				 material_program->glsl_current_program);
		 }
		 return 1;
	}
#endif
	return 0;
}
static int Material_program_uniform_write_glsl_values(Material_program_uniform *uniform,
	void *material_program_void)
{
#if defined (GL_VERSION_2_0)
	int return_code;
	struct Material_program *material_program;
	if (uniform && (material_program = static_cast<Material_program*>(material_program_void)))
	{
		GLint location = glGetUniformLocation(material_program->glsl_current_program,
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
				default:
				{
				} break;
			}
		}
		return 1;
	}
#  endif // defined (GL_VERSION_2_0)
	return 0;
}


int Material_program_execute_uniforms(struct Material_program *material_program,
		LIST(Material_program_uniform) *material_program_uniforms)
{
#if defined (GL_VERSION_2_0)
	if (material_program && material_program->shader_type==MATERIAL_PROGRAM_SHADER_GLSL &&
			material_program->glsl_current_program && material_program_uniforms)
	{
	 if (glIsProgram(material_program->glsl_current_program))
	 {
		GLint loc1 = -1;
		loc1 = glGetUniformLocation(material_program->glsl_current_program,"texture2");
		if (loc1 != (GLint)-1)
			 glUniform1i(loc1,2);
		loc1 = glGetUniformLocation(material_program->glsl_current_program,"texture1");
		if (loc1 != (GLint)-1)
			 glUniform1i(loc1,1);
		loc1 = glGetUniformLocation(material_program->glsl_current_program,"texture0");
		if (loc1 != (GLint)-1)
			 glUniform1i(loc1, 0);
		if (material_program_uniforms)
			FOR_EACH_OBJECT_IN_LIST(Material_program_uniform)(
					Material_program_uniform_write_glsl_values, material_program,
					material_program_uniforms);
		return 1;
	 }

	}
#endif
	return 0;
}

#endif /* defined (OPENGL_API) */

/*
 * Misusing the double array here as the vector parser function gives us an
 * array of doubles and I don't see the need to copy and pass floats.
 * It isn't called double_vector then because we are going to use it with Uniform?f
 */
int Material_program_uniform_set_float_vector(Material_program_uniform *uniform,
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

struct Material_program *CREATE(Material_program)(enum Material_program_type type)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/
{
	struct Material_program *material_program;

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

	return (material_program);
} /* CREATE(Material_program) */

struct Material_program *Material_program_create_from_program_strings(
	const char *vertex_program_string, const char *fragment_program_string,
	const char *geometry_program_string)
{
	struct Material_program *material_program;

	material_program = CREATE(Material_program)(MATERIAL_PROGRAM_SPECIFIED_STRINGS);
	if (material_program)
	{
#if defined (OPENGL_API)
		Material_program_set_vertex_string(material_program, vertex_program_string);

		Material_program_set_fragment_string(material_program,fragment_program_string);

		Material_program_set_geometry_string(material_program, geometry_program_string);

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

	return (material_program);
}

static int DESTROY(Material_program)(struct Material_program **material_program_address)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Frees the memory for the material_program.
==============================================================================*/
{
	int return_code;
	struct Material_program *material_program;

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

	return (return_code);
} /* DESTROY(Material_program) */


DECLARE_OBJECT_FUNCTIONS(Material_program)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Material_program, type, \
	enum Material_program_type, compare_int)
DECLARE_INDEXED_LIST_FUNCTIONS(Material_program)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Material_program, type,
	enum Material_program_type, compare_int)

struct Material_program_uniform *CREATE(Material_program_uniform)(char *name)
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
	const char *, strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Material_program_uniform)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Material_program_uniform, name,
	const char *, strcmp)

int cmzn_material_program_uniform_destroy(struct Material_program_uniform **material_program_uniform_address)
{
	if (material_program_uniform_address && *material_program_uniform_address)
	{
		DEACCESS(Material_program_uniform)(material_program_uniform_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

struct Material_program *cmzn_material_program_access(struct Material_program *material_program)
{
	if (material_program)
		return ACCESS(Material_program)(material_program);
	return 0;
}

int cmzn_material_program_destroy(struct Material_program **material_program_address)
{
	if (material_program_address && *material_program_address)
	{
		DEACCESS(Material_program)(material_program_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}


int Material_program_set_vertex_string(Material_program *material_program_to_be_modified,
	const char *vertex_program_string)
{
	if (material_program_to_be_modified && vertex_program_string)
	{
		if (material_program_to_be_modified->vertex_program_string)
		{
			DEALLOCATE(material_program_to_be_modified->vertex_program_string);
		}
		material_program_to_be_modified->vertex_program_string = duplicate_string(vertex_program_string);
		return CMZN_OK;
	}

	return CMZN_ERROR_ARGUMENT;
}

int Material_program_set_fragment_string(Material_program *material_program_to_be_modified,
	const char *fragment_program_string)
{
	if (material_program_to_be_modified && fragment_program_string)
	{
		if (material_program_to_be_modified->fragment_program_string)
		{
			DEALLOCATE(material_program_to_be_modified->fragment_program_string);
		}
		material_program_to_be_modified->fragment_program_string = duplicate_string(fragment_program_string);
		return CMZN_OK;
	}

	return CMZN_ERROR_ARGUMENT;
}

int Material_program_set_geometry_string(Material_program *material_program_to_be_modified,
	const char *geometry_program_string)
{
	if (material_program_to_be_modified && geometry_program_string)
	{
		if (material_program_to_be_modified->geometry_program_string)
		{
			DEALLOCATE(material_program_to_be_modified->geometry_program_string);
		}
		material_program_to_be_modified->geometry_program_string = duplicate_string(geometry_program_string);
		return CMZN_OK;
	}

	return CMZN_ERROR_ARGUMENT;
}
