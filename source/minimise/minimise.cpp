/**
 * FILE : minimise.cpp
 *
 * Optimisation/minimisation routines.
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

extern "C" {
#include <stdio.h>
#include <math.h>
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_optimisation.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/any_object_private.h"
#include "general/any_object_definition.h"
#include "general/callback_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "region/cmiss_region_private.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"
#include "minimise/minimise.h"
}
#include "general/enumerator_private_cpp.hpp"

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_optimisation_method)
;

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_optimisation_method) {
	const char *enumerator_string;
	ENTER(ENUMERATOR_STRING(Cmiss_optimisation_method));
	switch (enumerator_value) {
	case CMISS_OPTIMISATION_METHOD_NSDGSL: {
		enumerator_string = "NSDGSL";
	}
		break;
	case CMISS_OPTIMISATION_METHOD_QUASI_NEWTON: {
		enumerator_string = "QUASI_NEWTON";
	}
		break;
	case CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON: {
		enumerator_string = "LEAST_SQUARES_QUASI_NEWTON";
	}
		break;
	default: {
		enumerator_string = (const char *) NULL;
	}
		break;
	}LEAVE;
	return (enumerator_string);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS( Cmiss_optimisation_method)
;

struct Minimisation_package
{
	struct Time_keeper *time_keeper;
	struct Computed_field_package *computed_field_package;
	struct Cmiss_region *root_region;
	enum Cmiss_optimisation_method method;
};

struct Minimisation_package *CREATE(Minimisation_package)(
		struct Time_keeper *time_keeper,
		struct Computed_field_package *computed_field_package,
		struct Cmiss_region *root_region)
/*******************************************************************************
 LAST MODIFIED : 7 May 2007

 DESCRIPTION :
 Creates the package required for minimisation.
 ==============================================================================*/
{
	struct Minimisation_package *package;

	ENTER(CREATE(Minimisation_package));
	if (time_keeper)
	{
		ALLOCATE(package, struct Minimisation_package, 1);
		package->time_keeper = time_keeper;
		package->computed_field_package = computed_field_package;
		package->root_region = root_region;
		package->method = CMISS_OPTIMISATION_METHOD_INVALID;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Minimisation_package).  Invalid arguments.");
		package = (struct Minimisation_package *)NULL;
	}
	LEAVE;

	return (package);
} /* CREATE(Minimisation_package) */

int DESTROY(Minimisation_package)(struct Minimisation_package **package)
/*******************************************************************************
 LAST MODIFIED : 7 May 2007

 DESCRIPTION :
 Destroys the package required for minimisation.
 ==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Minimisation_package));
	if (*package)
	{
		DEALLOCATE(*package);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Minimisation_package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Minimisation_package) */

int gfx_minimise(struct Parse_state *state, void *dummy_to_be_modified,
		void *package_void)
