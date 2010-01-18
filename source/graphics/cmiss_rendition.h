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
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/light.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "time/time_keeper.h"

#define CMISS_RENDITION_H

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
 * @param element_point_ranges_selection  Element point ranges selection
 * @param element_selection  Element selection
 * @param data_selection Data  selection
 * @param node_selection Node  selection
 * @return  If successfully constructed, return the Cmiss_rendition
 */
struct Cmiss_graphics_module *Cmiss_graphics_module_create(
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
  struct FE_node_selection *data_selection,
	struct FE_node_selection *node_selection,
	struct Time_keeper *default_time_keeper);

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
 * Destroy Cmiss_graphics_module and clean up the memory it uses.
 *
 * @param cmiss_graphics_module_address  the address to the pointer of 
 *   the cmiss_graphics_packge to be deleted
 * @return  1 if successfully destroy cmiss_graphics_module, otherwise 0
 */
int DESTROY(Cmiss_graphics_module)(
	struct Cmiss_graphics_module **cmiss_graphics_module_address);

#endif /* !defined (CMISS_RENDITION_H) */

