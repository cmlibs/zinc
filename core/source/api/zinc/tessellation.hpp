/***************************************************************************//**
 * FILE : tessellation.hpp
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
#ifndef __ZN_TESSELLATION_HPP__
#define __ZN_TESSELLATION_HPP__

#include "zinc/tessellation.h"

namespace zinc
{

class Tessellation
{
protected:
	Cmiss_tessellation_id id;

public:

	Tessellation() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Tessellation(Cmiss_tessellation_id in_tessellation_id) :
		id(in_tessellation_id)
	{  }

	Tessellation(const Tessellation& tessellation) :
		id(Cmiss_tessellation_access(tessellation.id))
	{  }

	Tessellation& operator=(const Tessellation& tessellation)
	{
		Cmiss_tessellation_id temp_id = Cmiss_tessellation_access(tessellation.id);
		if (0 != id)
		{
			Cmiss_tessellation_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Tessellation()
	{
		if (0 != id)
		{
			Cmiss_tessellation_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_tessellation_id getId()
	{
		return id;
	}

	bool isManaged()
	{
		return Cmiss_tessellation_is_managed(id);
	}

	int setManaged(bool value)
	{
		return Cmiss_tessellation_set_managed(id, value);
	}

	int getCircleDivisions()
	{
		return Cmiss_tessellation_get_circle_divisions(id);
	}

	int setCircleDivisions(int circleDivisions)
	{
		return Cmiss_tessellation_set_circle_divisions(id, circleDivisions);
	}

	char *getName()
	{
		return Cmiss_tessellation_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_tessellation_set_name(id, name);
	}

	int getMinimumDivisions(int valuesCount, int *valuesOut)
	{
		return Cmiss_tessellation_get_minimum_divisions(id, valuesCount, valuesOut);
	}

	int setMinimumDivisions(int valuesCount, const int *valuesIn)
	{
		return Cmiss_tessellation_set_minimum_divisions(id, valuesCount, valuesIn);
	}

	int getRefinementFactors(int valuesCount, int *valuesOut)
	{
		return Cmiss_tessellation_get_refinement_factors(id, valuesCount, valuesOut);
	}

	int setRefinementFactors(int valuesCount, const int *valuesIn)
	{
		return Cmiss_tessellation_set_refinement_factors(id, valuesCount, valuesIn);
	}

};

class TessellationModule
{
protected:
	Cmiss_tessellation_module_id id;

public:

	TessellationModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit TessellationModule(Cmiss_tessellation_module_id in_tessellation_module_id) :
		id(in_tessellation_module_id)
	{  }

	TessellationModule(const TessellationModule& tessellationModule) :
		id(Cmiss_tessellation_module_access(tessellationModule.id))
	{  }

	TessellationModule& operator=(const TessellationModule& tessellationModule)
	{
		Cmiss_tessellation_module_id temp_id = Cmiss_tessellation_module_access(
			tessellationModule.id);
		if (0 != id)
		{
			Cmiss_tessellation_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TessellationModule()
	{
		if (0 != id)
		{
			Cmiss_tessellation_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_tessellation_module_id getId()
	{
		return id;
	}

	Tessellation createTessellation()
	{
		return Tessellation(Cmiss_tessellation_module_create_tessellation(id));
	}

	Tessellation findTessellationByName(const char *name)
	{
		return Tessellation(Cmiss_tessellation_module_find_tessellation_by_name(id, name));
	}

	int beginChange()
	{
		return Cmiss_tessellation_module_begin_change(id);
	}

	int endChange()
	{
		return Cmiss_tessellation_module_end_change(id);
	}

	Tessellation getDefaultTessellation()
	{
		return Tessellation(Cmiss_tessellation_module_get_default_tessellation(id));
	}

	int setDefaultTessellation(Tessellation &tessellation)
	{
		return Cmiss_tessellation_module_set_default_tessellation(id, tessellation.getId());
	}

	Tessellation getDefaultPointsTessellation()
	{
		return Tessellation(Cmiss_tessellation_module_get_default_points_tessellation(id));
	}

	int setDefaultPointsTessellation(Tessellation &tessellation)
	{
		return Cmiss_tessellation_module_set_default_points_tessellation(id, tessellation.getId());
	}
};

}  // namespace zinc

#endif /* __ZN_TESSELLATION_HPP__ */
