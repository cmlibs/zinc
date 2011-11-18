/*******************************************************************************
FILE : confirmation.c

LAST MODIFIED : 17 December 2002

DESCRIPTION :
Routines for waiting for user input.
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
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

extern "C" {
#if defined (UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
#include <direct.h>
#endif /* defined (WIN32_SYSTEM) */
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "user_interface/user_interface.h"
#include "user_interface/confirmation.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
}
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#endif /* defined (WX_USER_INTERFACE)*/


/*
Module types
------------
*/
enum Confirm_type
{
	WARNING_OK_CANCEL,
	ERROR_OK,
	INFORMATION_OK,
	WARNING_OK,
	QUESTION_YES_NO
}; /* enum Confirm_type */

enum Confirm_filetype
{
	CONFIRM_CHANGE_DIRECTORY,
	CONFIRM_READ,
	CONFIRM_WRITE
}; /* enum Confirm_filetype */

struct Confirmation
{
	int dialog_active;
	int response;
}; /* struct Confirmation */

struct Confirmation_string
{
	int dialog_active;
	char *response;
}; /* struct Confirmation */

struct File_confirmation
{
	int dialog_active;
	int response;
	char *filename;
}; /* struct Confirmation */

/*
Module functions
----------------
*/

static int confirmation(enum Confirm_type type,const char *title,const char *prompt,
	 struct User_interface *user_interface 
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
												)