/*******************************************************************************
 LAST MODIFIED : 04 May 2007

 DESCRIPTION : Minimises the <objective_field> by changing the <independent_field>
 over a <region>
 ==============================================================================*/
{
	int dimension, number_of_valid_strings, maxIters, return_code, i;
	struct Computed_field *objective_field, *data_field, *mesh_field;
	struct Multiple_strings field_names;
	struct Minimisation_package *package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_objective_field_data,
		set_data_field_data,set_mesh_field_data;
	const char *minimisation_method_string, **valid_strings;

	ENTER(gfx_minimise);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (package = (struct Minimisation_package *) package_void))
	{
		dimension = 2;
		struct Cmiss_region* meshRegion = NULL;
		struct Cmiss_region* dataRegion = NULL;
		objective_field = (struct Computed_field *) NULL;
		data_field = (struct Computed_field *) NULL;
		mesh_field = (struct Computed_field *) NULL;
		if (package->time_keeper)
		{
			// time = Time_keeper_get_time(package->time_keeper);
		} else {
			// time = 0;
		}

		option_table = CREATE(Option_table)();
		/* objective function */
		set_objective_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_objective_field_data.conditional_function_user_data = (void *) NULL;
		set_objective_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "objective_field",
				&objective_field, &set_objective_field_data,
				set_Computed_field_conditional);
		/* independent field(s) */
		field_names.number_of_strings = 0;
		field_names.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "independent_fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* data field */
		set_data_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_data_field_data.conditional_function_user_data
				= (void *) NULL;
		set_data_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "data_field",
				&data_field, &set_data_field_data,
				set_Computed_field_conditional);
		/* mesh field */
		set_mesh_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_mesh_field_data.conditional_function_user_data
				= (void *) NULL;
		set_mesh_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "mesh_field",
				&mesh_field, &set_mesh_field_data,
				set_Computed_field_conditional);
		/* region/groups */
		Option_table_add_set_Cmiss_region(option_table, "mesh_region",
			package->root_region, &meshRegion);
		Option_table_add_set_Cmiss_region(option_table, "data_region",
			package->root_region, &dataRegion);
		/* method to use */
		minimisation_method_string = ENUMERATOR_STRING(Cmiss_optimisation_method)(
				package->method);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_optimisation_method)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_optimisation_method) *) NULL,
				(void *) NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &minimisation_method_string);
		DEALLOCATE(valid_strings);
		/* limit the number of iterations (really the number of objective
		   function evaluations) */
		maxIters = 100; // default value
		Option_table_add_entry(option_table, "maximum_iterations", &maxIters,
			NULL, set_int_positive);
		/* the dimension of the elements to be fit */
		// FIXME: should be restricted to 1 or 2?
		Option_table_add_entry(option_table, "dimension", &dimension,
					NULL, set_int_positive);

		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if ((field_names.number_of_strings > 0) && (objective_field || (data_field && mesh_field))) {
				if (meshRegion && dataRegion)
				{
					if (minimisation_method_string)
					{
						STRING_TO_ENUMERATOR(Cmiss_optimisation_method)(
							minimisation_method_string, &(package->method));
					}

					Cmiss_field_module_id fieldModule = Cmiss_region_get_field_module(meshRegion);
					Cmiss_optimisation_id optimisation = Cmiss_field_module_create_optimisation(fieldModule);
					Cmiss_optimisation_set_method(optimisation, package->method);

					Cmiss_fe_mesh_id feMesh = NULL;
					if (dimension == 1) feMesh = Cmiss_field_module_get_fe_mesh_by_name(fieldModule, "cmiss_mesh_1d");
					else if (dimension == 2) feMesh = Cmiss_field_module_get_fe_mesh_by_name(fieldModule, "cmiss_mesh_2d");
					else if (dimension == 3) feMesh = Cmiss_field_module_get_fe_mesh_by_name(fieldModule, "cmiss_mesh_3d");
					if (feMesh == NULL)
					{
						display_message(ERROR_MESSAGE, "gfx_minimise.  Unable to get the fe_mesh, invalid dimension?");
						return_code = 0;
					}
					else
					{
						Cmiss_optimisation_set_fe_mesh(optimisation, feMesh);
						Cmiss_fe_mesh_destroy(&feMesh);
						Cmiss_optimisation_set_attribute_integer(optimisation, CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS,
								maxIters);
						Cmiss_optimisation_set_attribute_field(optimisation,
								CMISS_OPTIMISATION_ATTRIBUTE_OBJECTIVE_FIELD, objective_field);
						for (i=0;i<field_names.number_of_strings;i++)
						{
							Cmiss_field_id field = Cmiss_field_module_find_field_by_name(fieldModule, field_names.strings[i]);
							Cmiss_optimisation_add_independent_field(optimisation, field);
						}
						return_code = Cmiss_optimisation_optimise(optimisation);
					}
					Cmiss_field_module_destroy(&fieldModule);
					Cmiss_optimisation_destroy(&optimisation);
				} else {
					display_message(ERROR_MESSAGE,
							"gfx_minimise.  Must specify mesh and data regions");
					return_code = 0;
				}
			} else {
				display_message(ERROR_MESSAGE,
						"gfx_minimise.  Must specify independent field and either an objective field or a data and a mesh field");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (meshRegion)
		{
			Cmiss_region_destroy(&meshRegion);
		}
		if (dataRegion)
		{
			Cmiss_region_destroy(&dataRegion);
		}
		if (objective_field)
		{
			Cmiss_field_destroy(&objective_field);
		}
		if (data_field)
		{
			Cmiss_field_destroy(&data_field);
		}
		if (mesh_field)
		{
			Cmiss_field_destroy(&mesh_field);
		}
		if (field_names.strings)
		{
			for (int i = 0; i < field_names.number_of_strings; i++)
			{
				DEALLOCATE(field_names.strings[i]);
			}
			DEALLOCATE(field_names.strings);
		}
	} else {
		display_message(ERROR_MESSAGE, "gfx_minimise.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_minimise */

