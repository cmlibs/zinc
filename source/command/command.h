/*******************************************************************************
FILE : command.h

LAST MODIFIED : 15 July 2002

DESCRIPTION :
Functions and types associated with commands.
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
#if !defined (COMMAND_H)
#define COMMAND_H

#include "general/object.h"
#include "general/io_stream.h"
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#endif /* defined (MOTIF) */

/*
Global constants
----------------
*/
/*???DB.  Shouldn't be here ? */
/*#define CMGUI_EXAMPLE_DIRECTORY_SYMBOL "doc"*/
#define CMGUI_EXAMPLE_DIRECTORY_SYMBOL "example"

/*
Global types
------------
*/
typedef int (Execute_command_function)(char *command,void *user_data);

struct Execute_command;

/*
Global functions
----------------
*/
#if defined (MOTIF)
void callback_command(Widget widget_id,XtPointer command,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 11 June 1993

DESCRIPTION :
Allows easy execution of <command>s from menu buttons.
==============================================================================*/
#endif /* defined (MOTIF) */

int read_iod_file_via_selection_box(char *file_name,void *dummy);
/*******************************************************************************
LAST MODIFIED : 15 June 1993

DESCRIPTION :
==============================================================================*/

struct Execute_command *CREATE(Execute_command)(void);
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Creates a blank execute command. Must call Execute_command_set_command_function
to set the function it calls, and the user data to be passed with it.
==============================================================================*/

int DESTROY(Execute_command)(struct Execute_command **execute_command_address);
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
==============================================================================*/

int Execute_command_set_command_function(
	struct Execute_command *execute_command,
	Execute_command_function *execute_command_function,
	void *command_function_data);
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Sets the function called by <execute_command>, and the user data to be passed
with it.
==============================================================================*/

int Execute_command_execute_string(struct Execute_command *execute_command,
	const char *string);
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
Executes the given string using the Execute_command stucture
==============================================================================*/

int execute_comfile(char *file_name,struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command);
/******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/
#endif /* !defined (COMMAND_H) */
