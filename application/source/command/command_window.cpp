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
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include <stdio.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "command/command_window.h"
#if defined (WIN32_USER_INTERFACE)
#include "command/command_window.rc"
#endif /* defined (WIN32_USER_INTERFACE) */
#include "command/command.h"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include "license.h"
#include "wx/xrc/xmlres.h"
#include "command/command_window.xrch"
#include "icon/cmiss_icon.xpm"
#include <wx/aboutdlg.h>
#include <wx/fontdlg.h>
#include <wx/splitter.h>
#endif /* defined (WX_USER_INTERFACE)*/
#include "general/message.h"
#include "user_interface/user_interface.h"
#include "command/parser.h"

/*
Module types
------------
*/

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
class wxCommandLineTextCtrl : public wxTextCtrl
{
	 Command_window *command_window;
public:
	 wxCommandLineTextCtrl(Command_window *command_window, wxPanel *parent) :
			wxTextCtrl(parent, -1, wxT(""),wxPoint(0,0), wxSize(-1,24), wxTE_PROCESS_ENTER),
			command_window(command_window)
	{
			 wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
			 sizer->Add(this,
					wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
			 parent->SetSizer(sizer);
			 Show();
	};

	 wxCommandLineTextCtrl()
	 {
	 };

	 ~wxCommandLineTextCtrl()
	 {
	 };
	 void Reset(const char *command_prompt);

private:
	 void OnKeyDown(wxKeyEvent& event);
	 void OnCommandEntered(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};
#endif

struct Command_window
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
==============================================================================*/
{
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
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
	wxCommandLineTextCtrl *wx_command_line_text_ctrl;
	wxFrame *frame;
	wxPanel *lower_panel;
	wxTextCtrl *output_window;
	wxListBox *history_window;
	char *command_prompt;
//#	if defined(WIN32_SYSTEM)
	wxIcon icon(wxIcon(cmiss_icon));
//#	endif
#endif
	FILE *out_file;
	enum Command_window_outfile_mode out_file_mode;
	struct User_interface *user_interface;
	/* for executing commands */
	struct Execute_command *execute_command;
}; /* struct Command_window */

/*
Module functions
----------------
*/

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
					char command[2048];
					int length;
					struct Command_window *command_window;

					command_window = (struct Command_window *)GetWindowLong(window,
						GWL_USERDATA);
					length = (WORD) SendMessage(window,
						EM_LINELENGTH,
						(WPARAM) 0,
						(LPARAM) 0);
					*((LPWORD)command) = 2047;
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

				SetWindowLong(window,GWL_USERDATA,
					(LONG)command_window);
			}
		} break;
		case WM_SIZE:
		{
			if (command_window = (struct Command_window *)GetWindowLong(window,GWL_USERDATA))
			{
				RECT rcClient;
				GetClientRect(command_window->dialog, &rcClient);
				MoveWindow(command_window->command_history, 0, 0, rcClient.right, (rcClient.bottom - 35) / 2, true);
				MoveWindow(command_window->command_entry, 0, (rcClient.bottom - 35) / 2, rcClient.right, 35, true);
				MoveWindow(command_window->command_output_pane, 0, (rcClient.bottom - 35) / 2 + 35, rcClient.right, (rcClient.bottom - 35) / 2, true);
			}
		} break;
		case WM_COMMAND:
		{
			if (command_window = (struct Command_window *)GetWindowLong(window,GWL_USERDATA))
			{
				switch (HIWORD(first_message))
				{
				case LBN_SELCHANGE:
				{
					char command[2048];
					WORD selectionIndex = (WORD) SendMessage(command_window->command_history,
						LB_GETCURSEL,
						(WPARAM) 0,
						(LPARAM) 0);
					WORD length = (WORD) SendMessage(command_window->command_history,
						LB_GETTEXTLEN,
						(WPARAM) selectionIndex,
						(LPARAM) 0);
					if (length < 2048 - 1)
					{
						length = SendMessage(command_window->command_history,
								LB_GETTEXT,
								(WPARAM) selectionIndex,
								(LPARAM) command);
						command[length] = 0;
						SendMessage(command_window->command_entry,
							WM_SETTEXT,
							(WPARAM) 0,
							(LPARAM) command);
						SendMessage(command_window->command_entry,
						   EM_SETSEL,
							(WPARAM) length,
							(LPARAM) length);
					}
				} break;
				}
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

#if defined (GTK_USER_INTERFACE)
static void command_entered_gtk(GtkEntry *entry, gpointer command_window_void)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called when a command is entered in the command entry area.
==============================================================================*/
{
	ENTER(command_entered_gtk);
	struct Command_window *command_window=(struct Command_window *)command_window_void;
	if (command_window)
	{
		char *command = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
		if (command)
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
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Command_window_gtk_button_press(GtkWidget *widget,
	GdkEventButton *event, gpointer command_window_void)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called when a command is entered in the command entry area.
==============================================================================*/
{
	gboolean return_code = FALSE;;
#if GTK_MAJOR_VERSION >= 2
	gchar *selected_text;
	gint buffer_x, buffer_y;
	GtkTextIter end, start;
	GtkTextView *history_view;
#endif /* GTK_MAJOR_VERSION >= 2 */
	struct Command_window *command_window;

	ENTER(Command_window_gtk_button_press);
	USE_PARAMETER(widget);
	command_window=(struct Command_window *)command_window_void;
	if (command_window)
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
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
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
	if (NULL != (command_window=(struct Command_window *)command_window_void))
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
#endif /* defined (GTK_USER_INTERFACE) */

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
	const char *current_token;
	int return_code;
	struct Command_window *command_window;

	ENTER(modify_Command_window_out_file_close);
	USE_PARAMETER(dummy);
	return_code=0;
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
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
			command_window=(struct Command_window *)command_window_void;
			if (command_window != NULL)
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

struct TextCtrlMouseEventData
{
	int left;
	int d_click;
	long x, y;
};


class wxCommandWindow : public wxFrame
{
	Command_window *command_window;
	wxListBox *history_list;
	wxString SelectedCommand;
	wxString command;
	wxString blank;
	wxTextCtrl *output_list;
	int number;
	struct TextCtrlMouseEventData mouse_event_data;
	const char *name_string, *version_string, *date_string,
		*copyright_string, *build_string, *revision_string;

public:

  wxCommandWindow(Command_window *command_window):
	command_window(command_window)
  {
		 wxXmlInit_command_window();
		 command_window->wx_command_window = NULL;
		 wxXmlResource::Get()->LoadFrame(this,
				(wxWindow *)NULL, _T("CmguiCommandWindow"));
		 this->SetIcon(cmiss_icon_xpm);
		 output_list = XRCCTRL(*this,"OutputWindow", wxTextCtrl);
		 mouse_event_data.left = 0;
		 mouse_event_data.d_click = 0;
		 mouse_event_data.x = 0;
		 mouse_event_data.y = 0;
		 name_string = NULL;
		 version_string = NULL;
		 date_string= NULL;
		 copyright_string = NULL;
		 build_string = NULL;
		 revision_string = NULL;
		 output_list->Connect(wxEVT_LEFT_DOWN,
				wxMouseEventHandler(wxCommandWindow::OnOutputLeftClick), NULL, this);
		 output_list->Connect(wxEVT_LEFT_DCLICK,
				wxMouseEventHandler(wxCommandWindow::OnOutputDClick), NULL, this);
  }

  wxCommandWindow()
  {
  }


  ~wxCommandWindow()
  {
		if (name_string)
		{
			DEALLOCATE(name_string);
		}
		if (version_string)
		{
			DEALLOCATE(version_string);
		}
		if (date_string)
		{
			DEALLOCATE(date_string);
		}
		if (copyright_string)
		{
			DEALLOCATE(copyright_string);
		}
		if (build_string)
		{
			DEALLOCATE(build_string);
		}
		if (revision_string)
		{
			DEALLOCATE(revision_string);
		}
	}

	 void OutputListTripleClickEvent()
	 {
			long from, to, colume_no, line_no, line_length;
			output_list->GetSelection(&from, &to);
			output_list->PositionToXY(from, &colume_no, &line_no);
			line_length = output_list->GetLineLength(line_no);
			output_list->SetSelection(
				 output_list->XYToPosition(0, line_no),
				 output_list->XYToPosition(line_length, line_no));
			mouse_event_data.left = 0;
			mouse_event_data.d_click = 0;
	 }

	 void OnOutputLeftClick(wxMouseEvent &event)
	 {
			if (event.GetX() == mouse_event_data.x &&
				 event.GetY() == mouse_event_data.y)
			{
				 mouse_event_data.left++;
			}
			else
			{
				 mouse_event_data.left = 1;
				 mouse_event_data.d_click = 0;
				 mouse_event_data.x = event.GetX();
				 mouse_event_data.y = event.GetY();
				 event.Skip();
			}
#if defined (__WXMSW__)
			if (mouse_event_data.left == 2 && mouse_event_data.d_click == 1)
#else
			if (mouse_event_data.left == 3 && mouse_event_data.d_click == 1)
#endif
			{
				 OutputListTripleClickEvent();
			}
#if defined (__WXMSW__)
			else if (mouse_event_data.left > 2)
#else
			else if (mouse_event_data.left > 3)
#endif
			{
				 mouse_event_data.left = 1;
				 mouse_event_data.d_click = 0;
				 event.Skip();
			}
			else
			{
				 event.Skip();
			}
	 }

	 void OnOutputDClick(wxMouseEvent &event)
	 {
			if (!(event.GetX() == mouse_event_data.x &&
						event.GetY() == mouse_event_data.y))
			{
				 mouse_event_data.left = 0;
			}
			if (mouse_event_data.left == 0)
			{
				 mouse_event_data.d_click = 0 ;
			}
			else
			{
				 mouse_event_data.d_click++;
			}
			event.Skip();
	 }

	 void wx_Add_to_command_list(wxString command)
	 {
			history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
			history_list->Freeze();
			if(history_list->GetCount() == 0)
				 history_list->Append(command);
			else
				 history_list->SetString(history_list->GetCount()-1, command);
			history_list->Append(wxT(""));
			// make item visible
			history_list->SetSelection(history_list->GetCount()-1);
			history_list->Thaw();
	 }

	void SetCmguiStrings(const char *name_string_in, const char *version_string_in,
		const char *date_string_in, const char *copyright_string_in,
		const char *build_string_in, const char *revision_string_in)
	{
		name_string = duplicate_string(name_string_in);
		version_string = duplicate_string(version_string_in);
		date_string = duplicate_string(date_string_in);
		copyright_string = duplicate_string(copyright_string_in);
		build_string = duplicate_string(build_string_in);
		revision_string = duplicate_string(revision_string_in);
	}

private:

	 void SingleClick(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
			SelectedCommand = history_list->GetStringSelection();
			command_window->wx_command_line_text_ctrl->Clear();
			command_window->wx_command_line_text_ctrl->WriteText(SelectedCommand);
	 }

	 void DoubleClick(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			history_list = XRCCTRL(*this, "CommandHistory", wxListBox);
			SelectedCommand = history_list->GetStringSelection();
			number = history_list->GetCount();
			Execute_command_execute_string(command_window->execute_command,
				SelectedCommand.mb_str(wxConvUTF8));
			history_list->SetSelection ( history_list->GetCount() - 1 );
			history_list->Deselect( history_list->GetCount() - 1 );
	 }

	 void threeDwindow(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx cre win");
	 }

	 void materialeditor(wxCommandEvent &event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx create graphical_material_editor");
	 }
	 void sceneeditorwindow(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx edit scene");
	 }

	 void opencom(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "open comfile");
	 }

	 void readcurve(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx read curve");
	 }

	 void readdata(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx read data");
	 }

	 void readelements(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx read elements");
	 }

	 void readnodes(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx read nodes");
	 }

	 void writeall(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx write all");
	 }

	 void writecurve(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx write curve all");
	 }

	 void writedata(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx write data");
	 }

	 void writeelements(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx write elements");
	 }

	 void writenodes(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx write nodes");
	 }

	 void dataviewer(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx create data_viewer");
	 }

	 void nodeviewer(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx create node_viewer");
	 }

	 void elementpointviewer(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx create element_point_viewer");
	 }

	 void sceneeditor(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx edit scene");
	 }

	 void spectrumeditor(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
			Execute_command_execute_string(command_window->execute_command,
				 "gfx edit spectrum");
	 }

	 void OnFormatFont(wxCommandEvent& event)
	 {
			wxFontData fdata;
			wxFont font;
			wxColour colour;
		USE_PARAMETER(event);
			history_list = XRCCTRL(*this,"CommandHistory", wxListBox);
			font = history_list->GetFont();
			fdata.SetInitialFont(font);
			colour = history_list->GetForegroundColour();
			fdata.SetColour(colour);
			fdata.SetShowHelp(true);
			wxFontDialog *FontDlg = new wxFontDialog(this, fdata);
			if(FontDlg->ShowModal() == wxID_OK)
			{
				 fdata = FontDlg->GetFontData();
				 font = fdata.GetChosenFont();
				 output_list->SetFont(font);
				 command_window->wx_command_line_text_ctrl->SetFont(font);
				 history_list->SetFont(font);
				 output_list->SetForegroundColour(fdata.GetColour());
				 command_window->wx_command_line_text_ctrl->SetForegroundColour(fdata.GetColour());
				 history_list->SetForegroundColour(fdata.GetColour());
			}
	 }

	 void ShowSimpleAboutDialog(wxCommandEvent& event)
	 {
		 char *temp_string;
		 const char *license_string = "\nThis software is part of CMISS and originally developed \nat The University of Auckland.\n";
		 USE_PARAMETER(event);
		 wxAboutDialogInfo info;
		 info.SetName(_T("cmgui"));

		 if (copyright_string && ALLOCATE(temp_string, char, strlen(copyright_string) + strlen(license_string) + 1))
		 {
			 strcpy(temp_string, copyright_string);
			 strcat(temp_string, license_string);
			 info.SetDescription(wxString::FromAscii(temp_string));
			 DEALLOCATE(temp_string);
		 }
		 else
		 {
			 info.SetDescription(wxString::FromAscii(license_string));
		 }
		 if (build_string && revision_string && date_string &&
			 ALLOCATE(temp_string, char, strlen(build_string) + strlen(revision_string) + strlen(date_string) + 23))
		 {
			 strcpy(temp_string, "Build_information: ");
			 strcat(temp_string, build_string);
			 strcat(temp_string, "\n");
			 strcat(temp_string, revision_string);
			 strcat(temp_string, "\n");
			 strcat(temp_string, date_string);
			 info.SetCopyright(wxString::FromAscii(temp_string));
			 DEALLOCATE(temp_string);
		 }
		 if (version_string)
		 {
			 info.SetVersion(wxString::FromAscii(version_string));
		 }
		 info.SetWebSite(wxT("http://www.cmiss.org/cmgui"));
		 info.SetLicense(wxString::FromAscii(CMISS_LICENSE_STRING));
		 wxAboutBox(info);
	 }

	void ShowBugReport(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		wxLaunchDefaultBrowser(
			_T("https://tracker.physiomeproject.org/enter_bug.cgi?product=cmgui&format=software"));
	}

void Terminate(wxCloseEvent& event)
{
	USE_PARAMETER(event);
	wxMessageDialog *dlg = new wxMessageDialog(NULL,wxT("Are you sure you want to quit cmgui?"),
		wxT("Exit Confirmation"), wxYES_NO|wxICON_QUESTION|wxSTAY_ON_TOP);
	if ( dlg->ShowModal() == wxID_YES)
		Execute_command_execute_string(command_window->execute_command, "QUIT");
	//else: dialog was cancelled or some another button pressed
	dlg->Destroy();
}

void Exit(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	Execute_command_execute_string(command_window->execute_command, "QUIT");
}

   DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxCommandWindow, wxFrame)
	 EVT_LISTBOX(XRCID("CommandHistory"),wxCommandWindow::SingleClick)
	 EVT_LISTBOX_DCLICK(XRCID("CommandHistory"),wxCommandWindow::DoubleClick)
	 EVT_MENU(XRCID("GraphicsthreeDWindow"),wxCommandWindow::threeDwindow)
	 EVT_MENU(XRCID("GraphicsSceneeditor"),wxCommandWindow::sceneeditorwindow)
	 EVT_MENU(XRCID("FileComFile"),wxCommandWindow::opencom)
	 EVT_MENU(XRCID("ReadCurve"),wxCommandWindow::readcurve)
	 EVT_MENU(XRCID("ReadData"),wxCommandWindow::readdata)
	 EVT_MENU(XRCID("ReadElements"),wxCommandWindow::readelements)
	 EVT_MENU(XRCID("ReadNode"),wxCommandWindow::readnodes)
	 EVT_MENU(XRCID("WriteAll"),wxCommandWindow::writeall)
	 EVT_MENU(XRCID("WriteCurve"),wxCommandWindow::writecurve)
	 EVT_MENU(XRCID("WriteData"),wxCommandWindow::writedata)
	 EVT_MENU(XRCID("WriteElements"),wxCommandWindow::writeelements)
	 EVT_MENU(XRCID("WriteNode"),wxCommandWindow::writenodes)
	 EVT_MENU(XRCID("ModelDataViewer"),wxCommandWindow::dataviewer)
	 EVT_MENU(XRCID("ModelNodeviewer"),wxCommandWindow::nodeviewer)
	 EVT_MENU(XRCID("ModelElementpointviewer"),wxCommandWindow::elementpointviewer)
	 EVT_MENU(XRCID("GraphicsMaterialeditor"),wxCommandWindow::materialeditor)
	 EVT_MENU(XRCID("GraphicsSceneeditor"),wxCommandWindow::sceneeditor)
	 EVT_MENU(XRCID("GraphicsSpectrumeditor"),wxCommandWindow::spectrumeditor)
	 EVT_MENU(XRCID("FontCmgui"),wxCommandWindow::OnFormatFont)
	 EVT_MENU(XRCID("AboutCmgui"),wxCommandWindow::ShowSimpleAboutDialog)
	 EVT_MENU(XRCID("BugReport"),wxCommandWindow::ShowBugReport)
	 EVT_CLOSE(wxCommandWindow::Terminate)
	 EVT_MENU(XRCID("MenuExit"),wxCommandWindow::Exit)
END_EVENT_TABLE()

void wxCommandLineTextCtrl::Reset(const char *command_prompt)
{
	 this->ChangeValue(wxString::FromAscii(command_prompt));
	 this->SetInsertionPointEnd();
}

void wxCommandLineTextCtrl::OnKeyDown(wxKeyEvent& event)
{
	 wxString SelectedCommand;
	 wxListBox *history_list = XRCCTRL(*command_window->wx_command_window,
			"CommandHistory", wxListBox);
	 int selection = history_list->GetSelection();
	 int number_of_items = history_list->GetCount();
	 int key_code = event.GetKeyCode();
	 switch (key_code)
	 {
			case WXK_DOWN:
				 if (selection == wxNOT_FOUND)
				 {
						history_list->SetSelection(number_of_items - 1);
						this->Clear();
				 }
				 else if (number_of_items>selection+1)
				 {
						history_list->SetSelection(selection+1);
						SelectedCommand = history_list->GetStringSelection();
						this->ChangeValue(SelectedCommand);
				 }
				 history_list->SetFocus();
				 this->SetFocus();
				 this->SetInsertionPointEnd();
				 break;
			case WXK_UP:
				 if (selection == wxNOT_FOUND)
				 {
						history_list->SetSelection(number_of_items - 2);
				 }
				 else if (selection-1>=0)
				 {
						history_list->SetSelection(selection-1);
				 }
				 SelectedCommand = history_list->GetStringSelection();
				 history_list->SetFocus();
				 this->ChangeValue(SelectedCommand);
				 this->SetFocus();
				 this->SetInsertionPointEnd();
				 break;
			case WXK_HOME:
				 if (event.ControlDown())
				 {
						history_list->SetFocus();
						history_list->SetSelection(0);
						SelectedCommand = history_list->GetStringSelection();
						this->ChangeValue(SelectedCommand);
						this->SetFocus();
						this->SetInsertionPointEnd();
				 }
				 else
						event.Skip();
				 break;
			case WXK_END:
				 if (event.ControlDown())
				 {
						history_list->SetFocus();
						history_list->SetSelection(number_of_items-1);
						this->Clear();
						this->SetFocus();
						this->SetInsertionPointEnd();
				 }
				 else
						event.Skip();
				 break;
			default:
				 event.Skip();
	 }

}

void wxCommandLineTextCtrl::OnCommandEntered(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	wxString command_string = this->GetValue();
	Execute_command_execute_string(command_window->execute_command, command_string.mb_str(wxConvUTF8));
}

BEGIN_EVENT_TABLE(wxCommandLineTextCtrl, wxTextCtrl)
	 EVT_KEY_DOWN(wxCommandLineTextCtrl::OnKeyDown)
	 EVT_TEXT_ENTER(wxID_ANY, wxCommandLineTextCtrl::OnCommandEntered)
END_EVENT_TABLE()
#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/


struct Command_window *CREATE(Command_window)(
	struct Execute_command *execute_command,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION:
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	struct Command_window *command_window;
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
	BOOL win32_return_code;
	static const char *class_name="Command_window";
	WNDCLASSEX class_information;
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
	GtkTextIter end_iterator;
#endif /* GTK_MAJOR_VERSION >= 2 */
	GtkWidget *history_scroll, *output_scroll, *vbox, *vpaned;
#endif /* switch (USER_INTERFACE) */

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
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
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
				command_window->shell = gtk_window_new(GTK_WINDOW_TOPLEVEL);
				if (command_window->shell)
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
			command_window->wx_command_window = new
			  wxCommandWindow(command_window);
			if (ALLOCATE(command_window->command_prompt, char , 1))
			{
				 *command_window->command_prompt = 0;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Command_window).  Insufficient memory for command_window prompt");
			}
			command_window->wx_command_window->Show();
			command_window->output_window =
			  XRCCTRL(*command_window->wx_command_window, "OutputWindow", wxTextCtrl);
			command_window->output_window->SetSize(wxSize(400,-1));
			command_window->output_window->SetDefaultStyle(wxTextAttr(wxNullColour, wxNullColour,
					wxFont(10, wxDEFAULT, wxNORMAL, wxNORMAL, false)));
			command_window->history_window =
				 XRCCTRL(*command_window->wx_command_window, "CommandHistory", wxListBox);
			command_window->output_window->SetSize(wxSize(400,-1));
			command_window->output_window->Layout();
			command_window->lower_panel =
			  XRCCTRL(*command_window->wx_command_window, "LowerPanel", wxPanel);
			command_window->lower_panel->Layout();
			command_window->frame =
			  XRCCTRL(*command_window->wx_command_window, "CmguiCommandWindow", wxFrame);
			command_window->frame->SetSize(wxSize(400,600));
			command_window->frame->SetMinSize(wxSize(1,1));
			wxSplitterWindow *splitter_window =
				 XRCCTRL(*command_window->wx_command_window, "CommandSplitterWindow",
						wxSplitterWindow);
			splitter_window->SetSashPosition(220);
			wxPanel *new_panel = XRCCTRL(*command_window->wx_command_window, "CommandLinePanel", wxPanel);
			command_window->wx_command_line_text_ctrl = new wxCommandLineTextCtrl(command_window, new_panel);
			command_window->wx_command_line_text_ctrl->SetFocus();
			new_panel->SetSize(600,25);
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
#if defined (WX_USER_INTERFACE)
		delete command_window->wx_command_window;
		if (command_window->command_prompt)
			 DEALLOCATE(command_window->command_prompt);
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		if (command_window->command_prompt)
			 DEALLOCATE(command_window->command_prompt);
		/* Set the window proc back to the default so that we don't
			get any more messages, particularly the close message, we
			do not necessarily want to quit the whole program */
		SetWindowLong(command_window->dialog,
			GWL_WNDPROC, (long)DefWindowProc);
		DestroyWindow(command_window->dialog);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
		g_signal_handler_disconnect (G_OBJECT(command_window->shell),
			command_window->close_handler_id);
#endif /* GTK_MAJOR_VERSION >= 2 */
		gtk_widget_destroy (command_window->shell);
#endif /* (WX_USER_INTERFACE) */
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

int add_to_command_list(const char *command,struct Command_window *command_window)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/
{
	int return_code;
#if defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
	GtkTextIter end_iterator;
#else /* GTK_MAJOR_VERSION >= 2 */
	guint text_length;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* !defined (GTK_USER_INTERFACE) */


	ENTER(add_to_command_list);
/*???debug */
/* printf("enter add_to_command_list\n  %s\n",command); */
#if defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION < 2
	USE_PARAMETER(command);
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* defined (GTK_USER_INTERFACE) */
	if (command_window)
	{
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		SendMessage(command_window->command_history, LB_ADDSTRING, 0,
			(LPARAM)command);
#elif defined (WX_USER_INTERFACE)
		command_window->wx_command_window->wx_Add_to_command_list(wxString::FromAscii(command));
		return_code = 1;
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
	const char *prompt)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <command_window>.
==============================================================================*/
{
	int return_code;

	ENTER(set_command_prompt);
#if defined (GTK_USER_INTERFACE)
	USE_PARAMETER(prompt);
#endif /* switch (GTK_USER_INTERFACE) */
	if (command_window)
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)/* switch (USER_INTERFACE) */
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
#if defined (GTK_USER_INTERFACE)
		gtk_entry_set_text(GTK_ENTRY(command_window->entry),
			command_window->command_prompt);
		gtk_editable_set_position(GTK_EDITABLE(command_window->entry), -1);
#elif defined (WX_USER_INTERFACE)
		if (command_window->wx_command_line_text_ctrl)
			 command_window->wx_command_line_text_ctrl->Reset(command_window->command_prompt);
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
	const char *command_string)
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
#if defined (WX_USER_INTERFACE)
		if (command_window->wx_command_line_text_ctrl)
		{

			 command_window->wx_command_line_text_ctrl->Reset(command_string);
		}
#endif /* defined (WX_USER_INTERFACE) */
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

int write_command_window(const char *message,struct Command_window
*command_window)
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/
{
	int return_code;
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
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
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
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
		if (command_window->output_window)
		{
			 command_window->output_window->AppendText(wxString::FromAscii(message));
			 command_window->output_window->SetInsertionPointEnd();
			 return_code = 1;
		}
#endif /* switch (USER_INTERFACE) */
		if (command_window->out_file &&
			(command_window->out_file_mode & OUTFILE_OUTPUT))
		{
			fprintf(command_window->out_file, "%s", message);
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

int Command_window_set_cmgui_string(struct Command_window *command_window,
	const char *name_string, const char *version_string,const char *date_string,
	const char *copyright_string, const char *build_string, const char *revision_string)
{
	int return_code = 0;
	if (command_window)
	{
#if defined (WX_USER_INTERFACE)
		if (command_window->wx_command_window)
		{
			command_window->wx_command_window->SetCmguiStrings(name_string, version_string, date_string,
				copyright_string, build_string, revision_string);
		}
#else
		USE_PARAMETER(name_string);
		USE_PARAMETER(date_string);
		USE_PARAMETER(copyright_string);
		USE_PARAMETER(build_string);
		USE_PARAMETER(revision_string);
		USE_PARAMETER(version_string);
#endif /* switch (WX_USER_INTERFACE) */
		return_code = 1;
	}

	return return_code;
}
