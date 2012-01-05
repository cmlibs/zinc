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

#ifndef __CMISS_GRAPHICS_MODULE_H__
#define __CMISS_GRAPHICS_MODULE_H__

#include "types/cmiss_graphics_filter_id.h"
#include "types/cmiss_graphics_material_id.h"
#include "types/cmiss_graphics_module_id.h"
#include "types/cmiss_region_id.h"
#include "types/cmiss_rendition_id.h"
#include "types/cmiss_scene_id.h"
#include "types/cmiss_spectrum_id.h"
#include "types/cmiss_tessellation_id.h"

/***************************************************************************//**
 * Find the material with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * material in.
 * @param name  The name of the material.
 * @return  Handle to the material with that name, or NULL if not found.
 */
Cmiss_graphics_material_id Cmiss_graphics_module_find_material_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

/***************************************************************************//**
 * Create and return a handle to a new graphics material.
 *
 * @param graphics_module  The handle to the graphics module the material will
 * belong to.
 * @return  Handle to the newly created material if successful, otherwise NULL.
 */
Cmiss_graphics_material_id Cmiss_graphics_module_create_material(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Return an additional handle to the graphics module. Increments the
 * internal 'access count' of the module.
 *
 * @param graphics_module  Existing handle to the graphics module.
 * @return  Additional handle to graphics module.
 */
Cmiss_graphics_module_id Cmiss_graphics_module_access(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Destroy this handle to the graphics module. The graphics module itself will
 * only be destroyed when all handles to it are destroyed.
 *
 * @param graphics_module_address  Address of the graphics module handle to be
 * destroyed. 
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_graphics_module_destroy(
	Cmiss_graphics_module_id *graphics_module_address);

/***************************************************************************//** 
 * Creates a scene with an access_count of 1. Caller is responsible for calling
 * Cmiss_scene_destroy to destroy the reference to it.
 *
 * @param graphics_module  The module to own the new scene.
 * @return  Reference to the newly created scene.
 */
Cmiss_scene_id Cmiss_graphics_module_create_scene(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Find the scene with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * 	spectrum in.
 * @param name  The name of the scene.
 * @return  Handle to the scene with the matching name, or NULL if not found.
 */
Cmiss_scene_id Cmiss_graphics_module_find_scene_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

/***************************************************************************//**
 * Get a rendition of region from graphics module with an access_count incremented
 * by 1. Caller is responsible for calling Cmiss_rendition_destroy to destroy the
 * reference to it.
 *
 * @param graphics_module  The module at which the rendition will get its
 * graphics setting for.
 * @param region  The region at which the rendition is representing for.
 * @return  Reference to the rendition.
 */
Cmiss_rendition_id Cmiss_graphics_module_get_rendition(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id region);

/***************************************************************************//**
 * Enable rendition of region and all of its children region from graphics
 * module. This will create rendition for the specified region and all its
 * children.
 *
 * @param graphics_module  The module requires for region to enable rendition.
 * @param region  The region at which the rendition will be enabled for.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_graphics_module_enable_renditions(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id region);

/***************************************************************************//**
 * Find the spectrum with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * spectrum in.
 * @param name  The name of the spectrum.
 * @return  Handle to the spectrum with that name, or NULL if not found.
 */
Cmiss_spectrum_id Cmiss_graphics_module_find_spectrum_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

/***************************************************************************//**
 * Create and return a handle to a new spectrum.
 *
 * @param graphics_module  The handle to the graphics module the spectrum will
 * belong to.
 * @return  Handle to the newly created spectrum if successful, otherwise NULL.
 */
Cmiss_spectrum_id Cmiss_graphics_module_create_spectrum(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Find the tessellation with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * tessellation in.
 * @param name  The name of the tessellation.
 * @return  Handle to the tessellation with that name, or NULL if not found.
 */
Cmiss_tessellation_id Cmiss_graphics_module_find_tessellation_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

/***************************************************************************//**
 * Create and return a handle to a new tessellation.
 *
 * @param graphics_module  The handle to the graphics module the tessellation will
 * belong to.
 * @return  Handle to the newly created tessellation if successful, otherwise NULL.
 */
Cmiss_tessellation_id Cmiss_graphics_module_create_tessellation(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Define a list of standard cmgui materials and store them as they are managed
 * by graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  Status CMISS_OK if successfully create a list of standard materials
 * into graphics module, any other value on failure.
 */
int Cmiss_graphics_module_define_standard_materials(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Find the graphics_filter with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * graphics_filter in.
 * @param name  The name of the graphics_filter.
 * @return  Handle to the graphics_filter with the provided name,
 * 		or NULL if not found.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_find_filter_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

#endif /*__CMISS_GRAPHICS_MODULE_H__*/
