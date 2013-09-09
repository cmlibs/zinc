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


struct cmzn_graphics_module;

/***************************************************************************//**
 * Create cmzn_scene_graphics_module
 *
 * @param glyph_list  List of glyphs
 * @param graphical_material_manager  Material manager
 * @param default_font  Default font
 * @param light_manager  Light Manager
 * @param spectrum_manager  Spectrum manager
 * @param default_spectrum  Default spectrum
 * @return  If successfully constructed, return the cmzn_scene
 */
struct cmzn_graphics_module *cmzn_graphics_module_create(struct Context *context);

/***************************************************************************//**
 * Return the light module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the light module in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light_module *cmzn_graphics_module_get_light_module(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the specrtrum manager in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the spectrum manager in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(Spectrum) *cmzn_graphics_module_get_spectrum_manager(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default font in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the default font if successfully called, otherwise NULL.
 */
struct cmzn_font *cmzn_graphics_module_get_default_font(
	struct cmzn_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the light model module in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the light model module in graphics module if successfully called,
 *    otherwise NULL.
 */
struct Light_model_module *cmzn_graphics_module_get_light_model_module(
	struct cmzn_graphics_module *graphics_module);

struct cmzn_time_keeper *cmzn_graphics_module_get_time_keeper_internal(
	struct cmzn_graphics_module *module);

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

/***************************************************************************//**
 * Return the manager of graphics_filter objects in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the manager of graphics_filter in graphics module if successfully called,
 *    otherwise NULL.
 */
struct MANAGER(cmzn_graphics_filter) *cmzn_graphics_module_get_filter_manager(
		struct cmzn_graphics_module *graphics_module);

struct MANAGER(cmzn_font) *cmzn_graphics_module_get_font_manager(
	struct cmzn_graphics_module *graphics_module);

/**
 * this function will remove most of the callbacks in scenes belong to this
 * module. This function should only be called when context is being destroyed.
 */
void cmzn_graphics_module_remove_external_callback_dependency(
	struct cmzn_graphics_module *graphics_module);

#endif /* !defined (GRAPHICS_MODULE_H) */
