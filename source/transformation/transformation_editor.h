/*******************************************************************************
FILE : transformation_editor.h

LAST MODIFIED : 3 December 2001

DESCRIPTION :
This module creates a widget that will allow the user to position a model in
three dimensional space, relative to some 'parent' coordinate system.
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
