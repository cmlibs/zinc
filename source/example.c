/*******************************************************************************
FILE : example.c

LAST MODIFIED : 24 November 1997

DESCRIPTION :
Example showing how to write a "tool" for cmgui.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include "general/debug.h"
#include "graphics/volume_texture_editor_dialog.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
==============================================================================*/
{
	char *uid_directory;
} User_settings;

/*
Main program
------------
*/
#if defined (MOTIF)
int main(int argc,char *argv[])
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
#endif /* defined (WINDOWS) */
/*******************************************************************************
LAST MODIFIED : 24 October 1996

DESCRIPTION :
Main program for the CMISS Graphical User Interface
==============================================================================*/
{
	int
#if defined (WINDOWS)
		WINAPI
#endif /* defined (WINDOWS) */
		return_code;
#if defined (MOTIF)
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
#endif /* defined (MOTIF) */
	struct VT_volume_texture volume_texture;
	User_settings user_settings;
	struct User_interface user_interface;

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	ENTER(WinMain);
#endif /* defined (WINDOWS) */
	/* open the user interface */
#if defined (MOTIF)
	user_interface.application_context=(XtAppContext)NULL;
	user_interface.application_name="cmgui";
	user_interface.application_shell=(Widget)NULL;
	user_interface.argc_address= &argc;
	user_interface.argv=argv;
	user_interface.class_name="Cmgui";
	user_interface.display=(Display *)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	user_interface.instance=current_instance;
	user_interface.main_window=(HWND)NULL;
	user_interface.main_window_state=initial_main_window_state;
	user_interface.command_line=command_line;
#endif /* defined (WINDOWS) */
	if (open_user_interface(&user_interface))
	{
#if defined (MOTIF)
		identifiers[0].value=(XtPointer)5;
		/* register the identifiers in the global name table*/
		if (MrmSUCCESS==MrmRegisterNames(identifiers,XtNumber(identifiers)))
		{
			/* retrieve application specific constants */
			XtVaGetApplicationResources(user_interface.application_shell,
				&user_settings,resources,XtNumber(resources),NULL);
#endif /* defined (MOTIF) */
			/* create the main window */
			if (open_create_finite_elements_dialog(
				(struct Create_finite_elements_dialog **)NULL,&volume_texture,
				&user_interface))
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
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,"Unable to register identifiers");
			return_code=0;
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open user interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* main */
