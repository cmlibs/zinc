/*******************************************************************************
FILE : cell_export_dialog.h

LAST MODIFIED : 01 February 2001

DESCRIPTION :
The export dialog used to export cell variables to ipcell and ipmatc files
==============================================================================*/
#if !defined (CELL_EXPORT_DIALOG_H)
#define CELL_EXPORT_DIALOG_H

#include "cell/cell_interface.h"
#include "cell/cell_cmgui_interface.h"
#include "cell/distributed_editing_interface.h"

/*
Module types
============
*/
struct Cell_export_dialog;
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Used to store the information and widgets used to export ipcell and ipmatc
files from Cell.
==============================================================================*/

/*
Global functions
================
*/
struct Cell_export_dialog *CREATE(Cell_export_dialog)(
  struct Cell_interface *interface,
  struct Cell_cmgui_interface *cmgui_interface,
  struct Distributed_editing_interface *distributed_editing_interface,
  struct User_interface *user_interface,Widget parent);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Create a new export dialog.
==============================================================================*/
int DESTROY(Cell_export_dialog)(struct Cell_export_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Destroys a export dialog object.
==============================================================================*/
int Cell_export_dialog_pop_up(struct Cell_export_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops up the <dialog>
==============================================================================*/
int Cell_export_dialog_pop_down(struct Cell_export_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/

#endif
