/*******************************************************************************
FILE : parameter_dialog.h

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions and structures for using the parameter dialog boxes.
==============================================================================*/
#if !defined (CELL_PARAMETER_DIALOG_H)
#define CELL_PARAMETER_DIALOG_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/cell_window.h"
#include "cell/cell_parameter.h"

/*
Global types
============
*/
struct Parameter_dialog
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Structure containing information related to the "Parameter" dialogs.
==============================================================================*/
{
  int number_of_parameters;
  Widget shell;
  Widget window;
  Widget parameter_rowcol;
  Widget *labels;
  Widget *units;
  Widget *values;
  Widget *control_curve_toggles;
  Widget *control_curve_buttons;
  Widget *spatial_toggles;
}; /* Parameter_dialog */

/*
Global functions
================
*/
int bring_up_parameter_dialog(struct Cell_component *component);
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
It the component's dialog exists, bring it up, else create a new dialog.
==============================================================================*/

int bring_up_parameter_dialog_iterator(struct Cell_component *component,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
List iterator function for bringing up parameter dialogs for a cell component.
==============================================================================*/

void close_parameter_dialog(struct Cell_component *component);
/*******************************************************************************
LAST MODIFIED : 07 February 1999

DESCRIPTION :
If the <component> has a parameter dialog, destroy it. Used when
parameters are read from file to force the dialog to be re-created.
==============================================================================*/

void update_parameter_dialog_boxes(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 15 September 1999

DESCRIPTION :
Updates all parameter dialog boxes, used when values are reset via a FE_node
==============================================================================*/

#endif /* !defined (CELL_VARIABLES_DIALOG_H) */
