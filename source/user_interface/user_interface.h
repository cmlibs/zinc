/*******************************************************************************
FILE : user_interface.h

LAST MODIFIED : 28 February 2005

DESCRIPTION :
Function definitions for the user interface.
???DB.  The main purpose is to have a graphical user interface, but the
	possibility of a command line should be kept in mind.
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
#if !defined (USER_INTERFACE_H)
#define USER_INTERFACE_H

#if defined (BUILD_WITH_CMAKE)
#include "configure/zinc_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#if defined (WIN32_USER_INTERFACE) || defined (_MSC_VER)
//#define WINDOWS_LEAN_AND_MEAN
#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
	/*???DB.  Contains lots of convenience macros */
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
#include <carbon/carbon.h>
#endif /* defined (CARBON_USER_INTERFACE) */
#include "general/machine.h"

/*
Global types
------------
*/
struct Event_dispatcher;

struct User_interface;
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

/*
Global Functions
----------------
*/

struct Shell_list_item *create_Shell_list_item(
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function allocates memory for a shell list item, initializes the <shell>
field to the specified value and adds the item to the beginning of the shell
list.  It returns a pointer to the created item if successful and NULL if
unsuccessful.
???DB.  Move in with windowing macros ?
==============================================================================*/

int destroy_Shell_list_item(struct Shell_list_item **list_item);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
This function removes the <list_item> from the shell list and frees the memory
for the <list_item>.  <*list_item> is set to NULL.

???SAB.  Seems unnessary to insist that all the windows keep track of the Shell
list item for the shell list, rather pass the respective shell and the list can
find the appropriate list object and remove it.
???DB.  Move in with windowing macros ?
==============================================================================*/

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface *CREATE(User_interface)(int *argc_address, char **argv, 
	struct Event_dispatcher *event_dispatcher, const char *class_name, 
	const char *application_name);
#else /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
struct User_interface *CREATE(User_interface)(HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state,
	int *argc_address, char **argv, struct Event_dispatcher *event_dispatcher);
#endif /* !defined (WIN32_USER_INTERFACE && !defined (_MSC_VER)) */
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/

int DESTROY(User_interface)(struct User_interface **user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int User_interface_end_application_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
==============================================================================*/

#if defined (WX_USER_INTERFACE)
int User_interface_wx_main_loop(void);
/*******************************************************************************
LAST MODIFIED : 8 November 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
HINSTANCE User_interface_get_instance(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
GtkWidget *User_interface_get_main_window(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 9 July 2002

DESCRIPTION :
Returns the main window widget
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

int User_interface_get_local_machine_name(struct User_interface *user_interface,
	char **name);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
If the local machine name is know ALLOCATES and returns a string containing that
name.
==============================================================================*/

struct Event_dispatcher *User_interface_get_event_dispatcher(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int application_main_step(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Performs one step of the application_main_step update allowing the programmer
to execute the same main loop elsewhere under special conditions (i.e. waiting
for a response from a modal dialog).
==============================================================================*/

int
#if defined (WIN32_USER_INTERFACE)
	WINAPI
#endif /* defined (WIN32_USER_INTERFACE) */
	application_main_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 May 1996

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/

#endif /* !defined (USER_INTERFACE_H) */
