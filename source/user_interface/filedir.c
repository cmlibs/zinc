/*******************************************************************************
FILE : filedir.c

LAST MODIFIED : 23 April 2004

DESCRIPTION :
Routines for opening files using Motif widgets.
???DB.  Return sometimes doesn't work for writing ?  This is because Cancel is
the default button.
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
/* temp */
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/MessageB.h>
#include <Xm/List.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>

#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "user_interface/filedir.h"
#if defined (MOTIF)
static char filedir_uidh[] =
#include "user_interface/filedir.uidh"
	;
#endif /* defined (MOTIF) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Code switchs
------------
*/
/* if a file selection dialog changes directory then when the dialog is opened
	again it starts in the changed directory */
#define REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int filedir_hierarchy_open=0;
static MrmHierarchy filedir_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (MOTIF)
static void identify_file_selection_label(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Finds the id of a file selection label.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(identify_file_selection_label);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		file_open_data->selection_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_file_selection_label.  client_data missing");
	}
	LEAVE;
} /* identify_file_selection_label */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_file_selection_text(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Finds the id of a file selection text.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(identify_file_selection_text);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		file_open_data->selection_text= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_file_selection_text.  client_data missing");
	}
	LEAVE;
} /* identify_file_selection_text */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void destroy_file_selection(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Tidys up when the user destroys the file selection shell.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(destroy_file_selection);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		if (file_open_data->activation)
		{
			file_open_data->selection=(Widget)NULL;
			/* unghost the activation button */
			XtSetSensitive(file_open_data->activation,True);
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"destroy_file_selection.  activation missing");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_file_selection.  No client_data");
	}
	LEAVE;
} /* destroy_file_selection */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void close_file_selection(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 June 2003

DESCRIPTION :
Closes the windows associated with the file selection shell.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(close_file_selection);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		if (file_open_data->cancel_operation)
		{
			(file_open_data->cancel_operation)(file_open_data->cancel_arguments);
		}
		if (file_open_data->selection_shell)
		{
			busy_cursor_off(file_open_data->selection_shell,
				file_open_data->user_interface );
			/* close the selection shell */
			XtUnmanageChild(file_open_data->selection);
			if (file_open_data->activation)
			{
				/* unghost the activation button */
				XtSetSensitive(file_open_data->activation,True);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"close_file_selection.  selection_shell or activation missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_file_selection.  No client_data");
	}
	LEAVE;
} /* close_file_selection */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void cancel_file_selection(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 June 2003

DESCRIPTION :
Called in response to pressing the cancel button.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(cancel_file_selection);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		if (file_open_data->cancel_operation)
		{
			(file_open_data->cancel_operation)(file_open_data->cancel_arguments);
		}
		close_file_selection(widget,client_data,call_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,"cancel_file_selection.  No client_data");
	}
	LEAVE;
} /* cancel_file_selection */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void file_selection_read(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Reads the selected file in the user specified way.
==============================================================================*/
{
	char *file_name;
	struct File_open_data *file_open_data;
	XmFileSelectionBoxCallbackStruct *callback_data;

	ENTER(file_selection_read);
	USE_PARAMETER(widget);
	if (callback_data=(XmFileSelectionBoxCallbackStruct *)call_data)
	{
		if (XmCR_OK==callback_data->reason)
		{
			if (file_open_data=(struct File_open_data *)client_data)
			{
				if (file_open_data->operation)
				{
					/* get the file name */
					XmStringGetLtoR(callback_data->value,XmSTRING_DEFAULT_CHARSET,
						&file_name);
					busy_cursor_off(file_open_data->selection_shell,
						file_open_data->user_interface);
					/* perform the operation */
					if ((file_open_data->operation)(file_name,
						file_open_data->arguments))
					{
						/* close the file selection box */
						XtUnmanageChild(file_open_data->selection);
						if (file_open_data->activation)
						{
							/* unghost the activation button */
							XtSetSensitive(file_open_data->activation,True);
						}
					}
					else
					{
						busy_cursor_on(file_open_data->selection_shell,
							file_open_data->user_interface );
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_selection_read.  No file operation");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"file_selection_read.  No client_data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"file_selection_read.  Incorrect reason");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"file_selection_read.  No call_data");
	}
	LEAVE;
} /* file_selection_read */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void close_file_exists_warning(Widget widget,
	XtPointer warning_box_widget,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
==============================================================================*/
{
	Widget warning_box;

	ENTER(close_file_exists_warning);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (warning_box=(Widget)warning_box_widget)
	{
		XtUnmanageChild(warning_box);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_file_exists_warning.  Missing warning_box_widget");
	}
	LEAVE;
} /* close_file_exists_warning */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void overwrite_file(Widget widget,XtPointer void_file_open_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(overwrite_file);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)void_file_open_data)
	{
		/* perform the operation */
		if ((file_open_data->operation)(file_open_data->file_name,
			file_open_data->arguments))
		{
			/* close the file selection box */
			XtUnmanageChild(file_open_data->selection);
			if (file_open_data->activation)
			{
				/* unghost the activation button */
				XtSetSensitive(file_open_data->activation,True);
			}
		}
		else
		{
			busy_cursor_on(file_open_data->selection_shell,
				file_open_data->user_interface );
		}
		DEALLOCATE(file_open_data->file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"overwrite_file.  Invalid file_open_data");
	}
	LEAVE;
} /* overwrite_file */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void busy_cursor_off_warning_shell(Widget widget,
	XtPointer file_open_data_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Callback that switchs from the busy cursor to the default cursor for all shells
except the warning shell.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(busy_cursor_off_warning_shell);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)file_open_data_void)
	{
		busy_cursor_off(file_open_data->warning_shell,
			file_open_data->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"busy_cursor_off_warning_shell.  Missing file_open_data");
	}

	LEAVE;
} /* busy_cursor_off_warning_shell */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
static void busy_cursor_off_selection_shell(Widget widget,
	XtPointer file_open_data_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Callback that switchs from the busy cursor to the default cursor for all shells
except the selection shell.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(busy_cursor_off_selection_shell);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)file_open_data_void)
	{
		busy_cursor_off(file_open_data->selection_shell,
			file_open_data->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"busy_cursor_off_selection_shell.  Missing file_open_data");
	}

	LEAVE;
} /* busy_cursor_off_selection_shell */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
static void busy_cursor_on_selection_shell(Widget widget,
	XtPointer file_open_data_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Callback that switchs from the busy cursor to the default cursor for all shells
except the selection shell.
==============================================================================*/
{
	struct File_open_data *file_open_data;

	ENTER(busy_cursor_on_selection_shell);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)file_open_data_void)
	{
		busy_cursor_on(file_open_data->selection_shell,
			file_open_data->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"busy_cursor_on_selection_shell.  Missing file_open_data");
	}

	LEAVE;
} /* busy_cursor_on_selection_shell */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void file_selection_write(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes the selected file in the user specified way.
==============================================================================*/
{
	char *file_name,*xfile_name;
	FILE *temp_file;
	MrmType warning_box_class;
	struct File_open_data *file_open_data;
	XmSelectionBoxCallbackStruct *callback_data;

	ENTER(file_selection_write);
	USE_PARAMETER(widget);
/*	if (callback_data=(XmAnyCallbackStruct *)call_data)*/
	if (callback_data=(XmSelectionBoxCallbackStruct *)call_data)
	{
/*		if (XmCR_ACTIVATE==callback_data->reason)*/
		if (XmCR_OK==callback_data->reason)
		{
			if (file_open_data=(struct File_open_data *)client_data)
			{
				if (file_open_data->operation)
				{
					/* get the file name */
					XmStringGetLtoR(callback_data->value,XmSTRING_DEFAULT_CHARSET,
						&xfile_name);
/*					if (file_open_data->selection_text)
					{
						XtVaGetValues(file_open_data->selection_text,
							XmNvalue,&xfile_name,
							NULL);*/
						if (file_open_data->filter_extension)
						{
							if (ALLOCATE(file_name,char,strlen(xfile_name)+
								strlen(file_open_data->filter_extension)+1))
							{
								strcpy(file_name,xfile_name);
								strcat(file_name,file_open_data->filter_extension);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"file_selection_write.  Insufficient memory for file name");
							}
						}
						else
						{
							if (ALLOCATE(file_name,char,strlen(xfile_name)+1))
							{
								strcpy(file_name,xfile_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"file_selection_write.  Insufficient memory for file name");
							}
						}
						busy_cursor_off(file_open_data->selection_shell,
							file_open_data->user_interface );
						if ((REGULAR==file_open_data->type)&&
							(temp_file=fopen(file_name,"r")))
						{
							fclose(temp_file);
							if (!(file_open_data->warning_shell))
							{
								if (file_open_data->warning_shell=XtVaCreatePopupShell(
									"file_exists_warning_shell",xmDialogShellWidgetClass,
									User_interface_get_application_shell(file_open_data->user_interface),
									XmNtitle,"Warning",
									NULL))
								{
									file_open_data->warning_box=(Widget)NULL;
									if (filedir_hierarchy_open&&(MrmSUCCESS==MrmFetchWidget(
										filedir_hierarchy,"file_exists_warning_box",
										file_open_data->warning_shell,
										&(file_open_data->warning_box),&warning_box_class)))
									{
										/* remove the help button */
										XtUnmanageChild(XmMessageBoxGetChild(
											file_open_data->warning_box,XmDIALOG_HELP_BUTTON));
									}
									else
									{
										XtDestroyWidget(file_open_data->warning_shell);
										file_open_data->warning_shell=(Widget)NULL;
										display_message(ERROR_MESSAGE,
											"file_selection_write.  Could not retrieve warning box");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"file_selection_write.  Could not create warning shell");
								}
							}
							else
							{
								XtRemoveAllCallbacks(file_open_data->warning_box,XmNokCallback);
								XtRemoveAllCallbacks(file_open_data->warning_box,
									XmNcancelCallback);
								XtRemoveAllCallbacks(file_open_data->warning_box,
									XmNdestroyCallback);
							}
							if (file_open_data->warning_shell)
							{
								/* add the cancel callbacks */
								XtAddCallback(file_open_data->warning_box,XmNcancelCallback,
									close_file_exists_warning,
									(XtPointer)(file_open_data->warning_box));
								XtAddCallback(file_open_data->warning_box,XmNcancelCallback,
									busy_cursor_off_warning_shell,(XtPointer)file_open_data);
								XtAddCallback(file_open_data->warning_box,XmNcancelCallback,
									busy_cursor_on_selection_shell,(XtPointer)file_open_data);
								/* add the destroy callbacks */
								XtAddCallback(file_open_data->warning_box,XmNdestroyCallback,
									busy_cursor_off_warning_shell,(XtPointer)file_open_data);
								XtAddCallback(file_open_data->warning_box,XmNdestroyCallback,
									destroy_window_shell,(XtPointer)create_Shell_list_item(
									&(file_open_data->warning_shell),
									file_open_data->user_interface));
								XtAddCallback(file_open_data->warning_box,XmNdestroyCallback,
									busy_cursor_on_selection_shell,(XtPointer)file_open_data);
								/* add the OK callbacks */
								file_open_data->file_name=file_name;
								XtAddCallback(file_open_data->warning_box,XmNokCallback,
									busy_cursor_off_warning_shell,(XtPointer)file_open_data);
								XtAddCallback(file_open_data->warning_box,XmNokCallback,
									overwrite_file,(XtPointer)file_open_data);
								busy_cursor_on(file_open_data->warning_shell,
									file_open_data->user_interface );
								XtManageChild(file_open_data->warning_box);
							}
							else
							{
								busy_cursor_on(file_open_data->selection_shell,
									file_open_data->user_interface );
								DEALLOCATE(file_name);
							}
						}
						else
						{
							file_open_data->file_name=file_name;
							overwrite_file((Widget)NULL,(XtPointer)file_open_data,
								(XtPointer)NULL);
						}
/*          }
					else
					{
						display_message(ERROR_MESSAGE,
							"file_selection_write.  Selection text widget missing");
					}*/
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_selection_write.  No file operation");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"file_selection_write.  No client_data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"file_selection_write.  Incorrect reason");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"file_selection_write.  No call_data");
	}
	LEAVE;
} /* file_selection_write */
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
struct File_open_data *create_File_open_data(char *filter_extension,
	enum File_type type,File_operation operation,void *arguments,
	int allow_direct_to_printer,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
This function allocates memory for file open data and initializes the fields to
the specified values.  It returns a pointer to the created structure if
successful and NULL if unsuccessful.
==============================================================================*/
{
	struct File_open_data *file_open_data;
#if defined (WIN32_USER_INTERFACE)
	char *temp_filter_extension;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(create_File_open_data);
	/* check arguments */
	if (user_interface)
	{
		if (ALLOCATE(file_open_data,struct File_open_data,1))
		{
			if (filter_extension)
			{
				if (ALLOCATE(file_open_data->filter_extension,char,
					strlen(filter_extension)+1))
				{
					strcpy(file_open_data->filter_extension,filter_extension);
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"create_File_open_data.  Could not allocate memory for filter extension");
				}
			}
			else
			{
				file_open_data->filter_extension=(char *)NULL;
			}
			file_open_data->type=type;
			file_open_data->operation=operation;
			file_open_data->arguments=arguments;
			file_open_data->cancel_operation=(File_cancel_operation)NULL;
			file_open_data->cancel_arguments=NULL;
			file_open_data->allow_direct_to_printer=allow_direct_to_printer;
			file_open_data->user_interface=user_interface;
			file_open_data->file_name=(char *)NULL;
#if defined (MOTIF)
			file_open_data->activation=(Widget)NULL;
			file_open_data->file_list=(Widget)NULL;
			file_open_data->selection_shell=(Widget)NULL;
			file_open_data->selection=(Widget)NULL;
			file_open_data->selection_label=(Widget)NULL;
			file_open_data->selection_text=(Widget)NULL;
			file_open_data->warning_box=(Widget)NULL;
			file_open_data->warning_shell=(Widget)NULL;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
			(file_open_data->open_file_name).lStructSize=sizeof(OPENFILENAME);
			(file_open_data->open_file_name).hwndOwner=(HWND)NULL;
			(file_open_data->open_file_name).hInstance=(HINSTANCE)NULL;
			if (temp_filter_extension=filter_extension)
			{
				if ('.'==temp_filter_extension[0])
				{
					temp_filter_extension++;
				}
				if (ALLOCATE((file_open_data->open_file_name).lpstrFilter,char,
					2*strlen(temp_filter_extension)+11))
				{
					/*???DB.  Unicode ? */
					strcpy((char *)((file_open_data->open_file_name).lpstrFilter),
						temp_filter_extension);
					strcat((char *)((file_open_data->open_file_name).lpstrFilter),
						" files");
					strcpy((char *)(((file_open_data->open_file_name).lpstrFilter)+
						(strlen(temp_filter_extension)+7)),"*.");
					strcat((char *)(((file_open_data->open_file_name).lpstrFilter)+
						(strlen(temp_filter_extension)+7)),temp_filter_extension);
					((char *)((file_open_data->open_file_name).lpstrFilter))
						[2*strlen(temp_filter_extension)+10]='\0';
				}
			}
			else
			{
				(file_open_data->open_file_name).lpstrFilter=(char *)NULL;
			}
			(file_open_data->open_file_name).lpstrCustomFilter=(char *)NULL;
			(file_open_data->open_file_name).nMaxCustFilter=0;
			(file_open_data->open_file_name).nFilterIndex=1;
			(file_open_data->open_file_name).nMaxFile=256;
			if (ALLOCATE((file_open_data->open_file_name).lpstrFile,char,
				(file_open_data->open_file_name).nMaxFile))
			{
				((file_open_data->open_file_name).lpstrFile)[0]='\0';
				(file_open_data->open_file_name).lpstrFileTitle=(char *)NULL;
				(file_open_data->open_file_name).nMaxFileTitle=0;
				(file_open_data->open_file_name).lpstrInitialDir=(char *)NULL;
				(file_open_data->open_file_name).lpstrTitle=(char *)NULL;
				(file_open_data->open_file_name).Flags=OFN_EXPLORER|OFN_OVERWRITEPROMPT;
				(file_open_data->open_file_name).nFileOffset=0;
				(file_open_data->open_file_name).nFileExtension=0;
				if (ALLOCATE((file_open_data->open_file_name).lpstrDefExt,char,
					strlen(filter_extension)+1))
				{
					strcpy((char *)((file_open_data->open_file_name).lpstrDefExt),
						filter_extension);
				}
					/*???DB.  Only the first 3 characters are appended ! */
				(file_open_data->open_file_name).lCustData=(DWORD)NULL;
				(file_open_data->open_file_name).lpfnHook=(LPOFNHOOKPROC)NULL;
				(file_open_data->open_file_name).lpTemplateName=(char *)NULL;
			}
			else
			{
				DEALLOCATE(file_open_data->filter_extension);
				DEALLOCATE((file_open_data->open_file_name).lpstrFilter);
				DEALLOCATE(file_open_data);
				display_message(ERROR_MESSAGE,
					"create_File_open_data.  Could not allocate memory for lpstrFile");
			}
#endif /* defined (WIN32_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_File_open_data.  Could not allocate memory for file_open_data");
		}
	}
	else
	{
		file_open_data=(struct File_open_data *)NULL;
	}
	LEAVE;

	return (file_open_data);
} /* create_File_open_data */

int destroy_File_open_data(struct File_open_data **file_open_data)
/*******************************************************************************
LAST MODIFIED : 20 April 1997

DESCRIPTION :
This function frees the memory associated with the fields of <**file_open_data>,
frees the memory for <**file_open_data> and changes <*file_open_data> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_File_open_data);
	return_code=1;
	if (*file_open_data)
	{
#if defined (MOTIF)
		if ((*file_open_data)->selection_shell)
		{
			XtDestroyWidget((*file_open_data)->selection_shell);
		}
#endif /* defined (MOTIF) */
		DEALLOCATE((*file_open_data)->filter_extension);
		DEALLOCATE(*file_open_data);
		*file_open_data=(struct File_open_data *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_File_open_data */

void open_file_and_read(
#if defined (MOTIF)
	Widget widget,XtPointer client_data,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
	)
/*******************************************************************************
LAST MODIFIED : 23 April 2004

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data>.  Displays
a list of the file names matching the <filter>.  After the user selects a file
name the <file_operation> is performed on the file with the <arguments>.
==============================================================================*/
{
#if defined (MOTIF)
	Atom WM_DELETE_WINDOW;
	char *shell_title,*temp_string;
	MrmType selection_class;
	struct File_open_data *file_open_data;
	Widget file_selection_child,parent;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	char *temp_str;
	int length,retry;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(open_file_and_read);
#if defined (MOTIF)
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		if (!((file_open_data->activation==widget)&&
			file_open_data->selection_shell&&file_open_data->selection))
		{
			if (MrmOpenHierarchy_binary_string(filedir_uidh,sizeof(filedir_uidh),
				&filedir_hierarchy,&filedir_hierarchy_open))
			{
				file_open_data->activation=widget;
				if (!(parent=widget)||(True!=XtIsWidget(parent)))
				{
					parent=User_interface_get_application_shell(file_open_data->
						user_interface);
				}
				/* assign the shell title */
				temp_string=(char *)NULL;
				switch (file_open_data->type)
				{
					case REGULAR:
					{
						shell_title="Specify a file";
						if (file_open_data->filter_extension)
						{
							if (ALLOCATE(temp_string,char,strlen(shell_title)+
								strlen(file_open_data->filter_extension)+2))
							{
								strcpy(temp_string,"Specify a ");
								strcat(temp_string,file_open_data->filter_extension);
								strcat(temp_string," file");
								shell_title=temp_string;
							}
						}
					} break;
					case DIRECTORY:
					{
						shell_title="Specify a directory";
					} break;
					default:
					{
						shell_title=(char *)NULL;
					} break;
				}
				/* create the dialog shell */
				if (file_open_data->selection_shell=XtVaCreatePopupShell(
					"file_selection_shell",
					xmDialogShellWidgetClass,parent,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNtitle,shell_title,
					NULL))
				{
#if defined (OLD_CODE)
					XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
						busy_cursor_off_selection_shell,(XtPointer)file_open_data);
					XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
						destroy_window_shell,create_Shell_list_item(&(file_open_data->
						selection_shell),file_open_data->user_interface));
					XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
						destroy_file_selection,client_data);
#endif /* defined (OLD_CODE) */
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(file_open_data->selection_shell),
						file_open_data->user_interface);
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(
						XtDisplay(file_open_data->selection_shell),
						"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(file_open_data->selection_shell,
						WM_DELETE_WINDOW,close_file_selection,client_data);
					/* create the file selection box */
					if (MrmSUCCESS==MrmFetchWidget(filedir_hierarchy,"file_selection_box",
						file_open_data->selection_shell,&(file_open_data->selection),
						&selection_class))
					{
						/* remove the filter button */
	/*          file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_APPLY_BUTTON);
						XtUnmanageChild(file_selection_child);*/
						/* remove the filter label */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_FILTER_LABEL);
						XtUnmanageChild(file_selection_child);
						/* remove the filter text */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_FILTER_TEXT);
						XtUnmanageChild(file_selection_child);
						/* remove the help button */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_HELP_BUTTON);
						XtUnmanageChild(file_selection_child);
						/* remove the selection label */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_SELECTION_LABEL);
						XtUnmanageChild(file_selection_child);
						/* remove the file list label */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_LIST_LABEL);
						XtUnmanageChild(file_selection_child);
						/* remove the directory list */
	/*          file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_DIR_LIST);
						XtUnmanageChild(XtParent(file_selection_child));*/
						/* remove the directory list label */
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_DIR_LIST_LABEL);
						XtUnmanageChild(file_selection_child);
						file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_SEPARATOR);
						XtUnmanageChild(file_selection_child);
						/* remove the selection text */
						/* file_selection_child=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_TEXT);
							XtUnmanageChild(file_selection_child); */
						/* add cancel callback */
						XtAddCallback(file_open_data->selection,XmNcancelCallback,
							cancel_file_selection,client_data);
						/* add the OK callback */
						XtAddCallback(file_open_data->selection,XmNokCallback,
							file_selection_read,client_data);
						/* get the file list */
						file_open_data->file_list=XmFileSelectionBoxGetChild(
							file_open_data->selection,XmDIALOG_LIST);
