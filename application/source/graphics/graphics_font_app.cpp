#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include <string>
#include "api/cmiss_graphics_font.h"
#include "api/cmiss_graphics_module.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/font.h"


int gfx_define_font(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_module_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Executes a GFX DEFINE FONT command.
==============================================================================*/
{
	const char *current_token, *font_name;
	int return_code;
	Cmiss_graphics_module_id graphics_module = 0;

	if (state && (graphics_module = (Cmiss_graphics_module_id)graphics_module_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				font_name = current_token;
				if (shift_Parse_state(state,1)&&
					(current_token=state->current_token))
				{
					Cmiss_graphics_font_id font = Cmiss_graphics_module_find_font_by_name(
						graphics_module, font_name);
					if (!font)
					{
						font = Cmiss_graphics_module_create_font(graphics_module);
						Cmiss_graphics_font_set_name(font, font_name);
					}
					Cmiss_graphics_font_type font_type = Cmiss_graphics_font_get_type(font);
					Cmiss_graphics_font_true_type true_type = Cmiss_graphics_font_get_true_type(font);
					char *font_type_string = 0;
					char *true_type_string = 0;
					int number_of_valid_strings_font_type = 0;
					int number_of_valid_strings_true_type = 0;
					const char **valid_font_type_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_graphics_font_type)(
						&number_of_valid_strings_font_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_font_type) *)NULL,
						(void *)NULL);
					std::string all_font_types = " ";
					for (int i = 0; i < number_of_valid_strings_font_type; i++)
					{
						if (i)
							all_font_types += "|";

						all_font_types += valid_font_type_strings[i];
					}
					const char *all_font_types_help = all_font_types.c_str();
					const char **valid_font_true_type_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_graphics_font_true_type)(
						&number_of_valid_strings_true_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_font_true_type) *)NULL,
						(void *)NULL);
					std::string all_font_true_types = " ";
					for (int i = 0; i < number_of_valid_strings_true_type; i++)
					{
						if (i)
							all_font_true_types += "|";

						all_font_true_types += valid_font_true_type_strings[i];
					}
					const char *all_font_true_types_help = all_font_true_types.c_str();

					struct Option_table *option_table = CREATE(Option_table)();
					int bold_flag = 0;
					int italic_flag = 0;
					float depth = (float)Cmiss_graphics_font_get_depth(font);
					int size = Cmiss_graphics_font_get_size(font);
					/* bold */
					Option_table_add_entry(option_table, "bold",
						(void *)&bold_flag, NULL, set_char_flag);
					Option_table_add_entry(option_table, "italic",
						(void *)&italic_flag, NULL, set_char_flag);
					Option_table_add_entry(option_table,"depth",
						&(depth),NULL,set_float);
					Option_table_add_entry(option_table,"size",
						&(size),NULL,set_int_non_negative);
					Option_table_add_string_entry(option_table, "font_type",
						&font_type_string, all_font_types_help);
					Option_table_add_string_entry(option_table, "true_type",
						&true_type_string, all_font_true_types_help);
					return_code = Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (font_type_string)
						{
							STRING_TO_ENUMERATOR(Cmiss_graphics_font_type)(font_type_string,
								&font_type);
							if (CMISS_GRAPHICS_FONT_TYPE_INVALID == font_type)
							{
								display_message(ERROR_MESSAGE,
									"gfx_define_font:  Invalid font type %s", font_type_string);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_font:  Missing font_type argument");
							return_code = 0;
						}
						if (true_type_string)
						{
							STRING_TO_ENUMERATOR(Cmiss_graphics_font_true_type)(true_type_string,
								&true_type);
							if (CMISS_GRAPHICS_FONT_TRUE_TYPE_INVALID == true_type)
							{
								display_message(ERROR_MESSAGE,
									"gfx_define_font:  Invalid true type %s", true_type_string);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_font:  Missing true_type argument");
							return_code = 0;
						}
						if (font)
						{
							Cmiss_graphics_font_set_bold(font, bold_flag);
							Cmiss_graphics_font_set_italic(font, italic_flag);
							Cmiss_graphics_font_set_depth(font, depth);
							Cmiss_graphics_font_set_size(font, size);
							Cmiss_graphics_font_set_true_type(font, true_type);
							Cmiss_graphics_font_set_type(font, font_type);
						}

					}

					Cmiss_graphics_font_destroy(&font);
					DEALLOCATE(font_type_string);
					DEALLOCATE(true_type_string);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing font string.");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FONT_NAME FONT_STRING\n");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing font name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_font.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* gfx_define_font */

