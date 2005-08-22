/*******************************************************************************
FILE : colour_editor.h

LAST MODIFIED : 29 November 1997

DESCRIPTION :
Contains all the definitions necessary for support of a colour_editor control
dialog box to create colour_editors with.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (COLOUR_EDITOR_H)
#define COLOUR_EDITOR_H

#include <Xm/Xm.h>
#include "graphics/colour.h"
#include "general/callback_motif.h"
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
