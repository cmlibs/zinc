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

class Computed_field_nodeset_operator : public Computed_field_core
{
protected:
	Cmiss_nodeset_id nodeset;
	int number_of_terms;

public:
	Computed_field_nodeset_operator(Cmiss_nodeset_id nodeset_in) :
		Computed_field_core(),
		nodeset(Cmiss_nodeset_access(nodeset_in))
	{
		number_of_terms = 0;
	};

	virtual ~Computed_field_nodeset_operator()
	{
		Cmiss_nodeset_destroy(&nodeset);
	}

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

	int is_defined_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_nodeset_operator::is_defined_at_location(Field_location* location)
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

/** Lists a description of the nodeset_operator arguments */
int Computed_field_nodeset_operator::list()
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
char *Computed_field_nodeset_operator::get_command_string()
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


const char computed_field_nodeset_sum_type_string[] = "nodeset_sum";

class Computed_field_nodeset_sum : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_sum(Cmiss_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	};

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_sum(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_sum_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum *other =
			dynamic_cast<Computed_field_nodeset_sum*>(other_core);
		if (other)
			return Cmiss_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

protected:
	int evaluate_cache_at_location(Field_location* location);

};

int Computed_field_nodeset_sum::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code;
	if (field && location)
	{
		number_of_terms = 0;
		const int number_of_components = field->number_of_components;
		FE_value *values = field->values;
		Cmiss_field_id source_field = field->source_fields[0];
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
					values[i] += source_field->values[i];
				}
				++number_of_terms;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
		field->derivatives_valid = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodeset_sum::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}


const char computed_field_nodeset_mean_type_string[] = "nodeset_mean";

