/*******************************************************************************
FILE : auxiliary_graphics_types.c

LAST MODIFIED : 3 May 2001

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curvesin three xi-directions;
- Triple;
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/random.h"
#include "graphics/auxiliary_graphics_types.h"
#include "general/message.h"

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Graphics_select_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Graphics_select_mode));
	switch (enumerator_value)
	{
		case GRAPHICS_SELECT_ON:
		{
			enumerator_string = "select_on";
		} break;
		case GRAPHICS_NO_SELECT:
		{
			enumerator_string = "no_select";
		} break;
		case GRAPHICS_DRAW_SELECTED:
		{
			enumerator_string = "draw_selected";
		} break;
		case GRAPHICS_DRAW_UNSELECTED:
		{
			enumerator_string = "draw_unselected";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Graphics_select_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Graphics_select_mode)

int check_Circle_discretization(int *circle_discretization)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Ensures the <circle_discretization> is within the bounds of the minimum of 2
and the maximum read in from the defaults.
???RC user_interface argument not checked as may not be needed in
read_Circle_discretization_defaults().
==============================================================================*/
{
	int return_code;
	int initial_value;

	ENTER(check_Circle_discretization);
	if (circle_discretization)
	{
		initial_value = *circle_discretization;
		if (2 > *circle_discretization)
		{
			*circle_discretization=2;
		}
		if (*circle_discretization != initial_value)
		{
			display_message(WARNING_MESSAGE,
				"Circle discretization values must be at least 2\n"
				"%d changed to %d",initial_value,
				*circle_discretization);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"check_Circle_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* check_Circle_discretization */

int check_Element_discretization(struct Element_discretization
	*element_discretization)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Ensures the <element_discretization> is within the bounds of the minimum of 1
and the maximum read in from the defaults.
???DB.  Changed the lower bound to 1 because also used for elements.
???DB.  Need to make consistent.
???RC user_interface argument not checked as may not be needed in
read_Element_discretization_defaults().
==============================================================================*/
{
	int discretization_change,return_code;
	struct Element_discretization initial;

	ENTER(check_Element_discretization);
	if (element_discretization)
	{
		discretization_change=0;
		initial.number_in_xi1=element_discretization->number_in_xi1;
		initial.number_in_xi2=element_discretization->number_in_xi2;
		initial.number_in_xi3=element_discretization->number_in_xi3;
		if (1 > element_discretization->number_in_xi1)
		{
			element_discretization->number_in_xi1=1;
			discretization_change=1;
		}
		if (1 > element_discretization->number_in_xi2)
		{
			element_discretization->number_in_xi2=1;
			discretization_change=1;
		}
		if (1 > element_discretization->number_in_xi3)
		{
			element_discretization->number_in_xi3=1;
			discretization_change=1;
		}
		if (discretization_change)
		{
			display_message(WARNING_MESSAGE,
				"Element discretization values must be at least 1\n"
				"%d*%d*%d changed to %d*%d*%d",
				initial.number_in_xi1,initial.number_in_xi2,initial.number_in_xi3,
				element_discretization->number_in_xi1,
				element_discretization->number_in_xi2,
				element_discretization->number_in_xi3);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"check_Element_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* check_Element_discretization */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Streamline_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Streamline_type));
	switch (enumerator_value)
	{
		case STREAM_EXTRUDED_ELLIPSE:
		{
			enumerator_string = "ellipse";
		} break;
		case STREAM_LINE:
		{
			enumerator_string = "line";
		} break;
		case STREAM_EXTRUDED_RECTANGLE:
		{
			enumerator_string = "rectangle";
		} break;
		case STREAM_RIBBON:
		{
			enumerator_string = "ribbon";
		} break;
		case STREAM_EXTRUDED_CIRCLE:
		{
			enumerator_string = "cylinder";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Streamline_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Streamline_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Streamline_data_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Streamline_data_type));
	switch (enumerator_value)
	{
		case STREAM_NO_DATA:
		{
			enumerator_string = "no_data";
		} break;
		case STREAM_FIELD_SCALAR:
		{
			enumerator_string = "field_scalar";
		} break;
		case STREAM_MAGNITUDE_SCALAR:
		{
			enumerator_string = "magnitude_scalar";
		} break;
		case STREAM_TRAVEL_SCALAR:
		{
			enumerator_string = "travel_scalar";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Streamline_data_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Streamline_data_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphics_render_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Cmiss_graphics_render_type));
	switch (enumerator_value)
	{
		case CMISS_GRAPHICS_RENDER_TYPE_SHADED:
		{
			enumerator_string = "render_shaded";
		} break;
		case CMISS_GRAPHICS_RENDER_TYPE_WIREFRAME:
		{
			enumerator_string = "render_wireframe";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Cmiss_graphic_render_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphics_render_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Xi_discretization_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Xi_discretization_mode));
	switch (enumerator_value)
	{
		case XI_DISCRETIZATION_CELL_CENTRES:
		{
			enumerator_string = "cell_centres";
		} break;
		case XI_DISCRETIZATION_CELL_CORNERS:
		{
			enumerator_string = "cell_corners";
		} break;
		case XI_DISCRETIZATION_CELL_DENSITY:
		{
			enumerator_string = "cell_density";
		} break;
		case XI_DISCRETIZATION_CELL_POISSON:
		{
			enumerator_string = "cell_poisson";
		} break;
		case XI_DISCRETIZATION_CELL_RANDOM:
		{
			enumerator_string = "cell_random";
		} break;
		case XI_DISCRETIZATION_EXACT_XI:
		{
			enumerator_string = "exact_xi";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Xi_discretization_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Xi_discretization_mode)

