/***************************************************************************//**
 * FILE : graphicsmaterialid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GRAPHICSMATERIALID_H__
#define CMZN_GRAPHICSMATERIALID_H__

/***************************************************************************//**
 * A handle to zinc material. zinc material describes the
 * colour, shading and other graphical properties of a material, it is highly
 * similar to material described by OpenGL.
 * User can get a handle to material either through create new material using
 * cmzn_graphics_mateial_module_create_material or use existing materials in the
 * material_module provided by the context with
 * cmzn_graphics_material_module_find_material_by_name.
 * LibZinc also provide a number of preset materials in the default
 * graphics_packge.
 * Preset graphical materials are:
 * black, blue, bone, gray50, gold, green, muscle, red, silver, tissue,
 * transparent_gray50 and white.
 *
 * Please see available cmzn_graphics_material API functions belong for
 * configurable properties.
 */
	struct cmzn_graphics_material;
	typedef struct cmzn_graphics_material * cmzn_graphics_material_id;

	struct cmzn_graphics_material_module;
	typedef struct cmzn_graphics_material_module * cmzn_graphics_material_module_id;

#endif
