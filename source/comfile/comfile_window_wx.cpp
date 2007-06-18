/*******************************************************************************
FILE : comfile_window.c

LAST MODIFIED : 18 April 2002

DESCRIPTION :
Management routines for the comfile window.
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
#include <string.h>
#include "general/debug.h"
#include "command/command.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include <wx/fontdlg.h>
extern "C" {
#include "comfile/comfile_window_wx.h"
}
#include "comfile/comfile_window_wx.xrch"
#endif /*defined (WX_USER_INTERFACE)*/

/*
Module types
------------
*/

#if defined (WX_USER_INTERFACE)
class wxComfileWindow;
#endif /* defined (WX_USER_INTERFACE) */


struct Comfile_window
/*******************************************************************************
LAST MODIFIED : 5 April 2007

DESCRIPTION :
Command file or "comfile" window structure which lists a cmiss command file and
by user selection passes commands to the command window.
???DB.  Should there be a "window_structure" which contains shell information
etc ?
==============================================================================*/
{
	char *name;
	/* need to keep comfile window manager so window can be destroyed by self */
	struct MANAGER(Comfile_window) *comfile_window_manager;
	char *file_name;
	struct IO_stream_package *io_stream_package;
	int number_of_commands;
	char **commands;
	struct User_interface *user_interface;
	/* for executing commands */
	struct Execute_command *execute_command;
	/* for setting command text ready to edit and enter */
	struct Execute_command *set_command;
	/* the number of objects accessing this window. The window cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
#if defined (WX_USER_INTERFACE)
	wxComfileWindow *wx_comfile_window;
#endif /* (WX_USER_INTERFACE) */
}; /* struct Comfile_window */

FULL_DECLARE_INDEXED_LIST_TYPE(Comfile_window);

FULL_DECLARE_MANAGER_TYPE(Comfile_window);

/*
Module typess
----------------
*/
#if defined (WX_USER_INTERFACE)
class wxComfileWindow;
#endif /* defined (WX_USER_INTERFACE) */

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Comfile_window, name, char *, strcmp)
DECLARE_LOCAL_MANAGER_FUNCTIONS(Comfile_window)

#if defined (WX_USER_INTERFACE)
class wxComfileWindow : public wxFrame
{	
	Comfile_window *comfile_window;
	 wxListBox *comfile_listbox;
	 wxButton *all_button, *selectedbutton, *close_button;
	 wxFrame *this_frame;
	 wxString blank;	 
	 int number;
	 wxString selectedcommand;
	 wxFont comfile_font;
	 wxColour comfile_colour;
public:

	 wxComfileWindow(Comfile_window *comfile_window): 
			comfile_window(comfile_window)
	 {
			wxXmlInit_comfile_window_wx();
			comfile_window->wx_comfile_window = (wxComfileWindow *)NULL;
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiComfileWindow"));
			comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
			this_frame = XRCCTRL(*this, "CmguiComfileWindow", wxFrame);
			char **command,*line, *temp_string;
			struct IO_stream *comfile;
			int i,number_of_commands;
			wxString blank = "";	 
			wxString command_string;
			
			if ((comfile_window->file_name)&&
				 (comfile=CREATE(IO_stream)(comfile_window->io_stream_package))&&
				 IO_stream_open_for_read(comfile, comfile_window->file_name))
			{
				 number_of_commands=0;
				 while ((EOF!=IO_stream_scan(comfile," "))&&!IO_stream_end_of_stream(comfile)&&
						IO_stream_read_string(comfile,"[^\n]",&line))
				 {
						if (command_string=trim_string(line))
						{
							 if (command_string)
							 {
									number = comfile_listbox->GetCount();
									if (number == 0)
										 comfile_listbox->InsertItems(1, &blank,number);
									number = comfile_listbox->GetCount();
									comfile_listbox->InsertItems(1,&command_string, number-1);
									number_of_commands++;
							 }
						}
						DEALLOCATE(line);
				 }
				 if (!IO_stream_end_of_stream(comfile))
				 {
						display_message(ERROR_MESSAGE,
							 "identify_command_list.  Error reading comfile");
				 }
				 if (number_of_commands>0)
				 {
						if (ALLOCATE(command,char *,number_of_commands))
						{
							 comfile_window->number_of_commands=number_of_commands;
							 comfile_window->commands=command;
							 for (i=number_of_commands;i>0;i--)
							 {
									*command=(char *)NULL;
									command++;
							 }
							 temp_string = NULL;
							 if (ALLOCATE(temp_string,char,(strlen(comfile_window->name) + 10)))
							 {
									strcpy(temp_string, "comfile: ");
									strcat(temp_string, comfile_window->name);
									temp_string[(strlen(comfile_window->name) + 9)]='\0';
									this_frame->SetTitle(temp_string);
									if (temp_string)
									{
										 DEALLOCATE(temp_string);
									}	
							 }	
						}
						else
						{
							 display_message(ERROR_MESSAGE,
									"identify_command_list.  Could not allocate memory for commands");
						}
				 }
			IO_stream_close(comfile);
			DESTROY(IO_stream)(&comfile);
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"identify_command_list.  Could not open file");
			}
			this_frame->SetSize(wxSize(800,600));		
			this_frame->SetMinSize(wxSize(20,20));
			Show();
	 };
	 
  wxComfileWindow()
  {
  };

	 void OnFormatFont(wxCommandEvent& event)
	 {
			wxFontData fdata;
			wxFont font;
			wxColour colour;

			comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
			font = comfile_listbox->GetFont();
			fdata.SetInitialFont(font);
			colour = comfile_listbox->GetForegroundColour();
			fdata.SetColour(colour);
			fdata.SetShowHelp(true);
			wxFontDialog *FontDlg = new wxFontDialog(this, fdata);

			if(FontDlg->ShowModal() == wxID_OK)
			{
				 fdata = FontDlg->GetFontData();
				 font = fdata.GetChosenFont();
				 comfile_listbox->SetFont(font);
				 comfile_listbox->SetForegroundColour(fdata.GetColour()); 
			}
	 } 

