/**
 * @file fieldrange.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDRANGE_HPP__
#define CMZN_FIELDRANGE_HPP__

#include "cmlibs/zinc/fieldrange.h"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldcache.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Fieldrange
{
protected:
	cmzn_fieldrange_id id;

public:

	Fieldrange() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Fieldrange(cmzn_fieldrange_id in_fieldrange_id) :
		id(in_fieldrange_id)
	{  }

	Fieldrange(const Fieldrange& range) :
		id(cmzn_fieldrange_access(range.id))
	{  }

	Fieldrange& operator=(const Fieldrange& range)
	{
		cmzn_fieldrange_id temp_id = cmzn_fieldrange_access(range.id);
		if (0 != id)
		{
			cmzn_fieldrange_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Fieldrange()
	{
		if (0 != id)
		{
			cmzn_fieldrange_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_fieldrange_id getId() const
	{
		return id;
	}

	Element getComponentMinimumMeshLocation(int componentNumber,
		int coordinatesCount, double *coordinatesOut)
	{
		return Element(cmzn_fieldrange_get_component_minimum_mesh_location(this->id,
			componentNumber, coordinatesCount, coordinatesOut));
	}

	Element getComponentMaximumMeshLocation(int componentNumber,
		int coordinatesCount, double *coordinatesOut)
	{
		return Element(cmzn_fieldrange_get_component_maximum_mesh_location(this->id,
			componentNumber, coordinatesCount, coordinatesOut));
	}

	int getComponentMinimumValuesReal(int componentNumber,
		int coordinatesCount, double *coordinatesOut)
	{
		return cmzn_fieldrange_get_component_minimum_values_real(this->id,
			componentNumber, coordinatesCount, coordinatesOut);
	}

	int getComponentMaximumValuesReal(int componentNumber,
		int coordinatesCount, double *coordinatesOut)
	{
		return cmzn_fieldrange_get_component_maximum_values_real(this->id,
			componentNumber, coordinatesCount, coordinatesOut);
	}

	Field getField() const
	{
		return Field(cmzn_fieldrange_get_field(this->id));
	}

	int getRangeReal(int valuesCount, double *minimumValuesOut,
		double *maximumValuesOut) const
	{
		return cmzn_fieldrange_get_range_real(this->id,
			valuesCount, minimumValuesOut, maximumValuesOut);
	}

	bool hasValidRange() const
	{
		return cmzn_fieldrange_has_valid_range(this->id);
	}

};

inline Fieldrange Fieldcache::createFieldrange()
{
	return Fieldrange(cmzn_fieldcache_create_fieldrange(this->id));
}

inline int Field::evaluateFieldrange(const Fieldcache& cache, Fieldrange& range) const
{
	return cmzn_field_evaluate_fieldrange(this->id, cache.getId(), range.getId());
}

}  // namespace Zinc
}

#endif /* CMZN_FIELDRANGE_HPP__ */
