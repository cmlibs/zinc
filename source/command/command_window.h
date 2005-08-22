/*******************************************************************************
FILE : command_window.h

LAST MODIFIED : 27 June 2002

DESCRIPTION :
Definitions for command window structure, and associated functions
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
#if !defined (COMMAND_WINDOW_H)
#define COMMAND_WINDOW_H

#include "command/command.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

/*
Global constants
----------------
*/
/* for allowing other applications to communicate with cmgui (issue commands) */
#define CMGUI_VERSION_PROPERTY "_CMGUI_VERSION"
#define CMGUI_LOCK_PROPERTY "_CMGUI_LOCK"
#define CMGUI_COMMAND_PROPERTY "_CMGUI_COMMAND"
#define CMGUI_RESPONSE_PROPERTY "_CMGUI_RESPONSE"

/*
Global types
------------
*/
struct Command_window;

/*
Global functions
----------------
*/
struct Command_window *CREATE(Command_window)(
	struct Execute_command *execute_command,struct User_interface *user_interface,
	char *version_id_string);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
==============================================================================*/

int DESTROY(Command_window)(struct Command_window **command_window_pointer);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION:
==============================================================================*/

int add_to_command_list(char *command,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/

int Command_window_set_command_prompt(struct Command_window *command_window,
	char *prompt);
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <command_window>.
==============================================================================*/

int reset_command_box(struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Resets all functions of the command box widget.
==============================================================================*/

int Command_window_set_command_string(struct Command_window *command_window,
	char *command_string);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Fills the command entry area of the command window with <command_string>, ready
to be edited and entered. Used eg. by the comfile window to place a command in
responce to a single mouse click on it.
Does not override the command prompt.
==============================================================================*/

#if defined (MOTIF)
Widget Command_window_get_message_pane(struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Returns the message pane widget.
==============================================================================*/
#endif /* defined (MOTIF) */

int write_command_window(char *message,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/

int modify_Command_window(struct Parse_state *parse_state,void *dummy,
	void *command_window_void);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Modifys the <command_window_void> according to the command in the <parse_state>.
==============================================================================*/
#endif /* !defined (COMMAND_WINDOW_H) */
