/*******************************************************************************
FILE : chooser.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Widget allowing choice of a void *item from a named list using cascading menus.
Calls the client-specified callback routine if a different item is chosen.
These Chooser functions should be seen as core functions for more specific types
of chooser objects. The calling code is given ownership of the widget for the
Chooser once created and may add userdata and destroy callbacks to it to clean
up itself. As a consequence, it is up to the calling code to DESTROY its
Chooser.
==============================================================================*/
#if !defined (CHOOSER_H)
#define CHOOSER_H

#include <Xm/Xm.h>
#include "general/callback.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/
struct Chooser;

/*
Global Functions
----------------
*/

struct Chooser *CREATE(Chooser)(Widget parent, int number_of_items,
	void **items, char **item_names, void *current_item,
	Widget *chooser_widget, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Creates a menu from which any of the given <items> with <item_names> may be
chosen. Returns the <chooser_widget> which is then owned by the calling code.
Note that it has no userdata and no destroy callbacks associated with it - but
these may be added by the calling code.
<user_interface> supplies fonts.
==============================================================================*/

int DESTROY(Chooser)(struct Chooser **chooser_address);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Cleans up the chooser structure. Note does not destroy widgets!
==============================================================================*/

int Chooser_build_main_menu(struct Chooser *chooser,int number_of_items,
	void **items,char **item_names,void *new_item);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Makes a cascading menu of the <items> labelled with the given <item_names>.
Clears existing menu and detaches it from the main_cascade if required.
The new menu is attached to the main_cascade button.
==============================================================================*/

int Chooser_set_update_callback(struct Chooser *chooser,
	struct Callback_data *new_update_callback);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Changes the update callback of the chooser_widget.
==============================================================================*/

void *Chooser_get_item(struct Chooser *chooser);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Returns the currently chosen item in the chooser_widget.
==============================================================================*/

int Chooser_set_item(struct Chooser *chooser,void *new_item);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Changes the chosen item in the chooser_widget.
==============================================================================*/

#endif /* !defined (CHOOSER_H) */
