/*******************************************************************************
FILE : analysis.h

LAST MODIFIED : 19 November 2000

DESCRIPTION :
==============================================================================*/
#if !defined (ANALYSIS_H)
#define ANALYSIS_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "finite_element/finite_element.h"
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

enum Event_detection_objective
/*******************************************************************************
LAST MODIFIED : 25 April 2000

DESCRIPTION :
The objective used when detecting an event.
==============================================================================*/
{
	ABSOLUTE_SLOPE,
	NEGATIVE_SLOPE,
	POSITIVE_SLOPE,
	VALUE_OBJECTIVE
}; /* enum Event_detection_objective */

enum Event_detection_algorithm
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
The algorithm used for detecting an event from a signal.
EDA_INTERVAL = divide the search interval into a user specified number of 
	sub-intervals and select the maximum objective within each sub-interval
EDA_LEVEL = find the first time in the search interval at which the absolute
	value of the signal exceeds the user specified level
EDA_THRESHOLD = select all times within the search interval whose objectives
	exceed a user specified percentage of the maximum objective for the search
	interval, subject to user specifed minimum event separation.
NB.  New algorithms need to be added to the end because the detection algorithm
	is written to the signals file (when saving analysis)
==============================================================================*/
{
	EDA_INTERVAL,
	EDA_THRESHOLD,
	EDA_LEVEL
}; /* enum Event_detection_algorithm */

enum Datum_type
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The method for determining the datum from which the event times are calculated.
FIXED = specified by the user
AUTOMATIC = calculated by the system as the first event
==============================================================================*/
{
	AUTOMATIC_DATUM,
	FIXED_DATUM
}; /* enum Datum_type */

enum Edit_order
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The order in which the events are traversed while editing.
==============================================================================*/
{
	DEVICE_ORDER,
	BEAT_ORDER
}; /* enum Edit_order */

enum Signal_order
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The order in which the signals are drawn and edited.
==============================================================================*/
{
	EVENT_ORDER,
	CHANNEL_ORDER
}; /* enum Signal_order */

struct Signal_drawing_information
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Information needed for drawing a signal.  Windowing system dependent
==============================================================================*/
{
	int number_of_signal_overlay_colours;
	Pixel accepted_colour,axis_colour,background_drawing_colour,datum_colour,
		device_name_colour,highlighted_colour,interval_box_colour,
		potential_time_colour,rejected_colour,scaling_signal_colour,
		signal_accepted_colour,*signal_overlay_colours,signal_rejected_colour,
		signal_undecided_colour,undecided_colour,unhighlighted_colour;
	struct
	{
		GC accepted_colour,axis_colour,background_drawing_colour,copy,datum_colour,
			device_name_colour,highlighted_box_colour,highlighted_colour,
			interval_box_colour,potential_time_colour,rejected_colour,
			scaling_signal_colour,signal_accepted_colour,signal_overlay_colour,
			signal_rejected_colour,signal_undecided_colour,spectrum,undecided_colour,
			unhighlighted_colour;
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

int calculate_device_objective(struct Device *device,
	enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,float *objective_values,
	int number_of_objective_values,int objective_values_step,int average_width);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the <device>.
Storing the values in the array (<objective_values> every
<objective_values_step>) provided.
==============================================================================*/

int calculate_device_event_markers(struct Device *device,
	int start_search,int end_search,enum Event_detection_algorithm detection,
	float *objective_values,int number_of_objective_values,
	int objective_values_step,int number_of_events,int threshold_percentage,
	int minimum_separation_milliseconds,float level);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Calculate the positions of the event markers for a signal/<device>/<device_node> 
based upon the the start and end times, the number of events, the <detection> 
algorithm and the <objective_values>.
==============================================================================*/

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

DESCRIPTION :
==============================================================================*/

int draw_potential_time_marker(int time,struct Signal_buffer *buffer,
	struct Channel *channel,int signal_index,enum Signal_detail detail,
	int first_data,int last_data,int axes_left,int axes_top,int axes_width,
	int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
==============================================================================*/

int draw_event_marker(struct Event *event,int current_event_number,
	int datum,int *times,float frequency,enum Signal_detail detail,int first_data,
	int last_data,int signal_min,int signal_max,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
==============================================================================*/

int draw_device_markers(struct Device *device,int first_data,int last_data,
	int datum,char draw_datum,int potential_time,char draw_potential_time,
	enum Signal_detail detail,int current_event_number,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
==============================================================================*/

struct Signal_drawing_information *create_Signal_drawing_information(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
==============================================================================*/

int destroy_Signal_drawing_information(
	struct Signal_drawing_information **signal_drawing_information);
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
==============================================================================*/

int analysis_write_signal_file(char *file_name,struct Rig *rig,int datum,
	int potential_time,int start_search_interval,int end_search_interval,
	char calculate_events,enum Event_detection_algorithm detection,
	int event_number,int number_of_events,int minimum_separation,int threshold,
	enum Datum_type datum_type,enum Edit_order edit_order,
	enum Signal_order signal_order,float level,int average_width);
/*******************************************************************************
LAST MODIFIED : 19 November 2000

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
#endif /* !defined (ANALYSIS_H) */
