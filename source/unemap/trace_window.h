/*******************************************************************************
FILE : trace_window.h

LAST MODIFIED : 30 May 2001

DESCRIPTION :
==============================================================================*/
#if !defined (TRACE_WINDOW_H)
#define TRACE_WINDOW_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "unemap/analysis.h"
#include "unemap/rig.h"
#include "unemap/spectral_methods.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum Signal_analysis_mode 
/*******************************************************************************
LAST MODIFIED : 9 February 2001

DESCRIPTION :
The type of signal analysis being performed.
==============================================================================*/
{
	ELECTRICAL_IMAGING,
	EVENT_DETECTION,
	FREQUENCY_DOMAIN,
	POWER_SPECTRA,
	CROSS_CORRELATION,
	AUTO_CORRELATION,
	FILTERING,
	BEAT_AVERAGING
}; /* enum Signal_analysis_mode */

enum Inverse_electrodes_mode
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
The type of electrodes used for inverse
==============================================================================*/
{
	ELECTRODES_ACCEPTED,
	ELECTRODES_UNREJECTED,
	ELECTRODES_ALL
}; /* enum Inverse_electrodes_mode */

enum Inverse_wave_mode 
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
The type of wave used for inverse
==============================================================================*/
{
	NO_WAVE,
	P_WAVE,
	QRS_WAVE,
	T_WAVE,
	PQRS_WAVE,
	PT_WAVE,
	QRST_WAVE,
	PQRST_WAVE
}; /* enum Inverse_wave_mode */

enum Inverse_potential_activation_mode
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
The type  used for potential/activation
==============================================================================*/
{
	INVERSE_POTENTIAL,
	INVERSE_ACTIVATION
}; /* enum Inverse_potential_activation_mode */

enum Calculate_signal_mode
/*******************************************************************************
LAST MODIFIED :14 March 2001

DESCRIPTION :
The type of signal used for Calculate
==============================================================================*/
{
	CURRENT_SIGNAL,
	RMS_SIGNAL,
	RMS_AND_CURRENT_SIGNAL	
}; /* enum Calculate_signal_mode */


struct Enlarge_area
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
The area of the trace window where the enlarged signal is drawn.
==============================================================================*/
{
	Widget menu;
	Widget calculate_button;
	Widget detection_choice;
	struct
	{
		Widget interval_button;
		Widget level_button;
		Widget threshold_button;
	} detection;
	Widget objective_choice;
	struct
	{
		Widget absolute_slope_button;
		Widget positive_slope_button;
		Widget negative_slope_button;
		Widget absolute_value_button;
		Widget negative_value_button;
		Widget positive_value_button;
	} objective;
	Widget datum_choice;
	struct
	{
		Widget automatic_button;
		Widget fixed_button;
	} datum;
	Widget number_of_events_form;
	struct
	{
		Widget down_arrow;
		Widget label;
		Widget up_arrow;
	} number_of_events;
	Widget threshold_scroll;
	Widget threshold_label;
	Widget minimum_separation_scroll;
	Widget minimum_separation_label;
	Widget level_value;
	Widget all_current_choice;
	struct
	{
		Widget all_button;
		Widget current_button;
	} all_current;
	/* search box information */
	int *divisions,left_box,right_box;
	/* edit box information */
	int left_edit_box,right_edit_box;
	char calculate_all_events;
}; /* struct Enlarge_area */

struct Inverse_area 
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
The inverse area of the trace window 
==============================================================================*/
{
	Widget menu;
	Widget electrodes_choice_mode;
	struct 
	{
		Widget accepted_button;
		Widget unrejected_button;
		Widget all_button;
	} electrodes_choice;
	Widget wave_choice_mode;
	struct 
	{
		Widget p_button;
		Widget qrs_button;
		Widget t_button;
		Widget pqrs_button;
		Widget pt_button;
		Widget qrst_button;
		Widget pqrst_button;
		Widget none_button;	
	} wave_choice;
	Widget pot_act_choice_mode;
	struct 
	{
		Widget potential_button;
		Widget activation_button;	
	} pot_act_choice;	
	Widget load_button;
	Widget inverse_button;
	Widget forward_button;
	Widget improve_button;
}; /* struct Inverse_area */

struct Calculate_area 
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
The Calculate area of the trace window 
==============================================================================*/
{
	Widget menu;
	Widget apply_button;	
	Widget interval_rank_cutoff_mode_choice;
	struct 
	{
		Widget interval_button;
		Widget rank_cutoff_button;	
	} interval_rank_cutoff_choice;
	Widget RMS_current_mode_choice;
	struct 
	{
		Widget RMS_signal_button;
		Widget current_signal_button;
		Widget rms_current_signal_button;		
	} RMS_current_choice;	
	Widget calculate_button;
	Widget cutoff_value;
}; /* struct Calculate_area */

