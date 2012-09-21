/***************************************************************************//**
 * FILE : fieldtypesmatrixoperators.hpp
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
#ifndef __FIELD_TYPES_MATRIX_OPERATORS_HPP__
#define __FIELD_TYPES_MATRIX_OPERATORS_HPP__

extern "C" {
#include "api/cmiss_field_matrix_operators.h"
}
#include "api++/field.hpp"
#include "api++/fieldmodule.hpp"

namespace Zn
{

class FieldDeterminant : public Field
{
public:

	FieldDeterminant() : Field(NULL)
	{	}

	FieldDeterminant(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldEigenvalues : public Field
{
public:

	FieldEigenvalues() : Field(NULL)
	{	}

	FieldEigenvalues(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldEigenvectors : public Field
{
public:

	FieldEigenvectors() : Field(NULL)
	{	}

	FieldEigenvectors(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldMatrixInvert : public Field
{
public:

	FieldMatrixInvert() : Field(NULL)
	{	}

	FieldMatrixInvert(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldMatrixMultiply : public Field
{
public:

	FieldMatrixMultiply() : Field(NULL)
	{	}

	FieldMatrixMultiply(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldProjection : public Field
{
public:

	FieldProjection() : Field(NULL)
	{	}


	FieldProjection(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

class FieldTranspose : public Field
{
public:

	FieldTranspose() : Field(NULL)
	{	}


	FieldTranspose(Cmiss_field_id field_id) : Field(field_id)
	{	}

};

inline FieldDeterminant FieldModule::createDeterminant(Field& sourceField)
{
	return FieldDeterminant(Cmiss_field_module_create_determinant(id,
		sourceField.getId()));
}

inline FieldEigenvalues FieldModule::createEigenvalues(Field& sourceField)
{
	return FieldEigenvalues(Cmiss_field_module_create_eigenvalues(id,
		sourceField.getId()));
}

inline FieldEigenvectors FieldModule::createEigenvectors(FieldEigenvalues& eigenValuesField)
{
	return FieldEigenvectors(Cmiss_field_module_create_eigenvectors(id,
		eigenValuesField.getId()));
}

inline FieldMatrixInvert FieldModule::createMatrixInvert(Field& sourceField)
{
	return FieldMatrixInvert(Cmiss_field_module_create_matrix_invert(id,
		sourceField.getId()));
}

inline FieldMatrixMultiply FieldModule::createMatrixMultiply(int numberOfRows,
	Field sourceField1, Field& sourceField2)
{
	return FieldMatrixMultiply(Cmiss_field_module_create_matrix_multiply(id,
		numberOfRows, sourceField1.getId(), sourceField2.getId()));
}

inline FieldProjection FieldModule::createProjection(Field& sourceField,
	Field& projectionMatrixField)
{
	return FieldProjection(Cmiss_field_module_create_projection(id,
		sourceField.getId(), projectionMatrixField.getId()));
}

inline FieldTranspose FieldModule::createTranspose(int sourceNumberOfRows,
	Field& sourceField)
{
	return FieldTranspose(Cmiss_field_module_create_transpose(id,
		sourceNumberOfRows, sourceField.getId()));
}

}  // namespace Zn

#endif /* __FIELD_TYPES_MATRIX_OPERATORS_HPP__ */
