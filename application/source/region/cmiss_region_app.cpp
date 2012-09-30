
#include "api/cmiss_region.h"
#include "general/message.h"
#include "general/mystring.h"
#include "general/debug.h"
#include "general/object.h"
#include "command/parser.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_app.h"

int set_Cmiss_region(struct Parse_state *state, void *region_address_void,
	void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct Cmiss_region *region, **region_address, *root_region;

	ENTER(set_Cmiss_region);
	if (state && (root_region = static_cast<struct Cmiss_region *>(root_region_void)) &&
		(region_address = static_cast<struct Cmiss_region **>(region_address_void)))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				region = Cmiss_region_find_subregion_at_path(root_region, current_token);
				if (region)
				{
					//-- REACCESS(Cmiss_region)(region_address, region);
					return_code = shift_Parse_state(state, 1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Cmiss_region:  Could not find subregion %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if (*region_address)
				{
					char *path = Cmiss_region_get_path(*region_address);
					display_message(INFORMATION_MESSAGE, "[%s]", path);
					DEALLOCATE(path);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_Cmiss_region.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_set_Cmiss_region(struct Option_table *option_table,
	const char *token, struct Cmiss_region *root_region,
	struct Cmiss_region **region_address)
{
	int return_code;

	ENTER(Option_table_add_set_Cmiss_region);
	if (option_table && root_region && region_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_address, (void *)root_region, set_Cmiss_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_Cmiss_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int set_Cmiss_region_path(struct Parse_state *state, void *path_address_void,
	void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Modifier function for entering a path to a Cmiss_region, starting at
<root_region>.
==============================================================================*/
{
	const char *current_token;
	char **path_address;
	int return_code;
	struct Cmiss_region *region, *root_region;

	ENTER(set_Cmiss_region_path);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				region = Cmiss_region_find_subregion_at_path(
					root_region, current_token);
				if (region)
				{
					if ((path_address = (char **)path_address_void))
					{
						if (*path_address)
						{
							DEALLOCATE(*path_address);
						}
						if ((*path_address = duplicate_string(current_token)))
						{
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Cmiss_region_path.  Could not allocate memory for path");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_Cmiss_region_path.  Missing path_address");
						return_code = 0;
					}
					DEACCESS(Cmiss_region)(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Invalid region path: %s",
						current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if ((path_address = (char **)path_address_void) && (*path_address))
				{
					display_message(INFORMATION_MESSAGE, "[%s]", *path_address);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_Cmiss_region_path.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Cmiss_region_path */

int Option_table_add_set_Cmiss_region_path(struct Option_table *option_table,
	const char *entry_string, struct Cmiss_region *root_region, char **path_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Adds an entry to the <option_table> with name <entry_name> that returns a
region path in <path_address> relative to the <root_region>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Option_table_add_set_Cmiss_region_path);
	if (option_table && entry_string && root_region && path_address)
	{
		return_code = Option_table_add_entry(option_table, entry_string,
			(void *)path_address, (void *)root_region,
			set_Cmiss_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_Cmiss_region_path.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_set_Cmiss_region_path */



/***************************************************************************//**
 * Modifier function to set the region, region path and field name.
 * Fields must not be the same name as a child region.
 *
 * Examples:
 *   /heart/coordinates = region path and field name
 *   heart              = region path only
 *   coordinates        = field name only
 * @param region_path_and_name a struct Cmiss_region_path_and_name which if
 *   set contains an ACCESSed region and allocated path and name which caller
 *   is required to clean up. Name may be NULL if path is fully resolved.
 */
static int set_region_path_and_or_field_name(struct Parse_state *state,
	void *region_path_and_name_void, void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct Cmiss_region_path_and_name *name_data;
	struct Cmiss_region *root_region;

	ENTER(set_region_path_and_or_field_name);
	if (state && (name_data = (struct Cmiss_region_path_and_name *)region_path_and_name_void) &&
		(root_region = (struct Cmiss_region *)root_region_void))
	{
		current_token = state->current_token;
		if (!current_token)
		{
			display_message(WARNING_MESSAGE, "Missing region path and/or field name");
			display_parse_state_location(state);
			return_code = 0;
		}
		else if (Parse_state_help_mode(state))
		{
			display_message(INFORMATION_MESSAGE, " REGION_PATH|REGION_PATH/FIELD_NAME|FIELD_NAME");
			return_code = 1;
		}
		else if (Cmiss_region_get_partial_region_path(root_region, current_token,
			&name_data->region, &name_data->region_path, &name_data->name))
		{
			ACCESS(Cmiss_region)(name_data->region);
			if (!name_data->name || (NULL == strchr(name_data->name, CMISS_REGION_PATH_SEPARATOR_CHAR)))
			{
				return_code = shift_Parse_state(state, 1);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Bad region path and/or field name: %s",current_token);
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_region_path_and_or_field_name.  Failed to get path and name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_region_path_and_or_field_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_region_path_and_or_field_name_entry(
	struct Option_table *option_table, char *token,
	struct Cmiss_region_path_and_name *region_path_and_name,
	struct Cmiss_region *root_region)
{
	int return_code;

	ENTER(Option_table_add_region_path_and_or_field_name_entry);
	if (option_table && region_path_and_name && root_region)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_path_and_name, (void *)root_region,
			set_region_path_and_or_field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_region_path_and_or_field_name_entry.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_region_path_and_or_field_name_entry */

int set_Cmiss_region_or_group(struct Parse_state *state,
	void *region_address_void, void *group_address_void)
{
	Cmiss_region_id *region_address = reinterpret_cast<Cmiss_region_id*>(region_address_void);
	Cmiss_field_group_id *group_address = reinterpret_cast<Cmiss_field_group_id*>(group_address_void);
	if (!(state && region_address && *region_address && group_address && !*group_address))
		return 0;
	const char *current_token = state->current_token;
	int return_code = 1;
	if (!current_token)
	{
		display_message(WARNING_MESSAGE, "Missing region/group path");
		display_parse_state_location(state);
		return_code =  0;
	}
	else if (Parse_state_help_mode(state))
	{
		display_message(INFORMATION_MESSAGE, " REGION_PATH/GROUP");
	}
	else
	{
		char *region_path = 0;
		char *field_name = 0;
		Cmiss_region_id output_region = 0;
		if (Cmiss_region_get_partial_region_path(*region_address, current_token,
			&output_region, &region_path, &field_name) && output_region)
		{
			REACCESS(Cmiss_region)(region_address, output_region);
			if (field_name)
			{
				Cmiss_field *field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					field_name, Cmiss_region_get_Computed_field_manager(output_region));
				*group_address = Cmiss_field_cast_group(field);
				if (0 == *group_address)
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				shift_Parse_state(state, 1);
			}
		}
		else
		{
			return_code = 0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "Invalid region or group path: %s", current_token);
			display_parse_state_location(state);
		}
		DEALLOCATE(region_path);
		DEALLOCATE(field_name);
	}
	return return_code;
}

int Option_table_add_region_or_group_entry(struct Option_table *option_table,
	const char *token, Cmiss_region_id *region_address,
	Cmiss_field_group_id *group_address)
{
	if (!(option_table && region_address && *region_address && group_address))
	{
		display_message(ERROR_MESSAGE, "Option_table_add_region_or_group_entry.  Invalid argument(s)");
		return 0;
	}
	return Option_table_add_entry(option_table, token,
		(void *)region_address, (void *)group_address, set_Cmiss_region_or_group);
}
