/*******************************************************************************
FILE : material_editor.h

LAST MODIFIED : 1 December 1997

DESCRIPTION :
This module creates a free material_editor input device, using two dof3, two
control and one input widget.  The position is given relative to some coordinate
system, and the returned value is a global one.
==============================================================================*/
#if !defined (MATERIAL_EDITOR_H)
#define MATERIAL_EDITOR_H

#include "general/callback.h"
#include "graphics/material.h"
#include "user_interface/user_interface.h"

/*
Global Functions
----------------
*/
Widget create_material_editor_widget(Widget *material_editor_widget,
	Widget parent,struct MANAGER(Texture) *texture_manager,
	struct Graphical_material *material,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Creates a material_editor widget.
???RC what is the texture_manager needed for?
==============================================================================*/

int material_editor_get_callback(Widget material_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the update_callback for the material editor widget.
==============================================================================*/

int material_editor_set_callback(Widget material_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the update_callback for the material editor widget.
==============================================================================*/

struct Graphical_material *material_editor_get_material(
	Widget material_editor_widget);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the address of the material being edited in the material_editor widget.
Do not modify or DEALLOCATE the returned material; copy it to another material.
==============================================================================*/

int material_editor_set_material(Widget material_editor_widget,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the material in the material_editor widget.
==============================================================================*/
#endif
