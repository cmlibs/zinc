/*******************************************************************************
FILE : map_dialog.h

LAST MODIFIED : 3 May 2004

DESCRIPTION :
==============================================================================*/
#if !defined (MAP_DIALOG_H)
#define MAP_DIALOG_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "unemap/mapping.h"

/*
Global types
------------
*/
struct Map_dialog
/*******************************************************************************
LAST MODIFIED : 3 May 2004

DESCRIPTION :
The dialog box for configuring a map.
==============================================================================*/
{
#if defined (MOTIF)
	Widget activation,dialog,shell;
#endif /* defined (MOTIF) */
	struct Shell_list_item *shell_list_item;
#if defined (MOTIF)
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
#endif /* defined (MOTIF) */
	float range_maximum,range_minimum;
#if defined (MOTIF)
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
#endif /* defined (MOTIF) */
#if defined (MOTIF)
	struct
	{
		Widget option_menu;
		struct
		{
			Widget none;
			Widget bicubic;
			Widget direct;
		} option;
		Widget mesh_rows;
		Widget mesh_rows_text;
		Widget mesh_columns;
		Widget mesh_columns_text;
	} interpolation;
#endif /* defined (MOTIF) */
	int number_of_mesh_columns,number_of_mesh_rows;
#if defined (MOTIF)
	struct
	{
		Widget row_column;
		Widget type_option_menu;
		struct
		{
			Widget none;
			Widget constant_thickness;
			Widget variable_thickness;
		} type_option;
		Widget minimum;
		Widget minimum_text;
		Widget number;
		Widget number_text;
		Widget step;
		Widget step_text;
	} contours;
#endif /* defined (MOTIF) */
#if defined (MOTIF)
	struct
	{
		Widget label_menu;
		struct
		{
			Widget name;
			Widget value;
			Widget channel;
			Widget hide;
		} label;
		Widget marker_type_menu;
		struct
		{
			Widget circle;
			Widget plus;
			Widget square;
			Widget none;
		} marker_type;
		Widget marker_colour_toggle;
		Widget marker_size_text;
	} electrodes;
#endif /* defined (MOTIF) */
	int electrodes_marker_size;
#if defined (MOTIF)
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
	Widget regions_use_same_coordinates_toggle;
	Widget print_spectrum_toggle;
	struct
	{
		Widget row_column;
		Widget start_time_text;
		Widget end_time_text;
		Widget number_of_frames_text;
		Widget frame_number_text;
	} animation;
#endif /* defined (MOTIF) */
	float end_time,start_time;
	int frame_number,number_of_frames;
#if defined (MOTIF)
	Widget ok_button;
	Widget apply_button;
	Widget cancel_button;
#endif /* defined (MOTIF) */
	struct Map **map;
	struct Map_dialog **address;
	struct User_interface *user_interface;
}; /* struct Map_dialog */

/*
Global functions
----------------
*/
struct Map_dialog *create_Map_dialog(struct Map_dialog **map_dialog_address,
	struct Map **map,
#if defined (MOTIF)
	Widget activation,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
Allocates the memory for a map dialog.  Retrieves the necessary widgets and
initializes the appropriate fields.
==============================================================================*/

int open_map_dialog(struct Map_dialog *map_dialog);
/*******************************************************************************
LAST MODIFIED : 17 November 1992

DESCRIPTION :
Opens the <map_dialog>.
==============================================================================*/

#if defined (MOTIF)
void close_map_dialog(Widget widget,XtPointer map_dialog_structure,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
Closes the windows associated with the map_dialog box.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (MAP_DIALOG_H) */
