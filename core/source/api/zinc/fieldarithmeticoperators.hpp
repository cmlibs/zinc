/***************************************************************************//**
 * FILE : fieldarithmeticoperators.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDARITHMETICOPERATORS_HPP__
#define CMZN_FIELDARITHMETICOPERATORS_HPP__

#include "zinc/fieldarithmeticoperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldAdd : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAdd(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldAdd Fieldmodule::createFieldAdd(Field& sourceField1, Field& sourceField2);

public:

	FieldAdd() : Field(0)
	{ }

};

inline FieldAdd operator+(Field& operand1, Field& operand2)
{
	return operand1.getFieldmodule().createFieldAdd(operand1, operand2);
}

class FieldPower : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldPower(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldPower Fieldmodule::createFieldPower(Field& sourceField1, Field& sourceField2);

public:

	FieldPower() : Field(0)
	{ }

};

class FieldMultiply : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMultiply(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldMultiply Fieldmodule::createFieldMultiply(Field& sourceField1, Field& sourceField2);

public:

	FieldMultiply() : Field(0)
	{ }

};

inline FieldMultiply operator*(Field& operand1, Field& operand2)
{
	return operand1.getFieldmodule().createFieldMultiply(operand1, operand2);
}

class FieldDivide : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDivide(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldDivide Fieldmodule::createFieldDivide(Field& sourceField1, Field& sourceField2);

public:

	FieldDivide() : Field(0)
	{	}

};

inline FieldDivide operator/(Field& operand1, Field& operand2)
{
	return operand1.getFieldmodule().createFieldDivide(operand1, operand2);
}

class FieldSubtract : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSubtract(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldSubtract Fieldmodule::createFieldSubtract(Field& sourceField1, Field& sourceField2);

public:

	FieldSubtract() : Field(0)
	{	}

};

inline FieldSubtract operator-(Field& operand1, Field& operand2)
{
	return operand1.getFieldmodule().createFieldSubtract(operand1, operand2);
}

class FieldLog : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldLog(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldLog Fieldmodule::createFieldLog(Field& sourceField);

public:

	FieldLog() : Field(0)
	{	}

};

inline FieldLog log(Field& sourceField)
{
	return sourceField.getFieldmodule().createFieldLog(sourceField);
}

class FieldSqrt : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSqrt(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldSqrt Fieldmodule::createFieldSqrt(Field& sourceField);

public:

	FieldSqrt() : Field(0)
	{	}

};

inline FieldSqrt sqrt(Field& sourceField)
{
	return sourceField.getFieldmodule().createFieldSqrt(sourceField);
}

class FieldExp : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldExp(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldExp Fieldmodule::createFieldExp(Field& sourceField);

public:

	FieldExp() : Field(0)
	{	}

};

inline FieldExp exp(Field& sourceField)
{
	return sourceField.getFieldmodule().createFieldExp(sourceField);
}

class FieldAbs : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAbs(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldAbs Fieldmodule::createFieldAbs(Field& sourceField);

public:

	FieldAbs() : Field(0)
	{	}

};

inline FieldAbs abs(Field& sourceField)
{
	return sourceField.getFieldmodule().createFieldAbs(sourceField);
}

/* inline Fieldmodule factory methods */

inline FieldAdd Fieldmodule::createFieldAdd(Field& sourceField1, Field& sourceField2)
{
	return FieldAdd(cmzn_fieldmodule_create_field_add(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldPower Fieldmodule::createFieldPower(Field& sourceField1, Field& sourceField2)
{
	return FieldPower(cmzn_fieldmodule_create_field_power(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldMultiply Fieldmodule::createFieldMultiply(Field& sourceField1, Field& sourceField2)
{
	return FieldMultiply(cmzn_fieldmodule_create_field_multiply(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldDivide Fieldmodule::createFieldDivide(Field& sourceField1, Field& sourceField2)
{
	return FieldDivide(cmzn_fieldmodule_create_field_divide(id,
			sourceField1.getId(), sourceField2.getId()));
}

inline FieldSubtract Fieldmodule::createFieldSubtract(Field& sourceField1, Field& sourceField2)
{
	return FieldSubtract(cmzn_fieldmodule_create_field_subtract(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldLog Fieldmodule::createFieldLog(Field& sourceField)
{
	return FieldLog(cmzn_fieldmodule_create_field_log(id, sourceField.getId()));
}

inline FieldSqrt Fieldmodule::createFieldSqrt(Field& sourceField)
{
	return FieldSqrt(cmzn_fieldmodule_create_field_sqrt(id, sourceField.getId()));
}

inline FieldExp Fieldmodule::createFieldExp(Field& sourceField)
{
	return FieldExp(cmzn_fieldmodule_create_field_exp(id, sourceField.getId()));
}

inline FieldAbs Fieldmodule::createFieldAbs(Field& sourceField)
{
	return FieldAbs(cmzn_fieldmodule_create_field_abs(id, sourceField.getId()));
}

}  // namespace Zinc
}

#endif
