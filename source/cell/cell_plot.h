/*******************************************************************************
FILE : cell_plot.h

LAST MODIFIED : 13 March 2001

DESCRIPTION :
The object used to draw plots in Cell.
==============================================================================*/
#if !defined (CELL_PLOT_H)
#define CELL_PLOT_H

#include "cell/cell_interface.h"
#include "cell/cell_variable.h"

/*
Module objects
--------------
*/
struct Cell_plot;
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
The cell plot object
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_plot *CREATE(Cell_plot)(Widget parent);
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Creates a cell plot object. <parent> must be a form??
==============================================================================*/
int DESTROY(Cell_plot)(struct Cell_plot **plot_address);
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Destroys the given <plot>.
==============================================================================*/
int Cell_plot_set_pane_sizes(struct Cell_plot *plot,Widget shell);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Sets the sizes of the plot and key drawing area's, proportional to the height of
the given <shell> widget. The <shell> will also be resized to accomodate the new
dimensions of the paned window.
==============================================================================*/
int Cell_plot_add_variable(struct Cell_plot *plot,
  struct Cell_variable *variable);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Adds the given <variable> to the <plot>'s current data sets.
==============================================================================*/
int Cell_plot_remove_variable(struct Cell_plot *plot,
  struct Cell_variable *variable);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Removes the given <variable> from the <plot>'s current data sets.
==============================================================================*/

#endif /* !defined (CELL_PLOT_H) */
