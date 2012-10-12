
#include "api/cmiss_field_nodeset_operators.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "mesh/cmiss_node_private.hpp"

class Computed_field_nodeset_operators_package : public Computed_field_type_package
{
};

const char computed_field_nodeset_mean_squares_type_string[] = "nodeset_mean_squares";
const char computed_field_nodeset_sum_squares_type_string[] = "nodeset_sum_squares";
const char computed_field_nodeset_mean_type_string[] = "nodeset_mean";
const char computed_field_nodeset_sum_type_string[] = "nodeset_sum";
const char computed_field_nodeset_operator_type_string[] = "nodeset_operator";



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
	if (NULL != field_modify->get_field()  &&
			(computed_field_nodeset_operator_type_string ==
			Computed_field_get_type_string(field_modify->get_field())))
	{
		source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
		nodeset_name = Cmiss_field_get_name(source_field);
		//-- nodeset_name = Cmiss_nodeset_get_name(nodeset_operator_core->get_nodeset());
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
		" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_nodes|cmiss_data");
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
				nodeset = Cmiss_nodeset_group_base_cast(
					Cmiss_field_module_create_nodeset_group_from_name_internal(
						field_modify->get_field_module(), nodeset_name));
			}
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
