/*******************************************************************************
FILE : eimaging_time_dialog.h

LAST MODIFIED : 19 December 2001

DESCRIPTION :
==============================================================================*/
#if !defined (EIMAGING_TIME_DIALOG_H)
#define EIMAGING_TIME_DIALOG_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

/*
Global types
------------
*/
enum Electrical_imaging_base_event
{
	EVENT_CURRENT,
	EVENT_CLEAR,
	EVENT_P_WAVE_START,
	EVENT_P_WAVE_PEAK_OR_TROUGH,
	EVENT_P_WAVE_END,
	EVENT_QRS_WAVE_START,	
	EVENT_QRS_WAVE_PEAK_OR_TROUGH,
	EVENT_QRS_WAVE_END,
	EVENT_T_WAVE_START,
	EVENT_T_WAVE_PEAK_OR_TROUGH,
	EVENT_T_WAVE_END
};

struct Electrical_imaging_time_dialog
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
The dialog box for electrical imaging times .
==============================================================================*/
{
	enum Electrical_imaging_base_event reference_event;
	/* eimaging_marker_period in milliseconds */
	float marker_period;
	int max_period;
	int scroll_width,settings_changed; 
	struct Electrical_imaging_time_dialog **address;
	struct Shell_list_item *shell_list_item;
	struct User_interface *user_interface;
	Widget activation;
	Widget ok_button;	
	Widget apply_button;
	Widget cancel_button;	
	Widget current_button;	
	Widget clear_button;
	Widget p_wave_start_button;
	Widget p_wave_peak_or_trough_button;
	Widget p_wave_end_button;
	Widget qrs_wave_start_button;
	Widget qrs_wave_peak_or_trough_button;
	Widget qrs_wave_end_button;	
	Widget t_wave_start_button;
	Widget t_wave_peak_or_trough_button;
	Widget t_wave_end_button;
	Widget dialog;
	Widget period_label;
	Widget period_menu;
	Widget period_scroll;
	Widget period_value;
	Widget shell;
	Widget event_choice;
	Widget event_menu;
	Widget event_pull_down;
}; /* struct Map_dialog */

/*
Global functions
----------------
*/
struct Electrical_imaging_time_dialog *create_Electrical_imaging_time_dialog(
	struct Electrical_imaging_time_dialog **eimaging_time_dialog_address,
	Widget activation,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Allocates the memory for an Electrical_imaging_time dialog.  Retrieves the 
necessary widgets and initializes the appropriate fields.
==============================================================================*/

int open_eimaging_time_dialog(
	struct Electrical_imaging_time_dialog *eimaging_time_dialog);
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Opens the <eimaging_time_dialog>.
==============================================================================*/

#if defined (MOTIF)
void close_eimaging_time_dialog(Widget widget,
	XtPointer eimaging_time_dialog_structure,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Closes the windows associated with the imaging_time_dialog box.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (EIMAGING_TIME_DIALOG_H) */
