/*******************************************************************************
FILE : analysis_drawing.h

LAST MODIFIED : 7 June 2002

DESCRIPTION :
==============================================================================*/
#if !defined (ANALYSIS_DRAWING_H)
#define ANALYSIS_DRAWING_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "finite_element/finite_element.h"
#include "unemap/analysis.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum Signal_detail
/*******************************************************************************
LAST MODIFIED : 29 April 1993

DESCRIPTION :
The level of detail for drawing a signal.
==============================================================================*/
{
	SIGNAL_AREA_DETAIL,
	INTERVAL_AREA_DETAIL,
	EDIT_AREA_DETAIL,
	ENLARGE_AREA_DETAIL,
	PRINTER_DETAIL
}; /* enum Signal_detail */

enum Signal_layout
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The way in which the signals are layed out in the analysis window.
SEPARATE = signals are drawn in separate areas
OVERLAP = signals overlap vertically
==============================================================================*/
{
	SEPARATE_LAYOUT,
	OVERLAP_LAYOUT
}; /* enum Signal_layout */

struct Signal_drawing_information
/*******************************************************************************
LAST MODIFIED : 12 April 2001

DESCRIPTION :
Information needed for drawing a signal.  Windowing system dependent
==============================================================================*/
{
	int number_of_signal_overlay_colours;
	Pixel accepted_colour,axis_colour,background_drawing_colour,
		cardiac_interval_colour,eimaging_event_colour,datum_colour,
		device_name_colour,highlighted_colour,interval_box_colour,
		potential_time_colour,pwave_colour,qrswave_colour,rejected_colour,
		scaling_signal_colour,signal_accepted_colour,*signal_overlay_colours,
		signal_rejected_colour,signal_undecided_colour,twave_colour,
		undecided_colour,unhighlighted_colour;
	struct
	{
		GC accepted_colour,axis_colour,background_drawing_colour,
			cardiac_interval_colour,eimaging_event_colour,copy,datum_colour,
			device_name_colour,highlighted_box_colour,highlighted_colour,
			interval_box_colour,potential_time_colour,pwave_colour,qrswave_colour,
			rejected_colour,scaling_signal_colour,signal_accepted_colour,
			signal_overlay_colour,signal_rejected_colour,signal_undecided_colour,
			spectrum,undecided_colour,twave_colour,unhighlighted_colour;
		/*???DB.  When using an RS/6000 model with the Color Graphics Display
			Adapter (#2770), GCs stop writing text to the pixel map after they've
			been used for drawing lines to the window.  So I duplicate them */
		/*???DB.  How should background_drawing_colour,spectrum,copy be shared ? */
		GC accepted_colour_text,potential_time_colour_text,rejected_colour_text,
			undecided_colour_text;
	} graphics_context;
	struct User_interface *user_interface;
	XFontStruct *font;
}; /* struct Signal_drawing_information */

/*
Global functions
----------------
*/
int draw_signal(struct FE_node *device_node,
	struct Signal_drawing_package *signal_drawing_package,struct Device *device,
	enum Signal_detail detail,int number_of_data_intervals,
	int current_data_interval,int *first_data,int *last_data,int x_pos,int y_pos,
	int width,int height,Pixmap pixel_map,int *axes_left,int *axes_top,
	int *axes_width,int *axes_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 January 2000

DESCRIPTION :
Draws the <device> signal in the <pixel_map> at the specified position
(<x_pos>, <y_pos>), size (<width>, <height>) and <detail>.
NB.  0<=current_data_interval<number_of_data_intervals
???missing data ? times ?
???DB.  Needs more checking and more on return_code
???DB.  Change first_data and last_data to times
==============================================================================*/

#if defined (OLD_CODE)
int draw_signal(struct Device *device,enum Signal_detail detail,int first_data,
	int last_data,int x_pos,int y_pos,int width,int height,Pixmap pixel_map,
	int *axes_left,int *axes_top,int *axes_width,int *axes_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 December 1996

DESCRIPTION :
Draws the <device> signal in the <pixel_map> at the specified position
(<x_pos>, <y_pos>), size (<width>, <height>) and <detail>.
???missing data ? times ?
==============================================================================*/
#endif /* defined (OLD_CODE) */

int draw_datum_marker(int datum,enum Signal_detail detail,int first_data,
	int last_data,int axes_left,int axes_top,int axes_width,int axes_height,
	Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION : draws the datum marker
==============================================================================*/

int draw_potential_time_marker(int time,struct Signal_buffer *buffer,
	struct Channel *channel,int signal_index,enum Signal_detail detail,
	int first_data,int last_data,int axes_left,int axes_top,int axes_width,
	int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :draws the potential time_marker
==============================================================================*/

int draw_event_marker(struct Event *event,int current_event_number,
	int datum,int *times,float frequency,enum Signal_detail detail,int first_data,
	int last_data,int signal_min,int signal_max,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION : draws the event_marker
==============================================================================*/

int draw_device_markers(struct Device *device,int first_data,int last_data,
	int datum,char draw_datum,int potential_time,char draw_potential_time,
	enum Signal_detail detail,int current_event_number,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION : draws the device_markers
==============================================================================*/

struct Signal_drawing_information *create_Signal_drawing_information(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION : creates the Signal_drawing_information
==============================================================================*/

int destroy_Signal_drawing_information(
	struct Signal_drawing_information **signal_drawing_information);
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION : destroys  the Signal_drawing_information
==============================================================================*/
#endif /* !defined (ANALYSIS_DRAWING_H) */
