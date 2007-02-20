/*******************************************************************************
FILE : command_window.c

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
extern "C" {
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
#if defined (MOTIF)
static char command_window_uidh[] =
#include "command/command_window.uidh"
	;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
#include "command/command_window.rc"
#endif /* defined (WIN32_USER_INTERFACE) */
#include "command/command.h"
#if defined (MOTIF)
#include "help/help_interface.h"
#include "user_interface/filedir.h"
#endif /* defined (MOTIF) */
}
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "command/command_window.xrch"
#endif /* defined (WX_USER_INTERFACE)*/
extern "C" {
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}

/*
Module types
------------
*/
#if defined (MOTIF)
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
		Widget curve_editor_create_button;
		Widget data_viewer_create_button;
		Widget interactive_data_editor_create_button;
		Widget element_creator_create_button;
		Widget element_point_viewer_button;
		Widget grid_field_calculator_button;
		Widget im_control_create_button;
		Widget node_viewer_create_button;
		Widget interactive_node_editor_create_button;
		Widget tracking_editor_create_button;
		Widget unemap_create_button;
		Widget digitizer_3d_create_button;
	} model_menu;
	struct
	{
		Widget three_d_window_create_button;
		Widget material_editor_create_button;
		Widget scene_editor_create_button;
		Widget spectrum_editor_create_button;
		Widget time_editor_create_button;
		Widget volume_editor_create_button;
	} graphics_menu;
	Widget help_button;
}; /* struct Menu_bar */
#endif /* defined (MOTIF) */

enum Command_window_outfile_mode
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Controls what is written to the log file.
Must ensure OUTFILE_OUTPUT_AND_INPUT = OUTFILE_OUTPUT & OUTFILE_INPUT,
so that the bits can operate as independent flags.
==============================================================================*/
{
	OUTFILE_INVALID = 0,
	OUTFILE_OUTPUT = 1,
	OUTFILE_INPUT = 2
}; /* enum Command_window_outfile_mode */

#if defined (WX_USER_INTERFACE)
class wxCommandWindow;
#endif

struct Command_window
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF) /* switch (USER_INTERFACE) */
	struct Menu_bar main_menu;
	Widget command_box;
	Widget command_history;
	Widget command_entry;
	XmString command_prompt;
	Widget message_pane;
	Widget output_pane;
	Widget window;
	Widget shell;
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
	char *command_prompt;
	HWND dialog;
	HWND command_history;
	HWND command_entry;
	HWND command_output_pane;
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
	char *command_prompt;
	GtkWidget *entry;
	GtkWidget *history_view;
	GtkWidget *output_view;
	GtkWidget *shell;
#if GTK_MAJOR_VERSION >= 2
	GtkTextBuffer *history_buffer;
	GtkTextBuffer *output_buffer;
	GtkTextMark *history_end;
	GtkTextMark *output_end;
	gulong close_handler_id;
#else /* GTK_MAJOR_VERSION >= 2 */
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif  /* switch (USER_INTERFACE) */
	/* the information written to the command window can also be directed to a
		file */
#if defined (WX_USER_INTERFACE)
	       wxCommandWindow *wx_command_window;
#endif
	FILE *out_file;
	enum Command_window_outfile_mode out_file_mode;
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
#if defined (MOTIF)
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
		Command_window_set_command_prompt(command_window, (char *)NULL);
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

/*
Model menu widget identify functions
------------------------------------
*/

static void id_cell_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the cell create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_cell_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.cell_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_cell_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_cell_create_button */

static void id_curve_editor_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the curve_editor create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_curve_editor_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.curve_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_curve_editor_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_curve_editor_create_button */

static void id_data_viewer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_data_viewer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.data_viewer_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_data_viewer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_data_viewer_create_button */

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
		command_window->main_menu.model_menu.
			interactive_data_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_interactive_data_editor_cbut.  Command window structure missing");
	}
	LEAVE;
} /* id_interactive_data_editor_cbut */

static void id_element_creator_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_element_creator_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.element_creator_create_button
			=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_element_creator_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_element_creator_create_butto */

static void id_element_point_viewer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the element_point_viewer create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_element_point_viewer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.element_point_viewer_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_element_point_viewer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_element_point_viewer_create_button */

static void id_grid_field_calculator_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the grid_field_calculator create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_grid_field_calculator_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.grid_field_calculator_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_grid_field_calculator_button.  Command window structure missing");
	}
	LEAVE;
} /* id_grid_field_calculator_button */

static void id_im_control_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_im_control_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.im_control_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_im_control_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_im_control_create_button */

static void id_node_viewer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_node_viewer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.node_viewer_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_node_viewer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_node_viewer_create_button */

static void id_interactive_node_editor_cbut(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_interactive_node_editor_cbut);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.
			interactive_node_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_interactive_node_editor_cbut.  Command window structure missing");
	}
	LEAVE;
} /* id_interactive_node_editor_cbut */

static void id_tracking_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_tracking_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.tracking_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_tracking_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_tracking_editor_create_butto */

static void id_unemap_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the unemap create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_unemap_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.unemap_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_unemap_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_unemap_create_button */

static void id_3d_digitizer_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_3d_digitizer_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.model_menu.digitizer_3d_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_3d_digitizer_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_3d_digitizer_create_button */

/*
Graphics menu widget identify functions
---------------------------------------
*/

static void id_three_d_window_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_three_d_window_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.three_d_window_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_three_d_window_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_three_d_window_create_button */

static void id_material_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_material_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.material_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_material_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_material_editor_create_butto */

