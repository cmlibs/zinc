/*******************************************************************************
FILE : node_field_viewer_widget.h

LAST MODIFIED : 22 May 2000

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a node. One field at a time can be viewed/edited.
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
#if !defined (NODE_FIELD_VIEWER_WIDGET_H)
#define NODE_FIELD_VIEWER_WIDGET_H

#include "general/callback_motif.h"
#include "finite_element/finite_element.h"
#include "time/time.h"

/*
Global Functions
----------------
*/

Widget create_node_field_viewer_widget(Widget *node_field_viewer_widget_address,
	Widget parent,struct FE_node *node,struct Computed_field *field,
	struct Time_object *time_object);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Widget for displaying and editing computed field components/derivatives at a
node.
==============================================================================*/

int node_field_viewer_widget_set_callback(Widget node_field_viewer_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the callback for updates when the field of the node in the editor has been
modified.
==============================================================================*/

int node_field_viewer_widget_get_node_field(Widget node_field_viewer_widget,
	struct FE_node **node,struct Computed_field **field);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node/field being edited in the <node_field_viewer_widget>.
==============================================================================*/

int node_field_viewer_widget_set_node_field(Widget node_field_viewer_widget,
	struct FE_node *node,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node/field being edited in the <node_field_viewer_widget>. Note that
the editor works on the node itself, not a local copy. Hence, only pass
unmanaged nodes to this widget.
==============================================================================*/

#endif /* !defined (NODE_FIELD_VIEWER_WIDGET_H) */
