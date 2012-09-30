
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */


#include "command/parser.h"
#include "general/message.h"
#include "graphics/render_gl.h"

int set_Graphics_object(struct Parse_state *state,
	void *graphics_object_address_void,void *graphics_object_manager_void)
/*******************************************************************************
LAST MODIFIED : 26 November 1998

DESCRIPTION :
Modifier function to set the graphics_object from a command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct GT_object *graphics_object,**graphics_object_address;
	struct MANAGER(GT_object) *graphics_object_manager;

	ENTER(set_Graphics_Object);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((graphics_object_address=
					(struct GT_object **)graphics_object_address_void)&&
					(graphics_object_manager=
					(struct MANAGER(GT_object)*)graphics_object_manager_void ))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*graphics_object_address)
						{
							DEACCESS(GT_object)(graphics_object_address);
							*graphics_object_address=(struct GT_object *)NULL;
						}
						return_code=1;
					}
					else
					{
						graphics_object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
							current_token,graphics_object_manager);
						if (graphics_object)
						{
							if (*graphics_object_address != graphics_object)
							{
								ACCESS(GT_object)(graphics_object);
								if (*graphics_object_address)
								{
									DEACCESS(GT_object)(graphics_object_address);
								}
								*graphics_object_address=graphics_object;
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Unknown graphics object : %s",current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Graphics_object.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME|none");
				/* if possible, then write the name */
				graphics_object_address=(struct GT_object **)graphics_object_address_void;
				if (graphics_object_address)
				{
					graphics_object= *graphics_object_address;
					if (graphics_object)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",GT_object_get_name(graphics_object));
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics_object name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_object */

