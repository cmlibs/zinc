/*******************************************************************************
FILE : command_window.c

LAST MODIFIED : 11 May 2000

DESCRIPTION :
Management routines for the main command window.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <X11/Xmu/Xmu.h>
#include <X11/Xmu/WinUtil.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "command/command_window.h"
#include "command/command_window.uidh"
#include "command/command.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "help/help_interface.h"
#if defined (OLD_CODE)
#include "socket/socket.h"
#endif /* defined (OLD_CODE) */
#include "user_interface/filedir.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
#if !defined (WINDOWS_DEV_FLAG)
struct Menu_bar
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
The menu bar at the top of the command window
==============================================================================*/
{
	struct
	{
		struct
		{
			Widget open_comfile_button;
			Widget open_menu_button;
		} open_menu;
		Widget exit_button;
	} file_menu;
	struct
	{
		Widget cell_create_button;
		Widget data_viewer_create_button;
		Widget digitizer_3d_create_button;
		Widget element_creator_create_button;
		Widget g_element_editor_create_button;
		Widget image_processing_create_button;
		Widget im_control_create_button;
		Widget interactive_data_editor_create_button;
		Widget interactive_node_editor_create_button;
		Widget material_editor_create_button;
		Widget node_viewer_create_button;
		Widget spectrum_editor_create_button;
		Widget three_d_window_create_button;
		Widget time_editor_create_button;
		Widget tracking_editor_create_button;
		Widget unemap_create_button;
		Widget volume_editor_create_button;
	} windows_menu;
/*  struct Options_menu options_menu; */
/*  struct Help_menu help_menu; */
	Widget help_button;
}; /* struct Menu_bar */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Command_window
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	struct Menu_bar main_menu;
	Widget command_box;
	Widget command_history;
	Widget command_entry;
	XmString command_prompt;
	Widget message_pane;
	Widget output_pane;
	Widget window;
	Widget shell;
#else /* !defined (WINDOWS_DEV_FLAG) */
	HWND dialog;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	/* the information written to the command window can also be directed to a
		file */
	FILE *out_file;
	struct User_interface *user_interface;
	/* for executing commands */
	struct Execute_command *execute_command;
}; /* struct Command_window */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int command_window_hierarchy_open=0;
static MrmHierarchy command_window_hierarchy;

/* for allowing other applications to communicate with cmgui (issue commands) */
static Atom XA_CMGUI_VERSION=0;
static Atom XA_CMGUI_LOCK=0;
static Atom XA_CMGUI_COMMAND=0;
static Atom XA_CMGUI_RESPONSE=0;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if !defined (WINDOWS_DEV_FLAG)
static void identify_command_box(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 November 1994

DESCRIPTION :
Stores the id of the command areas.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(identify_command_box);
	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->command_box=widget;
		command_window->command_history=XmCommandGetChild(widget,
			XmDIALOG_HISTORY_LIST);
		command_window->command_entry=XmCommandGetChild(widget,
			XmDIALOG_COMMAND_TEXT);
		set_command_prompt((char *)NULL,command_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Command window structure missing");
	}
	LEAVE;
} /* identify_command_box */

static void identify_output_pane(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 November 1994

DESCRIPTION:
	Stores the id of the message areas.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(identify_output_pane);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->output_pane=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Command window structure missing");
	}
	LEAVE;
} /* identify_output_pane */

static void identify_message_pane(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 November 1994

DESCRIPTION:
	Stores the id of the message areas.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(identify_message_pane);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->message_pane=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Command window structure missing");
	}
	LEAVE;
} /* identify_message_pane */

static void id_three_d_window_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 1994

DESCRIPTION :
Stores the id of the command areas.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_three_d_window_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.three_d_window_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_three_d_window_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_three_d_window_create_button */

static void id_image_processing_create_butt(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 1994

DESCRIPTION:
Stores the id of the command areas.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_image_processing_create_butt);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.image_processing_create_button=
			widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_image_processing_create_butt.  Command window structure missing");
	}
	LEAVE;
} /* id_image_processing_create_butt */