void SingleClickedOnList(wxCommandEvent &event)
	 {		
			int i,number_of_selected_commands;
			wxArrayInt selected_commands;
			comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
			if (number_of_selected_commands = comfile_listbox->GetSelections(selected_commands))
			{
				 /* execute the selected commands */
				 for (i=0;i<number_of_selected_commands;i++)
				 {
						/* retrieve the command string */
						selectedcommand = comfile_listbox->GetString(selected_commands[i]);
// 						wx_Display_on_command_list(const_cast<char *>(selectedcommand.c_str()));
				 }
			}
	 }

void DoubleClickedOnList(wxCommandEvent &event)
	 {
			int i,number_of_selected_commands;
			wxArrayInt selected_commands;
			comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
			if (number_of_selected_commands = comfile_listbox->GetSelections(selected_commands))
			{
				 /* execute the selected commands */
				 for (i=0;i<number_of_selected_commands;i++)
				 {
						/* retrieve the command string */
						selectedcommand = comfile_listbox->GetString(selected_commands[i]);
						Execute_command_execute_string(comfile_window->execute_command,					
							 const_cast<char *>(selectedcommand.c_str())) ;
				 }
			}
// 			wx_Display_on_command_list((char)NULL);
	 }

void AllClicked(wxCommandEvent &event)
{
	 int i,number_of_selected_commands;
	 comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
	 /* get the number of selected commands and their positions */
	 if (number_of_selected_commands = comfile_listbox->GetCount());
	 {
			/* execute the selected commands */
			for (i=0;i<number_of_selected_commands-1;i++)
			{
				 /* retrieve the command string */
				 selectedcommand = comfile_listbox->GetString(i);
				 Execute_command_execute_string(comfile_window->execute_command,					
						const_cast<char *>(selectedcommand.c_str())) ;
			}
	 }
// 	 wx_Display_on_command_list((char)NULL);
}

void SelectedClicked(wxCommandEvent &event)
{
	 wxArrayInt selected_commands;
	 int i,number_of_selected_commands;
	 comfile_listbox = XRCCTRL(*this, "ComfileListBox", wxListBox);
	 /* get the number of selected commands and their positions */
	 if (number_of_selected_commands = comfile_listbox->GetSelections(selected_commands))
	 {
			/* execute the selected commands */
			for (i=0;i<number_of_selected_commands;i++)
			{
				 /* retrieve the command string */
				 selectedcommand = comfile_listbox->GetString(selected_commands[i]);
				 Execute_command_execute_string(comfile_window->execute_command,					
						const_cast<char *>(selectedcommand.c_str())) ;
			}
	 }
}


void CloseClicked(wxCommandEvent &event)
{
	 this_frame = XRCCTRL(*this, "CmguiComfileWindow", wxFrame);
	 this_frame->Destroy();
}

  DECLARE_DYNAMIC_CLASS(wxComfileWindow);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxComfileWindow, wxFrame)
BEGIN_EVENT_TABLE(wxComfileWindow, wxFrame)
	 EVT_MENU(XRCID("FontSettings"),wxComfileWindow::OnFormatFont)
	 EVT_LISTBOX(XRCID("ComfileListBox"),wxComfileWindow::SingleClickedOnList)
	 EVT_LISTBOX_DCLICK(XRCID("ComfileListBox"),wxComfileWindow::DoubleClickedOnList)
	 EVT_BUTTON(XRCID("AllButton"),wxComfileWindow::AllClicked)
	 EVT_BUTTON(XRCID("SelectedButton"),wxComfileWindow::SelectedClicked)
	 EVT_BUTTON(XRCID("CloseButton"),wxComfileWindow::CloseClicked)
