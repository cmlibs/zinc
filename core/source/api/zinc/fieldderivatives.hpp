/***************************************************************************//**
 * FILE : fieldderivatives.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDDERIVATIVES_HPP__
#define CMZN_FIELDDERIVATIVES_HPP__

#include "zinc/fieldderivatives.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldDerivative : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDerivative(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldDerivative FieldModule::createDerivative(Field& sourceField, int xi_index);

public:

	FieldDerivative() : Field(0)
	{	}

};

class FieldCurl : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCurl(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldCurl FieldModule::createCurl(Field& vectorField, Field& coordinateField);

public:

	FieldCurl() : Field(0)
	{	}

};

class FieldDivergence : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDivergence(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldDivergence FieldModule::createDivergence(Field& vectorField, Field& coordinateField);

public:

	FieldDivergence() : Field(0)
	{	}

};

class FieldGradient : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGradient(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldGradient FieldModule::createGradient(Field& sourceField, Field& coordinateField);

public:

	FieldGradient() : Field(0)
	{	}

};

inline FieldDerivative FieldModule::createDerivative(Field& sourceField, int xi_index)
{
	return FieldDerivative(cmzn_field_module_create_derivative(id, sourceField.getId(), xi_index));
}

inline FieldCurl FieldModule::createCurl(Field& vectorField, Field& coordinateField)
{
	return FieldCurl(cmzn_field_module_create_curl(id, vectorField.getId(), coordinateField.getId()));
}

inline FieldDivergence FieldModule::createDivergence(Field& vectorField, Field& coordinateField)
{
	return FieldDivergence(cmzn_field_module_create_divergence(id, vectorField.getId(), coordinateField.getId()));
}

inline FieldGradient FieldModule::createGradient(Field& sourceField, Field& coordinateField)
{
	return FieldGradient(cmzn_field_module_create_gradient(id, sourceField.getId(),
		coordinateField.getId()));
}

}  // namespace Zinc
}

#endif
