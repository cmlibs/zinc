/*******************************************************************************
FILE : interactive_toolbar_widget.h

LAST MODIFIED : 8 May 2000

DESCRIPTION :
Widget for choosing the Interactive_tool currently in-use in a dialog from a
series of icons presented as radio buttons.
==============================================================================*/
#if !defined (INTERACTIVE_TOOLBAR_WIDGET_H)
#define INTERACTIVE_TOOLBAR_WIDGET_H

#include "general/callback.h"
#include "interaction/interactive_tool.h"

/*
Global Functions
----------------
*/

Widget create_interactive_toolbar_widget(Widget parent,
	struct MANAGER(Interactive_tool) *interactive_tool_manager);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Creates a RowColumn widget for storing a set of radio buttons for selecting the
current Interactive_tool. Tools must be added with the
interactive_toolbar_widget_add_tool function; the <interactive_tool_manager> is
supplied to allow tools to be automatically removed from the tool bar if they
are removed from the manager - will have no effect on any unmanaged tools added
to the toolbar.
==============================================================================*/

int interactive_toolbar_widget_set_callback(Widget interactive_toolbar_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets callback for when a different interactive_tool is chosen in the toolbar.
==============================================================================*/

struct
Interactive_tool *interactive_toolbar_widget_get_current_interactive_tool(
	Widget interactive_toolbar_widget);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Returns the currently chosen interactive_tool in <interactive_toolbar_widget>.
==============================================================================*/

int interactive_toolbar_widget_set_current_interactive_tool(
	Widget interactive_toolbar_widget,struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets the current interactive_tool in the <interactive_toolbar_widget>.
==============================================================================*/

int interactive_toolbar_widget_add_interactive_tool(
	Widget interactive_toolbar_widget,struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Adds <interactive_tool> to the toolbar widget. Tool is represented by a button
with the tool's icon on it. If this is the first tool added to the toolbar then
it is automatically chosen as the current tool.
==============================================================================*/

int add_interactive_tool_to_interactive_toolbar_widget(
	struct Interactive_tool *interactive_tool,
	void *interactive_toolbar_widget_void);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Iterator version of interactive_toolbar_widget_add_interactive_tool.
==============================================================================*/

#endif /* !defined (INTERACTIVE_TOOLBAR_WIDGET_H) */
