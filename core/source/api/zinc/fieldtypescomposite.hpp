/***************************************************************************//**
 * FILE : fieldtypecomposite.hpp
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
#ifndef __ZN_FIELD_TYPES_COMPOSITE_HPP__
#define __ZN_FIELD_TYPES_COMPOSITE_HPP__


#include "zinc/fieldcomposite.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace zinc
{

class FieldIdentity : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldIdentity(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldIdentity FieldModule::createIdentity(Field& sourceField);

public:

	FieldIdentity() : Field(0)
	{	}

};

class FieldComponent : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldComponent(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldComponent FieldModule::createComponent(Field& sourceField, int componentIndex);

public:

	FieldComponent() : Field(0)
	{	}



};

class FieldConcatenate : public Field
{
private:

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldConcatenate(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldConcatenate FieldModule::createConcatenate(int fieldsCount, Field *fields);

public:

	FieldConcatenate() : Field(0)
	{	}

};

inline FieldIdentity FieldModule::createIdentity(Field& sourceField)
{
	return FieldIdentity(Cmiss_field_module_create_identity(id, sourceField.getId()));
}

inline FieldComponent FieldModule::createComponent(Field& sourceField, int componentIndex)
{
	return FieldComponent(Cmiss_field_module_create_component(id,
		sourceField.getId(), componentIndex));
}

inline FieldConcatenate FieldModule::createConcatenate(int fieldsCount, Field *fields)
{
	Cmiss_field_id concatenateField = 0;
	if (fieldsCount > 0)
	{
		Cmiss_field_id *source_fields = new Cmiss_field_id[fieldsCount];
		for (int i = 0; i < fieldsCount; i++)
		{
			source_fields[i] = fields[i].getId();
		}
		concatenateField = Cmiss_field_module_create_concatenate(id, fieldsCount, source_fields);
		delete[] source_fields;
	}
	return FieldConcatenate(concatenateField);
}

}  // namespace zinc

#endif /* __ZN_FIELD_TYPES_COMPOSITE_HPP__ */
