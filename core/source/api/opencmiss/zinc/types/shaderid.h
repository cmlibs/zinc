/**
 * @file shaderid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SHADERID_H__
#define CMZN_SHADERID_H__

/**
 * @brief Module managing all shader objects.
 *
 * Module managing all shader objects. It maintains shader uniforms and
 * shader program which can be used by material to add special effects
 * to graphics.
 */
struct cmzn_shadermodule;
typedef struct cmzn_shadermodule * cmzn_shadermodule_id;

/**
 * @brief Zinc shader uniforms provide an object to wor with
 * opengl shaders uniforms.
 *
 * Zinc shader uniforms specify uniforms that will pass into the shaders
 * and used by vertex/fragment shaders if a variable with mataching name
 * is found.
 */
struct cmzn_shaderuniforms;
typedef struct cmzn_shaderuniforms * cmzn_shaderuniforms_id;

/**
 * @brief Zinc shader program provide an object to set opengl shaders.
 *
 * Zinc shader program provide an object to pass opengl shaders to the
 * GPU and visualisations pipeline, currently support vertex shader and
 * fragment shader.
 */
struct cmzn_shaderprogram;
typedef struct cmzn_shaderprogram * cmzn_shaderprogram_id;

#endif
