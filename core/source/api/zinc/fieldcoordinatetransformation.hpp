/***************************************************************************//**
 * FILE : fieldcoordinatetransformation.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCOORDINATETRANSFORMATION_HPP__
#define CMZN_FIELDCOORDINATETRANSFORMATION_HPP__

#include "zinc/fieldcoordinatetransformation.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldCoordinateTransformation: public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCoordinateTransformation(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldCoordinateTransformation Fieldmodule::createFieldCoordinateTransformation(
		Field& sourceField);
public:

	FieldCoordinateTransformation() : Field(0)
	{ }

};

class FieldVectorCoordinateTransformation: public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldVectorCoordinateTransformation(cmzn_field_id field_id) : Field(field_id)
	{ }

	friend FieldVectorCoordinateTransformation Fieldmodule::createFieldVectorCoordinateTransformation(
		Field& vectorField, Field& coordinateField);

public:

	FieldVectorCoordinateTransformation() : Field(0)
	{ }

};

inline FieldCoordinateTransformation Fieldmodule::createFieldCoordinateTransformation(
	Field& sourceField)
{
	return FieldCoordinateTransformation(cmzn_fieldmodule_create_field_coordinate_transformation(
		id, sourceField.getId()));
}

inline FieldVectorCoordinateTransformation Fieldmodule::createFieldVectorCoordinateTransformation(
	Field& vectorField, Field& coordinateField)
{
	return FieldVectorCoordinateTransformation(cmzn_fieldmodule_create_field_vector_coordinate_transformation(id,
		vectorField.getId(), coordinateField.getId()));
}

}  // namespace Zinc
}

#endif
