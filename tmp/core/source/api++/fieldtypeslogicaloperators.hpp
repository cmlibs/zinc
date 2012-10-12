/***************************************************************************//**
 * FILE : fieldtypeslogicaloperators.hpp
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
#ifndef __ZN_FIELD_TYPES_LOGICAL_OPERATORS_HPP__
#define __ZN_FIELD_TYPES_LOGICAL_OPERATORS_HPP__

#include "api/cmiss_field_logical_operators.h"
#include "api++/field.hpp"
#include "api++/fieldmodule.hpp"

namespace Zn
{

class FieldAnd : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAnd(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldEqualTo(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldGreaterThan(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldLessThan(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldOr(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldNot(Cmiss_field_id field_id) : Field(field_id)
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
	explicit FieldXor(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldXor FieldModule::createXor(Field& sourceField1,
		Field& sourceField2);

public:

	FieldXor() : Field(0)
	{	}

};

inline FieldAnd FieldModule::createAnd(Field& sourceField1, Field& sourceField2)
{
	return FieldAnd(Cmiss_field_module_create_and(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldEqualTo FieldModule::createEqualTo(Field& sourceField1, Field& sourceField2)
{
	return FieldEqualTo(Cmiss_field_module_create_equal_to(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldGreaterThan FieldModule::createGreaterThan(Field& sourceField1, Field& sourceField2)
{
	return FieldGreaterThan(Cmiss_field_module_create_greater_than(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldLessThan FieldModule::createLessThan(Field& sourceField1, Field& sourceField2)
{
	return FieldLessThan(Cmiss_field_module_create_less_than(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldOr FieldModule::createOr(Field& sourceField1, Field& sourceField2)
{
	return FieldOr(Cmiss_field_module_create_or(id,
		sourceField1.getId(), sourceField2.getId()));
}

inline FieldNot FieldModule::createNot(Field& sourceField)
{
	return FieldNot(Cmiss_field_module_create_not(id, sourceField.getId()));
}

inline FieldXor FieldModule::createXor(Field& sourceField1, Field& sourceField2)
{
	return FieldXor(Cmiss_field_module_create_xor(id,
		sourceField1.getId(), sourceField2.getId()));
}

}  // namespace Zn

#endif /* __ZN_FIELD_TYPES_LOGICAL_OPERATORS_HPP__ */
