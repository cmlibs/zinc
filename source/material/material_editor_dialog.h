/*******************************************************************************
FILE : material_editor_dialog.h

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Header description for material_editor_dialog widget.
==============================================================================*/
#if !defined (MATERIAL_EDITOR_DIALOG_H)
#define MATERIAL_EDITOR_DIALOG_H

#include "general/callback.h"
#include "graphics/material.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

struct Material_editor_dialog;

/*
Global Functions
----------------
*/

int DESTROY(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_dialog_address> and sets
<*material_editor_dialog_address> to NULL.
==============================================================================*/

int bring_up_material_editor_dialog(
	struct Material_editor_dialog **material_editor_dialog_address,
	Widget parent, struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/

int material_editor_dialog_get_callback(
	struct Material_editor_dialog *material_editor_dialog,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Get the update <callback> information for the <material_editor_dialog>.
==============================================================================*/

int material_editor_dialog_set_callback(
	struct Material_editor_dialog *material_editor_dialog,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the update <callback> information for the <material_editor_dialog>.
==============================================================================*/

struct Graphical_material *material_editor_dialog_get_material(
	struct Material_editor_dialog *material_editor_dialog);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the material edited by the <material_editor_dialog>.
==============================================================================*/

int material_editor_dialog_set_material(
	struct Material_editor_dialog *material_editor_dialog,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <material> for the <material_editor_dialog>.
==============================================================================*/

#endif
