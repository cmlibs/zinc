/*******************************************************************************
FILE : transformation_editor.h

LAST MODIFIED : 3 December 2001

DESCRIPTION :
This module creates a widget that will allow the user to position a model in
three dimensional space, relative to some 'parent' coordinate system.
==============================================================================*/
#if !defined (TRANSFORMATION_EDITOR_H)
#define TRANSFORMATION_EDITOR_H

#include "general/callback.h"
#include "graphics/graphics_library.h"

/*
Global Functions
----------------
*/

Widget create_transformation_editor_widget(
	Widget *transformation_editor_widget_address, Widget parent,
	gtMatrix *initial_transformation_matrix);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a widget for editing a gtMatrix.
Note: currently restricted to editing 4x4 matrices that represent a rotation
and a translation only.
==============================================================================*/

int transformation_editor_get_callback(Widget transformation_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Fills the <callback> with the current update callback procedure and data for
the <transformation_editor>.
==============================================================================*/

int transformation_editor_set_callback(Widget transformation_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Changes the callback function for the transformation_editor_widget, which
will be called when the transformation changes in any way.
==============================================================================*/

int transformation_editor_get_transformation(
	Widget transformation_editor_widget, gtMatrix *transformation_matrix);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Puts the transformation being edited in the <transformation_editor> out to
<transformation_matrix>.
==============================================================================*/

int transformation_editor_set_transformation(
	Widget transformation_editor_widget, gtMatrix *transformation_matrix);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Sets the <transformation_editor> to editor the transformation encoded in
4x4 <transformation_matrix>.
==============================================================================*/

#endif
