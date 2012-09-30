

#include <stdio.h>

#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "general/multi_range.h"

int set_Multi_range(struct Parse_state *state,
	void *multi_range_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
Clears the existing Multi_range (which must have been created outside here) and
fills it with a new set of ranges. Ranges are entered as # or #..# in separate
tokens - as long as there is number in the token it is assumed to be a new
range. Numbers may start with + or - and have 0-9 as the first real entry.
Ranges may overlap and be increasing or decreasing. Typical inputs are:
4
5..7
3,5,7 (both commas and spaces are valid token separators).
1..2 3 6..9
3..5,9..1 (has the same result as 1..9)
==============================================================================*/
{
	const char *current_token;
	int first,last,number_of_characters_read,return_code;
	struct Multi_range *multi_range;

	ENTER(set_Multi_range);
	USE_PARAMETER(dummy_user_data);
	if (state&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		if (NULL != (current_token=state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Multi_range_clear(multi_range);
				return_code=1;
				/* read range from following tokens while they start with a number */
				while (return_code&&current_token&&
					(1==sscanf(current_token,"%d%n",&first,&number_of_characters_read)))
				{
					last=first;
					current_token += number_of_characters_read;
					if ('\0' != *current_token)
					{
						if (1==sscanf(current_token,"..%d%n",&last,
							&number_of_characters_read))
						{
							current_token += number_of_characters_read;
							if ('\0' != *current_token)
							{
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"set_Multi_range.  Invalid range text: %s",current_token);
						}
					}
					if (return_code)
					{
						if (Multi_range_add_range(multi_range,first,last))
						{
							return_code = shift_Parse_state(state,1);
							if (return_code)
							{
								current_token=state->current_token;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Multi_range.  Could not add range");
							return_code=0;
						}
					}
				}
				if (0==Multi_range_get_number_of_ranges(multi_range))
				{
					display_message(ERROR_MESSAGE,"Invalid integer range");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|#..#[,#|#..#[,etc.]]");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing integer range");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Multi_range */
