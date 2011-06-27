/*******************************************************************************
FILE : computed_field_wrappers.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Functions for converting fields in a not-so-usable state into more useful
quantities, usually for graphical display or editing. For example, making a
wrapper rectangular Cartesian field out of a prolate coordinate field, making
fibre_axes out of a fibre field.
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
#include "api/cmiss_field_module.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_wrappers.h"
#include "general/debug.h"
#include "user_interface/message.h"
}
#include "computed_field/computed_field_private.hpp"

struct Computed_field *Computed_field_begin_wrap_coordinate_field(
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns a RECTANGULAR_CARTESIAN coordinate field that may be the original
<coordinate field> if it is already in this coordinate system, or a
COMPUTED_FIELD_RC_COORDINATE wrapper for it if it is not.
Notes:
Used to ensure RC coordinate fields are passed to graphics functions.
Must call Computed_field_end_wrap to clean up the returned field after use.
The NORMALISED_WINDOW_COORDINATES system is a rectangular cartesian system
but indicates that the graphics objects produced should be displayed in the 
window coordinates rather than the model 3D coordinates and so are also not
wrapped.
==============================================================================*/
{
	enum Coordinate_system_type type;
	struct Computed_field *wrapper_field;

	ENTER(Computed_field_begin_wrap_coordinate_field);
	if (coordinate_field&&(3>=
		Computed_field_get_number_of_components(coordinate_field)))
	{
		type = get_coordinate_system_type(Computed_field_get_coordinate_system(coordinate_field));
		if (Coordinate_system_type_is_non_linear(type))
		{
			Cmiss_field_module *field_module =
				Cmiss_field_module_create(Computed_field_get_region(coordinate_field));
			struct Coordinate_system rc_coordinate_system;
			rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
			Cmiss_field_module_set_coordinate_system(field_module,
				rc_coordinate_system);
			wrapper_field = Computed_field_create_coordinate_transformation(field_module,
				coordinate_field);
			Cmiss_field_module_destroy(&field_module);
		}
		else
		{
			wrapper_field = ACCESS(Computed_field)(coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_begin_wrap_coordinate_field.  Invalid argument(s)");
		wrapper_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (wrapper_field);
} /* Computed_field_begin_wrap_coordinate_field */

struct Computed_field *Computed_field_begin_wrap_orientation_scale_field(
	struct Computed_field *orientation_scale_field,
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Takes the <orientation_scale_field> and returns a field ready for use in the
rest of the program. This involves making a COMPUTED_FIELD_FIBRE_AXES wrapper
if the field has 3 or fewer components and a FIBRE coordinate system (this
requires the coordinate_field too). If the field has 3 or fewer components and
a non-RECTANGULAR_CARTESIAN coordinate system, a wrapper of type
COMPUTED_FIELD_RC_ORIENTATION_SCALE will be made for it. If the field is deemed
already usable in in its orientation_scale role, it is simply returned. Note
that the function accesses any returned field.
Note:
Must call Computed_field_end_wrap to clean up the returned field after use.
==============================================================================*/
{
	struct Computed_field *wrapper_field;
	enum Coordinate_system_type coordinate_system_type;

	ENTER(Computed_field_begin_wrap_orientation_scale_field);
	if (orientation_scale_field&&coordinate_field&&
		Computed_field_is_orientation_scale_capable(orientation_scale_field,NULL)&&
		Computed_field_has_up_to_3_numerical_components(coordinate_field,NULL))
	{
		coordinate_system_type=get_coordinate_system_type(
			Computed_field_get_coordinate_system(orientation_scale_field));
		if ((RECTANGULAR_CARTESIAN == coordinate_system_type) ||
			((1 == Computed_field_get_number_of_components(orientation_scale_field)) &&
				(FIBRE != coordinate_system_type)))
		{
			/* RC fields and non-fibre scalars are already OK */
			wrapper_field = ACCESS(Computed_field)(orientation_scale_field);
		}
		else if ((FIBRE == coordinate_system_type) &&
			(3>=Computed_field_get_number_of_components(orientation_scale_field)))
		{
			/* make fibre_axes wrapper from fibre field */
			Cmiss_field_module *field_module =
				Cmiss_field_module_create(Computed_field_get_region(coordinate_field));
			wrapper_field = Computed_field_create_fibre_axes(field_module,
					orientation_scale_field, coordinate_field);
			Cmiss_field_module_destroy(&field_module);
		}
		else
		{
			/* make vector_coordinate_transformation wrapper of non-RC vector field */
			Cmiss_field_module *field_module =
				Cmiss_field_module_create(Computed_field_get_region(coordinate_field));
			struct Coordinate_system rc_coordinate_system;
			rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
			Cmiss_field_module_set_coordinate_system(field_module,
				rc_coordinate_system);
			wrapper_field = Computed_field_create_vector_coordinate_transformation(
				field_module, orientation_scale_field, coordinate_field);
			Cmiss_field_module_destroy(&field_module);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_begin_wrap_orientation_scale_field.  "
			"Invalid argument(s)");
		wrapper_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (wrapper_field);
} /* Computed_field_begin_wrap_orientation_scale_field */

int Computed_field_end_wrap(struct Computed_field **wrapper_field_address)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Cleans up a field accessed/created by a Computed_field_begin_wrap*() function.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_end_wrap);
	if (wrapper_field_address)
	{
		return_code=DEACCESS(Computed_field)(wrapper_field_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_end_wrap.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_end_wrap */
