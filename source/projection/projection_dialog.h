/*******************************************************************************
FILE : projection_dialog.h

LAST MODIFIED : 20 May 1997

DESCRIPTION :
???DB.  Started as map_dialog.h in emap
==============================================================================*/
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