struct Eimaging_interval_area 
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
The Interval area of the trace window 
==============================================================================*/
{
	Widget menu;
	Widget times_button;	
}; /* struct Eimaging_interval_area */

struct Edit_area
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
The area of the trace window where the event time is edited.
==============================================================================*/
{
	Widget menu;
	Widget previous_button;
	Widget next_button;
	Widget accept_button;
	Widget reject_button;
	Widget order_choice;
	struct
	{
		Widget device_button;
		Widget beat_button;
	} order;
	Widget objective_toggle;
	/* data interval being displayed */
	int first_data,last_data;
}; /* struct Edit_area */

struct Event_detection
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
The information for detecting events.
==============================================================================*/
{
	enum Event_detection_algorithm *detection;
	enum Event_detection_objective *objective;
	enum Datum_type *datum_type;
	enum Edit_order *edit_order;
	float *level;
	int *average_width,*datum,*event_number,*minimum_separation,*number_of_events,
		*potential_time,*threshold;
	/* search interval display information */
	int *end_search_interval,**search_interval_divisions,*start_search_interval;
}; /* struct Event_detection */

enum Frequency_domain_display_mode
/*******************************************************************************
LAST MODIFIED : 26 August 1993

DESCRIPTION :
How the signals should be displayed in the frequency domain.
==============================================================================*/
{
	AMPLITUDE_PHASE,
	REAL_IMAGINARY
}; /* enum Frequency_domain_display_mode */

struct Frequency_domain_area
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
The control widgets for a drawing area where a signal is displayed in the
frequency domain.
==============================================================================*/
{
	Widget menu;
	Widget display_mode_choice;
	struct
	{
		Widget amplitude_phase_button;
		Widget real_imaginary_button;
	} display_mode;
	Widget window_type_choice;
	struct
	{
		Widget hamming_button;
		Widget parzen_button;
		Widget square_button;
		Widget welch_button;
	} window_type_menu;
}; /* struct Frequency_domain_area */

struct Frequency_domain
/*******************************************************************************
LAST MODIFIED : 17 March 1995

DESCRIPTION :
The information for calculating the frequency domain response.
==============================================================================*/
{
	enum Frequency_domain_display_mode display_mode;
	enum Fourier_window_type window_type;
}; /* struct Frequency_domain */

struct Power_spectra_area
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
The control widgets for a drawing area where a power spectra is displayed.
==============================================================================*/
{
	Widget menu;
	Widget label;
	Widget minimum_frequency_scroll_bar;
	Widget minimum_frequency_text_field;
	Widget maximum_frequency_scroll_bar;
	Widget maximum_frequency_text_field;
}; /* struct Power_spectra_area */

struct Power_spectra
/*******************************************************************************
LAST MODIFIED : 15 August 1999

DESCRIPTION :
The information for displaying the power spectra.
==============================================================================*/
{
	float maximum_frequency,minimum_frequency;
}; /* struct Power_spectra */

struct Correlation_time_domain_area
/*******************************************************************************
LAST MODIFIED : 30 August 1993

DESCRIPTION :
The control widgets for a drawing area where a signal is displayed in the
correlation time domain.
==============================================================================*/
{
	Widget menu;
	Widget toggle;
}; /* struct Correlation_time_domain_area */

struct Correlation_area
/*******************************************************************************
LAST MODIFIED : 26 August 1993

DESCRIPTION :
The control widgets for a drawing area where a correlation (cross or auto) is
displayed.
==============================================================================*/
{
	Widget menu;
	Widget label;
}; /* struct Correlation_area */

struct Correlation
/*******************************************************************************
LAST MODIFIED : 30 August 1993

DESCRIPTION :
The information for calculating the correlation.
==============================================================================*/
{
	char signal_1_input;
	struct Device *device_1,*device_2;
}; /* struct Correlation */

struct Filtering_area
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
The control widgets for a drawing area where a filtered signal is displayed.
==============================================================================*/
{
	Widget menu;
	Widget label;
	Widget high_pass_scroll_bar;
	Widget high_pass_text_field;
	Widget low_pass_scroll_bar;
	Widget low_pass_text_field;
	Widget notch_toggle;
	Widget notch_text_field;
}; /* struct Filtering_area */

struct Filtering
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
The information for calculating the filtered response.
==============================================================================*/
{
	float high_pass_frequency,low_pass_frequency,notch_frequency;
	int notch_on;
}; /* struct Filtering */

struct Beat_averaging_area_1
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
The control widgets for beat averaging area 1.
???DB.  Number of beats is shared with number of events
==============================================================================*/
{
	Widget menu;
	Widget number_of_beats_form;
	struct
	{
		Widget down_arrow;
		Widget label;
		Widget up_arrow;
	} number_of_beats;
	Widget align_with_events_button;
}; /* struct Beat_averaging_area_1 */

