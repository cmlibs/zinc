/*******************************************************************************
FILE : track.c

LAST MODIFIED : 14 February 1998

DESCRIPTION :
Tracking editor main.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include "version.h"
#include "general/debug.h"
#include "mirage/tracking_editor_dialog.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 3 February 1998

DESCRIPTION :
==============================================================================*/
{
	char *uid_directory;
} User_settings;

/*
Module functions
----------------
*/
static void exit_track(Widget widget,XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Exits track
==============================================================================*/
{
	exit(0);
} /* exit_track */

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Main program for the tracking editor application
==============================================================================*/
{
	int return_code;
	static MrmRegisterArg identifiers[]=
	{
		{"widget_spacing",(XtPointer)NULL},
	};
#define XmNuidDirectory "uidDirectory"
#define XmCUidDirectory "UidDirectory"
	static XtResource resources[]=
	{
		{
			XmNuidDirectory,
			XmCUidDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,uid_directory),
			XmRString,
			""
		}
	};
	User_settings user_settings;
	struct User_interface user_interface;

	ENTER(main);
	return_code=1;
	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");
	/* open the user interface */
	user_interface.application_context=(XtAppContext) NULL;
	user_interface.application_name="track";
	user_interface.application_shell=(Widget) NULL;
	user_interface.argc_address= &argc;
	user_interface.argv=argv;
	user_interface.class_name="Track";
	user_interface.display=(Display *) NULL;
	if (open_user_interface(&user_interface))
	{
		identifiers[0].value=(XtPointer)5;
		/* register the identifiers in the global name table */
		if (MrmSUCCESS==MrmRegisterNames(identifiers,XtNumber(identifiers)))
		{
			/* retrieve application specific constants */
			XtVaGetApplicationResources(user_interface.application_shell,
				&user_settings,resources,XtNumber(resources),NULL);
			/* create the main window */
			if (open_tracking_editor_dialog((struct Tracking_editor_dialog **)NULL,
				exit_track,&user_interface))
			{
				/* user interface loop */
				return_code=application_main_loop(&user_interface);
				/* close the user interface */
				close_user_interface(&user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Unable to create dialog");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Unable to register identifiers");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open user interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
