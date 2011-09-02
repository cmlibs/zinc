/***************************************************************************//**
 * FILE : computed_field_nodeset_operators.cpp
 *
 * Implementation of field operators that act on a nodeset.
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
#include <cmath>
#include <iostream>
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_node_private.hpp"
extern "C" {
#include "api/cmiss_field_nodeset_operators.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_set.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "finite_element/finite_element_region.h"
}
using namespace std;

class Computed_field_nodeset_operators_package : public Computed_field_type_package
{
public:
};

namespace {

char computed_field_nodeset_sum_type_string[] = "nodeset_sum";

class Computed_field_nodeset_sum : public Computed_field_core
{
protected:
	Cmiss_nodeset_id nodeset;
	int number_of_values;

public:
	Computed_field_nodeset_sum(Cmiss_nodeset_id nodeset_in) :
		Computed_field_core(),
		nodeset(Cmiss_nodeset_access(nodeset_in)),
		number_of_values(0)
	{
	};

	virtual ~Computed_field_nodeset_sum();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}
	
	Cmiss_nodeset_id get_nodeset()
	{
		return nodeset;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_nodeset_sum_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	int list();

	char* get_command_string();
	
protected:
	int evaluate_cache_at_location(Field_location* location);
};

Computed_field_nodeset_sum::~Computed_field_nodeset_sum()
{
	Cmiss_nodeset_destroy(&nodeset);
}

Computed_field_core *Computed_field_nodeset_sum::copy()
{
	return new Computed_field_nodeset_sum(nodeset);
}

int Computed_field_nodeset_sum::compare(Computed_field_core *other_core)
{
	Computed_field_nodeset_sum *other =
		dynamic_cast<Computed_field_nodeset_sum*>(other_core);
	if (other)
		return Cmiss_nodeset_match(nodeset, other->nodeset);
	return 0;
}

int Computed_field_nodeset_sum::is_defined_at_location(Field_location* location)
{
	int return_code = 0;
	if (field && location)
	{
		// Checks if source field is defined at a node in nodeset.
		Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
		Cmiss_node_id node = 0;
		while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
		{
			if (Computed_field_is_defined_at_node(field->source_fields[0], node))
			{
				return_code = 1;
				break;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
	}
	return return_code;
}

int Computed_field_nodeset_sum::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code;

	ENTER(Computed_field_nodeset_sum::evaluate_cache_at_location);
	if (field && location)
	{
		number_of_values = 0;
		// initialise values
		const int number_of_components = field->number_of_components;
		FE_value *values = field->values;
		Cmiss_field_id source_field = field->source_fields[0];
		FE_value *source_values = source_field->values;
		FE_value current_time = location->get_time();
		int i;
		for (i = 0; i < number_of_components; i++)
		{
			values[i] = 0;
		}
		Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
		Cmiss_node_id node = 0;
		while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
		{
			Field_node_location nodal_location(node, current_time);
			if (Computed_field_evaluate_cache_at_location(source_field, &nodal_location))
			{
				for (i = 0 ; i < number_of_components ; i++)
				{
					values[i] += source_values[i];
				}
				++number_of_values;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
		field->derivatives_valid = 0;
		return_code = (number_of_values > 0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodeset_sum::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/** Lists a description of the nodeset_sum inputs */
int Computed_field_nodeset_sum::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		char *nodeset_name = Cmiss_nodeset_get_name(nodeset);
		display_message(INFORMATION_MESSAGE,"    nodeset : %s\n", nodeset_name);
		DEALLOCATE(nodeset_name);
		return 1;
	}
	return 0;
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_nodeset_sum::get_command_string()
{
	char *command_string = 0;
	if (field)
	{
		int error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " field ", &error);
		append_string(&command_string, field->source_fields[0]->name, &error);
		char *nodeset_name = Cmiss_nodeset_get_name(nodeset);
		append_string(&command_string, " nodeset ", &error);
		make_valid_token(&nodeset_name);
		append_string(&command_string, nodeset_name, &error);
		DEALLOCATE(nodeset_name);
	}
	return (command_string);
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_nodeset_sum(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_nodeset_id nodeset)
{
	Cmiss_field_id field = 0;
	if (source_field && nodeset &&
		(Cmiss_field_module_get_master_region_internal(field_module) ==
			Cmiss_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_sum(nodeset));
	}
	return field;
}

/***************************************************************************//**
 * Converts <field> into type nodeset_sum (if it is not already) and allows its
 * contents to be modified.
 */
