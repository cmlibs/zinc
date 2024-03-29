/**
 * @file materialid.h
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_MATERIALID_H__
#define CMZN_MATERIALID_H__

/**
 * @brief Zinc materials specify colouring of graphics.
 *
 * Zinc materials specify colouring of graphics similarly to the original OpenGL
 * shading model with diffuse, ambient, emission and specular colours, shininess
 * and alpha/opacity. Image fields can be attached for texturing (and will be
 * used by OpenGL shaders once enabled in a future release).
 */
struct cmzn_material;
typedef struct cmzn_material * cmzn_material_id;

/**
 * @brief An iterator for looping through all the materials in a materialmodule.
 *
 * An iterator for looping through all the materials in a materialmodule.
 */
struct cmzn_materialiterator;
typedef struct cmzn_materialiterator * cmzn_materialiterator_id;

/**
 * @brief Module managing all materials.
 *
 * Module managing all materials. Note that only default and default selected
 * materials exist on start-up, so many users will want to define standard
 * materials early in their program. Standard materials include basic colours,
 * greys, bioengineering materials (bone, muscle, tissue), etc. Alternative and
 * additional materials can be defined using the API.
 * @see cmzn_materialmodule_define_standard_materials
 */
struct cmzn_materialmodule;
typedef struct cmzn_materialmodule * cmzn_materialmodule_id;

/**
 * Labels of material attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum cmzn_material_attribute
{
	CMZN_MATERIAL_ATTRIBUTE_INVALID = 0, /*!< Unspecified material attribute. */
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

/**
 * Bit flags summarising changes to a material or materials in the material module.
 */
enum cmzn_material_change_flag
{
	CMZN_MATERIAL_CHANGE_FLAG_NONE = 0,
	/*!< material(s) not changed */
	CMZN_MATERIAL_CHANGE_FLAG_ADD = 1,
	/*!< one or more materials added */
	CMZN_MATERIAL_CHANGE_FLAG_REMOVE = 2,
	/*!< one or more materials removed */
	CMZN_MATERIAL_CHANGE_FLAG_IDENTIFIER = 4,
	/*!< material identifier changed */
	CMZN_MATERIAL_CHANGE_FLAG_DEFINITION = 8,
	/*!< change to material attributes other than identifier. */
	CMZN_MATERIAL_CHANGE_FLAG_FULL_RESULT = 16,
	/*!< all resultant values of material changed. */
	CMZN_MATERIAL_CHANGE_FLAG_FINAL = 32768
	/*!< final notification: owning material module i.e. material module
	 * has been destroyed */
};

/**
 * Type for passing logical OR of #cmzn_material_change_flag
 */
typedef int cmzn_material_change_flags;

/**
 * @brief Manages individual user notification of changes with a material module.
 *
 * Manages individual user notification of changes with a material module.
 */
struct cmzn_materialmodulenotifier;
typedef struct cmzn_materialmodulenotifier *cmzn_materialmodulenotifier_id;

/**
 * @brief Information about changes to materials in the material module.
 *
 * Information about changes to materials in the material module,
 * sent with each callback from the materialmodule notifier.
 */
struct cmzn_materialmoduleevent;
typedef struct cmzn_materialmoduleevent *cmzn_materialmoduleevent_id;

typedef void(*cmzn_materialmodulenotifier_callback_function)(
	cmzn_materialmoduleevent_id event, void *client_data);

#endif
