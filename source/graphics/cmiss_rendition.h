/*******************************************************************************
FILE : cmiss_rendition.h

==============================================================================*/
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

#if !defined (CMISS_RENDITION_H)
#define CMISS_RENDITION_H

#include "context/context.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/light.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
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
 * @param texture_manager  Texture manager
 * @return  If successfully constructed, return the Cmiss_rendition
 */
struct Cmiss_graphics_module *Cmiss_graphics_module_create(struct Context *context);

/***************************************************************************//** 
 * Return Graphical_material manager in the Cmiss_graphics_module.
 *
 * @param cmiss_graphics_module  the pointer to the cmiss_graphics_module
 * @return  the material manager in the graphics package if exists,
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
 * Return the texture manager in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  the texture manager in graphics module if successfully called, 
 *    otherwise NULL.
 */
struct MANAGER(Texture) *Cmiss_graphics_module_get_texture_manager(
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
 * Destroy Cmiss_graphics_module and clean up the memory it uses.
 *
 * @param cmiss_graphics_module_address  the address to the pointer of 
 *   the cmiss_graphics_packge to be deleted
 * @return  1 if successfully destroy cmiss_graphics_module, otherwise 0
 */
int DESTROY(Cmiss_graphics_module)(
	struct Cmiss_graphics_module **cmiss_graphics_module_address);

/***************************************************************************//**
 * Create a list of standard cmgui materials and store them as they are managed
 * by graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  1 if successfully create a list of standard materials into graphics
 *    module, otherwise 0.
 */
int Cmiss_graphics_module_create_standard_materials(
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Return the default list of GT _objects in graphics module.
 *
 * @param graphics_module  Pointer to a Graphics_module object.
 * @return  list of the GT_objects if successfully called, otherwise NULL.
 */
struct LIST(GT_object) * Cmiss_graphics_module_get_default_GT_object_list(
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
struct LIST(GT_object) *Cmiss_graphics_module_get_default_glyph_list(
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
#endif /* !defined (CMISS_RENDITION_H) */

