/*******************************************************************************
FILE : console.c

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Management routines for the main command window.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
	struct Event_dispatcher_descriptor_callback *console_callback;
	struct Execute_command *execute_command;
}; /* struct Console */

/*
Module functions
----------------
*/

int Console_callback(int file_descriptor, void *console_void)
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
	int i, length, prompt_length, return_code;
	struct Console *console;

	ENTER(Console_callback);
	if (console=(struct Console *)console_void)
	{
		length = read(file_descriptor, buffer, MAX_CONSOLE_BUFFER);
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
					ioctl(file_descriptor, TIOCSTI, console->command_prompt + i);
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
			console->event_dispatcher=event_dispatcher;
#if !defined(WIN32_USER_INTERFACE)
			if (!(console->console_callback = Event_dispatcher_add_simple_descriptor_callback(
				event_dispatcher, file_descriptor,
				Console_callback, (void *)console)))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Console).  Unable to register callback for console.");
				DEALLOCATE(console);
				console=(struct Console *)NULL;
			}
#endif /* !defined(WIN32_USER_INTERFACE) */
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
#if !defined(WIN32_USER_INTERFACE)
		Event_dispatcher_remove_descriptor_callback(console->event_dispatcher,
			console->console_callback);
#endif
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

