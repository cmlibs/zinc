/*******************************************************************************
FILE : message.h

LAST MODIFIED : 11 June 1999

DESCRIPTION :
Global types and function prototypes for displaying messages.
==============================================================================*/
#if !defined (MESSAGE_H)
#define MESSAGE_H
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

typedef int (Display_message_function)(char *,void *);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
The type of a function for displaying message strings.
==============================================================================*/

/*
Global functions
----------------
*/
int set_display_message_function(enum Message_type message_type,
	Display_message_function *display_message_function,void *data);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
A function for setting the <display_message_function> to be used for displaying
a message of the specified <message_type>.
==============================================================================*/

int display_message(enum Message_type message_type,char *format, ... );
/*******************************************************************************
LAST MODIFIED : 31 May 1996

DESCRIPTION :
A function for displaying a message of the specified <message_type>.  The printf
form of arguments is used.
==============================================================================*/
#endif /* !defined (MESSAGE_H) */
