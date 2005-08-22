/*******************************************************************************
FILE : node_viewer_widget.h

LAST MODIFIED : 24 January 2003

DESCRIPTION :
Widget for editing the contents of a node with multiple text fields, visible
one field at a time. Any computed field may be viewed. Most can be modified too.
Note the node passed to this widget should be a non-managed local copy.
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
#if !defined (NODE_VIEWER_WIDGET_H)
#define NODE_VIEWER_WIDGET_H

#include "general/callback.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "time/time.h"
#include "user_interface/user_interface.h"

/*
Global Functions
----------------
*/

Widget create_node_viewer_widget(Widget *node_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct FE_region *fe_region, struct FE_node *initial_node,
	struct Time_object *time_object, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Creates a widget for displaying and editing the contents of <initial_node> from
region <fe_region>. Can also pass a NULL node here and use the
Node_viewer_widget_set_node function instead. 
<initial_node> should be a local copy of a global node; up to the
parent dialog to make changes global.
==============================================================================*/

int node_viewer_widget_set_callback(Widget node_viewer_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the callback for updates when the contents of the node in the viewer have
changed.
==============================================================================*/

struct FE_node *node_viewer_widget_get_node(Widget node_viewer_widget);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node being edited in the <node_viewer_widget>.
==============================================================================*/

int node_viewer_widget_set_node(Widget node_viewer_widget,
	struct FE_region *fe_region, struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets the node being edited in the <node_viewer_widget>. Note that the viewer
works on the node itself, not a local copy. Hence, only pass unmanaged nodes to
this widget.
==============================================================================*/

#endif /* !defined (NODE_VIEWER_WIDGET_H) */
