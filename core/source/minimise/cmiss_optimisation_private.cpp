/**
 * @file cmiss_optimisation_private.cpp
 *
 * The cmzn_optimisation object.
 *
 * @see-also api/zinc/optimisation.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdio>
	#include "zinc/field.h"
	#include "zinc/region.h"
	#include "computed_field/computed_field.h"
	#include "computed_field/computed_field_composite.h"
	#include "computed_field/computed_field_finite_element.h"
	#include "general/mystring.h"
	#include "general/debug.h"
#include "general/enumerator_conversion.hpp"
#include "minimise/cmiss_optimisation_private.hpp"
#include "minimise/optimisation.hpp"

char *cmzn_optimisation::getSolutionReport()
{
	std::string temp = solution_report.str();
	return duplicate_string(temp.c_str());
}

int cmzn_optimisation::runOptimisation()
{
	int return_code = 1;
	solution_report.str("");
	// check required attributes are set
	if (method == CMZN_OPTIMISATION_METHOD_INVALID)
	{
		display_message(ERROR_MESSAGE, "cmzn_optimisation_optimise.  Optimisation method invalid or not set.");
		return_code = 0;
	}
	if (independentFields.size() < 1)
	{
		display_message(ERROR_MESSAGE, "cmzn_optimisation_optimise.  Must set at least one independent field.");
		return_code = 0;
	}
	if (objectiveFields.size() < 1)
	{
		display_message(ERROR_MESSAGE, "cmzn_optimisation_optimise.  Must set at least one objective field.");
		return_code = 0;
	}
	if (return_code)
	{
		Minimisation minimisation(*this);
		return_code = return_code && minimisation.prepareOptimisation();
		return_code = return_code && minimisation.runOptimisation();
	}
	return return_code;
}

cmzn_optimisation_id cmzn_fieldmodule_create_optimisation(cmzn_fieldmodule_id field_module)
{
	if (field_module)
		return new cmzn_optimisation(field_module);
	return 0;
}

cmzn_optimisation_id cmzn_optimisation_access(cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->access();

	return 0;
}

int cmzn_optimisation_destroy(cmzn_optimisation_id *optimisation_address)
{
	if (optimisation_address)
		return cmzn_optimisation::deaccess(*optimisation_address);
	return 0;
}

enum cmzn_optimisation_method cmzn_optimisation_get_method(cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->method;
	return CMZN_OPTIMISATION_METHOD_INVALID;
}

int cmzn_optimisation_set_method(cmzn_optimisation_id optimisation,
		enum cmzn_optimisation_method method)
{
	if (optimisation && (
		(method == CMZN_OPTIMISATION_METHOD_QUASI_NEWTON) ||
		(method == CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON)))
	{
		optimisation->method = method;
		return 1;
	}
	return 0;
}

class cmzn_optimisation_method_conversion
{
public:
	static const char *to_string(enum cmzn_optimisation_method method)
	{
		const char *enum_string = 0;
		switch (method)
		{
			case CMZN_OPTIMISATION_METHOD_QUASI_NEWTON:
				enum_string = "QUASI_NEWTON";
				break;
			case CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON:
				enum_string = "LEAST_SQUARES_QUASI_NEWTON";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_optimisation_method
	cmzn_optimisation_method_enum_from_string(const char *string)
{
	return string_to_enum<enum cmzn_optimisation_method,
	cmzn_optimisation_method_conversion>(string);
}

char *cmzn_optimisation_method_enum_to_string(
	enum cmzn_optimisation_method method)
{
	const char *method_string = cmzn_optimisation_method_conversion::to_string(method);
	return (method_string ? duplicate_string(method_string) : 0);
}

int cmzn_optimisation_get_attribute_integer(cmzn_optimisation_id optimisation,
		enum cmzn_optimisation_attribute attribute)
{
	if (optimisation)
	{
		switch (attribute)
		{
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
			return optimisation->maximumIterations;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
			return optimisation->maximumNumberFunctionEvaluations;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
			return optimisation->maximumBacktrackIterations;
			break;
		default:
			break;
		}
	}
	return 0;
}

int cmzn_optimisation_set_attribute_integer(cmzn_optimisation_id optimisation,
		enum cmzn_optimisation_attribute attribute, int value)
{
	int return_code = 1;
	if (optimisation)
	{
		switch (attribute)
		{
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
			optimisation->maximumIterations = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
			optimisation->maximumNumberFunctionEvaluations = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
			optimisation->maximumBacktrackIterations = value;
			break;
		default:
			return_code = 0;
			break;
		}
	}
	return return_code;
}

double cmzn_optimisation_get_attribute_real(cmzn_optimisation_id optimisation,
		enum cmzn_optimisation_attribute attribute)
{
	if (optimisation)
	{
		switch (attribute)
		{
		case CMZN_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
			return optimisation->functionTolerance;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
			return optimisation->gradientTolerance;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
			return optimisation->stepTolerance;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
			return optimisation->maximumStep;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
			return optimisation->minimumStep;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
			return optimisation->linesearchTolerance;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
			return optimisation->trustRegionSize;
			break;
		default:
			break;
		}
	}
	return 0.0;
}

int cmzn_optimisation_set_attribute_real(cmzn_optimisation_id optimisation,
		enum cmzn_optimisation_attribute attribute, double value)
{
	int return_code = 1;
	if (optimisation)
	{
		switch (attribute)
		{
		case CMZN_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
			optimisation->functionTolerance = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
			optimisation->gradientTolerance = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
			optimisation->stepTolerance = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
			optimisation->maximumStep = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
			optimisation->minimumStep = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
			optimisation->linesearchTolerance = value;
			break;
		case CMZN_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
			optimisation->trustRegionSize = value;
			break;
		default:
			return_code = 0;
			break;
		}
	}
	return return_code;
}

class cmzn_optimisation_attribute_conversion
{
public:
	static const char *to_string(enum cmzn_optimisation_attribute attribute)
	{
		const char *enum_string = 0;
		switch (attribute)
		{
			case CMZN_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
				enum_string = "FUNCTION_TOLERANCE";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
				enum_string = "GRADIENT_TOLERANCE";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
				enum_string = "STEP_TOLERANCE";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
				enum_string = "MAXIMUM_ITERATIONS";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
				enum_string = "MAXIMUM_FUNCTION_EVALUATIONS";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
				enum_string = "MAXIMUM_STEP";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
				enum_string = "MINIMUM_STEP";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
				enum_string = "LINESEARCH_TOLERANCE";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
				enum_string = "MAXIMUM_BACKTRACK_ITERATIONS";
				break;
			case CMZN_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
				enum_string = "TRUST_REGION_SIZE";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_optimisation_attribute cmzn_optimisation_attribute_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_optimisation_attribute,
	cmzn_optimisation_attribute_conversion>(string);
}

char *cmzn_optimisation_attribute_enum_to_string(
	enum cmzn_optimisation_attribute attribute)
{
	const char *attribute_string = cmzn_optimisation_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}

cmzn_field_id cmzn_optimisation_get_first_independent_field(
	cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->getFirstIndependentField();
	return 0;
}

cmzn_field_id cmzn_optimisation_get_next_independent_field(
	cmzn_optimisation_id optimisation, cmzn_field_id ref_field)
{
	if (optimisation && ref_field)
		return optimisation->getNextIndependentField(ref_field);
	return 0;
}

int cmzn_optimisation_add_independent_field(cmzn_optimisation_id optimisation,
	cmzn_field_id field)
{
	if (optimisation && field)
		return optimisation->addIndependentField(field);
	return 0;
}

int cmzn_optimisation_remove_independent_field(
	cmzn_optimisation_id optimisation, cmzn_field_id field)
{
	if (optimisation && field)
		return optimisation->removeIndependentField(field);
	return 0;
}

cmzn_field_id cmzn_optimisation_get_first_objective_field(
	cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->getFirstObjectiveField();
	return 0;
}

cmzn_field_id cmzn_optimisation_get_next_objective_field(
	cmzn_optimisation_id optimisation, cmzn_field_id ref_field)
{
	if (optimisation && ref_field)
		return optimisation->getNextObjectiveField(ref_field);
	return 0;
}

int cmzn_optimisation_add_objective_field(cmzn_optimisation_id optimisation,
	cmzn_field_id field)
{
	if (optimisation && field)
		return optimisation->addObjectiveField(field);
	return 0;
}

int cmzn_optimisation_remove_objective_field(
	cmzn_optimisation_id optimisation, cmzn_field_id field)
{
	if (optimisation && field)
		return optimisation->removeObjectiveField(field);
	return 0;
}

char *cmzn_optimisation_get_solution_report(cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->getSolutionReport();
	return 0;
}

int cmzn_optimisation_optimise(cmzn_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->runOptimisation();
	return 0;
}
