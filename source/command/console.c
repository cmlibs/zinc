/*******************************************************************************
FILE : console.c

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Management routines for the main command window.
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
#include <stdlib.h>
#if defined (WIN32_SYSTEM)
#  include <direct.h>
#else /* !defined (WIN32_SYSTEM) */
#  include <unistd.h>
#endif /* !defined (WIN32_SYSTEM) */
#if defined (UNIX)
#include <termios.h>
#include <sgtty.h>
#endif /* defined (UNIX) */
#include "general/debug.h"
#include "general/object.h"
#include "command/console.h"
#include "command/command.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Console
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
==============================================================================*/
{
	char *command_prompt;
	struct Event_dispatcher *event_dispatcher;
	Cmiss_native_socket_t fd;
	Fdio_id console_fdio;
	struct Execute_command *execute_command;
}; /* struct Console */

/*
Module functions
----------------
*/

static int Console_callback(Fdio_id fdio, void *console_void)
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
This function is called to process stdin from a console.
==============================================================================*/
{
#define ESC_KEYCODE '\033'
#define KILL_KEYCODE '\025'
#define MAX_CONSOLE_BUFFER (1000)
	char buffer[MAX_CONSOLE_BUFFER];
	int length, prompt_length, return_code;
	struct Console *console;
#if defined (UNIX)
	int i;
#endif /* defined (UNIX) */

	ENTER(Console_callback);

	USE_PARAMETER(fdio);

	if (console=(struct Console *)console_void)
	{
		length = read(console->fd, buffer, MAX_CONSOLE_BUFFER);
		if (length)
		{
			/* Look for control codes */
			buffer[length - 1] = 0;
			return_code=Execute_command_execute_string(console->execute_command,
				buffer);
			if (console->command_prompt)
			{
				prompt_length = strlen(console->command_prompt);
#if defined (UNIX)
				for (i = 0 ; i < prompt_length ; i++)
				{
					/* Put the prompt out to the terminal as if it had been typed in */
					ioctl(console->fd, TIOCSTI, console->command_prompt + i);
				}
#else
				printf("%s", console->command_prompt);
#endif
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Console_callback.  Missing console");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Console_callback */

/*
Global functions
----------------
*/
struct Console *CREATE(Console)(struct Execute_command *execute_command,
	struct Event_dispatcher *event_dispatcher, int file_descriptor)
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION:
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	struct Console *console;

	ENTER(CREATE(console));
	/* check arguments, file_descriptor can be 0 */
	if (execute_command&&event_dispatcher)
	{
		if (ALLOCATE(console,struct Console,1))
		{
			console->command_prompt = (char *)NULL;
			console->execute_command=execute_command;
			console->event_dispatcher = event_dispatcher;
			console->fd = file_descriptor;
			console->console_fdio =
				Event_dispatcher_create_Fdio(event_dispatcher, file_descriptor);

			if (!console->console_fdio)
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Console).  Unable to register callback for console.");
				DEALLOCATE(console);
				console=(struct Console *)NULL;
			}

			Fdio_set_read_callback(console->console_fdio, Console_callback,
				(void*)console);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Console).  Insufficient memory for Console structure");
			console=(struct Console *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Console).  Invalid argument(s)");
		console=(struct Console *)NULL;
	}
	LEAVE;

	return (console);
} /* CREATE(Console) */

int DESTROY(Console)(struct Console **console_pointer)
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION:
==============================================================================*/
{
	int return_code;
	struct Console *console;

	if (console_pointer && (console = *console_pointer))
	{
		DESTROY(Fdio)(&console->console_fdio);
		DEALLOCATE(*console_pointer);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(console).  Missing command window");
		return_code = 0;
	}

	return (return_code);
} /* DESTROY(console) */

int Console_set_command_prompt(struct Console *console, char *prompt)
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <console>.
==============================================================================*/
{
	char *new_prompt;
	int return_code;

	ENTER(set_command_prompt);
	if (console)
	{
		if (prompt)
		{
			if (REALLOCATE(new_prompt, console->command_prompt, char, strlen(prompt) + 2))
			{
				console->command_prompt = new_prompt;
				strcpy(new_prompt, prompt);
				strcat(new_prompt, " ");
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_command_prompt.  Missing command window");
				return_code=0;
			}
		}
		else
		{
			DEALLOCATE(console->command_prompt);
			console->command_prompt = (char *)NULL;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_command_prompt.  Missing command window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_command_prompt */

