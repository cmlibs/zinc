/*******************************************************************************
FILE : cell_output.h

LAST MODIFIED : 02 February 2001

DESCRIPTION :
Output routines for the cell interface.
==============================================================================*/
#if !defined (CELL_OUTPUT_H)
#define CELL_OUTPUT_H

#include "cell/cell_calculate.h"
#include "cell/cell_input.h"
#include "cell/cell_interface.h"
#include "cell/cell_variable.h"

/*
Module objects
--------------
*/
struct Cell_output;
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
A data object used to output cell models into a Cell configuration file.
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_output *CREATE(Cell_output)(struct Cell_input *cell_input,
  struct Cell_calculate *cell_calculate);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Creates a Cell_output object.
==============================================================================*/
int DESTROY(Cell_output)(struct Cell_output **cell_output_address);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Destroys a Cell_output object.
==============================================================================*/
int Cell_output_write_model_to_file(struct Cell_output *cell_output,
  char *filename,struct LIST(Cell_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Writes the current Cell configuration to a configuration file.
==============================================================================*/
int Cell_output_write_model_to_ipcell_file(char *filename,
  struct LIST(Cell_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Writes the current Cell model to a ipcell file
==============================================================================*/
int Cell_output_write_model_to_ipcell_file_from_fields(char *filename,
  struct LIST(Cell_variable) *variable_list,
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Writes the current Cell model to a ipcell file, using any defined fields
==============================================================================*/
int Cell_output_write_model_to_ipmatc_file_from_fields(char *filename,
  void *element_group_void,void *grid_field_void,
  struct Cell_cmgui_interface *cmgui_interface,
  struct LIST(Cell_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports the spatially varying variables to a ipmatc file, from element based
fields found in the element group. <grid_field> is used as the grid point
number field.
==============================================================================*/
#endif /* !defined (CELL_OUTPUT_H) */
