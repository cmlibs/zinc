/*******************************************************************************
FILE : cell_calculate_dialog.h

LAST MODIFIED : 22 January 2001

DESCRIPTION :
Routines for the model calculation dialog
==============================================================================*/
#if !defined (CELL_CALCULATE_DIALOG_H)
#define CELL_CALCULATE_DIALOG_H

#include "cell/cell_calculate.h"
#include "cell/cell_interface.h"

/*
Module objects
--------------
*/
struct Cell_calculate_dialog;
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
The object for the model calculation dialog
==============================================================================*/

/*
Global objects
--------------
*/

/*
Global functions
----------------
*/
struct Cell_calculate_dialog *CREATE(Cell_calculate_dialog)(
  struct Cell_calculate *cell_calculate,Widget parent,
  struct Cell_interface *cell_interface,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Creates the calculation dialog
==============================================================================*/
int DESTROY(Cell_calculate_dialog)(
  struct Cell_calculate_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Destroys the Cell_calculate_dialog object.
==============================================================================*/
int Cell_calculate_dialog_pop_up(struct Cell_calculate_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Pops up the calculate dialog
==============================================================================*/
int Cell_calculate_dialog_pop_down(struct Cell_calculate_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the calculate dialog
==============================================================================*/

#endif /* !defined (CELL_CALCULATE_DIALOG_H) */
