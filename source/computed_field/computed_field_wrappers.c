/*******************************************************************************
FILE : computed_field_wrappers.c

LAST MODIFIED : 27 October 2000

DESCRIPTION :
Functions for converting fields in a not-so-usable state into more useful
quantities, usually for graphical display or editing. For example, making a
wrapper rectangular Cartesian field out of a prolate coordinate field, making
fibre_axes out of a fibre field.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_wrappers.h"
#include "general/debug.h"
#include "user_interface/message.h"

struct Computed_field *Computed_field_begin_wrap_coordinate_field(
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Returns a RECTANGULAR_CARTESIAN coordinate field that may be the original
<coordinate field> if it is already in this coordinate system, or a
COMPUTED_FIELD_RC_COORDINATE wrapper for it if it is not.
Notes:
Used to ensure RC coordinate fields are passed to graphics functions.
Must call Computed_field_end_wrap to clean up the returned field after use.
==============================================================================*/
{
	struct Computed_field *wrapper_field;

	ENTER(Computed_field_begin_wrap_coordinate_field);
	if (coordinate_field&&(3>=
		Computed_field_get_number_of_components(coordinate_field)))
	{
		if (RECTANGULAR_CARTESIAN==get_coordinate_system_type(
			Computed_field_get_coordinate_system(coordinate_field)))
		{
			wrapper_field=ACCESS(Computed_field)(coordinate_field);
		}
		else
		{
			/* make RC wrapper for the coordinate_field */
			if ((wrapper_field=CREATE(Computed_field)("rc_wrapper"))&&
				Computed_field_set_type_rc_coordinate(wrapper_field,
					coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				DESTROY(Computed_field)(&wrapper_field);
			}
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
LAST MODIFIED : 18 October 2000

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
		if ((3>=Computed_field_get_number_of_components(orientation_scale_field))&&
			(FIBRE==coordinate_system_type))
		{
			/* make FIBRE_AXES wrapper */
			if ((wrapper_field=CREATE(Computed_field)("fibre_axes_wrapper"))&&
				Computed_field_set_type_fibre_axes(wrapper_field,
					orientation_scale_field,coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_begin_wrap_orientation_scale_field.  "
					"Unable to make fibre_axes wrapper for field");
				DESTROY(Computed_field)(&wrapper_field);
			}
		}
		else if ((1==Computed_field_get_number_of_components(
			orientation_scale_field))||
			(RECTANGULAR_CARTESIAN==coordinate_system_type))
		{
			/* scalar or RC fields are already OK */
			wrapper_field=ACCESS(Computed_field)(orientation_scale_field);
		}
		else
		{
			/* make RC_VECTOR wrapper for the orientation_scale_field */
			if ((wrapper_field=CREATE(Computed_field)("rc_wrapper"))&&
				Computed_field_set_type_rc_vector(wrapper_field,
					orientation_scale_field,coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_begin_wrap_orientation_scale_field.  "
					"Unable to make rc_component wrapper for field");
				DESTROY(Computed_field)(&wrapper_field);
			}
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
LAST MODIFIED : 11 March 1999

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
