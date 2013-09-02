/***************************************************************************//**
 * FILE : optimisation.hpp
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
#ifndef CMZN_OPTIMISATION_HPP__
#define CMZN_OPTIMISATION_HPP__

#include "zinc/optimisation.h"
#include "zinc/field.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Optimisation
{
protected:
	cmzn_optimisation_id id;

public:

	Optimisation() : id(0)
	{   }

	// takes ownership of C handle, responsibility for destroying it
	explicit Optimisation(cmzn_optimisation_id in_optimisation_id) :
		id(in_optimisation_id)
	{  }

	Optimisation(const Optimisation& optimisation) :
		id(cmzn_optimisation_access(optimisation.id))
	{ }

	Optimisation& operator=(const Optimisation& optimisation)
	{

		cmzn_optimisation_id temp_id = cmzn_optimisation_access(optimisation.id);
		if (0 != id)
		{
			cmzn_optimisation_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Optimisation()
	{
		if (0 != id)
		{
			cmzn_optimisation_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum Method
	{
		METHOD_INVALID = CMZN_OPTIMISATION_METHOD_INVALID,
		METHOD_QUASI_NEWTON = CMZN_OPTIMISATION_METHOD_QUASI_NEWTON,
		METHOD_LEAST_SQUARES_QUASI_NEWTON = CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON
	};

	/***************************************************************************//**
	 * Labels of optimisation attributes which may be set or obtained using generic
	 * get/set_attribute functions.
	 */
	enum Attribute
	{
		ATTRIBUTE_FUNCTION_TOLERANCE = CMZN_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE ,
		ATTRIBUTE_GRADIENT_TOLERANCE = CMZN_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE,
		ATTRIBUTE_STEP_TOLERANCE = CMZN_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE,
		ATTRIBUTE_MAXIMUM_ITERATIONS = CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS,
		ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS = CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS,
		ATTRIBUTE_MAXIMUM_STEP = CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP,
		ATTRIBUTE_MINIMUM_STEP = CMZN_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP,
		ATTRIBUTE_LINESEARCH_TOLERANCE = CMZN_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE,
		ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS = CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS,
		ATTRIBUTE_TRUST_REGION_SIZE = CMZN_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE
	};

	cmzn_optimisation_id getId()
	{
		return id;
	}

	Method getMethod()
	{
		return static_cast<Method>(cmzn_optimisation_get_method(id));
	}

	int setMethod(Method method)
	{
		return cmzn_optimisation_set_method(id,
			static_cast<cmzn_optimisation_method>(method));
	}

	int getAttributeInteger(Attribute attribute)
	{
		return cmzn_optimisation_get_attribute_integer(id,
			static_cast<cmzn_optimisation_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return cmzn_optimisation_set_attribute_integer(id,
			static_cast<cmzn_optimisation_attribute>(attribute), value);
	}

	double getAttributeReal(Attribute attribute)
	{
		return cmzn_optimisation_get_attribute_real(id,
					static_cast<cmzn_optimisation_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_optimisation_set_attribute_real(id,
			static_cast<cmzn_optimisation_attribute>(attribute), value);
	}

	Field getFirstIndependentField()
	{
		return Field(cmzn_optimisation_get_first_independent_field(id));
	}

	Field getNextIndependentField(Field& refField)
	{
		return Field(cmzn_optimisation_get_next_independent_field(id, refField.getId()));
	}

	int addIndependentField(Field& field)
	{
		return (cmzn_optimisation_add_independent_field(id, field.getId()));
	}

	int removeIndepdentField(Field& field)
	{
		return (cmzn_optimisation_remove_independent_field(id, field.getId()));
	}

	Field getFirstObjectiveField()
	{
		return Field(cmzn_optimisation_get_first_objective_field(id));
	}

	Field getNextObjectiveField(Field& refField)
	{
		return Field(cmzn_optimisation_get_next_objective_field(id, refField.getId()));
	}

	int addObjectiveField(Field& field)
	{
		return (cmzn_optimisation_add_objective_field(id, field.getId()));
	}

	int removeObjectiveField(Field& field)
	{
		return (cmzn_optimisation_remove_independent_field(id, field.getId()));
	}

	char *getSolutionReport()
	{
		return cmzn_optimisation_get_solution_report(id);
	}

	int optimise()
	{
		return cmzn_optimisation_optimise(id);
	}

};

}  // namespace Zinc
}

#endif
