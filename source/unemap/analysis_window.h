/*******************************************************************************
FILE : analysis_window.h

LAST MODIFIED : 13 October 1999

DESCRIPTION :
==============================================================================*/
#if !defined (ANALYSIS_WINDOW_H)
#define ANALYSIS_WINDOW_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

#include "general/managed_group.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

#include "unemap/analysis.h"
#include "unemap/drawing_2d.h"
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/
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

struct Analysis_map_menu
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
The menu associated with the map button.
==============================================================================*/
{
	Widget potential_button;
	Widget integral_button;
	Widget gradient_button;
	Widget single_activation_button;
	Widget multiple_activation_button;
}; /* struct Analysis_map_menu */

struct Analysis_file_menu
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
The menu associated with the file button.
==============================================================================*/
{
	Widget read_signals_button;
	Widget overlay_signals_button;
	Widget save_interval_button;
	Widget save_interval_as_button;
	Widget read_times_button;
	Widget save_times_button;
	Widget save_potentials_button;
	Widget read_bard_data_button;
	Widget read_beekeeper_data_button;
	Widget read_cardiomapp_data_button;
	Widget read_neurosoft_data_button;
}; /* struct Analysis_file_menu */

struct Analysis_print_menu
/*******************************************************************************
LAST MODIFIED : 27 April 1993

DESCRIPTION :
The menu associated with the print button.
==============================================================================*/
{
	Widget all_button;
	Widget page_button;
	Widget selected_button;
}; /* struct Analysis_print_menu */

struct Signals_area
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
The area of the analysis window where the signals are drawn.
==============================================================================*/
{
	Widget drawing_area;
	struct Drawing_2d *drawing;
	enum Signal_layout layout;
	/* axes position information */
	int axes_height,axes_left,axes_top,axes_width;
	/* signal layout */
	int number_of_columns,number_of_rows,number_of_signals,signal_width,
		signal_height;
}; /* struct Signals_area */

struct Interval_area
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
The area of the analysis window where the interval is drawn.
==============================================================================*/
{
	Widget reset_button;
	Widget baseline_button;
	Widget range_button;
	Widget previous_button;
	Widget next_button;
	Widget accept_button;
	Widget reject_button;
#if defined (OLD_CODE)
	struct
	{
		Widget previous_button;
		Widget next_button;
		Widget accept_button;
		Widget reject_button;
	} accelerator;
#endif
	Widget drawing_area;
	struct Drawing_2d *drawing;
	/* axes position information */
	int axes_height,axes_left,axes_top,axes_width;
	/* analysis box information */
	int left_box,right_box;
}; /* struct Interval_area */

struct Analysis_window
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
The analysis window object.
==============================================================================*/
{
	Widget activation,window;
	Widget menu;
	Widget displaypotential_button;
	Widget map_button;
	struct Analysis_map_menu map_menu;
	Widget display_map_warning_box,display_map_warning_box_shell;
	Widget display_map_warning_accept_butt,display_map_warning_reject_butt;
	Widget file_button;
	struct Analysis_file_menu file_menu;
	struct File_open_data *write_times_file_open_data;
	Widget print_button;
	struct Analysis_print_menu print_menu;
	Widget write_event_times_warning_box,write_event_times_warning_box_s;
	Widget order_choice;
	struct
	{
		Widget event_button;
		Widget channel_button;
	} order;
	Widget layout_choice;
	struct
	{
		Widget separate_button;
		Widget overlap_button;
	} layout;
	Widget region_choice;
	Widget region_pull_down_menu;
	int number_of_regions;
	WidgetList regions;
	Widget current_region;
	Widget close_button;
	struct Interval_area interval;
	struct Signals_area signals;
	struct Rig **rig;	
	struct GROUP(FE_node) **rig_node_group;	
	struct Draw_package **draw_package;
	struct Analysis_window **address;
	int *datum,*event_number,*number_of_events;
	int *potential_time;
	int *minimum_separation,*threshold;
	enum Event_detection_algorithm *detection;
	struct Device ***highlight;
	/* search interval display information */
	int *end_search_interval,*start_search_interval;
	struct User_interface *user_interface;
	/* user settings */
	float signal_aspect_ratio;
	int signals_per_printer_page,signal_overlap_spacing;
	struct Signal_drawing_information *signal_drawing_information;
}; /* struct Analysis_window */

