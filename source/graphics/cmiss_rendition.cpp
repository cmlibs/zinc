/*******************************************************************************
FILE : cmiss_rendition.cpp

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

extern "C" {
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/object.h"
#include "graphics/cmiss_rendition.h"
#include "graphics/graphics_object.h"
#include "graphics/element_point_ranges.h"
#include "graphics/font.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"
}

struct Cmiss_graphics_package
{
	/* attribute managers and defaults: */
	struct LIST(GT_object) *glyph_list;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_material;
	struct Graphics_font *default_font;
	struct MANAGER(Light) *light_manager;
	struct LIST(Light) *list_of_lights;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *default_spectrum;
	struct MANAGER(Texture) *texture_manager;
	
	/* selection */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;
	struct Time_keeper *default_time_keeper;
};

struct Cmiss_graphics_package *Cmiss_graphics_package_create(
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
	struct Time_keeper *default_time_keeper)
{
	struct Cmiss_graphics_package *package;

	ENTER(Cmiss_graphics_package_create);
	if (glyph_list && graphical_material_manager && default_material &&
		default_font && light_manager && spectrum_manager &&
		default_spectrum && texture_manager && element_point_ranges_selection &&
		element_selection && data_selection && node_selection)
	{
		if (ALLOCATE(package, struct Cmiss_graphics_package, 1))
		{
			package->glyph_list = glyph_list;
			package->graphical_material_manager = graphical_material_manager;
			package->default_material = ACCESS(Graphical_material)(default_material);
			package->default_font = ACCESS(Graphics_font)(default_font);
			package->light_manager = light_manager;
			package->spectrum_manager = spectrum_manager;
			package->default_spectrum = ACCESS(Spectrum)(default_spectrum);
			package->texture_manager = texture_manager;
			package->element_point_ranges_selection = element_point_ranges_selection;
			package->element_selection = element_selection;
			package->data_selection = data_selection;
			package->node_selection = node_selection;
			package->default_time_keeper = ACCESS(Time_keeper)(default_time_keeper);
		}
		else
		{
			package = (Cmiss_graphics_package *)NULL;
			display_message(ERROR_MESSAGE,
			"Cmiss_rendtion_graphics_package_create. Not enough memory for Cmiss rendition graphics package");
		}
	}
	else
	{
		package = (Cmiss_graphics_package *)NULL;
		display_message(ERROR_MESSAGE,"Cmiss_rendtion_graphics_package_create.  Invalid argument(s)");
	}
	LEAVE;

	return (package);
}

struct MANAGER(Graphical_material) *Cmiss_graphics_package_get_material_manager(
	struct Cmiss_graphics_package *graphics_package)
{
	struct MANAGER(Graphical_material) *material_manager;

	ENTER(Cmiss_graphics_package_get_material_manager);
	if (graphics_package)
	{
		material_manager = graphics_package->graphical_material_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_package_get_material_manager.  Invalid argument(s)");
		material_manager = (struct MANAGER(Graphical_material) *)NULL;
	}
	LEAVE;

	return (material_manager);
}

int DESTROY(Cmiss_graphics_package)(
	struct Cmiss_graphics_package **cmiss_graphics_package_address)
{
	int return_code;
	struct Cmiss_graphics_package *cmiss_graphics_package;

	ENTER(DESTROY(Cmiss_graphics_package));
	if (NULL != (cmiss_graphics_package =* cmiss_graphics_package_address))
	{
		DEACCESS(Graphical_material)(&cmiss_graphics_package->default_material);
		DEACCESS(Graphics_font)(&cmiss_graphics_package->default_font);
		DEACCESS(Spectrum)(&cmiss_graphics_package->default_spectrum);
		DEACCESS(Time_keeper)(&cmiss_graphics_package->default_time_keeper);
		DEALLOCATE(*cmiss_graphics_package_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_graphics_package).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
