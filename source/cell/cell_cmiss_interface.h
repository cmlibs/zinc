/*******************************************************************************
FILE : cell_cmiss_interface.h

LAST MODIFIED : 15 March 2001

DESCRIPTION :
Routines for using the Cell_cmiss_interface objects which map Cell_variables
into cmiss arrays
==============================================================================*/
#if !defined (CELL_CMISS_INTERFACE_H)
#define CELL_CMISS_INTERFACE_H

#include "cell/cell_variable.h"

/*
Module objects
--------------
*/
struct Cell_cmiss_interface;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
A data object which stores mapping information to transform Cell_variable
objects into cmiss arrays
==============================================================================*/

/*
Global objects
--------------
*/
/* The types for the CMISS variable arrays */
typedef CELL_DOUBLE CellTime;
typedef CELL_DOUBLE CellY;
typedef CELL_DOUBLE CellDY;
typedef CELL_DOUBLE CellDerived;
typedef CELL_DOUBLE CellParameters;
typedef CELL_DOUBLE CellProtocol;
typedef CELL_DOUBLE CellARI;
typedef CELL_DOUBLE CellARO;
typedef CELL_INTEGER CellControl;
typedef CELL_INTEGER CellModel;
typedef CELL_INTEGER CellSizes;
typedef CELL_INTEGER CellVariant;
typedef CELL_INTEGER CellAII;
typedef CELL_INTEGER CellAIO;
typedef CELL_INTEGER CellErrorCode;
/* The declaration for the Cell model routine - the standard "black box" !! */
typedef void (*CellModelRoutine)(CellTime *,CellY *,CellDY *,CellControl *,
  CellModel *,CellSizes *,CellVariant *,CellDerived *,CellParameters *,
  CellProtocol *,CellAII *,CellAIO *,CellARI *,CellARO *,CellErrorCode *);
/* The declaration for the Cell integration routine - the standard
   "black box" !! */
typedef void (*CellIntegratorRoutine)(CellAII *,CellAIO *,CellControl *,
  CellModel *,CELL_INTEGER *,CELL_INTEGER *,CellSizes *,CellVariant *,
  CellARI *,CellARO *,CellDerived *,CellParameters *,CellProtocol *,
  CELL_DOUBLE *,CELL_DOUBLE *,CellTime *,CellY *,CellModelRoutine,
  CELL_INTEGER *,CELL_DOUBLE *,CELL_INTEGER *,CELL_DOUBLE *,char *);

union Cell_cmiss_interface_value_address
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
An object used for keeping track of variable time histories.
==============================================================================*/
{
  /* All CMISS variables are either double or integer */
  CELL_DOUBLE *double_value_address;
  CELL_INTEGER *integer_value_address;
}; /* union Cell_cmiss_interface_value_address */

struct Cell_cmiss_interface_arrays
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
An object containing all the variables used in the CMISS cell model routines.

It is expected that this object is only ever created as one of the first steps
in a model solve, and destroyed immediately after the solve.
==============================================================================*/
{
  int size_time;
  CellTime *time;
  char **time_names;
	int num_odes;
  int size_y;
  CellY *y;
  char **y_names;
  int size_dy;
  CellDY *dy;
  char **dy_names;
  int size_derived;
  CellDerived *derived;
  char **derived_names;
  int size_parameters;
  CellParameters *parameters;
  char **parameters_names;
  int size_protocol;
  CellProtocol *protocol;
  char **protocol_names;
  int size_ari;
  CellARI *ari;
  char **ari_names;
  int size_aro;
  CellARO *aro;
  char **aro_names;
  int size_control;
  CellControl *control;
  char **control_names;
  int size_model;
  CellModel *model;
  char **model_names;
  int size_sizes;
  CellSizes *sizes;
  char **sizes_names;
  int size_variant;
  CellVariant *variant;
  char **variant_names;
  int size_aii;
  CellAII *aii;
  char **aii_names;
  int size_aio;
  CellAIO *aio;
  char **aio_names;
  int size_error_code;
  CellErrorCode *error_code;
  char **error_code_names;
}; /* struct Cell_cmiss_interface_arrays */

/*
Global functions
----------------
*/
struct Cell_cmiss_interface *CREATE(Cell_cmiss_interface)(char *array_string,
  int position);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Creates a Cell_cmiss_interface object. <array_string> is used to set the
variable's CMISS array and <position> is the variable's position in that array.
==============================================================================*/
int DESTROY(Cell_cmiss_interface)(
  struct Cell_cmiss_interface **cell_cmiss_interface_address);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Destroys a Cell_cmiss_interface object.
==============================================================================*/
void Cell_cmiss_interface_list(
  struct Cell_cmiss_interface *cell_cmiss_interface);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Lists out the <cell_cmiss_interface>
==============================================================================*/
struct Cell_cmiss_interface_arrays *CREATE(Cell_cmiss_interface_arrays)(
  struct LIST(Cell_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Creates a Cell_cmiss_interface_array object. If <variable_list> is non-NULL it
is used to create the CMISS arrays, otherwise they will all be NULL.
==============================================================================*/
int DESTROY(Cell_cmiss_interface_arrays)(
  struct Cell_cmiss_interface_arrays **cell_cmiss_interface_arrays_address);
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Destroys a Cell_cmiss_interface_arrays object.
==============================================================================*/
union Cell_cmiss_interface_value_address
*Cell_cmiss_interface_get_variable_value_address(
  struct Cell_cmiss_interface *cmiss_interface,
  struct Cell_cmiss_interface_arrays *cell_cmiss_interface_arrays);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns a pointer to the address of the given variable's position in the
<cmiss_arrays>.
==============================================================================*/
int Cell_cmiss_interface_set_ode(struct Cell_cmiss_interface *cmiss_interface);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the ode field of the given <cmiss_interface> object to be true.
==============================================================================*/

#endif /* !defined (CELL_CMISS_INTERFACE_H) */
