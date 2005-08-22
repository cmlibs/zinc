/*******************************************************************************
FILE : cell_window.h

LAST MODIFIED : 20 November 2000

DESCRIPTION :
The Cell Interface main window.
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
#if !defined (CELL_WINDOW_H)
#define CELL_WINDOW_H


#include "cell/cell_interface.h"

/*
Module objects
--------------
*/
struct Cell_window;
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
The Cell Interface main window structure.
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_window *CREATE(Cell_window)(
  struct Cell_interface *cell_interface,
  struct User_interface *user_interface,
  XtCallbackProc exit_callback);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Creates the Cell interface main window.
==============================================================================*/
int DESTROY(Cell_window)(struct Cell_window **cell_window_address);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Destroys the Cell_window object.
==============================================================================*/
void Cell_window_pop_up(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Brings up the <cell_window>
==============================================================================*/
int Cell_window_set_title_bar(struct Cell_window *cell_window,char *title);
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Sets the main windows <title>
==============================================================================*/
Widget Cell_window_get_shell(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the shell widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_window(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the window widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_unemap_activation_button(
  struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Returns the UnEmap activation widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_root_shell(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns the root shell widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_scene_form(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the scene form widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_toolbar_form(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the toolbar form widget from the given <cell_window>
==============================================================================*/
void exit_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used in stand-alone Cell
==============================================================================*/
void close_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used when Cell is part of CMGUI
==============================================================================*/

#endif /* !defined (CELL_WINDOW_H) */
