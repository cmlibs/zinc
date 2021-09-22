/*******************************************************************************
FILE : computed_field_wrappers.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Functions for converting fields in a not-so-usable state into more useful
quantities, usually for graphical display or editing. For example, making a
wrapper rectangular Cartesian field out of a prolate coordinate field, making
fibre_axes out of a fibre field.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldfibres.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_wrappers.h"
#include "computed_field/field_module.hpp"
#include "general/debug.h"
#include "general/message.h"
#include "computed_field/computed_field_private.hpp"

cmzn_field *cmzn_field_get_coordinate_field_wrapper(
	cmzn_field *coordinate_field)
{
	cmzn_field *wrapper_field = 0;

	if (Computed_field_has_up_to_3_numerical_components(coordinate_field, /*dummy*/0))
	{
		Coordinate_system_type type = coordinate_field->getCoordinateSystem().getType();
		if (Coordinate_system_type_is_non_linear(type))
		{
			cmzn_fieldmodule *fieldmodule = cmzn_field_get_fieldmodule(coordinate_field);
			cmzn_fieldmodule_begin_change(fieldmodule);
			// default coordinate system type is RC
			wrapper_field = cmzn_fieldmodule_create_field_coordinate_transformation(fieldmodule, coordinate_field);
			wrapper_field->setNameUnique(coordinate_field->getName(), "_cmiss_rc_wrapper", 0);
			cmzn_fieldmodule_end_change(fieldmodule);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
		else
		{
			wrapper_field = ACCESS(Computed_field)(coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_get_coordinate_field_wrapper.  Invalid argument(s)");
	}
	return (wrapper_field);
}

bool cmzn_field_vector_needs_wrapping(cmzn_field *vector_field)
{
	Coordinate_system_type coordinate_system_type = vector_field->getCoordinateSystem().getType();
	if ((RECTANGULAR_CARTESIAN == coordinate_system_type) ||
		((1 == cmzn_field_get_number_of_components(vector_field)) &&
		(FIBRE != coordinate_system_type)))
	{
		return false;
	}
	return true;
}

cmzn_field *cmzn_field_get_vector_field_wrapper(
	cmzn_field *vector_field, cmzn_field *coordinate_field)
{
	struct Computed_field *wrapper_field = 0;

	if (vector_field&&coordinate_field&&
		Computed_field_is_orientation_scale_capable(vector_field,NULL)&&
		Computed_field_has_up_to_3_numerical_components(coordinate_field,NULL))
	{
		Coordinate_system_type coordinate_system_type = vector_field->getCoordinateSystem().getType();
		if (!cmzn_field_vector_needs_wrapping(vector_field))
		{
			/* RC fields and non-fibre scalars are already OK */
			wrapper_field = ACCESS(Computed_field)(vector_field);
		}
		else if ((FIBRE == coordinate_system_type) &&
			(3>=cmzn_field_get_number_of_components(vector_field)))
		{
			/* make fibre_axes wrapper from fibre field */
			cmzn_fieldmodule *field_module = cmzn_field_get_fieldmodule(coordinate_field);
			wrapper_field = cmzn_fieldmodule_create_field_fibre_axes(field_module,
				vector_field, coordinate_field);
			if (wrapper_field)
			{
				wrapper_field->setNameUnique(vector_field->getName(), "_cmiss_rc_fibre_wrapper", 0);
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		else
		{
			/* make vector_coordinate_transformation wrapper of non-RC vector field */
			cmzn_fieldmodule *field_module = cmzn_field_get_fieldmodule(coordinate_field);
			cmzn_fieldmodule_begin_change(field_module);
			wrapper_field = cmzn_fieldmodule_create_field_vector_coordinate_transformation(
				field_module, vector_field, coordinate_field);
			if (wrapper_field)
			{
				wrapper_field->setNameUnique(vector_field->getName(), "_cmiss_rc_vector_wrapper", 0);
				Coordinate_system rc_coordinate_system(RECTANGULAR_CARTESIAN);
				wrapper_field->setCoordinateSystem(rc_coordinate_system);
			}
			cmzn_fieldmodule_end_change(field_module);
			cmzn_fieldmodule_destroy(&field_module);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_get_vector_field_wrapper.  Invalid argument(s)");
	}
	return (wrapper_field);
}
