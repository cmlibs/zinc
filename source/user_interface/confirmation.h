/*******************************************************************************
FILE : confirmation.h

LAST MODIFIED : 7 July 1999

DESCRIPTION :
Routines for waiting for user input.
==============================================================================*/
#if !defined (CONFIRMATION_H)
#define CONFIRMATION_H

#include "user_interface/user_interface.h"

/*
Global types
------------
*/
#if defined (MOTIF)
typedef int (*Confirmation_add_widgets_function)(Widget parent,void *user_data);
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
int confirmation_warning_ok_cancel(char *title,char *prompt,
#if defined (MOTIF)
	Widget parent,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns 1 if the OK button
is clicked and 0 if the cancel button is clicked.
==============================================================================*/

#if defined (MOTIF)
int confirmation_warning_ok_cancel_plus_options(char *title,char *prompt,
	Widget parent,Confirmation_add_widgets_function add_widgets,
	void *add_widgets_user_data,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns 1 if the OK button
is clicked and 0 if the cancel button is clicked.
==============================================================================*/
#endif /* defined (MOTIF) */

int confirmation_error_ok(char *title,char *prompt,
#if defined (MOTIF)
	Widget parent,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a error dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_information_ok(char *title,char *prompt,
#if defined (MOTIF)
	Widget parent,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a information dialog window which requires a response
before anything else will continue and returns as the OK button is clicked.  No
other options are supplied.
==============================================================================*/

int confirmation_warning_ok(char *title,char *prompt,
#if defined (MOTIF)
	Widget parent,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a warning dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_question_yes_no(char *title,char *prompt,
#if defined (MOTIF)
	Widget parent,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue.  It returns one if the Yes button
is clicked and No if it isn't.
==============================================================================*/

char *confirmation_get_read_filename(char *extension,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_get_write_filename(char *extension,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_change_current_working_directory(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
This routine supplies a file selection dialog window for changing the current
working directory.  The new directory will be created if necessary.
==============================================================================*/

#if defined (MOTIF)
char *confirmation_get_string(char *title,char *prompt,char *default_string,
	Widget parent,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns a char * pointer
if the OK button is clicked and NULL if the cancel button is clicked.
The string <default_string> is supplied as the initial text, the box
is initially blank if <default_string> is NULL.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (CONFIRMATION_H) */
