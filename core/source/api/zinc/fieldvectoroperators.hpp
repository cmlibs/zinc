/***************************************************************************//**
 * FILE : fieldvectoroperators.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDVECTOROPERATORS_HPP__
#define CMZN_FIELDVECTOROPERATORS_HPP__

#include "zinc/fieldvectoroperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldCrossProduct : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCrossProduct(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldCrossProduct FieldModule::createCrossProduct(int fieldsCount,
		Field *sourceFields);

	friend FieldCrossProduct FieldModule::createCrossProduct(
		Field& sourceField1, Field& sourceField2);

public:

	FieldCrossProduct() : Field(0)
	{	}

};

class FieldDotProduct : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDotProduct(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldDotProduct FieldModule::createDotProduct(Field& sourceField1,
		Field& sourceField2);

public:

	FieldDotProduct() : Field(0)
	{	}

};

class FieldMagnitude : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMagnitude(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldMagnitude FieldModule::createMagnitude(Field& sourceField);

public:

	FieldMagnitude() : Field(0)
	{	}

};

class FieldNormalise : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNormalise(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldNormalise FieldModule::createNormalise(Field& sourceField);

public:

	FieldNormalise() : Field(0)
	{	}

};

class FieldSumComponents : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSumComponents(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldSumComponents FieldModule::createSumComponents(Field& sourceField);

public:

	FieldSumComponents() : Field(0)
	{	}

};

inline FieldCrossProduct FieldModule::createCrossProduct(int fieldsCount, Field *sourceFields)
{
	cmzn_field_id field = 0;
	if (fieldsCount > 0)
	{
		cmzn_field_id *source_fields = new cmzn_field_id[fieldsCount];
		for (int i = 0; i < fieldsCount; i++)
		{
			source_fields[i] = sourceFields[i].getId();
		}
		field = cmzn_field_module_create_cross_product(id, fieldsCount, source_fields);
		delete[] source_fields;
	}
	return FieldCrossProduct(field);
}

inline FieldCrossProduct FieldModule::createCrossProduct(Field& sourceField1, Field& sourceField2)
{
	return FieldCrossProduct(cmzn_field_module_create_cross_product_3d(id, sourceField1.getId(),
		sourceField2.getId()));
}

inline FieldDotProduct FieldModule::createDotProduct(Field& sourceField1, Field& sourceField2)
{
	return FieldDotProduct(cmzn_field_module_create_dot_product(id, sourceField1.getId(),
		sourceField2.getId()));
}

inline FieldMagnitude FieldModule::createMagnitude(Field& sourceField)
{
	return FieldMagnitude(cmzn_field_module_create_magnitude(id, sourceField.getId()));
}

inline FieldNormalise FieldModule::createNormalise(Field& sourceField)
{
	return FieldNormalise(cmzn_field_module_create_normalise(id, sourceField.getId()));
}

inline FieldSumComponents FieldModule::createSumComponents(Field& sourceField)
{
	return FieldSumComponents(cmzn_field_module_create_sum_components(id,
		sourceField.getId()));
}

}  // namespace Zinc
}

#endif
