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

#include "zinc/font.h"
#include "zinc/graphicsfilter.h"
#include "general/object.h"
#include "context/context.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/light.h"
#include "graphics/scene.h"
#include "region/cmiss_region.h"
#include "selection/element_point_ranges_selection.h"
#include "time/time_keeper.hpp"


struct Cmiss_graphics_module;

/***************************************************************************//**
 * Create Cmiss_scene_graphics_module
 *
 * @param glyph_list  List of glyphs
 * @param graphical_material_manager  Material manager
 * @param default_font  Default font
 * @param light_manager  Light Manager
 * @param spectrum_manager  Spectrum manager
 * @param default_spectrum  Default spectrum
 * @return  If successfully constructed, return the Cmiss_scene
 */
struct Cmiss_graphics_module *Cmiss_graphics_module_create(struct Context *context);

/***************************************************************************//**
 * Return the light module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the light module in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light_module *Cmiss_graphics_module_get_light_module(
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
 * Return the default font in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default font if successfully called, otherwise NULL.
 */
struct Cmiss_font *Cmiss_graphics_module_get_default_font(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the light model module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the light model module in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light_model_module *Cmiss_graphics_module_get_light_model_module(
	struct Cmiss_graphics_module *graphics_module);

struct Cmiss_time_keeper *Cmiss_graphics_module_get_time_keeper_internal(
	struct Cmiss_graphics_module *module);

/***************************************************************************//**
 * Add a region with a scene created by this graphics module object
 * into a list, so that graphics module can deaccess the scene of the region
 * when the graphics module is being destroyed.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @param region  Pointer to a region.
 * @return  1 if successfully add region into a list, otherwise 0.
 */
int Cmiss_graphics_module_add_member_region(
	struct Cmiss_graphics_module *graphics_module, struct Cmiss_region *region);

/***************************************************************************//**
 * Remove a region which scene is created by this graphics module object
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

struct MANAGER(Cmiss_font) *Cmiss_graphics_module_get_font_manager(
	struct Cmiss_graphics_module *graphics_module);

/**
 * this function will remove most of the callbacks in scenes belong to this
 * module. This function should only be called when context is being destroyed.
 */
void Cmiss_graphics_module_remove_external_callback_dependency(
	struct Cmiss_graphics_module *graphics_module);

#endif /* !defined (GRAPHICS_MODULE_H) */
