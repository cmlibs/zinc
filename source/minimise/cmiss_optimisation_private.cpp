/**
 * @file cmiss_optimisation_private.cpp
 *
 * The Cmiss_optimisation object.
 *
 * @see-also api/cmiss_optimisation.h
 *
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#include <cstdio>
extern "C"
{
	#include "api/cmiss_field.h"
	#include "api/cmiss_region.h"
	#include "computed_field/computed_field.h"
	#include "computed_field/computed_field_composite.h"
	#include "computed_field/computed_field_finite_element.h"
	#include "general/mystring.h"
}
#include "general/enumerator_conversion.hpp"
#include "minimise/cmiss_optimisation_private.hpp"
#include "minimise/optimisation.hpp"

int Cmiss_optimisation::runOptimisation()
{
	int return_code = 1;
	// check required attributes are set
	if (method == CMISS_OPTIMISATION_METHOD_INVALID)
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_optimise.  Optimisation method invalid or not set.");
		return_code = 0;
	}
	if (independentFields.size() < 1)
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_optimise.  Must set at least one independent field.");
		return_code = 0;
	}
	if (objectiveFields.size() < 1)
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_optimise.  Must set at least one objective field.");
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

Cmiss_optimisation_id Cmiss_field_module_create_optimisation(Cmiss_field_module_id field_module)
{
	if (field_module)
		return new Cmiss_optimisation(field_module);
	return 0;
}

int Cmiss_optimisation_destroy(Cmiss_optimisation_id *optimisation_address)
{
	if (optimisation_address)
		return Cmiss_optimisation::deaccess(*optimisation_address);
	return 0;
}

enum Cmiss_optimisation_method Cmiss_optimisation_get_method(Cmiss_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->method;
	return CMISS_OPTIMISATION_METHOD_INVALID;
}

int Cmiss_optimisation_set_method(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_method method)
{
	if (optimisation && (
		(method == CMISS_OPTIMISATION_METHOD_QUASI_NEWTON) ||
		(method == CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON)))
	{
		optimisation->method = method;
		return 1;
	}
	return 0;
}

class Cmiss_optimisation_method_conversion
{
public:
    static const char *to_string(enum Cmiss_optimisation_method method)
    {
    	const char *enum_string = 0;
    	switch (method)
    	{
    		case CMISS_OPTIMISATION_METHOD_QUASI_NEWTON:
    			enum_string = "QUASI_NEWTON";
    			break;
    		case CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON:
    			enum_string = "LEAST_SQUARES_QUASI_NEWTON";
    			break;
    		default:
    			break;
    	}
    	return enum_string;
    }
};

enum Cmiss_optimisation_method
	Cmiss_optimisation_method_enum_from_string(const char *string)
{
	return string_to_enum<enum Cmiss_optimisation_method,
	Cmiss_optimisation_method_conversion>(string);
}

char *Cmiss_optimisation_method_enum_to_string(
	enum Cmiss_optimisation_method method)
{
	const char *method_string = Cmiss_optimisation_method_conversion::to_string(method);
	return (method_string ? duplicate_string(method_string) : 0);
}

int Cmiss_optimisation_get_attribute_integer(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute)
{
	if (optimisation)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
			return optimisation->maximumIterations;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
			return optimisation->maximumNumberFunctionEvaluations;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
			return optimisation->maximumBacktrackIterations;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_DISPLAY_OUTPUT:
			return optimisation->displayOutput;
			break;
		default:
			break;
		}
	}
	return 0;
}

int Cmiss_optimisation_set_attribute_integer(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, int value)
{
	int return_code = 1;
	if (optimisation)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
			optimisation->maximumIterations = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
			optimisation->maximumNumberFunctionEvaluations = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
			optimisation->maximumBacktrackIterations = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_DISPLAY_OUTPUT:
			optimisation->displayOutput = value;
			break;
		default:
			return_code = 0;
			break;
		}
	}
	return return_code;
}

double Cmiss_optimisation_get_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute)
{
	if (optimisation)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
			return optimisation->functionTolerance;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
			return optimisation->gradientTolerance;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
			return optimisation->stepTolerance;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
			return optimisation->maximumStep;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
			return optimisation->minimumStep;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
			return optimisation->linesearchTolerance;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
			return optimisation->trustRegionSize;
			break;
		default:
			break;
		}
	}
	return 0.0;
}

int Cmiss_optimisation_set_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, double value)
{
	int return_code = 1;
	if (optimisation)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
			optimisation->functionTolerance = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
			optimisation->gradientTolerance = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
			optimisation->stepTolerance = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
			optimisation->maximumStep = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
			optimisation->minimumStep = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
			optimisation->linesearchTolerance = value;
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
			optimisation->trustRegionSize = value;
			break;
		default:
			return_code = 0;
			break;
		}
	}
	return return_code;
}

class Cmiss_optimisation_attribute_conversion
{
public:
    static const char *to_string(enum Cmiss_optimisation_attribute attribute)
    {
        const char *enum_string = 0;
        switch (attribute)
        {
        	case CMISS_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE:
        		enum_string = "FUNCTION_TOLERANCE";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE:
        		enum_string = "GRADIENT_TOLERANCE";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE:
        		enum_string = "STEP_TOLERANCE";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS:
        		enum_string = "MAXIMUM_ITERATIONS";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS:
        		enum_string = "MAXIMUM_FUNCTION_EVALUATIONS";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP:
        		enum_string = "MAXIMUM_STEP";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP:
        		enum_string = "MINIMUM_STEP";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE:
        		enum_string = "LINESEARCH_TOLERANCE";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS:
        		enum_string = "MAXIMUM_BACKTRACK_ITERATIONS";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE:
        		enum_string = "TRUST_REGION_SIZE";
        		break;
        	case CMISS_OPTIMISATION_ATTRIBUTE_DISPLAY_OUTPUT:
        		enum_string = "DISPLAY_OUTPUT";
        		break;
        	default:
        		break;
        }
        return enum_string;
    }
};

enum Cmiss_optimisation_attribute Cmiss_optimisation_attribute_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_optimisation_attribute,
	Cmiss_optimisation_attribute_conversion>(string);
}

char *Cmiss_optimisation_attribute_enum_to_string(
	enum Cmiss_optimisation_attribute attribute)
{
	const char *attribute_string = Cmiss_optimisation_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}

Cmiss_field_id Cmiss_optimisation_get_first_independent_field(
	Cmiss_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->getFirstIndependentField();
	return 0;
}

Cmiss_field_id Cmiss_optimisation_get_next_independent_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id ref_field)
{
	if (optimisation && ref_field)
		return optimisation->getNextIndependentField(ref_field);
	return 0;
}

int Cmiss_optimisation_add_independent_field(Cmiss_optimisation_id optimisation,
	Cmiss_field_id field)
{
	if (optimisation && field)
		return optimisation->addIndependentField(field);
	return 0;
}

int Cmiss_optimisation_remove_independent_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id field)
{
	if (optimisation && field)
		return optimisation->removeIndependentField(field);
	return 0;
}

Cmiss_field_id Cmiss_optimisation_get_first_objective_field(
	Cmiss_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->getFirstObjectiveField();
	return 0;
}

Cmiss_field_id Cmiss_optimisation_get_next_objective_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id ref_field)
{
	if (optimisation && ref_field)
		return optimisation->getNextObjectiveField(ref_field);
	return 0;
}

int Cmiss_optimisation_add_objective_field(Cmiss_optimisation_id optimisation,
	Cmiss_field_id field)
{
	if (optimisation && field)
		return optimisation->addObjectiveField(field);
	return 0;
}

int Cmiss_optimisation_remove_objective_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id field)
{
	if (optimisation && field)
		return optimisation->removeObjectiveField(field);
	return 0;
}

int Cmiss_optimisation_optimise(Cmiss_optimisation_id optimisation)
{
	if (optimisation)
		return optimisation->runOptimisation();
	return 0;
}
