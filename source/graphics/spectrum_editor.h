/*******************************************************************************
FILE : spectrum_editor.h

LAST MODIFIED : 4 September 2000

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
==============================================================================*/
#if !defined (SPECTRUM_EDITOR_H)
#define SPECTRUM_EDITOR_H

#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/

Widget create_spectrum_editor_widget(Widget *gelem_editor_widget,
	Widget parent,struct Spectrum *spectrum,
	struct User_interface *user_interface,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Texture) *texture_manager);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/

int spectrum_editor_set_callback(
	Widget spectrum_editor_widget,struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the callback function for the spectrum_editor_widget, which
will be called when the Spectrum changes in any way.
==============================================================================*/

int spectrum_editor_set_Spectrum(
	Widget spectrum_editor_widget,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets the Spectrum to be edited by the spectrum_editor widget.
==============================================================================*/

struct Callback_data *spectrum_editor_get_callback(
	Widget spectrum_editor_widget);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns a pointer to the update_callback item of the
spectrum_editor_widget.
==============================================================================*/

struct Spectrum *spectrum_editor_get_Spectrum(
	Widget spectrum_editor_widget);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns the Spectrum currently being edited.
==============================================================================*/

int spectrum_editor_refresh(Widget spectrum_editor_widget);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Clears all the settings_changed flags globally (later) and in the list of
settings.
==============================================================================*/

int spectrum_editor_update_changes(Widget spectrum_editor_widget);
/*******************************************************************************
LAST MODIFIED : 23 July 1998
DESCRIPTION :
This function is called to update the editor when other
things (such as the autorange button) have changed the
edit spectrum.
==============================================================================*/
#endif /* !defined (SPECTRUM_EDITOR_H) */
