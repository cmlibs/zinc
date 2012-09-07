/*******************************************************************************
FILE : graphics_module.h

==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *LAST MODIFIED : 16 October 2008

DESCRIPTION :
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

#if !defined (GRAPHICS_MODULE_H)
#define GRAPHICS_MODULE_H

#include "api/cmiss_graphics_filter.h"
#include "general/object.h"
#include "context/context.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/light.h"
#include "region/cmiss_region.h"
#include "selection/element_point_ranges_selection.h"
#include "time/time_keeper.h"


struct Cmiss_graphics_module;

/***************************************************************************//**
 * Create Cmiss_rendition_graphics_module
 *
 * @param glyph_list  List of glyphs
 * @param graphical_material_manager  Material manager
 * @param default_font  Default font
 * @param light_manager  Light Manager
 * @param spectrum_manager  Spectrum manager
 * @param default_spectrum  Default spectrum
 * @return  If successfully constructed, return the Cmiss_rendition
 */
struct Cmiss_graphics_module *Cmiss_graphics_module_create(struct Context *context);

/***************************************************************************//**
 * Return Graphical_material manager in the Cmiss_graphics_module.
 *
 * @param cmiss_graphics_module  the pointer to the cmiss_graphics_module
 * @return  the material manager in the graphics_module if exists,
 *   otherwise NULL
 */
struct MANAGER(Graphical_material) *Cmiss_graphics_module_get_material_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the material package in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the material pacakage in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Material_package *Cmiss_graphics_module_get_material_package(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the manager of light in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of light in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Light) *Cmiss_graphics_module_get_light_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default light in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default light in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light *Cmiss_graphics_module_get_default_light(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the specrtrum manager in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the spectrum manager in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Spectrum) *Cmiss_graphics_module_get_spectrum_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default spectrum in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default spectrum in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Spectrum *Cmiss_graphics_module_get_default_spectrum(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default font package in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default font package if successfully called, otherwise NULL.
 */
struct Graphics_font_package *Cmiss_graphics_module_get_font_package(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default font in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default font if successfully called, otherwise NULL.
 */
struct Graphics_font *Cmiss_graphics_module_get_default_font(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default list of the glyphs stored in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default list of glyphs in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(GT_object) *Cmiss_graphics_module_get_default_glyph_manager(
		struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the scene manager of graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of scenes in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Scene) *Cmiss_graphics_module_get_scene_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default scene in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  default scene in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Scene *Cmiss_graphics_module_get_default_scene(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the light model manager of graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of light_models in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Light_model) *Cmiss_graphics_module_get_light_model_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default light model in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default light_models in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light_model *Cmiss_graphics_module_get_default_light_model(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the manager of tessellation objects in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of tessellation in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Cmiss_tessellation) *Cmiss_graphics_module_get_tessellation_manager(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default tessellation in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default tessellation in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Cmiss_tessellation *Cmiss_graphics_module_get_default_tessellation(
	struct Cmiss_graphics_module *graphics_module);

int Cmiss_graphics_module_set_time_keeper_internal(
		struct Cmiss_graphics_module *module, struct Time_keeper *time_keeper);

struct Time_keeper *Cmiss_graphics_module_get_time_keeper_internal(
	struct Cmiss_graphics_module *module);

struct Element_point_ranges_selection *Cmiss_graphics_module_get_element_point_ranges_selection(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Add a region with a rendition created by this graphics module object
 * into a list, so that graphics module can deaccess the rendition of the region
 * when the graphics module is being destroyed.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @param region  Pointer to a region.
 * @return  1 if successfully add region into a list, otherwise 0.
 */
int Cmiss_graphics_module_add_member_region(
	struct Cmiss_graphics_module *graphics_module, struct Cmiss_region *region);

/***************************************************************************//**
 * Remove a region which rendition is created by this graphics module object
 * from a list.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @param region  Pointer to a region.
 * @return  1 if successfully remove region from a list, otherwise 0.
 */
int Cmiss_graphics_module_remove_member_region(
		struct Cmiss_graphics_module *graphics_module, struct Cmiss_region *region);

/***************************************************************************//**
 * Return the manager of graphics_filter objects in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of graphics_filter in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Cmiss_graphics_filter) *Cmiss_graphics_module_get_filter_manager(
		struct Cmiss_graphics_module *graphics_module);

Cmiss_graphics_filter_id Cmiss_graphics_module_get_default_filter(
	struct Cmiss_graphics_module *graphics_module);
#endif /* !defined (GRAPHICS_MODULE_H) */
