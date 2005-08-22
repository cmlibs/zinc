/*******************************************************************************
FILE : projection_dialog.h

LAST MODIFIED : 20 May 1997

DESCRIPTION :
???DB.  Started as map_dialog.h in emap
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
#if !defined (PROJECTION_DIALOG_H)
#define PROJECTION_DIALOG_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "projection/projection.h"
#include "projection/projection_window.h"

/*
Global types
------------
*/
struct Projection_dialog
/*******************************************************************************
LAST MODIFIED : 20 May 1997

DESCRIPTION :
The dialog box for configuring a projection.
==============================================================================*/
{
	Widget creator,dialog,shell;
	Widget type_option_menu;
	struct
	{
		Widget hammer;
		Widget polar;
		Widget cylindrical;
	} type_option;
	Widget xi_3_value;
	float xi_3;
	struct
	{
		Widget type_option_menu;
		struct
		{
			Widget automatic;
			Widget fixed;
		} type_option;
		Widget minimum_value;
		Widget maximum_value;
	} range;
	float range_maximum,range_minimum;
	struct
	{
		Widget type_option_menu;
		struct
		{
			Widget none;
			Widget blue_red;
			Widget red_blue;
			Widget log_blue_red;
			Widget log_red_blue;
			Widget blue_white_red;
		} type_option;
	} spectrum;
	struct
	{
		Widget type_option_menu;
		struct
		{
			Widget none;
			Widget constant_thickness;
			Widget variable_thickness;
		} type_option;
		Widget down_arrow;
		Widget number;
		Widget up_arrow;
	} contours;
	int number_of_contours;
	Widget elements_option_menu;
	struct
	{
		Widget name_and_boundary;
		Widget boundary_only;
		Widget hide;
	} elements_option;
	Widget nodes_option_menu;
	struct
	{
		Widget name;
		Widget value;
		Widget hide;
	} nodes_option;
	Widget fibres_option_menu;
	struct
	{
		Widget hide;
		Widget fine;
		Widget medium;
		Widget coarse;
	} fibres_option;
	Widget show_landmarks_toggle;
	Widget show_extrema_toggle;
	Widget maintain_aspect_ratio_toggle;
	Widget print_spectrum_toggle;
	Widget ok_button;
	Widget apply_button;
	Widget cancel_button;
	struct Projection **projection;
	struct Projection_dialog **address;
};

/*
Global functions
----------------
*/
struct Projection_dialog *create_Projection_dialog(
	struct Projection_dialog **projection_dialog_address,
	struct Projection_window *projection_window,Widget creator,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Allocates the memory for a projection dialog.  Retrieves the necessary widgets
and initializes the appropriate fields.
==============================================================================*/

int open_projection_dialog(struct Projection_dialog *projection_dialog);
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Opens the <projection_dialog>.
==============================================================================*/

void close_projection_dialog(Widget widget,
	XtPointer projection_dialog_structure,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Closes the windows associated with the projection_dialog box.
==============================================================================*/
#endif
