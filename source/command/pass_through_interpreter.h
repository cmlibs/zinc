/*******************************************************************************
FILE : pass_through_interpreter.h

LAST MODIFIED : 18 April 2007

DESCRIPTION :
Just passes commands through without interpretation
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
#include <stdlib.h>

enum Message_type
/*******************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
The different message types.
==============================================================================*/
{
	ERROR_MESSAGE,
	INFORMATION_MESSAGE,
	WARNING_MESSAGE
}; /* enum Message_type */

typedef void (*execute_command_function_type)(char *, void *, int *, int *);

typedef int (Interpreter_display_message_function)(enum Message_type message_type,
	const char *format, ... );

struct Interpreter;
/*******************************************************************************
LAST MODIFIED : 18 April 2007

DESCRIPTION :
Internal data storage for this interpreter.
==============================================================================*/

void create_interpreter_(int argc, char **argv, const char *initial_comfile, 
	struct Interpreter **interpreter, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
Creates the interpreter for processing commands.
==============================================================================*/

void destroy_interpreter_(struct Interpreter *interpreter, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
==============================================================================*/

void interpreter_set_display_message_function_(struct Interpreter *interpreter, 
	Interpreter_display_message_function *function, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
Sets the function that will be called whenever the Interpreter wants to report
information.
==============================================================================*/

void redirect_interpreter_output_(struct Interpreter *interpreter, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
This redirects the output from stdout to a pipe so that the handle_output
routine can write this to the command window.
==============================================================================*/

void interpret_command_(struct Interpreter *interpreter, char *command_string, 
	 void *user_data, int *quit,
	execute_command_function_type execute_command_function, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
Takes a <command_string>, replaces simple variables and sends the command
back to the main program.
==============================================================================*/

void interpreter_evaluate_string_(struct Interpreter *interpreter, 
	char *expression, char **result, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
NOT_IMPLEMENTED
==============================================================================*/

void interpreter_set_string_(struct Interpreter *interpreter, 
	char *variable_name, char *value, int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
NOT_IMPLEMENTED
==============================================================================*/

void interpreter_set_pointer_(struct Interpreter *interpreter,
	char *variable_name, char *class_name, void *value,int *status);
/*******************************************************************************
LAST MODIFIED : 16 January 2007

DESCRIPTION:
NOT_IMPLEMENTED
==============================================================================*/
