/*******************************************************************************
FILE : tracking_editor_dialog.h

LAST MODIFIED : 22 March 2000

DESCRIPTION :
Structures and functions prototypes for the tracking editor dialog.
==============================================================================*/

#if !defined (TRACKING_EDITOR_DIALOG_H)
#define TRACKING_EDITOR_DIALOG_H

#include "general/managed_group.h"
#include "general/manager.h"
#include "mirage/tracking_editor_data.h"
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
LAST MODIFIED : 27 March 1998

DESCRIPTION :
Closes the dialog window, and any children dialogs that may be open.
???RC Why is this global?
==============================================================================*/

int open_tracking_editor_dialog(struct Tracking_editor_dialog **address,
	XtCallbackProc exit_button_callback,
	struct Colour *background_colour,
	struct MANAGER(FE_basis) *basis_manager,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Graphics_window) *graphics_window_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *default_scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (TRACKING_EDITOR_DIALOG_H) */
