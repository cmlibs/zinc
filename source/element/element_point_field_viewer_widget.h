/*******************************************************************************
FILE : element_point_field_viewer_widget.h

LAST MODIFIED : 15 October 2001

DESCRIPTION :
Widget for displaying and editing component values of computed fields defined
over a element_point. One field at a time can be viewed/edited.
Note the element_point passed to this widget should be a non-managed local copy.
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
#if !defined (ELEMENT_POINT_FIELD_VIEWER_WIDGET_H)
#define ELEMENT_POINT_FIELD_VIEWER_WIDGET_H

#include "general/callback_motif.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_value_index_ranges.h"
#include "finite_element/finite_element.h"
#include "graphics/element_point_ranges.h"
#include "time/time.h"

/*
Global Functions
----------------
*/

Widget create_element_point_field_viewer_widget(
	Widget *element_point_field_viewer_widget_address,Widget parent,
	struct LIST(Field_value_index_ranges) *modified_field_components,
	struct Element_point_ranges_identifier *initial_element_point_identifier,
	int initial_element_point_number,struct Computed_field *initial_field,
	struct Time_object *time_object);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

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
