/*******************************************************************************
FILE : gui_button.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Button routines.
???DB.  Not used yet (in Glen's Windows 95 version).  Needs splitting into
public and private
==============================================================================*/
#if !defined (GUI_BUTTON_H)
#define GUI_BUTTON_H

#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global functions
----------------
*/
#if defined (FULL_NAMES)
#define GUI_BUTTON_SET_CHECKED_( dialog_name , button_name ) \
	dialog_name ## _ ## button_name ## _set_checked
#else
#define GUI_BUTTON_SET_CHECKED_( dialog_name , button_name ) \
	sc ## dialog_name ## button_name
#endif
#define GUI_BUTTON_SET_CHECKED( dialog_name , button_name ) \
	GUI_BUTTON_SET_CHECKED_(dialog_name,button_name)

/*******************************************************************************
LAST MODIFIED : 1 March 1997

DESCRIPTION :
Sets the state of the button
==============================================================================*/
#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
#define DECLARE_GUI_BUTTON_SET_CHECKED(dialog_name,button_name) \
int GUI_BUTTON_SET_CHECKED(dialog_name,button_name)( \
	DIALOG_LOCAL_PARAM(dialog_name),int checked) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(GUI_BUTTON_SET_CHECKED(dialog_name,button_name)); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->button_name) \
		{ \
			if (checked) \
			{ \
				Button_SetCheck(temp_dialog->button_name,BST_CHECKED); \
			} \
			else \
			{ \
				Button_SetCheck(temp_dialog->button_name,BST_UNCHECKED); \
			} \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,
				"GUI_BUTTON_SET_CHECKED(" #dialog_name "," #button_name ").  %s", \
				"Invalid button"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GUI_BUTTON_SET_CHECKED(" #dialog_name "," #button_name ").  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GUI_BUTTON_SET_CHECKED(dialog_name,button_name) */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (FULL_NAMES)
#define GUI_BUTTON_GET_CHECKED_( dialog_name , button_name ) \
	dialog_name ## _ ## button_name ## _get_checked
#else
#define GUI_BUTTON_GET_CHECKED_( dialog_name , button_name ) \
	gc ## dialog_name ## button_name
#endif
#define GUI_BUTTON_GET_CHECKED( dialog_name , button_name ) \
	GUI_BUTTON_GET_CHECKED_(dialog_name,button_name)

/*******************************************************************************
LAST MODIFIED : 1 March 1997

DESCRIPTION :
Sets the state of the button
==============================================================================*/
#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
#define DECLARE_GUI_BUTTON_GET_CHECKED(dialog_name,button_name) \
int GUI_BUTTON_GET_CHECKED(dialog_name,button_name)( \
	DIALOG_LOCAL_PARAM(dialog_name)) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(GUI_BUTTON_GET_CHECKED(dialog_name,button_name)); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->button_name) \
		{ \
			return_code=Button_GetCheck(temp_dialog->button_name); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GUI_BUTTON_GET_CHECKED(" #dialog_name "," #button_name ").  %s", \
				"Invalid button"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GUI_BUTTON_GET_CHECKED(" #dialog_name "," #button_name ").  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GUI_BUTTON_GET_CHECKED(dialog_name,button_name) */
#endif /* defined (WIN32_USER_INTERFACE) */
#endif /* !defined (GUI_BUTTON_H) */
