/*******************************************************************************
FILE : calculate.h

LAST MODIFIED : 26 August 1999

DESCRIPTION :
Functions for calculating and displaying the model solutions.
==============================================================================*/
#if !defined (CELL_CALCULATE_H)
#define CELL_CALCULATE_H

#include "cell/cell_window.h"

/*
Global types
============
*/
enum Cell_output_type
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
The different types of output.
???DB.  Should prefix all with CELL_OUTPUT_
==============================================================================*/
{
  VARIABLE,
  CURRENT,
  RATE,
  MECHANICS,
  CELL_OUTPUT_UNKNOWN
}; /* enum Cell_output_type */

struct Cell_output
/*******************************************************************************
LAST MODIFIED : 03 March 1999

DESCRIPTION :
Stores information for each of the outputs of the model.
==============================================================================*/
{
  char *name;
  float value;
  enum Cell_output_type type;
  struct Cell_output *next;
}; /* struct Cell_output */

/*
Global functions
================
*/
void calculate_cell_window (struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 01 March 1999

DESCRIPTION :
Calculates the cell window.
==============================================================================*/
void clear_signals_window(Widget *widget_id,XtPointer cell_window,
  XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 01 March 1999

DESCRIPTION :
Clears the signals window.
==============================================================================*/
int set_output_information(struct Cell_window *cell,char *name,char *type);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the information in the output's structure for use with UnEMAP. Should only
be called when reading the default files.
==============================================================================*/
void destroy_cell_outputs(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 03 March 1999

DESCRIPTION :
Destroys the current list of outputs.
==============================================================================*/

#endif /* if !defined (CELL_CALCULATE_H) */
