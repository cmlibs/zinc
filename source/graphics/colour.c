/*******************************************************************************
FILE : colour.c

LAST MODIFIED : 25 November 1999

DESCRIPTION :
Colour structures and support code.
???DB.  I'm not sure that this needs to be abstracted/formalized.
???DB.  Should convert to CREATE and DESTROY
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stdio.h>
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/colour.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
struct Colour *create_Colour(COLOUR_PRECISION red,COLOUR_PRECISION green,
	COLOUR_PRECISION blue)
/*******************************************************************************
LAST MODIFIED : 27 August 1996

DESCRIPTION :
Allocates memory and assigns field for a colour.
==============================================================================*/
{
	struct Colour *colour;

	ENTER(create_Colour);
	if (ALLOCATE(colour,struct Colour,1))
	{
		colour->red=red;
		colour->green=green;
		colour->blue=blue;
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Colour.  Insufficient memory");
	}
	LEAVE;

	return (colour);
} /* create_Colour */

void destroy_Colour(struct Colour **colour_address)
/*******************************************************************************
LAST MODIFIED : 27 August 1996

DESCRIPTION :
Frees the memory for the colour and sets <*colour_address> to NULL.
==============================================================================*/
{
	ENTER(destroy_Colour);
	/* checking arguments */
	if (colour_address)
	{
		if (*colour_address)
		{
			DEALLOCATE(*colour_address);
			*colour_address=(struct Colour *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_Colour.  Invalid argument");
	}
	LEAVE;
} /* destroy_Colour */

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
{
	char *current_token;
	COLOUR_PRECISION blue,green,red;
	int return_code;
	struct Colour *colour;

	ENTER(set_Colour);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (colour=(struct Colour *)colour_void)
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
				if (colour=(struct Colour *)colour_void)
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
