/***************************************************************************//**
 * FILE : fieldtypearithmeticoperators.hpp
 */
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef __ZN_FIELD_TYPES_ARITHMETIC_OPERATORS_HPP__
#define __ZN_FIELD_TYPES_ARITHMETIC_OPERATORS_HPP__

#include "zinc/fieldarithmeticoperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace zinc
{

class FieldAdd : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAdd(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldPower(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldMultiply(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldDivide(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldSubtract(Cmiss_field_id field_id) : Field(field_id)
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

class FieldSumComponents : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSumComponents(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldSumComponents FieldModule::FieldModule::createSumComponents(Field& sourceField, double *weights);

public:

	FieldSumComponents() : Field(0)
	{	}

};

class FieldLog : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldLog(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldSqrt(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldExp(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldAbs(Cmiss_field_id field_id) : Field(field_id)
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
	return FieldAdd(Cmiss_field_module_create_add(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldPower FieldModule::createPower(Field& sourceField1, Field& sourceField2)
{
	return FieldPower(Cmiss_field_module_create_power(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldMultiply FieldModule::createMultiply(Field& sourceField1, Field& sourceField2)
{
	return FieldMultiply(Cmiss_field_module_create_multiply(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldDivide FieldModule::createDivide(Field& sourceField1, Field& sourceField2)
{
	return FieldDivide(Cmiss_field_module_create_divide(id,
			sourceField1.getId(), sourceField2.getId()));
}

inline FieldSubtract FieldModule::createSubtract(Field& sourceField1, Field& sourceField2)
{
	return FieldSubtract(Cmiss_field_module_create_subtract(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldSumComponents FieldModule::createSumComponents(Field& sourceField, double *weights)
{
	return FieldSumComponents(Cmiss_field_module_create_sum_components(id,
		sourceField.getId(), weights));
}

inline FieldLog FieldModule::createLog(Field& sourceField)
{
	return FieldLog(Cmiss_field_module_create_log(id, sourceField.getId()));
}

inline FieldSqrt FieldModule::createSqrt(Field& sourceField)
{
	return FieldSqrt(Cmiss_field_module_create_sqrt(id, sourceField.getId()));
}

inline FieldExp FieldModule::createExp(Field& sourceField)
{
	return FieldExp(Cmiss_field_module_create_exp(id, sourceField.getId()));
}

inline FieldAbs FieldModule::createAbs(Field& sourceField)
{
	return FieldAbs(Cmiss_field_module_create_abs(id, sourceField.getId()));
}

}  // namespace zinc

#endif /* __ZN_FIELD_TYPES_ARITHMETIC_OPERATORS_HPP__ */