#if defined (REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION)
						/* replace the file name filter */
						if ((file_open_data->filter_extension)&&ALLOCATE(temp_string,char,
							strlen(file_open_data->filter_extension)+2))
						{
							strcpy(temp_string,"*");
							strcat(temp_string,file_open_data->filter_extension);
							XmFileSelectionDoSearch(file_open_data->selection,
								XmStringCreate(temp_string,XmSTRING_DEFAULT_CHARSET));
							DEALLOCATE(temp_string);
						}
						else
						{
							XmFileSelectionDoSearch(file_open_data->selection,
								XmStringCreate("*",XmSTRING_DEFAULT_CHARSET));
						}
#endif /* defined (REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION) */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_file_and_read.  Could not create file_selection");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_read.  Could not create file_selection_dialog");
				}
				DEALLOCATE(temp_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_file_and_read.  Could not open hierarchy");
			}
		}
		if ((file_open_data->activation==widget)&&
			file_open_data->selection_shell&&file_open_data->selection)
		{
			/* ghost the activation button */
			if (widget)
			{
				XtSetSensitive(widget,False);
			}
#if !defined (REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION)
			/* replace the file name filter */
			if ((file_open_data->filter_extension)&&ALLOCATE(temp_string,char,
				strlen(file_open_data->filter_extension)+2))
			{
				strcpy(temp_string,"*");
				strcat(temp_string,file_open_data->filter_extension);
				XmFileSelectionDoSearch(file_open_data->selection,
					XmStringCreate(temp_string,XmSTRING_DEFAULT_CHARSET));
				DEALLOCATE(temp_string);
			}
			else
			{
				XmFileSelectionDoSearch(file_open_data->selection,
					XmStringCreate("*",XmSTRING_DEFAULT_CHARSET));
			}
#endif /* !defined (REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION) */
			busy_cursor_on(file_open_data->selection_shell,
				file_open_data->user_interface );
			/* manage the file selection box to pop it up */
			XtManageChild(file_open_data->selection);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_read.  No file_open_data");
	}
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	if (file_open_data)
	{
		retry=0;
		do
		{
			if (TRUE==GetOpenFileName(&(file_open_data->open_file_name)))
			{
				/* keep the current directory */
				length=0;
				if (temp_str=strrchr((file_open_data->open_file_name).lpstrFile,'\\'))
				{
					length=(temp_str-(file_open_data->open_file_name).lpstrFile)+1;
				}
				if ((0<length)&&REALLOCATE(temp_str,
					(file_open_data->open_file_name).lpstrInitialDir,char,length+1))
				{
					(file_open_data->open_file_name).lpstrInitialDir=temp_str;
					strncpy(temp_str,(file_open_data->open_file_name).lpstrFile,length);
					temp_str[length]='\0';
				}
				if (file_open_data->operation)
				{
					if ((file_open_data->operation)((file_open_data->open_file_name).
						lpstrFile,file_open_data->arguments))
					{
						retry=0;
					}
					else
					{
						retry=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_read.  No file operation");
					retry=0;
				}
			}
			else
			{
				retry=0;
			}
		} while (retry);
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_read.  No file_open_data");
	}
#endif /* defined (WIN32_USER_INTERFACE) */
	LEAVE;
} /* open_file_and_read */

