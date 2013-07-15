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
ZINC_API Cmiss_graphics_material_module_id Cmiss_graphics_module_get_material_module(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return an additional handle to the graphics module. Increments the
 * internal 'access count' of the module.
 *
 * @param graphics_module  Existing handle to the graphics module.
 * @return  Additional handle to graphics module.
 */
ZINC_API Cmiss_graphics_module_id Cmiss_graphics_module_access(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Destroy this handle to the graphics module. The graphics module itself will
 * only be destroyed when all handles to it are destroyed.
 *
 * @param graphics_module_address  Address of the graphics module handle to be
 * destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_module_destroy(
	Cmiss_graphics_module_id *graphics_module_address);

/**
 * Returns a handle to a scene viewer module
 * User interface must be enabled before this function can be called successfully.
 *
 *
 * @param graphics_module  The graphics module to request the module from.
 * @return The scene viewer module if successfully called otherwise NULL.
 */
ZINC_API Cmiss_scene_viewer_module_id Cmiss_graphics_module_get_scene_viewer_module(
	Cmiss_graphics_module_id graphics_module);

/**
 * Get the glyph module which stores static graphics for visualising points,
 * vectors, axes etc. Note on startup no glyphs are defined and glyph module
 * functions need to be called to set up standard glyphs.
 *
 * @param graphics_module  The graphics module to request manager from.
 * @return  Handle to the glyph module, or 0 on error. Up to caller to destroy.
 */
ZINC_API Cmiss_glyph_module_id Cmiss_graphics_module_get_glyph_module(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Get a scene of region from graphics module with an access_count incremented
 * by 1. Caller is responsible for calling Cmiss_scene_destroy to destroy the
 * reference to it.
 *
 * @param graphics_module  The module at which the scene will get its
 * graphics setting for.
 * @param region  The region at which the scene is representing for.
 * @return  Reference to the scene.
 */
ZINC_API Cmiss_scene_id Cmiss_graphics_module_get_scene(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id region);

/**
* Get the spectrum module which stores spectrum object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the spectrum module, or 0 on error. Up to caller to destroy.
*/
ZINC_API Cmiss_spectrum_module_id Cmiss_graphics_module_get_spectrum_module(
	Cmiss_graphics_module_id graphics_module);

/**
* Get the tessellation module which stores tessellation object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the tesselation module, or 0 on error. Up to caller to destroy.
*/
ZINC_API Cmiss_tessellation_module_id Cmiss_graphics_module_get_tessellation_module(
	Cmiss_graphics_module_id graphics_module);

/**
* Get the graphics filter module which stores graphics_filter object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the graphics filter module, or 0 on error. Up to caller to destroy.
*/
ZINC_API Cmiss_graphics_filter_module_id Cmiss_graphics_module_get_filter_module(
	Cmiss_graphics_module_id graphics_module);

/**
* Get the font module which stores font object.
*
* @param graphics_module  The graphics module to request module from.
* @return  Handle to the font module, or 0 on error. Up to caller to destroy.
*/
ZINC_API Cmiss_font_module_id Cmiss_graphics_module_get_font_module(
	Cmiss_graphics_module_id graphics_module);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_GRAPHICS_MODULE_H__*/
