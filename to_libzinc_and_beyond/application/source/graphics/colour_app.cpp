

#include <stdio.h>
#include "command/parser.h"

#include "general/message.h"
#include "general/debug.h"
#include "general/message.h"
#include "graphics/colour.h"
#include "user_interface/user_interface.h"
#include "graphics/colour_app.h"

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
{
	const char *current_token;
	COLOUR_PRECISION blue,green,red;
	int return_code;
	struct Colour *colour;

	ENTER(set_Colour);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (NULL != (current_token=state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (NULL != (colour=(struct Colour *)colour_void))
				{
					if ((1==sscanf(current_token," %"COLOUR_PRECISION_STRING" ",&red))&&
						shift_Parse_state(state,1)&&(state->current_token)&&
						(1==sscanf(state->current_token," %"COLOUR_PRECISION_STRING" ",
						&green))&&shift_Parse_state(state,1)&&(state->current_token)&&
						(1==sscanf(state->current_token," %"COLOUR_PRECISION_STRING" ",
						&blue)))
					{
						/* make sure that the rgb values are between 0 and 1 inclusive */
						if (red<0)
						{
							red=0;
						}
						else
						{
							if (red>1)
							{
								red=1;
							}
						}
						if (green<0)
						{
							green=0;
						}
						else
						{
							if (green>1)
							{
								green=1;
							}
						}
						if (blue<0)
						{
							blue=0;
						}
						else
						{
							if (blue>1)
							{
								blue=1;
							}
						}
						/* assign values */
						colour->red=red;
						colour->green=green;
						colour->blue=blue;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing/invalid rgb value(s)");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Colour.  Missing colour");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," RED#");
				if (NULL != (colour=(struct Colour *)colour_void))
				{
					display_message(INFORMATION_MESSAGE,"[%g]",colour->red);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,<=1} GREEN#");
				if (colour)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",colour->green);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,<=1} BLUE#");
				if (colour)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",colour->blue);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,<=1}");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing rgb value(s) for colour");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Colour.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Colour */
