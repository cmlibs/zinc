/*******************************************************************************
FILE : input_module_dialog.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
This is the control center for the input_module.  Things like setting
resolution, refresh rate and origins may be done from this widget.
==============================================================================*/
#if !defined (INPUT_MODULE_DIALOG_H)
#define INPUT_MODULE_DIALOG_H

#include "io_devices/input_module.h"

#ifdef EXT_INPUT
/*
Global functions
----------------
*/
int bring_up_input_module_dialog(Widget *input_module_dialog_address,
	Widget parent,struct Graphical_material *material,
	struct MANAGER(Graphical_material) *material_manager,struct Scene *scene,
	struct MANAGER(Scene) *scene_manager, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
If there is a input_module dialog in existence, then bring it to the front, else
create a new one.  Assumes we will only ever want one input_module controller at
a time.  This implementation may be changed later.
==============================================================================*/
#endif /* EXT_INPUT */

#endif /* INPUT_MODULE_DIALOG_H */
