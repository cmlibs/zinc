/*******************************************************************************
FILE : application.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
The functions linking cmgui with applications.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "application/application.h"
#include "command/parser.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/
int set_application_parameters(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 September 1996

DESCRIPTION :
Called in response to the command
	gfx set application_parameters ...
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(set_application_parameters);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
/*???debug */
{
	printf("called set_application_parameters with :\n");
	while (state->current_token)
	{
		printf("%s\n",state->current_token);
		shift_Parse_state(state,1);
	}
}
				return_code=0;
			}
			else
			{
				/*???DB.  Display help information */
				return_code=1;
			}
		}
		else
		{
			/*???DB.  Display missing parameters error message */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_application_parameters.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_application_parameters */
