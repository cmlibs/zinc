/***************************************************************************//**
 * FILE : fieldcomposite.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCOMPOSITE_HPP__
#define CMZN_FIELDCOMPOSITE_HPP__


#include "zinc/fieldcomposite.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldIdentity : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldIdentity(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldIdentity Fieldmodule::createFieldIdentity(Field& sourceField);

public:

	FieldIdentity() : Field(0)
	{	}

};

class FieldComponent : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldComponent(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldComponent Fieldmodule::createFieldComponent(Field& sourceField, int componentIndex);

public:

	FieldComponent() : Field(0)
	{	}

	int setComponentIndex(int componentIndex)
	{
		return cmzn_field_component_set_component_index(
			reinterpret_cast<cmzn_field_component_id>(id), componentIndex);
	}

};

class FieldConcatenate : public Field
{
private:

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldConcatenate(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldConcatenate Fieldmodule::createFieldConcatenate(int fieldsCount, Field *sourceFields);

public:

	FieldConcatenate() : Field(0)
	{	}

};

inline FieldIdentity Fieldmodule::createFieldIdentity(Field& sourceField)
{
	return FieldIdentity(cmzn_fieldmodule_create_field_identity(id, sourceField.getId()));
}

inline FieldComponent Fieldmodule::createFieldComponent(Field& sourceField, int componentIndex)
{
	return FieldComponent(cmzn_fieldmodule_create_field_component(id,
		sourceField.getId(), componentIndex));
}

inline FieldConcatenate Fieldmodule::createFieldConcatenate(int fieldsCount, Field *sourceFields)
{
	cmzn_field_id concatenateField = 0;
	if (fieldsCount > 0)
	{
		cmzn_field_id *source_fields = new cmzn_field_id[fieldsCount];
		for (int i = 0; i < fieldsCount; i++)
		{
			source_fields[i] = sourceFields[i].getId();
		}
		concatenateField = cmzn_fieldmodule_create_field_concatenate(id, fieldsCount, source_fields);
		delete[] source_fields;
	}
	return FieldConcatenate(concatenateField);
}

}  // namespace Zinc
}

#endif
