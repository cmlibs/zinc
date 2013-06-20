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

#ifndef __CMISS_GRAPHICS_MATERIAL_H__
#define __CMISS_GRAPHICS_MATERIAL_H__

#include "types/fieldimageid.h"
#include "types/graphicsmaterialid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Labels of material attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum Cmiss_graphics_material_attribute
{
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_INVALID = 0,
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA = 1,
	/*!< Opacity of the material. Transparent or translucent objects has
	 * lower alpha values then an opaque ones. Minimum acceptable value is 0
	 * and maximum acceptable value is 1. Use attribute_real to get and set
	 * its values.
	 */
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT = 2,
	/*!< Ambient colour of the material. Ambient colour simulates the colour
	 * of the material when it does not receive direct illumination.
	 * Composed of RGB components. Use attribute_real3 to get and set its
	 * values. Minimum acceptable value is 0 and maximum acceptable value is 1.
	 */
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE = 3,
	/*!< Diffuse colour of the material. Diffuse colour response to light that
	 * comes from one direction and this colour scattered equally in all directions
	 * once the light hits it. Composed of RGB components. Use attribute_real3
	 * to get and set its values. Minimum acceptable value is 0 and maximum acceptable
	 * value is 1.
	 */
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION = 4,
	/*!< Emissive colour of the material. Emissive colour simulates colours
	 * that is originating from the material itself. Composed of RGB components.
	 * Use attribute_real3 to get and set its values. Minimum acceptable value is 0
	 * and maximum acceptable value is 1.
	 */
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS = 5,
	/*!< Shininess determines the brightness of the highlight. Minimum acceptable
	 * value is 0 and maximum acceptable value is 1. Use attribute_real to get and
	 * set its values.
	 */
	CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR = 6
	/*!< Specular colour of the material. Specular colour produces highlights.
	 * Unlike ambient and diffuse, specular colour depends on location of
	 * the viewpoint, it is brightest along the direct angle of reflection.
	 * Composed of RGB components. Use attribute_real3 to get and set its values.
	 * Minimum acceptable value is 0 and maximum acceptable value is 1.
	 */
};

/**
* Returns a new reference to the graphics_material module with reference count
* incremented. Caller is responsible for destroying the new reference.
*
* @param material_module  The graphics_material module to obtain a new reference to.
* @return  graphics_material module with incremented reference count.
*/
ZINC_API Cmiss_graphics_material_module_id Cmiss_graphics_material_module_access(
	Cmiss_graphics_material_module_id material_module);

/**
* Destroys this reference to the graphics_material module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param material_module_address  Address of handle to graphics_material module
*   to destroy.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_material_module_destroy(
	Cmiss_graphics_material_module_id *material_module_address);

/**
 * Create and return a handle to a new graphics_material.
 *
 * @param material_module  The handle to the graphics_material module the
 * graphics_material will belong to.
 * @return  Handle to the newly created graphics_material if successful, otherwise NULL.
 */
ZINC_API Cmiss_graphics_material_id Cmiss_graphics_material_module_create_material(
	Cmiss_graphics_material_module_id material_module);

/**
* Begin caching or increment cache level for this graphics_material module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see Cmiss_graphics_material_module_end_change
*
* @param material_module  The graphics_material_module to begin change cache on.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_material_module_begin_change(
	Cmiss_graphics_material_module_id material_module);

/**
* Decrement cache level or end caching of changes for the graphics_material module.
* Call Cmiss_graphics_material_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param material_module  The glyph_module to end change cache on.
* @return  Status CMISS_OK on success, any other value on failure.
*/
ZINC_API int Cmiss_graphics_material_module_end_change(
	Cmiss_graphics_material_module_id material_module);

/**
* Find the graphics_material with the specified name, if any.
*
* @param material_module  graphics_material module to search.
* @param name  The name of the graphics_material.
* @return  Handle to the graphics_material of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_graphics_material_id Cmiss_graphics_material_module_find_material_by_name(
	Cmiss_graphics_material_module_id material_module, const char *name);

/**
* Get the default graphics_material, if any.
*
* @param material_module  graphics_material module to query.
* @return  Handle to the default graphics_material, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_graphics_material_id Cmiss_graphics_material_module_get_default_material(
	Cmiss_graphics_material_module_id material_module);

/**
* Set the default graphics_material.
*
* @param material_module  graphics_material module to modify
* @param material  The graphics_material to set as default.
* @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_material_module_set_default_material(
	Cmiss_graphics_material_module_id material_module,
	Cmiss_graphics_material_id material);

/**
* Get the default selected graphics_material, if any.
*
* @param material_module  graphics_material module to query.
* @return  Handle to the default graphics_material, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_graphics_material_id Cmiss_graphics_material_module_get_default_selected_material(
	Cmiss_graphics_material_module_id material_module);

/**
* Set the default selected graphics_material.
*
* @param material_module  graphics_material module to modify
* @param material  The graphics_material to set as default.
* @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_material_module_set_default_selected_material(
	Cmiss_graphics_material_module_id material_module,
	Cmiss_graphics_material_id material);

/***************************************************************************//**
 * Define a list of standard cmgui materials and store them as they are managed
 * by graphics module.
 *
 * @param material_module  Pointer to a Material_module object.
 * @return  Status CMISS_OK if successfully create a list of standard materials
 * into material module, any other value on failure.
 */
