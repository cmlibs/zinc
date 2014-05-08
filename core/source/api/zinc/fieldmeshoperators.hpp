/**
 * @file fieldmeshoperators.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMESHOPERATORS_HPP__
#define CMZN_FIELDMESHOPERATORS_HPP__

#include "zinc/fieldmeshoperators.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/element.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldMeshIntegral : public Field
{
private:

	inline cmzn_field_mesh_integral_id getDerivedId()
	{
		return reinterpret_cast<cmzn_field_mesh_integral_id>(id);
	}

public:

public:

	FieldMeshIntegral() : Field()
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldMeshIntegral(cmzn_field_mesh_integral_id field_mesh_integral_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_mesh_integral_id))
	{	}

	int setOrder(int order)
	{
		return cmzn_field_mesh_integral_set_order(getDerivedId(), order);
	}

};

inline FieldMeshIntegral Fieldmodule::createFieldMeshIntegral(
	const Field& integrandField, const Field& coordinateField, const Mesh& mesh)
{
	return FieldMeshIntegral(reinterpret_cast<cmzn_field_mesh_integral_id>(
		cmzn_fieldmodule_create_field_mesh_integral(id, integrandField.getId(),
		coordinateField.getId(), mesh.getId())));
}

inline FieldMeshIntegral Field::castMeshIntegral()
{
	return FieldMeshIntegral(cmzn_field_cast_mesh_integral(id));
}

}  // namespace Zinc
}

#endif
