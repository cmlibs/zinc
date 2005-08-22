/*******************************************************************************
FILE : interactive_toolbar_widget.h

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Widget for choosing the Interactive_tool currently in-use in a dialog from a
series of icons presented as radio buttons.
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
#if !defined (INTERACTIVE_TOOLBAR_WIDGET_H)
#define INTERACTIVE_TOOLBAR_WIDGET_H

#include "general/callback_motif.h"
#include "interaction/interactive_tool.h"

/*
Global Types
------------
*/

enum Interactive_toolbar_orientation
{
	INTERACTIVE_TOOLBAR_HORIZONTAL,
	INTERACTIVE_TOOLBAR_VERTICAL
};

/*
Global Functions
----------------
*/

Widget create_interactive_toolbar_widget(Widget parent,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	enum Interactive_toolbar_orientation orientation);
/*******************************************************************************
LAST MODIFIED : 28 August 2000

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
