/*******************************************************************************
FILE : colour_editor.h

LAST MODIFIED : 29 November 1997

DESCRIPTION :
Contains all the definitions necessary for support of a colour_editor control
dialog box to create colour_editors with.
==============================================================================*/
#if !defined (COLOUR_EDITOR_H)
#define COLOUR_EDITOR_H

#include <Xm/Xm.h>
#include "graphics/colour.h"
#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

enum Colour_editor_mode
/*******************************************************************************
LAST MODIFIED : 22 December 1994

DESCRIPTION :
Contains the different types of colour_editor input methods.
==============================================================================*/
{
	COLOUR_EDITOR_RGB,
	COLOUR_EDITOR_HSV,
	COLOUR_EDITOR_CMY
}; /* Colour_Editor_mode */

/*
Global Functions
---------------
*/
Widget create_colour_editor_widget(Widget parent,enum Colour_editor_mode mode,
	struct Colour *colour,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Create a colour_editor window that will allow the user to fully specify the
colour via rgb input (ie three sliders).  Initial data in <mode> format is
passed in <colour>.
==============================================================================*/

int colour_editor_get_callback(Widget colour_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns the update_callback for the colour editor widget.
==============================================================================*/

int colour_editor_set_callback(Widget colour_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes the update_callback for the colour editor widget.
==============================================================================*/

int colour_editor_get_colour(Widget colour_editor_widget,struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns into <colour> the value from the <colour_editor_widget>.
==============================================================================*/

int colour_editor_set_colour(Widget colour_editor_widget,struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Copies <colour> into the value in the <colour_editor_widget>.
==============================================================================*/
#endif /* !defined (COLOUR_EDITOR_H) */