static void id_unemap_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 December 1996

DESCRIPTION:
Stores the id of the unemap create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_unemap_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.unemap_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_unemap_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_unemap_create_button */

static void id_cell_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 April 1998

DESCRIPTION:
Stores the id of the cell create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_cell_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.cell_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_cell_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_cell_create_button */

static void id_time_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION:
Stores the id of the unemap create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_time_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.time_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_time_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_time_editor_create_butto */

static void id_tracking_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION:
Stores the id of the unemap create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_tracking_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.tracking_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_tracking_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_tracking_editor_create_butto */

static void id_volume_editor_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 1994

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_volume_editor_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.volume_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_volume_editor_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_volume_editor_create_button */

static void id_element_creator_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_element_creator_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.element_creator_create_button
			=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_element_creator_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_element_creator_create_butto */

static void id_g_element_editor_create_butt(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 August 1997

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_g_element_editor_create_butt);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.g_element_editor_create_button
			=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_g_element_editor_create_butt.  Command window structure missing");
	}
	LEAVE;
} /* id_g_element_editor_create_butt */

static void id_material_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1994

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_material_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.material_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_material_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_material_editor_create_butto */

static void id_interactive_node_editor_cbut(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_interactive_node_editor_cbut);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.
			interactive_node_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_interactive_node_editor_cbut.  Command window structure missing");
	}
	LEAVE;
} /* id_interactive_node_editor_cbut */

static void id_interactive_data_editor_cbut(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 November 1998

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_interactive_data_editor_cbut);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.
			interactive_data_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_interactive_data_editor_cbut.  Command window structure missing");
	}
	LEAVE;
} /* id_interactive_data_editor_cbut */

static void id_node_viewer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_node_viewer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.node_viewer_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_node_viewer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_node_viewer_create_button */

static void id_data_viewer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1997

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_data_viewer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.data_viewer_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_data_viewer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_data_viewer_create_button */

static void id_spectrum_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1994

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_spectrum_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.spectrum_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_spectrum_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_spectrum_editor_create_butto */

static void id_3d_digitizer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 April 1995

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_3d_digitizer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.digitizer_3d_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_3d_digitizer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_3d_digitizer_create_button */

static void id_im_control_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 April 1995

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_im_control_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.windows_menu.im_control_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_im_control_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_im_control_create_button */

static void command_changed(Widget widget,XtPointer command_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Called when a command is changed in the command entry area.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(command_changed);
 	USE_PARAMETER(widget);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)command_window_structure)
	{
		XmProcessTraversal(command_window->command_entry,XmTRAVERSE_CURRENT);
	}
	else
	{
		display_message(ERROR_MESSAGE,"command_changed.  Missing command window");
	}
	LEAVE;
} /* command_changed */

static void command_entered(Widget widget,XtPointer command_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 June 1993

DESCRIPTION :
Called when a command is entered in the command entry area.
==============================================================================*/
{
	char *command;
	struct Command_window *command_window;
	XmCommandCallbackStruct *command_callback;

	ENTER(command_entered);
 	USE_PARAMETER(widget);
	if ((command_window=(struct Command_window *)command_window_structure)&&
		(command_callback=(XmCommandCallbackStruct *)call_data))
	{
		if (True==XmStringGetLtoR(command_callback->value,XmSTRING_DEFAULT_CHARSET,
			&command))
		{
			/*???DB.  execute_command adds the command to the history list, but the
				XmCommand widget does this as well - so have to delete */
			/*???DB.  Short term solution ?  Stop using XmCommand ? */
			/* delete last element */
			XmListDeletePos(command_window->command_history,0);
			Execute_command_execute_string(command_window->execute_command,
				command);
/*      reset_command_box(command_window);*/
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"command_entered.  Error retrieving command");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"command_entered.  Invalid argument(s)");
	}
	LEAVE;
} /* command_entered */

