/***************************************************************************//**
 * FILE : fieldcache.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDCACHE_HPP__
#define CMZN_FIELDCACHE_HPP__

#include "zinc/fieldcache.h"
#include "zinc/field.hpp"
#include "zinc/element.hpp"
#include "zinc/node.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Fieldcache
{
protected:
	cmzn_fieldcache_id id;

public:

	Fieldcache() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Fieldcache(cmzn_fieldcache_id in_field_cache_id) :
		id(in_field_cache_id)
	{  }

	Fieldcache(const Fieldcache& fieldCache) :
		id(cmzn_fieldcache_access(fieldCache.id))
	{  }

	Fieldcache& operator=(const Fieldcache& fieldCache)
	{
		cmzn_fieldcache_id temp_id = cmzn_fieldcache_access(fieldCache.id);
		if (0 != id)
		{
			cmzn_fieldcache_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Fieldcache()
	{
		if (0 != id)
		{
			cmzn_fieldcache_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_fieldcache_id getId()
	{
		return id;
	}

	int setElement(Element& element)
	{
		return cmzn_fieldcache_set_element(id, element.getId());
	}

	int setMeshLocation(Element& element, int coordinatesCount,
		const double *coordinatesIn)
	{
		return cmzn_fieldcache_set_mesh_location(id, element.getId(),
			coordinatesCount, coordinatesIn);
	}

	int setFieldReal(Field& referenceField, int valuesCount,
		const double *valuesIn)
	{
		return cmzn_fieldcache_set_field_real(id,
			referenceField.getId(), valuesCount, valuesIn);
	}

	int setNode(Node& node)
	{
		return cmzn_fieldcache_set_node(id, node.getId());
	}

	int setTime(double time)
	{
		return cmzn_fieldcache_set_time(id, time);
	}
};

inline int Field::assignMeshLocation(Fieldcache& cache, Element element,
	int coordinatesCount, const double *coordinatesIn)
{
	return cmzn_field_assign_mesh_location(id, cache.getId(), element.getId(),
		coordinatesCount, coordinatesIn);
}

inline int Field::assignReal(Fieldcache& cache,	int valuesCount, const double *valuesIn)
{
	return cmzn_field_assign_real(id, cache.getId(), valuesCount, valuesIn);
}

inline int Field::assignString(Fieldcache& cache, const char *stringValue)
{
	return cmzn_field_assign_string(id, cache.getId(), stringValue);
}

inline Element Field::evaluateMeshLocation(Fieldcache& cache, int coordinatesCount,
	double *coordinatesOut)
{
	return Element(cmzn_field_evaluate_mesh_location(id,
		cache.getId(), coordinatesCount, coordinatesOut));
}

inline int Field::evaluateReal(Fieldcache& cache, int valuesCount, double *valuesOut)
{
	return cmzn_field_evaluate_real(id, cache.getId(), valuesCount, valuesOut);
}

inline char *Field::evaluateString(Fieldcache& cache)
{
	return cmzn_field_evaluate_string(id, cache.getId());
}

inline int Field::evaluateDerivative(Differentialoperator& differentialOperator,
	Fieldcache& cache, int valuesCount, double *valuesOut)
{
	return cmzn_field_evaluate_derivative(id, differentialOperator.getId(),
		cache.getId(), valuesCount, valuesOut);
}

inline bool Field::isDefinedAtLocation(Fieldcache& cache)
{
	return (0 != cmzn_field_is_defined_at_location(id, cache.getId()));
}

}  // namespace Zinc
}

#endif /* CMZN_FIELDCACHE_HPP__ */
