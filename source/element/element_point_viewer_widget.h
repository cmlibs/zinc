/*******************************************************************************
FILE : element_point_viewer_widget.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Widget for editing field values stored at an element point with multiple text
fields, visible one field at a time. Any computed field may be viewed. Most can
be modified too.
Note the element passed to this widget should be a non-managed local copy.
==============================================================================*/
#if !defined (ELEMENT_POINT_VIEWER_WIDGET_H)
#define ELEMENT_POINT_VIEWER_WIDGET_H

#include "general/callback.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_value_index_ranges.h"
#include "finite_element/finite_element.h"
#include "graphics/element_point_ranges.h"
#include "time/time.h"

/*
Global Functions
----------------
*/

Widget create_element_point_viewer_widget(
	Widget *element_point_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct LIST(Field_value_index_ranges) *modified_field_components,
	struct Element_point_ranges_identifier *initial_element_point_identifier,
	int initial_element_point_number, struct Time_object *time_object,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a widget for displaying and editing the contents of the element point
at <initial_element_point_identifier> <initial_element_point_number>.
<initial_element_point_identifier> must be passed, but no initial element point
is referred to if its element is NULL. The element in the identifier, if any,
should be a local copy of a global element; up to the parent dialog to make
changes global.
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
	Widget element_point_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int *element_point_number_address);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Returns the element_point being edited in the <element_point_viewer_widget>.
==============================================================================*/

int element_point_viewer_widget_set_element_point(
	Widget element_point_viewer_widget,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_VIEWER_WIDGET_H) */
