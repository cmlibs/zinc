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
	#include "minimise/cmiss_optimisation_private.h"
	#include "api/cmiss_field.h"
	#include "api/cmiss_region.h"
	#include "computed_field/computed_field.h"
	#include "computed_field/computed_field_composite.h"
	#include "computed_field/computed_field_finite_element.h"
}
#include "minimise/optimisation.hpp"

Cmiss_optimisation_id Cmiss_optimisation_create_private(void)
{
	return new Cmiss_optimisation();
}

int Cmiss_optimisation_destroy(Cmiss_optimisation_id *optimisation_address)
{
	if (optimisation_address)
		return Cmiss_optimisation::destroy(*optimisation_address);
	return 0;
}

enum Cmiss_optimisation_method Cmiss_optimisation_get_method(Cmiss_optimisation_id optimisation)
{
	if (optimisation) return optimisation->method;
	return CMISS_OPTIMISATION_METHOD_INVALID;
}

int Cmiss_optimisation_set_method(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_method method)
{
	if (optimisation)
	{
		optimisation->method = method;
		return 1;
	}
	return 0;
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
		default:
			fprintf(stderr, "Invalid integer attribute\n");
		}
	}
	return 0;
}

int Cmiss_optimisation_set_attribute_integer(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, int value)
{
	int code = 1;
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
		default:
			fprintf(stderr, "Invalid integer attribute\n");
			code = 0;
		}
	}
	return code;
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
			fprintf(stderr, "Invalid real attribute\n");
		}
	}
	return 0.0;
}

int Cmiss_optimisation_set_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, double value)
{
	int code = 1;
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
			fprintf(stderr, "Invalid real attribute\n");
			code = 0;
		}
	}
	return code;
}

Cmiss_field_id Cmiss_optimisation_get_attribute_field(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute)
{
	if (optimisation)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_OBJECTIVE_FIELD:
			return Cmiss_field_access(optimisation->objectiveField);
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MESH_FIELD:
			return Cmiss_field_access(optimisation->meshField);
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_DATA_FIELD:
			return Cmiss_field_access(optimisation->dataField);
			break;
		default:
			fprintf(stderr, "Invalid field attribute\n");
		}
	}
	return NULL;
}

int Cmiss_optimisation_set_attribute_field(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, Cmiss_field_id value)
{
	int code = 1;
	if (optimisation && value)
	{
		switch (attribute)
		{
		case CMISS_OPTIMISATION_ATTRIBUTE_OBJECTIVE_FIELD:
			if (optimisation->objectiveField) Cmiss_field_destroy(&(optimisation->objectiveField));
			optimisation->objectiveField = Cmiss_field_access(value);
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_MESH_FIELD:
			if (optimisation->meshField) Cmiss_field_destroy(&(optimisation->meshField));
			optimisation->meshField = Cmiss_field_access(value);
			break;
		case CMISS_OPTIMISATION_ATTRIBUTE_DATA_FIELD:
			if (optimisation->dataField) Cmiss_field_destroy(&(optimisation->dataField));
			optimisation->dataField = Cmiss_field_access(value);
			break;
		default:
			fprintf(stderr, "Invalid field attribute\n");
			code = 0;
		}
	}
	return code;
}

int Cmiss_optimisation_add_independent_field(Cmiss_optimisation_id optimisation,
		Cmiss_field_id field)
{
	int code = 0;
	if (optimisation && field)
	{
		if (optimisation->independentFieldsConstant && !Computed_field_is_constant(field))
		{
			display_message(ERROR_MESSAGE, "Cmiss_optimisation_add_independent_field.  "
					"All independent fields must be constant if any are.");
			return 0;
		}
		if (Computed_field_is_type_finite_element(field) && (optimisation->independentFields.size() > 0))
		{
			display_message(ERROR_MESSAGE, "Cmiss_optimisation_add_independent_field.  "
					"Can only have one finite element type independent field.");
			return 0;
		}
		FieldList::const_iterator iter;
		bool found = false;
		for (iter = optimisation->independentFields.begin(); iter != optimisation->independentFields.end(); ++iter)
		{
			if (*iter == field) found = true;
		}
		if (!found)
		{
			optimisation->independentFields.push_back(Cmiss_field_access(field));
			if (Computed_field_is_constant(field)) optimisation->independentFieldsConstant = true;
			code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cmiss_optimisation_add_independent_field.  "
					"An independent field can only be added once.");
			return 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_add_independent_field.  "
							"Invalid arguments.");
	}
	return code;
}

int Cmiss_optimisation_set_mesh(Cmiss_optimisation_id optimisation, Cmiss_mesh_id mesh)
{
	int code = 0;
	if (optimisation && mesh)
	{
		if (optimisation->feMesh) Cmiss_mesh_destroy(&(optimisation->feMesh));
		optimisation->feMesh = Cmiss_mesh_access(mesh);
		code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_set_mesh.  "
							"Invalid arguments.");
	}
	return code;
}

int Cmiss_optimisation_set_data_nodeset(Cmiss_optimisation_id optimisation, Cmiss_nodeset_id nodeset)
{
	int code = 0;
	if (optimisation && nodeset)
	{
		if (optimisation->dataNodeset) Cmiss_nodeset_destroy(&(optimisation->dataNodeset));
		optimisation->dataNodeset = Cmiss_nodeset_access(nodeset);
		code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_set_data_nodeset.  "
							"Invalid arguments.");
	}
	return code;
}

int Cmiss_optimisation_optimise(Cmiss_optimisation_id optimisation)
{
	int code = 0;
	if (optimisation)
	{
		code = 1;
		// check required attributes are set
		if (optimisation->feMesh == NULL)
		{
			code = 0;
			display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The FE mesh must be set.");
		}
		if (optimisation->independentFields.size() < 1)
		{
			code = 0;
			display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. Must set at least one independent field.");
		}
		if ((optimisation->method == CMISS_OPTIMISATION_METHOD_QUASI_NEWTON) ||
				(optimisation->method == CMISS_OPTIMISATION_METHOD_NSDGSL))
		{
			if (optimisation->objectiveField == NULL)
			{
				code = 0;
				display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The objective field must be set.");
			}
		}
		if (optimisation->method == CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON)
		{
			if (optimisation->meshField == NULL)
			{
				code = 0;
				display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The mesh field must be set.");
			}
			if (optimisation->dataField == NULL)
			{
				code = 0;
				display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The data field must be set.");
			}
			if (code)
			{
				int nM = Cmiss_field_get_number_of_components(optimisation->meshField);
				int nD = Cmiss_field_get_number_of_components(optimisation->dataField);
				if (nD < nM)
				{
					code = 0;
					display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The data field must have at least the same number "
							"of components as the mesh field.");
				}
			}
			if (optimisation->dataNodeset == NULL)
			{
				code = 0;
				display_message(ERROR_MESSAGE,"Cmiss_optimisation_optimise. The data nodeset must be set.");
			}
		}
		if (code != 0)
		{
			code = optimisation->runOptimisation();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_optimisation_optimise.  "
							"Invalid arguments.");
	}
	return code;
}

