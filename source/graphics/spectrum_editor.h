/*******************************************************************************
FILE : spectrum_editor.h

LAST MODIFIED : 19 November 1998

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

struct GT_object *create_Spectrum_colour_bar(char *name,
	struct Spectrum *spectrum,Triple bar_centre,Triple bar_axis,Triple side_axis,
	float bar_length,float bar_radius,float extend_length,int tick_divisions,
	float tick_direction,char *number_format,char *number_string,
	struct Graphical_material *bar_material,
	struct Graphical_material *tick_label_material);
/*******************************************************************************
LAST MODIFIED : 19 November 1998

DESCRIPTION :
Creates a coloured bar with annotation for displaying the scale of <spectrum>.
First makes a graphics object named <name> consisting of a cylinder of size
<bar_length>, <bar_radius> centred at <bar_centre> and aligned with the
<bar_axis>, combining the <bar_material> with the <spectrum>. The bar is
coloured by the current range of the spectrum. An extra <extend_length> is added
on to each end of the bar and given the colour the spectrum returns outside its
range at that end.
The <side_axis> must not be collinear with <bar_axis> and is used to calculate
points around the bar and the direction of the <tick_divisions>+1 ticks at which
spectrum values are written.
Attached to the bar graphics_object are two graphics objects using the
<tick_label_material>, one containing the ticks, the other the labels. The
labels are written using the <number_format>, printed into the <number_string>
which should be allocated large enough to hold it.

The side_axis is made to be orthogonal to the bar_axis, and both are made unit
length by this function.

???RC Function probably belongs elsewhere. We have module
finite_element_to_graphics_object, but how about a more generic module for
graphics_objects that don't come from finite_elements?
==============================================================================*/

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
