
#include "zinc/core.h"
#include "zinc/graphicsmodule.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/tessellation.hpp"
#include "graphics/graphics_module.h"


/***************************************************************************//**
 * Modifier function for setting positive numbers of divisions separated by *.
 *
 * @param divisions_address_void  address of where to allocate return int array.
 * Must be initialised to NULL or allocated array.
 * @param size_address_void  address of int to receive array size. Must be
 * initialised to size of initial divisions_address, or 0 if none.
 */
int set_divisions(struct Parse_state *state,
	void *divisions_address_void, void *size_address_void)
{
	int return_code = 1;

	int **divisions_address = (int **)divisions_address_void;
	int *size_address = (int *)size_address_void;
	if (state && divisions_address && size_address &&
		((NULL == *divisions_address) || (0 < *size_address)))
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				int size = 0;
				int *values = NULL;
				if (current_token)
				{
					return_code = string_to_divisions(current_token, &values, &size);
				}
				if (!return_code)
				{
					display_parse_state_location(state);
				}
				if (return_code)
				{
					shift_Parse_state(state,1);
					DEALLOCATE(*divisions_address);
					*divisions_address = values;
					*size_address = size;
				}
				else
				{
					DEALLOCATE(values);
				}
			}
			else
			{
				/* write help */
				display_message(INFORMATION_MESSAGE, " \"#*#*...\"");
				if (*divisions_address)
				{
					display_message(INFORMATION_MESSAGE, "[\"");
					list_divisions(*size_address, *divisions_address);
					display_message(INFORMATION_MESSAGE, "\"]");
				}
				display_message(INFORMATION_MESSAGE, "{>=0}");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing values");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_divisions.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Option_table_add_divisions_entry(struct Option_table *option_table,
	const char *token, int **divisions_address, int *size_address)
{
	int return_code;
	if (option_table && token && divisions_address && size_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)divisions_address, (void *)size_address, set_divisions);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_divisions_entry.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int gfx_define_tessellation_contents(struct Parse_state *state, void *tessellation_void,
	void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		Cmiss_tessellation *tessellation = (Cmiss_tessellation *)tessellation_void; // can be null
		int minimum_divisions_size = 1;
		int refinement_factors_size = 1;
		if (tessellation)
		{
			minimum_divisions_size = Cmiss_tessellation_get_attribute_integer(tessellation,
				CMISS_TESSELLATION_ATTRIBUTE_MINIMUM_DIVISIONS_SIZE);
			refinement_factors_size = Cmiss_tessellation_get_attribute_integer(tessellation,
				CMISS_TESSELLATION_ATTRIBUTE_REFINEMENT_FACTORS_SIZE);
		}
		int *minimum_divisions;
		int *refinement_factors;
		ALLOCATE(minimum_divisions, int, minimum_divisions_size);
		ALLOCATE(refinement_factors, int, refinement_factors_size);
		if (tessellation)
		{
			Cmiss_tessellation_get_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions);
			Cmiss_tessellation_get_refinement_factors(tessellation, refinement_factors_size, refinement_factors);
		}
		else
		{
			minimum_divisions[0] = 1;
			refinement_factors[0] = 1;
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Defines tessellation objects which control how finite elements are "
			"subdivided into graphics. The minimum_divisions option gives the "
			"minimum number of linear segments approximating geometry in each xi "
			"dimension of the element. If the coordinate field of a graphic uses "
			"non-linear basis functions the minimum_divisions is multiplied by "
			"the refinement_factors to give the refined number of segments. "
			"Both minimum_divisions and refinement_factors use the last supplied "
			"number for all higher dimensions, so \"4\" = \"4*4\" and so on.");
		Option_table_add_divisions_entry(option_table, "minimum_divisions",
			&minimum_divisions, &minimum_divisions_size);
		Option_table_add_divisions_entry(option_table, "refinement_factors",
			&refinement_factors, &refinement_factors_size);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
		if (return_code && tessellation)
		{
			return_code =
				Cmiss_tessellation_set_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions) &&
				Cmiss_tessellation_set_refinement_factors(tessellation, refinement_factors_size, refinement_factors);
		}
		DEALLOCATE(minimum_divisions);
		DEALLOCATE(refinement_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}