class Computed_field_nodeset_mean : public Computed_field_nodeset_sum
{
public:
	Computed_field_nodeset_mean(Cmiss_nodeset_id nodeset_in) :
		Computed_field_nodeset_sum(nodeset_in)
	{
	};

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_mean(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_mean_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum *other =
			dynamic_cast<Computed_field_nodeset_mean*>(other_core);
		if (other)
			return Cmiss_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

protected:
	int evaluate_cache_at_location(Field_location* location);

};

int Computed_field_nodeset_mean::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = Computed_field_nodeset_sum::evaluate_cache_at_location(location);
	if (return_code)
	{
		if (number_of_terms > 0)
		{
			FE_value scaling = 1.0 / (FE_value)number_of_terms;
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] *= scaling;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}


const char computed_field_nodeset_sum_squares_type_string[] = "nodeset_sum_squares";

class Computed_field_nodeset_sum_squares : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_sum_squares(Cmiss_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	};

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_sum_squares(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_sum_squares_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum_squares *other =
			dynamic_cast<Computed_field_nodeset_sum_squares*>(other_core);
		if (other)
			return Cmiss_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	virtual int supports_sum_square_terms() const
	{
		return 1;
	}

	virtual int get_number_of_sum_square_terms() const;

	int evaluate_sum_square_terms_at_location(Field_location* location,
		int number_of_values, FE_value *values);

protected:
	int evaluate_cache_at_location(Field_location* location);

};

int Computed_field_nodeset_sum_squares::get_number_of_sum_square_terms() const
{
	int return_number_of_terms = 0;
	if (field)
	{
		Cmiss_field_id source_field = field->source_fields[0];
		Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
		Cmiss_node_id node = 0;
		while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
		{
			Field_node_location nodal_location(node);
			if (source_field->core->is_defined_at_location(&nodal_location))
			{
				++return_number_of_terms;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
	}
	return return_number_of_terms;
}

int Computed_field_nodeset_sum_squares::evaluate_sum_square_terms_at_location(
	Field_location* location, int number_of_values, FE_value *values)
{
	int return_code = 0;
	if (field && location && (0 <= number_of_values) && values)
	{
		return_code = 1;
		number_of_terms = 0;
		const int number_of_components = field->number_of_components;
		const int max_terms = number_of_values / number_of_components;
		FE_value *value = values;
		Cmiss_field_id source_field = field->source_fields[0];
		FE_value current_time = location->get_time();
		int i;
		Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
		Cmiss_node_id node = 0;
		while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
		{
			Field_node_location nodal_location(node, current_time);
			if (Computed_field_evaluate_cache_at_location(source_field, &nodal_location))
			{
				if (number_of_terms >= max_terms)
				{
					return_code = 0;
					break;
				}
				for (i = 0 ; i < number_of_components ; i++)
				{
					*value = source_field->values[i];
					++value;
				}
				++number_of_terms;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
		if (number_of_terms*number_of_components != number_of_values)
		{
			return_code = 0;
		}
	}
	return return_code;
}

int Computed_field_nodeset_sum_squares::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code;
	if (field && location)
	{
		number_of_terms = 0;
		const int number_of_components = field->number_of_components;
		FE_value *values = field->values;
		Cmiss_field_id source_field = field->source_fields[0];
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
					values[i] += source_field->values[i]*source_field->values[i];
				}
				++number_of_terms;
			}
		}
		Cmiss_node_iterator_destroy(&iterator);
		field->derivatives_valid = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodeset_sum_squares::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}


const char computed_field_nodeset_mean_squares_type_string[] = "nodeset_mean_squares";

class Computed_field_nodeset_mean_squares : public Computed_field_nodeset_sum_squares
{
public:
	Computed_field_nodeset_mean_squares(Cmiss_nodeset_id nodeset_in) :
		Computed_field_nodeset_sum_squares(nodeset_in)
	{
	};

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_mean_squares(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_mean_squares_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_mean_squares *other =
			dynamic_cast<Computed_field_nodeset_mean_squares*>(other_core);
		if (other)
			return Cmiss_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate_sum_square_terms_at_location(Field_location* location,
		int number_of_values, FE_value *values);

protected:
	int evaluate_cache_at_location(Field_location* location);

};

int Computed_field_nodeset_mean_squares::evaluate_sum_square_terms_at_location(
	Field_location* location, int number_of_values, FE_value *values)
{
	int return_code = Computed_field_nodeset_sum_squares::evaluate_sum_square_terms_at_location(
		location, number_of_values, values);
	if (return_code)
	{
		if (number_of_terms > 0)
		{
			FE_value scaling = 1.0 / sqrt(number_of_terms);
			for (int i = 0 ; i < number_of_values ; i++)
			{
				values[i] *= scaling;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}

int Computed_field_nodeset_mean_squares::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = Computed_field_nodeset_sum_squares::evaluate_cache_at_location(location);
	if (return_code)
	{
		if (number_of_terms > 0)
		{
			FE_value scaling = 1.0 / (FE_value)number_of_terms;
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] *= scaling;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}

} //namespace

/***************************************************************************//**
 * Command modifier function for getting the arguments common to all
 * nodeset_operator-derived field types.
 * @return  1 on success with nodeset and source_field accessing respective
 * objects, or 0 on failure with no objects accessed.
 */
int define_Computed_field_type_nodeset_operator(struct Parse_state *state,
	Computed_field_modify_data *field_modify, const char *type_name, const char *help_string,
	Cmiss_field_id &source_field, Cmiss_nodeset_id &nodeset)
{
	if (!(state && field_modify && help_string))
		return 0;
	int return_code = 1;
	source_field = 0;
	nodeset = 0;
	char *nodeset_name = 0;
	if (NULL != field_modify->get_field())
	{
		Computed_field_nodeset_operator *nodeset_operator_core =
			dynamic_cast<Computed_field_nodeset_operator*>(field_modify->get_field()->core);
		if (nodeset_operator_core)
		{
			source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
			nodeset_name = Cmiss_nodeset_get_name(nodeset_operator_core->get_nodeset());
		}
	}
	Option_table *option_table = CREATE(Option_table)();
	Option_table_add_help(option_table, help_string);
	struct Set_Computed_field_conditional_data set_source_field_data =
	{
		Computed_field_has_numerical_components,
		(void *)0,
		field_modify->get_field_manager()
	};
	Option_table_add_entry(option_table, "field", &source_field,
		&set_source_field_data, set_Computed_field_conditional);
	Option_table_add_string_entry(option_table, "nodeset", &nodeset_name,
		" NODE_GROUP_FIELD_NAME|[GROUP_REGION_NAME.]cmiss_nodes|cmiss_data");
	return_code = Option_table_multi_parse(option_table, state);
	DESTROY(Option_table)(&option_table);
	if (return_code)
	{
		if (nodeset_name)
		{
			nodeset = Cmiss_field_module_find_nodeset_by_name(
				field_modify->get_field_module(), nodeset_name);
			if (!nodeset)
			{
				display_message(ERROR_MESSAGE,
					"gfx define field %s:  Unable to find nodeset %s", type_name, nodeset_name);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Must specify nodeset", type_name);
			return_code = 0;
		}
		if (!source_field)
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Must specify source field", type_name);
			return_code = 0;
		}
	}
	if (nodeset_name)
	{
		DEALLOCATE(nodeset_name);
	}
	if (!return_code)
	{
		if (nodeset)
		{
			Cmiss_nodeset_destroy(&nodeset);
		}
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
	}
	return (return_code);
}

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
	int return_code = 0;
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	Cmiss_field_id source_field = 0;
	Cmiss_nodeset_id nodeset = 0;
	if (define_Computed_field_type_nodeset_operator(state, field_modify, "nodeset_sum",
		"A nodeset_sum field calculates the sums of each of the supplied field's "
		"component values over all nodes in the nodeset over which it is defined.",
		source_field, nodeset))
	{
		return_code = field_modify->update_field_and_deaccess(
			Cmiss_field_module_create_nodeset_sum(field_modify->get_field_module(),
				source_field, nodeset));
		Cmiss_field_destroy(&source_field);
		Cmiss_nodeset_destroy(&nodeset);
	}
	return return_code;
}

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
	int return_code = 0;
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	Cmiss_field_id source_field = 0;
	Cmiss_nodeset_id nodeset = 0;
	if (define_Computed_field_type_nodeset_operator(state, field_modify, "nodeset_mean",
		"A nodeset_mean field calculates the means of each of the supplied field's "
		"component values over all nodes in the nodeset over which it is defined.",
		source_field, nodeset))
	{
		return_code = field_modify->update_field_and_deaccess(
			Cmiss_field_module_create_nodeset_mean(field_modify->get_field_module(),
				source_field, nodeset));
		Cmiss_field_destroy(&source_field);
		Cmiss_nodeset_destroy(&nodeset);
	}
	return return_code;
}

Cmiss_field_id Cmiss_field_module_create_nodeset_sum_squares(
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
			new Computed_field_nodeset_sum_squares(nodeset));
	}
	return field;
}

/***************************************************************************//**
 * Converts <field> into type nodeset_sum_squares (if it is not already) and
 * allows its contents to be modified.
 */
int define_Computed_field_type_nodeset_sum_squares(struct Parse_state *state,
	void *field_modify_void, void *computed_field_nodeset_operators_package_void)
{
	int return_code = 0;
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	Cmiss_field_id source_field = 0;
	Cmiss_nodeset_id nodeset = 0;
	if (define_Computed_field_type_nodeset_operator(state, field_modify, "nodeset_sum_squares",
		"A nodeset_sum_squares field calculates the sums of the squares of each of the "
		"supplied field's component values over all nodes in the nodeset over which it is "
		"defined. This field supplies individual terms to least-squares optimisation methods. "
		"See 'gfx minimise' command.", source_field, nodeset))
	{
		return_code = field_modify->update_field_and_deaccess(
			Cmiss_field_module_create_nodeset_sum_squares(field_modify->get_field_module(),
				source_field, nodeset));
		Cmiss_field_destroy(&source_field);
		Cmiss_nodeset_destroy(&nodeset);
	}
	return return_code;
}

Cmiss_field_id Cmiss_field_module_create_nodeset_mean_squares(
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
			new Computed_field_nodeset_mean_squares(nodeset));
	}
	return field;
}