void open_file_and_write(
#if defined (MOTIF)
	Widget widget,XtPointer client_data,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
	)
/*******************************************************************************
LAST MODIFIED : 9 June 2003

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data> (MOTIF).
Prompts the user for the name of file, omitting the <extension>, to write to.
The <file_operation> with the <user_arguments> and the output directed to the
specified file.
==============================================================================*/
{
#if defined (MOTIF)
	Atom WM_DELETE_WINDOW;
#if defined (OLD_CODE)
	char first;
#endif /* defined (OLD_CODE) */
	char *label,*print_command,*shell_title;
	MrmType selection_class;
	static char *printer_command=(char *)NULL;
	struct File_open_data *file_open_data;
#if defined (OLD_CODE)
	static char *uid_file_names[]=
	{
		"filedir.uid"
	};
#endif /* defined (OLD_CODE) */
	static MrmRegisterArg callback_list[]={
		{"identify_file_selection_label",(XtPointer)identify_file_selection_label},
		{"identify_file_selection_text",(XtPointer)identify_file_selection_text},
		{"file_selection_write",(XtPointer)file_selection_write},
		{"cancel_file_selection",(XtPointer)cancel_file_selection}};
#if defined (OLD_CODE)
	static MrmRegisterArg identifier_list[]=
	{
		{"file_open_data",(XtPointer)NULL}
	};
#endif /* defined (OLD_CODE) */
	Widget file_selection_child,parent;
#define XmNprinterCommand "printerCommand"
#define XmCPrinterCommand "PrinterCommand"
	static XtResource resources[]=
	{
		{
			XmNprinterCommand,
			XmCPrinterCommand,
			XmRString,
			sizeof(char *),
			0,
			XmRString,
			"print to file"
		}
	};
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	char *temp_str;
	int length,retry;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(open_file_and_write);
#if defined (MOTIF)
	USE_PARAMETER(call_data);
	if (file_open_data=(struct File_open_data *)client_data)
	{
		/*???DB.  Do better ? */
		if (!printer_command)
		{
			XtVaGetApplicationResources(
				User_interface_get_application_shell(file_open_data->user_interface),
				&printer_command,resources,XtNumber(resources),NULL);
		}
		if (printer_command&&strcmp(printer_command,"print to file")&&
			(file_open_data->allow_direct_to_printer))
		{
			if (file_open_data->operation)
			{
				if (ALLOCATE(print_command,char,strlen(printer_command)+
					L_tmpnam+2)&&strcpy(print_command,printer_command)&&
					tmpnam(print_command+(strlen(printer_command)+1)))
				{
					print_command[strlen(printer_command)]=' ';
					/* perform the operation */
					if ((file_open_data->operation)(
						print_command+(strlen(printer_command)+1),
						file_open_data->arguments))
					{
						system(print_command);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_write.  Could not create print command");
				}
				DEALLOCATE(print_command);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_file_and_write.  Missing file operation");
			}
		}
		else
		{
			if (!((file_open_data->activation==widget)&&
				file_open_data->selection_shell&&file_open_data->selection))
			{
				if (MrmOpenHierarchy_binary_string(filedir_uidh,sizeof(filedir_uidh),
					&filedir_hierarchy,&filedir_hierarchy_open))
				{
#if defined (OLD_CODE)
					first=1;
#endif /* defined (OLD_CODE) */
					file_open_data->activation=widget;
					if (!(parent=widget)||(True!=XtIsWidget(parent)))
					{
						parent=User_interface_get_application_shell(
							file_open_data->user_interface);
					}
					/* assign the shell title */
					switch (file_open_data->type)
					{
						case REGULAR:
						{
							shell_title="Specify a file";
						} break;
						case DIRECTORY:
						{
							shell_title="Specify a directory";
						} break;
						default:
						{
							shell_title=(char *)NULL;
						} break;
					}
					/* create the dialog shell */
					if (file_open_data->selection_shell=XtVaCreatePopupShell(
						"file_selection_shell",
						xmDialogShellWidgetClass,parent,
						XmNdeleteResponse,XmDO_NOTHING,
						XmNtitle,shell_title,
						NULL))
					{
#if defined (OLD_CODE)
						XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
							destroy_file_selection,client_data);
						XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
							busy_cursor_off_selection_shell,(XtPointer)file_open_data);
						XtAddCallback(file_open_data->selection_shell,XmNdestroyCallback,
							destroy_window_shell,create_Shell_list_item(&(file_open_data->
							selection_shell), file_open_data->user_interface));
#endif /* defined (OLD_CODE) */
						/* Register the shell with the busy signal list */
						create_Shell_list_item(&(file_open_data->selection_shell),
							file_open_data->user_interface);
						/* Set up window manager callback for close window message */
						WM_DELETE_WINDOW=XmInternAtom(
							XtDisplay(file_open_data->selection_shell),
							"WM_DELETE_WINDOW",False);
						XmAddWMProtocolCallback(file_open_data->selection_shell,
							WM_DELETE_WINDOW,close_file_selection,client_data);
						/* register the callbacks */
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(filedir_hierarchy,
							callback_list,XtNumber(callback_list)))
						{
							/* create the file selection box */
							if (MrmSUCCESS==MrmFetchWidget(filedir_hierarchy,
								"file_selection_box",file_open_data->selection_shell,
								&(file_open_data->selection),&selection_class))
							{
								/* remove the filter button */
/*								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_APPLY_BUTTON);
								XtUnmanageChild(file_selection_child);*/
								/* remove the filter label */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_FILTER_LABEL);
								XtUnmanageChild(file_selection_child);
								/* remove the filter text */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_FILTER_TEXT);
								XtUnmanageChild(file_selection_child);
								/* remove the help button */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_HELP_BUTTON);
								XtUnmanageChild(file_selection_child);
								/* remove the file list label */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_LIST_LABEL);
								XtUnmanageChild(file_selection_child);
								/* remove the file list */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_LIST);
								XtUnmanageChild(XtParent(file_selection_child));
								/* remove the directory list label */
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_DIR_LIST_LABEL);
								XtUnmanageChild(file_selection_child);
								file_selection_child=XmFileSelectionBoxGetChild(
									file_open_data->selection,XmDIALOG_SEPARATOR);
								XtUnmanageChild(file_selection_child);
								/* add cancel callback */
								XtAddCallback(file_open_data->selection,XmNcancelCallback,
									cancel_file_selection,client_data);
								/* add the OK callback */
								XtAddCallback(file_open_data->selection,XmNokCallback,
									file_selection_write,client_data);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_file_and_write.  Could not create file_selection");
							}
