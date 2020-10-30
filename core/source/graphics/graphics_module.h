/*******************************************************************************
FILE : graphics_module.h

==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (GRAPHICS_MODULE_H)
#define GRAPHICS_MODULE_H

#include "opencmiss/zinc/font.h"
#include "opencmiss/zinc/scenefilter.h"
#include "opencmiss/zinc/shader.h"
#include "general/object.h"
#include "context/context.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/light.hpp"
#include "graphics/scene.h"
#include "region/cmiss_region.hpp"
#include "selection/element_point_ranges_selection.h"
#include "time/time_keeper.hpp"


struct cmzn_graphics_module;

/***************************************************************************//**
 * Create cmzn_scene_graphics_module
 *
 * @param glyph_list  List of glyphs
 * @param graphical_material_manager  Material manager
 * @param default_font  Default font
 * @param light_manager  cmzn_light Manager
 * @param spectrum_manager  Spectrum manager
 * @param default_spectrum  Default spectrum
 * @return  If successfully constructed, return the cmzn_scene
 */
struct cmzn_graphics_module *cmzn_graphics_module_create(cmzn_context *context);

/***************************************************************************//**
 * Return an additional handle to the graphics module. Increments the
 * internal 'access count' of the module.
 *
 * @param graphics_module  Existing handle to the graphics module.
 * @return  Additional handle to graphics module.
 */
cmzn_graphics_module * cmzn_graphics_module_access(
	cmzn_graphics_module * graphics_module);

/***************************************************************************//**
 * Destroy this handle to the graphics module. The graphics module itself will
 * only be destroyed when all handles to it are destroyed.
 *
 * @param graphics_module_address  Address of the graphics module handle to be
 * destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_graphics_module_destroy(
	cmzn_graphics_module * *graphics_module_address);

/***************************************************************************//**
 * Return the light module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the light module in graphics module if successfully called,
 *    otherwise NULL.
 */
struct cmzn_lightmodule *cmzn_graphics_module_get_lightmodule(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the spectrum manager in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the spectrum manager in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(cmzn_spectrum) *cmzn_graphics_module_get_spectrum_manager(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default font in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default font if successfully called, otherwise NULL.
 */
struct cmzn_font *cmzn_graphics_module_get_default_font(
	struct cmzn_graphics_module *graphics_module);

/**
 * Return the timekeeper module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the timekeeper module in graphics module if successfully called,
 *    otherwise NULL. Up to caller to destroy.
 */
cmzn_timekeepermodule *cmzn_graphics_module_get_timekeepermodule(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Add a region with a scene created by this graphics module object
 * into a list, so that graphics module can deaccess the scene of the region
 * when the graphics module is being destroyed.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @param region  Pointer to a region.
 * @return  1 if successfully add region into a list, otherwise 0.
 */
int cmzn_graphics_module_add_member_region(
	struct cmzn_graphics_module *graphics_module, struct cmzn_region *region);

/***************************************************************************//**
 * Remove a region which scene is created by this graphics module object
 * from a list.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @param region  Pointer to a region.
 * @return  1 if successfully remove region from a list, otherwise 0.
 */
int cmzn_graphics_module_remove_member_region(
		struct cmzn_graphics_module *graphics_module, struct cmzn_region *region);

/**
 * Return the manager of scenefilter objects in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of scenefilter in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(cmzn_scenefilter) *cmzn_graphics_module_get_scenefilter_manager(
		struct cmzn_graphics_module *graphics_module);

struct MANAGER(cmzn_font) *cmzn_graphics_module_get_font_manager(
	struct cmzn_graphics_module *graphics_module);

/**
 * this function will remove most of the callbacks in scenes belong to this
 * module. This function should only be called when context is being destroyed.
 */
void cmzn_graphics_module_remove_external_callback_dependency(
	struct cmzn_graphics_module *graphics_module);


/**
 * Returns a handle to the scene viewer module.
 *
 * @param graphics_module  The graphics module to request the module from.
 * @return The scene viewer module if successfully called, otherwise NULL.
 */
cmzn_sceneviewermodule_id cmzn_graphics_module_get_sceneviewermodule(
	cmzn_graphics_module * graphics_module);

/**
 * Get the glyph module which stores static graphics for visualising points,
 * vectors, axes etc. Note on startup no glyphs are defined and glyph module
 * functions need to be called to set up standard glyphs.
 *
 * @param graphics_module  The graphics module to request manager from.
 * @return  Handle to the glyph module, or 0 on error. Up to caller to destroy.
 */
cmzn_glyphmodule_id cmzn_graphics_module_get_glyphmodule(
	cmzn_graphics_module * graphics_module);

/***************************************************************************//**
 * Get a scene of region from graphics module with an access_count incremented
 * by 1. Caller is responsible for calling cmzn_scene_destroy to destroy the
 * reference to it.
 *
 * @param graphics_module  The module at which the scene will get its
 * graphics setting for.
 * @param region  The region at which the scene is representing for.
 * @return  Reference to the scene.
 */
cmzn_scene_id cmzn_graphics_module_get_scene(
	cmzn_graphics_module * graphics_module, cmzn_region_id region);

/**
* Get the shader module which stores shaderuniforms and shaderprogram objects.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the shader module, or 0 on error. Up to caller to destroy.
*/
cmzn_shadermodule_id cmzn_graphics_module_get_shadermodule(
	cmzn_graphics_module * graphics_module);

/**
* Get the spectrum module which stores spectrum objects.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the spectrum module, or 0 on error. Up to caller to destroy.
*/
cmzn_spectrummodule_id cmzn_graphics_module_get_spectrummodule(
	cmzn_graphics_module * graphics_module);

/**
* Get the tessellation module which stores tessellation objects.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the tesselation module, or 0 on error. Up to caller to destroy.
*/
cmzn_tessellationmodule_id cmzn_graphics_module_get_tessellationmodule(
	cmzn_graphics_module * graphics_module);

/**
* Get the scene filter module which stores scenefilter objects.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the scene filter module, or 0 on error. Up to caller to destroy.
*/
cmzn_scenefiltermodule_id cmzn_graphics_module_get_scenefiltermodule(
	cmzn_graphics_module * graphics_module);

/**
* Get the font module which stores font object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the font module, or 0 on error. Up to caller to destroy.
*/
cmzn_fontmodule_id cmzn_graphics_module_get_fontmodule(
	cmzn_graphics_module * graphics_module);

/***************************************************************************//**
 * Return the material module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the material pacakage in graphics module if successfully called,
 *    otherwise NULL.
 */
cmzn_materialmodule_id cmzn_graphics_module_get_materialmodule(
	struct cmzn_graphics_module *graphics_module);

int cmzn_graphics_module_enable_scenes(
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *cmiss_region);

#endif /* !defined (GRAPHICS_MODULE_H) */
