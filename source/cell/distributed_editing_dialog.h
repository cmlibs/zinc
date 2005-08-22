/*******************************************************************************
FILE : distributed_editing_dialog.h

LAST MODIFIED : 22 January 2001

DESCRIPTION :
The dialog box for distributed parameter editing
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
#if !defined (DISTRIBUTED_EDITING_DIALOG_H)
#define DISTRIBUTED_EDITING_DIALOG_H

#include "cell/distributed_editing_interface.h"

/*
Global types
============
*/
struct Distributed_editing_dialog
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
The distributed editing dialog object.
==============================================================================*/
{
  /* A Pointer to the distributed editing interface */
  struct Distributed_editing_interface *interface;
  /* The dialog shell widget */
  Widget shell;
  /* The dialog main window */
  Widget window;
  /* The main user interface object */
  struct User_interface *user_interface;
  /* The dialog widgets */
  Widget element_form,element_widget,point_number_text,
    grid_field_form,grid_field_widget,grid_value_text,
    description_label;
  /* The activation toggle button widget */
  Widget activation;
}; /* struct Distributed_editing_dialog */

/*
Global functions
----------------
*/
struct Distributed_editing_dialog *CREATE(Distributed_editing_dialog)(
  struct Distributed_editing_interface *interface,Widget parent,
  struct User_interface *user_interface,Widget activation);
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
Creates a distributed editing dialog object. <activation> is the toggle button
widget which activated the dialog.
==============================================================================*/
int DESTROY(Distributed_editing_dialog)(
  struct Distributed_editing_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Destroys a distributed editing dialog object.
==============================================================================*/
int Distributed_editing_dialog_pop_up(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Brings up the <dialog>
==============================================================================*/
int Distributed_editing_dialog_pop_down(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/
int Distributed_editing_dialog_get_activation_state(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Returns 1 if the distributed editing toggle is set, 0 otherwise. The
distributed editing toggle is stored as the activation widget for the dialog.
==============================================================================*/

#endif /* !defined (DISTRIBUTED_EDITING_DIALOG_H) */
