/*******************************************************************************
FILE : cell_control_curve.h

LAST MODIFIED : 9 November 1999

DESCRIPTION :
Functions for CELL to interact with CMGUI time variables.
==============================================================================*/
#if !defined (CELL_CONTROL_CURVE_H)
#define CELL_CONTROL_CURVE_H
#include "curve/control_curve.h"
#include "curve/control_curve_editor_dialog.h"

/*
Global types
============
*/
struct Export_control_curve_dialog
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Used to store the information and widgets used to export time variables to 
iptime files.
==============================================================================*/
{
  Widget shell;
  Widget window;
	Widget variables_rowcol;
  Widget file_label;
  char *file_name;
}; /* Export_control_curve_dialog */

/*
Global functions
================
*/
int bring_up_parameter_control_curve_dialog(struct Cell_window *cell,
  struct Cell_parameter *parameter);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Brings up the cell time variable dialog.

To start with, just bring up the time varaible editor with the appropriate
variable, but, eventually, bring up a dialog which allows the user to
read/write time variables, set from default files, etc....
==============================================================================*/

int bring_up_variable_control_curve_dialog(struct Cell_window *cell,
  struct Cell_variable *variable);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Brings up the cell control curve dialog.

To start with, just bring up the control curve editor with the appropriate
curve, but, eventually, bring up a dialog which allows the user to
read/write time variables, set from default files, etc....
==============================================================================*/

void destroy_cell_control_curve(struct Control_curve *curve);
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Destroys the <variable> if it exists.
==============================================================================*/

int bring_up_export_control_curves_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
If a export window exists, pop it up, otherwise create it.
==============================================================================*/

void close_export_control_curves_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
If there is a export dialog in existence, then destroy it.
==============================================================================*/

#endif /* if !defined (CELL_CONTROL_CURVE_H) */
