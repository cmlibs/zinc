/*******************************************************************************
FILE : spectrum_editor_settings.h

LAST MODIFIED : 5 July 1999

DESCRIPTION :
Provides the widgets to manipulate point settings.
==============================================================================*/
#if !defined (SPECTRUM_EDITOR_SETTINGS_H)
#define SPECTRUM_EDITOR_SETTINGS_H

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
Widget create_spectrum_editor_settings_widget(Widget *spectrum_editor_settings_widget,
	Widget parent,struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Creates a spectrum_editor_settings widget.
==============================================================================*/

int spectrum_editor_settings_set_callback(Widget spectrum_editor_settings_widget,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Changes the callback function for the spectrum_editor_settings_widget, which will
be called when the chosen settings changes in any way.
==============================================================================*/

int spectrum_editor_settings_set_settings(Widget spectrum_editor_settings_widget,
	struct Spectrum_settings *new_settings);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Changes the currently chosen settings.
==============================================================================*/

struct Callback_data *spectrum_editor_settings_get_callback(
	Widget spectrum_editor_settings_widget);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns a pointer to the update_callback item of the
spectrum_editor_settings_widget.
==============================================================================*/

struct Spectrum_settings *spectrum_editor_settings_get_settings(
	Widget spectrum_editor_settings_widget);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the currently chosen settings.
==============================================================================*/
#endif /* !defined (SPECTRUM_EDITOR_SETTINGS_H) */
