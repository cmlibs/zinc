

#include "zinc/zincconfigure.h"


#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include "command/parser.h"

#include "three_d_drawing/graphics_buffer.h"
#include "general/message.h"
#include "general/enumerator_private.hpp"

#include "graphics/render_gl.h"

int set_Texture_storage(struct Parse_state *state,void *enum_storage_void_ptr,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
A modifier function to set the texture storage type.
==============================================================================*/
{
	const char *current_token, *storage_type_string;
	enum Texture_storage_type *storage_type_address, storage_type;
	int storage_type_int, return_code = 0;

	ENTER(set_Texture_storage);
	if (state && state->current_token && (!dummy_user_data))
	{
		storage_type_address=(enum Texture_storage_type *)enum_storage_void_ptr;
		if (storage_type_address)
		{
			if (NULL != (current_token=state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (STRING_TO_ENUMERATOR(Texture_storage_type)(
						state->current_token, &storage_type) &&
						Texture_storage_type_is_user_selectable(storage_type, (void *)NULL))
					{
						*storage_type_address = storage_type;
						return_code = shift_Parse_state(state, 1);
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE, "Invalid storage type %s",
							current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					/* write help */
					display_message(INFORMATION_MESSAGE, " ");
					storage_type_int = 0;
					while ((storage_type_string =
						ENUMERATOR_STRING(Texture_storage_type)(
						(enum Texture_storage_type)storage_type_int)) &&
						Texture_storage_type_is_user_selectable(
						(enum Texture_storage_type)storage_type_int, (void *)NULL))
					{
						if (storage_type_int != 0)
						{
							display_message(INFORMATION_MESSAGE,"|");
						}
						display_message(INFORMATION_MESSAGE, storage_type_string);
						if (storage_type_int == *storage_type_address)
						{
							display_message(INFORMATION_MESSAGE,"[%s]",
								storage_type_string);
						}
						storage_type_int++;
					}
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing texture storage type");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_storage.  Missing storage enum.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Texture_storage.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Texture_storage */


