/*******************************************************************************
FILE : cell_unemap_interface.h

LAST MODIFIED : 03 April 2001

DESCRIPTION :
The interface between Cell and UnEMAP
==============================================================================*/
#if !defined (CELL_UNEMAP_INTERFACE_H)
#define CELL_UNEMAP_INTERFACE_H

#include "cell/cell_interface.h"
#include "cell/cell_variable_unemap_interface.h"

/*
Module objects
--------------
*/
struct Cell_unemap_interface;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
A data object which stores all the information which enables Cell to access
UnEMAP
==============================================================================*/

/*
Global objects
--------------
*/

/*
Global functions
----------------
*/
struct Cell_unemap_interface *CREATE(Cell_unemap_interface)(
  Widget cell_interface_shell,Widget cell_interface_window,
  struct Time_keeper *time_keeper,
  struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Creates a Cell_unemap_interface object.
==============================================================================*/
int DESTROY(Cell_unemap_interface)(
  struct Cell_unemap_interface **cell_unemap_interface_address);
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Destroys a Cell_unemap_interface object.
==============================================================================*/
int Cell_unemap_interface_close(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Closes a Cell_unemap_interface object.
==============================================================================*/
int Cell_unemap_interface_pop_up_analysis_window(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Pops up the UnEmap analysis window.
==============================================================================*/
int Cell_unemap_interface_pop_down_analysis_window(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the UnEmap analysis window.
==============================================================================*/
int Cell_unemap_interface_add_signals(
  struct Cell_unemap_interface *cell_unemap_interface,int number_of_signals,
  struct Cell_variable_unemap_interface **variable_unemap_interfaces,
  float tabT);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Adds the signals defined by the array of <variable_unemap_interfaces> to the
analysis work area given in the <cell_unemap_interface>. If the save signals
toggle is set to true, then the new values are added to any existing devices
where matches are found and new devices are created for any missing variables.
If the save signals toggle is set to false, then any existins buffers and
devices are destroyed and new ones are created.
==============================================================================*/
int Cell_unemap_interface_clear_analysis_work_area(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Clears the analysis work area and destroys all the signals associated with it.
==============================================================================*/
int Cell_unemap_interface_update_analysis_work_area(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Updates the analsysis work area widgets.
==============================================================================*/
int Cell_unemap_interface_set_save_signals(
  struct Cell_unemap_interface *cell_unemap_interface,int save);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Sets the save signals toggle in the <cell_unemap_interface> object.
==============================================================================*/
int Cell_unemap_interface_get_save_signals(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Gets the save signals toggle in the <cell_unemap_interface> object.
==============================================================================*/

#endif /* !defined (CELL_UNEMAP_INTERFACE_H) */
