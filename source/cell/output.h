/*******************************************************************************
FILE : output.h

LAST MODIFIED : 16 June 2000

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
#if defined (CELL_DECENT_IPTIME_FILES)
int write_control_curve_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 02 August 1999

DESCRIPTION :
Write time variables to the specified <file>.
==============================================================================*/
#endif /* defined (CELL_DECENT_IPTIME_FILES) */
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
int export_to_ipcell(char *filename,struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 09 June 2000

DESCRIPTION :
Exports parameter and variable fields to the ipcell file given by <filename>.
==============================================================================*/
int export_to_ipmatc(char *filename,struct GROUP(FE_element) *element_group,
  struct FE_field *grid_field,struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Exports the spatially varying parameters to a ipmatc file, from element based
fields found in the element group. <grid_field> is used as the grid point
number field.
==============================================================================*/
#if defined (CELL_USE_NODES)
int export_FE_node_to_ipcell(char *filename,struct FE_node *node,
  struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports parameter and variable fields from the <node> to the ipcell file given
by <filename>.
==============================================================================*/
#endif /* defined (CELL_USE_NODES) */
#if defined (CELL_USE_NODES)
int export_FE_node_group_to_ipmatc(char *filename,
	struct GROUP(FE_node) *node_group,struct Cell_window *cell,
	struct FE_node *first_node,int offset);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports the spatially varying parameters to a ipmatc file, from the node group.
<offset> specifies the offset of the grid point number from the node number.
==============================================================================*/
#endif /* defined (CELL_USE_NODES) */
void write_ippara_file(char *filename);
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Writes out an ippara file for calculating.
==============================================================================*/
void write_ipequa_file(char *filename,char *model_name);
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Writes out an ipequa file for calculating.
==============================================================================*/
void write_ipmatc_file(char *filename);
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Writes out an ipmatc file for a single cell calculation. For single cell stuff,
no parameters are spatially varying ?!?
==============================================================================*/
void write_ipinit_file(char *filename);
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Writes out an ipinit file for a single cell calculation.
==============================================================================*/
void write_ipsolv_file(char *filename);
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Writes out an ipsolv file for a single cell calculation.
==============================================================================*/
void write_ipexpo_file(char *filename);
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Writes out an ipexpo file for a single cell calculation.
==============================================================================*/
void export_control_curves_to_file(struct Export_control_curve_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Exports all the control curves selected in the export dialog to a single iptime
file.
==============================================================================*/

#endif /* defined (CELL_OUTPUT_H) */
