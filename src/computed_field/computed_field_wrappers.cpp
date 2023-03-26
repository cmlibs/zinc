/*******************************************************************************
FILE : computed_field_wrappers.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Functions for converting fields in a not-so-usable state into more useful
quantities, usually for graphical display or editing. For example, making a
wrapper rectangular Cartesian field out of a prolate coordinate field, making
fibre_axes out of a fibre field.
==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldfibres.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_wrappers.h"
#include "computed_field/field_module.hpp"
#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"
#include <cstring>
#include <cstdio>

namespace {

/** Find wrapper field in sourceField1's manager of name starting with sourceField1's name plus suffix,
 * adding numbers starting at 1 etc, which is of the supplied type with has sourceField1 as first source field
 * @return  Non-accessed field. */
cmzn_field *findWrapperField(cmzn_field *sourceField1, const char *suffix, cmzn_field_type type)
{
	const size_t length = strlen(sourceField1->getName()) + strlen(suffix);
	char *name = new char[length + 10];
	sprintf(name, "%s%s", sourceField1->getName(), suffix);
	int i = 0;
	cmzn_field *wrapperField = nullptr;
	while (true)
	{
		if (i > 0)
		{
			sprintf(name + length, "%d", i);
		}
		wrapperField = FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_field, name)(name, sourceField1->getManager());
		if (!wrapperField)
		{
			break;
		}
		if ((wrapperField->core->get_type() == type) &&
			(wrapperField->getSourceField(0) == sourceField1) &&
			(wrapperField->getCoordinateSystem().getType() == RECTANGULAR_CARTESIAN))
		{
			break;
		}
		++i;
	}
	delete[] name;
	return wrapperField;
}

}

cmzn_field *cmzn_field_get_coordinate_field_wrapper(
	cmzn_field *coordinate_field)
{
	cmzn_field *wrapper_field = 0;

	if (Computed_field_has_up_to_3_numerical_components(coordinate_field, /*dummy*/0))
	{
		Coordinate_system_type type = coordinate_field->getCoordinateSystem().getType();
		if (Coordinate_system_type_is_non_linear(type))
		{
			wrapper_field = findWrapperField(coordinate_field, "_cmiss_rc_wrapper", CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION);
			if (wrapper_field)
			{
				wrapper_field->access();
			}
			else
			{
				cmzn_fieldmodule *fieldmodule = cmzn_field_get_fieldmodule(coordinate_field);
				cmzn_fieldmodule_begin_change(fieldmodule);
				// default coordinate system type is RC
				wrapper_field = cmzn_fieldmodule_create_field_coordinate_transformation(fieldmodule, coordinate_field);
				wrapper_field->setNameUnique(coordinate_field->getName(), "_cmiss_rc_wrapper", 0);
				cmzn_fieldmodule_end_change(fieldmodule);
				cmzn_fieldmodule_destroy(&fieldmodule);
			}
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
		(NOT_APPLICABLE == coordinate_system_type) ||
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
	cmzn_field *wrapper_field = nullptr;

	if ((vector_field) && Computed_field_is_orientation_scale_capable(vector_field, nullptr) &&
		((!coordinate_field) || Computed_field_has_up_to_3_numerical_components(coordinate_field, nullptr)))
	{
		if ((!coordinate_field) || (!cmzn_field_vector_needs_wrapping(vector_field)))
		{
			/* RC fields and non-fibre scalars are already OK */
			return vector_field->access();
		}
		Coordinate_system_type coordinate_system_type = vector_field->getCoordinateSystem().getType();
		if ((FIBRE == coordinate_system_type) &&
			(3 >= cmzn_field_get_number_of_components(vector_field)))
		{
			wrapper_field = findWrapperField(vector_field, "_cmiss_rc_fibre_wrapper", CMZN_FIELD_TYPE_FIBRE_AXES);
			if ((wrapper_field) && (wrapper_field->getSourceField(1) == coordinate_field))
			{
				wrapper_field->access();
			}
			else
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
		}
		else
		{
			wrapper_field = findWrapperField(vector_field, "_cmiss_rc_vector_wrapper", CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION);
			if ((wrapper_field) && (wrapper_field->getSourceField(1) == coordinate_field))
			{
				wrapper_field->access();
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_get_vector_field_wrapper.  Invalid argument(s)");
	}
	return (wrapper_field);
}
