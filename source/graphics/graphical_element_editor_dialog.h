/*******************************************************************************
FILE : graphical_element_editor_dialog.h

LAST MODIFIED : 31 October 2000

DESCRIPTION :
Routines for creating an element group editor dialog shell and standard buttons.
Form set aside for the actual element group editor.
==============================================================================*/
#if !defined (GRAPHICAL_ELEMENT_EDITOR_DIALOG_H)
#define GRAPHICAL_ELEMENT_EDITOR_DIALOG_H

#include "general/callback.h"
#include "graphics/material.h"
#include "user_interface/user_interface.h"
#include "finite_element/finite_element.h"

/*
Global Types
------------
*/

struct Graphical_element_editor_dialog;
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global Functions
----------------
*/

struct Graphical_element_editor_dialog *CREATE(Graphical_element_editor_dialog)(
	struct Graphical_element_editor_dialog
	  **graphical_element_editor_dialog_address,
	Widget parent,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Note on successful return the dialog is already put at
<*graphical_element_editor_dialog_address>.
==============================================================================*/

int DESTROY(Graphical_element_editor_dialog)(
	struct Graphical_element_editor_dialog
	  **graphical_element_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
==============================================================================*/

int Graphical_element_editor_dialog_bring_to_front(
	struct Graphical_element_editor_dialog *gelem_editor_dialog);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
De-iconifies and brings the graphical element editor to the front.
==============================================================================*/

int Graphical_element_editor_dialog_get_element_group_and_scene(
	struct Graphical_element_editor_dialog *gelem_editor_dialog,
	struct GROUP(FE_element) **element_group,struct Scene **scene);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns which element_group and scene are looked at with the
graphical_element_editor_dialog widget.
==============================================================================*/

int Graphical_element_editor_dialog_set_element_group_and_scene(
	struct Graphical_element_editor_dialog *gelem_editor_dialog,
	struct GROUP(FE_element) *element_group,struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Sets which element_group and scene are looked at with the
graphical_element_editor_dialog widget.
==============================================================================*/

#endif /* !defined (GRAPHICAL_ELEMENT_EDITOR_DIALOG_H) */
