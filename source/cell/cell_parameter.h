/*******************************************************************************
FILE : cell_parameter.h

LAST MODIFIED : 9 November 1999

DESCRIPTION :
Functions and structures for using the Cell_parameter structure.
==============================================================================*/
#if !defined (CELL_PARAMETER_H)
#define CELL_PARAMETER_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/cell_control_curve.h"
#include "cell/cell_window.h"

/*
Global types
============
*/
struct Cell_parameter
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Structure containing information related to the individual parameters.
==============================================================================*/
{
  char *label;
  char *units;
  char *spatial_label; /* set the spatial label to be the parameter's name */
  int spatial_switch;
  int time_variable_allowed;
  int time_variable_switch;
  enum Cell_array array;
  int position;
  float value;
  int number_of_components;
  /*enum Cell_components *components;*/
  char **components;
  struct Control_curve *control_curve[3]; /* can have 3 control curves per parameter */
  int *edit_distributed; /* a pointer to (cell->distributed).edit */
  struct Cell_parameter *next;
}; /* Cell_parameter */

/*
Global functions
================
*/
int set_parameter_information(struct Cell_window *cell,char *array,
  char *position,char *name,char *label,char *units,char *spatial,
  char *time_variable,char *value,int default_value);
/*******************************************************************************
LAST MODIFIED : 24 May 1999

DESCRIPTION :
Sets the information in the parameter structure for use when creating the
parameters dialog. The <name> is used as the label for the spatial toggle.
If <default_value> is 0, then the parameter is not a default value, and MUST
replace one in the list, if it does not correspond to a parameter already in
the list, then it is ignored.

<array> specifies the computational array the parameter is associated with,
and <array_position> gives the parameter's position in that array. Both are
used when writing out ipcell files.

?? Assume that the default values are always read in first, which sets all
parameter values and their order, so that when another set of parameters is
read in from a model file the existing parameters will simply be replaced by
the newer ones, keeping the order the same. ??
==============================================================================*/
int set_parameter_cell_component(struct Cell_window *cell,char *component);
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Sets <component> as a cell component of the last Cell_parameter in the
cell->parameters array.
==============================================================================*/
void destroy_cell_parameters(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 17 February 1999

DESCRIPTION :
Destroys the current list of parameters.
==============================================================================*/
#endif /* !defined (CELL_PARAMETER_H) */
