/*******************************************************************************
FILE : cell_window.h

LAST MODIFIED : 20 November 2000

DESCRIPTION :
The Cell Interface main window.
==============================================================================*/
#if !defined (CELL_WINDOW_H)
#define CELL_WINDOW_H


#include "cell/cell_interface.h"

/*
Module objects
--------------
*/
struct Cell_window;
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
The Cell Interface main window structure.
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_window *CREATE(Cell_window)(
  struct Cell_interface *cell_interface,
  struct User_interface *user_interface,
  XtCallbackProc exit_callback);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Creates the Cell interface main window.
==============================================================================*/
int DESTROY(Cell_window)(struct Cell_window **cell_window_address);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Destroys the Cell_window object.
==============================================================================*/
void Cell_window_pop_up(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Brings up the <cell_window>
==============================================================================*/
int Cell_window_set_title_bar(struct Cell_window *cell_window,char *title);
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Sets the main windows <title>
==============================================================================*/
Widget Cell_window_get_shell(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the shell widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_window(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the window widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_unemap_activation_button(
  struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Returns the UnEmap activation widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_root_shell(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns the root shell widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_scene_form(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the scene form widget from the given <cell_window>
==============================================================================*/
Widget Cell_window_get_toolbar_form(struct Cell_window *cell_window);
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the toolbar form widget from the given <cell_window>
==============================================================================*/
void exit_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used in stand-alone Cell
==============================================================================*/
void close_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used when Cell is part of CMGUI
==============================================================================*/

#endif /* !defined (CELL_WINDOW_H) */
