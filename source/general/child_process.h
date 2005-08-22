/*******************************************************************************
FILE : child_process.h

LAST MODIFIED : 24 August 1999

DESCRIPTION :
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
#if !defined (CHILD_PROCESS_H)
#define CHILD_PROCESS_H

#include "general/object.h"

struct Child_process;

PROTOTYPE_OBJECT_FUNCTIONS(Child_process);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Child_process);

struct Child_process *CREATE(Child_process)(char *filename);
/*******************************************************************************
LAST MODIFIED : 24 August 1999

DESCRIPTION :
==============================================================================*/

int DESTROY(Child_process)(struct Child_process **child_process);
/*******************************************************************************
LAST MODIFIED : 24 August 1999

DESCRIPTION :
Destroys a Child_process object
x==============================================================================*/

int Child_process_send_string_to_stdin(struct Child_process *child_process,
	char *string);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Sends a string to the stdin pipe of a Child_process.
==============================================================================*/

char *Child_process_get_line_from_stdout(struct Child_process *child_process,
	int timeout);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Waits on the pipe from the stdout of the child process until an end_of_line is
received and returns the character string for that line.  If the end_of_line
isn't recieved in the <timeout> limit then the function returns NULL.
==============================================================================*/
#endif /* !defined (CHILD_PROCESS_H) */

