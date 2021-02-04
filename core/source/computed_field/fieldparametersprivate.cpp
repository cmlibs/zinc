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
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element_field.hpp"
#include "finite_element/finite_element_field_parameters.hpp"
#include "general/debug.h"
#include "general/message.h"

cmzn_fieldparameters::cmzn_fieldparameters(cmzn_field *fieldIn, FE_field_parameters *feFieldParametersIn) :
	field(fieldIn->access()),
	feFieldParameters(feFieldParametersIn),  // take over access count
	access_count(1)
{
}

cmzn_fieldparameters::~cmzn_fieldparameters()
{
	FE_field_parameters::deaccess(this->feFieldParameters);
	this->field->clearFieldparameters();
	cmzn_field::deaccess(&(this->field));
}

cmzn_fieldparameters *cmzn_fieldparameters::create(cmzn_field *fieldIn)
{
	FE_field *feField = nullptr;
	if (Computed_field_get_type_finite_element(fieldIn, &feField))
	{
		// this requires a field is real, general type
		FE_field_parameters *feFieldParameters = feField->get_FE_field_parameters();
		if (feFieldParameters)
			return new cmzn_fieldparameters(fieldIn, feFieldParameters);
	}
	display_message(ERROR_MESSAGE, "Field getFieldparameters:  Can only get from finite element field");
	return nullptr;
}

int cmzn_fieldparameters::deaccess(cmzn_fieldparameters* &fieldparameters)
{
	if (!fieldparameters)
		return CMZN_RESULT_ERROR_ARGUMENT;
	--(fieldparameters->access_count);
	if (fieldparameters->access_count <= 0)
		delete fieldparameters;
	fieldparameters = 0;
	return CMZN_RESULT_OK;
}

int cmzn_fieldparameters::getNumberOfElementParameters(cmzn_element *element) const
{
	return this->feFieldParameters->getNumberOfElementParameters(element);
}

int cmzn_fieldparameters::getNumberOfParameters() const
{
	return this->feFieldParameters->getNumberOfParameters();
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

int cmzn_fieldparameters_get_number_of_element_parameters(
	cmzn_fieldparameters_id fieldparameters, cmzn_element_id element)
{
	if (fieldparameters)
		return fieldparameters->getNumberOfElementParameters(element);
	display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfElementParameters:  Invalid Fieldparameters");
	return -1;
}

int cmzn_fieldparameters_get_number_of_parameters(
	cmzn_fieldparameters_id fieldparameters)
{
	if (fieldparameters)
		return fieldparameters->getNumberOfParameters();
	display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfParameters:  Invalid Fieldparameters");
	return -1;
}
