/*******************************************************************************
FILE : cell_output.h

LAST MODIFIED : 02 February 2001

DESCRIPTION :
Output routines for the cell interface.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
