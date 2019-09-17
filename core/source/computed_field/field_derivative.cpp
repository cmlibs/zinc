/**
 * FILE : field_derivative.cpp
 *
 * Field derivative defining order and type of derivative operator to apply
 * to any given field. Each Field_derivative has a unique index within its
 * owning region for efficient look up in field cache. Object describes how to
 * evaluate derivative, including links to next lower Field_derivative so
 * can evaluate downstream derivatives using rules.
 */
 /* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/status.h"
#include "computed_field/field_derivative.hpp"
#include "finite_element/finite_element_region.h"

Field_derivative::~Field_derivative()
{
	if (this->region)
		cmzn_region_remove_field_derivative(this->region, this);
	if (this->lower_derivative)
		Field_derivative::deaccess(this->lower_derivative);
}

int Field_derivative::deaccess(Field_derivative* &field_derivative)
{
	if (!field_derivative)
		return CMZN_ERROR_ARGUMENT;
	--(field_derivative->access_count);
	if (field_derivative->access_count <= 0)
		delete field_derivative;
	field_derivative = 0;
	return CMZN_OK;
}
