/*******************************************************************************
FILE : spectrum_editor_dialog.h

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Header description for spectrum_editor_dialog widget.
==============================================================================*/
#if !defined (SPECTRUM_EDITOR_DIALOG_H)
#define SPECTRUM_EDITOR_DIALOG_H

#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

struct Spectrum_editor_dialog;

/*
Global Functions
----------------
*/

int bring_up_spectrum_editor_dialog(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	Widget parent, struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum, struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then de-iconify it and
bring it to the front, otherwise create a new one.
==============================================================================*/

int DESTROY(Spectrum_editor_dialog)(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroy the <*spectrum_editor_dialog_address> and sets
<*spectrum_editor_dialog_address> to NULL.
==============================================================================*/

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_dialog_set_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/

#endif
