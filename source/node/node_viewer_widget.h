/*******************************************************************************
FILE : node_viewer_widget.h

LAST MODIFIED : 11 May 2000

DESCRIPTION :
Widget for editing the contents of a node with multiple text fields, visible
one field at a time. Any computed field may be viewed. Most can be modified too.
Note the node passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (NODE_VIEWER_WIDGET_H)
#define NODE_VIEWER_WIDGET_H

#include "general/callback.h"
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"

/*
Global Functions
----------------
*/

Widget create_node_viewer_widget(Widget *node_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct FE_node *initial_node);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Creates a widget for displaying and editing the contents of <initial_node>. Can
also pass a NULL node here and use the Node_viewer_widget_set_node function
instead. <initial_node> should be a local copy of a global node; up to the
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

int node_viewer_widget_set_node(Widget node_viewer_widget,struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node being edited in the <node_viewer_widget>. Note that the viewer
works on the node itself, not a local copy. Hence, only pass unmanaged nodes to
this widget.
==============================================================================*/

#endif /* !defined (NODE_VIEWER_WIDGET_H) */
