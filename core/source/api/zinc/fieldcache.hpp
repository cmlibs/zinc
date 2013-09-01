/***************************************************************************//**
 * FILE : fieldcache.hpp
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

#ifndef CMZN_FIELDCACHE_HPP__
#define CMZN_FIELDCACHE_HPP__

#include "zinc/field.h"
#include "zinc/field.hpp"
#include "zinc/element.hpp"
#include "zinc/node.hpp"

namespace zinc
{

class FieldCache
{
protected:
	Cmiss_field_cache_id id;

public:

	FieldCache() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCache(Cmiss_field_cache_id in_field_cache_id) :
		id(in_field_cache_id)
	{  }

	FieldCache(const FieldCache& fieldCache) :
		id(Cmiss_field_cache_access(fieldCache.id))
	{  }

	FieldCache& operator=(const FieldCache& fieldCache)
	{
		Cmiss_field_cache_id temp_id = Cmiss_field_cache_access(fieldCache.id);
		if (0 != id)
		{
			Cmiss_field_cache_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~FieldCache()
	{
		if (0 != id)
		{
			Cmiss_field_cache_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_field_cache_id getId()
	{
		return id;
	}

	int setElement(Element& element)
	{
		return Cmiss_field_cache_set_element(id, element.getId());
	}

	int setMeshLocation(Element& element, int coordinatesCount,
		const double *coordinatesIn)
	{
		return Cmiss_field_cache_set_mesh_location(id, element.getId(),
			coordinatesCount, coordinatesIn);
	}

	int setFieldReal(Field& referenceField, int valuesCount,
		const double *valuesIn)
	{
		return Cmiss_field_cache_set_field_real(id,
			referenceField.getId(), valuesCount, valuesIn);
	}

	int setNode(Node& node)
	{
		return Cmiss_field_cache_set_node(id, node.getId());
	}

	int setTime(double time)
	{
		return Cmiss_field_cache_set_time(id, time);
	}
};

inline int Field::assignMeshLocation(FieldCache& cache, Element element,
	int coordinatesCount, const double *coordinatesIn)
{
	return Cmiss_field_assign_mesh_location(id, cache.getId(), element.getId(),
		coordinatesCount, coordinatesIn);
}

inline int Field::assignReal(FieldCache& cache,	int valuesCount, const double *valuesIn)
{
	return Cmiss_field_assign_real(id, cache.getId(), valuesCount, valuesIn);
}

inline int Field::assignString(FieldCache& cache, const char *stringValue)
{
	return Cmiss_field_assign_string(id, cache.getId(), stringValue);
}

inline Element Field::evaluateMeshLocation(FieldCache& cache, int coordinatesCount,
	double *coordinatesOut)
{
	return Element(Cmiss_field_evaluate_mesh_location(id,
		cache.getId(), coordinatesCount, coordinatesOut));
}

inline int Field::evaluateReal(FieldCache& cache, int valuesCount, double *valuesOut)
{
	return Cmiss_field_evaluate_real(id, cache.getId(), valuesCount, valuesOut);
}

inline char *Field::evaluateString(FieldCache& cache)
{
	return Cmiss_field_evaluate_string(id, cache.getId());
}

inline int Field::evaluateDerivative(DifferentialOperator& differentialOperator,
	FieldCache& cache, int valuesCount, double *valuesOut)
{
	return Cmiss_field_evaluate_derivative(id, differentialOperator.getId(),
		cache.getId(), valuesCount, valuesOut);
}

inline bool Field::isDefinedAtLocation(FieldCache& cache)
{
	return (0 != Cmiss_field_is_defined_at_location(id, cache.getId()));
}

}  // namespace zinc

#endif /* CMZN_FIELDCACHE_HPP__ */