struct Beat_averaging_area_3
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
The control widgets for beat averaging area 3.
==============================================================================*/
{
	Widget menu;
	Widget baseline_toggle;
	Widget beat_averaging_toggle;
	Widget overlay_beats_toggle;
}; /* struct Beat_averaging_area_3 */

struct Trace_window_menu
/*******************************************************************************
LAST MODIFIED : 9 February 2001

DESCRIPTION :
The menu in the trace window.
==============================================================================*/
{
	Widget analysis_mode_choice;
	struct
	{
		Widget event_detection_button;
		Widget frequency_domain_button;
		Widget power_spectra_button;
		Widget cross_correlation_button;
		Widget auto_correlation_button;
		Widget filtering_button;
		Widget beat_averaging_button;
		Widget eimaging_button;
	} analysis_mode;
	Widget apply_button;
	Widget close_button;
	Widget average_width;
	Widget average_width_txt;
}; /* struct Trace_window_menu */

struct Trace_window_area_1
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
The drawing area 1 in the trace window.
==============================================================================*/
{
	struct Inverse_area inverse;
	struct Calculate_area calculate; 
	struct Enlarge_area enlarge;
	struct Correlation_time_domain_area correlation_time_domain;
	struct Beat_averaging_area_1 beat_averaging;
	Widget pane;
	Widget drawing_area;
	struct Drawing_2d *drawing;
	/* axes position information */
	int axes_height,axes_left,axes_top,axes_width;
}; /* struct Trace_window_area_1 */

struct Trace_window_area_2
/*******************************************************************************
LAST MODIFIED : 24 December 1996

DESCRIPTION :
The drawing area 2 in the trace window.
==============================================================================*/
{
	struct Correlation_time_domain_area correlation_time_domain;
	Widget pane;
	Widget drawing_area;
	struct Drawing_2d *drawing;
	/* axes position information */
	int axes_height,axes_left,axes_top,axes_width;
}; /* struct Trace_window_area_2 */

struct Trace_window_area_3
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
The drawing area 3 in the trace window.
==============================================================================*/
{
	struct Edit_area edit;
	struct Frequency_domain_area frequency_domain;
	struct Eimaging_interval_area interval;
	struct Power_spectra_area power_spectra;
	struct Correlation_area correlation;
	struct Filtering_area filtering;
	struct Beat_averaging_area_3 beat_averaging;
	Widget pane;
	Widget drawing_area;
	struct Drawing_2d *drawing;
	/* axes position information */
	int axes_height,axes_left,axes_top,axes_width;
}; /* struct Trace_window_area_3 */

struct Cardiac_interval
/*******************************************************************************
LAST MODIFIED : 22 March 2001

DESCRIPTION :
the Cardiac interval on the electrical imaging pane. P, T QRS, etc
==============================================================================*/
{
	GC graphics_context; /*colour*/
	int peak_or_trough_time,start_time,end_time;		
	struct Cardiac_interval *next,*previous;
}; /* struct Cardiac_interval */

struct Electrical_imaging_event
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Events (times) on the cardiac_intervals_device signal.
Used to generate maps.
==============================================================================*/
{
	GC graphics_context; /*colour*/
	int time;		
	struct Electrical_imaging_event *next,*previous;
}; /* Electrical_imaging_event */

struct Trace_window
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
The trace window object.
==============================================================================*/
{
	char open;
	struct Trace_window **address;
	Widget activation,shell,window,paned_window;
	enum Calculate_signal_mode calculate_signal_mode;
	struct Cardiac_interval *first_interval;
	struct Electrical_imaging_event *first_eimaging_event; 
	enum Inverse_wave_mode inverse_wave_mode;
	enum Inverse_electrodes_mode inverse_electrodes_mode;
	enum Inverse_potential_activation_mode inverse_pot_act_mode;
	enum Signal_analysis_mode analysis_mode;
	struct Trace_window_menu menu;
	struct Trace_window_area_1 area_1;
	struct Trace_window_area_2 area_2;
	struct Trace_window_area_3 area_3;
	struct Event_detection event_detection;
	struct Frequency_domain frequency_domain;
	struct Power_spectra power_spectra;
	struct Correlation correlation;
	struct Filtering filtering;
	struct Device *imaginary_device_1,*imaginary_device_2,*processed_device,
		*real_device_1,*real_device_2,*cardiac_interval_device;	
	int calculate_rms; /* A flag. Don't always want to calculate rms of signals*/
	int valid_processing;
	struct Device ***highlight;
#if defined (UNEMAP_USE_NODES)
	struct FE_node **highlight_rig_node;
#endif /* defined (UNEMAP_USE_NODES) */
	struct Rig **rig;
	struct Signal_drawing_information *signal_drawing_information;
	struct Signal_drawing_package **signal_drawing_package;
	struct User_interface *user_interface;
}; /* struct Trace_window */

