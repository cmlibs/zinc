/***************************************************************************//**
 * FILE : graphicsmodule.h
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GRAPHICSMODULE_H__
#define CMZN_GRAPHICSMODULE_H__

#include "types/glyphid.h"
#include "types/fontid.h"
#include "types/graphicsfilterid.h"
#include "types/graphicsmaterialid.h"
#include "types/graphicsmoduleid.h"
#include "types/regionid.h"
#include "types/sceneid.h"
#include "types/sceneviewerid.h"
#include "types/spectrumid.h"
#include "types/tessellationid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Return the material module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the material pacakage in graphics module if successfully called,
 *    otherwise NULL.
 */
ZINC_API cmzn_graphics_material_module_id cmzn_graphics_module_get_material_module(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return an additional handle to the graphics module. Increments the
 * internal 'access count' of the module.
 *
 * @param graphics_module  Existing handle to the graphics module.
 * @return  Additional handle to graphics module.
 */
ZINC_API cmzn_graphics_module_id cmzn_graphics_module_access(
	cmzn_graphics_module_id graphics_module);

/***************************************************************************//**
 * Destroy this handle to the graphics module. The graphics module itself will
 * only be destroyed when all handles to it are destroyed.
 *
 * @param graphics_module_address  Address of the graphics module handle to be
 * destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_graphics_module_destroy(
	cmzn_graphics_module_id *graphics_module_address);

/**
 * Returns a handle to a scene viewer module
 * User interface must be enabled before this function can be called successfully.
 *
 *
 * @param graphics_module  The graphics module to request the module from.
 * @return The scene viewer module if successfully called otherwise NULL.
 */
ZINC_API cmzn_scene_viewer_module_id cmzn_graphics_module_get_scene_viewer_module(
	cmzn_graphics_module_id graphics_module);

/**
 * Get the glyph module which stores static graphics for visualising points,
 * vectors, axes etc. Note on startup no glyphs are defined and glyph module
 * functions need to be called to set up standard glyphs.
 *
 * @param graphics_module  The graphics module to request manager from.
 * @return  Handle to the glyph module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_glyph_module_id cmzn_graphics_module_get_glyph_module(
	cmzn_graphics_module_id graphics_module);

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
ZINC_API cmzn_scene_id cmzn_graphics_module_get_scene(
	cmzn_graphics_module_id graphics_module, cmzn_region_id region);

/**
* Get the spectrum module which stores spectrum object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the spectrum module, or 0 on error. Up to caller to destroy.
*/
ZINC_API cmzn_spectrum_module_id cmzn_graphics_module_get_spectrum_module(
	cmzn_graphics_module_id graphics_module);

/**
* Get the tessellation module which stores tessellation object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the tesselation module, or 0 on error. Up to caller to destroy.
*/
ZINC_API cmzn_tessellation_module_id cmzn_graphics_module_get_tessellation_module(
	cmzn_graphics_module_id graphics_module);

/**
* Get the graphics filter module which stores graphics_filter object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the graphics filter module, or 0 on error. Up to caller to destroy.
*/
ZINC_API cmzn_graphics_filter_module_id cmzn_graphics_module_get_filter_module(
	cmzn_graphics_module_id graphics_module);

/**
* Get the font module which stores font object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the font module, or 0 on error. Up to caller to destroy.
*/
ZINC_API cmzn_font_module_id cmzn_graphics_module_get_font_module(
	cmzn_graphics_module_id graphics_module);

#ifdef __cplusplus
}
#endif

#endif
