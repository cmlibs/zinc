/**
 * @file fieldmatrixoperators.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMATRIXOPERATORS_HPP__
#define CMZN_FIELDMATRIXOPERATORS_HPP__

#include "cmlibs/zinc/fieldmatrixoperators.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldDeterminant : public Field
{
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDeterminant(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldDeterminant Fieldmodule::createFieldDeterminant(const Field& sourceField);

public:

	FieldDeterminant() : Field(0)
	{	}
};

class FieldEigenvalues : public Field
{
	inline cmzn_field_eigenvalues_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_eigenvalues_id>(this->id);
	}

public:

	explicit FieldEigenvalues(cmzn_field_eigenvalues_id field_eigenvalues_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_eigenvalues_id))
	{	}

	FieldEigenvalues() : Field(0)
	{	}
};

class FieldEigenvectors : public Field
{
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEigenvectors(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEigenvectors Fieldmodule::createFieldEigenvectors(
		const Field& eigenvaluesField);

public:

	FieldEigenvectors() : Field(0)
	{	}
};

class FieldMatrixInvert : public Field
{
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMatrixInvert(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldMatrixInvert Fieldmodule::createFieldMatrixInvert(const Field& sourceField);

public:

	FieldMatrixInvert() : Field(0)
	{	}
};

class FieldMatrixMultiply : public Field
{
	inline cmzn_field_matrix_multiply_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_matrix_multiply_id>(this->id);
	}

public:

	FieldMatrixMultiply() : Field()
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMatrixMultiply(cmzn_field_matrix_multiply_id field_matrix_multiply_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_matrix_multiply_id))
	{	}

	int getNumberOfRows() const
	{
		return cmzn_field_matrix_multiply_get_number_of_rows(this->getDerivedId());
	}
};

class FieldProjection : public Field
{
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldProjection(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldProjection Fieldmodule::createFieldProjection(const Field& sourceField,
		const Field& projectionMatrixField);

public:

	FieldProjection() : Field(0)
	{	}
};

class FieldTranspose : public Field
{
	inline cmzn_field_transpose_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_transpose_id>(this->id);
	}

public:

	FieldTranspose() : Field()
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldTranspose(cmzn_field_transpose_id field_transpose_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_transpose_id))
	{	}

	int getSourceNumberOfRows() const
	{
		return cmzn_field_transpose_get_source_number_of_rows(this->getDerivedId());
	}

	int setSourceNumberOfRows(int sourceNumberOfRows)
	{
		return cmzn_field_transpose_set_source_number_of_rows(
			this->getDerivedId(), sourceNumberOfRows);
	}
};


inline FieldDeterminant Fieldmodule::createFieldDeterminant(const Field& sourceField)
{
	return FieldDeterminant(cmzn_fieldmodule_create_field_determinant(this->id,
		sourceField.getId()));
}

inline FieldEigenvalues Field::castEigenvalues()
{
	return FieldEigenvalues(cmzn_field_cast_eigenvalues(this->id));
}

inline FieldEigenvalues Fieldmodule::createFieldEigenvalues(const Field& sourceField)
{
	return FieldEigenvalues(reinterpret_cast<cmzn_field_eigenvalues_id>(
		cmzn_fieldmodule_create_field_eigenvalues(this->id, sourceField.getId())));
}

inline FieldEigenvectors Fieldmodule::createFieldEigenvectors(const Field& eigenvaluesField)
{
	return FieldEigenvectors(cmzn_fieldmodule_create_field_eigenvectors(this->id,
		eigenvaluesField.getId()));
}

inline FieldMatrixInvert Fieldmodule::createFieldMatrixInvert(const Field& sourceField)
{
	return FieldMatrixInvert(cmzn_fieldmodule_create_field_matrix_invert(this->id,
		sourceField.getId()));
}

inline FieldMatrixMultiply Field::castMatrixMultiply()
{
	return FieldMatrixMultiply(cmzn_field_cast_matrix_multiply(this->id));
}

inline FieldMatrixMultiply Fieldmodule::createFieldMatrixMultiply(int numberOfRows,
	const Field& sourceField1, const Field& sourceField2)
{
	return FieldMatrixMultiply(reinterpret_cast<cmzn_field_matrix_multiply_id>(
		cmzn_fieldmodule_create_field_matrix_multiply(this->id,
			numberOfRows, sourceField1.getId(), sourceField2.getId())));
}

inline FieldProjection Fieldmodule::createFieldProjection(const Field& sourceField,
	const Field& projectionMatrixField)
{
	return FieldProjection(cmzn_fieldmodule_create_field_projection(this->id,
		sourceField.getId(), projectionMatrixField.getId()));
}

inline FieldTranspose Field::castTranspose()
{
	return FieldTranspose(cmzn_field_cast_transpose(this->id));
}

inline FieldTranspose Fieldmodule::createFieldTranspose(int sourceNumberOfRows,
	const Field& sourceField)
{
	return FieldTranspose(reinterpret_cast<cmzn_field_transpose_id>(
		cmzn_fieldmodule_create_field_transpose(this->id,
			sourceNumberOfRows, sourceField.getId())));
}

}  // namespace Zinc
}

#endif
