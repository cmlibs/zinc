/*******************************************************************************
FILE : element_point_field_viewer_widget.h

LAST MODIFIED : 31 May 2000

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a element_point. One field at a time can be viewed/edited.
Note the element_point passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (ELEMENT_POINT_FIELD_VIEWER_WIDGET_H)
#define ELEMENT_POINT_FIELD_VIEWER_WIDGET_H

#include "general/callback.h"
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/element_point_ranges.h"

/*
Global Functions
----------------
*/

Widget create_element_point_field_viewer_widget(
	Widget *element_point_field_viewer_widget_address,Widget parent,
	struct Element_point_ranges_identifier *initial_element_point_identifier,
	int initial_element_point_number,struct Computed_field *initial_field);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Widget for displaying and editing computed field components at element points.
<initial_element_point_identifier> must be passed, but no initial element point
is referred to if its element is NULL. The element in the identifier, if any,
should be a local copy of a global element; up to the parent dialog to make
changes global.
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
	struct Element_point_ranges_identifier *element_point_identifier,
	int *element_point_number_address,struct Computed_field **field_address);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Returns the element_point/field being edited in the
<element_point_field_viewer_widget>.
==============================================================================*/

int element_point_field_viewer_widget_set_element_point_field(
	Widget element_point_field_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_FIELD_VIEWER_WIDGET_H) */
