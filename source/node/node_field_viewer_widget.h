/*******************************************************************************
FILE : node_field_viewer_widget.h

LAST MODIFIED : 22 May 2000

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a node. One field at a time can be viewed/edited.
Note the node passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (NODE_FIELD_VIEWER_WIDGET_H)
#define NODE_FIELD_VIEWER_WIDGET_H

#include "general/callback.h"
#include "finite_element/finite_element.h"

/*
Global Functions
----------------
*/

Widget create_node_field_viewer_widget(Widget *node_field_viewer_widget_address,
	Widget parent,struct FE_node *node,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

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
