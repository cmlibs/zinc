/*******************************************************************************
FILE : curve_editor.h

LAST MODIFIED : 8 November 1999

DESCRIPTION :
Provides the widgets to modify Curve structures.
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
#if !defined (CONTROL_CURVE_EDITOR_H)
#define CONTROL_CURVE_EDITOR_H

#include "general/callback_motif.h"
#include "curve/curve.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
Widget create_curve_editor_widget(Widget *gelem_editor_widget,
	Widget parent,struct Curve *curve,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Creates a curve_editor widget.
==============================================================================*/

int curve_editor_set_callback(Widget curve_editor_widget,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Changes the callback function for the curve_editor_widget, which will be
called when the curve is modified in any way.
==============================================================================*/

int curve_editor_set_curve(Widget curve_editor_widget,
	struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the Curve to be edited by the curve_editor widget.
==============================================================================*/

struct Callback_data *curve_editor_get_callback(
	Widget curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns a pointer to the update_callback item of the curve_editor
widget.
==============================================================================*/

struct Curve *curve_editor_get_curve(
	Widget curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns the Curve currently being edited.
==============================================================================*/

int curve_editor_set_cursor_parameter(
	Widget curve_editor_widget, float parameter);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the current position of the parameter cursor and displays it if it isn't
already shown.
==============================================================================*/

float curve_editor_get_cursor_parameter(
	Widget curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Gets the current position of the parameter cursor.
==============================================================================*/
#endif /* !defined (CONTROL_CURVE_EDITOR_H) */