ZINC_API int Cmiss_graphics_material_module_define_standard_materials(
	Cmiss_graphics_material_module_id material_module);

/***************************************************************************//**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param attribute_name  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_graphics_material_attribute
	Cmiss_graphics_material_attribute_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_graphics_material_attribute_enum_to_string(
	enum Cmiss_graphics_material_attribute attribute);

/**
 * Get managed status of graphics material in its owning graphics material module.
 * @see Cmiss_graphics_material_set_managed
 *
 * @param graphics material  The graphics material to query.
 * @return  1 (true) if graphics material is managed, otherwise 0 (false).
 */
ZINC_API int Cmiss_graphics_material_is_managed(Cmiss_graphics_material_id material);

/**
 * Set managed status of graphics material in its owning graphics material module.
 * If set (managed) the graphics material will remain indefinitely in the
 * graphics material module even if no external references are held.
 * If not set (unmanaged) the graphics material will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param graphics material  The graphics material to modify.
 * @param value  The new value for the managed flag: 0 or 1.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphics_material_set_managed(Cmiss_graphics_material_id material,
	int value);

/***************************************************************************//**
 * Access the material, increase the access count of the material by one.
 *
 * @param material  handle to the "to be access" cmiss material.
 * @return  handle to material if successfully access material.
 */
ZINC_API Cmiss_graphics_material_id Cmiss_graphics_material_access(Cmiss_graphics_material_id material);

/***************************************************************************//**
 * Destroys this reference to the material (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param material  address to the handle to the "to be destroyed"
 *   cmiss material.
 * @return  Status CMISS_OK if successfully destroy material, any other value on
 * failure.
 */
ZINC_API int Cmiss_graphics_material_destroy(Cmiss_graphics_material_id *material);

/***************************************************************************//**
 * Get a real value of an attribute of the graphics material.
 *
 * @param material  Handle to the cmiss material.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double Cmiss_graphics_material_get_attribute_real(Cmiss_graphics_material_id material,
	enum Cmiss_graphics_material_attribute attribute);

/***************************************************************************//**
 * Set a real value for an attribute of the graphics material.
 *
 * @param material  Handle to the cmiss material.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this material object.
 */
ZINC_API int Cmiss_graphics_material_set_attribute_real(Cmiss_graphics_material_id material,
	enum Cmiss_graphics_material_attribute attribute, double value);

/***************************************************************************//**
 * Get a 3 components vectors of an attribute of the graphics material.
 * <values> should be allocated with enough space for 3 components.
 *
 * @param material  Handle to the cmiss material.
 * @param attribute  The identifier of the vectors attribute to get.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_material_get_attribute_real3(Cmiss_graphics_material_id material,
	enum Cmiss_graphics_material_attribute attribute, double *values);

/***************************************************************************//**
 * Set a 3 components vectors of an attribute for the graphics material.
 * <values> should be a vectors with 3 components containg valid values.
 *
 * @param material  Handle to the cmiss material.
 * @param attribute  The identifier of the vectors attribute to get.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this material object.
 */
ZINC_API int Cmiss_graphics_material_set_attribute_real3(Cmiss_graphics_material_id material,
	enum Cmiss_graphics_material_attribute attribute, const double *values);

/***************************************************************************//**
 * Return an allocated string containing material name.
 *
 * @param material  handle to the cmiss material.
 * @return  allocated string containing material name, otherwise NULL. Up to
 * caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_graphics_material_get_name(Cmiss_graphics_material_id material);

/***************************************************************************//**
 * Set/change name for <material>.
 *
 * @param material  The handle to the cmiss material.
 * @param name  name to be set to the material
 * @return  Status CMISS_OK if successfully set/change name for material,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_material_set_name(
	Cmiss_graphics_material_id material, const char *name);

/***************************************************************************//**
 * Set a cmiss_field containing an image for a Cmiss_graphics_material.
 * This image will be displayed with the material as the corresponding texture.
 * This function read in a general field but it may not work properly if
 * the field passing in is not an image field.
 *
 * @param material  handle to the cmiss material.
 * @param field  handle to a general cmiss field.
 * @param image_number  integer to identify which image field to be set in
 * 		material.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_material_set_image_field(Cmiss_graphics_material_id material,
	int image_number, Cmiss_field_image_id image_field);

/***************************************************************************//**
 * Get the cmiss_field containing an image from a Cmiss_graphics_material which is
 * used as a texture when displaying. The target image field is specified by the
 * identifier.
 *
 * @param material  handle to the cmiss material.
 * @param image_number  integer to identify which image field to get in material.
 * @return  cmiss_field if a field has been sett for the material, otherwise NULL.
 * 		This also incremenet the access count of the cmiss_field by 1;
 */
ZINC_API Cmiss_field_image_id Cmiss_graphics_material_get_image_field(
	Cmiss_graphics_material_id material, int image_number);

#ifdef __cplusplus
}
#endif

#endif
