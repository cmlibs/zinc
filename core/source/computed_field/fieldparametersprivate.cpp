/**
 * FILE : fieldparametersprivate.cpp
 * 
 * Implementation of field parameters class.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldfiniteelement.h"
#include "opencmiss/zinc/result.h"
#include "computed_field/fieldparametersprivate.hpp"
#include "computed_field/computed_field_private.hpp"
#include "general/debug.h"
#include "general/message.h"

cmzn_fieldparameters::cmzn_fieldparameters(cmzn_field *fieldIn) :
	field(fieldIn->access()),
	access_count(1)
{
}

cmzn_fieldparameters::~cmzn_fieldparameters()
{
	this->field->clearFieldparameters();
	cmzn_field::deaccess(&(this->field));
}

cmzn_fieldparameters *cmzn_fieldparameters::create(cmzn_field *fieldIn)
{
	cmzn_field_finite_element_id feField = cmzn_field_cast_finite_element(fieldIn);
	if (feField)
	{
		cmzn_field_finite_element_destroy(&feField);
		return new cmzn_fieldparameters(fieldIn);
	}
	display_message(ERROR_MESSAGE, "Field getFieldparameters:  Can only get from finite element field");
	return nullptr;
}

int cmzn_fieldparameters::deaccess(cmzn_fieldparameters_id &fieldparameters)
{
	if (!fieldparameters)
		return CMZN_RESULT_ERROR_ARGUMENT;
	--(fieldparameters->access_count);
	if (fieldparameters->access_count <= 0)
		delete fieldparameters;
	fieldparameters = 0;
	return CMZN_RESULT_OK;
}

int cmzn_fieldparameters::getNumberOfParameters() const
{
	display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfParameters:  Not implemented");
	return -1;
}

/*
Global functions
----------------
*/

cmzn_fieldparameters_id cmzn_fieldparameters_access(cmzn_fieldparameters_id fieldparameters)
{
	if (fieldparameters)
		return fieldparameters->access();
	return 0;
}

int cmzn_fieldparameters_destroy(cmzn_fieldparameters_id *fieldparameters_address)
{
	if (!fieldparameters_address)
		return CMZN_RESULT_ERROR_ARGUMENT;
	return cmzn_fieldparameters::deaccess(*fieldparameters_address);
}

cmzn_field_id cmzn_fieldparameters_get_field(
	cmzn_fieldparameters_id fieldparameters)
{
	if (fieldparameters)
		return cmzn_field_access(fieldparameters->getField());
	display_message(ERROR_MESSAGE, "Fieldparameters getField:  Invalid field parameters object");
	return nullptr;
}

int cmzn_fieldparameters_get_number_of_parameters(
	cmzn_fieldparameters_id fieldparameters)
{
	if (fieldparameters)
		return fieldparameters->getNumberOfParameters();
	return -1;
}