END_EVENT_TABLE()
#endif /* defined (WX_USER_INTERFACE) */
/*
Global functions
----------------
*/
#if defined (WX_USER_INTERFACE)
struct Comfile_window *CREATE(Comfile_window)(char *name,
	char *file_name, struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 7 April 2007

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/
{
	struct IO_stream *comfile;
	struct Comfile_window *comfile_window;

	ENTER(CREATE(Comfile_window));
	if (name && file_name && (comfile = CREATE(IO_stream)(io_stream_package))
		&& IO_stream_open_for_read(comfile,file_name) &&
		execute_command && set_command && user_interface)
	{
		IO_stream_close(comfile);
		DESTROY(IO_stream)(&comfile);
		if (ALLOCATE(comfile_window, struct Comfile_window, 1) &&
			 (comfile_window->name = duplicate_string(name)) &&
			 (comfile_window->file_name = duplicate_string(file_name)))
		{
			 comfile_window->io_stream_package = io_stream_package;
			 comfile_window->comfile_window_manager=
					(struct MANAGER(Comfile_window) *)NULL;
			 comfile_window->number_of_commands=0;
			 comfile_window->commands=(char **)NULL;
			 comfile_window->execute_command=execute_command;
			 comfile_window->set_command=set_command;
			 comfile_window->user_interface=user_interface;
			 comfile_window->access_count = 0;
			 /* create the window shell */
#if defined (WX_USER_INTERFACE)
			 comfile_window->wx_comfile_window = (wxComfileWindow *)NULL;
			 comfile_window->wx_comfile_window = new 
					wxComfileWindow(comfile_window);
#endif /* defined (WX_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Comfile_window).  Could not open hierarchy");
			comfile_window=(struct Comfile_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
		comfile_window=(struct Comfile_window *)NULL;
	}
	LEAVE;

	return (comfile_window);
} /* CREATE(Comfile_window) */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
int DESTROY(Comfile_window)(struct Comfile_window **comfile_window_address)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION:
Frees the contents of the Comfile_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the comfile_window from a global list of windows is left with the
calling routine. See also Comfile_window_close_CB and
Comfile_window_destroy_CB.
==============================================================================*/
{
	char **command;
	int  return_code;
	struct Comfile_window *comfile_window;

	ENTER(DESTROY(Comfile_window));
	if (comfile_window_address && (comfile_window = *comfile_window_address))
	{
		/* destroy the comfile window widget */
		 delete comfile_window->wx_comfile_window;
		/* free the memory for the file name */
		DEALLOCATE(comfile_window->file_name);
		DEALLOCATE(comfile_window->name);
		/* free the memory for the commands */
		if (command = comfile_window->commands)
		{
			 DEALLOCATE(comfile_window->commands);
		}
		DEALLOCATE(*comfile_window_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Comfile_window).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Comfile_window) */
#endif /* defined (WX_USER_INTERFACE) */

DECLARE_OBJECT_FUNCTIONS(Comfile_window)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Comfile_window)

DECLARE_INDEXED_LIST_FUNCTIONS(Comfile_window)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Comfile_window, \
	name,char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Comfile_window,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Comfile_window,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Comfile_window,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name));
	if (source&&destination)
	{
		/*???RC have problems with copying scene_manager? messages? */
		printf("MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name).  "
			"Not used\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Comfile_window,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Comfile_window,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Comfile_window,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Comfile_window,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Comfile_window,name) */

/* NOTE: Using special ADD_OBJECT_TO_MANAGER function so that object keeps
	pointer to its manager while it is managed. */
DECLARE_MANAGER_FUNCTIONS(Comfile_window)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Comfile_window)

DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS(Comfile_window,name, \
	char *,comfile_window_manager)

char *Comfile_window_manager_make_unique_name(
	struct MANAGER(Comfile_window) *comfile_window_manager, char *file_name)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Allocates and returns a name based on <file_name> that is not currently in use
in the <comfile_window_manager>. Does so by appending a number in angle
brackets.
Up to the calling routine to deallocate the returned string.
==============================================================================*/
{
	char *return_name, temp_string[20];
	int error, length, number;

	ENTER(Comfile_window_manager_make_unique_name);
	if (comfile_window_manager && file_name)
	{
		if (return_name = duplicate_string(file_name))
		{
			error = 0;
			length = strlen(file_name);
			number = 1;
		}
		else
		{
			error = 1;
		}
		while (return_name && ((struct Comfile_window *)NULL !=
			FIND_BY_IDENTIFIER_IN_MANAGER(Comfile_window,name)(
				return_name, comfile_window_manager)))
		{
			number++;
			return_name[length] = '\0';
			sprintf(temp_string, "<%d>", number);
			append_string(&return_name, temp_string, &error);
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Comfile_window_manager_make_unique_name.  Could not allocate name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Comfile_window_manager_make_unique_name.  Invalid argument(s)");
		return_name = (char *)NULL;
	}
	LEAVE;

	return (return_name);
}