static void command_window_close(Widget widget,
	XtPointer command_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 January 1998

DESCRIPTION :
Called when the Close function is selected from the window manager menu.
???DB.  The closing of graphics windows has been moved to execute_command_quit
	Ideally the command window should be able toe execute commands, but know
	nothing about the details of the commands.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(command_window_close);
 	USE_PARAMETER(widget);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)command_window_structure)
	{
		Execute_command_execute_string(command_window->execute_command, "QUIT");
#if !defined (WINDOWS_DEV_FLAG)
		/*???DB.  To allow restarting of back end */
#if !defined (NO_HELP)
		destroy_help();
#endif /* !defined (NO_HELP) */
#if defined (OLD_CODE)
		close_socket();
#endif /* defined (OLD_CODE) */
#endif /* !defined (WINDOWS_DEV_FLAG) */
		close_user_interface(command_window->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"command_window_close.  Missing command window");
	}
	LEAVE;
} /* command_window_close */

static void destroy_Command_window_callback(Widget widget,
	XtPointer command_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Destroy the command_window structure and remove the window
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(destroy_Command_window_callback);
 	USE_PARAMETER(widget);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)command_window_structure)
	{
		DESTROY(Command_window)(&command_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Command_window_callback.  Missing command window");
	}
	LEAVE;
} /* destroy_Command_window_callback */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (WINDOWS)
static LRESULT CALLBACK Command_window_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 25 October 1996

