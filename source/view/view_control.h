/*******************************************************************************
FILE : view_control.h

LAST MODIFIED : 21 January 1995

DESCRIPTION :
Interfaces the view widget to the 3-D graphics window.  At present it creates a
separate shell for the view widget.
==============================================================================*/
#if !defined (VIEW_CONTROL_H)
#define VIEW_CONTROL_H

#include "graphics/graphics_window.h"

/*
Global types
------------
*/
struct View_control
/*******************************************************************************
LAST MODIFIED : 21 January 1995

DESCRIPTION :
Contains information required by the camera control dialog.
==============================================================================*/
{
	Widget widget_parent,widget;
	Widget view_widget;
	struct Graphics_window *graphics_window;
}; /* struct View_control */

/*
Global functions
----------------
*/
struct View_control *create_View_control(Widget parent,
	struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 21 January 1995

DESCRIPTION :
Creates a view widget, a shell around the view widget (with the specified
<parent>) and sets the callbacks for the view widget to interface with the
<graphics_window>.
==============================================================================*/
#endif
