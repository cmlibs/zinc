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

#include "zinc/field.h"
#include "zinc/field.hpp"
#include "zinc/element.hpp"
#include "zinc/node.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldCache
{
protected:
	cmzn_field_cache_id id;

public:

	FieldCache() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCache(cmzn_field_cache_id in_field_cache_id) :
		id(in_field_cache_id)
	{  }

	FieldCache(const FieldCache& fieldCache) :
		id(cmzn_field_cache_access(fieldCache.id))
	{  }

	FieldCache& operator=(const FieldCache& fieldCache)
	{
		cmzn_field_cache_id temp_id = cmzn_field_cache_access(fieldCache.id);
		if (0 != id)
		{
			cmzn_field_cache_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~FieldCache()
	{
		if (0 != id)
		{
			cmzn_field_cache_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_field_cache_id getId()
	{
		return id;
	}

	int setElement(Element& element)
	{
		return cmzn_field_cache_set_element(id, element.getId());
	}

	int setMeshLocation(Element& element, int coordinatesCount,
		const double *coordinatesIn)
	{
		return cmzn_field_cache_set_mesh_location(id, element.getId(),
			coordinatesCount, coordinatesIn);
	}

	int setFieldReal(Field& referenceField, int valuesCount,
		const double *valuesIn)
	{
		return cmzn_field_cache_set_field_real(id,
			referenceField.getId(), valuesCount, valuesIn);
	}

	int setNode(Node& node)
	{
		return cmzn_field_cache_set_node(id, node.getId());
	}

	int setTime(double time)
	{
		return cmzn_field_cache_set_time(id, time);
	}
};

inline int Field::assignMeshLocation(FieldCache& cache, Element element,
	int coordinatesCount, const double *coordinatesIn)
{
	return cmzn_field_assign_mesh_location(id, cache.getId(), element.getId(),
		coordinatesCount, coordinatesIn);
}

inline int Field::assignReal(FieldCache& cache,	int valuesCount, const double *valuesIn)
{
	return cmzn_field_assign_real(id, cache.getId(), valuesCount, valuesIn);
}

inline int Field::assignString(FieldCache& cache, const char *stringValue)
{
	return cmzn_field_assign_string(id, cache.getId(), stringValue);
}

inline Element Field::evaluateMeshLocation(FieldCache& cache, int coordinatesCount,
	double *coordinatesOut)
{
	return Element(cmzn_field_evaluate_mesh_location(id,
		cache.getId(), coordinatesCount, coordinatesOut));
}

inline int Field::evaluateReal(FieldCache& cache, int valuesCount, double *valuesOut)
{
	return cmzn_field_evaluate_real(id, cache.getId(), valuesCount, valuesOut);
}

inline char *Field::evaluateString(FieldCache& cache)
{
	return cmzn_field_evaluate_string(id, cache.getId());
}

inline int Field::evaluateDerivative(DifferentialOperator& differentialOperator,
	FieldCache& cache, int valuesCount, double *valuesOut)
{
	return cmzn_field_evaluate_derivative(id, differentialOperator.getId(),
		cache.getId(), valuesCount, valuesOut);
}

inline bool Field::isDefinedAtLocation(FieldCache& cache)
{
	return (0 != cmzn_field_is_defined_at_location(id, cache.getId()));
}

}  // namespace Zinc
}

#endif /* CMZN_FIELDCACHE_HPP__ */
