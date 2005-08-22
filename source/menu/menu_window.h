/*******************************************************************************
FILE : menu_window.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
Definitions for the menu window and structures.
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
#if !defined (MENU_WINDOW_H)
#define MENU_WINDOW_H
#include <stddef.h>
#include "command/command.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Macro
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
==============================================================================*/
{
	char *name;
	int number_of_commands;
	char **commands;
	char *file_name;
}; /* struct Macro */

struct Menu_window
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
==============================================================================*/
{
	Widget menu_list;
	int num_macros;
	struct Macro **macros;
	Widget window;
	Widget shell;
	char *file_name;
	struct Execute_command *execute_command;
}; /* struct Menu_window */

/*
Global functions
----------------
*/
void close_menu_window(Widget widget_id,XtPointer menu_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
Closes the menu window.
==============================================================================*/

struct Macro *create_Macro(char *file_name);
/*******************************************************************************
LAST MODIFIED : 13 June 1993

DESCRIPTION :
Create and parse the macro for a given <file_name>.
==============================================================================*/

struct Menu_window *create_Menu_window(char *file_name,
	struct Execute_command *execute_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Create the structures and retrieve the menu window from the uil file.
==============================================================================*/

int destroy_Macro(struct Macro **macro);
/*******************************************************************************
LAST MODIFIED : 15 June 1993

DESCRIPTION :
Free the memory associated with the fields of <**macro>, free the memory for
<**macro> and set <*macro> to NULL.
==============================================================================*/

void destroy_Menu_window(Widget widget_id,XtPointer menu_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Destroy the menu_window structure.
==============================================================================*/

void execute_menu_item(Widget widget_id,XtPointer menu_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Executes a double-clicked menu item.
==============================================================================*/

void identify_menu_list(Widget widget_id,XtPointer menu_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 13 June 1993

DESCRIPTION :
Stores the id of the message areas.
???DB.  What does this mean ?
==============================================================================*/

int open_menu(struct Parse_state *state,struct Execute_command *execute_command,
	char **example_directory_address, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/

int open_menu_from_fsb(char *file_name,void *dummy);
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Submits a command to open a menu.
==============================================================================*/
#endif /* !defined (MENU_WINDOW_H) */
