/*******************************************************************************
FILE : node_viewer.h

LAST MODIFIED : 24 January 2003

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
FE_node_selection to display the last selected node, or set it if entered in
this dialog.
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
#if !defined (NODE_VIEWER_H)
#define NODE_VIEWER_H
 
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "selection/node_selection.h"
#include "time/time.h"
#include "user_interface/user_interface.h"
/*
Global Types
------------
*/

struct Node_viewer;
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Contains all the information carried by the node_viewer widget.
The contents of this object are private.
==============================================================================*/

/*
Global Functions
----------------
*/

struct Node_viewer *CREATE(Node_viewer)(
	struct Node_viewer **node_viewer_address,
	char *dialog_title,
	struct FE_node *initial_node,
	struct Cmiss_region *root_region,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object,	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Creates a dialog for choosing nodes and displaying and editing their fields.
Nodes, starting with <initial_node> may be chosen from <this_node_manager>.
Pass <initial_data> and <data_manager> in these parameters for data viewer.
Since both nodes and data can depend on embedded fields, the
<actual_node_manager> and <actual_element_manager> also need to be passed.
==============================================================================*/

int DESTROY(Node_viewer)(struct Node_viewer **node_viewer_address);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION:
Destroys the Node_viewer. See also Node_viewer_close_CB.
==============================================================================*/

struct FE_node *Node_viewer_get_node(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node currently in the <node_viewer>.
==============================================================================*/

int Node_viewer_set_node(struct Node_viewer *node_viewer,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Makes <node> the current_node in <node_viewer>.
==============================================================================*/

int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <node_viewer> to the front of those visible.
==============================================================================*/
#endif /* !defined (NODE_VIEWER_HPP) */
