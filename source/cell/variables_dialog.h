/*******************************************************************************
FILE : variables_dialog.h

LAST MODIFIED : 13 March 1999

DESCRIPTION :
Functions and structures for using the variables dialog box.
==============================================================================*/
#if !defined (CELL_VARIABLES_DIALOG_H)
#define CELL_VARIABLES_DIALOG_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/cell_window.h"
#include "cell/cell_variable.h"

/*
Global types
============
*/
struct Variables_dialog_user_settings
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Holds the information related to the variables dialog specified in the resource
file.
==============================================================================*/
{
	Pixel name_colour,value_colour,units_colour,label_colour;
};

struct Variables_dialog
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Structure containing information related to the "Variables" dialog.
==============================================================================*/
{
  Widget shell;
  Widget window;
  Widget rowcol;
}; /* Variables_dialog */

/*
Global functions
================
*/
int bring_up_variables_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
If there is a variables dialog in existence, then bring it to the front, else
create a new one.
==============================================================================*/
void close_variables_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 07 February 1999

DESCRIPTION :
If there is a variables dialog in existence, then destroy it. Used when
variables arfe read from file to force the dialog to be re-created.
==============================================================================*/

#endif /* !defined (CELL_VARIABLES_DIALOG_H) */