static void id_scene_editor_create_butt(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_scene_editor_create_butt);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.scene_editor_create_button
			=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_scene_editor_create_butt.  Command window structure missing");
	}
	LEAVE;
} /* id_scene_editor_create_butt */

static void id_spectrum_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_spectrum_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.spectrum_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_spectrum_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_spectrum_editor_create_butto */

static void id_time_editor_create_butto(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
Stores the id of the unemap create button
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_time_editor_create_butto);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.time_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_time_editor_create_butto.  Command window structure missing");
	}
	LEAVE;
} /* id_time_editor_create_butto */

static void id_volume_editor_create_button(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION:
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(id_volume_editor_create_button);
 	USE_PARAMETER(call_data);
	if (command_window=(struct Command_window *)client_data)
	{
		command_window->main_menu.graphics_menu.volume_editor_create_button=widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_volume_editor_create_button.  Command window structure missing");
	}
	LEAVE;
} /* id_volume_editor_create_button */

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

			if (command_window->out_file &&
				(command_window->out_file_mode & OUTFILE_INPUT))
			{
				fprintf(command_window->out_file,"%s\n",command);
			}

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
#if defined (MOTIF)
		/*???DB.  To allow restarting of back end */
#if !defined (NO_HELP)
		destroy_help();
#endif /* !defined (NO_HELP) */
#endif /* defined (MOTIF) */
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
#endif /* defined (MOTIF) */

#if defined (WIN32_USER_INTERFACE)
WNDPROC old_command_edit_wndproc;

static LRESULT CALLBACK Command_window_command_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 25 June 2002

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(electrode_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					char command[1000];
					int length;
					struct Command_window *command_window;
					
					command_window = (struct Command_window *)GetWindowLong(window,
						GWL_USERDATA);
					length = (WORD) SendMessage(window, 
                        EM_LINELENGTH, 
                        (WPARAM) 0, 
                        (LPARAM) 0); 
					*((LPWORD)command) = 999;
					length = SendMessage(window, 
						EM_GETLINE, 
						(WPARAM) 0,
						(LPARAM) command); 
					command[length] = 0;
					Execute_command_execute_string(command_window->execute_command,
						command);
					SendMessage(window, 
						WM_SETTEXT, 
						(WPARAM) 0,
						(LPARAM) command_window->command_prompt); 
					length = strlen(command_window->command_prompt);
					SendMessage(window, 
					   EM_SETSEL, 
						(WPARAM) length,
						(LPARAM) length);
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_command_edit_wndproc,window,
		message_identifier,first_message,second_message);
	LEAVE;

	return (return_code);
} /* Command_window_command_edit_pick_up_enter */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static LRESULT CALLBACK Command_window_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 25 October 1996

