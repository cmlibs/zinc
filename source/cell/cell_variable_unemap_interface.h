/*******************************************************************************
FILE : cell_variable_unemap_interface.h

LAST MODIFIED : 06 November 2000

DESCRIPTION :
The interface between Cell_variable's and UnEMAP
==============================================================================*/
#if !defined (CELL_VARIABLE_UNEMAP_INTERFACE_H)
#define CELL_VARIABLE_UNEMAP_INTERFACE_H

#include "cell/cell_cmiss_interface.h"

/*
Module objects
--------------
*/
struct Cell_variable_unemap_interface;
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
A data object which stores all the information which enables Cell_variable's
to access UnEMAP
==============================================================================*/

/*
Global objects
--------------
*/

/*
Global functions
----------------
*/
struct Cell_variable_unemap_interface *CREATE(Cell_variable_unemap_interface)(
  enum Cell_value_type value_type,char *name);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Creates a Cell_variable_unemap_interface object.
==============================================================================*/
int DESTROY(Cell_variable_unemap_interface)(
  struct Cell_variable_unemap_interface
  **cell_variable_unemap_interface_address);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Destroys a Cell_variable_unemap_interface object.
==============================================================================*/
int Cell_variable_unemap_interface_add_value(
  struct Cell_variable_unemap_interface *unemap_interface,
  union Cell_cmiss_interface_value_address *value_address);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Adds the given <value> to the end of the <unemap_interface> time history
array. Allocates the memory required.
==============================================================================*/
int Cell_variable_unemap_interface_reset_values(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Resets the value array of the given variable <unemap_interface>.
==============================================================================*/
char *Cell_variable_unemap_interface_get_value_as_string_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a string.
==============================================================================*/
float Cell_variable_unemap_interface_get_value_as_float_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a float.
==============================================================================*/
int Cell_variable_unemap_interface_get_number_of_values(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 04 November 2000

DESCRIPTION :
Returns the number of values stored in the <unemap_interface>
==============================================================================*/
char *Cell_variable_unemap_interface_get_name(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Returns a copy of the name of the <unemap_interface>
==============================================================================*/

#endif /* !defined (CELL_VARIABLE_UNEMAP_INTERFACE_H) */
