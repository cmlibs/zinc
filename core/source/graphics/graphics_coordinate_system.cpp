/***************************************************************************//**
 * FILE: graphics_coordinate_system.cpp
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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

#include <string.h>
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
}
#include "general/enumerator_conversion.hpp"
#include "general/enumerator_private.hpp"
#include "graphics/graphics_coordinate_system.hpp"

class Cmiss_graphics_coordinate_system_conversion
{
public:
    static const char *to_string(enum Cmiss_graphics_coordinate_system system)
    {
        const char *enum_string = 0;
        switch (system)
        {
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL:
            enum_string = "LOCAL";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_WORLD:
            enum_string = "WORLD";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL:
            enum_string = "NORMALISED_WINDOW_FILL";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE:
            enum_string = "NORMALISED_WINDOW_FIT_CENTRE";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT:
            enum_string = "NORMALISED_WINDOW_FIT_LEFT";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT:
            enum_string = "NORMALISED_WINDOW_FIT_RIGHT";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM:
            enum_string = "NORMALISED_WINDOW_FIT_BOTTOM";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP:
            enum_string = "NORMALISED_WINDOW_FIT_TOP";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT:
            enum_string = "WINDOW_PIXEL_BOTTOM_LEFT";
            break;
        case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT:
            enum_string = "WINDOW_PIXEL_TOP_LEFT";
            break;
        default:
            break;
        }
        return enum_string;
    }
};

enum Cmiss_graphics_coordinate_system	Cmiss_graphics_coordinate_system_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_graphics_coordinate_system,
		Cmiss_graphics_coordinate_system_conversion>(string);
}

char *Cmiss_graphics_coordinate_system_enum_to_string(
	enum Cmiss_graphics_coordinate_system coordinate_system)
{
	const char *system_string = Cmiss_graphics_coordinate_system_conversion::to_string(coordinate_system);
	return (system_string ? duplicate_string(system_string) : 0);
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphics_coordinate_system)
{
	return Cmiss_graphics_coordinate_system_conversion::to_string(enumerator_value);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphics_coordinate_system)

int Cmiss_graphics_coordinate_system_get_viewport(
	enum Cmiss_graphics_coordinate_system coordinate_system,
	double viewport_width, double viewport_height,
	double *left, double *right, double *bottom, double *top)
{
	if (!((viewport_width > 0.0) && (viewport_height > 0.0) && left && right && bottom && top))
		return 0;
	*left = -1.0;
	*right = 1.0;
	*bottom = -1.0;
	*top = 1.0;
	int return_code = 1;
	switch (coordinate_system)
	{
	case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL:
		// do nothing
		break;
	case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT:
		*left = 0.0;
		*right = viewport_width;
		*bottom = 0.0;
		*top = viewport_height;
		break;
	case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT:
		*left = 0.0;
		*right = viewport_width;
		*bottom = -viewport_height;
		*top = 0.0;
		break;
	default:
		if (viewport_width > viewport_height)
		{
			double width_to_height_ratio = viewport_width/viewport_height;
			switch (coordinate_system)
			{
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT:
					*right = 2.0*width_to_height_ratio - 1.0;
					break;
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT:
					*left = -2.0*width_to_height_ratio + 1.0;
					break;
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE:
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP:
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM:
					*left = -width_to_height_ratio;
					*right = width_to_height_ratio;
					break;
				default:
					return_code = 0;
					break;
			}
		}
		else
		{
			double height_to_width_ratio = viewport_height / viewport_width;
			switch (coordinate_system)
			{
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE:
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT:
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT:
					*bottom = -height_to_width_ratio;
					*top = height_to_width_ratio;
					break;
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP:
					*bottom = -2.0*height_to_width_ratio + 1.0;
					break;
				case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM:
					*top = 2.0*height_to_width_ratio - 1.0;
					break;
				default:
					return_code = 0;
					break;
			}
		}
		break;
	}
	if (!return_code)
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_coordinate_system_get_viewport.  Unsupported coordinate system");
	}
	return return_code;
}

/*******************************************************************************
 * Returns true if the coordinate_system is window-relative, which currently
 * determines whether it is drawn as an overlay.
 *
 * @param coordinate_system  The graphics coordinate system.
 * @return  1 if window-relative, 0 if not.
 */
int Cmiss_graphics_coordinate_system_is_window_relative(
	enum Cmiss_graphics_coordinate_system coordinate_system)
{
	return
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT) ||
		(coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT);
}