DESCRIPTION:
==============================================================================*/
{
	HWND command_edit;
	LRESULT return_code;
	struct Command_window *command_window;

	ENTER(Command_window_class_proc);
	return_code=FALSE;
	switch (message_identifier)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		case WM_INITDIALOG:
		{
			if (command_window = (struct Command_window *)second_message)
			{
				command_edit = GetDlgItem(window, IDC_COMMAND_ENTRY);
				/* replace the window procedure so that enters can be picked up */
				old_command_edit_wndproc=(WNDPROC)SetWindowLong(
					command_edit, GWL_WNDPROC,
					(DWORD)Command_window_command_edit_pick_up_enter);
				SetWindowLong(command_edit,GWL_USERDATA,
					(LONG)command_window);
			}
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
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static void Command_window_WM_COMMAND_handler(HWND window,
	int item_control_accelerator_id,HWND control_window,UINT notify_code)
/*******************************************************************************
LAST MODIFIED : 27 October 1996

DESCRIPTION:
==============================================================================*/
{
	ENTER(Command_window_WM_COMMAND_handler);
	USE_PARAMETER(control_window);
	switch (item_control_accelerator_id)
	{
		case IDCANCEL:
		{
			DestroyWindow(window);
		} break;
	   case IDC_COMMAND_ENTRY:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
				{
					if (0x1000 & GetKeyState(VK_RETURN))
					{
						SendMessage(window, 
							IDOK, 
							(WPARAM) 0,  /* line 0 */
							(LPARAM) 0); 						
					}
				}
#if defined (OLD_CODE)
				case EN_CHANGE:
				{
					char buffer[1000];
					int length;
					
					length = (WORD) SendMessage(control_window, 
						EM_LINELENGTH, 
						(WPARAM) 0, 
						(LPARAM) 0); 
					*((LPWORD)buffer) = 999;
					SendMessage(control_window, 
						EM_GETLINE, 
						(WPARAM) 0,   /* line 0 */
						(LPARAM) buffer); 
					buffer[length] = 0;
					MessageBox(window, 
						buffer, 
						"Did it work?", 
						MB_OK); 
				} break;
#endif /* defined (OLD_CODE) */
			}
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
#endif /* defined (WIN32_USER_INTERFACE) */

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

#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
static void command_entered_gtk(GtkEntry *entry, gpointer command_window_void)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called when a command is entered in the command entry area.
==============================================================================*/
{
	char *command;
	struct Command_window *command_window;

	ENTER(command_entered_gtk);
	if (command_window=(struct Command_window *)command_window_void)
	{
		if (command = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1))
		{
			if (command_window->out_file &&
				(command_window->out_file_mode & OUTFILE_INPUT))
			{
				fprintf(command_window->out_file,"%s\n",command);
			}

			Execute_command_execute_string(command_window->execute_command,
				command);
		}
		else
		{
			display_message(ERROR_MESSAGE,"command_entered_gtk.  "
				"Unable to retrieve command string.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"command_entered_gtk.  Invalid argument(s)");
	}
	LEAVE;
} /* command_entered_gtk */
#endif /* ! defined (MOTIF) && defined (GTK_USER_INTERFACE) */

#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
static gboolean Command_window_gtk_button_press(GtkWidget *widget,
	GdkEventButton *event, gpointer command_window_void)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called when a command is entered in the command entry area.
==============================================================================*/
{
	gboolean return_code;
#if GTK_MAJOR_VERSION >= 2
	gchar *selected_text;
	gint buffer_x, buffer_y;
	GtkTextIter end, start;
	GtkTextView *history_view;
#endif /* GTK_MAJOR_VERSION >= 2 */
	struct Command_window *command_window;

	ENTER(Command_window_gtk_button_press);
	USE_PARAMETER(widget);
	if (command_window=(struct Command_window *)command_window_void)
	{
		switch(event->type)
		{
			case GDK_BUTTON_PRESS:
			{
#if GTK_MAJOR_VERSION >= 2
				/* Code from default triple click handler */
				history_view = GTK_TEXT_VIEW(command_window->history_view);
				gtk_text_view_window_to_buffer_coords(history_view,
					GTK_TEXT_WINDOW_WIDGET, (gint)event->x, (gint)event->y, &buffer_x, &buffer_y);
				gtk_text_view_get_iter_at_location (history_view,
					&start, buffer_x, buffer_y);

				end = start;
				if (gtk_text_view_starts_display_line (history_view, &start))
            {
					gtk_text_view_backward_display_line_start (history_view, &start);
            }
				else
            {
					gtk_text_view_backward_display_line_start (history_view, &start);
					if (!gtk_text_view_starts_display_line (history_view, &end))
						gtk_text_view_forward_display_line_end (history_view, &end);
            }
				
				gtk_text_buffer_move_mark_by_name (command_window->history_buffer,
					"selection_bound", &start);
				gtk_text_buffer_move_mark_by_name (command_window->history_buffer,
					"insert",&end);

				/* Set the entry box to show the selected line */
				selected_text = gtk_text_buffer_get_text(command_window->history_buffer,
					&start, &end, FALSE);
				gtk_entry_set_text(GTK_ENTRY(command_window->entry),
					selected_text);
				g_free(selected_text);
				gtk_editable_set_position(GTK_EDITABLE(command_window->entry), -1);

#else /* GTK_MAJOR_VERSION >= 2 */
#endif /* GTK_MAJOR_VERSION >= 2 */
				return_code = TRUE;
			} break;
			case GDK_2BUTTON_PRESS:
			{
				command_entered_gtk(GTK_ENTRY(command_window->entry), command_window_void);
				return_code = TRUE;
			} break;
			default:
			{
				return_code = FALSE;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Command_window_gtk_button_press.  Invalid argument(s)");
	}

	LEAVE;
	return (return_code);
} /* Command_window_gtk_button_press */
#endif /* ! defined (MOTIF) && defined (GTK_USER_INTERFACE) */

#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
static void command_window_close_gtk(GtkObject *object, gpointer command_window_void)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called when the Close function is selected from the window manager menu.
==============================================================================*/
{
	struct Command_window *command_window;

	ENTER(command_window_close_gtk);
	USE_PARAMETER(object);
	if (command_window=(struct Command_window *)command_window_void)
	{
		Execute_command_execute_string(command_window->execute_command, "QUIT");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"command_window_close_gtk.  Missing command window");
	}
	LEAVE;
} /* command_window_close_gtk */
#endif /* ! defined (MOTIF) && defined (GTK_USER_INTERFACE) */

static int modify_Command_window_out_file_open(struct Parse_state *state,
	void *dummy,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
==============================================================================*/
{
	char *file_name, input, output;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
	   {"input",NULL,NULL,set_char_flag},
	   {"output",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_file_name},
	};
	struct Command_window *command_window;

	ENTER(modify_Command_window_out_file_open);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		file_name=(char *)NULL;
		input = 0;
		output = 0;
		option_table[0].to_be_modified= &input;
		option_table[1].to_be_modified= &output;
		option_table[2].to_be_modified= &file_name;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if ((command_window=(struct Command_window *)command_window_void)
				&&file_name)
			{
			   if (input && output)
				{
					command_window->out_file_mode =
						(enum Command_window_outfile_mode)(OUTFILE_INPUT & OUTFILE_OUTPUT);
				}
				else if (input)
				{
					command_window->out_file_mode = OUTFILE_INPUT;
				}
				else if (output)
				{
					command_window->out_file_mode = OUTFILE_OUTPUT;
				}
				else
				{
					/* This is what it used to do, so this is the default */
					command_window->out_file_mode = OUTFILE_OUTPUT;
				}
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

#if defined (WX_USER_INTERFACE)
class wxCommandWindow : public wxFrame
{
  Command_window *command_window;
	wxListBox *history_list;
	wxTextCtrl *command_line;
	wxString SelectedCommand;
	wxString command;
	wxString blank;
	wxTextCtrl *output_list;
	int number;

public:

  wxCommandWindow(Command_window *command_window): 
    command_window(command_window)
  {
  };

  wxCommandWindow()
  {
  };

	void CommandEntered(wxCommandEvent& event)
	{
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  command_line = XRCCTRL(*this, "CommandLine", wxTextCtrl);
	  command = command_line->GetValue();
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
	  history_list->InsertItems(1,&command, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  const_cast<char *>(command.c_str()));
	  command_line -> Clear();
	  history_list->SetSelection(history_list->GetCount()-1);
	  history_list->Thaw();
	  }      
                
         void display_output(wxString outmessage)
         {
					 output_list = XRCCTRL(*this,"OutputWindow", wxTextCtrl);
					 output_list->AppendText(outmessage);
       	 }
    	 

        void SingleClick(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  command_line = XRCCTRL(*this, "CommandLine", wxTextCtrl);
	  SelectedCommand = history_list->GetStringSelection();
	  command_line -> Clear();
	  command_line -> WriteText(SelectedCommand);
	}


        void DoubleClick(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  command_line = XRCCTRL(*this, "CommandLine", wxTextCtrl);
	  SelectedCommand = history_list->GetStringSelection();  
 	  number = history_list->GetCount();
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  const_cast<char *>(SelectedCommand.c_str()));
	  command_line->Clear();
	  history_list->SetSelection ( history_list->GetCount() - 1 ); 	 
	  history_list->Deselect( history_list->GetCount() - 1 );
       	}

        void threeDwindow(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx cre win";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx cre win");
        }

        void sceneeditorwindow(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx edit scene";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx edit scene");
        }

        void opencom(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "open comfile";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "open comfile");
					}

        void readcurve(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx read curve";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx read curve");
					}

        void readdata(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx read data";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx read data");
					}

        void readelements(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx read elements";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx read elements");
					}

        void readnodes(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx read nodes";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx read nodes");
					}

        void writecurve(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx write curve all";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx write curve all");
					}

        void writedata(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx write data";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx write data");
					}

        void writeelements(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx write elements";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx write elements");
					}

        void writenodes(wxCommandEvent& event)
        {
	  history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
	  number = history_list->GetCount();
		if (number == 0)
	    history_list->InsertItems(1, &blank,number);
	  number = history_list->GetCount();
    SelectedCommand = "gfx write nodes";
	  history_list->InsertItems(1,&SelectedCommand, number-1);
	  Execute_command_execute_string(command_window->execute_command,
	  "gfx write nodes");
					}

        void Exit(wxCommandEvent& event)
        {
	  Execute_command_execute_string(command_window->execute_command, "QUIT");
        } 

