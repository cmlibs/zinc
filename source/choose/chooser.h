/*******************************************************************************
FILE : chooser.h

LAST MODIFIED : 20 January 2000

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (CHOOSER_H)
#define CHOOSER_H

#include <Xm/Xm.h>
#include "general/callback.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
Widget CREATE(Chooser)(Widget parent,int number_of_items,void **items,
	char **item_names,void *current_item);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Creates a menu from which any of the given <items> with <item_names> may be
chosen.
==============================================================================*/

int Chooser_build_main_menu(Widget chooser_widget,int number_of_items,
	void **items,char **item_names,void *new_item);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Makes a cascading menu of the <items> labelled with the given <item_names>.
Clears existing menu and detaches it from the main_cascade if required.
The new menu is attached to the main_cascade button.
==============================================================================*/

int Chooser_set_destroy_callback(Widget chooser_widget,
	struct Callback_data *new_destroy_callback);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Changes the destroy callback of the chooser_widget.
==============================================================================*/

int Chooser_set_update_callback(Widget chooser_widget,
	struct Callback_data *new_update_callback);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Changes the update callback of the chooser_widget.
==============================================================================*/

void *Chooser_get_item(Widget chooser_widget);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Returns the currently chosen item in the chooser_widget.
==============================================================================*/

int Chooser_set_item(Widget chooser_widget,void *new_item);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Changes the chosen item in the chooser_widget.
==============================================================================*/

#endif /* !defined (CHOOSER_H) */
