/*******************************************************************************
FILE : output.h

LAST MODIFIED : 14 September 1999

DESCRIPTION :
Functions for handling all file output for CELL.
==============================================================================*/
#if !defined (CELL_OUTPUT_H)
#define CELL_OUTPUT_H

#include <stdio.h>
#include "cell/cell_window.h"

/*
Global functions
================
*/
int write_time_variable_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 02 August 1999

DESCRIPTION :
Write time variables to the specified <file>.
==============================================================================*/
#if defined (OLD_CODE)
int write_variables_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
The function called when a variables file is selected via the file selection
dialog box, file -> write -> variables file.
==============================================================================*/
#endif /* defined (OLD_CODE) */
int write_model_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
The function called when a model file is selected via the file selection
dialog box, file -> write -> model file.
==============================================================================*/
int write_cmiss_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 12 May 1999

DESCRIPTION :
The function called when a cmiss file is selected via the file selection
dialog box, file -> write -> cmiss file.
==============================================================================*/
int export_FE_node_to_ipcell(char *filename,struct FE_node *node,
  struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports parameter and variable fields from the <node> to the ipcell file given
by <filename>.
==============================================================================*/
int export_FE_node_group_to_ipmatc(char *filename,
	struct GROUP(FE_node) *node_group,struct Cell_window *cell,
	struct FE_node *first_node,int offset);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports the spatially varying parameters to a ipmatc file, from the node group.
<offset> specifies the offset of the grid point number from the node number.
==============================================================================*/

#endif /* defined (CELL_OUTPUT_H) */