DECLARE_DYNAMIC_CLASS(wxCommandWindow);
   DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxCommandWindow, wxFrame)

BEGIN_EVENT_TABLE(wxCommandWindow, wxFrame)
	EVT_TEXT_ENTER(XRCID("CommandLine"), wxCommandWindow::CommandEntered)
	EVT_LISTBOX(XRCID("CommandHistory"),wxCommandWindow::SingleClick)
	EVT_LISTBOX_DCLICK(XRCID("CommandHistory"),wxCommandWindow::DoubleClick)
	EVT_MENU(XRCID("GraphicsthreeDWindow"),wxCommandWindow::threeDwindow)
	EVT_MENU(XRCID("GraphicsSceneeditor"),wxCommandWindow::sceneeditorwindow)
	EVT_MENU(XRCID("FileComFile"),wxCommandWindow::opencom)
	EVT_MENU(XRCID("ReadCurve"),wxCommandWindow::readcurve)
	EVT_MENU(XRCID("ReadData"),wxCommandWindow::readdata)
	EVT_MENU(XRCID("ReadElements"),wxCommandWindow::readelements)
	EVT_MENU(XRCID("ReadNode"),wxCommandWindow::readnodes)
	EVT_MENU(XRCID("WriteCurve"),wxCommandWindow::writecurve)
	EVT_MENU(XRCID("WriteData"),wxCommandWindow::writedata)
	EVT_MENU(XRCID("WriteElements"),wxCommandWindow::writeelements)
	EVT_MENU(XRCID("WriteNode"),wxCommandWindow::writenodes)
	EVT_MENU(XRCID("MenuExit"),wxCommandWindow::Exit)
END_EVENT_TABLE()

#endif /* defined (WX_USER_INTERFACE) */

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
	struct Command_window *command_window;
#if defined (MOTIF) /* switch (USER_INTERFACE) */
	Atom WM_DELETE_WINDOW;
	MrmType command_window_class;
	static MrmRegisterArg callback_list[]={
		{"command_changed",(XtPointer)command_changed},
		{"command_entered",(XtPointer)command_entered},
		{"identify_command_box",(XtPointer)identify_command_box},
		{"identify_output_pane",(XtPointer)identify_output_pane},
		{"identify_message_pane",(XtPointer)identify_message_pane},
		/* model menu */
		{"id_cell_create_button",(XtPointer)id_cell_create_button},
		{"id_curve_editor_create_button",(XtPointer)id_curve_editor_create_button},
		{"id_data_viewer_create_button",(XtPointer)id_data_viewer_create_button},
		{"id_interactive_data_editor_cbut",
			(XtPointer)id_interactive_data_editor_cbut},
		{"id_element_creator_create_butto",
			(XtPointer)id_element_creator_create_butto},
		{"id_element_point_viewer_button",
			(XtPointer)id_element_point_viewer_create_button},
		{"id_grid_field_calculator_button",
			(XtPointer)id_grid_field_calculator_button},
		{"id_im_control_create_button",(XtPointer)id_im_control_create_button},
		{"id_node_viewer_create_button",(XtPointer)id_node_viewer_create_button},
		{"id_interactive_node_editor_cbut",
			(XtPointer)id_interactive_node_editor_cbut},
		{"id_tracking_editor_create_butto",
			(XtPointer)id_tracking_editor_create_butto},
		{"id_unemap_create_button",(XtPointer)id_unemap_create_button},
		{"id_3d_digitizer_create_button",(XtPointer)id_3d_digitizer_create_button},
		/* graphics menu */
		{"id_three_d_window_create_button",
			(XtPointer)id_three_d_window_create_button},
		{"id_material_editor_create_butto",
			(XtPointer)id_material_editor_create_butto},
		{"id_scene_editor_create_button",
			(XtPointer)id_scene_editor_create_butt},
		{"id_spectrum_editor_create_butto",
			(XtPointer)id_spectrum_editor_create_butto},
		{"id_time_editor_create_butto",
			(XtPointer)id_time_editor_create_butto},
		{"id_volume_editor_create_button",
			(XtPointer)id_volume_editor_create_button},

		{"callback_command",(XtPointer)callback_command},
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"command_window_close",(XtPointer)command_window_close}
	};
	static MrmRegisterArg identifier_list[2];
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
	BOOL win32_return_code;
	static char *class_name="Command_window";
	WNDCLASSEX class_information;
	USE_PARAMETER(version_id_string);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
	GtkTextIter end_iterator;
