/*******************************************************************************
FILE : element_point_field_viewer_widget.h

LAST MODIFIED : 22 May 2000

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a element_point. One field at a time can be viewed/edited.
Note the element_point passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (ELEMENT_POINT_FIELD_VIEWER_WIDGET_H)
#define ELEMENT_POINT_FIELD_VIEWER_WIDGET_H

#include "general/callback.h"
#include "finite_element/finite_element.h"

/*
Global Functions
----------------
*/

Widget create_element_point_field_viewer_widget(
	Widget *element_point_field_viewer_widget_address,Widget parent,
	struct FE_element *element_point,FE_value *xi,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Widget for displaying and editing computed field components at element points.
==============================================================================*/

int element_point_field_viewer_widget_set_callback(
	Widget element_point_field_viewer_widget,struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Sets the callback for updates when the field of the element_point in the viewer
has been modified.
==============================================================================*/

int element_point_field_viewer_widget_get_element_point_field(
	Widget element_point_field_viewer_widget,
	struct FE_element **element,FE_value *xi,struct Computed_field **field);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Returns the element_point/field being edited in the
<element_point_field_viewer_widget>.
==============================================================================*/

int element_point_field_viewer_widget_set_element_point_field(
	Widget element_point_field_viewer_widget,
	struct FE_element *element,FE_value *xi,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements to this widget.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_FIELD_VIEWER_WIDGET_H) */
