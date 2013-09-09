/***************************************************************************//**
 * FILE : fieldtrigonometry.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDTRIGONOMETRY_HPP__
#define CMZN_FIELDTRIGONOMETRY_HPP__

#include "zinc/fieldtrigonometry.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldSin : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSin(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldCos(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldTan(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldAsin(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldAcos(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldAtan(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldAtan2(cmzn_field_id field_id) : Field(field_id)
	{	}

public:

	FieldAtan2() : Field(0)
	{	}

	friend FieldAtan2 FieldModule::createAtan2(Field& sourceField1,
		Field& sourceField2);

};

inline FieldSin FieldModule::createSin(Field& sourceField)
{
	return FieldSin(cmzn_field_module_create_sin(id, sourceField.getId()));
}

inline FieldCos FieldModule::createCos(Field& sourceField)
{
	return FieldCos(cmzn_field_module_create_cos(id, sourceField.getId()));
}

inline FieldTan FieldModule::createTan(Field& sourceField)
{
	return FieldTan(cmzn_field_module_create_tan(id, sourceField.getId()));
}

inline FieldAsin FieldModule::createAsin(Field& sourceField)
{
	return FieldAsin(cmzn_field_module_create_asin(id, sourceField.getId()));
}

inline FieldAcos FieldModule::createAcos(Field& sourceField)
{
	return FieldAcos(cmzn_field_module_create_acos(id, sourceField.getId()));
}

inline FieldAtan FieldModule::createAtan(Field& sourceField)
{
	return FieldAtan(cmzn_field_module_create_atan(id, sourceField.getId()));
}

inline FieldAtan2 FieldModule::createAtan2(Field& sourceField1, Field& sourceField2)
{
	return FieldAtan2(cmzn_field_module_create_atan2(id, sourceField1.getId(),
		sourceField2.getId()));
}

}  // namespace Zinc
}

#endif
