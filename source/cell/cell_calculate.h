/*******************************************************************************
FILE : cell_calculate.h

LAST MODIFIED : 21 February 2001

DESCRIPTION :
Routines for model calculation
==============================================================================*/
#if !defined (CELL_CALCULATE_H)
#define CELL_CALCULATE_H

#include "cell/cell_variable.h"
#include "cell/cell_unemap_interface.h"

/*
Module objects
--------------
*/
struct Cell_calculate;
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
A data object which stores information used in the calculation of models
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_calculate *CREATE(Cell_calculate)(
  struct Cell_unemap_interface *cell_unemap_interface);
/*******************************************************************************
LAST MODIFIED : 04 November 2000

DESCRIPTION :
Creates a Cell_calculate object.
==============================================================================*/
int DESTROY(Cell_calculate)(struct Cell_calculate **cell_calculate_address);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Destroys a Cell_calculate object.
==============================================================================*/
int Cell_calculate_set_model_routine_name(struct Cell_calculate *cell_calculate,
  char *name);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Sets the model routine name for the given <cell_calculate> object.
==============================================================================*/
char *Cell_calculate_get_model_routine_name(
  struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Gets the model routine name for the given <cell_calculate> object.
==============================================================================*/
int Cell_calculate_set_dso_name(struct Cell_calculate *cell_calculate,
  char *name);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Sets the DSO name for the given <cell_calculate> object. <name> can be NULL,
which simply indictes that the model routine is compiled into the executable.
==============================================================================*/
char *Cell_calculate_get_dso_name(struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Gets the DSO name for the given <cell_calculate> object.
==============================================================================*/
int Cell_calculate_set_intg_routine_name(struct Cell_calculate *cell_calculate,
  char *name);
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets the integrator routine name for the given <cell_calculate> object.
==============================================================================*/
char *Cell_calculate_get_intg_routine_name(
  struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Gets the integrator routine name for the given <cell_calculate> object.
==============================================================================*/
int Cell_calculate_set_intg_dso_name(struct Cell_calculate *cell_calculate,
  char *name);
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets the integrator DSO name for the given <cell_calculate> object. <name> can
be NULL, which simply indictes that the model routine is compiled into the
executable.
==============================================================================*/
char *Cell_calculate_get_intg_dso_name(struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Gets the integrator DSO name for the given <cell_calculate> object.
==============================================================================*/
int Cell_calculate_calculate_model(struct Cell_calculate *cell_calculate,
  struct LIST(Cell_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Calculates the current model
==============================================================================*/
int Cell_calculate_pop_up_dialog(
  struct Cell_calculate *cell_calculate,Widget parent,
  struct Cell_interface *cell_interface,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Brings up the calculate dialog - creating it if required.
==============================================================================*/
int Cell_calculate_pop_down_dialog(struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the calculation dialog.
==============================================================================*/
char *Cell_calculate_get_start_time_as_string(
  struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration start time as a string.
==============================================================================*/
char *Cell_calculate_get_end_time_as_string(
  struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration end time as a string.
==============================================================================*/
char *Cell_calculate_get_dt_as_string(struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration dT as a string.
==============================================================================*/
char *Cell_calculate_get_tabt_as_string(struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration tabT as a string.
==============================================================================*/
char *Cell_calculate_set_start_time_from_string(
  struct Cell_calculate *cell_calculate,char *value_string);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration start time and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
char *Cell_calculate_set_end_time_from_string(
  struct Cell_calculate *cell_calculate,char *value_string);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration end time and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
char *Cell_calculate_set_dt_from_string(
  struct Cell_calculate *cell_calculate,char *value_string);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration dt and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
char *Cell_calculate_set_tabt_from_string(
  struct Cell_calculate *cell_calculate,char *value_string);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration tabt and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/

#endif /* !defined (CELL_CALCULATE_H) */
