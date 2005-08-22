/*******************************************************************************
FILE : help_window.h

LAST MODIFIED : 24 June 1996

DESCRIPTION :
Interface file for opening and closing and working a CMISS help window.
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
#if !defined (HELP_WINDOW_H)
#define HELP_WINDOW_H

#include <X11/Intrinsic.h>
#include <Mrm/MrmPublic.h>

/*
Global types
------------
*/
/*???DB.  Needed ? */
typedef void  (*D_FUNC)(void *);

struct Help_window
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Contains information about a CMISS help window.
==============================================================================*/
{
	char popped_up;
	Widget window_shell;
	char help_file_name[64];
	D_FUNC destroy_func;
	Display *display;
	MrmHierarchy hierarchy;
	short help_from_file;
	void *data_ptr;
	Widget app_shell,help_copy_button,help_do_button,help_find_button,
		help_select_button,help_text,help_topic,main_window;
}; /* struct Help_window */

/*
Global functions
----------------
*/
struct Help_window *create_help_window(D_FUNC destroy_func,void *data_ptr);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Creates a help window, and returns a pointer to the structure describing the
window.  If <destroy_func> is not NULL, it will be called when the window is
destroyed, with <data_ptr> as a parameter.  If destroy_func is NULL, but
<data_ptr> is not, then when the window is created, a pointer to the window
structure will be stored at the location pointed to by <data_ptr>, and when the
window is destroyed, that location will be cleared.
==============================================================================*/

void pop_up_help_window(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops up the (already created) help window described in the structure the_window.
==============================================================================*/

void pop_down_help_window(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops down (but doesn't destroy) the window described in the_window.
==============================================================================*/

void close_help_window(XtPointer help_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Closes and destroys the window pointed to by the window.  After this the
window must be recreated.  The routine relies on close_help_window_callback
to actually dispose of the window description structure and call the
destroy function (if necessary).
==============================================================================*/

void do_topic_text_help(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the string in the help topic edit field.
==============================================================================*/

void do_strings_help(struct Help_window *the_window,char **the_strings,
	short num_strings);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the keywords in the_strings, and displays the help in the_window.
==============================================================================*/
#endif
