/*******************************************************************************
FILE : control_curve_editor.h

LAST MODIFIED : 8 November 1999

DESCRIPTION :
Provides the widgets to modify Control_curve structures.
==============================================================================*/
#if !defined (CONTROL_CURVE_EDITOR_H)
#define CONTROL_CURVE_EDITOR_H

#include "general/callback_motif.h"
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
Widget create_control_curve_editor_widget(Widget *gelem_editor_widget,
	Widget parent,struct Control_curve *curve,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Creates a control_curve_editor widget.
==============================================================================*/

int control_curve_editor_set_callback(Widget control_curve_editor_widget,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Changes the callback function for the control_curve_editor_widget, which will be
called when the curve is modified in any way.
==============================================================================*/

int control_curve_editor_set_curve(Widget control_curve_editor_widget,
	struct Control_curve *curve);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the Control_curve to be edited by the control_curve_editor widget.
==============================================================================*/

struct Callback_data *control_curve_editor_get_callback(
	Widget control_curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns a pointer to the update_callback item of the control_curve_editor
widget.
==============================================================================*/

struct Control_curve *control_curve_editor_get_curve(
	Widget control_curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Returns the Control_curve currently being edited.
==============================================================================*/

int control_curve_editor_set_cursor_parameter(
	Widget control_curve_editor_widget, float parameter);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the current position of the parameter cursor and displays it if it isn't
already shown.
==============================================================================*/

float control_curve_editor_get_cursor_parameter(
	Widget control_curve_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Gets the current position of the parameter cursor.
==============================================================================*/
#endif /* !defined (CONTROL_CURVE_EDITOR_H) */
