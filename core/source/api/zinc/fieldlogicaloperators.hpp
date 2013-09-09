/***************************************************************************//**
 * FILE : fieldlogicaloperators.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDLOGICALOPERATORS_HPP__
#define CMZN_FIELDLOGICALOPERATORS_HPP__

#include "zinc/fieldlogicaloperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldAnd : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAnd(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldAnd FieldModule::createAnd(Field& sourceField1,
		Field& sourceField2);

public:

	FieldAnd() : Field(0)
	{	}

};

inline FieldAnd operator&&(Field& operand1, Field& operand2)
{
    FieldModule fieldModule(operand1);
    return fieldModule.createAnd(operand1, operand2);
}

class FieldEqualTo : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEqualTo(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEqualTo FieldModule::createEqualTo(Field& sourceField1,
		Field& sourceField2);

public:

	FieldEqualTo() : Field(0)
	{	}

};

class FieldGreaterThan : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGreaterThan(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldGreaterThan FieldModule::createGreaterThan(Field& sourceField1,
		Field& sourceField2);

public:

	FieldGreaterThan() : Field(0)
	{	}

};

inline FieldGreaterThan operator>(Field& operand1, Field& operand2)
{
    FieldModule fieldModule(operand1);
    return fieldModule.createGreaterThan(operand1, operand2);
}

class FieldLessThan : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldLessThan(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldLessThan FieldModule::createLessThan(Field& sourceField1, Field& sourceField2);

public:

	FieldLessThan() : Field(0)
	{	}

};

inline FieldLessThan operator<(Field& operand1, Field& operand2)
{
    FieldModule fieldModule(operand1);
    return fieldModule.createLessThan(operand1, operand2);
}

class FieldOr : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldOr(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldOr FieldModule::createOr(Field& sourceField1,
		Field& sourceField2);

public:

	FieldOr() : Field(0)
	{	}

};

inline FieldOr operator||(Field& operand1, Field& operand2)
{
    FieldModule fieldModule(operand1);
    return fieldModule.createOr(operand1, operand2);
}

class FieldNot : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNot(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldNot FieldModule::createNot(Field& sourceField);

public:

	FieldNot() : Field(0)
	{	}

};

inline FieldNot operator!(Field& operand)
{
    FieldModule fieldModule(operand);
    return fieldModule.createNot(operand);
}

class FieldXor : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldXor(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldXor FieldModule::createXor(Field& sourceField1,
		Field& sourceField2);

public:

	FieldXor() : Field(0)
	{	}

};

inline FieldAnd FieldModule::createAnd(Field& sourceField1, Field& sourceField2)
{
	return FieldAnd(cmzn_field_module_create_and(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldEqualTo FieldModule::createEqualTo(Field& sourceField1, Field& sourceField2)
{
	return FieldEqualTo(cmzn_field_module_create_equal_to(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldGreaterThan FieldModule::createGreaterThan(Field& sourceField1, Field& sourceField2)
{
	return FieldGreaterThan(cmzn_field_module_create_greater_than(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldLessThan FieldModule::createLessThan(Field& sourceField1, Field& sourceField2)
{
	return FieldLessThan(cmzn_field_module_create_less_than(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldOr FieldModule::createOr(Field& sourceField1, Field& sourceField2)
{
	return FieldOr(cmzn_field_module_create_or(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldNot FieldModule::createNot(Field& sourceField)
{
	return FieldNot(cmzn_field_module_create_not(id, sourceField.getId()));
}

inline FieldXor FieldModule::createXor(Field& sourceField1, Field& sourceField2)
{
	return FieldXor(cmzn_field_module_create_xor(id,
		sourceField1.getId(), sourceField2.getId()));
}

}  // namespace Zinc
}

#endif
