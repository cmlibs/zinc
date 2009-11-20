/*******************************************************************************
FILE : cmiss_material.h

LAST MODIFIED : 05 Nov 2009

DESCRIPTION :
The public interface to the material.
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

#ifndef __CMISS_GRAPHICAL_MATERIAL_H__
#define __CMISS_GRAPHICAL_MATERIAL_H__

#include "api/cmiss_texture.h"

struct Cmiss_graphics_package;
#ifndef CMISS_GRAPHICS_PACKAGE_ID_DEFINED
typedef struct Cmiss_graphics_package * Cmiss_graphics_package_id;
#define CMISS_GRAPHICS_PACKAGE_ID_DEFINED
#endif /* CMISS_GRAPHICS_PACKAGE_ID_DEFINED */

struct Graphical_material;
#ifndef CMISS_GRAPHICAL_MATERIAL_ID_DEFINED
/***************************************************************************//**
 * A handle to cmiss graphical material. Cmiss graphical material describes the
 * colour, shading and other graphical properties of a material, it is highly 
 * similar to material described by OpenGL.
 * User can get a handle to material either through create new material using 
 * Cmiss_graphical_material_create or use existing materials in the 
 * graphics_package provided by the cmiss_command_data with 
 * Cmiss_graphical_material_get_with_name.
 * Cmgui also provide a number of preset materials in the default
 * graphics_packge.
 * Preset graphical materials are:
 * black, blue, bone, gray50, gold, green, muscle, red, silver, tissue,
 * transparent_gray50 and white. 
 *
 * Please see available Cmiss_graphic_material API functions belong for 
 * configuarble properties.
 */
typedef struct Graphical_material * Cmiss_graphical_material_id;
#define CMISS_GRAPHICAL_MATERIAL_ID_DEFINED
#endif /* CMISS_GRAPHICAL_MATERIAL_ID_DEFINED */

/***************************************************************************//**
 * Create a new cmiss graphical material. This new material will be returned and 
 * put in the Cmiss_graphic_package. User can also get the handle to the created
 * material through Cmiss_graphical_material_get_with_name function.
 *
 * @param graphics_package  The handle to cmiss grpahics package, it contains
 *   internal cmgui structure required by cmiss_graphical_material.
 * @param name  The name for the newly created material
 * @return  The newly created material if successfully created, otherwise NULL.
 */
Cmiss_graphical_material_id Cmiss_graphical_material_create(
	Cmiss_graphics_package_id graphics_package);

/***************************************************************************//**
 * Get the material with <name> from graphics_package if material with the
 * name provided exists in the graphics_package.
 *
 * @param graphics_package  The handle to cmiss grpahics package, it contains
 *   internal cmgui structure required by cmiss_graphical_material.
 * @param name  The name of the material.
 * @return  1 if successfully get material, otherwise 0.
 */
Cmiss_graphical_material_id Cmiss_graphical_material_get_with_name(
	Cmiss_graphics_package_id graphics_package, const char *name);

/***************************************************************************//**
 * Set/change name for <material>.
 *
 * @param material  The handle to cmiss grpahical material.
 * @param name  name to be set to the material
 * @return  1 if successfully set/change name for material, otherwise 0.
 */
int Cmiss_graphical_material_set_name(
	Cmiss_graphical_material_id material, const char *name);

/***************************************************************************//**
 * Set the alpha value (opacity) of the material.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param alpha  Opacity of the material. Transparent or translucent objects has
 *   lower alpha values then an opaque ones. Minimum acceptable value is 0
 *   and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_alpha(
	Cmiss_graphical_material_id material, float alpha);

/***************************************************************************//**
 * Set the size and brigtness of the highlight.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param shininess  Size and brightness of the hightlight, the higher the 
 *   value, the smaller and brighter the highlight. Minimum acceptable value is 0
 *   and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_shininess(
	Cmiss_graphical_material_id material, float shininess);

/***************************************************************************//**
 * Set the ambient colour of the material. Ambient colour simulates the colour 
 * of the material when it does not receive direct illumination.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param red  Value of the red component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param green  Value of the green component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param blue  Value of the blue component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_ambient(
	Cmiss_graphical_material_id material, float red, float green, float blue);

/***************************************************************************//**
 * Set the diffuse color of the material. Diffuse colour response to light that 
 * comes from one direction and this colour scattered equally in all directions 
 * once the light hits it.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param red  Value of the red component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param green  Value of the green component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param blue  Value of the blue component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_diffuse(
	Cmiss_graphical_material_id material, float red, float green, float blue);

/***************************************************************************//**
 * Set the emissive colour of the material. Emissive colour simulates colours
 * that is originating from the material itself. It is not affected by any
 * lighting.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param red  Value of the red component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param green  Value of the green component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param blue  Value of the blue component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_emission(
	Cmiss_graphical_material_id material, float red, float green, float blue);

/***************************************************************************//**
 * Set the specular colour of the material. Specular colour produces highlights.
 * Unlike ambient and diffuse reflect, specular colour depends on location of
 * the viewpoint, it is brightest along the direct angle of reflection.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param red  Value of the red component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param green  Value of the green component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @param blue  Value of the blue component to be set.
 *   Minimum acceptable value is 0 and maximum acceptable value is 1.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_specular(
	Cmiss_graphical_material_id material, float red, float green, float blue);

/***************************************************************************//**
 * Set the texture of the material.
 *
 * @param material  The handle to the to be modified cmiss graphical material.
 * @param texture  Handle to cmiss texture to be used for this material.
 * @return  1 if successfully modify material, otherwise 0.
 */
int Cmiss_graphical_material_set_texture(
	Cmiss_graphical_material_id material, Cmiss_texture_id texture);

/***************************************************************************//**
 * The volatile flag determine how the material is destroyed. If a material 
 * is volatile upon destroyed, it will also be removed from the graphics_package 
 * but if volatile flag is 0 then material will remain in the graphics_package.
 *
 * @param material  handle to the" cmiss graphical material.
 * @return  1 if successfully set the volatile flag, otherwise 0.
 */
int Cmiss_graphical_material_set_volatile(
	Cmiss_graphical_material_id material, int volatile_flag);

/***************************************************************************//**
 * Destroy the material.
 *
 * @param material  handle to the "to be destroyed" cmiss graphical material.
 * @return  1 if successfully destroy material, otherwise 0.
 */
int Cmiss_graphical_material_destroy(
	Cmiss_graphical_material_id *material);
#endif
