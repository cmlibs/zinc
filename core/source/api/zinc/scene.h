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

#include "types/graphicsfilterid.h"
#include "types/regionid.h"
#include "types/sceneid.h"
#include "types/scenepickerid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * Get managed status of scene in its owning scene_module.
 * @see Cmiss_scene_set_managed
 *
 * @param scene  The scene to query.
 * @return  1 (true) if scene is managed, otherwise 0 (false).
 */
ZINC_API int Cmiss_scene_is_managed(Cmiss_scene_id scene);

/**
 * Set managed status of scene in its owning scene module.
 * If set (managed) the scene will remain indefinitely in the scene module even
 * if no external references are held.
 * If not set (unmanaged) the scene will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param scene  The scene to modify.
 * @param value  The new value for the managed flag: 0 or 1.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_scene_set_managed(Cmiss_scene_id scene,
	int value);

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
/***************************************************************************//**
 * Create a scene picker which user can use to define a picking volume and
 * find the onjects included in this volume.
 *
 * @param scene  Scene to create the scene picker for.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API Cmiss_scene_picker_id Cmiss_scene_create_picker(Cmiss_scene_id scene);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_SCENE_H__*/
