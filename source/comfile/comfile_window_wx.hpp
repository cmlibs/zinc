/*******************************************************************************
FILE : comfile_window.h

LAST MODIFIED : 3 September 2004

DESCRIPTION :
Definitions for the comfile window and structures.
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
#if !defined (COMFILE_WINDOW_WX_H)
#define COMFILE_WINDOW_WX_H

extern "C" {
#include <stddef.h>
#include "command/command.h"
#include "command/parser.h"
#include "general/io_stream.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "user_interface/user_interface.h"
}
/*
Global types
------------
*/

struct Comfile_window;
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Command file or "comfile" window structure which lists a cmiss command file and
by user selection passes commands to the command window.
The contents of this structure are private.
==============================================================================*/

DECLARE_LIST_TYPES(Comfile_window);

DECLARE_MANAGER_TYPES(Comfile_window);

/*
Global functions
----------------
*/

struct Comfile_window *CREATE(Comfile_window)(char *name,
	char *file_name, struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/

int DESTROY(Comfile_window)(struct Comfile_window **comfile_window_address);
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION:
Frees the contents of the Comfile_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the comfile_window from a global list of windows is left with the
calling routine. See also Comfile_window_close_CB and
Comfile_window_destroy_CB.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Comfile_window);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Comfile_window);

PROTOTYPE_LIST_FUNCTIONS(Comfile_window);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Comfile_window,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Comfile_window,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Comfile_window);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Comfile_window,name,char *);

char *Comfile_window_manager_make_unique_name(
	struct MANAGER(Comfile_window) *comfile_window_manager, char *file_name);
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Allocates and returns a name based on <file_name> that is not currently in use
in the <comfile_window_manager>. Does so by appending a number in angle
brackets.
Up to the calling routine to deallocate the returned string.
==============================================================================*/
#endif /* !defined (COMFILE_WINDOW_H) */
