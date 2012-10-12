
#include <stdio.h>

#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "finite_element/finite_element_region.h"
// insert app headers here
#include "finite_element/finite_element_app.h"

int set_FE_field_component_FE_region(struct Parse_state *state,
	void *fe_field_component_address_void, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_field_component.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region, *master_fe_region;

	ENTER(set_FE_field_component_FE_region);
	if (state && fe_field_component_address_void &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		/* get the ultimate master FE_region; only it has field info */
		master_fe_region = FE_region_get_ultimate_master_FE_region(fe_region);
		return_code = set_FE_field_component(state, fe_field_component_address_void,
			(void *)(FE_region_get_FE_field_list(master_fe_region)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_component_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_component_FE_region */

int set_FE_field_conditional_FE_region(struct Parse_state *state,
	void *fe_field_address_void, void *parse_field_data_void)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
FE_region wrapper for set_FE_field_conditional. <parse_field_data_void> points
at a struct Set_FE_field_conditional_FE_region_data.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region, *master_fe_region;
	struct Set_FE_field_conditional_data set_field_data;
	struct Set_FE_field_conditional_FE_region_data *parse_field_data;

	ENTER(set_FE_field_conditional_FE_region);
	if (state && fe_field_address_void && (parse_field_data =
		(struct Set_FE_field_conditional_FE_region_data *)parse_field_data_void)
		&& (fe_region = parse_field_data->fe_region))
	{
		/* get the ultimate master FE_region; only it has field info */
		master_fe_region = FE_region_get_ultimate_master_FE_region(fe_region);
		set_field_data.conditional_function =
			parse_field_data->conditional_function;
		set_field_data.conditional_function_user_data =
			parse_field_data->user_data;
		set_field_data.fe_field_list = FE_region_get_FE_field_list(master_fe_region);
		return_code = set_FE_field_conditional(state, fe_field_address_void,
			(void *)&set_field_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_conditional_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_conditional_FE_region */

int set_FE_fields_FE_region(struct Parse_state *state,
	void *fe_field_order_info_address_void, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_fields.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region, *master_fe_region;

	ENTER(set_FE_fields_FE_region);
	if (state && fe_field_order_info_address_void &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		/* get the ultimate master FE_region; only it has fe_field_list */
		master_fe_region = FE_region_get_ultimate_master_FE_region(fe_region);
		return_code = set_FE_fields(state, fe_field_order_info_address_void,
			(void *)(FE_region_get_FE_field_list(master_fe_region)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_fields_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_fields_FE_region */

int Option_table_add_set_FE_field_from_FE_region(
	struct Option_table *option_table, const char *entry_string,
	struct FE_field **fe_field_address, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
Adds an entry for selecting an FE_field.
==============================================================================*/
{
	int return_code = 0;
	struct FE_region *master_fe_region;

	ENTER(Option_table_add_set_FE_field_from_FE_region);
	if (option_table && entry_string && fe_field_address && fe_region)
	{
		/* get the ultimate master FE_region; only it has field info */
		master_fe_region = FE_region_get_ultimate_master_FE_region(fe_region);
		Option_table_add_entry(option_table, entry_string,
			(void *)fe_field_address, FE_region_get_FE_field_list(master_fe_region),
			set_FE_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_FE_field_from_FE_region.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_set_FE_field_from_FE_region */


int set_FE_node_FE_region(struct Parse_state *state, void *node_address_void,
	void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Used in command parsing to translate a node name into an node from <fe_region>.
==============================================================================*/
{
	const char *current_token;
	int identifier, return_code;
	struct FE_node *node;
	struct FE_region *fe_region;

	ENTER(set_FE_node_FE_region);
	struct FE_node **node_address = reinterpret_cast<struct FE_node **>(node_address_void);
	if ((state) && (node_address) &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((1 == sscanf(current_token, "%d", &identifier)) &&
					(node = FE_region_get_FE_node_from_identifier(fe_region, identifier)))
				{
					REACCESS(FE_node)(node_address, node);
					return_code = shift_Parse_state(state,1);
				}
				else
				{
					display_message(WARNING_MESSAGE, "Unknown node: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " NODE_NUMBER");
				node= *node_address;
				if (node)
				{
					display_message(INFORMATION_MESSAGE, "[%d]",
						get_FE_node_identifier(node));
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing number for node");
			display_parse_state_location(state);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_node_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_FE_region */



int set_FE_element_top_level_FE_region(struct Parse_state *state,
	void *element_address_void, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
A modifier function for specifying a top level element, used, for example, to
set the seed element for a xi_texture_coordinate computed_field.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct FE_element *element, **element_address;
	struct FE_region *fe_region;

	ENTER(set_FE_element_top_level_FE_region);
	if (state && (element_address = (struct FE_element **)element_address_void) &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				int element_number = 0;
				if (1 == sscanf(current_token, "%d", &element_number))
				{
					shift_Parse_state(state,1);
					element = FE_region_get_top_level_FE_element_from_identifier(fe_region, element_number);
					if (element)
					{
						REACCESS(FE_element)(element_address, element);
						return_code = 1;
					}
				}
				if (!return_code)
				{
					display_message(WARNING_MESSAGE,
						"Unknown seed element: %s", current_token);
					display_parse_state_location(state);
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " ELEMENT_NUMBER");
				element = *element_address;
				if (element)
				{
					struct CM_element_information cm;
					get_FE_element_identifier(element, &cm);
					display_message(INFORMATION_MESSAGE, "[%d]", cm.number);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing seed element number");
			display_parse_state_location(state);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_top_level_FE_region.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_element_top_level_FE_region */
