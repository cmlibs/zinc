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
#if !defined (CHOOSER_H)
#define CHOOSER_H

#include <Xm/Xm.h>
#include "general/callback_motif.h"
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
