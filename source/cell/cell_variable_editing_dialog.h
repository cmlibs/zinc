/*******************************************************************************
FILE : cell_variable_editing_dialog.h

LAST MODIFIED : 22 January 2001

DESCRIPTION :
The dialog used for editing variables in Cell.
==============================================================================*/
#if !defined (CELL_VARIABLE_EDITING_DIALOG_H)
#define CELL_VARIABLE_EDITING_DIALOG_H

#include "cell/cell_component.h"
#include "cell/cell_interface.h"

/*
Module objects
--------------
*/
struct Cell_variable_widget_list;
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
A structure used to keep track of a particular variable's widgets.
==============================================================================*/
struct Cell_variable_editing_dialog;
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
The object used to store information about the Cell variable editing dialog.
==============================================================================*/

/*
Global objects
--------------
*/

/*
Global functions
----------------
*/
struct Cell_variable_editing_dialog *CREATE(Cell_variable_editing_dialog)(
  struct Cell_interface *cell_interface,Widget parent,
  struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Create a new parameter dialog for the cell <component>
==============================================================================*/
int DESTROY(Cell_variable_editing_dialog)(
  struct Cell_variable_editing_dialog **dialog_address);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Destroys the Cell_variable_editing_dialog object.
==============================================================================*/
int Cell_variable_editing_dialog_pop_up(
  struct Cell_variable_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Pops up the variable editing dialog
==============================================================================*/
int Cell_variable_editing_dialog_pop_down(
  struct Cell_variable_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the variable editing dialog
==============================================================================*/
int Cell_variable_editing_dialog_add_component(
  struct Cell_variable_editing_dialog *dialog,
  struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Adds the given <cell_component> to the variable editing <dialog>.
==============================================================================*/
int Cell_variable_editing_dialog_clear_dialog(
  struct Cell_variable_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Removes all the component widgets from the variable editing dialog.
==============================================================================*/
struct Cell_variable_widget_list *CREATE(Cell_variable_widget_list)(void);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Create a new widget list object
==============================================================================*/
int DESTROY(Cell_variable_widget_list)(
  struct Cell_variable_widget_list **list_address);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Destroys the Cell_variable_widget_list object.
==============================================================================*/
int Cell_variable_widget_list_add_widget(
  struct Cell_variable_widget_list *list,void *widget_void);
/*******************************************************************************
LAST MODIFIED : 18 January 2001

DESCRIPTION :
Adds the given <widget> to the <list>.
==============================================================================*/
int Cell_variable_widget_list_update_text_field_value(
  struct Cell_variable_widget_list *list,char *value_string);
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Sets the value of all the text field widgets in the widget <list> to the given
<value_string>
==============================================================================*/

#endif /* !defined (CELL_VARIABLE_EDITING_DIALOG_H) */
