/*******************************************************************************
FILE : settings_editor.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Provides the widgets to manipulate point settings.
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
#if !defined (SETTINGS_EDITOR_H)
#define SETTINGS_EDITOR_H

#include "general/callback.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
Widget create_settings_editor_widget(Widget *settings_editor_widget,
	Widget parent,struct GT_element_settings *settings,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *glyph_list,struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates a settings_editor widget.
==============================================================================*/

struct Callback_data *settings_editor_get_callback(
	Widget settings_editor_widget);
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Returns a pointer to the update_callback item of the
settings_editor_widget.
==============================================================================*/

int settings_editor_set_callback(Widget settings_editor_widget,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Changes the callback function for the settings_editor widget, which will
be called when the chosen settings changes in any way.
==============================================================================*/

struct GT_element_settings *settings_editor_get_settings(
	Widget settings_editor_widget);
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Returns the current settings.
==============================================================================*/

int settings_editor_set_settings(Widget settings_editor_widget,
	struct GT_element_settings *new_settings);
/*******************************************************************************
LAST MODIFIED : 4 November 1997

DESCRIPTION :
Changes the current settings.
==============================================================================*/

#endif /* !defined (SETTINGS_EDITOR_H) */
