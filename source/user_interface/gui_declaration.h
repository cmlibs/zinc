/*******************************************************************************
FILE : gui_declaration.h

LAST MODIFIED : 1 March 1997

DESCRIPTION :
Window routines.
???DB.  Check on TRUE and FALSE for WINDOWS
???DB.  Needs to be split up like gui_button.  Part of Glen's Windows 95 version
==============================================================================*/
#if !defined (GUI_DECLARATION_H)
#define GUI_DECLARATION_H

#include "user_interface/gui_prototype.h"
#include "user_interface/message.h"

#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <windows.h>
#endif /* defined (WINDOWS) */

#if defined (OLD_CODE)
/*???DB.  Moved to gui_button.h */
#define WINDOW_BUTTON_SET_CHECKED(dialog_name,button_name,checked) \
	dialog_name ## _ ## button_name ## _set_checked(temp_dialog,checked)

#define WINDOW_BUTTON_GET_CHECKED(dialog_name,button_name) \
	dialog_name ## _ ## button_name ## _get_checked(temp_dialog)
#endif /* defined (OLD_CODE) */

#define WINDOW_LIST_ADD_ITEM(dialog_name,list_name,item) \
	dialog_name ## _ ## list_name ## _add_item(temp_dialog,item)

#define WINDOW_LIST_GET_NUM_ITEMS(dialog_name,list_name) \
	dialog_name ## _ ## list_name ## _get_num_items(temp_dialog)

#define WINDOW_LIST_GET_SELECTED_ITEMS(dialog_name,list_name,num_items_address) \
	dialog_name ## _ ## list_name ## _get_selected_items(temp_dialog,num_items_address)

#define WINDOW_LIST_GET_ITEM(dialog_name,list_name,item_number) \
	dialog_name ## _ ## list_name ## _get_item(temp_dialog,item_number)

#define WINDOW_LIST_GET_SELECTED(dialog_name,list_name) \
	dialog_name ## _ ## list_name ## _get_selected(temp_dialog)

#define WINDOW_LIST_VIEW_END(dialog_name,list_name) \
	dialog_name ## _ ## list_name ## _view_end(temp_dialog)

#define WINDOW_LIST_VIEW_BEGINNING(dialog_name,list_name) \
	dialog_name ## _ ## list_name ## _view_beginning(temp_dialog)

#define WINDOW_TEXT_APPEND_TEXT(dialog_name,text_name,text) \
	dialog_name ## _ ## text_name ## _append_text(temp_dialog,text)

#define WINDOW_TEXT_GET_TEXT(dialog_name,text_name) \
	dialog_name ## _ ## text_name ## _get_text(temp_dialog)

#define WINDOW_TEXT_VIEW_END(dialog_name,text_name) \
	dialog_name ## _ ## text_name ## _view_end(temp_dialog)

#define WINDOW_TEXT_SET_TEXT(dialog_name,text_name,text) \
	dialog_name ## _ ## text_name ## _set_text(temp_dialog,text)

#define WINDOW_TEXT_SET_FOCUS(dialog_name,text_name) \
	dialog_name ## _ ## text_name ## _set_focus(temp_dialog)


#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Does absolutely nothing - does not set the focus.

Except in response to the WM_INITDIALOG message, the dialog box procedure
should return nonzero if it processes the message, and zero if it does not.
In response to a WM_INITDIALOG message, the dialog box procedure should
return zero if it calls the SetFocus function to set the focus to one of
the controls in the dialog box. Otherwise, it should return nonzero, in
which case the system sets the focus to the first control in the dialog box
that can be given the focus.
==============================================================================*/
#define DECLARE_DEFAULT_DIALOG_PROCEDURE_FUNCTION(dialog_name)\
LRESULT CALLBACK DIALOG_PROCEDURE(dialog_name)(HWND hwndDialog, \
	UINT message,WPARAM wParam,LPARAM lParam) \
{ \
	LRESULT return_code; \
\
	return_code=0; \
	ENTER(DIALOG_PROCEDURE(dialog_name)); \
	LEAVE; \
\
	return (return_code); \
} /* DIALOG_PROCEDURE(dialog_name) */

/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Allocates memory for the local structure, initialises local data.

