/*******************************************************************************
FILE : distributed_editing_dialog.h

LAST MODIFIED : 22 January 2001

DESCRIPTION :
The dialog box for distributed parameter editing
==============================================================================*/
#if !defined (DISTRIBUTED_EDITING_DIALOG_H)
#define DISTRIBUTED_EDITING_DIALOG_H

#include "cell/distributed_editing_interface.h"

/*
Global types
============
*/
struct Distributed_editing_dialog
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
The distributed editing dialog object.
==============================================================================*/
{
  /* A Pointer to the distributed editing interface */
  struct Distributed_editing_interface *interface;
  /* The dialog shell widget */
  Widget shell;
  /* The dialog main window */
  Widget window;
  /* The main user interface object */
  struct User_interface *user_interface;
  /* The dialog widgets */
  Widget element_form,element_widget,point_number_text,
    grid_field_form,grid_field_widget,grid_value_text,
    description_label;
  /* The activation toggle button widget */
  Widget activation;
}; /* struct Distributed_editing_dialog */

/*
Global functions
----------------
*/
struct Distributed_editing_dialog *CREATE(Distributed_editing_dialog)(
  struct Distributed_editing_interface *interface,Widget parent,
  struct User_interface *user_interface,Widget activation);
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
Creates a distributed editing dialog object. <activation> is the toggle button
widget which activated the dialog.
==============================================================================*/
int DESTROY(Distributed_editing_dialog)(
  struct Distributed_editing_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Destroys a distributed editing dialog object.
==============================================================================*/
int Distributed_editing_dialog_pop_up(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Brings up the <dialog>
==============================================================================*/
int Distributed_editing_dialog_pop_down(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/
int Distributed_editing_dialog_get_activation_state(
  struct Distributed_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Returns 1 if the distributed editing toggle is set, 0 otherwise. The
distributed editing toggle is stored as the activation widget for the dialog.
==============================================================================*/

#endif /* !defined (DISTRIBUTED_EDITING_DIALOG_H) */
