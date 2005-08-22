/*******************************************************************************
FILE : volume_texture_editor_dialog.h

LAST MODIFIED : 18 May 1998

DESCRIPTION :
Structures and functions prototypes for the create finite elements dialog for
the volume texture editor.
???DB.  There may be further dialogs
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
#if !defined (VOLUME_TEXTURE_EDITOR_DIALOG_H)
#define VOLUME_TEXTURE_EDITOR_DIALOG_H

#include "graphics/volume_texture.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Create_finite_elements_dialog
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct Create_finite_elements_dialog **address;
	struct User_interface *user_interface;
	struct VT_volume_texture *volume_texture;
	Widget dialog,shell;
	Widget coordinate_field_toggle;
	Widget undeformed_field_toggle;
	Widget fibre_field_toggle;
	Widget x_interpolation_option_menu;
	struct
	{
		Widget linear_lagrange;
		Widget cubic_hermite;
	} x_interpolation_option;
	Widget y_interpolation_option_menu;
	struct
	{
		Widget linear_lagrange;
		Widget cubic_hermite;
	} y_interpolation_option;
	Widget z_interpolation_option_menu;
	struct
	{
		Widget linear_lagrange;
		Widget cubic_hermite;
	} z_interpolation_option;
	Widget starting_node_number_text_field;
	Widget starting_line_number_text_field;
	Widget starting_face_number_text_field;
	Widget starting_element_number_text_field;
	Widget file_group_name_text_field;
	Widget x_output_elements_text_field;
	Widget y_output_elements_text_field;
	Widget z_output_elements_text_field;
	Widget ok_button;
	Widget cancel_button;
}; /* struct Create_finite_elements_dialog */

/*
Global functions
----------------
*/
int open_create_finite_elements_dialog(
	struct Create_finite_elements_dialog **address,
	struct VT_volume_texture *volume_texture,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 25 November 1996

DESCRIPTION :
==============================================================================*/
#endif /* !defined (VOLUME_TEXTURE_EDITOR_DIALOG_H) */
