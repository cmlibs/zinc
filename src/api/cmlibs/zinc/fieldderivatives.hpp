/**
 * @file fieldderivatives.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDDERIVATIVES_HPP__
#define CMZN_FIELDDERIVATIVES_HPP__

#include "cmlibs/zinc/fieldderivatives.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldDerivative : public Field
{
private:
	inline cmzn_field_derivative_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_derivative_id>(this->id);
	}

public:
	FieldDerivative() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDerivative(cmzn_field_derivative_id field_derivative_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_derivative_id))
	{	}

	int getXiIndex() const
	{
		return cmzn_field_derivative_get_xi_index(this->getDerivedId());
	}

	int setXiIndex(int xiIndex)
	{
		return cmzn_field_derivative_set_xi_index(this->getDerivedId(), xiIndex);
	}

};

class FieldCurl : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCurl(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldCurl Fieldmodule::createFieldCurl(const Field& vectorField, const Field& coordinateField);

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

	friend FieldDivergence Fieldmodule::createFieldDivergence(const Field& vectorField, const Field& coordinateField);

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

	friend FieldGradient Fieldmodule::createFieldGradient(const Field& sourceField, const Field& coordinateField);

public:

	FieldGradient() : Field(0)
	{	}

};

inline FieldDerivative Fieldmodule::createFieldDerivative(const Field& sourceField, int xiIndex)
{
	return FieldDerivative(reinterpret_cast<cmzn_field_derivative_id>(
		cmzn_fieldmodule_create_field_derivative(this->id, sourceField.getId(), xiIndex)));
}

inline FieldDerivative Field::castDerivative()
{
	return FieldDerivative(cmzn_field_cast_derivative(this->id));
}

inline FieldCurl Fieldmodule::createFieldCurl(const Field& vectorField, const Field& coordinateField)
{
	return FieldCurl(cmzn_fieldmodule_create_field_curl(id, vectorField.getId(), coordinateField.getId()));
}

inline FieldDivergence Fieldmodule::createFieldDivergence(const Field& vectorField, const Field& coordinateField)
{
	return FieldDivergence(cmzn_fieldmodule_create_field_divergence(id, vectorField.getId(), coordinateField.getId()));
}

inline FieldGradient Fieldmodule::createFieldGradient(const Field& sourceField, const Field& coordinateField)
{
	return FieldGradient(cmzn_fieldmodule_create_field_gradient(id, sourceField.getId(),
		coordinateField.getId()));
}

}  // namespace Zinc
}

#endif
