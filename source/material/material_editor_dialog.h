/*******************************************************************************
FILE : material_editor_dialog.h

LAST MODIFIED : 1 December 1997

DESCRIPTION :
Header description for material_editor_dialog widget.
==============================================================================*/
#if !defined (MATERIAL_EDITOR_DIALOG_H)
#define MATERIAL_EDITOR_DIALOG_H

#include "general/callback.h"
#include "graphics/material.h"
#include "user_interface/user_interface.h"

/*
Global Functions
----------------
*/
int material_editor_dialog_get_callback(Widget material_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the update_callback for the material editor_dialog widget.
==============================================================================*/

int material_editor_dialog_set_callback(Widget material_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the update_callback for the material editor_dialog widget.
==============================================================================*/

struct Graphical_material *material_editor_dialog_get_material(
	Widget material_editor_dialog_widget);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If <material_editor_dialog_widget> is not NULL, then get the data item from
<material_editor_dialog widget>.  Otherwise, get the data item from
<material_editor_dialog>.
==============================================================================*/

int material_editor_dialog_set_material(Widget material_editor_dialog_widget,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If <material_editor_dialog_widget> is not NULL, then change the data item on
<material_editor_dialog widget>.  Otherwise, change the data item on
<material_editor_dialog>.
==============================================================================*/

int bring_up_material_editor_dialog(Widget *material_editor_dialog_address,
	Widget parent,struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
#endif