enum Analysis_window_update_flags
{
	ANALYSIS_FLAGS_CLEAR = 0,
	ANALYSIS_INTERVAL_NO_POTENTIAL_ERASE = 2
}; /* enum Analysis_window_update_flags */

/*
Global variables
----------------
*/
/*???DB.  Have to do this because analysis_work_area needs access */
#if defined (MOTIF)
extern int analysis_window_hierarchy_open;
extern MrmHierarchy analysis_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
int analysis_window_update_interval_area_time(struct Analysis_window *analysis,
	int potential_time, int previous_potential_time, 
	enum Analysis_window_update_flags *update_flags);
/*******************************************************************************
LAST MODIFIED : 29 October 1998

DESCRIPTION :
Shifts the bar in the interval area part of the analysis window to represent
the new potential time.
==============================================================================*/

int analysis_window_update_signal_area_time(struct Analysis_window *analysis,
	int potential_time, int previous_potential_time);
/*******************************************************************************
LAST MODIFIED : 29 October 1998

DESCRIPTION :
Shifts the bar in the signal area part of the analysis window to represent
the new potential time.
==============================================================================*/

int analysis_window_update_signal_area_datum(struct Analysis_window *analysis,
	int datum_marker, int previous_datum_marker);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Shifts the bar in the signal area part of the analysis window to represent
the new datum time.
==============================================================================*/

int draw_interval_box(int left,int top,int width,int height,
	struct Interval_area *interval,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED :  23 December 1996

DESCRIPTION :
==============================================================================*/

struct Analysis_window *create_Analysis_window(
	struct Analysis_window **address,Widget activation,Widget parent,
	struct Rig **rig,struct GROUP(FE_node) **rig_node_group,
	struct Draw_package **draw_package,
	struct Device ***highlight,int *datum,int *event_number,
	int *number_of_events,int *potential_time,
	enum Event_detection_algorithm *detection,int *threshold,
	int *minimum_separation,Pixel identifying_colour,enum Signal_order order,
	enum Signal_layout layout,int *start_search_interval,int *end_search_interval,
	int screen_height,
		/*???DB.  height of interval drawing area.  Better ? */
	char *postscript_file_extension,char *events_file_extension,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
This function allocates the memory for an analysis window and sets the fields
to the specified values (<address>, <activation>, <rig_address>).  It then
retrieves an analysis window widget with the specified parent/<shell> and
assigns the widget ids to the appropriate fields of the structure.  If
successful it returns a pointer to the created analysis window and, if
<address> is not NULL, makes <*address> point to the created analysis window.
If unsuccessful, NULL is returned.
==============================================================================*/

Widget create_analysis_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for an analysis window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
==============================================================================*/

int update_signals_drawing_area(struct Analysis_window *analysis);
/*******************************************************************************
LAST MODIFIED : 28 June 1992

DESCRIPTION :
The callback for redrawing the analysis signals drawing area.
==============================================================================*/

int update_interval_drawing_area(struct Analysis_window *analysis);
/*******************************************************************************
LAST MODIFIED : 28 July 1992

DESCRIPTION :
The callback for redrawing the analysis interval drawing area.
==============================================================================*/

int draw_all_signals(struct Rig *rig,int datum,int potential_time,
	struct Signals_area *signals,enum Signal_layout layout,
	float signal_aspect_ratio,int signal_overlap_spacing,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,struct GROUP(FE_node) *rig_node_group,
	struct Draw_package *draw_package);
/*******************************************************************************
LAST MODIFIED : 17 June 1999

DESCRIPTION :
==============================================================================*/

int update_analysis_window_menu(struct Analysis_window *analysis);
/*******************************************************************************
LAST MODIFIED : 7 August 1992

DESCRIPTION :
Updates the analysis down menu to be consistent with the current rig.
==============================================================================*/

int highlight_signal(struct Device *device,int device_number,int start_data,
	int end_data,int datum,int potential_time,struct Signals_area *signals,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 December 1996

DESCRIPTION :
Highlights/dehighlights the <device> in the <signals> area.
==============================================================================*/
#endif
