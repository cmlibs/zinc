/*******************************************************************************
FILE : gui_button.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Button routines.
???DB.  Not used yet (in Glen's Windows 95 version).  Needs splitting into
public and private
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
#if !defined (GUI_BUTTON_H)
#define GUI_BUTTON_H

#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global functions
----------------
*/
#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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
