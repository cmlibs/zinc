/*******************************************************************************
FILE : graphical_element_editor.h

LAST MODIFIED : 16 February 1999

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
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
#if !defined (GRAPHICAL_ELEMENT_EDITOR_H)
#define GRAPHICAL_ELEMENT_EDITOR_H

#include "general/callback_motif.h"
#include "graphics/material.h"
#include "graphics/graphics_object.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/

Widget create_graphical_element_editor_widget(Widget *gelem_editor_widget,
	Widget parent,struct GT_element_group *gt_element_group,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates a graphical_element_editor widget.
==============================================================================*/

struct Callback_data *graphical_element_editor_get_callback(
	Widget graphical_element_editor_widget);
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Returns a pointer to the update_callback item of the
graphical_element_editor_widget.
==============================================================================*/

int graphical_element_editor_set_callback(
	Widget graphical_element_editor_widget,struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Changes the callback function for the graphical_element_editor_widget, which
will be called when the gt_element_group changes in any way.
==============================================================================*/

struct GT_element_group *graphical_element_editor_get_gt_element_group(
	Widget graphical_element_editor_widget);
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Returns the gt_element_group currently being edited.
==============================================================================*/

int graphical_element_editor_set_gt_element_group(
	Widget graphical_element_editor_widget,
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 22 July 1997

DESCRIPTION :
Sets the gt_element_group to be edited by the graphical_element_editor widget.
==============================================================================*/
#endif /* !defined (GRAPHICAL_ELEMENT_EDITOR_H) */