#endif /* GTK_MAJOR_VERSION >= 2 */
	GtkWidget *history_scroll, *output_scroll, *vbox, *vpaned;
#endif /* switch (USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
	USE_PARAMETER(version_id_string);
#endif /* switch (GTK_USER_INTERFACE) */
	ENTER(CREATE(Command_window));
	/* check arguments */
	if (execute_command&&user_interface)
	{
		if (ALLOCATE(command_window,struct Command_window,1))
		{
			command_window->user_interface=user_interface;
			command_window->execute_command=execute_command;
			command_window->out_file=(FILE *)NULL;
			command_window->out_file_mode=OUTFILE_INVALID;
#if defined (MOTIF) /* switch (USER_INTERFACE) */
			if (MrmOpenHierarchy_binary_string(command_window_uidh,sizeof(command_window_uidh),
				&command_window_hierarchy,&command_window_hierarchy_open))
			{

				/* file menu */
				command_window->main_menu.file_menu.open_menu.open_comfile_button=
					(Widget)NULL;
				command_window->main_menu.file_menu.open_menu.open_menu_button=
					(Widget)NULL;
				command_window->main_menu.file_menu.exit_button=(Widget)NULL;

				/* model menu */
				command_window->main_menu.model_menu.cell_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.curve_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.data_viewer_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.
					interactive_data_editor_create_button=(Widget)NULL;
				command_window->main_menu.model_menu.element_creator_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.element_point_viewer_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.grid_field_calculator_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.im_control_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.node_viewer_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.
					interactive_node_editor_create_button=(Widget)NULL;
				command_window->main_menu.model_menu.tracking_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.unemap_create_button=
					(Widget)NULL;
				command_window->main_menu.model_menu.digitizer_3d_create_button=
					(Widget)NULL;

				/* graphics menu */
				command_window->main_menu.graphics_menu.three_d_window_create_button=
					(Widget)NULL;
				command_window->main_menu.graphics_menu.material_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.graphics_menu.scene_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.graphics_menu.spectrum_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.graphics_menu.time_editor_create_button=
					(Widget)NULL;
				command_window->main_menu.graphics_menu.volume_editor_create_button=
					(Widget)NULL;

				/* help menu */
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
					xmDialogShellWidgetClass,User_interface_get_application_shell(user_interface),
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
								XA_CMGUI_VERSION=XInternAtom(User_interface_get_display(user_interface),
									CMGUI_VERSION_PROPERTY,False);
								XA_CMGUI_LOCK=XInternAtom(User_interface_get_display(user_interface),
									CMGUI_LOCK_PROPERTY,False);
								XA_CMGUI_COMMAND=XInternAtom(User_interface_get_display(user_interface),
									CMGUI_COMMAND_PROPERTY,False);
								XA_CMGUI_RESPONSE=XInternAtom(User_interface_get_display(user_interface),
									CMGUI_RESPONSE_PROPERTY,False);
								XChangeProperty(User_interface_get_display(user_interface),
									XtWindow(command_window->shell),XA_CMGUI_VERSION,XA_STRING,8,
									PropModeReplace,(unsigned char *)version_id_string,
									strlen(version_id_string));
#if defined (OLD_CODE)								
								XGetWindowAttributes(User_interface_get_display(user_interface),
									XtWindow(command_window->shell),&window_attributes);
								event_mask=(window_attributes.your_event_mask)|
									PropertyChangeMask;
								XSelectInput(User_interface_get_display(user_interface),
									XtWindow(command_window->shell), event_mask);
#endif /* defined (OLD_CODE) */
								set_property_notify_callback(user_interface,
									command_window_property_notify_callback,
									(void *)command_window, command_window->shell);
								XtPopup(command_window->shell,XtGrabNone);
								/*???DB.  If I don't have this call to XmuClientWindow, then
								  sometimes cmgui doesn't pick up any of the properties set by
								  Netscape.  It is some sort of timing problem.  XmuClientWindow
								  finds a window at or below the specified one which has the
								  WM_STATE property set */
#if defined (OLD_CODE)								
								XmuClientWindow(User_interface_get_display(user_interface),
									XtWindow(command_window->shell));
#endif /* defined (OLD_CODE) */
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
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Command_window).  Could not open hierarchy");
				command_window=(struct Command_window *)NULL;
			}
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
			command_window->command_history = (HWND)NULL;
			command_window->command_entry = (HWND)NULL;
			command_window->command_output_pane = (HWND)NULL;
			if (ALLOCATE(command_window->command_prompt, char , 1))
			{
				*command_window->command_prompt = 0;
				/* check if the class is registered, need to check against FALSE
				 as the return value is successful while declared as BOOL is 'nonzero' */
				if (FALSE==GetClassInfoEx(
					User_interface_get_instance(user_interface),
					class_name,&class_information))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=
						DLGWINDOWEXTRA+sizeof(struct Command_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=LoadIcon(
						User_interface_get_instance(user_interface),
						"Command_window_icon");
					class_information.hInstance=User_interface_get_instance(
						user_interface);
					class_information.lpfnWndProc=Command_window_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=LoadIcon(User_interface_get_instance
						(user_interface),"Command_window_icon_small");
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				else
				{
					win32_return_code = TRUE;
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					if (command_window->dialog=CreateDialogParam(
						GetModuleHandle("libcmgui.dll"),
						"Command_window",
						(HWND)NULL,
						Command_window_dialog_proc,
						(LPARAM)command_window))
					{
						command_window->command_history = GetDlgItem(command_window->dialog,
							IDC_COMMAND_HISTORY); 
						command_window->command_entry = GetDlgItem(command_window->dialog,
							IDC_COMMAND_ENTRY); 
						command_window->command_output_pane = GetDlgItem(command_window->dialog,
							IDC_COMMAND_OUTPUT_PANE); 
						ShowWindow(command_window->dialog,SW_SHOW);
					}
					else
					{
						TCHAR szBuf[80]; 
						LPVOID lpMsgBuf;
						DWORD dw = GetLastError(); 

						FormatMessage(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							dw,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR) &lpMsgBuf,
							0, NULL );

						wsprintf(szBuf, 
							"failed with error %d: %s", 
							dw, lpMsgBuf); 
 
						MessageBox(NULL, szBuf, "Error", MB_OK); 

#if defined (OLD_CODE)
						LocalFree(lpMsgBuf);
						ExitProcess(dw); 
						display_message(ERROR_MESSAGE,
							"CREATE(Command_window).  Could not create dialog");
						DEALLOCATE(command_window);
#endif /* defined (OLD_CODE) */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Command_window).  Unable to register class information");
					DEALLOCATE(command_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Command_window).  Insufficient memory for command_window prompt");
			}
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
			command_window->entry = (GtkWidget *)NULL;
			command_window->history_view = (GtkWidget *)NULL;
			command_window->output_view = (GtkWidget *)NULL;
#if GTK_MAJOR_VERSION >= 2
			command_window->history_buffer = (GtkTextBuffer *)NULL;
			command_window->output_buffer = (GtkTextBuffer *)NULL;
			command_window->close_handler_id = 0;
#else /* GTK_MAJOR_VERSION >= 2 */
#endif /* GTK_MAJOR_VERSION >= 2 */
			command_window->shell = (GtkWidget *)NULL;

			if (ALLOCATE(command_window->command_prompt, char , 1))
			{
				*command_window->command_prompt = 0;
				if (command_window->shell = gtk_window_new(GTK_WINDOW_TOPLEVEL))
				{
					gtk_window_set_title(GTK_WINDOW(command_window->shell), "CMISS Command Window");
					gtk_widget_set_name(command_window->shell, "command_window");
				
					/* create a vpaned widget and add it to our toplevel window */
					vpaned = gtk_vpaned_new ();
					gtk_container_add (GTK_CONTAINER (command_window->shell), vpaned);
					gtk_widget_show (vpaned);
				
					vbox = gtk_vbox_new(FALSE, 3);

#if GTK_MAJOR_VERSION >= 2
					command_window->history_view = gtk_text_view_new ();
					command_window->history_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (command_window->history_view));
					gtk_widget_set_name(command_window->history_view, "command_history");
					gtk_text_view_set_editable(GTK_TEXT_VIEW(command_window->history_view),
						FALSE);
					gtk_text_buffer_get_end_iter(command_window->history_buffer,
						&end_iterator);
					command_window->history_end = gtk_text_buffer_create_mark(
						command_window->history_buffer, "end", &end_iterator, FALSE);
					g_signal_connect (GTK_WIDGET(command_window->history_view),
						"button-press-event", G_CALLBACK(Command_window_gtk_button_press),
						(gpointer)command_window);
					GTK_WIDGET_UNSET_FLAGS(command_window->history_view, GTK_CAN_FOCUS);
					gtk_widget_show (command_window->history_view);
#else /* GTK_MAJOR_VERSION >= 2 */
					command_window->history_view = gtk_text_new (NULL, NULL);
					gtk_widget_set_name(command_window->history_view, "command_history");
					gtk_text_set_editable(GTK_TEXT(command_window->history_view), FALSE);
					gtk_signal_connect (GTK_OBJECT(command_window->history_view),
						"button-press-event", GTK_SIGNAL_FUNC(Command_window_gtk_button_press),
						(gpointer)command_window);
					GTK_WIDGET_UNSET_FLAGS(command_window->history_view, GTK_CAN_FOCUS);
					gtk_widget_show (command_window->history_view);
#endif /* GTK_MAJOR_VERSION >= 2 */

					history_scroll = gtk_scrolled_window_new(NULL, NULL);
					gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_scroll),
						GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
					gtk_container_add (GTK_CONTAINER (history_scroll),
						command_window->history_view);

					gtk_box_pack_start (GTK_BOX (vbox), history_scroll, TRUE, TRUE, 0);
					gtk_widget_show (history_scroll);

					command_window->entry = gtk_entry_new();
#if GTK_MAJOR_VERSION >= 2
					g_signal_connect (GTK_ENTRY(command_window->entry), "activate",
						G_CALLBACK(command_entered_gtk), (gpointer)command_window);
#else /* GTK_MAJOR_VERSION >= 2 */
					gtk_signal_connect(GTK_OBJECT(command_window->entry), "activate",
						GTK_SIGNAL_FUNC(command_entered_gtk), (gpointer)command_window);
#endif /* GTK_MAJOR_VERSION >= 2 */
					gtk_widget_set_name(command_window->entry, "command_entry");
					gtk_widget_show (command_window->entry);
					gtk_box_pack_end (GTK_BOX (vbox), command_window->entry, FALSE, TRUE, 0);
				
					gtk_widget_show (vbox);
					gtk_paned_pack1 (GTK_PANED (vpaned), vbox, TRUE, TRUE);

#if GTK_MAJOR_VERSION >= 2
					command_window->output_view = gtk_text_view_new ();
					command_window->output_buffer = gtk_text_view_get_buffer(
						GTK_TEXT_VIEW(command_window->output_view));
					gtk_text_view_set_editable(GTK_TEXT_VIEW(command_window->output_view),
						FALSE);
					gtk_widget_set_name(command_window->output_view, "command_output");
					gtk_text_buffer_get_end_iter(command_window->output_buffer,
						&end_iterator);
					command_window->output_end = gtk_text_buffer_create_mark(
						command_window->output_buffer, "end", &end_iterator, FALSE);
					gtk_widget_show (command_window->output_view);
#else /* GTK_MAJOR_VERSION >= 2 */
					command_window->output_view = gtk_text_new (NULL, NULL);
					gtk_text_set_editable(GTK_TEXT(command_window->output_view), FALSE);
					gtk_widget_set_name(command_window->output_view, "command_output");
					gtk_widget_show (command_window->output_view);
#endif /* GTK_MAJOR_VERSION >= 2 */

					output_scroll = gtk_scrolled_window_new(NULL, NULL);
					gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll),
						GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
					gtk_container_add (GTK_CONTAINER (output_scroll),
						command_window->output_view);
					gtk_widget_show (output_scroll);

					gtk_paned_pack2 (GTK_PANED (vpaned), output_scroll, TRUE, TRUE);
					
					gtk_widget_show (vpaned);

#if GTK_MAJOR_VERSION >= 2
					command_window->close_handler_id =
						g_signal_connect (G_OBJECT(command_window->shell), "destroy",
						G_CALLBACK(command_window_close_gtk), (gpointer)command_window);

					gtk_window_resize(GTK_WINDOW(command_window->shell), 600, 500);
#else /* GTK_MAJOR_VERSION >= 2 */
					gtk_signal_connect(GTK_OBJECT(command_window->shell), "destroy",
						GTK_SIGNAL_FUNC(command_window_close_gtk), (gpointer)command_window);

					gtk_widget_set_usize(command_window->shell, 600, 500);
#endif /* GTK_MAJOR_VERSION >= 2 */

					gtk_widget_show (command_window->shell);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Command_window).  Unable to create top level shell");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Command_window).  Insufficient memory for command_window prompt");
			}