/***************************************************************************//**
 * Converts <field> into type nodeset_mean_squares (if it is not already) and
 * allows its contents to be modified.
 */
int define_Computed_field_type_nodeset_mean_squares(struct Parse_state *state,
	void *field_modify_void, void *computed_field_nodeset_operators_package_void)
{
	int return_code = 0;
	USE_PARAMETER(computed_field_nodeset_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	Cmiss_field_id source_field = 0;
	Cmiss_nodeset_id nodeset = 0;
	if (define_Computed_field_type_nodeset_operator(state, field_modify, "nodeset_mean_squares",
		"A nodeset_mean_squares field calculates the means of the squares of each of the "
		"supplied field's component values over all nodes in the nodeset over which it is "
		"defined. This field supplies individual terms to least-squares optimisation methods. "
		"See 'gfx minimise' command.", source_field, nodeset))
	{
		return_code = field_modify->update_field_and_deaccess(
			Cmiss_field_module_create_nodeset_mean_squares(field_modify->get_field_module(),
				source_field, nodeset));
		Cmiss_field_destroy(&source_field);
		Cmiss_nodeset_destroy(&nodeset);
	}
	return return_code;
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
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodeset_sum_squares_type_string,
			define_Computed_field_type_nodeset_sum_squares,
			computed_field_nodeset_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodeset_mean_squares_type_string,
			define_Computed_field_type_nodeset_mean_squares,
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
