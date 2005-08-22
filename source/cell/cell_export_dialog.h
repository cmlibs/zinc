/*******************************************************************************
FILE : cell_export_dialog.h

LAST MODIFIED : 01 February 2001

DESCRIPTION :
The export dialog used to export cell variables to ipcell and ipmatc files
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
#if !defined (CELL_EXPORT_DIALOG_H)
#define CELL_EXPORT_DIALOG_H

#include "cell/cell_interface.h"
#include "cell/cell_cmgui_interface.h"
#include "cell/distributed_editing_interface.h"

/*
Module types
============
*/
struct Cell_export_dialog;
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Used to store the information and widgets used to export ipcell and ipmatc
files from Cell.
==============================================================================*/

/*
Global functions
================
*/
struct Cell_export_dialog *CREATE(Cell_export_dialog)(
  struct Cell_interface *interface,
  struct Cell_cmgui_interface *cmgui_interface,
  struct Distributed_editing_interface *distributed_editing_interface,
  struct User_interface *user_interface,Widget parent);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Create a new export dialog.
==============================================================================*/
int DESTROY(Cell_export_dialog)(struct Cell_export_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Destroys a export dialog object.
==============================================================================*/
int Cell_export_dialog_pop_up(struct Cell_export_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops up the <dialog>
==============================================================================*/
int Cell_export_dialog_pop_down(struct Cell_export_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/

#endif
