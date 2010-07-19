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
#include "graphics/rendition.h"

int Cmiss_material_set_alpha(
	Cmiss_material_id material, float alpha)
{
	int return_code = 0;

	ENTER(Cmiss_material_set_alpha);
	if (material)
	{
		return_code = Graphical_material_set_alpha(material, alpha);
	}
	LEAVE;

	return return_code;
}

int Cmiss_material_set_shininess(
	Cmiss_material_id material, float shininess)
{
	int return_code = 0;

	ENTER(Cmiss_material_set_shininess);
	if (material)
	{
		return_code = Graphical_material_set_shininess(material, shininess);
	}
	LEAVE;

	return return_code;
}

int Cmiss_material_set_ambient(
	Cmiss_material_id material, float red, float green, float blue)
{
	struct Colour colour;
	int return_code = 0;

	ENTER(Cmiss_material_set_ambient);
	if (material)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Graphical_material_set_ambient(material, &colour);
	}
	LEAVE;
	
	return return_code;
}

int Cmiss_material_set_diffuse(
	Cmiss_material_id material, float red, float green, float blue)
{
	struct Colour colour;
	int return_code = 0;

	ENTER(Cmiss_material_set_diffuse);
	if (material)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Graphical_material_set_diffuse(material, &colour);
	}
	LEAVE;
	
	return return_code;
}

int Cmiss_material_set_emission(
	Cmiss_material_id material, float red, float green, float blue)
{
	struct Colour colour;
	int return_code = 0;

	ENTER(Cmiss_material_set_emission);
	if (material)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Graphical_material_set_emission(material, &colour);
	}
	LEAVE;

	return return_code;
}

int Cmiss_material_set_specular(
	Cmiss_material_id material, float red, float green, float blue)
{
	struct Colour colour;
	int return_code = 0;

	ENTER(Cmiss_material_set_specular);
	if (material)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Graphical_material_set_specular(material, &colour);
	}
	LEAVE;
	
	return return_code;
}

char *Cmiss_material_get_name(Cmiss_material_id material)
{
	char *name = NULL;
	if (material)
	{
		name = duplicate_string(Graphical_material_name(material));
	}
	
	return name;
}
