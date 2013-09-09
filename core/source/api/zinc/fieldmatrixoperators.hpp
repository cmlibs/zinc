/***************************************************************************//**
 * FILE : fieldmatrixoperators.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMATRIXOPERATORS_HPP__
#define CMZN_FIELDMATRIXOPERATORS_HPP__

#include "zinc/fieldmatrixoperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldDeterminant : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDeterminant(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldDeterminant FieldModule::createDeterminant(Field& sourceField);

public:

	FieldDeterminant() : Field(0)
	{	}

};

class FieldEigenvalues : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEigenvalues(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEigenvalues FieldModule::createEigenvalues(Field& sourceField);

public:

	FieldEigenvalues() : Field(0)
	{	}

};

class FieldEigenvectors : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEigenvectors(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEigenvectors FieldModule::createEigenvectors(
		FieldEigenvalues& eigenValuesField);

public:

	FieldEigenvectors() : Field(0)
	{	}

};

class FieldMatrixInvert : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMatrixInvert(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldMatrixInvert FieldModule::createMatrixInvert(Field& sourceField);

public:

	FieldMatrixInvert() : Field(0)
	{	}

};

class FieldMatrixMultiply : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMatrixMultiply(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldMatrixMultiply FieldModule::createMatrixMultiply(int numberOfRows,
		Field sourceField1, Field& sourceField2);

public:

	FieldMatrixMultiply() : Field(0)
	{	}

};

class FieldProjection : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldProjection(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldProjection FieldModule::createProjection(Field& sourceField,
		Field& projectionMatrixField);

public:

	FieldProjection() : Field(0)
	{	}

};

class FieldTranspose : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldTranspose(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldTranspose FieldModule::createTranspose(int sourceNumberOfRows,
		Field& sourceField);

public:

	FieldTranspose() : Field(0)
	{	}

};

inline FieldDeterminant FieldModule::createDeterminant(Field& sourceField)
{
	return FieldDeterminant(cmzn_field_module_create_determinant(id,
		sourceField.getId()));
}

inline FieldEigenvalues FieldModule::createEigenvalues(Field& sourceField)
{
	return FieldEigenvalues(cmzn_field_module_create_eigenvalues(id,
		sourceField.getId()));
}

inline FieldEigenvectors FieldModule::createEigenvectors(FieldEigenvalues& eigenValuesField)
{
	return FieldEigenvectors(cmzn_field_module_create_eigenvectors(id,
		eigenValuesField.getId()));
}

inline FieldMatrixInvert FieldModule::createMatrixInvert(Field& sourceField)
{
	return FieldMatrixInvert(cmzn_field_module_create_matrix_invert(id,
		sourceField.getId()));
}

inline FieldMatrixMultiply FieldModule::createMatrixMultiply(int numberOfRows,
	Field sourceField1, Field& sourceField2)
{
	return FieldMatrixMultiply(cmzn_field_module_create_matrix_multiply(id,
		numberOfRows, sourceField1.getId(), sourceField2.getId()));
}

inline FieldProjection FieldModule::createProjection(Field& sourceField,
	Field& projectionMatrixField)
{
	return FieldProjection(cmzn_field_module_create_projection(id,
		sourceField.getId(), projectionMatrixField.getId()));
}

inline FieldTranspose FieldModule::createTranspose(int sourceNumberOfRows,
	Field& sourceField)
{
	return FieldTranspose(cmzn_field_module_create_transpose(id,
		sourceNumberOfRows, sourceField.getId()));
}

}  // namespace Zinc
}

#endif
