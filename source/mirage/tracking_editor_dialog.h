/*******************************************************************************
FILE : tracking_editor_dialog.h

LAST MODIFIED : 15 January 2003

DESCRIPTION :
Structures and functions prototypes for the tracking editor dialog.
==============================================================================*/

#if !defined (TRACKING_EDITOR_DIALOG_H)
#define TRACKING_EDITOR_DIALOG_H

#include "general/indexed_multi_range.h"
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/

/*
Global types
------------
*/
enum Tracking_editor_control_mode
/*******************************************************************************
LAST MODIFIED : 27 March 1998

DESCRIPTION :
Modes of operation of the tracking editor.
Must be kept in sync with the names in tracking_editor_dialog.uil
==============================================================================*/
{
	TRACK_MODE=0,
	BACKTRACK_MODE,
	SUBSTITUTE_MODE,
	INTERPOLATE_MODE,
	MAKE_BAD_MODE,
	MAKE_GOOD_MODE
}; /* Tracking_editor_control_mode */

struct Tracking_editor_dialog;

/*
Global functions
----------------
*/

void tracking_editor_close_cb(Widget widget_id,XtPointer client_data,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 11 February 2002

DESCRIPTION :
Closes the dialog window, and any children dialogs that may be open.
???RC Why is this global?
==============================================================================*/

int open_tracking_editor_dialog(struct Tracking_editor_dialog **address,
	XtCallbackProc exit_button_callback,
	struct Colour *background_colour,
	struct Cmiss_region *root_region,
	struct Cmiss_region *data_root_region,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Graphics_window) *graphics_window_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *default_scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
==============================================================================*/

#endif /* !defined (TRACKING_EDITOR_DIALOG_H) */