#elif defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			wxXmlInit_command_window();
			
			command_window->wx_command_window = new 
			  wxCommandWindow(command_window);

			wxXmlResource::Get()->LoadFrame(command_window->wx_command_window,
			   (wxWindow *)NULL, _T("CmguiCommandWindow"));
			command_window->wx_command_window->Show();
#endif /* switch (USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Command_window).  Insufficient memory for command_window");
		}
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

	if (command_window_pointer && (command_window = *command_window_pointer))
	{
		if (command_window->out_file)
		{
			fclose(command_window->out_file);
		}
#if defined (MOTIF) /* switch (USER_INTERFACE) */
		set_property_notify_callback(command_window->user_interface,
			(Property_notify_callback)NULL, (void *)NULL, (Widget)NULL);
		destroy_Shell_list_item_from_shell(&(command_window->shell),
			command_window->user_interface);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
		g_signal_handler_disconnect (G_OBJECT(command_window->shell), 
			command_window->close_handler_id);
#endif /* GTK_MAJOR_VERSION >= 2 */
		gtk_widget_destroy (command_window->shell);
#endif /* switch (USER_INTERFACE) */

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

int add_to_command_list(char *command,struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int max_commands,num_commands;
	XmString new_command;
#endif /* defined (MOTIF) */
#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
	GtkTextIter end_iterator;
#else /* GTK_MAJOR_VERSION >= 2 */
	guint text_length;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* ! defined (MOTIF) && defined (GTK_USER_INTERFACE) */


	ENTER(add_to_command_list);
/*???debug */
/* printf("enter add_to_command_list\n  %s\n",command); */
#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION < 2
	USE_PARAMETER(command);
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* ! defined (MOTIF) && defined (GTK_USER_INTERFACE) */
	if (command_window)
	{
#if defined (MOTIF) /* switch (USER_INTERFACE) */
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
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		SendMessage(command_window->command_history, LB_ADDSTRING, 0, 
			(LPARAM)command);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
		gtk_text_buffer_get_end_iter(command_window->history_buffer,
			&end_iterator);
		gtk_text_buffer_insert(command_window->history_buffer,
			&end_iterator, command, strlen(command));
		gtk_text_buffer_insert(command_window->history_buffer,
			&end_iterator, "\n", 1);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(command_window->history_view),
			command_window->history_end, 0.0, FALSE, 0.0, 0.0);
#else /* GTK_MAJOR_VERSION >= 2 */
		text_length = gtk_text_get_length(GTK_TEXT(command_window->history_view));
		gtk_text_set_point(GTK_TEXT(command_window->history_view), text_length);
		gtk_text_insert(GTK_TEXT(command_window->history_view), NULL, NULL, NULL,
			command, -1);
		gtk_text_insert(GTK_TEXT(command_window->history_view), NULL, NULL, NULL,
			"\n", 1);
		return_code = 1;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* switch (USER_INTERFACE) */
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

int Command_window_set_command_prompt(struct Command_window *command_window,
	char *prompt)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <command_window>.
==============================================================================*/
{
#if defined (MOTIF)
	char *temp;
#endif /* defined (MOTIF) */
	int return_code;

	ENTER(set_command_prompt);
#if ! defined (MOTIF) && defined (GTK_USER_INTERFACE)
	USE_PARAMETER(prompt);
#endif /* switch (GTK_USER_INTERFACE) */
	if (command_window)
	{
#if defined (MOTIF) /* switch (USER_INTERFACE) */
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
#elif defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
		if (command_window->command_prompt)
		{
			DEALLOCATE(command_window->command_prompt);
		}
		if (prompt&&(*prompt))
		{
			if (ALLOCATE(command_window->command_prompt,char,strlen(prompt)+2))
			{
				sprintf(command_window->command_prompt,"%s ",prompt);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_command_prompt.  Could not allocate prompt string");
			}
		}
		else
		{
			if (ALLOCATE(command_window->command_prompt,char,2))
			{
				command_window->command_prompt[0] = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_command_prompt.  Could not allocate prompt string");
			}
		}
#endif /* switch (USER_INTERFACE) */
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
#elif defined (GTK_USER_INTERFACE)
		gtk_entry_set_text(GTK_ENTRY(command_window->entry),
			command_window->command_prompt);
		gtk_editable_set_position(GTK_EDITABLE(command_window->entry), -1);
#endif /* defined (GTK_USER_INTERFACE) */

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
#if defined (MOTIF)
		XtVaSetValues(command_window->command_entry,
			XmNvalue,(XtPointer)command_string,NULL);
#endif /* defined (MOTIF) */
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

#if defined (MOTIF)
Widget Command_window_get_message_pane(struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the message pane widget.
==============================================================================*/
{
	Widget return_widget;

	ENTER(Command_window_get_message_pane);
	if (command_window)
	{
		return_widget = command_window->output_pane;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Command_window_get_message_pane.  Invalid argument(s)");
		return_widget = (Widget)NULL;
	}
	LEAVE;

	return (return_widget);
} /* Command_window_get_message_pane */
#endif /* defined (MOTIF) */
int write_command_window(char *message,struct Command_window
*command_window)
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF) /* switch (USER_INTERFACE) */
	Widget output_pane;
	XmTextPosition text_pos;
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
#define MAX_OUTPUT (10000)
	int new_length, position;
#elif defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
	GtkTextIter end_iterator;
#else /* GTK_MAJOR_VERSION >= 2 */
	guint text_length;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* switch (USER_INTERFACE) */
	
	ENTER(write_command_window);
	return_code=0;

	if (command_window)
	{
#if defined (MOTIF) /* switch (USER_INTERFACE) */
		if (output_pane=command_window->output_pane)
		{
			text_pos=XmTextGetLastPosition(output_pane);
			XmTextInsert(output_pane,text_pos,message);
			text_pos=XmTextGetLastPosition(output_pane);
			XmTextShowPosition(output_pane,text_pos);
			return_code=1;
		}
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		new_length = SendMessage(command_window->command_output_pane,
			WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
		new_length += strlen (message);
		if (new_length > MAX_OUTPUT)
		{
			/* Remove entire lines to under MAX_OUTPUT characters */
			position = SendMessage(command_window->command_output_pane,
				EM_LINEFROMCHAR, (WPARAM)(new_length - MAX_OUTPUT), (LPARAM)0);
			/* Add a few extra lines so that we don't do this every time */
			position = SendMessage(command_window->command_output_pane,
				EM_LINEINDEX, (WPARAM)(position + 10), (LPARAM)0);
			SendMessage(command_window->command_output_pane, EM_SETSEL,
				(WPARAM)0, (LPARAM)(position-1));
			SendMessage(command_window->command_output_pane, WM_CLEAR,
				(WPARAM)0, (LPARAM)0);
		}
		SendMessage(command_window->command_output_pane, EM_SETSEL,
			(WPARAM)-1, (LPARAM)-1);
		SendMessage(command_window->command_output_pane, EM_REPLACESEL,
			(WPARAM)FALSE, (LPARAM)message);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
		gtk_text_buffer_get_end_iter(command_window->output_buffer,
			&end_iterator);
		gtk_text_buffer_insert(command_window->output_buffer,
			&end_iterator, message, strlen(message));
		gtk_text_buffer_get_end_iter(command_window->output_buffer,
			&end_iterator);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(command_window->output_view),
			command_window->output_end, 0.0, FALSE, 0.0, 0.0);
		return_code = 1;
#else /* GTK_MAJOR_VERSION >= 2 */
		text_length = gtk_text_get_length(GTK_TEXT(command_window->output_view));
		gtk_text_set_point(GTK_TEXT(command_window->output_view), text_length);
		gtk_text_insert(GTK_TEXT(command_window->output_view), NULL, NULL, NULL,
			message, -1);
		return_code = 1;
#endif /* GTK_MAJOR_VERSION >= 2 */
#elif defined (WX_USER_INTERFACE)
		command_window->wx_command_window->display_output(message);
		return_code = 1;
#endif /* switch (USER_INTERFACE) */
		if (command_window->out_file &&
			(command_window->out_file_mode & OUTFILE_OUTPUT))
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

int modify_Command_window(struct Parse_state *state,void *dummy,
	void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 11 November 1998-1516


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
