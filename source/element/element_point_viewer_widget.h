/*******************************************************************************
FILE : element_point_viewer_widget.h

LAST MODIFIED : 11 May 2000

DESCRIPTION :
Widget for editing field values stored at an element point with multiple text
fields, visible one field at a time. Any computed field may be viewed. Most can
be modified too.
Note the element passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (ELEMENT_POINT_VIEWER_WIDGET_H)
#define ELEMENT_POINT_VIEWER_WIDGET_H

#include "general/callback.h"
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"

/*
Global Functions
----------------
*/

Widget create_element_point_viewer_widget(
	Widget *element_point_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct FE_element *initial_element,FE_value *initial_xi);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Creates a widget for displaying and editing the contents of the element point
at <initial_element><initial_xi>. Can also pass a NULL element here and use the
Element_Point_viewer_widget_set_element_point function instead.
<initial_element> should be a local copy of a global element; up to the
parent dialog to make changes global.
==============================================================================*/

int element_point_viewer_widget_set_callback(Widget element_point_viewer_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Sets the callback for updates when the contents of the element point in the
viewer have changed.
==============================================================================*/

int element_point_viewer_widget_get_element_point(
	Widget element_point_viewer_widget,struct FE_element **element_address,
	FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns the element_point being edited in the <element_point_viewer_widget>.
==============================================================================*/

int element_point_viewer_widget_set_element_point(
	Widget element_point_viewer_widget,struct FE_element *element,FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements to this widget.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_VIEWER_WIDGET_H) */
