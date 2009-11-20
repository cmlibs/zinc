/*******************************************************************************
FILE : cmiss_material.c

LAST MODIFIED : 05 Nov 2009

DESCRIPTION :
The public interface to the material.
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

#include "api/cmiss_material.h"
#include "general/debug.h"
#include "graphics/colour.h"
#include "graphics/material.h"
#include "graphics/cmiss_rendition.h"
#include "graphics/texture.h"

Cmiss_graphical_material_id Cmiss_graphical_material_create(
	Cmiss_graphics_package_id graphics_package)
{
	struct MANAGER(Graphical_material) *material_manager =
		Cmiss_graphics_package_get_material_manager(graphics_package);
	int i = 0;
	char *temp_string = NULL;
	char *num = NULL;
	do 
	{
		if (temp_string)
		{
			DEALLOCATE(temp_string);
		}
		ALLOCATE(temp_string, char, 18);
		strcpy(temp_string, "temp_material");
		num = strrchr(temp_string, 'l') + 1;
		sprintf(num, "%i", i);
		strcat(temp_string, "\0");
		i++;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(temp_string,
			material_manager));
			
	if (temp_string)
	{
		Cmiss_graphical_material_id material;
		if (NULL != (material = CREATE(Graphical_material)(temp_string)))
		{
			if (ADD_OBJECT_TO_MANAGER(Graphical_material)(
						material, material_manager))
			{
				return ACCESS(Graphical_material)(material);
			}
			else
			{
				DESTROY(Graphical_material)(&material);
			}
		}
		DEALLOCATE(temp_string);
	}
	return NULL;
}

Cmiss_graphical_material_id Cmiss_graphical_material_get_with_name(
	Cmiss_graphics_package_id graphics_package, const char *name)
{
	Cmiss_graphical_material_id material = NULL;

	if (graphics_package && name)
	{
		struct MANAGER(Graphical_material) *material_manager =
			Cmiss_graphics_package_get_material_manager(graphics_package);
		material = FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
			name, material_manager);
	}

	return ACCESS(Graphical_material)(material);
}

int Cmiss_graphical_material_set_alpha(
	Cmiss_graphical_material_id material, float alpha)
{
	if (material)
	{
		return Graphical_material_set_alpha(material, alpha);
	}
	return 0;
}

int Cmiss_graphical_material_set_shininess(
	Cmiss_graphical_material_id material, float shininess)
{
	if (material)
	{
		return Graphical_material_set_shininess(material, shininess);
	}
	return 0;
}

int Cmiss_graphical_material_set_ambient(
	Cmiss_graphical_material_id material, float red, float green, float blue)
{
	struct Colour colour;
	
	colour.red = red;
	colour.green = green;
	colour.blue = blue;
	
	return Graphical_material_set_ambient(material, &colour);
}

int Cmiss_graphical_material_set_diffuse(
	Cmiss_graphical_material_id material, float red, float green, float blue)
{

	struct Colour colour;
	
	colour.red = red;
	colour.green = green;
	colour.blue = blue;
	
	return Graphical_material_set_diffuse(material, &colour);
}

int Cmiss_graphical_material_set_emission(
	Cmiss_graphical_material_id material, float red, float green, float blue)
{
	struct Colour colour;

	colour.red = red;
	colour.green = green;
	colour.blue = blue;

	return Graphical_material_set_emission(material, &colour);
}

int Cmiss_graphical_material_set_specular(
	Cmiss_graphical_material_id material, float red, float green, float blue)
{
	struct Colour colour;

	colour.red = red;
	colour.green = green;
	colour.blue = blue;

	return Graphical_material_set_specular(material, &colour);
}
