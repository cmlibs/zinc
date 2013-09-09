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

	friend FieldAdd FieldModule::createAdd(Field& sourceField1, Field& sourceField2);

public:

	FieldAdd() : Field(0)
	{ }

};

inline FieldAdd operator+(Field& operand1, Field& operand2)
{
	FieldModule fieldModule(operand1);
	return fieldModule.createAdd(operand1, operand2);
}

class FieldPower : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldPower(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldPower FieldModule::createPower(Field& sourceField1, Field& sourceField2);

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

	friend FieldMultiply FieldModule::createMultiply(Field& sourceField1, Field& sourceField2);

public:

	FieldMultiply() : Field(0)
	{ }

};

inline FieldMultiply operator*(Field& operand1, Field& operand2)
{
   FieldModule fieldModule(operand1);
	return fieldModule.createMultiply(operand1, operand2);
}

class FieldDivide : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDivide(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldDivide FieldModule::createDivide(Field& sourceField1, Field& sourceField2);

public:

	FieldDivide() : Field(0)
	{	}

};

inline FieldDivide operator/(Field& operand1, Field& operand2)
{
   FieldModule fieldModule(operand1);
	return fieldModule.createDivide(operand1, operand2);
}

class FieldSubtract : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSubtract(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldSubtract FieldModule::createSubtract(Field& sourceField1, Field& sourceField2);

public:

	FieldSubtract() : Field(0)
	{	}

};

inline FieldSubtract operator-(Field& operand1, Field& operand2)
{
   FieldModule fieldModule(operand1);
	return fieldModule.createSubtract(operand1, operand2);
}

class FieldLog : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldLog(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldLog FieldModule::createLog(Field& sourceField);

public:

	FieldLog() : Field(0)
	{	}

};

inline FieldLog log(Field& sourceField)
{
	FieldModule fieldModule(sourceField);
	return fieldModule.createLog(sourceField);
}

class FieldSqrt : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSqrt(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldSqrt FieldModule::createSqrt(Field& sourceField);

public:

	FieldSqrt() : Field(0)
	{	}

};

inline FieldSqrt sqrt(Field& sourceField)
{
	FieldModule fieldModule(sourceField);
	return fieldModule.createSqrt(sourceField);
}

class FieldExp : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldExp(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldExp FieldModule::createExp(Field& sourceField);

public:

	FieldExp() : Field(0)
	{	}

};

inline FieldExp exp(Field& sourceField)
{
	FieldModule fieldModule(sourceField);
	return fieldModule.createExp(sourceField);
}

class FieldAbs : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAbs(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldAbs FieldModule::createAbs(Field& sourceField);

public:

	FieldAbs() : Field(0)
	{	}

};

inline FieldAbs abs(Field& sourceField)
{
	FieldModule fieldModule(sourceField);
	return fieldModule.createAbs(sourceField);
}

/* inline FieldModule factory methods */

inline FieldAdd FieldModule::createAdd(Field& sourceField1, Field& sourceField2)
{
	return FieldAdd(cmzn_field_module_create_add(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldPower FieldModule::createPower(Field& sourceField1, Field& sourceField2)
{
	return FieldPower(cmzn_field_module_create_power(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldMultiply FieldModule::createMultiply(Field& sourceField1, Field& sourceField2)
{
	return FieldMultiply(cmzn_field_module_create_multiply(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldDivide FieldModule::createDivide(Field& sourceField1, Field& sourceField2)
{
	return FieldDivide(cmzn_field_module_create_divide(id,
			sourceField1.getId(), sourceField2.getId()));
}

inline FieldSubtract FieldModule::createSubtract(Field& sourceField1, Field& sourceField2)
{
	return FieldSubtract(cmzn_field_module_create_subtract(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldLog FieldModule::createLog(Field& sourceField)
{
	return FieldLog(cmzn_field_module_create_log(id, sourceField.getId()));
}

inline FieldSqrt FieldModule::createSqrt(Field& sourceField)
{
	return FieldSqrt(cmzn_field_module_create_sqrt(id, sourceField.getId()));
}

inline FieldExp FieldModule::createExp(Field& sourceField)
{
	return FieldExp(cmzn_field_module_create_exp(id, sourceField.getId()));
}

inline FieldAbs FieldModule::createAbs(Field& sourceField)
{
	return FieldAbs(cmzn_field_module_create_abs(id, sourceField.getId()));
}

}  // namespace Zinc
}

#endif
