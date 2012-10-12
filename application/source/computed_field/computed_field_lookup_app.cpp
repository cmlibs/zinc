
#include "general/mystring.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_lookup.h"
#include "finite_element/finite_element_region.h"

class Computed_field_lookup_package : public Computed_field_type_package
{
public:
	struct Cmiss_region *root_region;
};

const char computed_field_nodal_lookup_type_string[] = "nodal_lookup";

const char computed_field_quaternion_SLERP_type_string[] = "quaternion_SLERP";

int Computed_field_get_type_quaternion_SLERP(struct Computed_field *field,
	struct Computed_field **quaternion_SLERP_field,
	struct FE_node **lookup_node);

int define_Computed_field_type_nodal_lookup(struct Parse_state *state,
	void *field_modify_void, void *computed_field_lookup_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODAL_LOOKUP (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field_lookup_package *computed_field_lookup_package;
	Computed_field_modify_data *field_modify;

	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_lookup_package=
		(Computed_field_lookup_package *)
		computed_field_lookup_package_void))
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		char *nodeset_name = duplicate_string("cmiss_nodes");
		char node_flag = 0;
		int node_identifier = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_nodal_lookup_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			Cmiss_node_id lookup_node = 0;
			return_code = Computed_field_get_type_nodal_lookup(field_modify->get_field(),
				&source_field, &lookup_node);
			if (source_field)
			{
				 ACCESS(Computed_field)(source_field);
			}
			if (lookup_node)
			{
				node_identifier = get_FE_node_identifier(lookup_node);
				FE_region *fe_region = FE_node_get_FE_region(lookup_node);
				if (!FE_region_contains_FE_node(fe_region, lookup_node))
				{
					DEALLOCATE(nodeset_name);
					nodeset_name = duplicate_string("cmiss_data");
				}
				node_flag = 1;
			}
		}

		Option_table *option_table = CREATE(Option_table)();
		/* source field */
		Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		set_source_field_data.conditional_function = Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table,"field", &source_field,
			&set_source_field_data,set_Computed_field_conditional);
		/* the node to nodal_lookup */
		Option_table_add_entry(option_table, "node", &node_identifier,
			&node_flag, set_int_and_char_flag);
		/* the nodeset the node is from */
		Option_table_add_string_entry(option_table, "nodeset", &nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_nodes|cmiss_data[cmiss_nodes]");
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code && node_flag)
		{
			Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_modify->get_field_module(), nodeset_name);
			Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_identifier);
			if (node)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_nodal_lookup(field_modify->get_field_module(),
						source_field, node));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define field nodal lookup.  Invalid node %d", node_identifier);
				return_code = 0;
			}
			Cmiss_node_destroy(&node);
			Cmiss_nodeset_destroy(&nodeset);
		}
		else
		{
			if ((!state->current_token)||
				(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_time_nodal_lookup.  Failed");
			}
		}
		DEALLOCATE(nodeset_name);
		REACCESS(Computed_field)(&source_field, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}



int define_Computed_field_type_quaternion_SLERP(struct Parse_state *state,
	void *field_modify_void, void *computed_field_lookup_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'quaterions' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field_lookup_package *computed_field_lookup_package;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_quaternion_SLERP);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void) &&
		 (computed_field_lookup_package=
				(Computed_field_lookup_package *)
				computed_field_lookup_package_void))
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		char *nodeset_name = duplicate_string("cmiss_nodes");
		char node_flag = 0;
		int node_identifier = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_quaternion_SLERP_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			Cmiss_node_id lookup_node = 0;
			return_code = Computed_field_get_type_quaternion_SLERP(field_modify->get_field(),
				 &source_field, &lookup_node);
			if (source_field)
			{
				 ACCESS(Computed_field)(source_field);
			}
			if (lookup_node)
			{
				node_identifier = get_FE_node_identifier(lookup_node);
				FE_region *fe_region = FE_node_get_FE_region(lookup_node);
				if (!FE_region_contains_FE_node(fe_region, lookup_node))
				{
					DEALLOCATE(nodeset_name);
					nodeset_name = duplicate_string("cmiss_data");
				}
				node_flag = 1;
			}
		}

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			 "A 4 components quaternion field. The components of "
			 "the quaternion field are expected to be the w, x, y, z components"
			 "of a quaternion (4 components in total). The quaternion field  is"
			 "evaluated and interpolated using SLERP at a normalised time between two"
			 "quaternions (read in from the exnode generally). This quaternion field"
			 "can be convert to a matrix with quaternion_to_matrix field, the resulting"
			 "matrix can be used to create a smooth time dependent rotation for an object"
			 "using the quaternion_to_matrix field. This field must be define directly from"
			 "exnode file or from a matrix_to_quaternion field");
		Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		set_source_field_data.conditional_function = Computed_field_has_4_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		/* identifier of the node to lookup */
		Option_table_add_entry(option_table, "node", &node_identifier,
			&node_flag, set_int_and_char_flag);
		/* the nodeset the node is from */
		Option_table_add_string_entry(option_table, "nodeset", &nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_nodes|cmiss_data[cmiss_nodes]");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code && node_flag)
		{
			Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_modify->get_field_module(), nodeset_name);
			Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_identifier);
			if (node)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_quaternion_SLERP(field_modify->get_field_module(),
						source_field, node));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define field quaternion_SLERP.  Invalid node %d", node_identifier);
				return_code = 0;
			}
			Cmiss_node_destroy(&node);
			Cmiss_nodeset_destroy(&nodeset);
		}
		else
		{
			if ((!state->current_token)||
				(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_quaternion_SLERP.  Failed");
			}
		}
		DEALLOCATE(nodeset_name);
		REACCESS(Computed_field)(&source_field, NULL);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"define_Computed_field_type_quaternion_SLERP.  Invalid argument(s)");
		 return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_quaternion_SLERP */

int Computed_field_register_types_lookup(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_lookup_package
		*computed_field_lookup_package =
		new Computed_field_lookup_package;

	ENTER(Computed_field_register_types_lookup);
	if (computed_field_package)
	{
		computed_field_lookup_package->root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodal_lookup_type_string,
			define_Computed_field_type_nodal_lookup,
			computed_field_lookup_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			 computed_field_quaternion_SLERP_type_string,
			 define_Computed_field_type_quaternion_SLERP,
			 computed_field_lookup_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_lookup */
