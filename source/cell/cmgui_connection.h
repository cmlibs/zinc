/*******************************************************************************
FILE : cmgui_connection.h

LAST MODIFIED : 17 September 1999

DESCRIPTION :
Functions for talking between Cell and Cmgui.
==============================================================================*/
#if !defined (CMGUI_CONNECTION_H)
#define CMGUI_CONNECTION_H

#include "cell/cell_window.h"

/*
Global types
============
*/
struct Export_dialog
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Used to store the information and widgets used to export ipcell and ipmatc
files from Cell.
==============================================================================*/
{
  Widget shell;
  Widget window;
  Widget group_chooser_form;
  Widget group_chooser_widget;
  Widget ipcell_file_label;
  char *ipcell_file_name;
  char *ipmatc_file_name;
  Widget ipmatc_file_label;
	Widget offset_textfield;
}; /* Export_dialog */

/*
Global functions
================
*/
int check_model_id(struct Cell_window *cell,struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Checks the model_id field of the given <node> and returns 1 if this matches
the current model ID of the <cell>.
==============================================================================*/
int file_open_ipcell_callback(char *filename,XtPointer export_dialog);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the file selection dialog for the ipcell file in the export dialog.
==============================================================================*/
int file_open_ipmatc_callback(char *filename,XtPointer export_dialog);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the file selection dialog for the ipmatc file in the export dialog.
==============================================================================*/
int FE_node_to_Cell_window(struct Cell_window *cell,struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Extracts values from a FE_node and sets up the cell window.
==============================================================================*/
int Cell_window_to_FE_node(struct Cell_window *cell,
  struct FE_node *node_to_be_modified);
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Re-sets the <node> values from the Cell window.
==============================================================================*/
int bring_up_export_window(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
If a export window exists, pop it up, otherwise create it.
==============================================================================*/
void close_export_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
If there is a export dialog in existence, then destroy it.
==============================================================================*/
int check_field_type(struct FE_field *field,void *field_type_void);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns true if the FIELD_TYPE specified in the <field_type_void> matches
the FIELD_TYPE of the <field>.
==============================================================================*/
int Cell_window_update_field(struct Cell_window *cell,char *control_curve_name,
	char *field_name,char *file_name,char *element_number,float value);
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Temp. hack to use control curves to assign values to element based fields.
==============================================================================*/

#endif /* !defined (CMGUI_CONNECTION_H) */
