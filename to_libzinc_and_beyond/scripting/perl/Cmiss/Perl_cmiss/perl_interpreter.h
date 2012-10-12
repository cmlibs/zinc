/*******************************************************************************
FILE : perl_interpreter.h

LAST MODIFIED : 12 June 2003

DESCRIPTION :
Provides an interface between cmiss and a Perl interpreter.
==============================================================================*/

typedef void (*execute_command_function_type)(char *, void *, int *, int *);

void set_interpreter(PerlInterpreter *perl_in);
/*******************************************************************************
LAST MODIFIED : 24 July 2001

DESCRIPTION:
Allows an external perl to set itself as the one used by the interpreter functions.
==============================================================================*/

#if ! defined (MESSAGE_H)
/*
From message.h:
===============
*/

/*
Global types
------------
*/

enum Message_type
/*******************************************************************************
LAST MODIFIED : 31 May 1996

DESCRIPTION :
The different message types.
==============================================================================*/
{
	ERROR_MESSAGE,
	INFORMATION_MESSAGE,
	WARNING_MESSAGE
}; /* enum Message_type */
#endif /* ! defined (MESSAGE_H) */

typedef int (Interpreter_display_message_function)(enum Message_type message_type,
	char *format, ... );

