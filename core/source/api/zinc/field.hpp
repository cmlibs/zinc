/***************************************************************************//**
 * FILE : field.hpp
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELD_HPP__
#define CMZN_FIELD_HPP__

#include "zinc/field.h"
#include "zinc/differentialoperator.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldCache;

class Element;

class FieldModule;

class Field
{
protected:

	cmzn_field_id id;

public:

	Field() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Field(cmzn_field_id field_id) : id(field_id)
	{ }

	Field(const Field& field) : id(cmzn_field_access(field.id))
	{ }

	Field& operator=(const Field& field)
	{
		cmzn_field_id temp_id = cmzn_field_access(field.id);
		if (0 != id)
		{
			cmzn_field_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Field()
	{
		if (0 != id)
		{
			cmzn_field_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_field_id getId()
	{
		return id;
	}

	/** @see cmzn_field_attribute */
	enum Attribute
	{
		ATTRIBUTE_INVALID = CMZN_FIELD_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMZN_FIELD_ATTRIBUTE_IS_MANAGED,
		ATTRIBUTE_IS_COORDINATE = CMZN_FIELD_ATTRIBUTE_IS_COORDINATE,
		ATTRIBUTE_NUMBER_OF_COMPONENTS = CMZN_FIELD_ATTRIBUTE_NUMBER_OF_COMPONENTS,
		ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS = CMZN_FIELD_ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS,
		ATTRIBUTE_COORDINATE_SYSTEM_FOCUS = CMZN_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS
	};

	enum CoordinateSystemType
	{
		COORDINATE_SYSTEM_TYPE_INVALID = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID,
		COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN ,
		COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR,
		COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR,
		COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL,
			/*!< uses ATTRIBUTE_COORDINATE_SYSTEM_FOCUS */
		COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL,
			/*!< uses ATTRIBUTE_COORDINATE_SYSTEM_FOCUS */
		COORDINATE_SYSTEM_TYPE_FIBRE = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE
			/*!< For Euler angles specifying fibre axes orientation from default
			 * aligned with element xi coordinates. */
	};

	enum DomainType
	{
		DOMAIN_TYPE_INVALID = CMZN_FIELD_DOMAIN_TYPE_INVALID,
		DOMAIN_POINT = CMZN_FIELD_DOMAIN_POINT,
		DOMAIN_NODES = CMZN_FIELD_DOMAIN_NODES,
		DOMAIN_DATA = CMZN_FIELD_DOMAIN_DATA,
		DOMAIN_MESH_1D = CMZN_FIELD_DOMAIN_MESH_1D,
		DOMAIN_MESH_2D = CMZN_FIELD_DOMAIN_MESH_2D,
		DOMAIN_MESH_3D = CMZN_FIELD_DOMAIN_MESH_3D,
		DOMAIN_MESH_HIGHEST_DIMENSION = CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION
	};

	enum ValueType
	{
		VALUE_TYPE_INVALID = CMZN_FIELD_VALUE_TYPE_INVALID,
		VALUE_TYPE_REAL = CMZN_FIELD_VALUE_TYPE_REAL,
		VALUE_TYPE_STRING = CMZN_FIELD_VALUE_TYPE_STRING,
		VALUE_TYPE_MESH_LOCATION = CMZN_FIELD_VALUE_TYPE_MESH_LOCATION
	};

	bool isManaged()
	{
		return cmzn_field_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_field_set_managed(id, value);
	}

	int getAttributeInteger(Attribute attribute)
	{
		return cmzn_field_get_attribute_integer(id, static_cast<cmzn_field_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return cmzn_field_set_attribute_integer(id, static_cast<cmzn_field_attribute>(attribute), value);
	}

	double getAttributeReal(Attribute attribute)
	{
		return cmzn_field_get_attribute_real(id,
			static_cast<cmzn_field_attribute>(attribute));
	}

	double setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_field_set_attribute_real(id,
			static_cast<cmzn_field_attribute>(attribute), value);
	}

	CoordinateSystemType getCoordinateSystemType()
	{
		return static_cast<CoordinateSystemType>(
			cmzn_field_get_coordinate_system_type(id));
	}

	int setCoordinateSystemType(CoordinateSystemType coordinateSystemType)
	{
		 return cmzn_field_set_coordinate_system_type(id,
			 static_cast<cmzn_field_coordinate_system_type>(coordinateSystemType));
	}

	int getNumberOfComponents()
	{
		return cmzn_field_get_number_of_components(id);
	}

	char *getName()
	{
		return cmzn_field_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_field_set_name(id, name);
	}

	Field getSourceField(int index)
	{
		return Field(cmzn_field_get_source_field(id, index));
	}

	ValueType getValueType()
	{
		return static_cast<ValueType>(cmzn_field_get_value_type(id));
	}

	FieldModule getFieldModule();

	int assignMeshLocation(FieldCache& cache, Element element,
		int coordinatesCount, const double *coordinatesIn);

	int assignReal(FieldCache& cache, int valuesCount, const double *valuesIn);

	int assignString(FieldCache& cache, const char *stringValue);

	Element evaluateMeshLocation(FieldCache& cache, int coordinatesCount,
		double *coordinatesOut);

	int evaluateReal(FieldCache& cache, int valuesCount, double *valuesOut);

	char *evaluateString(FieldCache& cache);

	int evaluateDerivative(DifferentialOperator& differentialOperator,
		FieldCache& cache, int valuesCount, double *valuesOut);

	bool isDefinedAtLocation(FieldCache& cache);

};

}  // namespace Zinc
}
#endif /* CMZN_FIELD_HPP__ */
