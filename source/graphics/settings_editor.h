/*******************************************************************************
FILE : settings_editor.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Provides the widgets to manipulate point settings.
==============================================================================*/
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
