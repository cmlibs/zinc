/*******************************************************************************
FILE : volume_texture_editor_dialog.h

LAST MODIFIED : 18 May 1998

DESCRIPTION :
Structures and functions prototypes for the create finite elements dialog for
the volume texture editor.
???DB.  There may be further dialogs
==============================================================================*/
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