int define_Computed_field_type_nodeset_sum(struct Parse_state *state,
	void *field_modify_void, void *computed_field_nodeset_operators_package_void)
{
	int return_code;

	ENTER(define_Computed_field_type_nodeset_sum);
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		char *nodeset_name = 0;
		if (NULL != field_modify->get_field())
		{
			Computed_field_nodeset_sum *nodeset_sum_core =
				dynamic_cast<Computed_field_nodeset_sum*>(field_modify->get_field()->core);
			if (nodeset_sum_core)
			{
				source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
				nodeset_name = Cmiss_nodeset_get_name(nodeset_sum_core->get_nodeset());
			}
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"A nodeset_sum field calculates the sum of the supplied field's "
			"values over the nodes in the nodeset.");
		struct Set_Computed_field_conditional_data set_source_field_data =
			{ Computed_field_has_numerical_components, (void *)0, field_modify->get_field_manager() };
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		Option_table_add_string_entry(option_table, "nodeset", &nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_REGION_NAME.]cmiss_nodes|cmiss_data|none");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_nodeset_id nodeset = 0;
			if (nodeset_name)
			{
				nodeset = Cmiss_field_module_find_nodeset_by_name(
					field_modify->get_field_module(), nodeset_name);
				if (!nodeset)
				{
					display_message(ERROR_MESSAGE,
						"gfx define field nodeset_sum:  Unable to find nodeset %s", nodeset_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define field nodeset_sum:  Must specify nodeset");
				return_code = 0;
			}
			if (!source_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx define field nodeset_sum:  Must specify source field to sum");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_nodeset_sum(field_modify->get_field_module(),
						source_field, nodeset));
			}
			if (nodeset)
			{
				Cmiss_nodeset_destroy(&nodeset);
			}
		}
		if (nodeset_name)
		{
			DEALLOCATE(nodeset_name);
		}
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_nodeset_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}


namespace {

char computed_field_nodeset_mean_type_string[] = "nodeset_mean";

class Computed_field_nodeset_mean : public Computed_field_nodeset_sum
{
public:
	Computed_field_nodeset_mean(Cmiss_nodeset_id nodeset_in) :
		Computed_field_nodeset_sum(nodeset_in)
	{		
	};

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return (computed_field_nodeset_mean_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);
};

Computed_field_core *Computed_field_nodeset_mean::copy()
{
	return new Computed_field_nodeset_mean(nodeset);
}

int Computed_field_nodeset_mean::compare(Computed_field_core *other_core)
{
	Computed_field_nodeset_sum *other =
		dynamic_cast<Computed_field_nodeset_mean*>(other_core);
	if (other)
		return Cmiss_nodeset_match(nodeset, other->get_nodeset());
	return 0;
}

int Computed_field_nodeset_mean::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = Computed_field_nodeset_sum::evaluate_cache_at_location(location);
	if (return_code)
	{
		if (number_of_values > 0)
		{
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->values[i]/number_of_values;
			}
		}
	}
	return (return_code);
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_nodeset_mean(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_nodeset_id nodeset)
{
	Cmiss_field_id field = 0;
	if (source_field && nodeset &&
		(Cmiss_field_module_get_master_region_internal(field_module) ==
			Cmiss_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_mean(nodeset));
	}
	return field;
}

/***************************************************************************//**
 * Converts <field> into type nodeset_mean (if it is not already) and allows its
 * contents to be modified.
 */
int define_Computed_field_type_nodeset_mean(struct Parse_state *state,
	void *field_modify_void, void *computed_field_nodeset_operators_package_void)
{
	int return_code;

	ENTER(define_Computed_field_type_nodeset_mean);
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		char *nodeset_name = 0;
		if (NULL != field_modify->get_field())
		{
			Computed_field_nodeset_mean *nodeset_mean_core =
				dynamic_cast<Computed_field_nodeset_mean*>(field_modify->get_field()->core);
			if (nodeset_mean_core)
			{
				source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
				nodeset_name = Cmiss_nodeset_get_name(nodeset_mean_core->get_nodeset());
			}
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"A nodeset_mean field calculates the mean of the supplied field's "
			"values over the nodes in the nodeset.");
		struct Set_Computed_field_conditional_data set_source_field_data =
			{ Computed_field_has_numerical_components, (void *)0, field_modify->get_field_manager() };
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		Option_table_add_string_entry(option_table, "nodeset", &nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_REGION_NAME.]cmiss_nodes|cmiss_data|none");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_nodeset_id nodeset = 0;
			if (nodeset_name)
			{
				nodeset = Cmiss_field_module_find_nodeset_by_name(
					field_modify->get_field_module(), nodeset_name);
				if (!nodeset)
				{
					display_message(ERROR_MESSAGE,
						"gfx define field nodeset_mean:  Unable to find nodeset %s", nodeset_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define field nodeset_mean:  Must specify nodeset");
				return_code = 0;
			}
			if (!source_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx define field nodeset_mean:  Must specify source field to compute mean of");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_nodeset_mean(field_modify->get_field_module(),
						source_field, nodeset));
			}
			if (nodeset)
			{
				Cmiss_nodeset_destroy(&nodeset);
			}
		}
		if (nodeset_name)
		{
			DEALLOCATE(nodeset_name);
		}
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_nodeset_mean.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Computed_field_register_types_nodeset_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 01 May 2007

DESCRIPTION :
Registering the region operations.
==============================================================================*/
{
	int return_code;
	Computed_field_nodeset_operators_package
		*computed_field_nodeset_operators_package = 
		new Computed_field_nodeset_operators_package;

	ENTER(Computed_field_register_types_nodeset_operators);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodeset_sum_type_string, 
			define_Computed_field_type_nodeset_sum,
			computed_field_nodeset_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodeset_mean_type_string, 
			define_Computed_field_type_nodeset_mean,
			computed_field_nodeset_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_nodeset_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
