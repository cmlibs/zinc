/*******************************************************************************
FILE : message.c

LAST MODIFIED : 5 November 2000

DESCRIPTION :
Declaration of functions for displaying messages.
???GMH.  Should all the printf('s be fprintf(stderr,'s?
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
static Display_message_function
	*display_error_message_function=(Display_message_function *)NULL,
	*display_information_message_function=(Display_message_function *)NULL,
	*display_warning_message_function=(Display_message_function *)NULL;
static void
	*display_error_message_data=(void *)NULL,
	*display_information_message_data=(void *)NULL,
	*display_warning_message_data=(void *)NULL;

#define MESSAGE_STRING_SIZE 1000
static char message_string[MESSAGE_STRING_SIZE];

/*
Global functions
----------------
*/
int set_display_message_function(enum Message_type message_type,
	Display_message_function *display_message_function,void *data)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
A function for setting the <display_message_function> to be used for displaying
a message of the specified <message_type>.
==============================================================================*/
{
	int return_code;

	ENTER(set_display_message_function);
	/* in case forget to set return_code */
	return_code=0;
	switch (message_type)
	{
		case ERROR_MESSAGE:
		{
			display_error_message_function=display_message_function;
			display_error_message_data=data;
			return_code=1;
		} break;
		case INFORMATION_MESSAGE:
		{
			display_information_message_function=display_message_function;
			display_information_message_data=data;
			return_code=1;
		} break;
		case WARNING_MESSAGE:
		{
			display_warning_message_function=display_message_function;
			display_warning_message_data=data;
			return_code=1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"set_display_message_function.  Unknown message_type");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* set_display_message_function */

int display_message(enum Message_type message_type,char *format, ... )
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
A function for displaying a message of the specified <message_type>.  The printf
form of arguments is used.
==============================================================================*/
{
	int return_code;
	va_list ap;

	ENTER(display_message);
	va_start(ap,format);
/*	return_code=vsnprintf(message_string,MESSAGE_STRING_SIZE,format,ap);*/
	return_code=vsprintf(message_string,format,ap);
	if (return_code >= (MESSAGE_STRING_SIZE-1))
	{
		char error_string[100];
		sprintf(error_string,"Overflow of message_string.  "
			"Following is truncated to %d characters:",return_code);
		if (display_error_message_function)
		{
			return_code=(*display_error_message_function)(error_string,
				display_error_message_data);
		}
		else
		{
			return_code=printf("ERROR: %s\n",error_string);
		}
	}
	switch (message_type)
	{
		case ERROR_MESSAGE:
		{
			if (display_error_message_function)
			{
				return_code=(*display_error_message_function)(message_string,
					display_error_message_data);
			}
			else
			{
				return_code=printf("ERROR: %s\n",message_string);
			}
		} break;
		case INFORMATION_MESSAGE:
		{
			if (display_information_message_function)
			{
				return_code=(*display_information_message_function)(message_string,
					display_information_message_data);
			}
			else
			{
				/* make sure we don't interpret % characters by printing the string */
				return_code=printf("%s",message_string);
			}
		} break;
		case WARNING_MESSAGE:
		{
			if (display_warning_message_function)
			{
				return_code=(*display_warning_message_function)(message_string,
					display_warning_message_data);
			}
			else
			{
				return_code=printf("WARNING: %s\n",message_string);
			}
		} break;
		default:
		{
			return_code=printf("UNKNOWN: %s\n",message_string);
		} break;
	}
	va_end(ap);
	LEAVE;

	return (return_code);
} /* display_message */
