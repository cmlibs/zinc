/*******************************************************************************
 * cmiss_scene.h
 *
 * Public interface to the Cmiss_scene object which represents a set of graphics
 * able to be output to the Cmiss_scene_viewer or other outputs/devices.
 * It broadly comprises a reference to a region sub-tree and filters controlling
 * which graphics are displayed from its renditions.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef __CMISS_SCENE_H__
#define __CMISS_SCENE_H__

#include "types/cmiss_graphics_filter_id.h"
#include "types/cmiss_region_id.h"
#include "types/cmiss_scene_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Labels of scene attributes which may be set or obtained using generic
 * get/set_attribute functions.
 * Note: not all attributes can be set.
 */
enum Cmiss_scene_attribute
{
	CMISS_SCENE_ATTRIBUTE_INVALID = 0,
	CMISS_SCENE_ATTRIBUTE_IS_MANAGED = 1,
	/*!< Boolean as integer, when 0 (default) scene is destroyed when no
	 * longer in use, i.e. when number of external references to it drops to
	 * zero. Set to 1 to manage scene object indefinitely, or until this
	 * attribute is reset to zero, effectively marking it as pending destruction.
	 */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_scene_attribute
	Cmiss_scene_attribute_enum_from_string(const char  *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_scene_attribute_enum_to_string(enum Cmiss_scene_attribute attribute);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the scene object.
 *
 * @param scene  Handle to the cmiss scene.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_scene_get_attribute_integer(Cmiss_scene_id scene,
	enum Cmiss_scene_attribute attribute);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the scene object.
 *
 * @param scene  Handle to the cmiss scene.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or able to be set for this scene object.
 */
ZINC_API int Cmiss_scene_set_attribute_integer(Cmiss_scene_id scene,
	enum Cmiss_scene_attribute attribute, int value);

/*******************************************************************************
 * Returns a new reference to the scene with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param scene  The scene to obtain a new reference to.
 * @return  New scene reference with incremented reference count.
 */
ZINC_API Cmiss_scene_id Cmiss_scene_access(Cmiss_scene_id scene);

/*******************************************************************************
* Destroys this reference to the scene (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param scene_address  The address to the handle of the scene to be destroyed.
* @return  Status CMISS_OK if scene is successfully destroyed, any other value
* on failure.
*/
ZINC_API int Cmiss_scene_destroy(Cmiss_scene_id *scene_address);

/***************************************************************************//**
 * Set the top region of the scene. Rendition from this top region and its child
 * region will be compiled into a display list for rendering.
 *
 * @param scene Handle to the scene to be edited
 * @param root_region Handle to the region to be set in the scene.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_scene_set_region(Cmiss_scene_id scene, Cmiss_region_id root_region);

/***************************************************************************//**
 * Return the name of the scene.
 *
 * @param scene  The scene whose name is requested.
 * @return  On success: allocated string containing scene name. Up to caller to
 * free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_scene_get_name(Cmiss_scene_id scene);

/***************************************************************************//**
 * Set the name of the scene.
 *
 * @param scene  The scene whose name is requested.
 * @param name  New name for scene.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_scene_set_name(Cmiss_scene_id scene, const char *name);

/***************************************************************************//**
 * Get the filter currently used in <scene>.
 *
 * @param scene  Scene to get the filters from.
 * @return  filter if successful, otherwise NULL.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_scene_get_filter(Cmiss_scene_id scene);

/***************************************************************************//**
 * Set the filter to be used in <scene>. No graphics will be shown until a filter
 * showing graphic is set for a scene.
 *
 * @param scene  Scene to set filter for.
 * @param filter  Filter to be set for scene.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_scene_set_filter(Cmiss_scene_id scene, Cmiss_graphics_filter_id filter);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_SCENE_H__*/
