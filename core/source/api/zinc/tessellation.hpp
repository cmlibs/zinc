/**
 * @file tessellation.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TESSELLATION_HPP__
#define CMZN_TESSELLATION_HPP__

#include "zinc/tessellation.h"
#include "zinc/context.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Tessellation
{
protected:
	cmzn_tessellation_id id;

public:

	Tessellation() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Tessellation(cmzn_tessellation_id in_tessellation_id) :
		id(in_tessellation_id)
	{  }

	Tessellation(const Tessellation& tessellation) :
		id(cmzn_tessellation_access(tessellation.id))
	{  }

	Tessellation& operator=(const Tessellation& tessellation)
	{
		cmzn_tessellation_id temp_id = cmzn_tessellation_access(tessellation.id);
		if (0 != id)
		{
			cmzn_tessellation_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Tessellation()
	{
		if (0 != id)
		{
			cmzn_tessellation_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_tessellation_id getId() const
	{
		return id;
	}

	bool isManaged()
	{
		return cmzn_tessellation_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_tessellation_set_managed(id, value);
	}

	int getCircleDivisions()
	{
		return cmzn_tessellation_get_circle_divisions(id);
	}

	int setCircleDivisions(int circleDivisions)
	{
		return cmzn_tessellation_set_circle_divisions(id, circleDivisions);
	}

	char *getName()
	{
		return cmzn_tessellation_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_tessellation_set_name(id, name);
	}

	int getMinimumDivisions(int valuesCount, int *valuesOut)
	{
		return cmzn_tessellation_get_minimum_divisions(id, valuesCount, valuesOut);
	}

	int setMinimumDivisions(int valuesCount, const int *valuesIn)
	{
		return cmzn_tessellation_set_minimum_divisions(id, valuesCount, valuesIn);
	}

	int getRefinementFactors(int valuesCount, int *valuesOut)
	{
		return cmzn_tessellation_get_refinement_factors(id, valuesCount, valuesOut);
	}

	int setRefinementFactors(int valuesCount, const int *valuesIn)
	{
		return cmzn_tessellation_set_refinement_factors(id, valuesCount, valuesIn);
	}

};

class Tessellationmodule
{
protected:
	cmzn_tessellationmodule_id id;

public:

	Tessellationmodule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Tessellationmodule(cmzn_tessellationmodule_id in_tessellationmodule_id) :
		id(in_tessellationmodule_id)
	{  }

	Tessellationmodule(const Tessellationmodule& tessellationModule) :
		id(cmzn_tessellationmodule_access(tessellationModule.id))
	{  }

	Tessellationmodule& operator=(const Tessellationmodule& tessellationModule)
	{
		cmzn_tessellationmodule_id temp_id = cmzn_tessellationmodule_access(
			tessellationModule.id);
		if (0 != id)
		{
			cmzn_tessellationmodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Tessellationmodule()
	{
		if (0 != id)
		{
			cmzn_tessellationmodule_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_tessellationmodule_id getId() const
	{
		return id;
	}

	Tessellation createTessellation()
	{
		return Tessellation(cmzn_tessellationmodule_create_tessellation(id));
	}

	Tessellation findTessellationByName(const char *name)
	{
		return Tessellation(cmzn_tessellationmodule_find_tessellation_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_tessellationmodule_begin_change(id);
	}

	int endChange()
	{
		return cmzn_tessellationmodule_end_change(id);
	}

	Tessellation getDefaultTessellation()
	{
		return Tessellation(cmzn_tessellationmodule_get_default_tessellation(id));
	}

	int setDefaultTessellation(const Tessellation& tessellation)
	{
		return cmzn_tessellationmodule_set_default_tessellation(id, tessellation.getId());
	}

	Tessellation getDefaultPointsTessellation()
	{
		return Tessellation(cmzn_tessellationmodule_get_default_points_tessellation(id));
	}

	int setDefaultPointsTessellation(const Tessellation& tessellation)
	{
		return cmzn_tessellationmodule_set_default_points_tessellation(id, tessellation.getId());
	}
};

inline Tessellationmodule Context::getTessellationmodule()
{
	return Tessellationmodule(cmzn_context_get_tessellationmodule(id));
}

}  // namespace Zinc
}

#endif
