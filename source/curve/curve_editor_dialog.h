/*******************************************************************************
FILE : control_curve_editor_dialog.h

LAST MODIFIED : 8 Novemeber 1999

DESCRIPTION :
Routines for creating a variable editor dialog shell and standard buttons.
Form set aside for the actual variable editor.
==============================================================================*/
#if !defined (CONTROL_CURVE_EDITOR_DIALOG_H)
#define CONTROL_CURVE_EDITOR_DIALOG_H

#include "general/callback.h"
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