enum Trace_window_update_flags
{
	TRACE_FLAGS_CLEAR = 0,
	TRACE_1_NO_POTENTIAL_ERASE = 2,
	TRACE_3_NO_POTENTIAL_ERASE = 4,
	TRACE_1_NO_DATUM_ERASE = 8,
	TRACE_3_NO_DATUM_ERASE = 16
}; /* enum Trace_window_update_flags */

/*
Global functions
----------------
*/
int trace_process_device(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 14 January 2000

DESCRIPTION :
Calculates the processed device.
==============================================================================*/

int draw_highlight_event_box(int left,int top,int width,int height,
	enum Event_detection_algorithm detection,Widget drawing_area,
	struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information);
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
???DB.  Maybe not needed anymore ?
==============================================================================*/

int draw_search_box(int left,int top,int width,int height,
	enum Event_detection_algorithm detection,int number_of_events,int *divisions,
	Widget drawing_area,struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information);
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION : draws the search box
==============================================================================*/

int open_trace_window(struct Trace_window **trace_address,Widget parent,
	Pixel identifying_colour,enum Signal_analysis_mode analysis_mode,
	enum Event_detection_algorithm *detection,
	enum Event_detection_objective *objective,enum Datum_type *datum_type,
	enum Edit_order *edit_order,struct Device ***highlight,
#if defined (UNEMAP_USE_NODES)
	struct FE_node **highlight_rig_node,
#endif /* defined (UNEMAP_USE_NODES) */
	struct Rig **rig,struct Signal_drawing_package **signal_drawing_package,
	int *datum,int *potential_time,int *event_number,int *number_of_events,
	int *threshold,int *minimum_separation,float *level,int *average_width,
	int *start_search_interval,int **search_interval_divisions,
	int *end_search_interval,int screen_width,int screen_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 January 2000

DESCRIPTION :
If <*trace_address> is NULL, a trace window with the specified <parent> and 
<identifying_colour> is created.  The trace window is opened.
==============================================================================*/

void redraw_trace_1_drawing_area(Widget widget,
	XtPointer trace_window,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 30 August 1993

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 1.
???DB.  Should be a module function.  Need to move select_trace_1_drawing_area
first.
==============================================================================*/

void redraw_trace_3_drawing_area(Widget widget,
	XtPointer trace_window,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 30 August 1993

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 3.
???DB.  Should be a module function.  Need to move select_trace_1_drawing_area
first.
==============================================================================*/

int trace_change_signal(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Called when the "highlighted_device" is changed.
???DB.  More work needed.  Pass device/signal ?
==============================================================================*/

int trace_change_event(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Change the event without changing the device.
???DB.  The same as trace_change_signal for EVENT_DETECTION at present, but
should be able to do better.  Will have to keep track of event changed from ?
See analysis_previous_event.
==============================================================================*/

int trace_change_signal_status(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Change the signal status (accept, reject, undecided) without changing the
device.
==============================================================================*/

int trace_change_display_interval(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Change the signal interval displayed in the trace window.
==============================================================================*/

int trace_change_search_interval(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Change the search interval displayed in the trace window.
==============================================================================*/

int trace_update_signal_controls(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Updates the selectability of the signal controls.
==============================================================================*/

int trace_draw_event_marker(struct Event *event,struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Draws the <event> marker in the <trace> window.
==============================================================================*/

int trace_draw_potential_time(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Draws the potential time marker in the <trace> window.
==============================================================================*/

int trace_update_potential_time(struct Trace_window *trace, int potential_time,
	int previous_potential_time, enum Trace_window_update_flags *update_flags);
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Updates the potential time marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/

int trace_update_datum(struct Trace_window *trace, int datum,
	int previous_datum, int event_number, int potential_time,
	enum Trace_window_update_flags *update_flags);
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Updates the datum marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/

int trace_draw_markers(char draw_datum,char draw_potential_time,
	struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Draws the markers in the <trace> window.
==============================================================================*/

int trace_update_edit_interval(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 30 August 1993

DESCRIPTION :
???DB.  Should be a module function.  Wait until select_trace_1_drawing_area
has been moved.
==============================================================================*/

int trace_change_rig(struct Trace_window *trace);
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
Sets both the cross correlation devices to the highlight device.  Should be
called when the analysis rig is changed.
==============================================================================*/

int move_cardiac_interval(XmDrawingAreaCallbackStruct *callback,
	struct Device *highlight_device,struct Trace_window *trace,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,int pointer_sensitivity);
/*******************************************************************************
LAST MODIFIED :  28 March 2001

DESCRIPTION : Moves the cardiac interval in time.
==============================================================================*/

#endif /* !defined (TRACE_WINDOW_H) */