The dialog box procedure should return TRUE to direct Windows to set the
keyboard focus to the control given by hwndFocus. Otherwise, it should return
FALSE to prevent Windows from setting the default keyboard focus.
==============================================================================*/
#define DECLARE_DIALOG_HANDLER_FUNCTION_WM_INITDIALOG(dialog_name)\
PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_INITDIALOG(dialog_name)\
{ \
	int return_code,height,width; \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog=\
		(struct DIALOG_STRUCT(dialog_name) *)NULL; \
	DIALOG_INITIALISATION_STRUCT(dialog_name) *initial_data=\
		(DIALOG_INITIALISATION_STRUCT(dialog_name) *)NULL; \
	RECT size; \
\
	return_code=0; \
	ENTER(DIALOG_HANDLER(dialog_name,WM_INITDIALOG)); \
	/* get the initialisation data */ \
	initial_data=(DIALOG_INITIALISATION_STRUCT(dialog_name) *)lParam; \
	/* allocate memory for the structure */ \
	if (ALLOCATE(temp_dialog,struct DIALOG_STRUCT(dialog_name),1)) \
	{ \
		/* Set everything to zero (cheats way) */ \
		memset(temp_dialog,0,sizeof(struct DIALOG_STRUCT(dialog_name))); \
		/* initialise the dialog data */ \
		temp_dialog->dialog=hwndDialog; \
/*    temp_dialog->parent=???; */ \
		if (DIALOG_DATA_INITIALISE(dialog_name)(&(temp_dialog->data), \
			initial_data)) \
		{ \
			/* have to do tricky checking to see if the function failed */ \
			/* If SetWindowLong succeeds it returns the previous value */ \
			/* (which may be FALSE) => check GetLastError */ \
			SetLastError(0); \
			if (SetWindowLong(hwndDialog,DLGWINDOWEXTRA,(LONG)temp_dialog)|| \
				!GetLastError()) \
			{ \
				if (DIALOG_WINDOW_INITIALISE(dialog_name)(temp_dialog)) \
				{ \
					/* Send a 'size' message */ \
					GetClientRect(temp_dialog->dialog,&size); \
					width=size.right-size.left; \
					height=size.bottom-size.top; \
					SendMessage(temp_dialog->dialog,WM_SIZE,(WPARAM)SIZE_RESTORED, \
						MAKELPARAM(width,height)); \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"DIALOG_HANDLER(dialog_name,WM_INITDIALOG).  %s", \
						"Could not initialise windows"); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"DIALOG_HANDLER(dialog_name,WM_INITDIALOG).  %s", \
					"Could not set dialog information"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"DIALOG_HANDLER(dialog_name,WM_INITDIALOG).  %s", \
				"Could not initialise dialog data"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_HANDLER(dialog_name,WM_INITDIALOG).  %s", \
			"Could not allocate dialog structure"); \
	} \
	if (!return_code) \
	{ \
		DestroyWindow(hwndDialog); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DIALOG_HANDLER(dialog_name,WM_INITDIALOG) */


/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Destroys the window.

If an application processes this message, it should return zero.
GMH:  I think this means we always return FALSE so that windows will get to free
the information stored.

==============================================================================*/
#define DECLARE_DIALOG_HANDLER_FUNCTION_WM_DESTROY(dialog_name) \
PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_DESTROY(dialog_name)\
{ \
	DIALOG_WRAPPER_VARIABLES(dialog_name); \
\
	ENTER(DIALOG_HANDLER(dialog_name,WM_DESTROY)); \
	if (DIALOG_WRAPPER_INITIALISE_VARIABLES(dialog_name)) \
	{ \
		if (DIALOG_DATA_DESTROY(dialog_name)(&(temp_dialog->data))) \
		{ \
			if (DIALOG_WINDOW_DESTROY(dialog_name)(temp_dialog)) \
			{ \
				DEALLOCATE(temp_dialog); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"DIALOG_HANDLER(dialog_name,WM_DESTROY).  %s", \
					"Could not destroy window data"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"DIALOG_HANDLER(dialog_name,WM_DESTROY).  %s", \
				"Could not destroy dialog data"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_HANDLER(dialog_name,WM_DESTROY).  %s", \
			"Could not initialise dialog variables"); \
	} \
	LEAVE; \
\
	FORWARD_WM_DESTROY(hwndDialog,DefDlgProc); \
} /* DIALOG_HANDLER(dialog_name,WM_DESTROY) */
#endif /* defined (WINDOWS) */

/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Creates the dialog.
==============================================================================*/
#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define DECLARE_DIALOG_CREATE_FUNCTION(dialog_name) \
PROTOTYPE_DIALOG_CREATE_FUNCTION(dialog_name) \
{ \
	int return_code; \
	struct DIALOG_STRUCT(dialog_name) *return_struct; \
	HWND dialog; \
	WNDCLASSEX  wcex; \
	static char classname[]=#dialog_name; \
\
	return_struct=(struct DIALOG_STRUCT(dialog_name) *)NULL; \
	ENTER(DIALOG_CREATE(dialog_name)); \
	/* is the class already registered? */ \
	if (GetClassInfoEx(hInstance,classname,&wcex)) \
	{ \
		return_code=1; \
	} \
	else \
	{ \
		wcex.style        =CS_HREDRAW | CS_VREDRAW; \
		wcex.lpfnWndProc  =(WNDPROC)DIALOG_CLASS_PROCEDURE(dialog_name); \
		wcex.cbClsExtra   =0; \
		wcex.cbWndExtra   =DLGWINDOWEXTRA+ \
									sizeof(struct DIALOG_STRUCT(dialog_name) *); \
		wcex.hInstance    =hInstance; \
		wcex.hIcon        =LoadIcon (hInstance, #dialog_name); \
		wcex.hCursor      =LoadCursor(NULL, IDC_ARROW); \
		wcex.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); \
		wcex.lpszMenuName =NULL;  /* allow resource to specify the menu */ \
\
		/* And set the all-important class name: */ \
		wcex.lpszClassName=classname; \
\
		/* Added elements for Windows 95: */ \
		wcex.cbSize=sizeof(WNDCLASSEX); \
		wcex.hIconSm=LoadIcon(wcex.hInstance, #dialog_name"_small"); \
		return_code=RegisterClassEx(&wcex); \
	} \
	if (return_code) \
	{ \
		dialog=CreateDialogParam(hInstance,#dialog_name,\
			hwndParent,(DLGPROC)DIALOG_PROCEDURE(dialog_name), \
			(LPARAM)initial_data); \
		if (dialog) \
		{ \
			ShowWindow(dialog, SW_SHOW); \
			if (!DIALOG_WRAPPER_GET_INFORMATION(dialog_name)(dialog,&return_struct)) \
			{ \
				display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s", \
					"Unable to retrieve structure"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s", \
				"Unable to create dialog"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"DIALOG_CREATE(dialog_name).  %s", \
			"Unable to register class information"); \
	} \
	LEAVE; \
\
	return (return_struct); \
} /* DIALOG_CREATE(dialog_name) */
#endif /* defined (WINDOWS) */

/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Gets dialog information from the dialog handle.
==============================================================================*/
#if defined (MOTIF)
#define DECLARE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
PROTOTYPE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(DIALOG_GET_INFORMATION(dialog_name)); \
	if (temp_dialog&&temp_data_address) \
	{ \
		*temp_data_address=&(temp_dialog->data); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"DIALOG_GET_INFORMATION(dialog_name).  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
}
#define DECLARE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
PROTOTYPE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(DIALOG_WRAPPER_GET_INFORMATION(dialog_name)); \
	if (dialog&&temp_dialog_address) \
	{ \
		XtVaGetValues(dialog,XmNuserData,temp_dialog_address,NULL); \
		if (*temp_dialog_address) \
		{ \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"DIALOG_WRAPPER_GET_INFORMATION(dialog_name).  %s", \
				"Invalid dialog pointer"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_WRAPPER_GET_INFORMATION(dialog_name).  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define DECLARE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
PROTOTYPE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(DIALOG_GET_INFORMATION(dialog_name)); \
	if (temp_dialog&&temp_data_address) \
	{ \
		*temp_data_address=&(temp_dialog->data); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"DIALOG_GET_INFORMATION(dialog_name).  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
}
#define DECLARE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
PROTOTYPE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(DIALOG_WRAPPER_GET_INFORMATION(dialog_name)); \
	if (hwndDialog&&temp_dialog_address) \
	{ \
		if ((*temp_dialog_address= \
			(struct DIALOG_STRUCT(dialog_name) *)GetWindowLong(hwndDialog,DLGWINDOWEXTRA))!=NULL) \
		{ \
			return_code=1; \
		} \
		/* else */ \
		/* { */ \
			/* display_message(ERROR_MESSAGE, */ \
				"DIALOG_WRAPPER_GET_INFORMATION(dialog_name).  %s", */ \
				/* "Invalid dialog pointer"); */ \
		/* } */ \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DIALOG_WRAPPER_GET_INFORMATION(dialog_name).  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
}
#define DECLARE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name) \
PROTOTYPE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(WINDOW_GET_INFORMATION(dialog_name)); \
	if (hwndWindow&&temp_window_address&&temp_data_address) \
	{ \
		if ((*temp_window_address= \
			(WINDOW_STRUCT(dialog_name) *)GetWindowLong(hwndWindow,0))!=NULL) \
		{ \
			*temp_data_address=&((*temp_window_address)->data); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"WINDOW_GET_INFORMATION(dialog_name).  %s", \
				"Invalid window pointer"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"WINDOW_GET_INFORMATION(dialog_name).  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
}
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
#define DECLARE_LOCAL_GUI_DIALOG_FUNCTIONS(dialog_name) \
DECLARE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
DECLARE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name)
#define DECLARE_LOCAL_GUI_WINDOW_FUNCTIONS(dialog_name) \
DECLARE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define DECLARE_LOCAL_GUI_DIALOG_FUNCTIONS(dialog_name) \
DECLARE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
DECLARE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
DECLARE_DIALOG_HANDLER_FUNCTION_WM_INITDIALOG(dialog_name) \
DECLARE_DIALOG_HANDLER_FUNCTION_WM_DESTROY(dialog_name)
#define DECLARE_LOCAL_GUI_WINDOW_FUNCTIONS(dialog_name) \
DECLARE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name)
#endif /* defined (WINDOWS) */

#define DECLARE_GLOBAL_GUI_DIALOG_FUNCTIONS(dialog_name) \
DECLARE_DIALOG_CREATE_FUNCTION(dialog_name)
#define DECLARE_GLOBAL_GUI_WINDOW_FUNCTIONS(dialog_name) \
DECLARE_WINDOW_CREATE_FUNCTION(dialog_name)

#endif /* !defined (GUI_DECLARATION_H) */
