/*******************************************************************************
FILE : control_curve_editor_dialog.h

LAST MODIFIED : 8 Novemeber 1999

DESCRIPTION :
Routines for creating a variable editor dialog shell and standard buttons.
Form set aside for the actual variable editor.
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
#if !defined (CONTROL_CURVE_EDITOR_DIALOG_H)
#define CONTROL_CURVE_EDITOR_DIALOG_H

#include "curve/control_curve.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
int control_curve_editor_dialog_set_curve(
	Widget control_curve_editor_dialog_widget,
	struct Control_curve *curve);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the curve being edited by the curve editor dialog.
==============================================================================*/

int control_curve_editor_dialog_set_cursor_parameter(
	Widget control_curve_editor_dialog_widget,float parameter);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the parameter curve cursor position in the curve editor.
==============================================================================*/

int bring_up_control_curve_editor_dialog(
	Widget *control_curve_editor_dialog_address,Widget parent,
	struct MANAGER(Control_curve) *control_curve_manager,
	struct Control_curve *curve,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
If there is a control curve editor dialog in existence, then bring it to the
front, else create a new one. In either case start by editing the specified
<curve>.
==============================================================================*/
#endif /* !defined (CONTROL_CURVE_EDITOR_DIALOG_H) */
