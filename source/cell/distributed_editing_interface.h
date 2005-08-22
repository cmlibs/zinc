/*******************************************************************************
FILE : distributed_editing_interface.h

LAST MODIFIED : 01 February 2001

DESCRIPTION :
The interface routines for editing distributed cellular parameters.
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
#if !defined (DISTRIBUTED_EDITING_INTERFACE_H)
#define DISTRIBUTED_EDITING_INTERFACE_H


#include "cell/cell_cmgui_interface.h"
#include "cell/cell_interface.h"

/*
Module types
============
*/
struct Distributed_editing_interface;
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
The interface object
==============================================================================*/

/*
Global functions
----------------
*/
struct Distributed_editing_interface *CREATE(Distributed_editing_interface)(
  struct Cell_interface *cell_interface,
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Creates a distributed editing interface object.
==============================================================================*/
int DESTROY(Distributed_editing_interface)(
  struct Distributed_editing_interface **interface_address);
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Destroys a distributed editing interface object.
==============================================================================*/
int Distributed_editing_interface_pop_up_dialog(
  struct Distributed_editing_interface *interface,Widget parent,
  struct User_interface *user_interface,Widget activation);
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
Pops up the distributed editing dialog, creating it if nescessary. <activation>
is the toggle button which activated the dialog.
==============================================================================*/
int Distributed_editing_interface_pop_down_dialog(
  struct Distributed_editing_interface *interface);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the distributed editing dialog.
==============================================================================*/
void Distributed_editing_interface_point_number_text_CB(Widget widget,
	void *interface_void,void *call_data);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
void Distributed_editing_interface_grid_value_text_CB(Widget widget,
	void *interface_void,void *call_data);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Called when entry is made into the grid_value_text field.
==============================================================================*/
int Distributed_editing_interface_create_choosers(
  struct Distributed_editing_interface *interface,void *dialog_void);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Creates the chooser widgets for the distributed editing dialog.
==============================================================================*/
int Distributed_editing_interface_update_from_element_point(
  struct Distributed_editing_interface *interface);
/*******************************************************************************
LAST MODIFIED : 23 January 2001

DESCRIPTION :
Called from the "reset" button callback in the distributed editing dialog -
simply resets the variable values to the field values at the current element
point.
==============================================================================*/
int Distributed_editing_interface_update_element_point(
  struct Distributed_editing_interface *interface,int apply_all);
/*******************************************************************************
LAST MODIFIED : 23 January 2001

DESCRIPTION :
Called from the "apply" and "apply all" button callbacks in the distributed
editing dialog. If <apply_all> is true, then all selected points will have
their field values updated (where possible), otherwise only alters the current
element point.
==============================================================================*/
int Distributed_editing_interface_has_element_copy(
  struct Distributed_editing_interface *interface);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns 1 if the <interface> contains a valid element_copy, otherwise returns 0
==============================================================================*/
struct FE_element *Distributed_editing_interface_get_element_copy(
  struct Distributed_editing_interface *interface);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns the element_copy for the given <interface>
==============================================================================*/

#endif /* !defined (DISTRIBUTED_EDITING_INTERFACE_H) */