DESCRIPTION:
==============================================================================*/
{
	LRESULT return_code;

	ENTER(Command_window_class_proc);
	return_code=FALSE;
	switch (message_identifier)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		default:
		{
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Command_window_class_proc */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
static void Command_window_WM_COMMAND_handler(HWND window,
	int item_control_accelerator_id,HWND control_window,UINT notify_code)
/*******************************************************************************
LAST MODIFIED : 27 October 1996

DESCRIPTION:
==============================================================================*/
{
	ENTER(Command_window_WM_COMMAND_handler);
	switch (item_control_accelerator_id)
	{
		case IDCANCEL:
		{
			DestroyWindow(window);
		} break;
	}
	LEAVE;
} /* Command_window_WM_COMMAND_handler */

static BOOL CALLBACK Command_window_dialog_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 27 October 1996

DESCRIPTION:
==============================================================================*/
{
	BOOL return_code;

	ENTER(Command_window_dialog_proc);
	return_code=FALSE;
	switch (message_identifier)
	{
		case WM_COMMAND:
		{
			return_code=(BOOL)HANDLE_WM_COMMAND(window,first_message,second_message,
				Command_window_WM_COMMAND_handler);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Command_window_dialog_proc */
#endif /* defined (WINDOWS) */

#if !defined (WINDOWS_DEV_FLAG)
void change_scrolling(Widget widget,XtPointer command_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Change from scrolling to not and back.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(change_scrolling);
 	USE_PARAMETER(widget);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)command_window_structure)
	{
		USE_PARAMETER(command_window);
		display_message(WARNING_MESSAGE,"change_scrolling.	Not implemented");
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_scrolling.	Missing command_window");
	}
	LEAVE;
} /* change_scrolling */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (MOTIF)
static int command_window_property_notify_callback(XPropertyEvent *event,
	void *command_window_void,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION:
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	Atom property_type;
	char *command_string,*example_string,*temp_char;
	Display *display;
	int count,data_unit,result,return_code;
	struct Command_window *command_window;
	unsigned long bytes_left,number_of_units;

	ENTER(command_window_property_notify_callback);
	return_code=0;
	/* check arguments */
	if (event&&(command_window=(struct Command_window *)command_window_void)&&
		user_interface)
	{
		return_code=1;
		if ((PropertyNotify==event->type)&&(PropertyNewValue==event->state)&&
			(XtWindow(command_window->shell)==event->window))
		{
			if (XA_CMGUI_VERSION==event->atom)
			{
#if defined (DEBUG)
				/*???debug */
				printf("XA_CMGUI_VERSION changed\n");
#endif /* defined (DEBUG) */
			}
			else
			{
				if (XA_CMGUI_LOCK==event->atom)
				{
#if defined (DEBUG)
					/*???debug */
					printf("XA_CMGUI_LOCK changed\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					if (XA_CMGUI_COMMAND==event->atom)
					{
#if defined (DEBUG)
						/*???debug */
						printf("XA_CMGUI_COMMAND changed\n");
#endif /* defined (DEBUG) */
						if (display=XtDisplay(command_window->shell))
						{
							example_string=(char *)NULL;
							result=XGetWindowProperty(display,XtWindow(command_window->shell),
								XA_CMGUI_COMMAND,0,(65536/sizeof(long)),True,XA_STRING,
								&property_type,&data_unit,&number_of_units,&bytes_left,
								(unsigned char **)&example_string);
							if ((Success==result)&&example_string&&(*example_string))
							{
#if defined (DEBUG)
								/*???debug */
								printf("example_string %s\n",(char *)example_string);
#endif /* defined (DEBUG) */
								if (!strncmp(example_string,"example:",8))
								{
									if (temp_char=strchr(example_string,'_'))
									{
										temp_char++;
									}
									else
									{
										temp_char=example_string+8;
									}
									count=0;
									while ((*(temp_char+count))&&('?'!= *(temp_char+count)))
									{
										count++;
									}
									if (count>0)
									{
										if (ALLOCATE(command_string,char,
											count+strlen(CMGUI_EXAMPLE_DIRECTORY_SYMBOL)+20))
										{
											strcpy(command_string,"open comfile ");
											strcat(command_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
											strcat(command_string," name ");
											*(command_string+(strlen(command_string)+count))='\0';
											strncpy(command_string+strlen(command_string),temp_char,
												count);
											Execute_command_execute_string(command_window->execute_command,
												command_string);
											DEALLOCATE(command_string);
										}
										else
										{
											display_message(ERROR_MESSAGE,
							"command_window_property_notify_callback.  Insufficient memory");
										}
									}
								}
								else
								{
									/* assume we have a command */
									Execute_command_execute_string(command_window->execute_command,
										example_string);
								}
								XChangeProperty(display,XtWindow(command_window->shell),
									XA_CMGUI_RESPONSE,XA_STRING,8,PropModeReplace,
									(unsigned char *)"received",8);
							}
							if (example_string)
							{
								XFree(example_string);
							}
						}
					}
					else
					{
						if (XA_CMGUI_RESPONSE==event->atom)
						{
/*???debug */
printf("XA_CMGUI_RESPONSE changed\n");
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"command_window_property_notify_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* command_window_property_notify_callback */
#endif /* defined (MOTIF) */

static int modify_Command_window_out_file_open(struct Parse_state *state,
	void *dummy,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 11 November 1998

DESCRIPTION :
==============================================================================*/
{
	char *file_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_file_name},
	};
	struct Command_window *command_window;

	ENTER(modify_Command_window_out_file_open);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		file_name=(char *)NULL;
		option_table[0].to_be_modified= &file_name;
		return_code=process_option(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if ((command_window=(struct Command_window *)command_window_void)
				&&file_name)
			{
				if (command_window->out_file)
				{
					display_message(WARNING_MESSAGE,"Closing existing file");
					fclose(command_window->out_file);
					command_window->out_file=(FILE *)NULL;
				}
				if (!(command_window->out_file=fopen(file_name,"w")))
				{
					display_message(ERROR_MESSAGE,"Could not open %s",file_name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing command_window or file name");
				return_code=0;
			}
		} /* parse error, help */
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Command_window_out_file_open.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Command_window_out_file_open */

static int modify_Command_window_out_file_close(struct Parse_state *state,
	void *dummy,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 11 November 1998

DESCRIPTION :
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Command_window *command_window;

	ENTER(modify_Command_window_out_file_close);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option(s)");
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_window=(struct Command_window *)command_window_void)
			{
				if (command_window->out_file)
				{
					fclose(command_window->out_file);
					command_window->out_file=(FILE *)NULL;
				}
				else
				{
					display_message(WARNING_MESSAGE,"No file open");
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"modify_Command_window_out_file_close.  Missing command_window");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Command_window_out_file_close.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Command_window_out_file_close */

static int modify_Command_window_out_file(struct Parse_state *state,void *dummy,
	void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 11 November 1998

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"close",NULL,NULL,modify_Command_window_out_file_close},
		{"open",NULL,NULL,modify_Command_window_out_file_open},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(modify_Command_window_out_file);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		i=0;
		/* close */
		option_table[i].user_data=command_window_void;
		i++;
		/* open */
		option_table[i].user_data=command_window_void;
		i++;
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Command_window_out_file.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Command_window_out_file */

/*
Global functions
----------------
*/
struct Command_window *CREATE(Command_window)(
	struct Execute_command *execute_command,struct User_interface *user_interface,
	char *version_id_string)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION:
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
#if defined (MOTIF)
	Atom WM_DELETE_WINDOW;
	long event_mask;
	MrmType command_window_class;
	static MrmRegisterArg callback_list[]={
		{"command_changed",(XtPointer)command_changed},
		{"command_entered",(XtPointer)command_entered},
		{"identify_command_box",(XtPointer)identify_command_box},
		{"identify_output_pane",(XtPointer)identify_output_pane},
		{"identify_message_pane",(XtPointer)identify_message_pane},
		{"id_three_d_window_create_button",
			(XtPointer)id_three_d_window_create_button},
		{"id_image_processing_create_butt",
			(XtPointer)id_image_processing_create_butt},
		{"id_unemap_create_button",(XtPointer)id_unemap_create_button},
		{"id_cell_create_button",(XtPointer)id_cell_create_button},
		{"id_element_creator_create_butto",
			(XtPointer)id_element_creator_create_butto},
		{"id_time_editor_create_butto",
			(XtPointer)id_time_editor_create_butto},
		{"id_tracking_editor_create_butto",
			(XtPointer)id_tracking_editor_create_butto},
		{"id_volume_editor_create_button",
			(XtPointer)id_volume_editor_create_button},
		{"id_g_element_editor_create_butt",
			(XtPointer)id_g_element_editor_create_butt},
		{"id_material_editor_create_butto",
			(XtPointer)id_material_editor_create_butto},
		{"id_spectrum_editor_create_butto",
			(XtPointer)id_spectrum_editor_create_butto},
		{"id_interactive_node_editor_cbut",
			(XtPointer)id_interactive_node_editor_cbut},
		{"id_node_viewer_create_button",(XtPointer)id_node_viewer_create_button},
		{"id_data_viewer_create_button",(XtPointer)id_data_viewer_create_button},
		{"id_interactive_data_editor_cbut",
			(XtPointer)id_interactive_data_editor_cbut},
		{"id_3d_digitizer_create_button",(XtPointer)id_3d_digitizer_create_button},
		{"id_im_control_create_button",(XtPointer)id_im_control_create_button},
		{"callback_command",(XtPointer)callback_command},
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"change_scrolling",(XtPointer)change_scrolling},
		{"command_window_close",(XtPointer)command_window_close}
	};
	static MrmRegisterArg identifier_list[3];
	XWindowAttributes window_attributes;
#endif /* defined (MOTIF) */
	struct Command_window *command_window;
#if defined (MOTIF)
	struct File_open_data *iod_file_open_data;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	BOOL win32_return_code;
	static char *class_name="Command_window";
	WNDCLASSEX class_information;
#endif /* defined (WINDOWS) */

	ENTER(CREATE(Command_window));
	/* check arguments */
	if (execute_command&&user_interface)
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(command_window_uidh,
			&command_window_hierarchy,&command_window_hierarchy_open))
		{
#endif /* defined (MOTIF) */
			if (ALLOCATE(command_window,struct Command_window,1))
			{
				command_window->user_interface=user_interface;
				command_window->execute_command=execute_command;
				command_window->out_file=(FILE *)NULL;
#if defined (MOTIF)
				command_window->main_menu.file_menu.open_menu.open_comfile_button=
					(Widget)NULL;
				command_window->main_menu.file_menu.open_menu.open_menu_button=
					(Widget)NULL;
				command_window->main_menu.file_menu.exit_button=(Widget)NULL;
				command_window->main_menu.windows_menu.cell_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.data_viewer_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.digitizer_3d_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.element_creator_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.g_element_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.image_processing_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.im_control_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.
					interactive_data_editor_create_button=(Widget)NULL;
				command_window->main_menu.windows_menu.
					interactive_node_editor_create_button=(Widget)NULL;
				command_window->main_menu.windows_menu.material_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.node_viewer_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.spectrum_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.three_d_window_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.time_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.tracking_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.unemap_create_button=
					(Widget)NULL;
				command_window->main_menu.windows_menu.volume_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.help_button=(Widget)NULL;
				command_window->command_box=(Widget)NULL;
				command_window->command_history=(Widget)NULL;
				command_window->command_entry=(Widget)NULL;
				command_window->command_prompt=XmStringCreateSimple("");
				command_window->output_pane=(Widget)NULL;
				command_window->message_pane=(Widget)NULL;
				command_window->window=(Widget)NULL;
				command_window->shell=(Widget)NULL;
				/* create the command window shell */
				if (command_window->shell=XtVaCreatePopupShell("command_window_shell",
					xmDialogShellWidgetClass,user_interface->application_shell,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
/*          XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions,MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,*/
					XmNtransient,FALSE,
					NULL))
				{
					/* Identify the shell for the busy icon */
					create_Shell_list_item(
						&(command_window->shell),user_interface);
					/* Add destroy callback */
					XtAddCallback(command_window->shell,XmNdestroyCallback,
						destroy_Command_window_callback,(XtPointer)command_window);
					WM_DELETE_WINDOW=XmInternAtom(XtDisplay(command_window->shell),
						"WM_DELETE_WINDOW",FALSE);
					XmAddWMProtocolCallback(command_window->shell,WM_DELETE_WINDOW,
						command_window_close,(XtPointer)command_window);
					/* Register callbacks in UIL */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(command_window_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						identifier_list[0].name="command_window_structure";
						identifier_list[0].value=(XtPointer)command_window;
						identifier_list[1].name="execute_command_structure";
						identifier_list[1].value=(XtPointer)execute_command;
#if defined (MOTIF)
						if (iod_file_open_data=create_File_open_data(".iod",REGULAR,
							read_iod_file_via_selection_box,(XtPointer)NULL,0,user_interface))
						{
							identifier_list[2].name="read_iod_file_structure";
							identifier_list[2].value=(XtPointer)iod_file_open_data;
						}
						else
#endif /* defined (MOTIF) */
						{
							identifier_list[2].name="read_iod_file_structure";
							identifier_list[2].value=(XtPointer)NULL;
						}
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							command_window_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							if (MrmSUCCESS==MrmFetchWidget(command_window_hierarchy,
								"command_window",command_window->shell,
								&(command_window->window),&command_window_class))
							{
								/*???DB.  Get rid of image processing and unemap if not being
									used ? */
								XtManageChild(command_window->window);
								XtRealizeWidget(command_window->shell);
								/* set up for communication with other applications */
								XA_CMGUI_VERSION=XInternAtom(user_interface->display,
									CMGUI_VERSION_PROPERTY,False);
								XA_CMGUI_LOCK=XInternAtom(user_interface->display,
									CMGUI_LOCK_PROPERTY,False);
								XA_CMGUI_COMMAND=XInternAtom(user_interface->display,
									CMGUI_COMMAND_PROPERTY,False);
								XA_CMGUI_RESPONSE=XInternAtom(user_interface->display,
									CMGUI_RESPONSE_PROPERTY,False);
								XChangeProperty(user_interface->display,
									XtWindow(command_window->shell),XA_CMGUI_VERSION,XA_STRING,8,
									PropModeReplace,(unsigned char *)version_id_string,
									strlen(version_id_string));
								XGetWindowAttributes(user_interface->display,
									XtWindow(command_window->shell),&window_attributes);
								event_mask=(window_attributes.your_event_mask)|
									PropertyChangeMask;
								XSelectInput(user_interface->display,
									XtWindow(command_window->shell), event_mask);
								set_property_notify_callback(user_interface,
									command_window_property_notify_callback,
									(void *)command_window);
								XtPopup(command_window->shell,XtGrabNone);
								/*???DB.  If I don't have this call to XmuClientWindow, then
									sometimes cmgui doesn't pick up any of the properties set by
									Netscape.  It is some sort of timing problem.  XmuClientWindow
									finds a window at or below the specified one which has the
									WM_STATE property set */
								XmuClientWindow(user_interface->display,
									XtWindow(command_window->shell));
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Command_window).  Could not retrieve widget");
								DEALLOCATE(command_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Command_window).  Could not register identifiers");
							DEALLOCATE(command_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Command_window).  Could not register callbacks");
						DEALLOCATE(command_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Command_window).  Could not create shell");
					DEALLOCATE(command_window);
				}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				/* check if the class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(user_interface->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=
						DLGWINDOWEXTRA+sizeof(struct Command_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=LoadIcon(user_interface->instance,class_name);
					class_information.hInstance=user_interface->instance;
					class_information.lpfnWndProc=Command_window_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=LoadIcon(user_interface->instance,
						"Command_window" "_small");
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					if (command_window->dialog=CreateDialogParam(user_interface->instance,
						"Command_window",(HWND)NULL,Command_window_dialog_proc,
						(LPARAM)NULL))
					{
						ShowWindow(command_window->dialog,SW_SHOW);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Command_window).  Could not create dialog");
						DEALLOCATE(command_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Command_window).  Unable to register class information");
					DEALLOCATE(command_window);
				}
#endif /* defined (WINDOWS) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Command_window).  Insufficient memory for command_window");
			}
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Command_window).  Could not open hierarchy");
			command_window=(struct Command_window *)NULL;
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Command_window).  Invalid argument(s)");
		command_window=(struct Command_window *)NULL;
	}
	LEAVE;

	return (command_window);
} /* CREATE(Command_window) */

int DESTROY(Command_window)(struct Command_window **command_window_pointer)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION:
==============================================================================*/
{
	int return_code;
	struct Command_window *command_window;

	if (command_window_pointer && (command_window = (struct Command_window *)
		command_window_pointer))
	{
		if (command_window->out_file)
		{
			fclose(command_window->out_file);
		}
		destroy_Shell_list_item_from_shell(&(command_window->shell),
			command_window->user_interface);
		DEALLOCATE(*command_window_pointer);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Command_window).  Missing command window");
		return_code = 0;
	}

	return (return_code);
} /* DESTROY(Command_window) */

#if !defined (WINDOWS_DEV_FLAG)
int add_to_command_list(char *command,struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/
{
	int max_commands,num_commands,return_code;
	XmString new_command;

	ENTER(add_to_command_list);
/*???debug */
/*printf("enter add_to_command_list\n  %s\n",command);*/
	if (command_window)
	{
#if defined (MOTIF)
		/* create XmString of the command */
		new_command=XmStringCreateSimple(command);
		/* get the number of items and the maximum number to make sure that we don't
			overflow the list */
		XtVaGetValues(command_window->command_box,
			XmNhistoryItemCount,&num_commands,
			XmNhistoryMaxItems,&max_commands,
			NULL);
		if (num_commands==max_commands)
		{
			/* delete first element */
			XmListDeletePos(command_window->command_history,1);
		}
		/* add new command */
		XmListAddItem(command_window->command_history,new_command,0);
		XmStringFree(new_command);
		/* show last command */
		XmListSetBottomPos(command_window->command_history,0);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_to_command_list.  Missing command window");
		return_code=0;
	}
/*???debug */
/*printf("leave add_to_command_list\n");*/
	LEAVE;

	return (return_code);
} /* add_to_command_list */

int set_command_prompt(char *prompt,struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 2 August 1998

DESCRIPTION :
Sets the value of the <prompt> for the <command_window>.
==============================================================================*/
{
	char *temp;
	int return_code;

	ENTER(set_command_prompt);
	if (command_window)
	{
		if (command_window->command_prompt)
		{
			XmStringFree(command_window->command_prompt);
		}
		if (prompt&&(*prompt))
		{
			if (ALLOCATE(temp,char,strlen(prompt)+2))
			{
				sprintf(temp,"%s ",prompt);
				command_window->command_prompt=XmStringCreateSimple(temp);
				DEALLOCATE(temp);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_command_prompt.  Could not allocate prompt string");
			}
		}
		else
		{
			command_window->command_prompt=XmStringCreateSimple("");
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

int reset_command_box(struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Resets all functions of the command box widget.
==============================================================================*/
{
	int return_code;

	ENTER(reset_command_box);
	if (command_window)
	{
#if defined (MOTIF)
		/* Blank line is used so that arrows work properly */
		/* delete old blank line */
		if (XmListItemExists(command_window->command_history,
			XmStringCreateSimple("")))
		{
			XmListDeleteItem(command_window->command_history,
				XmStringCreateSimple(""));
		}
		/* add in new blank line at the bottom */
		XmListAddItem(command_window->command_history,
			XmStringCreateSimple(""),0);
		XmListDeselectAllItems(command_window->command_history);
		XmListSelectPos(command_window->command_history,0,True);
		/* Set default prompt */
		XmCommandSetValue(command_window->command_box,XmStringCreateSimple(""));
		if (!XmStringEmpty(command_window->command_prompt))
		{
			XmCommandAppendValue(command_window->command_box,
				command_window->command_prompt);
		}
#endif /* defined (MOTIF) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"reset_command_box.  Missing command window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* reset_command_box */

int Command_window_set_command_string(struct Command_window *command_window,
	char *command_string)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Fills the command entry area of the command window with <command_string>, ready
to be edited and entered. Used eg. by the comfile window to place a command in
responce to a single mouse click on it.
Does not override the command prompt.
==============================================================================*/
{
	int return_code;

	ENTER(Command_window_set_command_string);
	if (command_window&&command_string)
	{
		XtVaSetValues(command_window->command_entry,
			XmNvalue,(XtPointer)command_string,NULL);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Command_window_set_command_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Command_window_set_command_string */

int write_command_window(char *message,struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/
{
	int return_code;
	Widget output_pane;
	XmTextPosition text_pos;

	ENTER(write_command_window);
	return_code=0;
	if (command_window)
	{
		if (output_pane=command_window->output_pane)
		{
#if defined (MOTIF)
			text_pos=XmTextGetLastPosition(output_pane);
			XmTextInsert(output_pane,text_pos,message);
			text_pos=XmTextGetLastPosition(output_pane);
			XmTextShowPosition(output_pane,text_pos);
			return_code=1;
#endif /* defined (MOTIF) */
		}
		if (command_window->out_file)
		{
			fprintf(command_window->out_file,message);
			return_code=1;
		}
	}
	if (!return_code)
	{
		/* window does not exist */
		printf("%s",message);
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* write_command_window */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int modify_Command_window(struct Parse_state *state,void *dummy,
	void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 11 November 1998

DESCRIPTION :
Modifys the <command_window_void> according to the command in the <state>.
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"out_file",NULL,NULL,modify_Command_window_out_file},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(modify_Command_window);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		i=0;
		/* out_file */
		option_table[i].user_data=command_window_void;
		i++;
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"modify_Command_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Command_window_out_file */
