/*******************************************************************************
FILE : spectrum_editor_dialog.h

LAST MODIFIED : 10 March 1998

DESCRIPTION :
Header description for spectrum_editor_dialog widget.
==============================================================================*/
#if !defined (SPECTRUM_EDITOR_DIALOG_H)
#define SPECTRUM_EDITOR_DIALOG_H

#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Functions
----------------
*/
int spectrum_editor_dialog_get_callback(Widget spectrum_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns the update_callback for the spectrum editor_dialog widget.
==============================================================================*/

int spectrum_editor_dialog_set_callback(Widget spectrum_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the update_callback for the spectrum editor_dialog widget.
==============================================================================*/

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	Widget spectrum_editor_dialog_widget);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If <spectrum_editor_dialog_widget> is not NULL, then get the data item from
<spectrum_editor_dialog widget>.  Otherwise, get the data item from
<spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_dialog_set_spectrum(Widget spectrum_editor_dialog_widget,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If <spectrum_editor_dialog_widget> is not NULL, then change the data item on
<spectrum_editor_dialog widget>.  Otherwise, change the data item on
<spectrum_editor_dialog>.
==============================================================================*/

int bring_up_spectrum_editor_dialog(Widget *spectrum_editor_dialog_address,
	Widget parent,struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum, struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
#endif
