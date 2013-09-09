/***************************************************************************//**
 * FILE : tessellation.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TESSELLATION_HPP__
#define CMZN_TESSELLATION_HPP__

#include "zinc/tessellation.h"

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

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_tessellation_id getId()
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

class TessellationModule
{
protected:
	cmzn_tessellation_module_id id;

public:

	TessellationModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit TessellationModule(cmzn_tessellation_module_id in_tessellation_module_id) :
		id(in_tessellation_module_id)
	{  }

	TessellationModule(const TessellationModule& tessellationModule) :
		id(cmzn_tessellation_module_access(tessellationModule.id))
	{  }

	TessellationModule& operator=(const TessellationModule& tessellationModule)
	{
		cmzn_tessellation_module_id temp_id = cmzn_tessellation_module_access(
			tessellationModule.id);
		if (0 != id)
		{
			cmzn_tessellation_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TessellationModule()
	{
		if (0 != id)
		{
			cmzn_tessellation_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_tessellation_module_id getId()
	{
		return id;
	}

	Tessellation createTessellation()
	{
		return Tessellation(cmzn_tessellation_module_create_tessellation(id));
	}

	Tessellation findTessellationByName(const char *name)
	{
		return Tessellation(cmzn_tessellation_module_find_tessellation_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_tessellation_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_tessellation_module_end_change(id);
	}

	Tessellation getDefaultTessellation()
	{
		return Tessellation(cmzn_tessellation_module_get_default_tessellation(id));
	}

	int setDefaultTessellation(Tessellation &tessellation)
	{
		return cmzn_tessellation_module_set_default_tessellation(id, tessellation.getId());
	}

	Tessellation getDefaultPointsTessellation()
	{
		return Tessellation(cmzn_tessellation_module_get_default_points_tessellation(id));
	}

	int setDefaultPointsTessellation(Tessellation &tessellation)
	{
		return cmzn_tessellation_module_set_default_points_tessellation(id, tessellation.getId());
	}
};

}  // namespace Zinc
}

#endif
