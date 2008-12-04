/*******************************************************************************
FILE : field_location.cpp

LAST MODIFIED : 31 July 2007

DESCRIPTION :
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
 * Portions created by the Initial Developer are Copyright (C) 2006
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
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_location.hpp"

int Field_element_xi_location::check_cache_for_location(Computed_field *field)
{
 	int cache_is_valid, element_dimension, i;

	/* clear the cache if values already cached for a node */
	if (field->node || field->coordinate_reference_field)
	{
		Computed_field_clear_cache(field);
	}
	/* Are the values and derivatives in the cache not already calculated? */
	if ((element == field->element) && (time == field->time) &&
		((0 == number_of_derivatives) || (field->derivatives_valid)))
	{
		element_dimension=get_FE_element_dimension(element);
		cache_is_valid = 1;
		for (i = 0; cache_is_valid && (i < element_dimension); i++)
		{
			if (field->xi[i] != xi[i])
			{
				cache_is_valid = 0;
			}
		}
	}
	else
	{
		cache_is_valid = 0;
	}
	return (cache_is_valid);
}

int Field_element_xi_location::update_cache_for_location(Computed_field *field)
{
	int element_dimension, i, return_code;

	return_code = 1;
	REACCESS(FE_element)(&field->element, element);
	element_dimension=get_FE_element_dimension(element);
	field->time = time;
	for (i = 0; i < element_dimension; i++)
	{
		field->xi[i] = xi[i];
	}
	for (; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		field->xi[i] = 0.0;
	}

	return (return_code);
}

int Field_node_location::check_cache_for_location(Computed_field *field)
{
 	int cache_is_valid;

	/* clear the cache if values already cached for an element */
	if (field->element || field->coordinate_reference_field)
	{
		Computed_field_clear_cache(field);
	}
	/* allocate cache */
	if ((node == field->node) && (time == field->time))
	{
		cache_is_valid = 1;
	}
	else
	{
		cache_is_valid = 0;
	}
	return (cache_is_valid);
}

int Field_node_location::update_cache_for_location(Computed_field *field)
{
	int return_code;

	return_code = 1;
	REACCESS(FE_node)(&field->node, node);
	field->time = time;

	return (return_code);
}

Field_coordinate_location::Field_coordinate_location(
	Computed_field *reference_field,
	int number_of_values_in, FE_value* values_in, FE_value time,
	int number_of_derivatives, FE_value* derivatives_in):
		Field_location(time, number_of_derivatives),
		reference_field(reference_field)
{
	int i;
	number_of_values = reference_field->number_of_components;
	values = new FE_value[number_of_values];
	for (i = 0 ; (i < number_of_values) && (i < number_of_values_in) ; i++)
	{
		values[i] = values_in[i];
	}
	for (; i < number_of_values ; i++)
	{
		values[i] = 0.0;
	}
	if (number_of_derivatives && derivatives_in)
	{
		derivatives = new FE_value[number_of_values * number_of_derivatives];			
		for (i = 0 ; (i < number_of_values * number_of_derivatives) &&
			(i < number_of_values_in * number_of_derivatives) ; i++)
		{
			derivatives[i] = derivatives_in[i];
		}
		for (; i < number_of_values * number_of_derivatives ; i++)
		{
			derivatives[i] = 0.0;
		}
	}
	else
	{
	   this->number_of_derivatives = 0;
	   derivatives = NULL;
	}
}
	
Field_coordinate_location::~Field_coordinate_location()
{
	delete [] values;
	if (derivatives)
	{
		delete [] derivatives;
	}
}

int Field_coordinate_location::set_values(int number_of_values_in, 
	FE_value* values_in)
{
	int i;
	for (i = 0 ; (i < number_of_values) && (i < number_of_values_in) ; i++)
	{
		values[i] = values_in[i];
	}
	for (; i < number_of_values ; i++)
	{
		values[i] = 0.0;
	}
	return 1;
}

int Field_coordinate_location::check_cache_for_location(Computed_field *field)
{
 	int cache_is_valid;

	/* clear the cache if values already cached for an element or node */
	if (field->element || field->node)
	{
		Computed_field_clear_cache(field);
	}
	if (field->field_does_not_depend_on_cached_location &&
		(field->coordinate_reference_field == reference_field) &&
		(time == field->time))
	{
		cache_is_valid = 1;
	}
	else if (reference_field == field)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
				field->values[i] = values[i];
		}
		if (derivatives && number_of_derivatives)
		{
			for (int i = 0 ; i < field->number_of_components * number_of_derivatives ; i++)
			{
					field->derivatives[i] = derivatives[i];
			}
			field->derivatives_valid = 1;
		}
		cache_is_valid = 1;
	}
	else
	{
		cache_is_valid = 0;
	}
	return (cache_is_valid);
}

int Field_coordinate_location::update_cache_for_location(Computed_field *field)
{
	int return_code;

	return_code = 1;
	if (field->coordinate_reference_field != reference_field)
	{
		REACCESS(Computed_field)(&field->coordinate_reference_field,
			reference_field);
		field->field_does_not_depend_on_cached_location = 
			!Computed_field_depends_on_Computed_field(field, reference_field);
	}
	field->time = time;

	return (return_code);
}

int Field_coordinate_location::set_values_for_location(Computed_field *field,
	FE_value *values_in)
{
	int return_code;
	if ((field == reference_field) &&
		(number_of_values == field->number_of_components))
	{
		int i;
		for (i = 0 ; (i < number_of_values) ; i++)
		{
			values[i] = values_in[i];
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}
