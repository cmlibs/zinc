/*******************************************************************************
FILE : material_editor.h

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Widgets for editing a graphical material.
==============================================================================*/
#if !defined (MATERIAL_EDITOR_H)
#define MATERIAL_EDITOR_H

#include "general/callback.h"
#include "graphics/material.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

struct Material_editor;

/*
Global Functions
----------------
*/

struct Material_editor *CREATE(Material_editor)(Widget parent,
	struct MANAGER(Texture) *texture_manager,
	struct Graphical_material *material, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Creates a Material_editor.
==============================================================================*/

int DESTROY(Material_editor)(struct Material_editor **material_editor_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_address> and sets
<*material_editor_address> to NULL.
==============================================================================*/

int material_editor_get_callback(
	struct Material_editor *material_editor,struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Get the update <callback> information for the <material_editor>.
==============================================================================*/

int material_editor_set_callback(
	struct Material_editor *material_editor,struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the update <callback> information for the <material_editor>.
==============================================================================*/

struct Graphical_material *material_editor_get_material(
	struct Material_editor *material_editor);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the material edited by the <material_editor>.
==============================================================================*/

int material_editor_set_material(
	struct Material_editor *material_editor, struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Sets the <material> to be edited by the <material_editor>.
==============================================================================*/

#endif
