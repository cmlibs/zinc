
#include "zinc/fieldmodule.h"
#include "general/debug.h"
#include "general/message.h"
#include "general/list.h"
#include "general/mystring.h"
#include "command/parser.h"
#include "mesh/cmiss_element_private.hpp"

static int set_Cmiss_mesh(struct Parse_state *state, void *region_void, void *mesh_address_void)
{
	Cmiss_region_id region = reinterpret_cast<Cmiss_region_id>(region_void);
	Cmiss_mesh_id *mesh_address = reinterpret_cast<Cmiss_mesh_id*>(mesh_address_void);
	if (!(state && region && mesh_address))
		return 0;
	const char *current_token = state->current_token;
	int return_code = 1;
	if (!current_token)
	{
		display_message(WARNING_MESSAGE, "Missing mesh name");
		display_parse_state_location(state);
		return_code =  0;
	}
	else if ((0 == strcmp(PARSER_HELP_STRING, current_token)) ||
		(0 == strcmp(PARSER_RECURSIVE_HELP_STRING, current_token)))
	{
		display_message(INFORMATION_MESSAGE, " ELEMENT_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_mesh_1d|cmiss_mesh_2d|cmiss_mesh_3d[");
		if (*mesh_address)
		{
			char *mesh_name = Cmiss_mesh_get_name(*mesh_address);
			make_valid_token(&mesh_name);
			display_message(INFORMATION_MESSAGE, "%s]", mesh_name);
			DEALLOCATE(mesh_name);
		}
		else
		{
			display_message(INFORMATION_MESSAGE, "none]");
		}
	}
	else
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		Cmiss_mesh_id new_mesh = Cmiss_field_module_find_mesh_by_name(field_module, current_token);
		if (new_mesh)
		{
			if (*mesh_address)
				Cmiss_mesh_destroy(mesh_address);
			*mesh_address = new_mesh;
			shift_Parse_state(state, 1);
		}
		else
		{
			display_message(ERROR_MESSAGE, "Invalid mesh: %s", current_token);
			display_parse_state_location(state);
			return_code = 0;
		}
		Cmiss_field_module_destroy(&field_module);
	}
	return return_code;
}

int Option_table_add_mesh_entry(struct Option_table *option_table,
	const char *token, Cmiss_region_id region, Cmiss_mesh_id *mesh_address)
{
	if (!(option_table && token && region && mesh_address))
	{
		display_message(ERROR_MESSAGE, "Option_table_add_mesh_entry.  Invalid argument(s)");
		return 0;
	}
	return Option_table_add_entry(option_table, token,
		(void *)region, (void *)mesh_address, set_Cmiss_mesh);
}
