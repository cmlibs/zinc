/*******************************************************************************
FILE : cell_variable.h

LAST MODIFIED : 8 November 1999

DESCRIPTION :
Functions and structures for using the Cell_variable structure.
==============================================================================*/
#if !defined (CELL_VARIABLE_H)
#define CELL_VARIABLE_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/variables_dialog.h"

/*
Global types
============
*/
struct Cell_variable
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Structure containing information related to the individual variables.
==============================================================================*/
{
  char *label;
  Widget label_widget;
  char *units;
  Widget units_widget;
  enum Cell_array array;
  int position;
  char *spatial_label; /* the name of the variable */
  int spatial_switch;
  Widget spatial_switch_widget;
  float value;
  Widget value_widget;
  char *control_curve_label;
  int control_curve_switch;
  int control_curve_allowed;
  Widget control_curve_toggle;
  Widget control_curve_button;
  struct Control_curve *control_curve;
  struct Cell_variable *next;
}; /* Cell_variable */

/*
Global functions
================
*/
int set_variable_information(struct Cell_window *cell,char *array,
  char *position,char *name,char *label,char *units,char *spatial,
  char *control_curve,char *value,int default_value);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Sets the information in the variables structure for use when creating the
variables dialog. If <default_value> is 0, then the variable is not a default
value, and MUST replace one in the list, if it does not correspond to a
variable already in then list, then it is ignored.

<array> specifies the computational array that the variable belongs in, with
a specified <position>.

?? Assume that the default values are always read in first, which sets all
variable values and their order, so that when another set of variables is
read in from a model file the existing variables will simply be replaced by
the newer ones, keeping the order the same. ??
==============================================================================*/
int add_variables_to_variables_dialog(struct Cell_window *cell,
  struct Variables_dialog_user_settings *user_settings);
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Add the variables widgets to the variables dialog.
==============================================================================*/
void destroy_cell_variables(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Destroys the current list of variables.
==============================================================================*/
void format_variable_widgets(struct Cell_variable *variables);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Formats the widget placement for all the <variables>
==============================================================================*/
void reset_variable_values(struct Cell_variable *variables);
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Resets each of the <variables> to their respective values.
==============================================================================*/
void update_variable_values(struct Cell_variable *variables);
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Updates each of the <variables> from the value text field.
==============================================================================*/

#endif /* !defined (CELL_VARIABLE_H) */