/*******************************************************************************
LAST MODIFIED : 17 December 2002

DESCRIPTION :
This routine supplies a dialog window which requires a response before anything
else will continue and returns 1 if the OK button is clicked and 0 if the cancel
button is clicked.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation);
	USE_PARAMETER(type);
	USE_PARAMETER(title);
	USE_PARAMETER(prompt);
#if defined (WX_USER_INTERFACE)
	USE_PARAMETER(execute_command);
#endif /* defined (WX_USER_INTERFACE) */
	return_code=0;
	if (user_interface)
	{
#if defined (WIN32_USER_INTERFACE)
		switch (type)
		{
			case ERROR_OK:
			{
				return_code=MessageBox((HWND)NULL,prompt,title,
					MB_OK|MB_ICONERROR|MB_APPLMODAL|MB_TOPMOST);
				if (IDOK==return_code)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			case INFORMATION_OK:
			{
				return_code=MessageBox((HWND)NULL,prompt,title,
					MB_OK|MB_ICONINFORMATION|MB_APPLMODAL|MB_TOPMOST);
				if (IDOK==return_code)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			case QUESTION_YES_NO:
			{
				return_code=MessageBox((HWND)NULL,prompt,title,
					MB_YESNO|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);
				if (IDYES==return_code)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			case WARNING_OK:
			{
				return_code=MessageBox((HWND)NULL,prompt,title,
					MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);
				if (IDOK==return_code)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			case WARNING_OK_CANCEL:
			{
				return_code=MessageBox((HWND)NULL,prompt,title,
					MB_OKCANCEL|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);
				if (IDOK==return_code)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
		}
#endif /* defined (WIN32_USER_INTERFACE) */
	}
	LEAVE;

	return (return_code);
} /* confirmation */

static int confirmation_get_filename_callback(char *file_name,void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct File_confirmation *confirmation;

	ENTER(confirmation_get_filename_callback);
	confirmation=(struct File_confirmation *)data_void;
	if (confirmation != NULL)
	{
		confirmation->response=1;
		confirmation->dialog_active=0;
		if (ALLOCATE(confirmation->filename,char,strlen(file_name)+1))
		{
			strcpy(confirmation->filename,file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"confirmation_get_filename_callback.  Unable to allocate filename");
			confirmation->filename=(char *)NULL;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
	"confirmation_get_filename_callback.  Could not get confirmation structure");
	}
	LEAVE;

	return (return_code);
} /* confirmation_get_filename_callback */

static int confirmation_cancel_filename_callback(void *data_void)
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct File_confirmation *confirmation;

	ENTER(confirmation_cancel_filename_callback);
	confirmation=(struct File_confirmation *)data_void;
	if (confirmation != NULL)
	{
		confirmation->response=0;
		confirmation->dialog_active=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"confirmation_cancel_filename_callback.  Could not get confirmation structure");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* confirmation_cancel_filename_callback */

static char *confirmation_get_filename(enum Confirm_filetype type,
	const char *extension,struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 8 September 2000

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/
{
	char *filename;
	struct File_confirmation confirmation;
	struct File_open_data *file_open_data;

	ENTER(confirmation_get_filename);
	filename=(char *)NULL;
	if (user_interface)
	{
		/* open a file selection box */
		file_open_data=(struct File_open_data *)NULL;
		switch (type)
		{
			case CONFIRM_READ:
			case CONFIRM_WRITE:
			{
				file_open_data=create_File_open_data(extension,REGULAR,
					confirmation_get_filename_callback,(void *)&confirmation,0,
					 user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
			} break;
			case CONFIRM_CHANGE_DIRECTORY:
			{
				file_open_data=create_File_open_data((char *)NULL,DIRECTORY,
					confirmation_get_filename_callback,(void *)&confirmation,0,
					 user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
																						 );
			} break;
		}
		if (file_open_data)
		{
			register_file_cancel_callback(file_open_data,
				confirmation_cancel_filename_callback,(void *)&confirmation);
			confirmation.response=1;
			confirmation.dialog_active=1;
			confirmation.filename=(char *)NULL;
			switch (type)
			{
				case CONFIRM_READ:
				{
					open_file_and_read(
#if defined (WIN32_USER_INTERFACE)
						file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
						file_open_data
#endif /* defined (WX_USER_INTERFACE) */
						);
				} break;
				case CONFIRM_CHANGE_DIRECTORY:
				case CONFIRM_WRITE:
				{
					open_file_and_write(
#if defined (WIN32_USER_INTERFACE)
						file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
						file_open_data
#endif /* defined (WX_USER_INTERFACE) */
						);
				} break;
			}
#if defined (WIN32_USER_INTERFACE)
			/*???DB.  For WIN32_USER_INTERFACE the event loop is inside open_file_and_*.  Should
				probably be moved here ? */
#endif /* defined (WIN32_USER_INTERFACE) */
			if ((confirmation.response)&&(confirmation.filename))
			{
				switch (type)
				{
					case CONFIRM_READ:
					case CONFIRM_WRITE:
					{
						filename=confirmation.filename;
					} break;
					case CONFIRM_CHANGE_DIRECTORY:
					{
						if (confirmation.filename)
						{
							mkdir(confirmation.filename
#if defined (UNIX)
								,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#endif /* defined (UNIX) */
								);
							if (0==chdir(confirmation.filename))
							{
								filename=confirmation.filename;
							}
						}
					} break;
				}
			}
			/*???DB.  The X events to do with processing the closing of the dialog
				haven't necessarily been completed */
#if defined (WX_USER_INTERFACE)
			{
				 destroy_File_open_data(&file_open_data);
			}
#endif /*defined (WX_USER_INTERFACE)*/
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"confirmation_get_filename.  Could not allocate file open data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"confirmation_get_filename.  Invalid argument(s).  %d (%d) %p %p",type,
			CONFIRM_CHANGE_DIRECTORY,extension,user_interface);
	}
	LEAVE;

	return (filename);
} /* confirmation_get_filename */

/*
Global functions
----------------
*/
int confirmation_warning_ok_cancel(const char *title,const char *prompt,
	 struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns 1 if the OK button
is clicked and 0 if the cancel button is clicked.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation_warning_ok_cancel);
	return_code=confirmation(WARNING_OK_CANCEL,title,prompt,
		 user_interface
#if defined (WX_USER_INTERFACE) 
		 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (return_code);
} /* confirmation_warning_ok_cancel */

int confirmation_error_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a error dialog window which requires a response before
anything else will continue and returns when the OK button is clicked.  No other
options are supplied.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation_error_ok);
	return_code=confirmation(ERROR_OK,title,prompt,
		user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (return_code);
} /* confirmation_error_ok */

int confirmation_information_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a information dialog window which requires a response
before anything else will continue and returns when the OK button is clicked.
No other options are supplied.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation_information_ok);
	return_code=confirmation(INFORMATION_OK,title,prompt,
		 user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (return_code);
} /* confirmation_information_ok */

int confirmation_warning_ok(const char *title,const char *prompt,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a warning dialog window which requires a response before
anything else will continue and returns when the OK button is clicked.  No other
options are supplied.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation_warning_ok);
	return_code=confirmation(WARNING_OK,title,prompt,
		 user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (return_code);
} /* confirmation_warning_ok */

int confirmation_question_yes_no(char *title,char *prompt,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue.  It returns one if the Yes button
is clicked and No if it isn't.
==============================================================================*/
{
	int return_code;

	ENTER(confirmation_question_yes_no);
	return_code=confirmation(QUESTION_YES_NO,title,prompt,
		user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (return_code);
} /* confirmation_question_yes_no */

char *confirmation_get_read_filename(const char *extension,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/
{
	char *filename;

	ENTER(confirmation_get_read_filename);
	filename=confirmation_get_filename(CONFIRM_READ,extension,user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (filename);
} /* confirmation_get_read_filename */

char *confirmation_get_write_filename(const char *extension,
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/
{
	char *filename;

	ENTER(confirmation_get_write_filename);
	filename=confirmation_get_filename(CONFIRM_WRITE,extension,user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (filename);
} /* confirmation_get_write_filename */

char *confirmation_change_current_working_directory(
	struct User_interface *user_interface
#if defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
This routine supplies a file selection dialog window for changing the current
working directory.  The new directory will be created if necessary.
==============================================================================*/
{
	char *filename;

	ENTER(confirmation_change_current_working_directory);
	filename=confirmation_get_filename(CONFIRM_CHANGE_DIRECTORY,(char *)NULL,
		user_interface
#if defined (WX_USER_INTERFACE) 
					 , execute_command
#endif /* defined (WX_USER_INTERFACE) */
);
	LEAVE;

	return (filename);
} /* confirmation_change_current_working_directory */
