/*******************************************************************************
FILE : acquisition_window.h

LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
#if !defined (ACQUISITION_WINDOW_H)
#define ACQUISITION_WINDOW_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/
struct Acquisition_window
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
The acquisition window object.
==============================================================================*/
{
#if defined (MOTIF)
	Widget activation,window;
	Widget acquire_button;
#endif /* defined (MOTIF) */
	struct File_open_data *acquire_file_open_data;
#if defined (MOTIF)
	Widget monitoring_toggle;
#endif /* defined (MOTIF) */
	/*???DB.  Temporary ? */
	struct File_open_data *monitoring_file_open_data;
#if defined (MOTIF)
	Widget experiment_toggle;
#endif /* defined (MOTIF) */
	struct File_open_data *experiment_file_open_data;
#if defined (MOTIF)
	Widget calibrate_button;
	Widget page_button;
	Widget close_button;
	Widget drawing_area;
#endif /* defined (MOTIF) */
	struct Drawing_2d *drawing;
	struct Rig **rig_address;
	int acquire_time,end_time,start_time;
	int acquire_point,end_point,marker_bottom,marker_centre,marker_top,
		start_point;
	struct Mapping_window **mapping_window_address;
	struct Acquisition_window **address;
	struct User_interface *user_interface;
	/* user settings */
	char *calibration_directory,*signal_file_extension_write;
	int pointer_sensitivity,sampling_frequency_hz;
#if defined (MOTIF)
	Pixel acquisition_interval_colour,background_drawing_colour;
#endif /* defined (MOTIF) */
#if defined (MOTIF)
	/* graphics contexts */
	struct
	{
		GC acquisition_interval_colour,background_drawing_colour,copy;
		/*???DB.  When using an RS/6000 model with the Color Graphics Display
			Adapter (#2770) the GCs stop writing text to the pixel map after they've
			been used for drawing lines to the window.  So I have duplicates */
		GC acquisition_interval_colour_tex;
	} graphics_context;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HANDLE device_driver;
	HWND window;
#endif /* defined (WINDOWS) */
}; /* struct Acquisition_window */

/*
Global functions
----------------
*/
struct Acquisition_window *create_Acquisition_window(
	struct Acquisition_window **address,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
	struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	struct Mapping_window **mapping_window_address,int pointer_sensitivity,
	char *signal_file_extension_write,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
This function allocates the memory for an acquisition window and sets the fields
to the specified values (<address>, <activation>, <rig_address>).  It then
retrieves an acquisition window widget with the specified parent/<shell> and
assigns the widget ids to the appropriate fields of the structure.  If
successful it returns a pointer to the created acquisition window and, if
<address> is not NULL, makes <*address> point to the created acquisition window.
If unsuccessful, NULL is returned.
==============================================================================*/

#if defined (MOTIF)
Widget create_acquisition_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for an acquisition window.  If <address> is not
NULL, <*address> is set to the id of the created shell and on destruction
<*address> will be set to NULL.  The id of the created widget is returned.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (ACQUISITION_WINDOW_H) */
