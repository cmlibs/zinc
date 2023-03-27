/**
 * @file fieldapply.hpp
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDAPPLY_HPP__
#define CMZN_FIELDAPPLY_HPP__

#include "cmlibs/zinc/fieldapply.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"

namespace CMLibs
{
namespace Zinc
{

class FieldApply : public Field
{
	inline cmzn_field_apply_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_apply_id>(this->id);
	}

public:

	FieldApply() : Field()
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldApply(cmzn_field_apply_id field_apply_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_apply_id))
	{ }

	Field getBindArgumentField(int bindIndex) const
	{
		return Field(cmzn_field_apply_get_bind_argument_field(this->getDerivedId(), bindIndex));
	}

	Field getBindArgumentSourceField(const Field& argumentField) const
	{
		return Field(cmzn_field_apply_get_bind_argument_source_field(this->getDerivedId(), argumentField.getId()));
	}

	int setBindArgumentSourceField(const Field& argumentField, const Field& sourceField)
	{
		return cmzn_field_apply_set_bind_argument_source_field(this->getDerivedId(), argumentField.getId(), sourceField.getId());
	}

	int getNumberOfBindings() const
	{
		return cmzn_field_apply_get_number_of_bindings(this->getDerivedId());
	}
};

class FieldArgumentReal : public Field
{
	inline cmzn_field_argument_real_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_argument_real_id>(this->id);
	}

public:

	FieldArgumentReal() : Field()
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldArgumentReal(cmzn_field_argument_real_id field_argument_real_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_argument_real_id))
	{ }

};

inline FieldApply Fieldmodule::createFieldApply(const Field& evaluateField)
{
	return FieldApply(reinterpret_cast<cmzn_field_apply_id>(
		cmzn_fieldmodule_create_field_apply(id, evaluateField.getId())));
}

inline FieldApply Field::castApply()
{
	return FieldApply(cmzn_field_cast_apply(id));
}

inline FieldArgumentReal Fieldmodule::createFieldArgumentReal(int numberOfComponents)
{
	return FieldArgumentReal(reinterpret_cast<cmzn_field_argument_real_id>(
		cmzn_fieldmodule_create_field_argument_real(id, numberOfComponents)));
}

inline FieldArgumentReal Field::castArgumentReal()
{
	return FieldArgumentReal(cmzn_field_cast_argument_real(id));
}

}  // namespace Zinc
}
#endif
