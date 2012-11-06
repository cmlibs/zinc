/***************************************************************************//**
 * FILE : fieldtypestrigonometry.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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
#ifndef __ZN_FIELD_TYPES_TRIGONOMETRY_HPP__
#define __ZN_FIELD_TYPES_TRIGONOMETRY_HPP__

#include "zinc/fieldtrigonometry.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace zinc
{

class FieldSin : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSin(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldSin FieldModule::createSin(Field& sourceField);

public:

	FieldSin() : Field(0)
	{	}

};

class FieldCos : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCos(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldCos FieldModule::createCos(Field& sourceField);

public:

	FieldCos() : Field(0)
	{	}

};

class FieldTan : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldTan(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldTan FieldModule::createTan(Field& sourceField);

public:

	FieldTan() : Field(0)
	{	}

};

class FieldAsin : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAsin(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldAsin FieldModule::createAsin(Field& sourceField);

public:

	FieldAsin() : Field(0)
	{	}

};

class FieldAcos : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAcos(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldAcos FieldModule::createAcos(Field& sourceField);

public:

	FieldAcos() : Field(0)
	{	}

};

class FieldAtan : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAtan(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldAtan FieldModule::createAtan(Field& sourceField);

public:

	FieldAtan() : Field(0)
	{	}

};

class FieldAtan2 : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldAtan2(Cmiss_field_id field_id) : Field(field_id)
	{	}

public:

	FieldAtan2() : Field(0)
	{	}

	friend FieldAtan2 FieldModule::createAtan2(Field& sourceField1,
		Field& sourceField2);

};

inline FieldSin FieldModule::createSin(Field& sourceField)
{
	return FieldSin(Cmiss_field_module_create_sin(id, sourceField.getId()));
}

inline FieldCos FieldModule::createCos(Field& sourceField)
{
	return FieldCos(Cmiss_field_module_create_cos(id, sourceField.getId()));
}

inline FieldTan FieldModule::createTan(Field& sourceField)
{
	return FieldTan(Cmiss_field_module_create_tan(id, sourceField.getId()));
}

inline FieldAsin FieldModule::createAsin(Field& sourceField)
{
	return FieldAsin(Cmiss_field_module_create_asin(id, sourceField.getId()));
}

inline FieldAcos FieldModule::createAcos(Field& sourceField)
{
	return FieldAcos(Cmiss_field_module_create_acos(id, sourceField.getId()));
}

inline FieldAtan FieldModule::createAtan(Field& sourceField)
{
	return FieldAtan(Cmiss_field_module_create_atan(id, sourceField.getId()));
}

inline FieldAtan2 FieldModule::createAtan2(Field& sourceField1, Field& sourceField2)
{
	return FieldAtan2(Cmiss_field_module_create_atan2(id, sourceField1.getId(),
		sourceField2.getId()));
}

}  // namespace zinc

#endif /* __ZN_FIELD_TYPES_TRIGONOMETRY_HPP__ */
