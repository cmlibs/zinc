/*******************************************************************************
FILE : graphical_element_editor.h

LAST MODIFIED : 16 February 1999

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
==============================================================================*/
#if !defined (GRAPHICAL_ELEMENT_EDITOR_H)
#define GRAPHICAL_ELEMENT_EDITOR_H

#include "general/callback.h"
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
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 February 1999

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
