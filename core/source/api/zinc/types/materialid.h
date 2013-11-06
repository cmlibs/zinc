/**
 * FILE : materialid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_MATERIALID_H__
#define CMZN_MATERIALID_H__

/**
 * A handle to zinc material. zinc material describes the
 * colour, shading and other graphical properties of a material, it is highly
 * similar to material described by OpenGL.
 * User can get a handle to material either through create new material using
 * cmzn_graphics_mateial_module_create_material or use existing materials in the
 * materialmodule provided by the context with
 * cmzn_materialmodule_find_material_by_name.
 * LibZinc also provide a number of preset materials in the default
 * graphics_packge.
 * Preset graphical materials are:
 * black, blue, bone, gray50, gold, green, muscle, red, silver, tissue,
 * transparent_gray50 and white.
 *
 * Please see available cmzn_material API functions belong for
 * configurable properties.
 */
	struct cmzn_material;
	typedef struct cmzn_material * cmzn_material_id;

	struct cmzn_materialmodule;
	typedef struct cmzn_materialmodule * cmzn_materialmodule_id;


	/**
	 * Labels of material attributes which may be set or obtained using generic
	 * get/set_attribute functions.
	 */
	enum cmzn_material_attribute
	{
		CMZN_MATERIAL_ATTRIBUTE_INVALID = 0,
		CMZN_MATERIAL_ATTRIBUTE_ALPHA = 1,
		/*!< Opacity of the material. Transparent or translucent objects has
		 * lower alpha values then an opaque ones. Minimum acceptable value is 0
		 * and maximum acceptable value is 1. Use attribute_real to get and set
		 * its values.
		 */
		CMZN_MATERIAL_ATTRIBUTE_AMBIENT = 2,
		/*!< Ambient colour of the material. Ambient colour simulates the colour
		 * of the material when it does not receive direct illumination.
		 * Composed of RGB components. Use attribute_real3 to get and set its
		 * values. Minimum acceptable value is 0 and maximum acceptable value is 1.
		 */
		CMZN_MATERIAL_ATTRIBUTE_DIFFUSE = 3,
		/*!< Diffuse colour of the material. Diffuse colour response to light that
		 * comes from one direction and this colour scattered equally in all directions
		 * once the light hits it. Composed of RGB components. Use attribute_real3
		 * to get and set its values. Minimum acceptable value is 0 and maximum acceptable
		 * value is 1.
		 */
		CMZN_MATERIAL_ATTRIBUTE_EMISSION = 4,
		/*!< Emissive colour of the material. Emissive colour simulates colours
		 * that is originating from the material itself. Composed of RGB components.
		 * Use attribute_real3 to get and set its values. Minimum acceptable value is 0
		 * and maximum acceptable value is 1.
		 */
		CMZN_MATERIAL_ATTRIBUTE_SHININESS = 5,
		/*!< Shininess determines the brightness of the highlight. Minimum acceptable
		 * value is 0 and maximum acceptable value is 1. Use attribute_real to get and
		 * set its values.
		 */
		CMZN_MATERIAL_ATTRIBUTE_SPECULAR = 6
		/*!< Specular colour of the material. Specular colour produces highlights.
		 * Unlike ambient and diffuse, specular colour depends on location of
		 * the viewpoint, it is brightest along the direct angle of reflection.
		 * Composed of RGB components. Use attribute_real3 to get and set its values.
		 * Minimum acceptable value is 0 and maximum acceptable value is 1.
		 */
	};

#endif