#if defined (OLD_CODE)
							/* assign and register the identifiers */
							identifier_list[0].value=(XtPointer)file_open_data;
							if (MrmSUCCESS==MrmRegisterNamesInHierarchy(filedir_hierarchy,
								identifier_list,XtNumber(identifier_list)))
							{
								/* retrieve the file selection form */
								if (MrmSUCCESS!=MrmFetchWidget(filedir_hierarchy,
									"file_selection_form",file_open_data->selection_shell,
									&(file_open_data->selection),&selection_class))
								{
									display_message(ERROR_MESSAGE,
										"open_file_and_write.  Could not create file_selection");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_file_and_write.  Could not register identifiers");
							}
#endif /* defined (OLD_CODE) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_file_and_write.  Could not register callbacks");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_file_and_write.  Could not create file_selection_dialog");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_write.  Could not open hierarchy");
				}
			}
#if defined (OLD_CODE)
			else
			{
				first=0;
			}
#endif /* defined (OLD_CODE) */
			if ((file_open_data->activation==widget)&&
				file_open_data->selection_shell&&file_open_data->selection)
			{
				/* ghost the activation button */
				if (widget)
				{
					XtSetSensitive(widget,False);
				}
				/* reset the label */
				switch (file_open_data->type)
				{
					case REGULAR:
					{
						if (file_open_data->filter_extension)
						{
							if (ALLOCATE(label,char,
								strlen(file_open_data->filter_extension)+21))
							{
								strcpy(label,"File name (");
								strcat(label,file_open_data->filter_extension);
								strcat(label," assumed)");
							}
							else
							{
								label="File name";
							}
						}
						else
						{
							label="File name";
						}
					} break;
					case DIRECTORY:
					{
						label="Directory name";
					}
				}
	/*      XtVaSetValues(file_open_data->selection_label,
					XmNlabelString,XmStringCreate(label,XmSTRING_DEFAULT_CHARSET),
					NULL);*/
				file_selection_child=XmFileSelectionBoxGetChild(
					file_open_data->selection,XmDIALOG_SELECTION_LABEL);
				XtVaSetValues(file_selection_child,
					XmNlabelString,XmStringCreate(label,XmSTRING_DEFAULT_CHARSET),
					NULL);
				busy_cursor_on(file_open_data->selection_shell,
					file_open_data->user_interface );
				/* manage the file selection shell to pop it up */
				XtManageChild(file_open_data->selection);
#if defined (OLD_CODE)
				/* If we don't change it the default button is the OK button */
				if (first)
				{
					file_selection_child=XmFileSelectionBoxGetChild(
						file_open_data->selection,XmDIALOG_CANCEL_BUTTON);
					XmProcessTraversal(file_selection_child,XmTRAVERSE_CURRENT);
				}
#endif /* defined (OLD_CODE) */
				/* set the keyboard focus to the selection text */
				file_selection_child=XmFileSelectionBoxGetChild(
					file_open_data->selection,XmDIALOG_TEXT);
				XtSetKeyboardFocus(file_open_data->selection_shell,
					file_selection_child);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_write.  No file_open_data");
	}
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	if (file_open_data)
	{
		retry=0;
		do
		{
			if (TRUE==GetSaveFileName(&(file_open_data->open_file_name)))
			{
				/* keep the current directory */
				length=0;
				if (temp_str=strrchr((file_open_data->open_file_name).lpstrFile,'\\'))
				{
					length=(temp_str-(file_open_data->open_file_name).lpstrFile)+1;
				}
				if ((0<length)&&REALLOCATE(temp_str,
					(file_open_data->open_file_name).lpstrInitialDir,char,length+1))
				{
					(file_open_data->open_file_name).lpstrInitialDir=temp_str;
					strncpy(temp_str,(file_open_data->open_file_name).lpstrFile,length);
					temp_str[length]='\0';
				}
				if (file_open_data->operation)
				{
					if ((file_open_data->operation)((file_open_data->open_file_name).
						lpstrFile,file_open_data->arguments))
					{
						retry=0;
					}
					else
					{
						retry=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_read.  No file operation");
					retry=0;
				}
			}
			else
			{
				retry=0;
			}
		} while (retry);
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_write.  No file_open_data");
	}
#endif /* defined (WIN32_USER_INTERFACE) */
	LEAVE;
} /* open_file_and_write */

int register_file_cancel_callback(struct File_open_data *file_open_data,
	File_cancel_operation cancel_operation,void *cancel_arguments)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Register a callback that gets called when the file dialog is cancelled.
==============================================================================*/
{
	int return_code;

	ENTER(register_file_cancel_callback);
	return_code=0;
	if (file_open_data&&cancel_operation)
	{
		file_open_data->cancel_operation=cancel_operation;
		file_open_data->cancel_arguments=cancel_arguments;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"register_file_cancel_callback.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* register_file_cancel_callback */
