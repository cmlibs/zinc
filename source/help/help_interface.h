/*******************************************************************************
FILE : help_interface.h

LAST MODIFIED : 19 November 1998

DESCRIPTION :
Interface routines for CMISS commands to work with the help window.
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
#if !defined (HELP_INTERFACE_H)
#define HELP_INTERFACE_H

#include "command/command.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
void do_help(char *help_string,char *help_examples_directory,
	struct Execute_command *execute_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Gets help for help_string.
If the help window hasn't been created,it creates it.
If the help window is popped down it pops it up.
Then it sets the help topic edit field and gets the help on the string.
==============================================================================*/

void pop_down_help(void);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops down the help window.
==============================================================================*/

void destroy_help(void);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Destroys the help window.
==============================================================================*/
#endif /* !defined (HELP_INTERFACE_H) */
