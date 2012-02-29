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

Field_location::~Field_location() {} /* Declaration of pure virtual destructor */

int Field_element_xi_location::set_element_xi(struct FE_element *element_in,
	int number_of_xi_in, const FE_value *xi_in, struct FE_element *top_level_element_in)
{
	if ((!element_in) || (!xi_in))
		return 0;
	if (element_in != element)
	{
		int new_dimension = get_FE_element_dimension(element_in);
		if (number_of_xi_in < new_dimension)
			return 0;
		REACCESS(FE_element)(&element, element_in);
		dimension = new_dimension;
	}
	else if (number_of_xi_in < dimension)
	{
		return 0;
	}
	for (int i = 0; i < dimension; i++)
	{
		xi[i] = xi_in[i];
	}
	if (top_level_element_in != top_level_element)
	{
		REACCESS(FE_element)(&top_level_element, top_level_element_in);
	}
	return 1;
}

Field_coordinate_location::Field_coordinate_location(
	Computed_field *reference_field_in,
	int number_of_values_in, const FE_value* values_in, FE_value time,
	int number_of_derivatives_in, const FE_value* derivatives_in):
		Field_location(time, number_of_derivatives_in),
		reference_field(ACCESS(Computed_field)(reference_field_in))
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
	if (number_of_derivatives_in && derivatives_in)
	{
		derivatives = new FE_value[number_of_values * number_of_derivatives_in];
		for (i = 0 ; (i < number_of_values * number_of_derivatives_in) &&
			(i < number_of_values_in * number_of_derivatives_in) ; i++)
		{
			derivatives[i] = derivatives_in[i];
		}
		for (; i < number_of_values * number_of_derivatives_in ; i++)
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
	DEACCESS(Computed_field)(&reference_field);
	delete [] values;
	delete [] derivatives;
}

int Field_coordinate_location::set_field_values(Cmiss_field_id reference_field_in,
	int number_of_values_in, const FE_value *values_in)
{
	if ((!reference_field_in) || (number_of_values_in < 1) || (!values_in))
		return 0;
	if (reference_field_in != reference_field)
	{
		REACCESS(Computed_field)(&reference_field, reference_field_in);
		number_of_values = reference_field->number_of_components;
		delete [] values;
		values = new FE_value[number_of_values];
	}
	int i;
	for (i = 0 ; (i < number_of_values) && (i < number_of_values_in) ; i++)
	{
		values[i] = values_in[i];
	}
	for (; i < number_of_values ; i++)
	{
		values[i] = 0.0;
	}
	if (derivatives)
	{
		delete[] derivatives;
		derivatives = 0;
		number_of_derivatives = 0;
	}
	return 1;
}

int Field_coordinate_location::set_values_for_location(Computed_field *field,
	const FE_value *values_in)
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
