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

class Fieldcache;

class Element;

class Fieldmodule;

class Field
{
friend bool operator==(const Field& field1, const Field& field2);
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

	char *getComponentName(int componentNumber)
	{
		return cmzn_field_get_component_name(id, componentNumber);
	}

	int setComponentName(int componentNumber, const char *name)
	{
		return cmzn_field_set_component_name(id, componentNumber, name);
	}

	double getCoordinateSystemFocus()
	{
		return cmzn_field_get_coordinate_system_focus(id);
	}

	int setCoordinateSystemFocus(double focus)
	{
		 return cmzn_field_set_coordinate_system_focus(id, focus);
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

	int getNumberOfSourceFields()
	{
		return cmzn_field_get_number_of_source_fields(id);
	}

	Field getSourceField(int index)
	{
		return Field(cmzn_field_get_source_field(id, index));
	}

	bool isTypeCoordinate()
	{
		return cmzn_field_is_type_coordinate(id);
	}

	int setTypeCoordinate(bool value)
	{
		return cmzn_field_set_type_coordinate(id, value);
	}

	ValueType getValueType()
	{
		return static_cast<ValueType>(cmzn_field_get_value_type(id));
	}

	Fieldmodule getFieldmodule();

	int assignMeshLocation(Fieldcache& cache, Element element,
		int coordinatesCount, const double *coordinatesIn);

	int assignReal(Fieldcache& cache, int valuesCount, const double *valuesIn);

	int assignString(Fieldcache& cache, const char *stringValue);

	Element evaluateMeshLocation(Fieldcache& cache, int coordinatesCount,
		double *coordinatesOut);

	int evaluateReal(Fieldcache& cache, int valuesCount, double *valuesOut);

	char *evaluateString(Fieldcache& cache);

	int evaluateDerivative(Differentialoperator& differentialOperator,
		Fieldcache& cache, int valuesCount, double *valuesOut);

	bool isDefinedAtLocation(Fieldcache& cache);

};

inline bool operator==(const Field& a, const Field& b)
{
	return a.id == b.id;
}

class Fielditerator
{
private:

	cmzn_fielditerator_id id;

public:

	Fielditerator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Fielditerator(cmzn_fielditerator_id iterator_id) :
		id(iterator_id)
	{ }

	Fielditerator(const Fielditerator& fielditerator) :
		id(cmzn_fielditerator_access(fielditerator.id))
	{ }

	Fielditerator& operator=(const Fielditerator& fielditerator)
	{
		cmzn_fielditerator_id temp_id = cmzn_fielditerator_access(fielditerator.id);
		if (0 != id)
		{
			cmzn_fielditerator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Fielditerator()
	{
		if (0 != id)
		{
			cmzn_fielditerator_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Field next()
	{
		return Field(cmzn_fielditerator_next(id));
	}
};

}  // namespace Zinc
}
#endif /* CMZN_FIELD_HPP__ */