/***************************************************************************//**
 * @see Option_table_add_set_Cmiss_tessellation
 */
int set_Cmiss_tessellation(struct Parse_state *state,
	void *tessellation_address_void, void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_tessellation **tessellation_address = (Cmiss_tessellation **)tessellation_address_void;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && tessellation_address && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Cmiss_tessellation *tessellation = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					tessellation = Cmiss_graphics_module_find_tessellation_by_name(graphics_module, current_token);
					if (!tessellation)
					{
						display_message(ERROR_MESSAGE, "Unknown tessellation : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(Cmiss_tessellation)(tessellation_address, tessellation);
					shift_Parse_state(state,1);
				}
				if (tessellation)
					Cmiss_tessellation_destroy(&tessellation);
			}
			else
			{
				char *name = Cmiss_tessellation_get_name(*tessellation_address);
				display_message(INFORMATION_MESSAGE," TESSELLATION_NAME|none[%s]",
					(*tessellation_address) ? name : "none");
				Cmiss_deallocate(name);
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Cmiss_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Option_table_add_Cmiss_tessellation_entry(struct Option_table *option_table,
	const char *token, struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_tessellation **tessellation_address)
{
	int return_code;
	if (option_table && token && graphics_module && tessellation_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)tessellation_address, (void *)graphics_module,
			set_Cmiss_tessellation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_Cmiss_tessellation_entry.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int gfx_define_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void)
{
	int return_code = 1;

	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				MANAGER(Cmiss_tessellation) *tessellation_manager =
					Cmiss_graphics_module_get_tessellation_manager(graphics_module);
				MANAGER_BEGIN_CACHE(Cmiss_tessellation)(tessellation_manager);
				Cmiss_tessellation *tessellation =
					Cmiss_graphics_module_find_tessellation_by_name(graphics_module, current_token);
				if (!tessellation)
				{
					tessellation = Cmiss_graphics_module_create_tessellation(graphics_module);
					Cmiss_tessellation_set_name(tessellation, current_token);
				}
				shift_Parse_state(state,1);
				if (tessellation)
				{
					// set managed state for all tessellations created or edited otherwise
					// cleaned up at end of command.
					Cmiss_tessellation_set_attribute_integer(tessellation, CMISS_TESSELLATION_ATTRIBUTE_IS_MANAGED, 1);
					return_code = gfx_define_tessellation_contents(state, (void *)tessellation, graphics_module_void);
				}
				Cmiss_tessellation_destroy(&tessellation);
				MANAGER_END_CACHE(Cmiss_tessellation)(tessellation_manager);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "TESSELLATION_NAME",
					/*tessellation*/(void *)NULL, graphics_module_void,
					gfx_define_tessellation_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_destroy_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_module_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	if (state && graphics_module_void)
	{
		Cmiss_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, graphics_module_void, set_Cmiss_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				Cmiss_tessellation_set_attribute_integer(tessellation, CMISS_TESSELLATION_ATTRIBUTE_IS_MANAGED, 0);
				//-- if (tessellation->access_count > 2)
				//-- {
				//-- 	display_message(INFORMATION_MESSAGE, "Tessellation marked for destruction when no longer in use.\n");
				//-- }
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing tessellation name");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		if (tessellation)
		{
			Cmiss_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int gfx_list_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_module_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		Cmiss_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, graphics_module_void, set_Cmiss_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				return_code = list_Cmiss_tessellation_iterator(tessellation, (void *)NULL);
			}
			else
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Cmiss_tessellation)(list_Cmiss_tessellation_iterator,
					(void *)NULL, Cmiss_graphics_module_get_tessellation_manager(graphics_module));
			}
		}
		if (tessellation)
		{
			Cmiss_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

