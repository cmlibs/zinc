/***************************************************************************//**
 * FILE : fieldtypevectoroperators.hpp
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
#ifndef __FIELD_TYPES_VECTOR_OPERATORS_HPP__
#define __FIELD_TYPES_VECTOR_OPERATORS_HPP__

extern "C" {
#include "api/cmiss_field_vector_operators.h"
}
#include "api++/field.hpp"
#include "api++/fieldmodule.hpp"

namespace Zn
{

class FieldCrossProduct : public Field
{
public:

	FieldCrossProduct() : Field(NULL)
	{	}

	FieldCrossProduct(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldCrossProduct3D : public Field
{
public:

	FieldCrossProduct3D() : Field(NULL)
	{	}

	FieldCrossProduct3D(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldDotProduct : public Field
{
public:

	FieldDotProduct() : Field(NULL)
	{	}

	FieldDotProduct(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldMagnitude : public Field
{
public:

	FieldMagnitude() : Field(NULL)
	{	}

	FieldMagnitude(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldNormalise : public Field
{
public:

	FieldNormalise() : Field(NULL)
	{	}

	FieldNormalise(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

inline FieldCrossProduct FieldModule::createCrossProduct(int dimension, Field* sourceFields)
{
	Cmiss_field_id field = NULL;
	if (dimension - 1 > 0)
	{
		Cmiss_field_id *source_fields = new Cmiss_field_id[dimension - 1];
		for (int i = 0; i < (dimension - 1); i++)
		{
			source_fields[i] = sourceFields[i].getId();
		}
		field = Cmiss_field_module_create_cross_product(id,
			dimension, source_fields);
		delete[] source_fields;
	}
	return FieldCrossProduct(field);
}

inline FieldCrossProduct3D FieldModule::createCrossProduct3D(Field& sourceField1, Field& sourceField2)
{
	return FieldCrossProduct3D(Cmiss_field_module_create_cross_product_3d(id, sourceField1.getId(),
		sourceField2.getId()));
}

inline FieldDotProduct FieldModule::createDotProduct(Field& sourceField1, Field& sourceField2)
{
	return FieldDotProduct(Cmiss_field_module_create_dot_product(id, sourceField1.getId(),
		sourceField2.getId()));
}

inline FieldMagnitude FieldModule::createMagnitude(Field& sourceField)
{
	return FieldMagnitude(Cmiss_field_module_create_magnitude(id, sourceField.getId()));
}

inline FieldNormalise FieldModule::createNormalise(Field& sourceField)
{
	return FieldNormalise(Cmiss_field_module_create_normalise(id, sourceField.getId()));
}

}  // namespace Zn

#endif /* __FIELD_TYPES_VECTOR_OPERATORS_HPP__ */
