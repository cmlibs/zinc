/*******************************************************************************
FILE : analysis_work_area.c

LAST MODIFIED : 14 April 2004

DESCRIPTION :
???DB.  Everything or nothing should be using the datum_time_object.  Currently
	it is used when the datum is dragged, but not when it is calculated.
???DB.  Have yet to tie event objective and preprocessor into the event times
	file

???DB.  Temp.  Beat averaging considerations
1 Beat averaging is done in trace_analysis_mode_apply (analysis_work_area.c) and
	trace_change_signal (trace_window.c)
2 Greg's algorithm
	- determine the times to average about (one for each beat), by picking a
		stimulation signal (could be the current signal in unemap) and finding
		"peaks" on the stimulation signal (could be the event times in unemap)
	- specify start_offset - the start of the signal relative to the peak (same
		for all beats
	- for each signal, align [peak[0]-start_offset,peak[1]-start_offset),
		[peak[1]-start_offset,peak[2]-start_offset), ... [peak[n-2]-start_offset,
		peak[n-1]-start_offset) and average
3 Questions
	- QRS is coincident for body surface electrodes, but not for epicardial.  This
		means that using the same averaging times for all electrodes could cause
		problems.  If we use different averaging times for each electrode, how do we
		get the offsets?  Ignore at present - see how Greg's algorithm works
	- How does the edit box (white) work for threshold/level?
		When the current event is changed, if the new current marker is not within
		the edit box or is within an 1/8 of the beginning or an 1/8 of the end of
		the edit box, the edit box is centred on the new current marker
	- What does Baseline do?
		Makes the first and last values in each interval 0 by subtracting a straight
		line that goes from the first value to the last value
4 How should the interface work?
	- extend the search box so that the intervals are specified rather than
		assumed to be constant width
		DONE
	- extend moving the search box so that up to the end of the first interval
		can go off the left and up to the beginning of the last interval can go off
		the right
	- change dragging the search box ends so that internal divisions don't change
		when the ends are moved (limited by first and last internal divisions)
		DONE.  Instead of limiting, the divisions "concertina"
	- add an "Align with events" button.  This redraws the search box (see
		draw_search_box in trace_window.c) so that the start of each search
		interval is an event time (starting with the first event in the initial
		search box)
		DONE
	- retain constant beat width version by having that when the number of
		intervals is changed, it goes to constant width intervals
		DONE
	- be able to reject beats by rejecting the corresponding events (the first
		event in the beat)
		DONE
	- allow event editing in beat averaging
	- save search/edit intervals when apply (for reset)
	- add an "Overlay beats" check box
		DONE
5 Intended method of use
	- read signal file
	- pick a signal to use to determine the averaging intervals
	- select Event detection using the Interval algorithm
	- choose the number of beats to average over and place the search interval
		over the beats
	- calculate the event times
	- edit the event times
	- select Beat averaging
	- click Align
	- move the search box to include information before event
	- choose Baseline and Beat average
	- check that happy for current signal.  If not, adjust event times and
		realign the search interval
	- identify bad averages by either stepping through or applying and
		looking at the signals window (if applying, need to reset before fixing)
	- for a bad average can reject beats by rejecting events
6 Bug fixes
	- make beat averaging always use EDA_INTERVAL.
		DONE by adding set_detection_interval to the beat_averaging_button in
		unemap/trace_window.uil
	- don't use the highlight colour for the electrode name in the trace window
		(because inconsistent for cross correlation)
		DONE
	- beat average should start from time 0
		DONE
	- widget layout for event detection under linux
		IMPROVED, but still not happy with vertical centering of text in buttons or
		having to use a 1x1 pixmap (no_cascade_pixmap) to remove blank area on right
		from option menus
	- increase the maximum number of beats to average over
		DONE
	- start_search_interval and end_search_interval assume no missing samples
	- not updating beat window properly when reset (maybe others as well)

???DB.  Displaying event detection objective
1 Modify calculate_device_event_markers and processed_device(?) so that
	processed_device (if passed) is filled with the objective function by
	calculate_device_event_markers.
2 How to display objective?
	Would be nice to have on same graph, but won't have same y-scale and not
	another signal for highlight device (can't use overlay in draw_signal).
	Always display processed device for event calculation (copy highlight_device
	in?)
	Could have another pane, but getting too many panes and objective not close
	enough to signal.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#if defined (UNEMAP_USE_NODES)
#include "finite_element/finite_element.h"
#endif /* defined (UNEMAP_USE_NODES) */
#include "general/debug.h"
#include "general/geometry.h"
#include "general/heapsort.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "time/time.h"
#include "time/time_editor_dialog.h"
#include "time/time_keeper.h"
#include "unemap/analysis.h"
#include "unemap/analysis_window.h"
#include "unemap/analysis_work_area.h"
#include "unemap/bard.h"
#include "unemap/beekeeper.h"
#include "unemap/cardiomapp.h"
#include "unemap/drawing_2d.h"
#include "unemap/edf.h"
#include "unemap/interpolate.h"
#include "unemap/mapping_work_area.h"
#include "unemap/neurosoft.h"
#include "unemap/trace_window.h"
#include "unemap/unemap_package.h"
#include "user_interface/confirmation.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
/*
Module constants
----------------
*/
#define MAX_SPECTRUM_COLOURS 256
#define MAX_EVENTS 99

/*
Module types
------------
*/
#if defined (UNEMAP_USE_3D)
struct rig_node_selection_change_data
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION : Stores info used by  rig_node_group_node_selection_change
==============================================================================*/
{
	struct Analysis_work_area *analysis_work_area;
	int multiple_selection,highlight;
};
#endif /* defined (UNEMAP_USE_3D) */

struct Set_highlight_iterator
/*******************************************************************************
LAST MODIFIED : 15 August 2000

DESCRIPTION : Stores info for iterating through rig nodes and setting
highlight field
==============================================================================*/
{
	int highlight;
	int count;
	struct FE_field *highlight_field;
};

typedef struct Analysis_work_area Analysis_work_area_settings;

/*
Module variables
----------------
*/
struct Rig *bard_rig=(struct Rig *)NULL;
struct Rig *cardiomapp_rig=(struct Rig *)NULL;
struct Rig *neurosoft_rig=(struct Rig *)NULL;

#define analysis_flag_trace_3_no_potential_text_erase (8)
#define analysis_flag_trace_3_no_potential_bar_erase (16)
#define analysis_flag_trace_1_no_potential_bar_erase (32)

/*
Module functions
----------------
*/
static enum Projection_type ensure_projection_type_matches_region_type(
	struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 20 September 2001

DESCRIPTION : Ensure that the map->projection_type and the
rig->current_region->type are compatible.
==============================================================================*/
{
	enum Projection_type projection_type;

	ENTER(ensure_projection_type_matches_region_type);
	projection_type=HAMMER_PROJECTION;
	if (analysis)
	{
		if ((analysis->mapping_window)&&(analysis->mapping_window->map))
		{
			ensure_map_projection_type_matches_region_type(
				analysis->mapping_window->map);
		}
	}
	else/* if (analysis) */
	{
		display_message(ERROR_MESSAGE,
			"ensure_projection_type_matches_region_type. Invalid argument");
	}
	LEAVE;

	return (projection_type);
} /* ensure_projection_type_matches_region_type */

static void display_map(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
??? colour bar ?
??? multiple beats ?
==============================================================================*/
{
	enum Projection_type projection_type;
	int maintain_aspect_ratio;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Signal_buffer *buffer;
	struct Spectrum *spectrum;
	struct User_interface *user_interface;

	ENTER(display_map);
	spectrum=(struct Spectrum *)NULL;
	analysis_window=(struct Analysis_window *)NULL;
	analysis=(struct Analysis_work_area *)NULL;
	map=(struct Map *)NULL;
	mapping=(struct Mapping_window *)NULL;
	user_interface=(struct User_interface *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window)&&
		(user_interface=analysis->user_interface)&&
		(spectrum=analysis->map_drawing_information->spectrum))
	{
		if ((analysis->map_type_changed)||
			(UNKNOWN_SPECTRUM==Spectrum_get_simple_type(spectrum)))
		{
			/* revert to the default spectrum type if the map type has changed  */
#if defined (OLD_CODE)
			/*??JW Why do this?*/
			switch (analysis->map_type)
			{
				case SINGLE_ACTIVATION:
				case MULTIPLE_ACTIVATION:
				{
					Spectrum_set_simple_type(spectrum,RED_TO_BLUE_SPECTRUM);
				} break;
				default:
				{
					Spectrum_set_simple_type(spectrum,BLUE_TO_RED_SPECTRUM);
				} break;
			}
#endif
			analysis->map_type_changed=0;
		}

		if (UNKNOWN_SPECTRUM==Spectrum_get_simple_type(spectrum))
		{
			Spectrum_set_simple_type(spectrum,BLUE_TO_RED_SPECTRUM);
		}
		if (widget==analysis_window->display_map_warning_box)
		{
			busy_cursor_off(analysis_window->display_map_warning_box_shell,
				analysis->user_interface);
		}
		if (analysis->map_drawing_information->maintain_aspect_ratio)
		{
			maintain_aspect_ratio=1;
		}
		else
		{
			maintain_aspect_ratio=0;
		}
		/*ensure projection_type matches region type */
		projection_type=ensure_projection_type_matches_region_type(analysis);
		/*???should create mapping window and map if not present */
		if (open_mapping_window(&(analysis->mapping_window),
			*(analysis->mapping_work_area->activation),
			*(analysis->mapping_work_area->parent),
			&(analysis->mapping_work_area->window_shell),
			&(analysis->mapping_work_area->outer_form),
			&(analysis->mapping_work_area->current_mapping_window),
			&(analysis->mapping_work_area->open),
			&(analysis->mapping_work_area->associate),&(analysis->map_type),
			SHOW_COLOUR,HIDE_CONTOURS,SHOW_ELECTRODE_VALUES,HIDE_FIBRES,
			HIDE_LANDMARKS,HIDE_EXTREMA,maintain_aspect_ratio,1,projection_type,
			VARIABLE_THICKNESS,&(analysis->rig),&(analysis->event_number),
			&(analysis->potential_time),&(analysis->datum),
			&(analysis->start_search_interval),&(analysis->end_search_interval),
			analysis->identifying_colour,ANALYSIS_ASSOCIATE,
			(XtPointer)set_mapping_analysis_region,
			(XtPointer)analysis_select_map_drawing_are,
			(XtPointer)analysis_select_auxiliary_drawi,analysis_work_area,
			User_interface_get_screen_width(user_interface),
			User_interface_get_screen_height(user_interface),
			analysis->configuration_file_extension,
			analysis->postscript_file_extension,analysis->map_drawing_information,
			analysis->user_interface,analysis->unemap_package,
			&(analysis->first_eimaging_event),&(analysis->analysis_mode)))
		{
			mapping=analysis->mapping_window;
			map=mapping->map;
			Mapping_window_set_potential_time_object(mapping,
				analysis->potential_time_object);
			/* determine if undecided events are accepted or rejected */
			if ((widget==analysis_window->map_menu.single_activation_button)||
				(widget==analysis_window->map_menu.multiple_activation_button)||
				(widget==analysis_window->map_menu.activation_potential_button)||
				(widget==analysis_window->map_menu.integral_button)||
				(widget==analysis_window->map_menu.potential_button))
			{
				/* should be no undecided events */
				map->undecided_accepted=0;
			}
			else
			{
				if (widget==analysis_window->display_map_warning_box)
				{
					switch (((XmAnyCallbackStruct *)call_data)->reason)
					{
						case XmCR_OK:
						{
							map->undecided_accepted=1;
						} break;
						case XmCR_CANCEL:
						{
							map->undecided_accepted=0;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,"display_map.  Invalid reason");
							map->undecided_accepted=0;
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"display_map.  Invalid widget");
					map->undecided_accepted=0;
				}
			}
			if ((analysis->time_keeper)&&
				Time_keeper_is_playing(analysis->time_keeper))
			{
				map->activation_front=0;
			}
			else
			{
				map->activation_front= -1;
			}
#if defined (OLD_CODE)
			if (!(map->fixed_range))
			{
				map->minimum_value=1;
				map->maximum_value=0;
			}
#endif /* defined (OLD_CODE) */
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
			else
			{
				map->start_time=0;
				map->end_time=0;
			}
			/*we're drawing a single map*/
			map->number_of_frames=1;
			update_mapping_drawing_area(mapping,2);
			update_mapping_colour_or_auxili(mapping);
			mapping_window_set_animation_buttons(mapping);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"display_map.  Invalid rig");
	}
	LEAVE;
} /* display_map */

static void close_display_map_warning(Widget widget,
	XtPointer warning_box_widget,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 May 1996

DESCRIPTION :
==============================================================================*/
{
	Widget warning_box;

	ENTER(close_display_map_warning);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (warning_box=(Widget)warning_box_widget)
	{
		XtUnmanageChild(warning_box);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_display_map_warning.  Missing warning_box_widget");
	}
	LEAVE;
} /* close_display_map_warning */

static void busy_cursor_off_warning_box_shell(Widget widget,
	XtPointer analysis_window_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Callback that switchs from the busy cursor to the default cursor for all shells
except the warning box shell.
==============================================================================*/
{
	struct Analysis_window *analysis_window;

	ENTER(busy_cursor_off_warning_box_shell);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis_window=(struct Analysis_window *)analysis_window_void)
	{
		busy_cursor_off(analysis_window->display_map_warning_box_shell,
			analysis_window->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"busy_cursor_off_warning_box_shell.  Missing file_open_data");
	}
	LEAVE;
} /* busy_cursor_off_warning_box_shell */

static void display_map_with_check(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Check the analysis rig for events and for undecided events.  Prompt user for
continuation.  Display the map.
==============================================================================*/
{
	char no_accepted,no_event,no_undecided;
	enum Map_type map_type;
	int end_search_interval,event_number,number_of_devices,potential_time,
		start_search_interval;
	MrmType warning_box_class;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Device_description *description;
	struct Event *event;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct User_interface *user_interface;
	Widget parent,warning_box,warning_box_shell;

	ENTER(display_map_with_check);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(user_interface=analysis->user_interface)&&
		(analysis_window=analysis->window)&&(rig=analysis->rig)&&
		(device=rig->devices)&&((number_of_devices=rig->number_of_devices)>0))
	{
		if (widget==analysis_window->map_menu.single_activation_button)
		{
			map_type=SINGLE_ACTIVATION;
			event_number=analysis->event_number;
		}
		else
		{
			if (widget==analysis_window->map_menu.multiple_activation_button)
			{
				map_type=MULTIPLE_ACTIVATION;
				event_number=analysis->event_number;
			}
			else
			{
				if (widget==analysis_window->map_menu.potential_button)
				{
					map_type=POTENTIAL;
					potential_time=analysis->potential_time;
				}
				else
				{
					if (widget==analysis_window->map_menu.integral_button)
					{
						map_type=INTEGRAL;
						start_search_interval=analysis->start_search_interval;
						end_search_interval=analysis->end_search_interval;
					}
					else
					{
						map_type=ACTIVATION_POTENTIAL;
						event_number=analysis->event_number;
					}
				}
			}
		}
		if (analysis->map_type!=map_type)
		{
			analysis->map_type_changed=1;
		}
		else
		{
			analysis->map_type_changed=0;
		}
		analysis->map_type=map_type;
		current_region=get_Rig_current_region(rig);
		no_event=1;
		no_undecided=1;
		no_accepted=1;
		switch (map_type)
		{
			case SINGLE_ACTIVATION:
			case ACTIVATION_POTENTIAL:
			{
				while ((number_of_devices>0)&&(no_undecided||no_accepted))
				{
					if ((ELECTRODE==(description=(*device)->description)->type)&&
						((*device)->signal)&&
						(!current_region||(description->region==current_region)))
					{
						event=(*device)->signal->first_event;
						while (event&&(event->number<event_number))
						{
							event=event->next;
						}
						if (event&&(event_number==event->number))
						{
							no_event=0;
							switch (event->status)
							{
								case UNDECIDED:
								{
									no_undecided=0;
								} break;
								case ACCEPTED:
								{
									no_accepted=0;
								} break;
							}
						}
					}
					device++;
					number_of_devices--;
				}
			} break;
			case MULTIPLE_ACTIVATION:
			{
				while ((number_of_devices>0)&&(no_undecided||no_accepted))
				{
					if ((ELECTRODE==(description=(*device)->description)->type)&&
						((*device)->signal)&&
						(!current_region||(description->region==current_region)))
					{
						event=(*device)->signal->first_event;
						while (event)
						{
							no_event=0;
							switch (event->status)
							{
								case UNDECIDED:
								{
									no_undecided=0;
								} break;
								case ACCEPTED:
								{
									no_accepted=0;
								} break;
							}
							event=event->next;
						}
					}
					device++;
					number_of_devices--;
				}
			} break;
			case POTENTIAL:
			{
				while ((number_of_devices>0)&&(no_undecided||no_accepted))
				{
					if ((ELECTRODE==(description=(*device)->description)->type)&&
						(!current_region||(description->region==current_region))&&
						(signal=(*device)->signal)&&(buffer=signal->buffer)&&
						(buffer->start<=potential_time)&&(potential_time<=buffer->end))
					{
						no_event=0;
						switch (signal->status)
						{
							case UNDECIDED:
							{
								no_undecided=0;
							} break;
							case ACCEPTED:
							{
								no_accepted=0;
							} break;
						}
					}
					device++;
					number_of_devices--;
				}
			} break;
			case INTEGRAL:
			{
				while ((number_of_devices>0)&&(no_undecided||no_accepted))
				{
					if ((ELECTRODE==(description=(*device)->description)->type)&&
						(!current_region||(description->region==current_region))&&
						(signal=(*device)->signal)&&(buffer=signal->buffer)&&
						(buffer->start<=start_search_interval)&&
						(start_search_interval<=end_search_interval)&&
						(end_search_interval<=buffer->end))
					{
						no_event=0;
						switch (signal->status)
						{
							case UNDECIDED:
							{
								no_undecided=0;
							} break;
							case ACCEPTED:
							{
								no_accepted=0;
							} break;
						}
					}
					device++;
					number_of_devices--;
				}
			} break;
		}
		if (no_event)
		{
			display_message(WARNING_MESSAGE,"No events to display");
		}
		else
		{
			if (no_undecided)
			{
				display_map(widget,analysis_work_area,(XtPointer)NULL);
			}
			else
			{
				if (!(warning_box_shell=analysis_window->display_map_warning_box_shell))
				{
					if ((!(parent=widget))||(True!=XtIsWidget(parent)))
					{
						parent=User_interface_get_application_shell(user_interface);
					}
					if (warning_box_shell=XtVaCreatePopupShell(
						"display_map_warning_shell",
						xmDialogShellWidgetClass,parent,
						XmNtitle,"Warning",
						NULL))
					{
						analysis_window->display_map_warning_box_shell=warning_box_shell;
						warning_box=(Widget)NULL;
						if (analysis_window_hierarchy_open&&
							(MrmSUCCESS==MrmFetchWidget(analysis_window_hierarchy,
							"event_times_warning_box",warning_box_shell,&warning_box,
							&warning_box_class)))
						{
							/*???SAB.  Adding this so that the busy cursor callback can get
								this value from the widget */
							XtVaSetValues(warning_box,
								XmNuserData,analysis->user_interface,
								NULL);
							analysis_window->display_map_warning_box=warning_box;
							/* add the accept all undecided callback */
							XtAddCallback(warning_box,XmNokCallback,display_map,
								analysis_work_area);
							analysis_window->display_map_warning_accept_butt=
								XmMessageBoxGetChild(warning_box,XmDIALOG_OK_BUTTON);
							/* add the reject all undecided callback */
							XtAddCallback(warning_box,XmNcancelCallback,display_map,
								analysis_work_area);
							analysis_window->display_map_warning_reject_butt=
								XmMessageBoxGetChild(warning_box,XmDIALOG_CANCEL_BUTTON);
							/* add the cancel callbacks */
							XtAddCallback(warning_box,XmNhelpCallback,
								close_display_map_warning,(XtPointer)warning_box);
							XtAddCallback(warning_box,XmNhelpCallback,
								busy_cursor_off_warning_box_shell,(XtPointer)analysis_window);
							/* add the destroy callbacks */
							XtAddCallback(warning_box,XmNdestroyCallback,
								busy_cursor_off_warning_box_shell,(XtPointer)analysis_window);
							XtAddCallback(warning_box,XmNdestroyCallback,destroy_window_shell,
								(XtPointer)create_Shell_list_item(&(analysis_window->
								display_map_warning_box_shell),
								analysis_window->user_interface));
						}
						else
						{
							XtDestroyWidget(warning_box_shell);
							warning_box_shell=(Widget)NULL;
							display_message(ERROR_MESSAGE,
								"display_map_with_check.  Could not retrieve warning box");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"display_map_with_check.  Could not create warning box shell");
					}
				}
				else
				{
					warning_box=analysis_window->display_map_warning_box;
				}
				if (warning_box_shell)
				{
					busy_cursor_on(warning_box_shell,analysis->user_interface);
					if (no_accepted)
					{
						XtUnmanageChild(analysis_window->display_map_warning_reject_butt);
					}
					else
					{
						XtManageChild(analysis_window->display_map_warning_reject_butt);
					}
					/* pop up the warning */
					XtManageChild(warning_box);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"display_map_with_check.  Invalid argument(s)");
	}
	LEAVE;
} /* display_map_with_check */

static void set_analysis_analysis_region(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 May 2003

DESCRIPTION :
Called when a new rig region is selected from the analysis window in the
analysis work area.
==============================================================================*/
{
	char found;
	int number_of_devices;
	struct Device **highlight;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Region *current_region;
	struct Region_list_item *region_item;
	Widget *region,selected_region;
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
	struct FE_field *device_name_field;
	struct FE_node *rig_node;
	struct FE_region *rig_node_group;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(set_analysis_analysis_region);
#if defined (UNEMAP_USE_3D)
	node_selection=(struct FE_node_selection *)NULL;
	device_name_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((analysis_window=analysis->window)&&(rig=analysis->rig))
		{
			/* determine the new region selected */
			selected_region=((XmRowColumnCallbackStruct *)call_data)->widget;
			if (selected_region!=analysis_window->current_region)
			{
				found=0;
				region=analysis_window->regions;
				/* check for "all regions" */
				if (analysis_window->number_of_regions>1)
				{
					if (selected_region== *region)
					{
						/* update the analysis rig */
						current_region=(struct Region *)NULL;
						found=1;
					}
					else
					{
						region++;
					}
				}
				if (!found)
				{
					region_item=get_Rig_region_list(rig);
					while (region_item&&(selected_region!= *region))
					{
						region_item=get_Region_list_item_next(region_item);
						region++;
					}
					if (region_item)
					{
						found=1;
						/* update the analysis rig */
						current_region=get_Region_list_item_region(region_item);
					}
				}
				if (found)
				{
					analysis_window->current_region=selected_region;
					set_Rig_current_region(rig,current_region);
					/* update the highlight device */
					if ((*(analysis->highlight))&&(current_region)&&
						((*(analysis->highlight))->description->region!=current_region))
					{
						/* pick the first highlighted device in the region */
						highlight=rig->devices;
						number_of_devices=rig->number_of_devices;
						while ((number_of_devices>0)&&
							(((*highlight)->description->region!=current_region)||
							!((*highlight)->highlight)))
						{
							number_of_devices--;
							highlight++;
						}
						if (number_of_devices<=0)
						{
							/* pick the first device in the region */
							highlight=rig->devices;
							number_of_devices=rig->number_of_devices;
							while ((number_of_devices>0)&&
								((*highlight)->description->region!=current_region))
							{
								number_of_devices--;
								highlight++;
							}
						}
						if (number_of_devices>0)
						{
/*???pre multiple select							(*(analysis->highlight))->highlight=0;*/
							analysis->highlight=highlight;
							(*highlight)->highlight=1;
							/* update the trace window */
							trace_change_signal(analysis->trace);
							/* update the analysis window */
							update_interval_drawing_area(analysis_window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_analysis_analysis_region.  Error updating highlight");
						}
					}
					else
					{
						/* update the trace window */
						trace_update_signal_controls(analysis->trace);
					}
					/* update the mapping window */
					if (mapping=analysis->mapping_window)
					{
						/* update the region choice in the mapping window */
						mapping->current_region=
							(mapping->regions)[region-analysis_window->regions];
						XtVaSetValues(mapping->region_choice,
							XmNmenuHistory,mapping->current_region,
							NULL);
#if defined (OLD_CODE)
						if ((map=mapping->map)&&!(map->fixed_range))
						{
							map->minimum_value=1;
							map->maximum_value=0;
						}
#endif /* defined (OLD_CODE) */
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
					}
					/* update the analysis window */
					update_signals_drawing_area(analysis_window);
					/* update the mapping_window_menu */
					update_mapping_window_menu(mapping);
					/* to recalculate RMS_signal. Perhaps should do trace_change_signal for all modes */
					if (*analysis->trace->analysis_mode==ELECTRICAL_IMAGING)
					{
						analysis->trace->calculate_rms=1;
						trace_change_signal(analysis->trace);
					}
#if defined (UNEMAP_USE_3D)
					/* must do this after analysis window has been updated */
					if ((number_of_devices>0)&&(analysis->highlight)&&(*(analysis->highlight)))
					{
						/* don't do a FE_node_selection_clear, as this will change analysis->highlight*/
						/* in rig_node_group_node_selection_change ??JW perhaps could cache it*/
						/* or change */
						/*get the rig_node corresponding to the device */
						node_selection=
							get_unemap_package_FE_node_selection(analysis->unemap_package);
						device_name_field=
							get_unemap_package_device_name_field(analysis->unemap_package);
						rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
						rig_node=find_rig_node_given_device(*(analysis->highlight),rig_node_group,
							device_name_field);
						/*trigger the selction callback*/
						FE_node_selection_select_node(node_selection,rig_node);
					}
#endif /* defined (UNEMAP_USE_3D) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_analysis_analysis_region.  Invalid region");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_analysis_analysis_region.  Missing analysis window or rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_analysis_region.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_analysis_analysis_region */

static int draw_all_markers(char draw_datum,char draw_potential_time,
	struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION : draws all_markers
==============================================================================*/
{
	int axes_left,axes_height,axes_top,axes_width,datum,drawing_height,
		drawing_width,end_analysis_interval,i,j,number_of_columns,number_of_devices,
		number_of_rows,potential_time,return_code,signal_index,
		start_analysis_interval,xpos,ypos;
	Pixmap pixel_map;
	struct Device **device,*highlight_device;
	struct Drawing_2d *drawing;
	struct Interval_area *interval;
	struct Region *current_region;
	struct Rig *rig;
	struct Signals_area *signals;
	struct Signal_buffer *buffer;
	Window drawing_area_window;

	ENTER(draw_all_markers);
	if (analysis)
	{
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			(buffer=get_Device_signal_buffer(*device)))
		{
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			datum=analysis->datum;
			potential_time=analysis->potential_time;
			if (analysis->window)
			{
				if (draw_potential_time&&(highlight_device= *(analysis->highlight)))
				{
					if (highlight_device->signal)
					{
						signal_index=highlight_device->signal->index;
					}
					else
					{
						signal_index= -1;
					}
					interval=get_Analysis_window_interval_area(analysis->window);
					draw_potential_time_marker(potential_time,buffer,
						highlight_device->channel,signal_index,INTERVAL_AREA_DETAIL,0,
						buffer->number_of_samples-1,interval->axes_left,interval->axes_top,
						interval->axes_width,interval->axes_height,
						XtWindow(interval->drawing_area),interval->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
				}
				signals= &(analysis->window->signals);
				drawing=signals->drawing;
				pixel_map=drawing->pixel_map;
				drawing_area_window=XtWindow(signals->drawing_area);
				drawing_width=drawing->width;
				drawing_height=drawing->height;
				number_of_columns=signals->number_of_columns;
				number_of_rows=signals->number_of_rows;
				/* draw event markers for each signal */
				number_of_devices=rig->number_of_devices;
				i=0;
				j=0;
				axes_left=signals->axes_left;
				axes_top=signals->axes_top;
				axes_width=signals->axes_width;
				axes_height=signals->axes_height;
				xpos=axes_left;
				ypos=axes_top;
				current_region=get_Rig_current_region(rig);
				while (number_of_devices>0)
				{
					if (!current_region||(current_region==(*device)->description->region))
					{
						draw_device_markers(*device,start_analysis_interval,
							end_analysis_interval,datum,draw_datum,potential_time,
							draw_potential_time,SIGNAL_AREA_DETAIL,0,xpos,ypos,axes_width,
							axes_height,drawing_area_window,pixel_map,
							analysis->signal_drawing_information,analysis->user_interface);
						i++;
						if (i>=number_of_rows)
						{
							i=0;
							ypos=axes_top;
							j++;
							xpos=axes_left+(j*drawing_width)/number_of_columns;
						}
						else
						{
							switch (signals->layout)
							{
								case SEPARATE_LAYOUT:
								{
									ypos=axes_top+(i*drawing_height)/number_of_rows;
								} break;
								case OVERLAP_LAYOUT:
								{
									ypos=axes_top+(i*drawing_height)/(number_of_rows+3);
								} break;
							}
						}
					}
					device++;
					number_of_devices--;
				}
			}
			trace_draw_markers(draw_datum,draw_potential_time,analysis->trace);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_all_markers.  signals missing or incomplete");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_all_markers */

static void set_detection_interval(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the detection algorithm to interval.
==============================================================================*/
{
	Dimension left_margin;
	enum Event_detection_algorithm detection_old;
	int widget_spacing;
	struct Analysis_work_area *analysis;
	struct Device *processed_device;
	struct Enlarge_area *enlarge;
	struct Signal_buffer *processed_buffer;
	Widget child_widget;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_detection_interval);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (EDA_INTERVAL!=analysis->detection)
		{
			widget_spacing=User_interface_get_widget_spacing(analysis->user_interface);
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->detection_choice,
					XmNmenuHistory,enlarge->detection.interval_button,
					NULL);
				XtVaSetValues(enlarge->calculate_button,
					XmNleftWidget,enlarge->number_of_events_form,
					NULL);
				child_widget=XmOptionLabelGadget(enlarge->datum_choice);
				XtVaGetValues(child_widget,
					XmNmarginLeft,&left_margin,
					NULL);
				if (left_margin>widget_spacing)
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->calculate_button,
						XmNleftOffset,0,
						NULL);
				}
				else
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->calculate_button,
						XmNleftOffset,widget_spacing-left_margin,
						NULL);
				}
				XtUnmanageChild(enlarge->threshold_scroll);
				XtUnmanageChild(enlarge->threshold_label);
				XtUnmanageChild(enlarge->minimum_separation_scroll);
				XtUnmanageChild(enlarge->minimum_separation_label);
				XtUnmanageChild(enlarge->all_current_choice);
				XtUnmanageChild(enlarge->level_value);
				XtManageChild(enlarge->number_of_events_form);
				XtManageChild(enlarge->objective_choice);
				enlarge->calculate_all_events=1;
				if (analysis->event_number>analysis->number_of_events)
				{
					analysis->event_number=analysis->number_of_events;
				}
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,False);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				False);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				False);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
			detection_old=analysis->detection;
			analysis->detection=EDA_INTERVAL;
			if (analysis->trace)
			{
				if (EDA_THRESHOLD!=detection_old)
				{
					if ((processed_device=analysis->trace->processed_device)&&
						(processed_device->signal)&&
						(processed_buffer=processed_device->signal->buffer)&&
						(processed_device->channel))
					{
						/* calculate objective function */
						calculate_device_objective(processed_device,analysis->detection,
							analysis->objective,((processed_buffer->signals).float_values)+
							(processed_device->signal->next->index),
							processed_buffer->number_of_samples,
							processed_buffer->number_of_signals,analysis->average_width);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_detection_interval.  Missing processed_device");
					}
				}
				trace_update_edit_interval(analysis->trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
				trace_update_signal_controls(analysis->trace);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_detection_interval.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_detection_interval */

static void set_detection_level(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the detection algorithm to level.
==============================================================================*/
{
	Dimension left_margin;
	int widget_spacing;
	struct Analysis_work_area *analysis;
	struct Device *processed_device;
	struct Enlarge_area *enlarge;
	struct Signal_buffer *processed_buffer;
	Widget child_widget;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_detection_level);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (EDA_LEVEL!=analysis->detection)
		{
			widget_spacing=User_interface_get_widget_spacing(analysis->user_interface);
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->detection_choice,
					XmNmenuHistory,enlarge->detection.level_button,
					NULL);
				XtVaSetValues(enlarge->calculate_button,
					XmNleftWidget,enlarge->level_value,
					NULL);
				child_widget=XmOptionLabelGadget(enlarge->datum_choice);
				XtVaGetValues(child_widget,
					XmNmarginLeft,&left_margin,
					NULL);
				if (left_margin>widget_spacing)
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->calculate_button,
						XmNleftOffset,0,
						NULL);
				}
				else
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->calculate_button,
						XmNleftOffset,widget_spacing-left_margin,
						NULL);
				}
				XtUnmanageChild(enlarge->threshold_scroll);
				XtUnmanageChild(enlarge->threshold_label);
				XtUnmanageChild(enlarge->minimum_separation_scroll);
				XtUnmanageChild(enlarge->minimum_separation_label);
				XtUnmanageChild(enlarge->all_current_choice);
				XtUnmanageChild(enlarge->number_of_events_form);
				XtUnmanageChild(enlarge->objective_choice);
				XtManageChild(enlarge->level_value);
				enlarge->calculate_all_events=1;
				if (analysis->event_number>analysis->number_of_events)
				{
					analysis->event_number=analysis->number_of_events;
				}
				/* clear the search and edit interval boxes */
				draw_search_box(enlarge->left_box,analysis->trace->area_1.axes_top,
					enlarge->right_box-enlarge->left_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->number_of_events,enlarge->divisions,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				draw_highlight_event_box(enlarge->left_edit_box,
					analysis->trace->area_1.axes_top,
					enlarge->right_edit_box-enlarge->left_edit_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
			analysis->detection=EDA_LEVEL;
			if (analysis->trace)
			{
				if ((processed_device=analysis->trace->processed_device)&&
					(processed_device->signal)&&
					(processed_buffer=processed_device->signal->buffer)&&
					(processed_device->channel))
				{
					/* calculate objective function */
					calculate_device_objective(processed_device,analysis->detection,
						analysis->objective,((processed_buffer->signals).float_values)+
						(processed_device->signal->next->index),
						processed_buffer->number_of_samples,
						processed_buffer->number_of_signals,analysis->average_width);
					if (True==XmToggleButtonGadgetGetState((analysis->trace->area_3).
						edit.objective_toggle))
					{
						redraw_trace_3_drawing_area((Widget)NULL,
							(XtPointer)(analysis->trace),(XtPointer)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_detection_level.  Missing processed_device");
				}
				/* draw the search and edit interval boxes */
				draw_search_box(enlarge->left_box,analysis->trace->area_1.axes_top,
					enlarge->right_box-enlarge->left_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->number_of_events,enlarge->divisions,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				draw_highlight_event_box(enlarge->left_edit_box,
					analysis->trace->area_1.axes_top,
					enlarge->right_edit_box-enlarge->left_edit_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				trace_update_signal_controls(analysis->trace);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_detection_level.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_detection_level */

static void set_detection_threshold(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the detection algorithm to threshold.
==============================================================================*/
{
	Dimension left_margin,right_margin;
	enum Event_detection_algorithm detection_old;
	int widget_spacing;
	struct Analysis_work_area *analysis;
	struct Device *processed_device;
	struct Enlarge_area *enlarge;
	struct Signal_buffer *processed_buffer;
	Widget child_widget;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_detection_threshold);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (EDA_THRESHOLD!=analysis->detection)
		{
			widget_spacing=User_interface_get_widget_spacing(analysis->user_interface);
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->detection_choice,
					XmNmenuHistory,enlarge->detection.threshold_button,
					NULL);
				XtVaSetValues(enlarge->calculate_button,
					XmNleftWidget,enlarge->minimum_separation_label,
					NULL);
				child_widget=XmOptionButtonGadget(enlarge->all_current_choice);
				XtVaGetValues(child_widget,
					XmNmarginRight,&right_margin,
					NULL);
				child_widget=XmOptionLabelGadget(enlarge->datum_choice);
				XtVaGetValues(child_widget,
					XmNmarginLeft,&left_margin,
					NULL);
				if (left_margin+right_margin>widget_spacing)
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->all_current_choice,
						XmNleftOffset,0,
						NULL);
				}
				else
				{
					XtVaSetValues(enlarge->datum_choice,
						XmNleftWidget,enlarge->all_current_choice,
						XmNleftOffset,widget_spacing-(left_margin+right_margin),
						NULL);
				}
				XtUnmanageChild(enlarge->level_value);
				XtUnmanageChild(enlarge->number_of_events_form);
				XtManageChild(enlarge->threshold_scroll);
				XtManageChild(enlarge->threshold_label);
				XtManageChild(enlarge->minimum_separation_scroll);
				XtManageChild(enlarge->minimum_separation_label);
				XtManageChild(enlarge->all_current_choice);
				XtManageChild(enlarge->objective_choice);
				enlarge->calculate_all_events=1;
				XtVaSetValues(enlarge->all_current_choice,
					XmNmenuHistory,enlarge->all_current.all_button,
					NULL);
				/* clear the search and edit interval boxes */
				draw_search_box(enlarge->left_box,analysis->trace->area_1.axes_top,
					enlarge->right_box-enlarge->left_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->number_of_events,enlarge->divisions,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				draw_highlight_event_box(enlarge->left_edit_box,
					analysis->trace->area_1.axes_top,
					enlarge->right_edit_box-enlarge->left_edit_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
			detection_old=analysis->detection;
			analysis->detection=EDA_THRESHOLD;
			if (analysis->trace)
			{
				if (EDA_INTERVAL!=detection_old)
				{
					if ((processed_device=analysis->trace->processed_device)&&
						(processed_device->signal)&&
						(processed_buffer=processed_device->signal->buffer)&&
						(processed_device->channel))
					{
						/* calculate objective function */
						calculate_device_objective(processed_device,analysis->detection,
							analysis->objective,((processed_buffer->signals).float_values)+
							(processed_device->signal->next->index),
							processed_buffer->number_of_samples,
							processed_buffer->number_of_signals,analysis->average_width);
						if (True==XmToggleButtonGadgetGetState((analysis->trace->area_3).
							edit.objective_toggle))
						{
							redraw_trace_3_drawing_area((Widget)NULL,
								(XtPointer)(analysis->trace),(XtPointer)NULL);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_detection_threshold.  Missing processed_device");
					}
				}
				/* draw the search and edit interval boxes */
				draw_search_box(enlarge->left_box,analysis->trace->area_1.axes_top,
					enlarge->right_box-enlarge->left_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->number_of_events,enlarge->divisions,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				draw_highlight_event_box(enlarge->left_edit_box,
					analysis->trace->area_1.axes_top,
					enlarge->right_edit_box-enlarge->left_edit_box,
					analysis->trace->area_1.axes_height,analysis->detection,
					analysis->trace->area_1.drawing_area,analysis->trace->area_1.drawing,
					analysis->signal_drawing_information);
				trace_update_signal_controls(analysis->trace);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_detection_threshold.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_detection_threshold */

static void set_objective_absolute_slope(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to absolute slope.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_absolute_slope);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (ABSOLUTE_SLOPE!=analysis->objective)
		{
			analysis->objective=ABSOLUTE_SLOPE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.absolute_slope_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_absolute_slope.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_absolute_slope */

static void set_objective_positive_slope(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to positive slope.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_positive_slope);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (POSITIVE_SLOPE!=analysis->objective)
		{
			analysis->objective=POSITIVE_SLOPE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.positive_slope_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_positive_slope.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_positive_slope */

static void set_objective_negative_slope(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to negative slope.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_negative_slope);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (NEGATIVE_SLOPE!=analysis->objective)
		{
			analysis->objective=NEGATIVE_SLOPE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.negative_slope_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_negative_slope.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_negative_slope */

static void set_objective_absolute_value(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to absolute value.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_absolute_value);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (ABSOLUTE_VALUE!=analysis->objective)
		{
			analysis->objective=ABSOLUTE_VALUE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.absolute_value_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_absolute_value.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_absolute_value */

static void set_objective_positive_value(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to positive value.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_positive_value);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (POSITIVE_VALUE!=analysis->objective)
		{
			analysis->objective=POSITIVE_VALUE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.positive_value_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_positive_value.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_positive_value */

static void set_objective_negative_value(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Sets the objective for the detection algorithm to negative value.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Enlarge_area *enlarge;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(set_objective_negative_value);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (NEGATIVE_VALUE!=analysis->objective)
		{
			analysis->objective=NEGATIVE_VALUE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.negative_value_button,
					NULL);
			}
			if ((analysis->trace)&&
				(processed_device=analysis->trace->processed_device)&&
				(processed_device->signal)&&(processed_device->signal->next)&&
				(processed_buffer=processed_device->signal->next->buffer))
			{
				/* calculate objective function */
				calculate_device_objective(processed_device,analysis->detection,
					analysis->objective,((processed_buffer->signals).float_values)+
					(processed_device->signal->next->index),
					processed_buffer->number_of_samples,
					processed_buffer->number_of_signals,analysis->average_width);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(analysis->trace),
					(XtPointer)NULL);
			}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			/* clear the present markers */
				/* ???signals area only ? */
			draw_all_markers(0,0,analysis);
			destroy_all_events(analysis->rig);
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_negative_value.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_negative_value */

static int file_analysis_write_signal_file(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	char *temp_string;
	int return_code;
	struct Analysis_work_area *analysis;
	struct Rig *analysis_rig;
	XmString new_dialog_title,old_dialog_title;

	ENTER(file_analysis_write_signal_file);
	return_code=0;
	/* check the arguments */
	analysis=(struct Analysis_work_area *)NULL;
	analysis_rig=(struct Rig *)NULL;
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_rig=analysis->rig))
	{
		if (return_code=analysis_write_signal_file(file_name,
			analysis_rig,analysis->datum,analysis->potential_time,
			analysis->start_search_interval,analysis->end_search_interval,
			analysis->calculate_events,analysis->detection,analysis->event_number,
			analysis->number_of_events,analysis->minimum_separation,
			analysis->threshold,analysis->datum_type,analysis->edit_order,
			analysis->signal_order,analysis->level,analysis->average_width))
		{
			if (!(analysis_rig->signal_file_name)||
				strcmp(analysis_rig->signal_file_name,file_name))
			{
				XtVaGetValues(analysis->window->window,
					XmNdialogTitle,&old_dialog_title,
					NULL);
				/* assign the signal file name */
				if (ALLOCATE(temp_string,char,strlen(file_name)+1))
				{
					DEALLOCATE(analysis_rig->signal_file_name);
					analysis_rig->signal_file_name=temp_string;
					strcpy(temp_string,file_name);
					/* unghost the write interval button */
					XtSetSensitive(analysis->window->file_menu.save_interval_button,
						True);
					/* unghost the overlay signals button */
					XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
						True);
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
"file_analysis_write_signal_file.  Could not allocate memory for signal file name");
				}
				/* set the analysis window title */
				if (ALLOCATE(temp_string,char,strlen(file_name)+12))
				{
					strcpy(temp_string,"Analysing: ");
					strcat(temp_string,file_name);
					new_dialog_title=XmStringCreateSimple(temp_string);
					DEALLOCATE(temp_string);
				}
				else
				{
					new_dialog_title=XmStringCreateSimple("Analysis");
					display_message(ERROR_MESSAGE,
"file_analysis_write_signal_file.  Could not allocate memory for window title");
				}
				XtVaSetValues(analysis->window->window,
					XmNdialogTitle,new_dialog_title,
					NULL);
				XmStringFree(old_dialog_title);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"file_analysis_write_signal_file.  "
			"Missing analysis_work_area (%p) or rig (%p)",analysis,analysis_rig);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_analysis_write_signal_file */

static void analysis_write_interval(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 November 2000

DESCRIPTION :
Called when the "Save interval" button is clicked.
==============================================================================*/
{
	FILE *temp_file;
	int write_file;
	struct Analysis_work_area *analysis;

	ENTER(analysis_write_interval);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->rig)&&(analysis->rig->signal_file_name))
	{
		if (temp_file=fopen(analysis->rig->signal_file_name,"r"))
		{
			fclose(temp_file);
			write_file=confirmation_warning_ok_cancel("Warning",
				"File already exists.  Overwrite ?",
#if defined (MOTIF)
				widget,
#endif /* defined (MOTIF) */
				analysis->user_interface);
		}
		else
		{
			write_file=1;
		}
		if (write_file)
		{
			file_analysis_write_signal_file(analysis->rig->signal_file_name,
				(void *)analysis);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_write_interval.  Missing analysis rig file name");
	}
	LEAVE;
} /* analysis_write_interval */

static int set_up_time_keeper_after_read(struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 16 May 2002

DESCRIPTION :
After a signal (or EDF or similar) file is loaded, sets up the time_keeper
to be consistant with it.
==============================================================================*/
{
	int potential_time,return_code,*times;
	float frequency;
	struct Signal_buffer *buffer;
	struct Rig *rig;

	ENTER(set_up_time_keeper_after_read);
	buffer=(struct Signal_buffer *)NULL;
	rig=(struct Rig *)NULL;
	times=(int *)NULL;
	if ((analysis)&&(rig=analysis->rig)&&(rig->devices)&&(*(rig->devices))&&
		(buffer=get_Device_signal_buffer(*(rig->devices))))
	{
		return_code=1;
		/*set the time keeper to the new current time. Important to keep any */
		/*movie player in sync */
		frequency=buffer->frequency;
		times=buffer->times;
		potential_time=analysis->potential_time;
		Time_keeper_request_new_time(Time_object_get_time_keeper(
			analysis->potential_time_object),((double)times[potential_time]/
			frequency));
		Time_keeper_set_minimum(Time_object_get_time_keeper(
			analysis->potential_time_object),
			(float)buffer->times[analysis->start_search_interval]/frequency);
		Time_keeper_set_maximum(Time_object_get_time_keeper(
			analysis->potential_time_object),
			(float)buffer->times[analysis->end_search_interval]/frequency);
		Time_keeper_set_speed(Time_object_get_time_keeper(
			analysis->potential_time_object),20.0/frequency);
		if ((analysis->mapping_window)&&(0<frequency)&&
			(analysis->mapping_window->time_editor_dialog))
		{
			time_editor_dialog_set_step(analysis->mapping_window->time_editor_dialog,
				1/frequency);
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"set_up_time_keeper_after_read.  Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* set_up_time_keeper_after_read */

static int clean_Analysis_work_area_before_load(
	struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 12 December 2001

DESCRIPTION :
cleans up <analysis> map and rig and node  things before loading a new
signal type file
==============================================================================*/
{
	struct Map *map;
	struct Signal_buffer *buffer;
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
#endif /* defined (UNEMAP_USE_3D) */
	int return_code;

	ENTER(clean_Analysis_work_area_before_load);
	map=(struct Map *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if (analysis)
	{
		/*don't want to animate during load*/
		mapping_window_stop_time_keeper(analysis->mapping_window);
		return_code=1;
#if defined (UNEMAP_USE_3D)
		/* need to unselect nodes, as selecting them accesses them */
		if (node_selection=get_unemap_package_FE_node_selection
			(analysis->unemap_package))
		{
			FE_node_selection_clear(node_selection);
		}
#endif /* defined (UNEMAP_USE_3D)	 */
		/* clear the old analysis */
		if (analysis->raw_rig)
		{
			if ((*(analysis->raw_rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->raw_rig->devices))))
			{
				destroy_Signal_buffer(&buffer);
			}
			destroy_Rig(&(analysis->raw_rig));
		}
		if (analysis->rig)
		{
			if ((*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
			{
				destroy_Signal_buffer(&buffer);
			}
#if defined (UNEMAP_USE_3D)
			if ((analysis->mapping_window)&&(analysis->mapping_window->map&&
				(analysis->mapping_window->map->drawing_information)))
			{
				map_remove_torso_arm_labels(analysis->mapping_window->map->drawing_information);
			}
#endif /* defined (UNEMAP_USE_3D)*/
#if defined (UNEMAP_USE_NODES)
			/* remove nodes from window, so can remove from rig */
			if (analysis->window)
			{
				analysis_Window_free_rig_node_order_info(analysis->window);
			}
#endif /* defined (UNEMAP_USE_NODES)*/
			destroy_Rig(&(analysis->rig));
#if defined (UNEMAP_USE_NODES)
			if (analysis->signal_drawing_package)
			{
				DEACCESS(Signal_drawing_package)(&(analysis->signal_drawing_package));
			}
#endif /* defined (UNEMAP_USE_NODES)*/
#if defined (UNEMAP_USE_3D)
			free_unemap_package_time_computed_fields(analysis->unemap_package);
			free_unemap_package_rig_fields(analysis->unemap_package);
#endif /* defined (UNEMAP_USE_NODES)*/
		}
		/* might not have events for ACIVATION maps, so reset to NO_MAP_FIELD*/
		/*perhaps we should look for events after signal file is loaded?*/
		if ((analysis->map_type==SINGLE_ACTIVATION)||(analysis->map_type==MULTIPLE_ACTIVATION))
		{
			analysis->map_type=NO_MAP_FIELD;
			if ((analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->colour_option=HIDE_COLOUR;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
			}
		}
		if (analysis->mapping_window)
		{
			XtSetSensitive(analysis->mapping_window->animate_button,False);
			if (map=analysis->mapping_window->map)
			{
				map->activation_front= -1;
				Map_flush_cache(analysis->mapping_window->map);
#if defined (UNEMAP_USE_3D)
				if (map->drawing_information)
				{
					set_map_drawing_information_electrodes_accepted_or_rejected(
						map->drawing_information,1);
					set_map_drawing_information_viewed_scene(map->drawing_information,0);
				}
#endif /* defined (UNEMAP_USE_NODES)*/
			}/* if (map=analysis->mapping_window->map)*/
		}/* if (analysis->mapping_window)*/
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"clean_Analysis_work_area_before_load.  Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* clean_Analysis_work_area_before_load */

static int analysis_work_area_read_signal_file(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Sets up the analysis work area for analysing a set of signals.
==============================================================================*/
{
	char calculate_events,*temp_string,value_string[10];
	enum Datum_type datum_type;
	enum Edit_order edit_order;
	enum Event_detection_algorithm detection;
	enum Signal_order signal_order;
	float level;
	int analysis_information,average_width,datum,end_search_interval,event_number,
		minimum_separation,number_of_events,potential_time,return_code,
		start_search_interval,threshold;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;
	XmString new_dialog_title,old_dialog_title,value_xmstring;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *field,*highlight_field;
	struct FE_field_component component;
#endif /* defined (UNEMAP_USE_NODES) */
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
	struct FE_field *device_name_field;
	struct FE_node *rig_node;
	struct FE_region *rig_node_group;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(analysis_work_area_read_signal_file);
	return_code=0;
#if defined (UNEMAP_USE_3D)
	node_selection=(struct FE_node_selection *)NULL;
	device_name_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	/* check the arguments */
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		clean_Analysis_work_area_before_load(analysis);
		/* initialize the new analysis */
		analysis->datum=0;
		analysis->potential_time=0;
		analysis->highlight=(struct Device **)NULL;
		/* get the analysis window title */
		XtVaGetValues(analysis->window->window,
			XmNdialogTitle,&old_dialog_title,
			NULL);
		/* open the input file */
		analysis_information=0;
		level=analysis->level;
		average_width=analysis->average_width;
		if ((return_code=analysis_read_signal_file(file_name,&(analysis->rig),
			&analysis_information,&datum,&calculate_events,&detection,
			&event_number,&number_of_events,&potential_time,&minimum_separation,
			&threshold,&datum_type,&edit_order,&signal_order,&start_search_interval,
			&end_search_interval,&level,&average_width
#if defined (UNEMAP_USE_3D)
			,analysis->unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
			))&&(rig=analysis->rig)&&(rig->devices)&&(*(rig->devices))&&
			(buffer=get_Device_signal_buffer(*(rig->devices))))
		{
			/* read the event detection settincgs */
			if (analysis_information)
			{
				return_code=1;
				/* update the analysis work area */
				switch (detection)
				{
					case EDA_INTERVAL:
					{
						set_detection_interval((Widget)NULL,(XtPointer)analysis,
							(XtPointer)NULL);
					} break;
					case EDA_LEVEL:
					{
						set_detection_level((Widget)NULL,(XtPointer)analysis,
							(XtPointer)NULL);
					} break;
					case EDA_THRESHOLD:
					{
						set_detection_threshold((Widget)NULL,(XtPointer)analysis,
							(XtPointer)NULL);
					} break;
				}
				if (trace=analysis->trace)
				{
					/* set the all/current choice */
					if (calculate_events&&!(trace->area_1.enlarge.calculate_all_events))
					{
						XtVaSetValues(trace->area_1.enlarge.all_current_choice,
							XmNmenuHistory,trace->area_1.enlarge.all_current.current_button,
							NULL);
						trace->area_1.enlarge.calculate_all_events=0;
					}
					else
					{
						if (!calculate_events&&
							(trace->area_1.enlarge.calculate_all_events))
						{
							XtVaSetValues(trace->area_1.enlarge.all_current_choice,
								XmNmenuHistory,trace->area_1.enlarge.all_current.all_button,
								NULL);
							trace->area_1.enlarge.calculate_all_events=1;
						}
					}
					/* set the threshold */
					if (threshold!=analysis->threshold)
					{
						sprintf(value_string,"%3d%%",threshold);
						value_xmstring=XmStringCreateSimple(value_string);
						XtVaSetValues(trace->area_1.enlarge.threshold_label,
							XmNlabelString,value_xmstring,
							NULL);
						XmStringFree(value_xmstring);
						XtVaSetValues(trace->area_1.enlarge.threshold_scroll,
							XmNvalue,threshold,
							NULL);
					}
					/* set the minimum separation */
					if (minimum_separation!=analysis->minimum_separation)
					{
						sprintf(value_string,"%3d ms",minimum_separation);
						value_xmstring=XmStringCreateSimple(value_string);
						XtVaSetValues(trace->area_1.enlarge.minimum_separation_label,
							XmNlabelString,value_xmstring,
							NULL);
						XmStringFree(value_xmstring);
						XtVaSetValues(trace->area_1.enlarge.minimum_separation_scroll,
							XmNvalue,minimum_separation,
							NULL);
					}
					/* set the level */
					if (level!=analysis->level)
					{
						sprintf(value_string,"%g",level);
						XtVaSetValues(trace->area_1.enlarge.level_value,
							XmNvalue,value_string,
							NULL);
					}
					/* set the level width */
					if (average_width!=analysis->average_width)
					{
						sprintf(value_string,"%d",average_width);
						XtVaSetValues(trace->menu.average_width_txt,
							XmNvalue,value_string,
							NULL);
					}
					/* set the datum type */
					if (datum_type!=analysis->datum_type)
					{
						switch (datum_type)
						{
							case AUTOMATIC_DATUM:
							{
								XtVaSetValues(trace->area_1.enlarge.datum_choice,
									XmNmenuHistory,trace->area_1.enlarge.datum.automatic_button,
									NULL);
							} break;
							case FIXED_DATUM:
							{
								XtVaSetValues(trace->area_1.enlarge.datum_choice,
									XmNmenuHistory,trace->area_1.enlarge.datum.fixed_button,
									NULL);
							} break;
						}
					}
					/* set the edit order */
					if (edit_order!=analysis->edit_order)
					{
						switch (edit_order)
						{
							case DEVICE_ORDER:
							{
								XtVaSetValues(trace->area_3.edit.order_choice,
									XmNmenuHistory,trace->area_3.edit.order.device_button,
									NULL);
							} break;
							case BEAT_ORDER:
							{
								XtVaSetValues(trace->area_3.edit.order_choice,
									XmNmenuHistory,trace->area_3.edit.order.beat_button,
									NULL);
							} break;
						}
					}
					/* set the number of events */
					if (number_of_events!=analysis->number_of_events)
					{
						sprintf(value_string,"%1d",number_of_events);
						value_xmstring=XmStringCreateSimple(value_string);
						XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
							XmNlabelString,value_xmstring,
							NULL);
						XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
							XmNlabelString,value_xmstring,
							NULL);
						XmStringFree(value_xmstring);
						if (1==number_of_events)
						{
							XtUnmanageChild(
								trace->area_1.enlarge.number_of_events.down_arrow);
							XtUnmanageChild(
								trace->area_1.beat_averaging.number_of_beats.down_arrow);
						}
						else
						{
							XtManageChild(
								trace->area_1.enlarge.number_of_events.down_arrow);
							XtManageChild(
								trace->area_1.beat_averaging.number_of_beats.down_arrow);
						}
						if (MAX_EVENTS==number_of_events)
						{
							XtUnmanageChild(
								trace->area_1.enlarge.number_of_events.up_arrow);
							XtUnmanageChild(
								trace->area_1.beat_averaging.number_of_beats.up_arrow);
						}
						else
						{
							XtManageChild(
								trace->area_1.enlarge.number_of_events.up_arrow);
							XtManageChild(
								trace->area_1.beat_averaging.number_of_beats.up_arrow);
						}
					}
				}
				analysis->threshold=threshold;
				analysis->minimum_separation=minimum_separation;
				analysis->level=level;
				analysis->average_width=average_width;
				analysis->datum_type=datum_type;
				analysis->edit_order=edit_order;
				analysis->number_of_events=number_of_events;
				if (analysis->window)
				{
					/* set whether or not the activation map can be drawn */
					if (calculate_events)
					{
						XtSetSensitive(
							analysis->window->map_menu.single_activation_button,True);
						XtSetSensitive(
							analysis->window->map_menu.multiple_activation_button,True);
						XtSetSensitive(
							analysis->window->map_menu.activation_potential_button,True);
						XtSetSensitive(analysis->window->file_menu.save_times_button,
							True);
					}
					else
					{
						XtSetSensitive(
							analysis->window->map_menu.single_activation_button,False);
						XtSetSensitive(
							analysis->window->map_menu.multiple_activation_button,False);
						XtSetSensitive(
							analysis->window->map_menu.activation_potential_button,False);
						XtSetSensitive(analysis->window->file_menu.save_times_button,
							False);
					}
					/* set the signal order */
					if (signal_order!=analysis->signal_order)
					{
						switch (signal_order)
						{
							case EVENT_ORDER:
							{
								XtVaSetValues(analysis->window->order_choice,
									XmNmenuHistory,analysis->window->order.event_button,
									NULL);
							} break;
							case CHANNEL_ORDER:
							{
								XtVaSetValues(analysis->window->order_choice,
									XmNmenuHistory,analysis->window->order.channel_button,
									NULL);
							} break;
						}
					}
				}
				analysis->calculate_events=calculate_events;
				analysis->signal_order=signal_order;
				analysis->datum=datum;
				analysis->event_number=event_number;
				analysis->potential_time=potential_time;
				analysis->start_search_interval=start_search_interval;
				analysis->end_search_interval=end_search_interval;
				DEALLOCATE(analysis->search_interval_divisions);
				/* set the edit interval */
				if (trace)
				{
					switch (detection)
					{
						case EDA_INTERVAL:
						{
							trace_update_edit_interval(analysis->trace);
						} break;
						case EDA_LEVEL:
						case EDA_THRESHOLD:
						{
							trace->area_3.edit.first_data=start_search_interval;
							trace->area_3.edit.last_data=end_search_interval;
						} break;
					}
				}
			}
			else
			{
				return_code=1;
				if (analysis->window)
				{
					XtSetSensitive(analysis->window->map_menu.single_activation_button,
						False);
					XtSetSensitive(
						analysis->window->map_menu.multiple_activation_button,False);
					XtSetSensitive(
						analysis->window->map_menu.activation_potential_button,False);
					XtSetSensitive(analysis->window->file_menu.save_times_button,False);
				}
				/* initialize the search interval */
				analysis->start_search_interval=buffer->start;
				analysis->end_search_interval=buffer->end;
				DEALLOCATE(analysis->search_interval_divisions);
				/* initialize the potential time */
				analysis->potential_time=((buffer->end)-(buffer->start))/3;
				/* initialize the datum */
				analysis->datum=2*(analysis->potential_time);
			}
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"analysis_work_area_read_signal_file.  Invalid file: %s",file_name);
		}
		/* set the start and end frame times based on the potential time */
		if ((analysis->rig)&&(analysis->rig->devices)&&
			(*(analysis->rig->devices))&&
			(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
			(analysis->mapping_window)&&(map=analysis->mapping_window->map))
		{
			map->start_time=
				(float)((buffer->times)[analysis->potential_time])*1000./
				(buffer->frequency);
			map->end_time=map->start_time;
		}
#if defined (UNEMAP_USE_3D)
		/* convert the loaded rig to nodes/elements/fields */
		if (convert_rig_to_nodes(analysis->rig))
		{
#if defined (OLD_CODE)
/*???DB.  No matching DEACCESS */
			/* same as rig->unemap_package */
			ACCESS(Unemap_package)(analysis->unemap_package);
#endif /* defined (OLD_CODE) */
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"analysis_work_area_read_signal_file.  convert_rig_to_nodes failed ");
		}
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_NODES)
		if (return_code)
		{
			/* create the signal_drawing_package, store it, set it up */
			if (!analysis->signal_drawing_package)
			{
				analysis->signal_drawing_package=CREATE(Signal_drawing_package)();
				ACCESS(Signal_drawing_package)(analysis->signal_drawing_package);
			}
			field=get_unemap_package_device_name_field(analysis->unemap_package);
			set_Signal_drawing_package_device_name_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_device_type_field(analysis->unemap_package);
			set_Signal_drawing_package_device_type_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_channel_number_field(analysis->unemap_package);
			set_Signal_drawing_package_channel_number_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_display_start_time_field(
				analysis->unemap_package);
			set_Signal_drawing_package_display_start_time_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_display_end_time_field(analysis->unemap_package);
			set_Signal_drawing_package_display_end_time_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_read_order_field(analysis->unemap_package);
			set_Signal_drawing_package_read_order_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_highlight_field(analysis->unemap_package);
			set_Signal_drawing_package_highlight_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_signal_field(analysis->unemap_package);
			set_Signal_drawing_package_signal_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_channel_offset_field(analysis->unemap_package);
			set_Signal_drawing_package_channel_offset_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_channel_gain_field(analysis->unemap_package);
			set_Signal_drawing_package_channel_gain_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_signal_minimum_field(analysis->unemap_package);
			set_Signal_drawing_package_signal_minimum_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_signal_maximum_field(analysis->unemap_package);
			set_Signal_drawing_package_signal_maximum_field(
				analysis->signal_drawing_package,field);
			field=get_unemap_package_signal_status_field(analysis->unemap_package);
			set_Signal_drawing_package_signal_status_field(
				analysis->signal_drawing_package,field);
			/* for the moment assuming only one signal per node, ie only one
				signal_field */
			/* highlight the first rig_node */
			highlight_field=(struct FE_field *)NULL;
			rig_node=(struct FE_node *)NULL;
			rig_node_group=(struct FE_region *)NULL;
			if ((rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig))&&
				(rig_node=FE_region_get_first_FE_node_that(rig_node_group,
					(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL)) &&
				(highlight_field=get_Signal_drawing_package_highlight_field(
					analysis->signal_drawing_package)))
			{
				analysis->highlight_rig_node=rig_node;
				component.number=0;
				component.field=highlight_field;
				/*??JW should be copying out of and into node with MANAGER_MODIFY */
				set_FE_nodal_int_value(rig_node,&component,/*version*/0,FE_NODAL_VALUE,
					/*time*/0,1/*highlight*/);
			}
		} /* if (return_code) */
#endif /* defined (UNEMAP_USE_NODES) */
		if (return_code)
		{
			/*highlight the first device*/
			if (analysis->highlight=analysis->rig->devices)
			{
				(*(analysis->highlight))->highlight=1;
			}
			/* assign the signal file name */
			if (ALLOCATE(analysis->rig->signal_file_name,char,strlen(file_name)+1))
			{
				strcpy(analysis->rig->signal_file_name,file_name);
				/* unghost the write interval button */
				XtSetSensitive(analysis->window->file_menu.save_interval_button,True);
				/* unghost the overlay signals button */
				XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			}
			else
			{
				display_message(ERROR_MESSAGE,"analysis_work_area_read_signal_file.  "
					"Could not allocate memory for signal file name");
			}
			/* set the analysis window title */
			if (ALLOCATE(temp_string,char,strlen(file_name)+12))
			{
				strcpy(temp_string,"Analysing: ");
				strcat(temp_string,file_name);
				new_dialog_title=XmStringCreateSimple(temp_string);
				DEALLOCATE(temp_string);
			}
			else
			{
				new_dialog_title=XmStringCreateSimple("Analysis");
				display_message(ERROR_MESSAGE,"analysis_work_area_read_signal_file.  "
					"Could not allocate memory for window title");
			}
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			if (analysis->mapping_window)
			{
				/* unghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,True);
			}
			/* unghost the write interval buttons */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,True);
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,True);
			/* unghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			/* ghost the reset button */
			XtSetSensitive(analysis->window->interval.reset_button,False);
			/* unghost the set baseline button */
			XtSetSensitive(analysis->window->interval.baseline_button,True);
			/* unghost the set range button */
			XtSetSensitive(analysis->window->interval.range_button,True);
			/* unghost the signal_range */
			XtSetSensitive(analysis->window->interval.signal_range,True);
			/* unghost the display integral map button */
			XtSetSensitive(analysis->window->map_menu.integral_button,True);
			/* unghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,True);
			/* unghost the print selected signals button */
			XtSetSensitive(analysis->window->print_menu.selected_button,True);
			/* unghost the print all signals button */
			XtSetSensitive(analysis->window->print_menu.all_button,True);
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,&(analysis->analysis_mode),
				&(analysis->detection),&(analysis->objective),&(analysis->datum_type),
				&(analysis->edit_order),&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
				&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
				&(analysis->rig),&(analysis->signal_drawing_package),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				User_interface_get_screen_width(analysis->user_interface),
				User_interface_get_screen_height(analysis->user_interface),
				analysis->signal_drawing_information,analysis->user_interface,
				&(analysis->first_eimaging_event)))
			{
				display_message(ERROR_MESSAGE,
					"analysis_work_area_read_signal_file.  Could not open trace window");
			}
		}
		else
		{
			if (analysis->rig)
			{
				if ((*(analysis->rig->devices))&&
					(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
				{
					destroy_Signal_buffer(&buffer);
				}
				destroy_Rig(&(analysis->rig));
			}
			if (analysis->mapping_window)
			{
				/* ghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,False);
			}
			/* set the analysis window title */
			new_dialog_title=XmStringCreateSimple("Analysis");
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			/* ghost the write interval buttons */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,False);
			/* ghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,False);
			/* ghost the write event times button */
			XtSetSensitive(analysis->window->file_menu.save_times_button,False);
			/* ghost the reset button */
			XtSetSensitive(analysis->window->interval.reset_button,False);
			/* ghost the set baseline button */
			XtSetSensitive(analysis->window->interval.baseline_button,False);
			/* ghost the set range button */
			XtSetSensitive(analysis->window->interval.range_button,False);
			/* ghost the signal_range */
			XtSetSensitive(analysis->window->interval.signal_range,False);
			/* ghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,False);
			/* ghost the display single activation map button */
			XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
			/* ghost the display multiple activation map button */
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				False);
			/* ghost the display activation potential map button */
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				False);
			/* ghost the print selected signals button */
			XtSetSensitive(analysis->window->print_menu.selected_button,False);
			/* ghost the print all signals button */
			XtSetSensitive(analysis->window->print_menu.all_button,False);
			/* close the trace window */
			if (analysis->trace)
			{
				XtPopdown(analysis->trace->shell);
				analysis->trace->open=0;
			}
		}
		update_analysis_window_menu(analysis->window);
		update_analysis_window_buffer_range_menu(analysis->window);
		update_mapping_window_menu(analysis->mapping_window);
		/* ensure projection_type matches region type */
		ensure_projection_type_matches_region_type(analysis);
		/* update the drawing areas */
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		/*set the time keeper to the new current time. Important to keep any */
		/*movie player in sync */
		set_up_time_keeper_after_read(analysis);
		update_mapping_drawing_area(analysis->mapping_window,2);
		update_mapping_colour_or_auxili(analysis->mapping_window);
		/* free the old analysis window title */
		XmStringFree(old_dialog_title);
#if defined (UNEMAP_USE_3D)
		/* highlight the  node (and everything else) */
		if ((analysis->highlight)&&(*(analysis->highlight)))
		{
			/* get the rig_node corresponding to the device */
			node_selection=get_unemap_package_FE_node_selection(
				analysis->unemap_package);
			device_name_field=get_unemap_package_device_name_field(
				analysis->unemap_package);
			rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
			rig_node=find_rig_node_given_device(*(analysis->highlight),rig_node_group,
				device_name_field);
			/*trigger the selction callback*/
			FE_node_selection_select_node(node_selection,rig_node);
		}
#endif /* defined (UNEMAP_USE_3D) */
		mapping_window_set_animation_buttons(analysis->mapping_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_work_area_read_signal_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_work_area_read_signal_file */

static int read_event_times_file(char *file_name,void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Sets up the analysis work area for analysing a previously analysed set of
signals.
==============================================================================*/
{
	char *detection_name,*device_line,*device_line_position,*device_name_end,
		end_of_line,*signal_file_name,*temp_string,value_string[10];
	FILE *input_file,*signal_input_file;
	float end_search_float,frequency,level,start_search_float,temp_float;
	int analysis_number_of_events,average_width,bisect_left,bisect_right,datum,
		comma_separated_table,device_line_length,device_name_length,
		end_search_interval,event_number,event_time,i,minimum_separation,
		next_accepted_event_number,next_accepted_event_time,number_of_devices,
		number_of_events,previous_accepted_event_number,
		previous_accepted_event_time,return_code,signal_read_cancelled,
		start_search_interval,threshold,*times;
	struct Analysis_work_area *analysis;
	struct Event *event,**event_next,*next_event;
	struct Device **device;
	struct Map *map;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title,value_xmstring;
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
	struct FE_field *device_name_field;
	struct FE_node *rig_node;
	struct FE_region *rig_node_group;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(read_event_times_file);
	return_code=0;
	signal_read_cancelled=0;
#if defined (UNEMAP_USE_3D)
	node_selection=(struct FE_node_selection *)NULL;
	device_name_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		if (input_file=fopen(file_name,"r"))
		{
			/* read the signal file name */
			fscanf(input_file,"signal file name : ");
			if (read_string(input_file,"[^ \n]",&signal_file_name))
			{
				fscanf(input_file,"\n");
				clean_Analysis_work_area_before_load(analysis);
				/* initialize the new analysis */
				analysis->datum=0;
				analysis->potential_time=0;
				analysis->highlight=(struct Device **)NULL;
				/* get the analysis window title */
				XtVaGetValues(analysis->window->window,XmNdialogTitle,&old_dialog_title,
					NULL);
				if ((signal_input_file=fopen(signal_file_name,"rb"))&&
					read_signal_file(signal_input_file,&(analysis->rig)
#if defined (UNEMAP_USE_3D)
						,analysis->unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
						))
				{
					/* found the signal file at the location stored in the events file*/
					return_code=1;
				}
				else
				{
					/* prompt the user to search for a signal file */
					/*NOTE: they could load in the wrong one! */
					confirmation_information_ok("Warning!",
					"Can't find this Events File's Signal File. Please locate it.",
#if defined (MOTIF)
					(Widget)(NULL),
#endif /* defined (MOTIF) */
					analysis->user_interface);
					if (signal_file_name=confirmation_get_read_filename(
						analysis->signal_file_extension_read,analysis->user_interface))
					{
						if ((signal_input_file=fopen(signal_file_name,"rb"))&&
							read_signal_file(signal_input_file,&(analysis->rig)
#if defined (UNEMAP_USE_3D)
								,analysis->unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
															))
						{
							return_code=1;
						}
					}
					else
					{
						/*pressed cancel on confirmation_get_read_filename dialogue*/
						signal_read_cancelled=1;
					}
				}
				if (return_code)
				{
					rig=analysis->rig;
					rig->signal_file_name=signal_file_name;
					/* unghost the write interval button */
					XtSetSensitive(analysis->window->file_menu.save_interval_button,True);
					/* unghost the overlay signals button */
					XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
						True);
					if ((analysis->highlight=(device=rig->devices))&&(*device)&&
						(buffer=get_Device_signal_buffer(*device))&&(times=buffer->times))
					{
						frequency=buffer->frequency;
						(*device)->highlight=1;
						/* initialize the potential time */
						analysis->potential_time=buffer->start+
							(buffer->end-buffer->start)/3;
						/* start with all signals rejected */
						number_of_devices=rig->number_of_devices;
						for (i=number_of_devices;i>0;i--)
						{
							if ((*device)->signal)
							{
								(*device)->signal->status=REJECTED;
							}
							device++;
						}
						/* read the table format */
						if (read_string(input_file,"[^ \n]",&detection_name))
						{
							if (strcmp(detection_name,"table"))
							{
								comma_separated_table=0;
								fscanf(input_file," method : ");
							}
							else
							{
								comma_separated_table=1;
								fscanf(input_file,"%*[^\n]\n");
								fscanf(input_file,"detection method : ");
							}
							DEALLOCATE(detection_name);
							/* read the detection method */
							if (read_string(input_file,"[^ \n]",&detection_name))
							{
								fscanf(input_file,"\n");
								if (strcmp(detection_name,"interval")&&
									strcmp(detection_name,"search"))
								{
									if (strcmp(detection_name,"threshold"))
									{
										if (strcmp(detection_name,"level"))
										{
											return_code=0;
											display_message(ERROR_MESSAGE,
										"read_event_times_file.  Invalid detection algorithm : %s",
												detection_name);
										}
										else
										{
											set_detection_level((Widget)NULL,(XtPointer)analysis,
												(XtPointer)NULL);
											/* read the start search and end search sample numbers */
											if (2==fscanf(input_file,
												"search start : %f, search end : %f\n",
												&start_search_float,&end_search_float))
											{
												if (2==fscanf(input_file,"level : %f, width : %d\n",
													&level,&average_width))
												{
													if (average_width<1)
													{
														average_width=1;
													}
													if (analysis->trace)
													{
														/* set the level */
														if (level!=analysis->level)
														{
															sprintf(value_string,"%g",level);
															XtVaSetValues(
																analysis->trace->area_1.enlarge.level_value,
																XmNvalue,value_string,
																NULL);
														}
														/* set the level width */
														if (average_width!=analysis->average_width)
														{
															sprintf(value_string,"%d",average_width);
															XtVaSetValues(
																analysis->trace->menu.average_width_txt,
																XmNvalue,value_string,
																NULL);
														}
													}
													analysis->level=level;
													analysis->average_width=average_width;
													/* read the number of activations being input */
													if (1!=fscanf(input_file,"number of events : %d\n\n",
														&number_of_events))
													{
														return_code=0;
														display_message(ERROR_MESSAGE,
											"read_event_times_file.  Error reading number of events");
													}
												}
												else
												{
													return_code=0;
													display_message(ERROR_MESSAGE,
														"read_event_times_file.  Error reading level");
												}
											}
											else
											{
												return_code=0;
												display_message(ERROR_MESSAGE,
											"read_event_times_file.  Error reading search start/end");
											}
										}
									}
									else
									{
										set_detection_threshold((Widget)NULL,(XtPointer)analysis,
											(XtPointer)NULL);
										/* read the start search and end search sample numbers */
										if (2==fscanf(input_file,
											"search start : %f, search end : %f\n",
											&start_search_float,&end_search_float))
										{
											if (2==fscanf(input_file,
												"threshold : %d%%, minimum separation : %d ms\n",
												&threshold,&minimum_separation))
											{
												if (analysis->trace)
												{
													/* set the threshold */
													if (threshold!=analysis->threshold)
													{
														sprintf(value_string,"%3d%%",threshold);
														value_xmstring=XmStringCreateSimple(value_string);
														XtVaSetValues(
															analysis->trace->area_1.enlarge.threshold_label,
															XmNlabelString,value_xmstring,
															NULL);
														XmStringFree(value_xmstring);
														XtVaSetValues(
															analysis->trace->area_1.enlarge.threshold_scroll,
															XmNvalue,threshold,
															NULL);
													}
													/* set the minimum separation */
													if (minimum_separation!=analysis->minimum_separation)
													{
														sprintf(value_string,"%3d ms",minimum_separation);
														value_xmstring=XmStringCreateSimple(value_string);
														XtVaSetValues(analysis->trace->area_1.enlarge.
															minimum_separation_label,
															XmNlabelString,value_xmstring,
															NULL);
														XmStringFree(value_xmstring);
														XtVaSetValues(analysis->trace->area_1.enlarge.
															minimum_separation_scroll,
															XmNvalue,minimum_separation,
															NULL);
													}
												}
												analysis->threshold=threshold;
												analysis->minimum_separation=minimum_separation;
												/* read the number of activations being input */
												if (1!=fscanf(input_file,"number of events : %d\n\n",
													&number_of_events))
												{
													return_code=0;
													display_message(ERROR_MESSAGE,
											"read_event_times_file.  Error reading number of events");
												}
											}
											else
											{
												return_code=0;
												display_message(ERROR_MESSAGE,
					"read_event_times_file.  Error reading threshold/minimum separation");
											}
										}
										else
										{
											return_code=0;
											display_message(ERROR_MESSAGE,
											"read_event_times_file.  Error reading search start/end");
										}
									}
								}
								else
								{
									set_detection_interval((Widget)NULL,(XtPointer)analysis,
										(XtPointer)NULL);
									if (strcmp(detection_name,"search"))
									{
										fscanf(input_file,"search");
									}
									/* read the start search and end search sample numbers */
									if (2==fscanf(input_file," start : %f, search end : %f",
										&start_search_float,&end_search_float))
									{
										if (1!=fscanf(input_file,", search events : %d\n",
											&analysis_number_of_events))
										{
											analysis_number_of_events= -1;
											fscanf(input_file,"\n");
										}
										/* read the number of activations being input */
										if (1==fscanf(input_file,"number of events : %d\n\n",
											&number_of_events))
										{
											/* set the number of events */
											if ((analysis_number_of_events>0)&&
												(analysis_number_of_events!=analysis->number_of_events))
											{
												if (analysis->trace)
												{
													sprintf(value_string,"%1d",analysis_number_of_events);
													value_xmstring=XmStringCreateSimple(value_string);
													XtVaSetValues(analysis->trace->area_1.enlarge.
														number_of_events.label,
														XmNlabelString,value_xmstring,
														NULL);
													XtVaSetValues(analysis->trace->area_1.beat_averaging.
														number_of_beats.label,
														XmNlabelString,value_xmstring,
														NULL);
													XmStringFree(value_xmstring);
													if (1==analysis_number_of_events)
													{
														XtUnmanageChild(analysis->trace->area_1.enlarge.
															number_of_events.down_arrow);
														XtUnmanageChild(analysis->trace->area_1.
															beat_averaging.number_of_beats.down_arrow);
													}
													else
													{
														XtManageChild(analysis->trace->area_1.enlarge.
															number_of_events.down_arrow);
														XtManageChild(analysis->trace->area_1.
															beat_averaging.number_of_beats.down_arrow);
													}
													if (MAX_EVENTS==analysis_number_of_events)
													{
														XtUnmanageChild(analysis->trace->area_1.enlarge.
															number_of_events.up_arrow);
														XtUnmanageChild(analysis->trace->area_1.
															beat_averaging.number_of_beats.up_arrow);
													}
													else
													{
														XtManageChild(analysis->trace->area_1.enlarge.
															number_of_events.up_arrow);
														XtManageChild(analysis->trace->area_1.
															beat_averaging.number_of_beats.up_arrow);
													}
												}
												analysis->number_of_events=analysis_number_of_events;
											}
										}
										else
										{
											return_code=0;
											display_message(ERROR_MESSAGE,
										"read_event_times_file.  Error reading number of events");
										}
									}
									else
									{
										return_code=0;
										display_message(ERROR_MESSAGE,
											"read_event_times_file.  Error reading search start/end");
									}
								}
								DEALLOCATE(detection_name);
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"read_event_times_file.  Error reading detection algorithm");
							}
						}
						else
						{
							return_code=0;
							display_message(ERROR_MESSAGE,
								"read_event_times_file.  Error reading table format");
						}
						if (return_code)
						{
							/* determine the start and end of the search interval */
							start_search_interval=(int)(start_search_float*frequency/1000.);
							end_search_interval=(int)(end_search_float*frequency/1000.);
							if ((start_search_interval<end_search_interval)&&
								(start_search_interval<times[buffer->end])&&
								(times[buffer->start]<end_search_interval))
							{
								bisect_left=buffer->start;
								bisect_right=buffer->end;
								while (bisect_left+1<bisect_right)
								{
									i=(bisect_left+bisect_right)/2;
									if (times[i]<start_search_interval)
									{
										bisect_left=i;
									}
									else
									{
										bisect_right=i;
									}
								}
								if (times[bisect_left]<start_search_interval)
								{
									analysis->start_search_interval=bisect_right;
								}
								else
								{
									analysis->start_search_interval=bisect_left;
								}
								bisect_left=buffer->start;
								bisect_right=buffer->end;
								while (bisect_left+1<bisect_right)
								{
									i=(bisect_left+bisect_right)/2;
									if (times[i]<end_search_interval)
									{
										bisect_left=i;
									}
									else
									{
										bisect_right=i;
									}
								}
								if (times[bisect_left]<end_search_interval)
								{
									analysis->end_search_interval=bisect_right;
								}
								else
								{
									analysis->end_search_interval=bisect_left;
								}
							}
							else
							{
								analysis->start_search_interval=buffer->start;
								analysis->end_search_interval=buffer->end;
							}
							DEALLOCATE(analysis->search_interval_divisions);
							/* read the heading line */
							read_string(input_file,"[^\n]",&device_line);
							DEALLOCATE(device_line);
							fscanf(input_file,"%c",&end_of_line);
							/* read the device names and event times */
							if (comma_separated_table)
							{
								while (return_code&&(read_string(input_file,"[^\n]",
									&device_line))&&strncmp("Reference",device_line,9))
								{
									fscanf(input_file,"%c",&end_of_line);
									/* determine the device */
									device_line_position=device_line;
									if (temp_string=strchr(device_line_position,','))
									{
										*temp_string='\0';
									}
									device=rig->devices;
									i=number_of_devices;
									while ((i>0)&&strcmp((*device)->description->name,
										device_line_position))
									{
										device++;
										i--;
									}
									if ((i>0)&&
										!(((*device)->signal)&&((*device)->signal->first_event)))
									{
										(*device)->signal->status=ACCEPTED;
										/* read the events */
										event_next= &((*device)->signal->first_event);
										event=(struct Event *)NULL;
										event_number=1;
										device_line_position=device_line+11;
										while (return_code&&
											(event_number<=number_of_events)&&event_next&&temp_string)
										{
											device_line_position=temp_string+1;
											if (temp_string=strchr(device_line_position,','))
											{
												*temp_string='\0';
											}
											if (strncmp(device_line_position,"none",4))
											{
												if (strncmp(device_line_position,"n/a",3))
												{
													if (1==sscanf(device_line_position,"%f",&temp_float))
													{
														event_time=(int)(temp_float*frequency/1000);
														bisect_left=buffer->start;
														bisect_right=buffer->end;
														if ((times[bisect_left]<event_time)&&
															(event_time<=times[bisect_right]))
														{
															while (bisect_left+1<bisect_right)
															{
																i=(bisect_left+bisect_right)/2;
																if (times[i]<event_time)
																{
																	bisect_left=i;
																}
																else
																{
																	bisect_right=i;
																}
															}
															if (times[bisect_left]<event_time)
															{
																event_time=bisect_right;
															}
															else
															{
																event_time=bisect_left;
															}
															if (*event_next=create_Event(event_time,
																event_number,ACCEPTED,event,
																(struct Event *)NULL))
															{
																event= *event_next;
																event_next= &(event->next);
															}
															else
															{
																return_code=0;
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_event_times_file.  Invalid event time : %s",
															event_time);
														return_code=0;
													}
												}
												else
												{
													if (*event_next=create_Event(0,event_number,
														REJECTED,event,(struct Event *)NULL))
													{
														if (event)
														{
															(*event_next)->time=event->time;
														}
														event= *event_next;
														event_next= &(event->next);
													}
													else
													{
														return_code=0;
													}
												}
											}
											event_number++;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_event_times_file.  Invalid device : %s",
											device_line_position);
									}
									DEALLOCATE(device_line);
								}
							}
							else
							{
								while (return_code&&(read_string(input_file,"[^\n]",
									&device_line))&&strncmp("Reference",device_line,9))
								{
									fscanf(input_file,"%c",&end_of_line);
									/* determine the device */
									device_line_position=device_line;
									if ((device_line_length=strlen(device_line))>10)
									{
										device_name_end=device_line_position+10;
									}
									else
									{
										device_name_end=device_line_position+device_line_length;
									}
									*device_name_end='\0';
									while (isspace(*device_line_position))
									{
										device_line_position++;
									}
									device_name_length=device_name_end-device_line_position;
									while ((device_name_end>device_line_position)&&
										(isspace(*(device_name_end-1))))
									{
										device_name_end--;
									}
									*device_name_end='\0';
									device=rig->devices;
									i=number_of_devices;
									while ((i>0)&&strncmp((*device)->description->name,
										device_line_position,device_name_length))
									{
										device++;
										i--;
									}
									if ((device_name_length>0)&&(i>0)&&
										!(((*device)->signal)&&((*device)->signal->first_event)))
									{
										(*device)->signal->status=ACCEPTED;
										/* read the events */
										event_next= &((*device)->signal->first_event);
										event=(struct Event *)NULL;
										event_number=1;
										device_line_position=device_line+11;
										while (return_code&&
											(event_number<=number_of_events)&&event_next&&
											(device_line_position-device_line<device_line_length))
										{
											if (strncmp(device_line_position,"none",4))
											{
												if (strncmp(device_line_position,"n/a",3))
												{
													if (1==sscanf(device_line_position,"%6f",&temp_float))
													{
														event_time=(int)(temp_float*frequency/1000);
														bisect_left=buffer->start;
														bisect_right=buffer->end;
														if ((times[bisect_left]<event_time)&&
															(event_time<=times[bisect_right]))
														{
															while (bisect_left+1<bisect_right)
															{
																i=(bisect_left+bisect_right)/2;
																if (times[i]<event_time)
																{
																	bisect_left=i;
																}
																else
																{
																	bisect_right=i;
																}
															}
															if (times[bisect_left]<event_time)
															{
																event_time=bisect_right;
															}
															else
															{
																event_time=bisect_left;
															}
															if (*event_next=create_Event(event_time,
																event_number,ACCEPTED,event,
																(struct Event *)NULL))
															{
																event= *event_next;
																event_next= &(event->next);
															}
															else
															{
																return_code=0;
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_event_times_file.  Invalid event time : %s",
															event_time);
														return_code=0;
													}
												}
												else
												{
													if (*event_next=create_Event(0,event_number,
														REJECTED,event,(struct Event *)NULL))
													{
														if (event)
														{
															(*event_next)->time=event->time;
														}
														event= *event_next;
														event_next= &(event->next);
													}
													else
													{
														return_code=0;
													}
												}
											}
											event_number++;
											device_line_position += 6;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_event_times_file.  Invalid device : ",
											device_line_position);
										return_code=0;
									}
									DEALLOCATE(device_line);
								}
							}
							/* calculate the times for rejected events */
							device=rig->devices;
							for (i=number_of_devices;i>0;i--)
							{
								if (((*device)->signal)&&
									(event=(*device)->signal->first_event)&&
									((*device)->signal->buffer))
								{
									previous_accepted_event_time=0;
									previous_accepted_event_number=0;
									while (event)
									{
										if (REJECTED==event->status)
										{
											while ((event->next)&&(REJECTED==event->next->status))
											{
												event=event->next;
											}
											if (next_event=event->next)
											{
												next_accepted_event_time=next_event->time;
												next_accepted_event_number=next_event->number;
											}
											else
											{
												next_accepted_event_time=
													((*device)->signal->buffer->number_of_samples)-1;
												next_accepted_event_number=(event->number)+1;
											}
											while (event&&(REJECTED==event->status))
											{
												event->time=previous_accepted_event_time+
													(((event->number)-previous_accepted_event_number)*
													(next_accepted_event_time-
													previous_accepted_event_time))/
													(next_accepted_event_number-
													previous_accepted_event_number);
												event=event->previous;
											}
											event=next_event;
										}
										else
										{
											previous_accepted_event_time=event->time;
											previous_accepted_event_number=event->number;
											event=event->next;
										}
									}
								}
								device++;
							}
							/* read the datum */
							if (return_code&&device_line&&
								!strncmp("Reference",device_line,9)&&
								(1==sscanf(device_line+9,"%f",&temp_float)))
							{
								datum=(int)(temp_float*frequency/1000);
								bisect_left=buffer->start;
								bisect_right=buffer->end;
								if ((times[bisect_left]<=datum)&&(datum<=times[bisect_right]))
								{
									while (bisect_left+1<bisect_right)
									{
										i=(bisect_left+bisect_right)/2;
										if (times[i]<datum)
										{
											bisect_left=i;
										}
										else
										{
											bisect_right=i;
										}
									}
									if (times[bisect_left]<datum)
									{
										analysis->datum=bisect_right;
									}
									else
									{
										analysis->datum=bisect_left;
									}
								}
								else
								{
									analysis->datum=2*(analysis->potential_time);
								}
							}
							else
							{
								if (return_code)
								{
									display_message(ERROR_MESSAGE,
										"read_event_times_file.  Error reading devices or datum");
									return_code=0;
									/* initialize the datum */
									analysis->datum=2*(analysis->potential_time);
								}
							}
							DEALLOCATE(device_line);
							/* unghost the single activation map button */
							XtSetSensitive(
								analysis->window->map_menu.single_activation_button,True);
							/* unghost the multiple activation map button */
							XtSetSensitive(
								analysis->window->map_menu.multiple_activation_button,True);
							/* unghost the activation potential map button */
							XtSetSensitive(
								analysis->window->map_menu.activation_potential_button,True);
							/* allow event times to be saved */
							XtSetSensitive(analysis->window->file_menu.save_times_button,
								True);
							analysis->calculate_events=1;
						}
						else
						{
							/* ghost the single activation map button */
							XtSetSensitive(
								analysis->window->map_menu.single_activation_button,False);
							/* ghost the multiple activation map button */
							XtSetSensitive(
								analysis->window->map_menu.multiple_activation_button,False);
							/* ghost the activation potential map button */
							XtSetSensitive(
								analysis->window->map_menu.activation_potential_button,False);
							/* no event times to be saved */
							XtSetSensitive(analysis->window->file_menu.save_times_button,
								False);
							analysis->calculate_events=0;
							/* initialize the search interval */
							analysis->start_search_interval=buffer->start;
							analysis->end_search_interval=buffer->end;
							DEALLOCATE(analysis->search_interval_divisions);
							/* initialize the datum */
							analysis->datum=2*(analysis->potential_time);
						}
						if (analysis->mapping_window)
						{
							/* unghost the mapping window file button */
							XtSetSensitive(analysis->mapping_window->file_button,True);
						}
						/* unghost the write interval buttons */
						XtSetSensitive(analysis->window->file_menu.save_interval_button,
							True);
						XtSetSensitive(analysis->window->file_menu.save_interval_as_button,
							True);
						/* unghost the overlay signals button */
						XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
							True);
						/* ghost the reset button */
						XtSetSensitive(analysis->window->interval.reset_button,False);
						/* unghost the set baseline button */
						XtSetSensitive(analysis->window->interval.baseline_button,True);
						/* unghost the set range button */
						XtSetSensitive(analysis->window->interval.range_button,True);
						/* unghost the signal_range */
						XtSetSensitive(analysis->window->interval.signal_range,True);
						/* unghost the display integral map button */
						XtSetSensitive(analysis->window->map_menu.integral_button,True);
						/* unghost the display potential map button */
						XtSetSensitive(analysis->window->map_menu.potential_button,True);
						/* unghost the print selected signals button */
						XtSetSensitive(analysis->window->print_menu.selected_button,True);
						/* unghost the print all signals button */
						XtSetSensitive(analysis->window->print_menu.all_button,True);
						/* open the trace window */
						if (!open_trace_window(&(analysis->trace),analysis->window_shell,
							analysis->identifying_colour,&(analysis->analysis_mode),
							&(analysis->detection),&(analysis->objective),
							&(analysis->datum_type),&(analysis->edit_order),
							&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
							&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
							&(analysis->rig),&(analysis->signal_drawing_package),
							&(analysis->datum),&(analysis->potential_time),
							&(analysis->event_number),&(analysis->number_of_events),
							&(analysis->threshold),&(analysis->minimum_separation),
							&(analysis->level),&(analysis->average_width),
							&(analysis->start_search_interval),
							&(analysis->search_interval_divisions),
							&(analysis->end_search_interval),
							User_interface_get_screen_width(analysis->user_interface),
							User_interface_get_screen_height(analysis->user_interface),
							analysis->signal_drawing_information,analysis->user_interface,
							&(analysis->first_eimaging_event)))
						{
							display_message(ERROR_MESSAGE,
								"read_event_times_file.  Could not open trace window");
						}
					}
					else
					{
						return_code=0;
					}
					/* set the analysis window title */
					if (ALLOCATE(temp_string,char,strlen(signal_file_name)+12))
					{
						strcpy(temp_string,"Analysing: ");
						strcat(temp_string,signal_file_name);
						new_dialog_title=XmStringCreateSimple(temp_string);
						DEALLOCATE(temp_string);
					}
					else
					{
						new_dialog_title=XmStringCreateSimple("Analysis");
						display_message(ERROR_MESSAGE,
					"read_event_times_file.  Could not allocate memory for window title");
					}
					XtVaSetValues(analysis->window->window,
						XmNdialogTitle,new_dialog_title,
						NULL);
					fclose(signal_input_file);
				}
				else
				{
					return_code=0;
					if (!signal_read_cancelled)
					{
						display_message(ERROR_MESSAGE,
							"read_event_times_file.  Invalid signal file: %s",signal_file_name);
					}
					DEALLOCATE(signal_file_name);
					/* set the analysis window title */
					new_dialog_title=XmStringCreateSimple("Analysis");
					XtVaSetValues(analysis->window->window,
						XmNdialogTitle,new_dialog_title,
						NULL);
					if (signal_input_file)
					{
						fclose(signal_input_file);
					}
					if (analysis->mapping_window)
					{
						/* ghost the mapping window file button */
						XtSetSensitive(analysis->mapping_window->file_button,False);
					}
					/* ghost the reset button */
					XtSetSensitive(analysis->window->interval.reset_button,False);
					/* ghost the set baseline button */
					XtSetSensitive(analysis->window->interval.baseline_button,False);
					/* ghost the set range button */
					XtSetSensitive(analysis->window->interval.range_button,False);
					/* ghost the signal_range */
					XtSetSensitive(analysis->window->interval.signal_range,False);
					/* ghost the display integral map button */
					XtSetSensitive(analysis->window->map_menu.integral_button,False);
					/* ghost the display potential map button */
					XtSetSensitive(analysis->window->map_menu.potential_button,False);
					/* ghost the display single activation map button */
					XtSetSensitive(analysis->window->map_menu.single_activation_button,
						False);
					/* ghost the display multiple activation map button */
					XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
						False);
					/* ghost the display activation potential map button */
					XtSetSensitive(analysis->window->map_menu.activation_potential_button,
						False);
					/* ghost the print selected signals button */
					XtSetSensitive(analysis->window->print_menu.selected_button,False);
					/* ghost the print all signals button */
					XtSetSensitive(analysis->window->print_menu.all_button,False);
					/* ghost the write interval buttons */
					XtSetSensitive(analysis->window->file_menu.save_interval_button,
						False);
					XtSetSensitive(analysis->window->file_menu.save_interval_as_button,
						False);
					/* ghost the overlay signals button */
					XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
						False);
					/* stop event times from being saved */
					XtSetSensitive(analysis->window->file_menu.save_times_button,False);
					/* close the trace window */
					if (analysis->trace)
					{
						XtPopdown(analysis->trace->shell);
					}
				}
				trace_change_rig(analysis->trace);
				update_analysis_window_menu(analysis->window);
				update_analysis_window_buffer_range_menu(analysis->window);
				update_mapping_window_menu(analysis->mapping_window);
				/* update the drawing areas */
				update_signals_drawing_area(analysis->window);
				update_interval_drawing_area(analysis->window);
				/*set the time keeper to the new current time. Important to keep any */
				/*movie player in sync */
				set_up_time_keeper_after_read(analysis);
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				trace_change_signal(analysis->trace);
				/* free the old analysis window title */
				XmStringFree(old_dialog_title);
#if defined (UNEMAP_USE_3D)
				/* read the signal file into nodes */
				/*???DB.  Would be better to be another callback from the same button ? */
				if (return_code&&signal_file_name)
				{
					/* convert the loaded rig to nodes/elements/fields */
					if (convert_rig_to_nodes(analysis->rig))
					{
#if defined (OLD_CODE)
/*???DB.  No matching DEACCESS */
						ACCESS(Unemap_package)(analysis->unemap_package);
#endif /* defined (OLD_CODE) */
						/* highlight the  node (and everything else) */
						if ((analysis->highlight)&&(*(analysis->highlight)))
						{
							/*get the rig_node corresponding to the device */
							node_selection=get_unemap_package_FE_node_selection(analysis->unemap_package);
							device_name_field=
								get_unemap_package_device_name_field(analysis->unemap_package);
							rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
							rig_node=find_rig_node_given_device(*(analysis->highlight),rig_node_group,
								device_name_field);
							/*trigger the selction callback*/
							FE_node_selection_select_node(node_selection,rig_node);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"read_event_times_file. convert_rig_to_nodes failed ");
					}
				}
#endif /* defined (UNEMAP_USE_3D) */

				mapping_window_set_animation_buttons(analysis->mapping_window);
				/* set the start and end frame times based on the potential time */
				if ((analysis->rig)&&(analysis->rig->devices)&&
					(*(analysis->rig->devices))&&
					(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
					(analysis->mapping_window)&&(map=analysis->mapping_window->map))
				{
					map->start_time=
						(float)((buffer->times)[analysis->potential_time])*1000./
						(buffer->frequency);
					map->end_time=map->start_time;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_event_times_file.  Error reading signal file name");
				return_code=0;
			}
			fclose(input_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_event_times_file.  Invalid file: %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_event_times_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_event_times_file */

static int analysis_read_bard_signal_file(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Reads in the signals from the Bard file and sets up the analysis work area for
analysing the signals.
==============================================================================*/
{
	char *temp_string;
	int return_code;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;

	ENTER(analysis_read_bard_signal_file);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area)&&
		bard_rig)
	{
		if (return_code=read_bard_signal_file(file_name,(void *)bard_rig))
		{
			clean_Analysis_work_area_before_load(analysis);
			/* initialize the new analysis */
			analysis->rig=bard_rig;
			bard_rig=(struct Rig *)NULL;
			analysis->datum=0;
			analysis->potential_time=0;
			analysis->highlight=(struct Device **)NULL;
			analysis->map_type=NO_MAP_FIELD;
			if (analysis->mapping_window)
			{
				XtSetSensitive(analysis->mapping_window->animate_button,False);
				if (map=analysis->mapping_window->map)
				{
					map->activation_front= -1;
					map->colour_option=HIDE_COLOUR;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				}
			}
			if (analysis->mapping_window)
			{
				/* unghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,True);
			}
			/* set the analysis window title */
			XtVaGetValues(analysis->window->window,
				XmNdialogTitle,&old_dialog_title,
				NULL);
			if (ALLOCATE(temp_string,char,strlen(file_name)+12))
			{
				strcpy(temp_string,"Analysing: ");
				strcat(temp_string,file_name);
				new_dialog_title=XmStringCreateSimple(temp_string);
				DEALLOCATE(temp_string);
			}
			else
			{
				new_dialog_title=XmStringCreateSimple("Analysis");
				display_message(ERROR_MESSAGE,
"analysis_read_bard_signal_file.  Could not allocate memory for window title");
			}
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			XmStringFree(old_dialog_title);
			/* ghost the write interval button */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
			/* unghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			/* unghost the write interval as button */
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,True);
			/* unghost the display integral map button */
			XtSetSensitive(analysis->window->map_menu.integral_button,True);
			/* unghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,True);
			if ((analysis->highlight=analysis->rig->devices)&&
				(*(analysis->highlight)))
			{
				(*(analysis->highlight))->highlight=1;
				if (buffer=get_Device_signal_buffer(*(analysis->highlight)))
				{
					/* initialize the search interval */
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					/* initialize potential time */
					analysis->potential_time=(buffer->number_of_samples-1)/3;
					/* initialize datum */
					analysis->datum=2*(analysis->potential_time);
				}
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
				(analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,&(analysis->analysis_mode),&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
				&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
				&(analysis->rig),&(analysis->signal_drawing_package),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				User_interface_get_screen_width(analysis->user_interface),
				User_interface_get_screen_height(analysis->user_interface),
				analysis->signal_drawing_information,analysis->user_interface,
				&(analysis->first_eimaging_event)))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_bard_signal_file.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
			update_analysis_window_buffer_range_menu(analysis->window);
			update_mapping_window_menu(analysis->mapping_window);
			/* update the drawing areas */
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_bard_signal_file.  Missing argument(s)");
		/* stop the file selection box from prompting for another file */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_bard_signal_file */

static int analysis_read_bard_electrode_fi(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 24 December 1996

DESCRIPTION :
Reads in the Bard electrode file, creates the rig and prompts the user for the
Bard signal file (window.dat).
==============================================================================*/
{
	int return_code;
	struct Analysis_work_area *analysis;
	Widget activation;

	ENTER(analysis_read_bard_electrode_fi);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		/* clear the existing Bard rig */
		destroy_Rig(&bard_rig);
		/* read the rig configuration from the electrode file */
		if (read_bard_electrode_file(file_name,&bard_rig))
		{
			/* prompt the user for the name of the signal file */
			if (!(analysis->bard_signal_file_data))
			{
				analysis->bard_signal_file_data=create_File_open_data(".dat",
					REGULAR,analysis_read_bard_signal_file,(void *)analysis,0,
					analysis->user_interface);
			}
			if (analysis->window)
			{
				activation=(analysis->window->file_menu).read_bard_data_button;
			}
			else
			{
				activation=(Widget)NULL;
			}
			open_file_and_read(activation,
				(XtPointer)(analysis->bard_signal_file_data),(XtPointer)NULL);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_bard_electrode_fi.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_bard_electrode_fi */

static int analysis_read_beekeeper_eeg_fil(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Reads in the signals from the Beekeeper file and sets up the analysis work area
for analysing the signals.
==============================================================================*/
{
	char *temp_string;
	int return_code;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;

	ENTER(analysis_read_beekeeper_eeg_fil);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		if (return_code=read_beekeeper_eeg_file(file_name,(void *)&rig))
		{
			clean_Analysis_work_area_before_load(analysis);
			/* initialize the new analysis */
			analysis->rig=rig;
			analysis->datum=0;
			analysis->potential_time=0;
			analysis->highlight=(struct Device **)NULL;
			analysis->map_type=NO_MAP_FIELD;
			if (analysis->mapping_window)
			{
				XtSetSensitive(analysis->mapping_window->animate_button,False);
				if (map=analysis->mapping_window->map)
				{
					map->activation_front= -1;
					map->colour_option=HIDE_COLOUR;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				}
			}
			if (analysis->mapping_window)
			{
				/* unghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,True);
			}
			/* set the analysis window title */
			XtVaGetValues(analysis->window->window,
				XmNdialogTitle,&old_dialog_title,
				NULL);
			if (ALLOCATE(temp_string,char,strlen(file_name)+12))
			{
				strcpy(temp_string,"Analysing: ");
				strcat(temp_string,file_name);
				new_dialog_title=XmStringCreateSimple(temp_string);
				DEALLOCATE(temp_string);
			}
			else
			{
				new_dialog_title=XmStringCreateSimple("Analysis");
				display_message(ERROR_MESSAGE,
"analysis_read_beekeeper_eeg_fil.  Could not allocate memory for window title");
			}
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			XmStringFree(old_dialog_title);
			/* ghost the write interval button */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
			/* unghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			/* unghost the write interval as button */
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,True);
			/* unghost the display integral map button */
			XtSetSensitive(analysis->window->map_menu.integral_button,True);
			/* unghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,True);
			if ((analysis->highlight=analysis->rig->devices)&&
				(*(analysis->highlight)))
			{
				(*(analysis->highlight))->highlight=1;
				if (buffer=get_Device_signal_buffer(*(analysis->highlight)))
				{
					/* initialize the search interval */
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					/* initialize potential time */
					analysis->potential_time=(buffer->number_of_samples-1)/3;
					/* initialize datum */
					analysis->datum=2*(analysis->potential_time);
				}
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
				(analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,&(analysis->analysis_mode),&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
				&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
				&(analysis->rig),&(analysis->signal_drawing_package),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				User_interface_get_screen_width(analysis->user_interface),
				User_interface_get_screen_height(analysis->user_interface),
				analysis->signal_drawing_information,analysis->user_interface,
				&(analysis->first_eimaging_event)))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_beekeeper_eeg_fil.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
			update_analysis_window_buffer_range_menu(analysis->window);
			update_mapping_window_menu(analysis->mapping_window);
			/* update the drawing areas */
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_beekeeper_eeg_fil.  Missing argument(s)");
		/* stop the file selection box from prompting for another file */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_beekeeper_eeg_fil */

static int analysis_read_bdf_or_edf_file(struct Analysis_work_area *analysis,
	int bdf)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Reads in the signals from the bdf or edf file and sets up the analysis work area
for analysing the signals.  <bdf>!=0 reads bdf files, else reads edf files
==============================================================================*/
{
	char *file_name,*temp_string,ext_str[5];
	int buffer_end,buffer_start,return_code;
	struct Map *map;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
	struct FE_field *device_name_field;
	struct FE_node *rig_node;
	struct FE_region *rig_node_group;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(analysis_read_bdf_or_edf_file);
	return_code=0;
#if defined (UNEMAP_USE_3D)
	node_selection=(struct FE_node_selection *)NULL;
	device_name_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	if (analysis)
	{
		if (bdf)
		{
			strcpy(ext_str,".bdf");
		}
		else
		{
			strcpy(ext_str,".edf");
		}
		/* read the edf or bdf file name */
		if (file_name=confirmation_get_read_filename(ext_str,
			analysis->user_interface))
		{
			return_code=1;
		}
		else
		{
			/*pressed cancel on confirmation_get_read_filename dialogue*/
			return_code=0;
		}
		if (return_code)
		{
			clean_Analysis_work_area_before_load(analysis);
			/* initialize the new analysis */
			analysis->datum=0;
			analysis->potential_time=0;
			analysis->highlight=(struct Device **)NULL;
			/* get the analysis window title */
			XtVaGetValues(analysis->window->window,
				XmNdialogTitle,&old_dialog_title,
				NULL);
			/* open the input file */
			if (read_bdf_or_edf_file(file_name,&analysis->rig,analysis->user_interface
#if defined (UNEMAP_USE_3D)
				,analysis->unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
				,bdf)
				&&(rig=analysis->rig)&&(rig->devices)&&(*(rig->devices))&&
				(buffer=get_Device_signal_buffer(*(rig->devices))))
			{
				return_code=1;
				buffer_start=buffer->start;
				buffer_end=buffer->end;
				if (analysis->window)
				{
					XtSetSensitive(analysis->window->map_menu.single_activation_button,
						False);
					XtSetSensitive(
						analysis->window->map_menu.multiple_activation_button,False);
					XtSetSensitive(
						analysis->window->map_menu.activation_potential_button,False);
					XtSetSensitive(analysis->window->file_menu.save_times_button,False);
				}
				/* initialize the search interval */
				analysis->start_search_interval=buffer_start;
				analysis->end_search_interval=buffer_end;
				DEALLOCATE(analysis->search_interval_divisions);
				/* initialize the potential time */
				analysis->potential_time=(buffer_end-buffer_start)/3;
				/* initialize the datum */
				analysis->datum=2*(analysis->potential_time);
			}
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,"analysis_read_bdf_or_edf_file.  "
					"Invalid edf file: %s or cnfg file ",file_name);
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
				(analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
#if defined (UNEMAP_USE_3D)
			/* put the rig into nodes */
			/*???DB.  Would be better to be another callback from the same button ? */
			if (convert_rig_to_nodes(analysis->rig))
			{
#if defined (OLD_CODE)
/*???DB.  No matching DEACCESS */
				/* same as rig->unemap_package */
				ACCESS(Unemap_package)(analysis->unemap_package);
#endif /* defined (OLD_CODE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_bdf_or_edf_file. convert_rig_to_nodes  failed ");
			}
#endif /* defined (UNEMAP_USE_3D) */
			if (return_code)
			{
				/*highlight the first device*/
				if (analysis->highlight=analysis->rig->devices)
				{
					(*(analysis->highlight))->highlight=1;
				}
				/* assign the signal file name */
				if (ALLOCATE(analysis->rig->signal_file_name,char,strlen(file_name)+1))
				{
					strcpy(analysis->rig->signal_file_name,file_name);
					/* unghost the write interval button */
					XtSetSensitive(analysis->window->file_menu.save_interval_button,True);
					/* unghost the overlay signals button */
					XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
						True);
				}
				else
				{
					display_message(ERROR_MESSAGE,"analysis_read_bdf_or_edf_file.  "
						"Could not allocate memory for signal file name");
				}
				/* set the analysis window title */
				if (ALLOCATE(temp_string,char,strlen(file_name)+12))
				{
					strcpy(temp_string,"Analysing: ");
					strcat(temp_string,file_name);
					new_dialog_title=XmStringCreateSimple(temp_string);
					DEALLOCATE(temp_string);
				}
				else
				{
					new_dialog_title=XmStringCreateSimple("Analysis");
					display_message(ERROR_MESSAGE,"analysis_read_bdf_or_edf_file.  "
						"Could not allocate memory for window title");
				}
				XtVaSetValues(analysis->window->window,
					XmNdialogTitle,new_dialog_title,
					NULL);
				if (analysis->mapping_window)
				{
					/* unghost the mapping window file button */
					XtSetSensitive(analysis->mapping_window->file_button,True);
				}
				/* unghost the write interval buttons */
				XtSetSensitive(analysis->window->file_menu.save_interval_button,True);
				XtSetSensitive(analysis->window->file_menu.save_interval_as_button,
					True);
				/* unghost the overlay signals button */
				XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
				/* ghost the reset button */
				XtSetSensitive(analysis->window->interval.reset_button,False);
				/* unghost the set baseline button */
				XtSetSensitive(analysis->window->interval.baseline_button,True);
				/* unghost the set range button */
				XtSetSensitive(analysis->window->interval.range_button,True);
				/* unghost the signal_range */
				XtSetSensitive(analysis->window->interval.signal_range,True);
				/* unghost the display integral map button */
				XtSetSensitive(analysis->window->map_menu.integral_button,True);
				/* unghost the display potential map button */
				XtSetSensitive(analysis->window->map_menu.potential_button,True);
				/* unghost the print selected signals button */
				XtSetSensitive(analysis->window->print_menu.selected_button,True);
				/* unghost the print all signals button */
				XtSetSensitive(analysis->window->print_menu.all_button,True);
				trace_change_rig(analysis->trace);
				/* open the trace window */
				if (!open_trace_window(&(analysis->trace),analysis->window_shell,
					analysis->identifying_colour,&(analysis->analysis_mode),
					&(analysis->detection),&(analysis->objective),&(analysis->datum_type),
					&(analysis->edit_order),&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
					&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
					&(analysis->rig),&(analysis->signal_drawing_package),
					&(analysis->datum),&(analysis->potential_time),
					&(analysis->event_number),&(analysis->number_of_events),
					&(analysis->threshold),&(analysis->minimum_separation),
					&(analysis->level),&(analysis->average_width),
					&(analysis->start_search_interval),
					&(analysis->search_interval_divisions),
					&(analysis->end_search_interval),
					User_interface_get_screen_width(analysis->user_interface),
					User_interface_get_screen_height(analysis->user_interface),
					analysis->signal_drawing_information,analysis->user_interface,
					&(analysis->first_eimaging_event)))
				{
					display_message(ERROR_MESSAGE,
						"analysis_read_bdf_or_edf_file.  Could not open trace window");
				}
			}
			else
			{
				if (analysis->rig)
				{
					if ((*(analysis->rig->devices))&&
						(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
					{
						destroy_Signal_buffer(&buffer);
					}
					destroy_Rig(&(analysis->rig));
				}
				if (analysis->mapping_window)
				{
					/* ghost the mapping window file button */
					XtSetSensitive(analysis->mapping_window->file_button,False);
				}
				/* set the analysis window title */
				new_dialog_title=XmStringCreateSimple("Analysis");
				XtVaSetValues(analysis->window->window,
					XmNdialogTitle,new_dialog_title,
					NULL);
				/* ghost the write interval buttons */
				XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
				XtSetSensitive(analysis->window->file_menu.save_interval_as_button,
					False);
				/* ghost the overlay signals button */
				XtSetSensitive(analysis->window->file_menu.overlay_signals_button,
					False);
				/* ghost the write event times button */
				XtSetSensitive(analysis->window->file_menu.save_times_button,False);
				/* ghost the reset button */
				XtSetSensitive(analysis->window->interval.reset_button,False);
				/* ghost the set baseline button */
				XtSetSensitive(analysis->window->interval.baseline_button,False);
				/* ghost the set range button */
				XtSetSensitive(analysis->window->interval.range_button,False);
				/* ghost the signal_range */
				XtSetSensitive(analysis->window->interval.signal_range,False);
				/* ghost the display potential map button */
				XtSetSensitive(analysis->window->map_menu.potential_button,False);
				/* ghost the display single activation map button */
				XtSetSensitive(analysis->window->map_menu.single_activation_button,
					False);
				/* ghost the display multiple activation map button */
				XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
					False);
				/* ghost the display activation potential map button */
				XtSetSensitive(analysis->window->map_menu.activation_potential_button,
					False);
				/* ghost the print selected signals button */
				XtSetSensitive(analysis->window->print_menu.selected_button,False);
				/* ghost the print all signals button */
				XtSetSensitive(analysis->window->print_menu.all_button,False);
				/* close the trace window */
				if (analysis->trace)
				{
					XtPopdown(analysis->trace->shell);
					analysis->trace->open=0;
				}
			}
			update_analysis_window_menu(analysis->window);
			update_analysis_window_buffer_range_menu(analysis->window);
			update_mapping_window_menu(analysis->mapping_window);
			/* ensure projection_type matches region type */
			ensure_projection_type_matches_region_type(analysis);
			/* update the drawing areas */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			/*set the time keeper to the new current time. Important to keep any */
			/*movie player in sync */
			set_up_time_keeper_after_read(analysis);
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			/* free the old analysis window title */
			XmStringFree(old_dialog_title);
#if defined (UNEMAP_USE_3D)
			/* highlight the  node (and everything else) */
			if ((analysis->highlight)&&(*(analysis->highlight)))
			{
				/*get the rig_node corresponding to the device */
				node_selection=get_unemap_package_FE_node_selection(
					analysis->unemap_package);
				device_name_field=get_unemap_package_device_name_field(
					analysis->unemap_package);
				rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
				rig_node=find_rig_node_given_device(*(analysis->highlight),
					rig_node_group,device_name_field);
				/*trigger the selction callback*/
				FE_node_selection_select_node(node_selection,rig_node);
			}
#endif /* defined (UNEMAP_USE_3D) */
			mapping_window_set_animation_buttons(analysis->mapping_window);
		}/* if (return_code) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_bdf_or_edf_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_bdf_or_edf_file */

static void analysis_read_edf_file(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
Reads in the signals from the edf file and sets up the analysis work area
for analysing the signals.
==============================================================================*/
{
	struct Analysis_work_area *analysis;

	ENTER(analysis_read_edf_file);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	analysis=(struct Analysis_work_area *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		analysis_read_bdf_or_edf_file(analysis,0/*bdf*/);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_edf_file.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_read_edf_file */

static void analysis_read_bdf_file(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
Reads in the signals from the bdf file and sets up the analysis work area
for analysing the signals.
==============================================================================*/
{
	struct Analysis_work_area *analysis;

	ENTER(analysis_read_bdf_file);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	analysis=(struct Analysis_work_area *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		analysis_read_bdf_or_edf_file(analysis,1/*bdf*/);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_bdf_file.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_read_bdf_file */

static int analysis_read_neurosoft_sig_fil(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Reads in the signals from the Neurosoft file and sets up the analysis work area
for analysing the signals.
==============================================================================*/
{
	char *temp_string;
	int return_code;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;

	ENTER(analysis_read_neurosoft_sig_fil);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area)&&
		neurosoft_rig)
	{
		if (return_code=read_neurosoft_row_points_file(file_name,
			(void *)neurosoft_rig))
		{
			clean_Analysis_work_area_before_load(analysis);
			/* initialize the new analysis */
			analysis->rig=neurosoft_rig;
			neurosoft_rig=(struct Rig *)NULL;
			analysis->datum=0;
			analysis->potential_time=0;
			analysis->highlight=(struct Device **)NULL;
			analysis->map_type=NO_MAP_FIELD;
			if (analysis->mapping_window)
			{
				XtSetSensitive(analysis->mapping_window->animate_button,False);
				if (map=analysis->mapping_window->map)
				{
					map->activation_front= -1;
					map->colour_option=HIDE_COLOUR;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				}
			}
			if (analysis->mapping_window)
			{
				/* unghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,True);
			}
			/* set the analysis window title */
			XtVaGetValues(analysis->window->window,
				XmNdialogTitle,&old_dialog_title,
				NULL);
			if (ALLOCATE(temp_string,char,strlen(file_name)+12))
			{
				strcpy(temp_string,"Analysing: ");
				strcat(temp_string,file_name);
				new_dialog_title=XmStringCreateSimple(temp_string);
				DEALLOCATE(temp_string);
			}
			else
			{
				new_dialog_title=XmStringCreateSimple("Analysis");
				display_message(ERROR_MESSAGE,
"analysis_read_neurosoft_sig_fil.  Could not allocate memory for window title");
			}
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			XmStringFree(old_dialog_title);
			/* ghost the write interval button */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
			/* unghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			/* unghost the write interval as button */
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,True);
			/* unghost the display integral map button */
			XtSetSensitive(analysis->window->map_menu.integral_button,True);
			/* unghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,True);
			if ((analysis->highlight=analysis->rig->devices)&&
				(*(analysis->highlight)))
			{
				(*(analysis->highlight))->highlight=1;
				if (buffer=get_Device_signal_buffer(*(analysis->highlight)))
				{
					/* initialize the search interval */
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					/* initialize potential time */
					analysis->potential_time=(buffer->number_of_samples-1)/3;
					/* initialize datum */
					analysis->datum=2*(analysis->potential_time);
				}
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
				(analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,&(analysis->analysis_mode),&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
				&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
				&(analysis->rig),&(analysis->signal_drawing_package),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				User_interface_get_screen_width(analysis->user_interface),
				User_interface_get_screen_height(analysis->user_interface),
				analysis->signal_drawing_information,analysis->user_interface,
				&(analysis->first_eimaging_event)))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_neurosoft_sig_fil.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
			update_analysis_window_buffer_range_menu(analysis->window);
			update_mapping_window_menu(analysis->mapping_window);
			/* update the drawing areas */
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_neurosoft_sig_fil.  Missing argument(s)");
		/* stop the file selection box from prompting for another file */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_neurosoft_sig_fil */

static int analysis_read_neurosoft_electro(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 4 August 1997

DESCRIPTION :
Reads in configuration file for the Neurosoft rig, creates the rig and prompts
the user for the Neurosoft signal file.
==============================================================================*/
{
	int return_code;
	struct Analysis_work_area *analysis;
	Widget activation;

	ENTER(analysis_read_neurosoft_electro);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		/* clear the existing Neurosoft rig */
		destroy_Rig(&neurosoft_rig);
		/* read the rig configuration */
		if (read_configuration_file(file_name,&neurosoft_rig
#if defined (UNEMAP_USE_3D)
			,analysis->unemap_package
#endif /* defined (UNEMAP_USE_3D) */
				))
		{
			/* prompt the user for the name of the signal file */
			if (!(analysis->neurosoft_signal_file_data))
			{
				analysis->neurosoft_signal_file_data=create_File_open_data(".dat",
					REGULAR,analysis_read_neurosoft_sig_fil,(void *)analysis,0,
					analysis->user_interface);
			}
			if (analysis->window)
			{
				activation=(analysis->window->file_menu).read_neurosoft_data_button;
			}
			else
			{
				activation=(Widget)NULL;
			}
			open_file_and_read(activation,
				(XtPointer)(analysis->neurosoft_signal_file_data),(XtPointer)NULL);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_neurosoft_electro.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_neurosoft_electro */

static int analysis_read_cardiomapp_sig_fi(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
Reads in the signals from the CardioMapp file and sets up the analysis work area
for analysing the signals.
==============================================================================*/
{
	char *temp_string;
	int return_code;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;

	ENTER(analysis_read_cardiomapp_sig_fi);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area)&&
		cardiomapp_rig)
	{
		if (return_code=read_cardiomapp_file(file_name,(void *)cardiomapp_rig))
		{
			clean_Analysis_work_area_before_load(analysis);
			/* initialize the new analysis */
			analysis->rig=cardiomapp_rig;
			cardiomapp_rig=(struct Rig *)NULL;
			analysis->datum=0;
			analysis->potential_time=0;
			analysis->highlight=(struct Device **)NULL;
			analysis->map_type=NO_MAP_FIELD;
			if (analysis->mapping_window)
			{
				XtSetSensitive(analysis->mapping_window->animate_button,False);
				if (map=analysis->mapping_window->map)
				{
					map->activation_front= -1;
					map->colour_option=HIDE_COLOUR;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				}
			}
			if (analysis->mapping_window)
			{
				/* unghost the mapping window file button */
				XtSetSensitive(analysis->mapping_window->file_button,True);
			}
			/* set the analysis window title */
			XtVaGetValues(analysis->window->window,
				XmNdialogTitle,&old_dialog_title,
				NULL);
			if (ALLOCATE(temp_string,char,strlen(file_name)+12))
			{
				strcpy(temp_string,"Analysing: ");
				strcat(temp_string,file_name);
				new_dialog_title=XmStringCreateSimple(temp_string);
				DEALLOCATE(temp_string);
			}
			else
			{
				new_dialog_title=XmStringCreateSimple("Analysis");
				display_message(ERROR_MESSAGE,
"analysis_read_cardiomapp_sig_fi.  Could not allocate memory for window title");
			}
			XtVaSetValues(analysis->window->window,
				XmNdialogTitle,new_dialog_title,
				NULL);
			XmStringFree(old_dialog_title);
			/* ghost the write interval button */
			XtSetSensitive(analysis->window->file_menu.save_interval_button,False);
			/* unghost the overlay signals button */
			XtSetSensitive(analysis->window->file_menu.overlay_signals_button,True);
			/* unghost the write interval as button */
			XtSetSensitive(analysis->window->file_menu.save_interval_as_button,True);
			/* unghost the display integral map button */
			XtSetSensitive(analysis->window->map_menu.integral_button,True);
			/* unghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,True);
			if ((analysis->highlight=analysis->rig->devices)&&
				(*(analysis->highlight)))
			{
				(*(analysis->highlight))->highlight=1;
				if (buffer=get_Device_signal_buffer(*(analysis->highlight)))
				{
					/* initialize the search interval */
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					/* initialize potential time */
					analysis->potential_time=(buffer->number_of_samples-1)/3;
					/* initialize datum */
					analysis->datum=2*(analysis->potential_time);
				}
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
				(analysis->mapping_window)&&(map=analysis->mapping_window->map))
			{
				map->start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->end_time=map->start_time;
			}
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,&(analysis->analysis_mode),&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
				&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
				&(analysis->rig),&(analysis->signal_drawing_package),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				User_interface_get_screen_width(analysis->user_interface),
				User_interface_get_screen_height(analysis->user_interface),
				analysis->signal_drawing_information,analysis->user_interface,
				&(analysis->first_eimaging_event)))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_cardiomapp_sig_fi.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
			update_analysis_window_buffer_range_menu(analysis->window);
			update_mapping_window_menu(analysis->mapping_window);
			/* update the drawing areas */
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_cardiomapp_sig_fi.  Missing argument(s)");
		/* stop the file selection box from prompting for another file */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_cardiomapp_sig_fi */

static int analysis_read_cardiomapp_electr(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 23 August 1997

DESCRIPTION :
Reads in configuration file for the CardioMapp rig, creates the rig and prompts
the user for the CardioMapp signal file.
==============================================================================*/
{
	int return_code;
	struct Analysis_work_area *analysis;
	Widget activation;

	ENTER(analysis_read_cardiomapp_electr);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		/* clear the existing Neurosoft rig */
		destroy_Rig(&cardiomapp_rig);
		/* read the rig configuration */
		if (read_configuration_file(file_name,&cardiomapp_rig
#if defined (UNEMAP_USE_3D)
			,analysis->unemap_package
#endif /* defined (UNEMAP_USE_3D) */
			))
		{
			/* prompt the user for the name of the signal file */
			if (!(analysis->cardiomapp_signal_file_data))
			{
				analysis->cardiomapp_signal_file_data=create_File_open_data(".rdt",
					REGULAR,analysis_read_cardiomapp_sig_fi,(void *)analysis,0,
					analysis->user_interface);
			}
			if (analysis->window)
			{
				activation=(analysis->window->file_menu).read_cardiomapp_data_button;
			}
			else
			{
				activation=(Widget)NULL;
			}
			open_file_and_read(activation,
				(XtPointer)(analysis->cardiomapp_signal_file_data),(XtPointer)NULL);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_cardiomapp_electr.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_cardiomapp_electr */

static void select_signals_drawing_area(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 January 2000

DESCRIPTION :
The callback for selecting a device in the analysis work area (signals drawing
area, mapping drawing area, colour bar or auxiliary devices drawing area).
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	XmDrawingAreaCallbackStruct *callback;
	XButtonEvent *event;
	struct Signals_area *signals;
	int column_number,row_number,signal_number;

	ENTER(select_signals_drawing_area);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->window))
	{
		signals= &(analysis->window->signals);
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if (callback->event)
				{
					if (ButtonPress==callback->event->type)
					{
						event= &(callback->event->xbutton);
						if (signals->number_of_signals>0)
						{
							/* determine the signal number */
							switch (signals->layout)
							{
								case SEPARATE_LAYOUT:
								{
									row_number=((event->y)*(signals->number_of_rows))/
										(signals->drawing->height);
									column_number=((event->x)*(signals->number_of_columns))/
										(signals->drawing->width);
									signal_number=column_number*(signals->number_of_rows)+
										row_number;
								} break;
								case OVERLAP_LAYOUT:
								{
									row_number=((float)(event->y-(signals->signal_height)/2)*
										(float)(signals->number_of_rows+3))/
										(float)(signals->drawing->height)+0.5;
									if (row_number<0)
									{
										row_number=0;
									}
									else
									{
										if (row_number>=signals->number_of_rows)
										{
											row_number=signals->number_of_rows-1;
										}
									}
									column_number=((event->x)*(signals->number_of_columns))/
										(signals->drawing->width);
									signal_number=column_number*(signals->number_of_rows)+
										row_number;
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"select_signals_drawing_area.  Invalid signal layout");
									signal_number= -1;
								} break;
							}
							if ((signal_number>=0)&&
								(signal_number<signals->number_of_signals))
							{
#if defined (UNEMAP_USE_NODES)
								highlight_analysis_device_node((event->state)&ShiftMask,
									(struct FE_node *)NULL,&signal_number,(int *)NULL,
									(int *)NULL,analysis);
#else
								highlight_analysis_device((event->state)&ShiftMask,
									(struct Device **)NULL,&signal_number,(int *)NULL,
									(int *)NULL,analysis);
#endif /*  defined (UNEMAP_USE_NODES) */
								/* make sure that the trace window is open */
								open_trace_window(&(analysis->trace),analysis->window_shell,
									analysis->identifying_colour,&(analysis->analysis_mode),
									&(analysis->detection),&(analysis->objective),
									&(analysis->datum_type),&(analysis->edit_order),
									&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
									&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
									&(analysis->rig),&(analysis->signal_drawing_package),
									&(analysis->datum),
									&(analysis->potential_time),&(analysis->event_number),
									&(analysis->number_of_events),&(analysis->threshold),
									&(analysis->minimum_separation),&(analysis->level),
									&(analysis->average_width),&(analysis->start_search_interval),
									&(analysis->search_interval_divisions),
									&(analysis->end_search_interval),
									User_interface_get_screen_width(analysis->user_interface),
									User_interface_get_screen_height(analysis->user_interface),
									analysis->signal_drawing_information,
									analysis->user_interface,&(analysis->first_eimaging_event));
							}
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_signals_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_signals_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_signals_drawing_area.  analysis work area missing");
	}
	LEAVE;
} /* select_signals_drawing_area */

static void analysis_accept_event(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION : accepts the analysis event
==============================================================================*/
{
	float frequency;
	int end_analysis_interval,event_number,highlight_number,
		start_analysis_interval,*times,xpos,ypos;
	struct Analysis_work_area *analysis;
	struct Device **highlight;
	struct Event *event;
	struct Mapping_window *mapping;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;

	ENTER(analysis_accept_event);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((analysis->rig)&&(highlight=analysis->highlight)&&(*highlight)&&
			(signal=(*highlight)->signal)&&(event=signal->first_event)&&
			(buffer=signal->buffer)&&(times=buffer->times))
		{
			event_number=analysis->event_number;
			while (event&&(event->number<event_number))
			{
				event=event->next;
			}
			if (event&&(event->number==event_number)&&(event->status!=ACCEPTED))
			{
				frequency=buffer->frequency;
				start_analysis_interval=buffer->start;
				end_analysis_interval=buffer->end;
				/* clear the present event marker */
				if (analysis->window)
				{
					signals= &(analysis->window->signals);
					/* determine the number of the highlighted signal */
					highlight_number=highlight-(analysis->rig->devices);
					switch (signals->layout)
					{
						case SEPARATE_LAYOUT:
						{
							xpos=signals->axes_left+
								((highlight_number/(signals->number_of_rows))*
								(signals->drawing->width))/(signals->number_of_columns);
							ypos=signals->axes_top+
								((highlight_number%(signals->number_of_rows))*
								(signals->drawing->height))/(signals->number_of_rows);
						} break;
						case OVERLAP_LAYOUT:
						{
							xpos=signals->axes_left+
								((highlight_number/(signals->number_of_rows))*
								(signals->drawing->width))/(signals->number_of_columns);
							ypos=signals->axes_top+
								((highlight_number%(signals->number_of_rows))*
								(signals->drawing->height))/(signals->number_of_rows+3);
						} break;
					}
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				else
				{
					signals=(struct Signals_area *)NULL;
				}
				trace_draw_event_marker(event,analysis->trace);
				/* change the event status */
				event->status=ACCEPTED;
				/* draw the new event marker */
				if (signals)
				{
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				trace_draw_event_marker(event,analysis->trace);
			}
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&
				(mapping=analysis->mapping_window)&&(mapping->map)&&
				(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_label_type))
			{
				update_mapping_drawing_area(mapping,0);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_accept_event.  analysis work area missing");
	}
	LEAVE;
} /* analysis_accept_event */

static void analysis_previous_event(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION : moves to the previous analysis event
==============================================================================*/
{
	int event_number,i;
	struct Analysis_work_area *analysis;
	struct Device **devices,**highlight;
	struct Event *event;
	struct Rig *rig;

	ENTER(analysis_previous_event);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((rig=analysis->rig)&&(highlight=analysis->highlight))
		{
			if (((*highlight)->signal)&&(event=(*highlight)->signal->first_event))
			{
				event_number=analysis->event_number;
				while (event&&(event->number<event_number))
				{
					event=event->next;
				}
				/* accept the current event if undecided */
				if (event&&(event_number==event->number)&&
					(UNDECIDED==event->status))
				{
					analysis_accept_event(widget,analysis_work_area,call_data);
				}
			}
			/* move to the previous event */
			switch (analysis->edit_order)
			{
				case DEVICE_ORDER:
				{
					highlight--;
					i=highlight-(devices=rig->devices);
					while ((i>=0)&&(get_Rig_current_region(rig))&&
						(get_Rig_current_region(rig)!=(*highlight)->description->region))
					{
						highlight--;
						i--;
					}
					if (i<0)
					{
						i=(rig->number_of_devices)-1;
						highlight=devices+i;
						while ((i>0)&&(get_Rig_current_region(rig))&&
							(get_Rig_current_region(rig)!=(*highlight)->description->region))
						{
							highlight--;
							i--;
						}
						if (((*highlight)->signal)&&
							(event=(*highlight)->signal->first_event))
						{
							while ((event->next)&&
								(event->next->number<=analysis->event_number))
							{
								event=event->next;
							}
							if (event->number<analysis->event_number)
							{
								analysis->event_number=event->number;
							}
							else
							{
								(analysis->event_number)--;
							}
						}
						else
						{
							(analysis->event_number)--;
						}
						trace_update_edit_interval(analysis->trace);
					}
					highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
						(int *)NULL,analysis);
				} break;
				case BEAT_ORDER:
				{
					if (event&&(event->previous))
					{
						event=event->previous;
						analysis->event_number=event->number;
						trace_change_event(analysis->trace);
					}
					else
					{
						highlight--;
						devices=rig->devices;
						i=highlight-devices;
						while ((i>0)&&(get_Rig_current_region(rig))&&
							(get_Rig_current_region(rig)!=(*highlight)->description->region))
						{
							highlight--;
							i--;
						}
						if (((*highlight)->signal)&&
							(event=(*highlight)->signal->first_event))
						{
							while (event->next)
							{
								event=event->next;
							}
							analysis->event_number=event->number;
							trace_update_edit_interval(analysis->trace);
						}
						highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
							(int *)NULL,analysis);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"analysis_previous_event.  Invalid event order");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_previous_event.  analysis work area missing");
	}
	LEAVE;
} /* analysis_previous_event */

static void analysis_next_event(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 November 1993

DESCRIPTION : moves to the next analysis event
==============================================================================*/
{
	int event_number,i;
	struct Analysis_work_area *analysis;
	struct Device **devices,**highlight;
	struct Event *event;
	struct Rig *rig;

	ENTER(analysis_next_event);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((rig=analysis->rig)&&(highlight=analysis->highlight))
		{
			if (((*highlight)->signal)&&(event=(*highlight)->signal->first_event))
			{
				event_number=analysis->event_number;
				while (event&&(event->number<event_number))
				{
					event=event->next;
				}
				/* accept the current event if undecided */
				if (event&&(event_number==event->number)&&
					(UNDECIDED==event->status))
				{
					analysis_accept_event(widget,analysis_work_area,call_data);
				}
			}
			/* move to the next event */
			switch (analysis->edit_order)
			{
				case DEVICE_ORDER:
				{
					devices=rig->devices;
					highlight++;
					i=(rig->number_of_devices)-(highlight-devices)-1;
					while ((i>=0)&&(get_Rig_current_region(rig))&&
						(get_Rig_current_region(rig)!=(*highlight)->description->region))
					{
						highlight++;
						i--;
					}
					if (i<0)
					{
						i=(rig->number_of_devices)-1;
						highlight=devices;
						while ((i>0)&&(get_Rig_current_region(rig))&&
							(get_Rig_current_region(rig)!=(*highlight)->description->region))
						{
							highlight++;
							i--;
						}
						if (((*highlight)->signal)&&
							(event=(*highlight)->signal->first_event))
						{
							while ((event->next)&&
								(event->number<=analysis->event_number))
							{
								event=event->next;
							}
							if (event->number<=analysis->event_number)
							{
								(analysis->event_number)++;
							}
							else
							{
								analysis->event_number=event->number;
							}
						}
						else
						{
							(analysis->event_number)++;
						}
						trace_update_edit_interval(analysis->trace);
					}
					highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
						(int *)NULL,analysis);
				} break;
				case BEAT_ORDER:
				{
					if (event&&(event->next))
					{
						event=event->next;
						analysis->event_number=event->number;
						trace_change_event(analysis->trace);
					}
					else
					{
						highlight++;
						devices=rig->devices;
						i=(rig->number_of_devices)-(highlight-devices)-1;
						while ((i>0)&&(get_Rig_current_region(rig))&&
							(get_Rig_current_region(rig)!=(*highlight)->description->region))
						{
							highlight++;
							i--;
						}
						if (((*highlight)->signal)&&
							(event=(*highlight)->signal->first_event))
						{
							analysis->event_number=event->number;
							trace_update_edit_interval(analysis->trace);
						}
						highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
							(int *)NULL,analysis);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"analysis_next_event.  Invalid event order");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_next_event.  analysis work area missing");
	}
	LEAVE;
} /* analysis_next_event */

static void analysis_reject_event(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
???DB.  Should be in trace_window.c and do work on other windows via
trace_window callbacks.
==============================================================================*/
{
	float frequency;
	int event_number,end_analysis_interval,highlight_number,
		start_analysis_interval,*times,xpos,ypos;
	struct Analysis_work_area *analysis;
	struct Device **highlight;
	struct Event *event;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(analysis_reject_event);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((rig=analysis->rig)&&(highlight=analysis->highlight)&&(*highlight)&&
			(signal=(*highlight)->signal)&&(event=signal->first_event)&&
			(buffer=signal->buffer)&&(times=buffer->times))
		{
			event_number=analysis->event_number;
			while (event&&(event->number<event_number))
			{
				event=event->next;
			}
			if (event&&(event_number==event->number)&&
				(event->status!=REJECTED))
			{
				frequency=buffer->frequency;
				start_analysis_interval=buffer->start;
				end_analysis_interval=buffer->end;
				/* clear the present event marker */
				if (analysis->window)
				{
					signals= &(analysis->window->signals);
					/* determine the number of the highlighted signal */
					highlight_number=highlight-(rig->devices);
					switch (signals->layout)
					{
						case SEPARATE_LAYOUT:
						{
							xpos=signals->axes_left+
								((highlight_number/(signals->number_of_rows))*
								(signals->drawing->width))/(signals->number_of_columns);
							ypos=signals->axes_top+
								((highlight_number%(signals->number_of_rows))*
								(signals->drawing->height))/(signals->number_of_rows);
						} break;
						case OVERLAP_LAYOUT:
						{
							xpos=signals->axes_left+
								((highlight_number/(signals->number_of_rows))*
								(signals->drawing->width))/(signals->number_of_columns);
							ypos=signals->axes_top+
								((highlight_number%(signals->number_of_rows))*
								(signals->drawing->height))/(signals->number_of_rows+3);
						} break;
					}
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				else
				{
					signals=(struct Signals_area *)NULL;
				}
				if (analysis->trace)
				{
					trace_area_1= &(analysis->trace->area_1);
					trace_area_3= &(analysis->trace->area_3);
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,trace_area_1->axes_left,
						trace_area_1->axes_top,trace_area_1->axes_width,
						trace_area_1->axes_height,XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,trace_area_3->axes_left,
						trace_area_3->axes_top,trace_area_3->axes_width,
						trace_area_3->axes_height,XtWindow(trace_area_3->drawing_area),
						trace_area_3->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
				}
				else
				{
					trace_area_1=(struct Trace_window_area_1 *)NULL;
					trace_area_3=(struct Trace_window_area_3 *)NULL;
				}
				/* change the event status */
				event->status=REJECTED;
				/* draw the new event marker */
				if (signals)
				{
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				if (trace_area_1&&trace_area_3)
				{
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,trace_area_1->axes_left,
						trace_area_1->axes_top,trace_area_1->axes_width,
						trace_area_1->axes_height,XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,(*highlight)->signal_display_minimum,
						(*highlight)->signal_display_maximum,trace_area_3->axes_left,
						trace_area_3->axes_top,trace_area_3->axes_width,
						trace_area_3->axes_height,XtWindow(trace_area_3->drawing_area),
						trace_area_3->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
				}
				if (((SINGLE_ACTIVATION==analysis->map_type)||
					(MULTIPLE_ACTIVATION==analysis->map_type))&&
					(mapping=analysis->mapping_window)&&(mapping->map)&&
					(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_label_type))
				{
					update_mapping_drawing_area(mapping,0);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_reject_event.  analysis work area missing");
	}
	LEAVE;
} /* analysis_reject_event */

#if defined (OLD_CODE)
static void event_hot_key_handler(Widget widget,
	XtPointer analysis_work_area_structure,XEvent *event,
	Boolean *continue_to_dispatch)
/*******************************************************************************
LAST MODIFIED : 24 December 1996

DESCRIPTION :
The hot key handler for the <widget>.
==============================================================================*/
{
	Boolean sensitive;
	Display *display;
	static KeyCode a_key_code=0,n_key_code,p_key_code,r_key_code;
	struct Trace_window *trace_window;
	struct User_interface *user_interface;
	XKeyEvent *key_press;

	if ((key_press=(XKeyEvent *)event)&&analysis_work_area_structure&&
		(trace_window=((struct Analysis_work_area *)analysis_work_area_structure)->
		trace)&&(user_interface=((struct Analysis_work_area *)
		analysis_work_area_structure)->user_interface))
	{
		display=User_interface_get_display(user_interface);
		if (EVENT_DETECTION==*trace_window->analysis_mode)
		{
			if (!((key_press->state)&(~(ShiftMask|LockMask))))
			{
				if (0==a_key_code)
				{
					a_key_code=(unsigned int)XKeysymToKeycode(display,97);
					n_key_code=(unsigned int)XKeysymToKeycode(display,110);
					p_key_code=(unsigned int)XKeysymToKeycode(display,112);
					r_key_code=(unsigned int)XKeysymToKeycode(display,114);
				}
				if (a_key_code==key_press->keycode)
				{
					XtVaGetValues((trace_window->area_3).edit.accept_button,
						XmNsensitive,&sensitive,
						NULL);
					if (True==sensitive)
					{
						analysis_accept_event((trace_window->area_3).edit.accept_button,
							analysis_work_area_structure,(XtPointer)NULL);
					}
				}
				else
				{
					if (n_key_code==key_press->keycode)
					{
						XtVaGetValues((trace_window->area_3).edit.next_button,
							XmNsensitive,&sensitive,
							NULL);
						if (True==sensitive)
						{
							analysis_next_event((trace_window->area_3).edit.next_button,
								analysis_work_area_structure,(XtPointer)NULL);
						}
					}
					else
					{
						if (p_key_code==key_press->keycode)
						{
							XtVaGetValues((trace_window->area_3).edit.previous_button,
								XmNsensitive,&sensitive,
								NULL);
							if (True==sensitive)
							{
								analysis_previous_event(
									(trace_window->area_3).edit.previous_button,
									analysis_work_area_structure,(XtPointer)NULL);
							}
						}
						else
						{
							if (r_key_code==key_press->keycode)
							{
								XtVaGetValues((trace_window->area_3).edit.reject_button,
									XmNsensitive,&sensitive,
									NULL);
								if (True==sensitive)
								{
									analysis_reject_event(
										(trace_window->area_3).edit.reject_button,
										analysis_work_area_structure,(XtPointer)NULL);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"event_hot_key_handler.  Missing event or trace window");
	}
} /* event_hot_key_handler */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void add_event_hot_key_handler(Widget *widget_id,
	XtPointer analysis_work_area_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 April 1996

DESCRIPTION :
Adds the hot key handler to the widget.
==============================================================================*/
{
	ENTER(add_event_hot_key_handler);
	if (analysis_work_area_structure)
	{
		XtAddEventHandler(*widget_id,KeyPressMask,False,event_hot_key_handler,
			analysis_work_area_structure);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_event_hot_key_handler.  Missing analysis_window_structure");
	}
	LEAVE;
} /* add_event_hot_key_handler */
#endif /* defined (OLD_CODE) */

static void calculate_all_event_markers(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 2003

DESCRIPTION :
Calculate the positions of all event markers, recalculate the datum if this is
set to automatic and reorder the devices if this is required.
==============================================================================*/
{
	char *value_string;
	float level,*objective_values;
	int average_width,datum,highlight_number,minimum_separation,number_of_devices,
		number_of_objective_values,objective_values_step,threshold_percentage,xpos,
		ypos;
	struct Analysis_work_area *analysis;
	struct Device **device,**highlight,*highlight_device;
	struct Event *event;
	struct Map *map;
	struct Region *current_region;
	struct Rig *rig;
	struct Signals_area *signals_area;
	struct Signal_buffer *buffer;

	ENTER(calculate_all_event_markers);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->trace))
	{
		if (rig=analysis->rig)
		{
			/* clear the present markers */
			if (analysis->trace->area_1.enlarge.calculate_all_events)
			{
				draw_all_markers(1,0,analysis);
			}
			else
			{
				if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
					(highlight_device->signal)&&
					(buffer=highlight_device->signal->buffer)&&
					(REJECTED!=highlight_device->signal->status))
				{
					/* clear the present markers */
					signals_area= &(analysis->window->signals);
					current_region=get_Rig_current_region(rig);
					highlight=rig->devices;
					highlight_number=0;
					while (*highlight!=highlight_device)
					{
						if (!current_region||(current_region==(*highlight)->description->
							region))
						{
							highlight_number++;
						}
						highlight++;
					}
					xpos=(signals_area->axes_left)+
						((highlight_number/(signals_area->number_of_rows))*
						(signals_area->drawing->width))/(signals_area->number_of_columns);
					switch (signals_area->layout)
					{
						case SEPARATE_LAYOUT:
						{
							ypos=(signals_area->axes_top)+
								((highlight_number%(signals_area->number_of_rows))*
								(signals_area->drawing->height))/(signals_area->number_of_rows);
						} break;
						case OVERLAP_LAYOUT:
						{
							ypos=(signals_area->axes_top)+
								((highlight_number%(signals_area->number_of_rows))*
								(signals_area->drawing->height))/
								((signals_area->number_of_rows)+3);
						} break;
					}
					draw_device_markers(highlight_device,buffer->start,buffer->end,
						analysis->datum,0,analysis->potential_time,0,SIGNAL_AREA_DETAIL,0,
						xpos,ypos,signals_area->axes_width,signals_area->axes_height,
						XtWindow(signals_area->drawing_area),
						signals_area->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					trace_draw_markers(0,0,analysis->trace);
				}
				else
				{
					highlight_device=(struct Device *)NULL;
					display_message(ERROR_MESSAGE,
						"calculate_all_event_markers.  Invalid highlight");
				}
			}
			average_width=analysis->average_width;
			switch (analysis->detection)
			{
				case EDA_LEVEL:
				{
					value_string=(char *)NULL;
					XtVaGetValues(analysis->trace->area_1.enlarge.level_value,
						XmNvalue,&value_string,
						NULL);
					if (1==sscanf(value_string,"%f",&level))
					{
						if (level<0)
						{
							level=0;
						}
					}
					else
					{
						level=analysis->level;
					}
					analysis->level=level;
					XtFree(value_string);
					sprintf(global_temp_string,"%g",level);
					XtVaSetValues(analysis->trace->area_1.enlarge.level_value,
						XmNvalue,global_temp_string,
						NULL);
					value_string=(char *)NULL;
					XtVaGetValues(analysis->trace->menu.average_width_txt,
						XmNvalue,&value_string,
						NULL);
					if (1==sscanf(value_string,"%d",&average_width))
					{
						if (average_width<1)
						{
							average_width=1;
						}
					}
					else
					{
						average_width=analysis->average_width;
					}
					analysis->average_width=average_width;
					XtFree(value_string);
					sprintf(global_temp_string,"%d",average_width);
					XtVaSetValues(analysis->trace->menu.average_width_txt,
						XmNvalue,global_temp_string,
						NULL);
				} break;
				case EDA_THRESHOLD:
				{
					XtVaGetValues(analysis->trace->area_1.enlarge.threshold_scroll,
						XmNvalue,&threshold_percentage,
						NULL);
					XtVaGetValues(
						analysis->trace->area_1.enlarge.minimum_separation_scroll,
						XmNvalue,&minimum_separation,
						NULL);
				} break;
			}
			if (analysis->trace->area_1.enlarge.calculate_all_events)
			{
				if ((analysis->trace)&&(analysis->trace->processed_device)&&
					(analysis->trace->processed_device->signal)&&
					(analysis->trace->processed_device->signal->buffer)&&
					(analysis->trace->processed_device->signal->next)&&
					(objective_values=analysis->trace->processed_device->signal->
					buffer->signals.float_values)&&(0<(objective_values_step=analysis->
					trace->processed_device->signal->buffer->number_of_signals))&&
					(0<(number_of_objective_values=analysis->trace->processed_device->
					signal->buffer->number_of_samples)))
				{
					objective_values +=
						analysis->trace->processed_device->signal->next->index;
					/* for each device/signal calculate the event markers */
					busy_cursor_on((Widget)NULL,analysis->user_interface);
					device=rig->devices;
					number_of_devices=rig->number_of_devices;
					while (number_of_devices>0)
					{
						if (((*device)->signal)&&(*device)->signal->status!=REJECTED)
						{
							calculate_device_objective(*device,analysis->detection,
								analysis->objective,objective_values,number_of_objective_values,
								objective_values_step,average_width);
							calculate_device_event_markers(*device,
								analysis->start_search_interval,analysis->end_search_interval,
								analysis->detection,objective_values,number_of_objective_values,
								objective_values_step,analysis->number_of_events,
								threshold_percentage,minimum_separation,level);
						}
						device++;
						number_of_devices--;
					}
					busy_cursor_off((Widget)NULL,analysis->user_interface);
					/*??? calculate the event times */
					/* reorder the signals if required */
					if (EVENT_ORDER==analysis->signal_order)
					{
						/* save highlighted device */
						if (highlight=analysis->highlight)
						{
							highlight_device= *highlight;
						}
						/* sort devices */
/*						qsort((void *)(rig->devices),rig->number_of_devices,
							sizeof(struct Device *),sort_devices_by_event_time);*/
						heapsort((void *)(rig->devices),rig->number_of_devices,
							sizeof(struct Device *),sort_devices_by_event_time);
						/* update highlight */
						if (highlight)
						{
							current_region=get_Rig_current_region(rig);
							highlight=rig->devices;
							while (*highlight!=highlight_device)
							{
								highlight++;
							}
							analysis->highlight=highlight;
						}
						/* determine the datum */
						if (AUTOMATIC_DATUM==analysis->datum_type)
						{
							if ((device=rig->devices)&&((*device)->signal)&&
								(event=(*device)->signal->first_event))
							{
								analysis->datum=event->time;
							}
						}
						/* redraw the signals */
						trace_change_signal(analysis->trace);
						update_signals_drawing_area(analysis->window);
					}
					else
					{
						/* determine the datum */
						if (AUTOMATIC_DATUM==analysis->datum_type)
						{
							if (device=rig->devices)
							{
								buffer=(*device)->signal->buffer;
								datum=(buffer->times)[buffer->number_of_samples-1]+1;
								number_of_devices=rig->number_of_devices;
								while (number_of_devices>0)
								{
									if (((*device)->signal)&&
										(event=(*device)->signal->first_event)&&(event->time<datum))
									{
										datum=event->time;
									}
									device++;
									number_of_devices--;
								}
								if (datum<=(buffer->times)[buffer->number_of_samples-1])
								{
									analysis->datum=datum;
									/*??? update the datum markers */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calculate_all_event_markers.  Could not find datum");
								}
							}
						}
						if (highlight=analysis->highlight)
						{
							/* put the correct objective in the trace window */
							calculate_device_objective(*highlight,analysis->detection,
								analysis->objective,objective_values,number_of_objective_values,
								objective_values_step,average_width);
						}
						/* draw the event markers */
						draw_all_markers(1,0,analysis);
					}
					if (EDA_THRESHOLD==analysis->detection)
					{
						/* change to calculating events for the current device only */
						analysis->trace->area_1.enlarge.calculate_all_events=0;
						XtVaSetValues(analysis->trace->area_1.enlarge.all_current_choice,
							XmNmenuHistory,
							analysis->trace->area_1.enlarge.all_current.current_button,
							NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_all_event_markers.  Invalid processed_device");
				}
			}
			else
			{
				if (highlight_device)
				{
					if ((analysis->trace)&&(analysis->trace->processed_device)&&
						(analysis->trace->processed_device->signal)&&
						(analysis->trace->processed_device->signal->buffer)&&
						(objective_values=analysis->trace->processed_device->signal->
						buffer->signals.float_values)&&(0<(objective_values_step=analysis->
						trace->processed_device->signal->buffer->number_of_signals))&&
						(0<(number_of_objective_values=analysis->trace->processed_device->
						signal->buffer->number_of_samples)))
					{
						calculate_device_objective(highlight_device,analysis->detection,
							analysis->objective,objective_values,number_of_objective_values,
							objective_values_step,average_width);
						calculate_device_event_markers(highlight_device,
							analysis->start_search_interval,analysis->end_search_interval,
							analysis->detection,objective_values,number_of_objective_values,
							objective_values_step,analysis->number_of_events,
							threshold_percentage,minimum_separation,level);
						draw_device_markers(highlight_device,buffer->start,buffer->end,
							analysis->datum,0,analysis->potential_time,0,SIGNAL_AREA_DETAIL,0,
							xpos,ypos,signals_area->axes_width,signals_area->axes_height,
							XtWindow(signals_area->drawing_area),
							signals_area->drawing->pixel_map,
							analysis->signal_drawing_information,analysis->user_interface);
						trace_draw_markers(0,0,analysis->trace);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_all_event_markers.  Invalid processed_device");
					}
				}
			}
			/* update the mapping window */
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			/* allow event times to be saved */
			XtSetSensitive(analysis->window->file_menu.save_times_button,True);
			/* unghost the display single activation map button */
			XtSetSensitive(analysis->window->map_menu.single_activation_button,True);
			/* unghost the display multiple activation map button */
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
				True);
			/* unghost the display activation potential map button */
			XtSetSensitive(analysis->window->map_menu.activation_potential_button,
				True);
			analysis->calculate_events=1;
			trace_update_signal_controls(analysis->trace);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_all_event_markers.  Analysis rig missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_all_event_markers.  Missing analysis_work_area");
	}
	LEAVE;
} /* calculate_all_event_markers */

static void set_analysis_order_event(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 1993

DESCRIPTION :
Sets the order in which signals are displayed and edited to event.
==============================================================================*/
{
	int device_number;
	struct Region *current_region;
	struct Analysis_work_area *analysis;
	struct Rig *rig;
	struct Device **highlight,*highlight_device;

	ENTER(set_analysis_order_event);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if (EVENT_ORDER!=analysis->signal_order)
		{
			analysis->signal_order=EVENT_ORDER;
			/* reorder and redraw the signals */
			if (rig=analysis->rig)
			{
				/* save highlighted device */
				if (highlight=analysis->highlight)
				{
					highlight_device= *highlight;
				}
				/* sort devices */
/*				qsort((void *)(rig->devices),rig->number_of_devices,
					sizeof(struct Device *),sort_devices_by_event_time);*/
				heapsort((void *)(rig->devices),rig->number_of_devices,
					sizeof(struct Device *),sort_devices_by_event_time);
				/* update highlight */
				if (highlight)
				{
					current_region=get_Rig_current_region(rig);
					device_number=0;
					highlight=rig->devices;
					while (*highlight!=highlight_device)
					{
						if (!current_region||(current_region==(*highlight)->description->
							region))
						{
							device_number++;
						}
						highlight++;
					}
					analysis->highlight=highlight;
					/* reset the next and previous buttons */
					trace_update_signal_controls(analysis->trace);
				}
				/* redraw the signals */
				update_signals_drawing_area(analysis->window);
				/* redraw the map */
				update_mapping_drawing_area(analysis->mapping_window,0);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_order_event.  analysis_work_area missing");
	}
	LEAVE;
} /* set_analysis_order_event */

static void set_analysis_order_channel(Widget *widget_id,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 August 1993

DESCRIPTION :
Sets the order in which signals are displayed and edited to channel.
==============================================================================*/
{
	int device_number;
	struct Region *current_region;
	struct Analysis_work_area *analysis;
	struct Rig *rig;
	struct Device **highlight,*highlight_device;

	ENTER(set_analysis_order_channel);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if (CHANNEL_ORDER!=analysis->signal_order)
		{
			analysis->signal_order=CHANNEL_ORDER;
			/* reorder and redraw the signals */
			if (rig=analysis->rig)
			{
				/* save highlighted device */
				if (highlight=analysis->highlight)
				{
					highlight_device= *highlight;
				}
				/* sort devices */
/*				qsort((void *)(rig->devices),rig->number_of_devices,
					sizeof(struct Device *),sort_devices_by_number);*/
				heapsort((void *)(rig->devices),rig->number_of_devices,
					sizeof(struct Device *),sort_devices_by_number);
				/* update highlight */
				if (highlight)
				{
					current_region=get_Rig_current_region(rig);
					device_number=0;
					highlight=rig->devices;
					while (*highlight!=highlight_device)
					{
						if (!current_region||(current_region==(*highlight)->description->
							region))
						{
							device_number++;
						}
						highlight++;
					}
					analysis->highlight=highlight;
					/* reset the next and previous buttons */
					trace_update_signal_controls(analysis->trace);
				}
				/* redraw the signals */
				update_signals_drawing_area(analysis->window);
				/* redraw the map */
				update_mapping_drawing_area(analysis->mapping_window,0);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_order_channel.  analysis_work_area missing");
	}
	LEAVE;
} /* set_analysis_order_channel */

enum Moving_status
{
	MOVING_BOX,
	MOVING_DATUM_MARKER,
	MOVING_EVENT_MARKER,
	MOVING_LEFT,
	MOVING_NONE,
	MOVING_POTENTIAL_TIME_MARKER,
	MOVING_RIGHT,
	SCALING_Y_AXIS_NEGATIVE,
	SCALING_Y_AXIS_POSITIVE
};

static int change_analysis_interval(struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
What needs to be done when the analysis interval (buffer range) is changed.
==============================================================================*/
{
	char event_interval_changed,search_interval_changed;
	int i,return_code,temp;
	struct Device *highlight_device;
	struct Map *map;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;
#if defined (UNEMAP_USE_NODES)
	FE_value end_time,start_time;
	int time_index;
	struct FE_field *display_end_time_field,*display_start_time_field,
		*signal_field;
	struct Signal_drawing_package *signal_drawing_package;
#endif /* defined (UNEMAP_USE_NODES)*/

	ENTER(change_analysis_interval);
	return_code=0;
	if (analysis&&(analysis->window)&&
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device)))
	{
		return_code=1;
#if defined (UNEMAP_USE_NODES)
		signal_drawing_package=analysis->signal_drawing_package;
		display_start_time_field=
			get_Signal_drawing_package_display_start_time_field(
			signal_drawing_package);
		display_end_time_field=get_Signal_drawing_package_display_end_time_field(
			signal_drawing_package);
		signal_field=get_Signal_drawing_package_signal_field(
			signal_drawing_package);
#endif /* defined (UNEMAP_USE_NODES)*/
#if defined (UNEMAP_USE_NODES)
		time_index=buffer->start;
		/* get the start time, update the display_start_time_field */
		get_FE_field_time_FE_value(signal_field,time_index,&start_time);
		set_FE_field_FE_value_value(display_start_time_field,0,start_time);
#endif /* defined (UNEMAP_USE_NODES)*/
#if defined (UNEMAP_USE_NODES)
		time_index=buffer->end;
		/* get the end time, update the display_end_time_field */
		get_FE_field_time_FE_value(signal_field,time_index,&end_time);
		set_FE_field_FE_value_value(display_end_time_field,0,end_time);
#endif /* defined (UNEMAP_USE_NODES)*/
		/* update the search interval */
		temp=analysis->end_search_interval-analysis->start_search_interval;
		search_interval_changed=0;
		if (buffer->start>analysis->start_search_interval)
		{
			search_interval_changed=1;
			if (analysis->search_interval_divisions)
			{
				for (i=analysis->number_of_events-2;i>=0;i--)
				{
					(analysis->search_interval_divisions)[i] +=
						(buffer->start)-(analysis->start_search_interval);
				}
			}
			analysis->start_search_interval=buffer->start;
			analysis->end_search_interval=
				analysis->start_search_interval+temp;
		}
		if (buffer->end<analysis->end_search_interval)
		{
			search_interval_changed=1;
			if (analysis->search_interval_divisions)
			{
				for (i=analysis->number_of_events-2;i>=0;i--)
				{
					(analysis->search_interval_divisions)[i] +=
						(buffer->end)-(analysis->end_search_interval);
				}
			}
			analysis->end_search_interval=buffer->end;
			analysis->start_search_interval=analysis->end_search_interval-temp;
			if (buffer->start>analysis->start_search_interval)
			{
				if (analysis->search_interval_divisions)
				{
					for (i=analysis->number_of_events-2;i>=0;i--)
					{
						(analysis->search_interval_divisions)[i] +=
							(buffer->start)-(analysis->start_search_interval);
						if ((analysis->search_interval_divisions)[i]>buffer->end)
						{
							(analysis->search_interval_divisions)[i]=buffer->end;
						}
					}
				}
				analysis->start_search_interval=buffer->start;
			}
		}
		if ((trace=analysis->trace)&&
			((EDA_LEVEL==analysis->detection)||(EDA_THRESHOLD==analysis->detection)))
		{
			/* update the highlight event interval */
				/*???DB.  Should be in trace_change_display_interval ? */
			temp=trace->area_3.edit.last_data-trace->area_3.edit.first_data;
			event_interval_changed=0;
			if (buffer->start>
				trace->area_3.edit.first_data)
			{
				event_interval_changed=1;
				trace->area_3.edit.first_data=buffer->start;
				trace->area_3.edit.last_data=
					analysis->trace->area_3.edit.first_data+temp;
			}
			if (buffer->end<trace->area_3.edit.last_data)
			{
				event_interval_changed=1;
				trace->area_3.edit.last_data=buffer->end;
				trace->area_3.edit.first_data=trace->area_3.edit.last_data-temp;
				if (buffer->start>trace->area_3.edit.first_data)
				{
					trace->area_3.edit.first_data=buffer->start;
				}
			}
			if (event_interval_changed)
			{
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			}
		}
		if (search_interval_changed)
		{
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
			if (EDA_INTERVAL==analysis->detection)
			{
				/* free the event memory */
				destroy_all_events(analysis->rig);
				XtSetSensitive(analysis->window->file_menu.save_times_button,False);
				XtSetSensitive(analysis->window->map_menu.single_activation_button,
					False);
				XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
					False);
				XtSetSensitive(analysis->window->map_menu.activation_potential_button,
					False);
				analysis->calculate_events=0;
			}
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
			trace_change_search_interval(analysis->trace);
			if ((
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
				((EDA_INTERVAL==analysis->detection)&&
				((SINGLE_ACTIVATION==analysis->map_type)||
				(ACTIVATION_POTENTIAL==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type)))||
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
				(INTEGRAL==analysis->map_type))&&
				(analysis->mapping_window)&&
				(map=analysis->mapping_window->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
		}
		trace_change_display_interval(analysis->trace);
		update_signals_drawing_area(analysis->window);
		update_analysis_window_buffer_range_menu(analysis->window);
	}
	LEAVE;

	return (return_code);
} /* change_analysis_interval */

static void select_analysis_interval(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
The callback for modifying the analysis interval in the analysis interval
drawing area.
==============================================================================*/
{
	char number_string[20];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float frequency,x_scale;
	GC potential_time_colour,potential_time_colour_text;
	int ascent,axes_bottom,axes_left,axes_right,axes_top,box_range,descent,
		direction,initial_potential_time,left_box,length,minimum_box_range,
		pointer_sensitivity,pointer_x,pointer_y,potential_time,previous_right_box,
		previous_pointer_x,previous_potential_time,previous_left_box,right_box,temp,
		*times,x_string,y_string;
	Pixmap pixel_map;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Interval_area *interval;
	struct Signal_buffer *buffer;
	struct Time_keeper *time_keeper;
	unsigned int working_button;
	Window working_window;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XFontStruct *font;
	XmDrawingAreaCallbackStruct *callback;
#if defined (UNEMAP_USE_NODES)
	FE_value end_time,start_time;
	int time_index;
	struct FE_field *display_start_time_field,*display_end_time_field,
		*signal_field;
	struct Signal_drawing_package *signal_drawing_package;
#endif /* defined (UNEMAP_USE_NODES)*/

	ENTER(select_analysis_interval);
	USE_PARAMETER(widget);
#if defined (UNEMAP_USE_NODES)
	display_start_time_field=(struct FE_field *)NULL;
	display_end_time_field=(struct FE_field *)NULL;
	signal_field=(struct FE_field *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
#endif /* defined (UNEMAP_USE_NODES)*/
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device))&&(times=buffer->times))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if ((callback->event)&&(ButtonPress==callback->event->type))
				{
					display=User_interface_get_display(analysis->user_interface);
					pointer_sensitivity=analysis->pointer_sensitivity;
					potential_time_colour=(analysis->signal_drawing_information->
						graphics_context).potential_time_colour;
					potential_time_colour_text=(analysis->signal_drawing_information->
						graphics_context).potential_time_colour_text;
					font=analysis->signal_drawing_information->font;
					frequency=buffer->frequency;
					button_event= &(callback->event->xbutton);
					pointer_x=button_event->x;
					pointer_y=button_event->y;
					working_button=button_event->button;
					interval= &(analysis->window->interval);
					axes_left=interval->axes_left;
					axes_right=axes_left+(interval->axes_width)-1;
					axes_top=interval->axes_top;
					axes_bottom=axes_top+(interval->axes_height)-1;
					left_box=interval->left_box;
					right_box=interval->right_box;
					x_scale=
						SCALE_FACTOR((buffer->number_of_samples)-1,axes_right-axes_left);
					potential_time=SCALE_X(analysis->potential_time,0,axes_left,x_scale);
					if ((((pointer_x>=left_box-pointer_sensitivity)&&
						(pointer_x<=right_box+pointer_sensitivity))||
						((pointer_x>=potential_time-pointer_sensitivity)&&
						(pointer_x<=potential_time+pointer_sensitivity)))&&
						(pointer_y>=axes_top-pointer_sensitivity)&&
						(pointer_y<=axes_bottom+pointer_sensitivity))
					{
						/* stop the time keeper */
						time_keeper=(struct Time_keeper *)NULL;
						if ((analysis->potential_time_object)&&(time_keeper=
							Time_object_get_time_keeper(analysis->potential_time_object)))
						{
							if (Time_keeper_is_playing(time_keeper))
							{
								Time_keeper_stop(time_keeper);
							}
						}
						if ((pointer_x>=potential_time-pointer_sensitivity)&&
							(pointer_x<=potential_time+pointer_sensitivity))
						{
							moving=MOVING_POTENTIAL_TIME_MARKER;
							cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
						}
						else
						{
							if (pointer_x<=left_box+pointer_sensitivity)
							{
								moving=MOVING_LEFT;
								cursor=XCreateFontCursor(display,XC_left_side);
							}
							else
							{
								if (pointer_x>=right_box-pointer_sensitivity)
								{
									moving=MOVING_RIGHT;
									cursor=XCreateFontCursor(display,XC_right_side);
								}
								else
								{
									moving=MOVING_BOX;
									cursor=XCreateFontCursor(display,XC_dotbox);
								}
							}
						}
#if defined (UNEMAP_USE_NODES)
						signal_drawing_package=analysis->signal_drawing_package;
						display_start_time_field=
							get_Signal_drawing_package_display_start_time_field(
								signal_drawing_package);
						display_end_time_field=
							get_Signal_drawing_package_display_end_time_field(
								signal_drawing_package);
						signal_field=get_Signal_drawing_package_signal_field(
							signal_drawing_package);
#endif /* defined (UNEMAP_USE_NODES)*/
						/* set the cursor */
						XDefineCursor(display,XtWindow(interval->drawing_area),
							cursor);
						XmUpdateDisplay(interval->drawing_area);
						box_range=right_box-left_box;
						minimum_box_range=2*pointer_sensitivity+1;
						working_window=XtWindow(interval->drawing_area);
						pixel_map=interval->drawing->pixel_map;
						initial_potential_time=potential_time;
						switch (moving)
						{
							case MOVING_POTENTIAL_TIME_MARKER:
							{
								XWarpPointer(display,None,None,0,0,0,0,
									potential_time-pointer_x,0);
								pointer_x=potential_time;
								/* clear the potential time */
								sprintf(number_string,"%d",
									(int)((float)(times[analysis->potential_time])*1000./
										frequency));
								length=strlen(number_string);
								XTextExtents(font,number_string,length,&direction,&ascent,
									&descent,&bounds);
								x_string=pointer_x+(bounds.lbearing-bounds.rbearing+1)/2;
								if (x_string+bounds.rbearing>=axes_right)
								{
									x_string=axes_right-bounds.rbearing;
								}
								if (x_string-bounds.lbearing<axes_left)
								{
									x_string=axes_left+bounds.lbearing;
								}
								y_string=axes_top-descent;
								XDrawString(display,pixel_map,potential_time_colour_text,
									x_string,y_string,number_string,length);
								XDrawString(display,working_window,potential_time_colour,
									x_string,y_string,number_string,length);
							} break;
							case MOVING_LEFT:
							{
								XWarpPointer(display,None,None,0,0,0,0,left_box-pointer_x,0);
								pointer_x=left_box;
							} break;
							case MOVING_RIGHT:
							{
								XWarpPointer(display,None,None,0,0,0,0,right_box-pointer_x,0);
								pointer_x=right_box;
							} break;
						}
						while (moving!=MOVING_NONE)
						{
							XNextEvent(display,&xevent);
							switch (xevent.type)
							{
								case MotionNotify:
								{
									previous_left_box=left_box;
									previous_right_box=right_box;
									previous_pointer_x=pointer_x;
									previous_potential_time=potential_time;
									/* reduce the number of motion events displayed */
									while (XCheckMaskEvent(display,ButtonMotionMask,&xevent));
									pointer_x=xevent.xmotion.x;
									pointer_y=xevent.xmotion.y;
									if ((xevent.xmotion.window==working_window)&&
										(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
									{
										switch (moving)
										{
											case MOVING_POTENTIAL_TIME_MARKER:
											{
												if (pointer_x<axes_left)
												{
													pointer_x=axes_left;
													potential_time=axes_left;
												}
												else
												{
													if (pointer_x>axes_right)
													{
														pointer_x=axes_right;
														potential_time=axes_right;
													}
													else
													{
														potential_time=pointer_x;
													}
												}
											} break;
											case MOVING_LEFT:
											{
												if (pointer_x<axes_left)
												{
													left_box=axes_left;
												}
												else
												{
													if (pointer_x>(temp=right_box-minimum_box_range))
													{
														left_box=temp;
													}
													else
													{
														left_box=pointer_x;
													}
												}
											} break;
											case MOVING_RIGHT:
											{
												if (pointer_x>axes_right)
												{
													right_box=axes_right;
												}
												else
												{
													if (pointer_x<(temp=left_box+minimum_box_range))
													{
														right_box=temp;
													}
													else
													{
														right_box=pointer_x;
													}
												}
											} break;
											case MOVING_BOX:
											{
												if (pointer_x<axes_left)
												{
													pointer_x=axes_left;
													left_box=axes_left;
													right_box=left_box+box_range;
												}
												else
												{
													if (pointer_x>axes_right)
													{
														pointer_x=axes_right;
														right_box=axes_right;
														left_box=right_box-box_range;
													}
													else
													{
														left_box=left_box+pointer_x-previous_pointer_x;
														if (left_box<axes_left)
														{
															left_box=axes_left;
															right_box=left_box+box_range;
														}
														else
														{
															right_box=left_box+box_range;
															if (right_box>axes_right)
															{
																right_box=axes_right;
																left_box=right_box-box_range;
															}
														}
													}
												}
											} break;
										}
										if ((left_box!=previous_left_box)||
											(right_box!=previous_right_box))
										{
											/* clear the old box */
											draw_interval_box(previous_left_box,axes_top,
												previous_right_box-previous_left_box+1,
												interval->axes_height,interval,
												analysis->signal_drawing_information,
												analysis->user_interface);
											/* draw the new box */
											draw_interval_box(left_box,axes_top,
												right_box-left_box+1,interval->axes_height,interval,
												analysis->signal_drawing_information,
												analysis->user_interface);
										}
										if (potential_time!=previous_potential_time)
										{
											/* clear the old marker */
											XDrawLine(display,pixel_map,potential_time_colour,
												previous_potential_time,axes_top,
												previous_potential_time,axes_bottom);
											XDrawLine(display,working_window,potential_time_colour,
												previous_potential_time,axes_top,
												previous_potential_time,axes_bottom);
											/* draw the new marker */
											XDrawLine(display,pixel_map,potential_time_colour,
												potential_time,axes_top,potential_time,axes_bottom);
											XDrawLine(display,working_window,potential_time_colour,
												potential_time,axes_top,potential_time,axes_bottom);
										}
									}
								} break;
								case ButtonPress:
								{
									if (xevent.xbutton.button==working_button)
									{
										display_message(ERROR_MESSAGE,
											"select_analysis_interval.  Unexpected button press");
										moving=MOVING_NONE;
									}
								} break;
								case ButtonRelease:
								{
									if (xevent.xbutton.button==working_button)
									{
										if (xevent.xbutton.window==working_window)
										{
											switch (moving)
											{
												case MOVING_LEFT:
												{
													if (left_box!=interval->left_box)
													{
														buffer->start=SCALE_X(left_box,axes_left,0,
															1/x_scale);
														interval->left_box=left_box;
														change_analysis_interval(analysis);
													}
												} break;
												case MOVING_RIGHT:
												{
													if (right_box!=interval->right_box)
													{
														buffer->end=SCALE_X(right_box,axes_left,0,
															1/x_scale);
														interval->right_box=right_box;
														change_analysis_interval(analysis);
													}
												} break;
												case MOVING_BOX:
												{
													if ((left_box!=interval->left_box)||
														(right_box!=interval->right_box))
													{
														int buffer_length,buffer_start;

														buffer_length=(buffer->end)-(buffer->start);
														/* keep the same number of samples in the
															interval */
														buffer_start=SCALE_X(left_box,axes_left,0,
															1/x_scale);
														buffer->start=buffer_start+
															((SCALE_X(right_box,axes_left,0,1/x_scale)-
															buffer_start)-buffer_length)/2;
														if (buffer->start<0)
														{
															buffer->start=0;
														}
														buffer->end=buffer->start+buffer_length;
														if (buffer->end>=buffer->number_of_samples)
														{
															buffer->end=(buffer->number_of_samples)-1;
															buffer->start=(buffer->end)-buffer_length;
														}
														interval->left_box=left_box;
														interval->right_box=right_box;
														change_analysis_interval(analysis);
													}
												} break;
												case MOVING_POTENTIAL_TIME_MARKER:
												{
													if (potential_time!=initial_potential_time)
													{
														/* clear the cursor */
														XDrawLine(display,pixel_map,
															potential_time_colour,
															potential_time,axes_top,
															potential_time,axes_bottom);
														XDrawLine(display,working_window,
															potential_time_colour,
															potential_time,axes_top,
															potential_time,axes_bottom);
														/* update time objects */
														/* This conversion to time_keeper time should
															be much more robust.  The frequency is not
															guaranteed to divide the potential time and
															this takes no account of time transformations */
														analysis->analysis_update_flags |=
															ANALYSIS_INTERVAL_NO_POTENTIAL_ERASE;
														potential_time=SCALE_X(potential_time,
															axes_left,0,1/x_scale);
														Time_keeper_request_new_time(
															Time_object_get_time_keeper(
															analysis->potential_time_object),
															((double)times[potential_time]/frequency));
													}
													else
													{
														/* rewrite the potential time */
														XDrawString(display,pixel_map,
															potential_time_colour_text,
															x_string,y_string,number_string,length);
														XDrawString(display,working_window,
															potential_time_colour,
															x_string,y_string,number_string,length);
													}
												} break;
											}
										}
										else
										{
											if ((left_box!=interval->left_box)||
												(right_box!=interval->right_box))
											{
												/* clear the new box */
												draw_interval_box(left_box,axes_top,
													right_box-left_box+1,
													interval->axes_height,interval,
													analysis->signal_drawing_information,
													analysis->user_interface);
												/* draw the old box */
												draw_interval_box(interval->left_box,axes_top,
													interval->right_box-interval->left_box+1,
													interval->axes_height,interval,
													analysis->signal_drawing_information,
													analysis->user_interface);
											}
											if (potential_time!=initial_potential_time)
											{
												/* clear the new marker */
												XDrawLine(display,pixel_map,potential_time_colour,
													potential_time,axes_top,potential_time,axes_bottom);
												XDrawLine(display,working_window,
													potential_time_colour,
													potential_time,axes_top,potential_time,axes_bottom);
												/* draw the initial marker */
												XDrawLine(display,pixel_map,potential_time_colour,
													initial_potential_time,axes_top,
													initial_potential_time,axes_bottom);
												XDrawLine(display,working_window,
													potential_time_colour,
													initial_potential_time,axes_top,
													initial_potential_time,axes_bottom);
												XDrawString(display,pixel_map,
													potential_time_colour_text,
													x_string,y_string,number_string,length);
												XDrawString(display,working_window,
													potential_time_colour,
													x_string,y_string,number_string,length);
											}
										}
										moving=MOVING_NONE;
									}
								} break;
								default:
								{
									XtDispatchEvent(&xevent);
								}
							}
						}
						XUndefineCursor(display,XtWindow(interval->drawing_area));
						XFreeCursor(display,cursor);
						if (time_keeper&&(analysis->mapping_window))
						{
							mapping_window_update_time_limits(analysis->mapping_window);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_analysis_interval.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_analysis_interval.  call_data missing");
		}
	}
	LEAVE;
} /* select_analysis_interval */

static void analysis_set_buffer_start(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
Sets the buffer start from the buffer range menu.
==============================================================================*/
{
	char *value_string;
	int start;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;

	ENTER(analysis_set_buffer_start);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window))
	{
		if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			XtVaGetValues(
				(analysis_window->interval).buffer_range.start_sample_number_text,
				XmNvalue,&value_string,NULL);
			if (1==sscanf(value_string,"%d",&start))
			{
				if (start<0)
				{
					buffer->start=0;
				}
				else
				{
					if (start>=buffer->end)
					{
						buffer->start=(buffer->end)-1;
					}
					else
					{
						buffer->start=start;
					}
				}
				update_interval_drawing_area(analysis_window);
				change_analysis_interval(analysis);
			}
			else
			{
				update_analysis_window_buffer_range_menu(analysis_window);
			}
		}
	}
	LEAVE;
} /* analysis_set_buffer_start */

static void analysis_set_buffer_start_time(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
Sets the buffer start from the buffer range menu.
==============================================================================*/
{
	char *value_string;
	float frequency,start_time;
	int number_of_samples,start,start_time_int,*times;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;

	ENTER(analysis_set_buffer_start_time);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window))
	{
		if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			XtVaGetValues(
				(analysis_window->interval).buffer_range.start_time_text,
				XmNvalue,&value_string,NULL);
			if ((1==sscanf(value_string,"%f",&start_time))&&
				(0<(frequency=buffer->frequency))&&(times=buffer->times)&&
				(0<(number_of_samples=buffer->number_of_samples)))
			{
				start_time /= 1000.;
				start_time_int=(int)(frequency*start_time+0.5);
				start=0;
				while ((start<number_of_samples)&&(times[start]<start_time_int))
				{
					start++;
				}
				if ((0<start)&&((float)(times[start])/frequency-start_time>
					start_time-(float)(times[start-1])/frequency))
				{
					start--;
				}
				if (start<0)
				{
					buffer->start=0;
				}
				else
				{
					if (start>=buffer->end)
					{
						buffer->start=(buffer->end)-1;
					}
					else
					{
						buffer->start=start;
					}
				}
				update_interval_drawing_area(analysis_window);
				change_analysis_interval(analysis);
			}
			else
			{
				update_analysis_window_buffer_range_menu(analysis_window);
			}
		}
	}
	LEAVE;
} /* analysis_set_buffer_start_time */

static void analysis_set_buffer_end(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
Sets the buffer end from the buffer range menu.
==============================================================================*/
{
	char *value_string;
	int end;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;

	ENTER(analysis_set_buffer_end);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window))
	{
		if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			XtVaGetValues(
				(analysis_window->interval).buffer_range.end_sample_number_text,
				XmNvalue,&value_string,NULL);
			if (1==sscanf(value_string,"%d",&end))
			{
				if (end<=buffer->start)
				{
					buffer->end=(buffer->start)+1;
				}
				else
				{
					if (end>=buffer->number_of_samples)
					{
						buffer->end=(buffer->number_of_samples)-1;
					}
					else
					{
						buffer->end=end;
					}
				}
				update_interval_drawing_area(analysis_window);
				change_analysis_interval(analysis);
			}
			else
			{
				update_analysis_window_buffer_range_menu(analysis_window);
			}
		}
	}
	LEAVE;
} /* analysis_set_buffer_end */

static void analysis_set_buffer_end_time(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
Sets the buffer end from the buffer range menu.
==============================================================================*/
{
	char *value_string;
	float frequency,end_time;
	int number_of_samples,end,end_time_int,*times;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;

	ENTER(analysis_set_buffer_end_time);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window))
	{
		if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			XtVaGetValues(
				(analysis_window->interval).buffer_range.end_time_text,
				XmNvalue,&value_string,NULL);
			if ((1==sscanf(value_string,"%f",&end_time))&&
				(0<(frequency=buffer->frequency))&&(times=buffer->times)&&
				(0<(number_of_samples=buffer->number_of_samples)))
			{
				end_time /= 1000.;
				end_time_int=(int)(frequency*end_time+0.5);
				end=number_of_samples-1;
				while ((end>0)&&(times[end]>end_time_int))
				{
					end--;
				}
				if ((end<number_of_samples-1)&&(end_time-(float)(times[end])/frequency>
					(float)(times[end+1])/frequency-end_time))
				{
					end++;
				}
				if (end<=buffer->start)
				{
					buffer->end=(buffer->start)+1;
				}
				else
				{
					if (end>=buffer->number_of_samples)
					{
						buffer->end=(buffer->number_of_samples)-1;
					}
					else
					{
						buffer->end=end;
					}
				}
				update_interval_drawing_area(analysis_window);
				change_analysis_interval(analysis);
			}
			else
			{
				update_analysis_window_buffer_range_menu(analysis_window);
			}
		}
	}
	LEAVE;
} /* analysis_set_buffer_end_time */

static void decrement_number_of_events(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Decrement the number of events.
???DB.  CLEAR_EVENTS_ON_SEARCH_CHANGE should be done as a callback from the
trace window.
???DB.  Shared by beat averaging for the number of beats
==============================================================================*/
{
	char number_string[10*(MAX_EVENTS/10+1)+1];
	struct Analysis_work_area *analysis;
	struct Trace_window *trace;
	XmString xm_string;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(decrement_number_of_events);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((trace=analysis->trace)&&(trace->event_detection.event_number)&&
			(trace->event_detection.number_of_events))
		{
			if (MAX_EVENTS== *(trace->event_detection.number_of_events))
			{
				XtManageChild(trace->area_1.enlarge.number_of_events.up_arrow);
				XtManageChild(trace->area_1.beat_averaging.number_of_beats.up_arrow);
			}
			(*(trace->event_detection.number_of_events))--;
			if (*(trace->event_detection.event_number)>
				*(trace->event_detection.number_of_events))
			{
				*(trace->event_detection.event_number)=
					*(trace->event_detection.number_of_events);
			}
			sprintf(number_string,"%1d",*(trace->event_detection.number_of_events));
			xm_string=XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET);
			XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
				XmNlabelString,xm_string,
				NULL);
			XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
				XmNlabelString,xm_string,
				NULL);
			XmStringFree(xm_string);
			if (1== *(trace->event_detection.number_of_events))
			{
				XtUnmanageChild(trace->area_1.enlarge.number_of_events.down_arrow);
				XtUnmanageChild(
					trace->area_1.beat_averaging.number_of_beats.down_arrow);
			}
			/* go back to constant width divisions */
			DEALLOCATE(*(trace->event_detection.search_interval_divisions));
			DEALLOCATE(trace->area_1.enlarge.divisions);
			if (BEAT_AVERAGING==*trace->analysis_mode)
			{
				trace_process_device(trace);
			}
			trace_update_edit_interval(trace);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
		}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
		/* clear the present markers */
			/* ???signals area only ? */
		draw_all_markers(0,0,analysis);
		destroy_all_events(analysis->rig);
		/* update the mapping window */
		if (((SINGLE_ACTIVATION==analysis->map_type)||
			(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
			(map=analysis->mapping_window->map))
		{
			analysis->map_type=NO_MAP_FIELD;
			map->colour_option=HIDE_COLOUR;
			map->contours_option=HIDE_CONTOURS;
			map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
			/* clear the colour map */
			map->activation_front= -1;
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			XtSetSensitive(analysis->mapping_window->animate_button,False);
		}
		XtSetSensitive(analysis->window->file_menu.save_times_button,False);
		XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.multiple_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.activation_potential_button,
			False);
		analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"decrement_number_of_events.  Missing analysis_work_area");
	}
	LEAVE;
} /* decrement_number_of_events */

static void increment_number_of_events(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
Increment the number of events.
???DB.  CLEAR_EVENTS_ON_SEARCH_CHANGE should be done as a callback from the
trace window.
???DB.  Shared by beat averaging for the number of beats
==============================================================================*/
{
	char number_string[10*(MAX_EVENTS/10+1)+1];
	struct Analysis_work_area *analysis;
	struct Trace_window *trace;
	XmString xm_string;
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
	struct Map *map;
	struct Mapping_window *mapping;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */

	ENTER(increment_number_of_events);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((trace=analysis->trace)&&(trace->event_detection.event_number)&&
			(trace->event_detection.number_of_events))
		{
			if (1== *(trace->event_detection.number_of_events))
			{
				XtManageChild(trace->area_1.enlarge.number_of_events.down_arrow);
				XtManageChild(trace->area_1.beat_averaging.number_of_beats.down_arrow);
			}
			(*(trace->event_detection.number_of_events))++;
			sprintf(number_string,"%1d",*(trace->event_detection.number_of_events));
			xm_string=XmStringCreate(number_string,XmSTRING_DEFAULT_CHARSET);
			XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
				XmNlabelString,xm_string,
				NULL);
			XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
				XmNlabelString,xm_string,
				NULL);
			XmStringFree(xm_string);
			if (MAX_EVENTS== *(trace->event_detection.number_of_events))
			{
				XtUnmanageChild(trace->area_1.enlarge.number_of_events.up_arrow);
				XtUnmanageChild(trace->area_1.beat_averaging.number_of_beats.up_arrow);
			}
			/* go back to constant width divisions */
			DEALLOCATE(*(trace->event_detection.search_interval_divisions));
			DEALLOCATE(trace->area_1.enlarge.divisions);
			if (BEAT_AVERAGING==*trace->analysis_mode)
			{
				trace_process_device(trace);
			}
			trace_update_edit_interval(trace);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
		}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
		/* clear the present markers */
			/* ???signals area only ? */
		draw_all_markers(0,0,analysis);
		destroy_all_events(analysis->rig);
		if (((SINGLE_ACTIVATION==analysis->map_type)||
			(MULTIPLE_ACTIVATION==analysis->map_type))&&(analysis->mapping_window)&&
			(map=analysis->mapping_window->map))
		{
			analysis->map_type=NO_MAP_FIELD;
			map->colour_option=HIDE_COLOUR;
			map->contours_option=HIDE_CONTOURS;
			map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
			/* clear the colour map */
			map->activation_front= -1;
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			XtSetSensitive(analysis->mapping_window->animate_button,False);
		}
		XtSetSensitive(analysis->window->file_menu.save_times_button,False);
		XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.multiple_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.activation_potential_button,
			False);
		analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"increment_number_of_events.  Missing trace_window");
	}
	LEAVE;
} /* increment_number_of_events */

static int move_Electrical_imaging_event_marker(
	struct Trace_window_area_3 *trace_area_3,
	int initial_marker,struct Electrical_imaging_event *event,float x_scale,
	struct Signal_drawing_information *signal_drawing_information,
	Display *display,int x_pointer,enum Trace_moving_status moving,int *times,
	float frequency,int start_analysis_interval,unsigned int working_button,
	struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED 22 November 2001

DESCRIPTION : moves the Electrical_imaging_event <event> in response to mouse
event.
==============================================================================*/
{
	char time_string[20];
	Cursor cursor;
	GC graphics_context;
	int axes_bottom,axes_left,axes_right,axes_top,axes_width,
		marker,pointer_x,pointer_y,previous_marker,
		return_code,time;
	Pixmap pixel_map;
	Window working_window;
	XEvent xevent;

	ENTER(move_Electrical_imaging_event_marker);
	if (trace_area_3&&event&&signal_drawing_information&&display&&times)
	{
		graphics_context=(signal_drawing_information->graphics_context).
			eimaging_event_colour;
		axes_left=trace_area_3->axes_left;
		axes_width=trace_area_3->axes_width;
		axes_right=axes_left+axes_width-1;
		axes_top=trace_area_3->axes_top;
		axes_bottom=axes_top+(trace_area_3->axes_height)-1;
		/* grab the pointer */
		cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
		XDefineCursor(display,XtWindow(trace_area_3->drawing_area),cursor);
		XmUpdateDisplay(trace_area_3->drawing_area);
		XWarpPointer(display,None,None,0,0,0,0,
			initial_marker-x_pointer,0);
		pointer_x=initial_marker;
		marker=initial_marker;
		working_window=XtWindow(trace_area_3->drawing_area);
		pixel_map=trace_area_3->drawing->pixel_map;
		while (TRACE_MOVING_NONE!=moving)
		{
			XNextEvent(display,&xevent);
			switch (xevent.type)
			{
				case MotionNotify:
				{
					previous_marker=marker;
					/* reduce the number of motion events displayed */
					while (True==XCheckMaskEvent(display,ButtonMotionMask,&xevent));
					pointer_x=xevent.xmotion.x;
					pointer_y=xevent.xmotion.y;
					if ((xevent.xmotion.window==working_window)&&
						(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
					{
						if (pointer_x<axes_left)
						{
							marker=axes_left;
						}
						else
						{
							if (pointer_x>axes_right)
							{
								marker=axes_right;
							}
							else
							{
								marker=pointer_x;
							}
						}
						reconcile_Electrical_imaging_event_marker(&marker,event,x_scale,
							axes_left,start_analysis_interval);
						if (marker!=previous_marker)
						{
							/* clear the old marker */
							XDrawLine(display,pixel_map,graphics_context,
								previous_marker,axes_top+1,previous_marker,axes_bottom);
							XDrawLine(display,working_window,graphics_context,
								previous_marker,axes_top+1,previous_marker,axes_bottom);
							/* draw the new marker */
							XDrawLine(display,pixel_map,graphics_context,marker,
								axes_top+1,marker,axes_bottom);
							XDrawLine(display,working_window,graphics_context,
								marker,axes_top+1,marker,axes_bottom);
						}	/* if (marker!=previous_marker)	*/
					}
				} break;
				case ButtonRelease:
				{
					/*event->time, marker always reconciled when moved, so don't need */
					/* to clear, reconcile, draw the marker again. I think! */

					/* write in the new time (&set flag),so last-selected/current event is
						labelled */
					time=(int)((float)((times)[event->time])
						*1000./frequency+0.5);
					sprintf(time_string,"%d",time);
					write_marker_time(time_string,marker,axes_left,
						axes_width,axes_top,signal_drawing_information->font,
						graphics_context,display,trace_area_3->drawing_area,
						trace_area_3->drawing);
					event->is_current_event=1;
					moving=TRACE_MOVING_NONE;
					/*if we have a map, update it */
					if (mapping&&mapping->map)
					{
						update_map_from_manual_time_update(mapping);
					}
				} break;
				case ButtonPress:
				{
					if (xevent.xbutton.button==working_button)
					{
						display_message(ERROR_MESSAGE,
							"move_Electrical_imaging_event_marker.  Unexpected button press");
						moving=TRACE_MOVING_NONE;
					}
				} break;
				default:
				{
					XtDispatchEvent(&xevent);
				}
			}
		}
		XUndefineCursor(display,XtWindow(trace_area_3->drawing_area));
		XFreeCursor(display,cursor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"move_Electrical_imaging_event_marker. Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* move_Electrical_imaging_event_marker */

static int erase_Electrical_imaging_event_markers_time_text(
	struct Trace_window *trace,struct Signal_buffer *buffer,
	float x_scale,struct Signal_drawing_information *signal_drawing_information,
	Display *display)
/*******************************************************************************
LAST MODIFIED : 13 June 2001

DESCRIPTION : loop therough all the Electrical_imaging_event markers in <trace>,
erasing the time text from any events that have it written.
==============================================================================*/
{
	char time_string[20];
	GC graphics_context;
	float frequency;
	int axes_left,axes_top,axes_width,marker,return_code,start_analysis_interval,
		time,*times;
	struct Electrical_imaging_event *event;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(erase_Electrical_imaging_event_markers_time_text);
	if (trace&&(trace_area_3=&(trace->area_3))&&display&&buffer&&
		signal_drawing_information)
	{
		graphics_context=(signal_drawing_information->graphics_context).
			eimaging_event_colour;
		axes_left=trace_area_3->axes_left;
		axes_width=trace_area_3->axes_width;
		axes_top=trace_area_3->axes_top;
		start_analysis_interval=buffer->start;
		frequency=buffer->frequency;
		return_code=1;
		times=buffer->times;
		event=*trace->first_eimaging_event;
		while (event)
		{
			if (event->is_current_event)
			{
				/* erase the time text from any events that have it written */
				marker=SCALE_X(event->time,start_analysis_interval,
					axes_left,x_scale);
				reconcile_Electrical_imaging_event_marker(&marker,event,x_scale,
					axes_left,start_analysis_interval);
				time=(int)((float)((times)[event->time])
					*1000./frequency+0.5);
				sprintf(time_string,"%d",time);
				write_marker_time(time_string,marker,axes_left,
					axes_width,axes_top,signal_drawing_information->font,
					graphics_context,display,trace_area_3->drawing_area,
					trace_area_3->drawing);
				event->is_current_event=0;
			}
			event=event->next;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"erase_Electrical_imaging_event_marker_time_text. Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}	/* erase_Electrical_imaging_event_marker_time_text */

int move_add_remove_Electrical_imaging_event(
	XmDrawingAreaCallbackStruct *callback,struct Trace_window *trace,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,int pointer_sensitivity,
	struct Mapping_window *mapping)
/*******************************************************************************
LAST MODIFIED : 24 September 2001

DESCRIPTION : detect mouse or keyboard activity and moves, adds or removes an
Electrical_imaging_event.
==============================================================================*/
{
	enum Trace_moving_status moving;
	struct Device *cardiac_interval_device;
	Display *display;
	float frequency,x_scale;
	GC graphics_context;
	int axes_bottom,axes_height,axes_left,axes_right,axes_top,axes_width,
		end_analysis_interval,event_time,found,marker,pointer_x,
		pointer_y,return_code,start_analysis_interval,*times;
	struct Drawing_2d *drawing;
	struct Electrical_imaging_event *event;
	struct Signal_buffer *buffer;
	struct Trace_window_area_3 *trace_area_3;
	unsigned int working_button;
	Widget drawing_area;
	XButtonEvent *button_event;

	ENTER(move_add_remove_Electrical_imaging_event);
	times=(int *)NULL;
	event=(struct Electrical_imaging_event *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	trace_area_3=(struct Trace_window_area_3 *)NULL;
	button_event=(XButtonEvent *)NULL;
	display=(Display *)NULL;
	if (callback&&trace&&(trace_area_3=&(trace->area_3))&&
		signal_drawing_information&&user_interface&&
		(cardiac_interval_device=trace->cardiac_interval_device)&&
		(buffer=get_Device_signal_buffer(cardiac_interval_device))&&
		(times=buffer->times))
	{
		graphics_context=(signal_drawing_information->graphics_context).
			eimaging_event_colour;
		return_code=1;
		if (ButtonPress==callback->event->type)
		{
			event=*trace->first_eimaging_event;
			drawing_area=trace_area_3->drawing_area;
			drawing=trace_area_3->drawing;
			display=User_interface_get_display(user_interface);
			button_event= &(callback->event->xbutton);
			working_button=button_event->button;
			pointer_x=button_event->x;
			pointer_y=button_event->y;
			axes_height=trace_area_3->axes_height;
			axes_left=trace_area_3->axes_left;
			axes_width=trace_area_3->axes_width;
			axes_right=axes_left+axes_width-1;
			axes_top=trace_area_3->axes_top;
			axes_bottom=axes_top+(trace_area_3->axes_height)-1;
			if ((pointer_x>=axes_left-pointer_sensitivity)&&
				(pointer_x<=axes_right+pointer_sensitivity)&&
				(pointer_y>=axes_top-pointer_sensitivity)&&
				(pointer_y<=axes_bottom+pointer_sensitivity))
			{
				start_analysis_interval=buffer->start;
				end_analysis_interval=buffer->end;
				x_scale=SCALE_FACTOR(end_analysis_interval-
					start_analysis_interval,axes_right-axes_left);
				frequency=buffer->frequency;
				/* are we at an electrical imaging event? */
				found=0;
				moving=TRACE_MOVING_NONE;
				while (event&&(!found))
				{
					marker=SCALE_X(event->time,start_analysis_interval,
						axes_left,x_scale);
					if ((pointer_x>=marker-pointer_sensitivity)&&
						(pointer_x<=marker+pointer_sensitivity))
					{
						moving=TRACE_MOVING_ELECTRICAL_IMAGING_EVENT;
						found=1;
					}
					if (!found)
					{
						event=event->next;
					}
				}/* while (interval&&(!found))*/
				if (found)
				{
					if ((moving==TRACE_MOVING_ELECTRICAL_IMAGING_EVENT)&&
						(working_button!=Button3))
					{
						erase_Electrical_imaging_event_markers_time_text(trace,
							buffer,x_scale,signal_drawing_information,display);
						move_Electrical_imaging_event_marker(trace_area_3,marker,event,
							x_scale,signal_drawing_information,display,pointer_x,moving,times,
							frequency,start_analysis_interval,working_button,mapping);
					}
					else if (working_button==Button3)
					{
						int is_current_event,time;
						char time_string[20];


						/*remove the Electrical_imaging_event */
						/*erase graphic */
						draw_Electrical_imaging_event_marker(event,end_analysis_interval,
							start_analysis_interval,axes_top,axes_height,axes_left,axes_width,
							drawing_area,drawing,signal_drawing_information,buffer);
						/* remove from list */
						is_current_event=event->is_current_event;
						remove_Electrical_imaging_event_from_list(
							trace->first_eimaging_event,event);
						/*if we have a map, update it */
						if (mapping&&mapping->map)
						{
							update_map_from_manual_time_update(mapping);
						}
						/* if the deleted event was the current one, and there's an event*/
						/* remaining, make it the current event*/
						if (is_current_event&&(event=*trace->first_eimaging_event))
						{
							event->is_current_event=1;
							marker=SCALE_X(event->time,start_analysis_interval,
								axes_left,x_scale);
							reconcile_Electrical_imaging_event_marker(&marker,event,x_scale,
								axes_left,start_analysis_interval);
							time=(int)((float)((times)[event->time])*1000./frequency+0.5);
							sprintf(time_string,"%d",time);
							write_marker_time(time_string,marker,axes_left,
								axes_width,axes_top,signal_drawing_information->font,
								graphics_context,display,drawing_area,drawing);
						}
					}/* else if (working_button==Button3) */
				}/* if (found) */
				else
				{
					/* add an event */
					if (working_button==Button3)
					{
						erase_Electrical_imaging_event_markers_time_text(trace,
							buffer,x_scale,signal_drawing_information,display);
						/* create and add event */
						event_time=SCALE_X(pointer_x,axes_left,start_analysis_interval,
							1/x_scale);
						event=create_Electrical_imaging_event(event_time);
						event->is_current_event=1;
						add_Electrical_imaging_event_to_sorted_list(
							trace->first_eimaging_event,event);
						/* draw graphic */
						draw_Electrical_imaging_event_marker(event,end_analysis_interval,
							start_analysis_interval,axes_top,axes_height,axes_left,axes_width,
							drawing_area,drawing,signal_drawing_information,buffer);
						/*if we have a map, update it */
						if (mapping&&mapping->map)
						{
							update_map_from_manual_time_update(mapping);
						}
					}/*if (working_button==Button3)*/
				}/* if (found) */
			}/* if ((pointer_x>=axes_left-pointer_sensitivity) */
		}/* if ((ButtonPress==callback->e */
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"move_add_remove_Electrical_imaging_event. invalid argument");
	}
	LEAVE;

	return (return_code);
} /* move_add_remove_Electrical_imaging_event */

static void update_eimaging_evnts_frm_dlg(Widget widget,
	XtPointer analysis_work_area,	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2001

DESCRIPTION :
Updates the electrical imaging event time settings based on the dialog and
redraws if necessary.
Needs to be here in analysis_work_area as need to have access to map to do
update_map_from_manual_time_update
==============================================================================*/
{
	int current_event_time,found,step;
	struct Analysis_work_area *analysis;
	struct Cardiac_interval *interval;
	struct Device *cardiac_interval_device;
	struct Electrical_imaging_event  *event;
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;

	ENTER(update_eimaging_evnts_frm_dlg);
	USE_PARAMETER(call_data);
	cardiac_interval_device=(struct Device *)NULL;
	eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	trace=(struct Trace_window *)NULL;
	event=(struct Electrical_imaging_event *)NULL;
	interval=(struct Cardiac_interval *)NULL;
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(trace=analysis->trace)&&
		(eimaging_time_dialog=trace->area_3.eimaging_time_dialog)&&
		(cardiac_interval_device=trace->cardiac_interval_device)&&
		(buffer=get_Device_signal_buffer(cardiac_interval_device)))
	{
		if (eimaging_time_dialog->settings_changed)
		{
			step=(int)(((eimaging_time_dialog->marker_period*1000)/buffer->frequency)+
				0.5);
			switch (eimaging_time_dialog->reference_event)
			{
				case EVENT_P_WAVE_START:
				case EVENT_P_WAVE_PEAK_OR_TROUGH:
				case EVENT_P_WAVE_END:
				{
					if (interval=find_Cardiac_interval_in_list_given_type(
						trace->first_interval,P_WAVE_INTERVAL))
					{
						switch (eimaging_time_dialog->reference_event)
						{
							case EVENT_P_WAVE_START:
							{
								current_event_time=interval->start_time;
							} break;
							case EVENT_P_WAVE_PEAK_OR_TROUGH:
							{
								current_event_time=interval->peak_or_trough_time;
							} break;
							case EVENT_P_WAVE_END:
							{
								current_event_time=interval->end_time;
							} break;
						}
						create_Electrical_imaging_events_from_time(trace,current_event_time,step);
					}
				} break;
				case EVENT_QRS_WAVE_START:
				case EVENT_QRS_WAVE_PEAK_OR_TROUGH:
				case EVENT_QRS_WAVE_END:
				{
					if (interval=find_Cardiac_interval_in_list_given_type(trace->first_interval,
						QRS_WAVE_INTERVAL))
					{
						switch (eimaging_time_dialog->reference_event)
						{
							case EVENT_QRS_WAVE_START:
							{
								current_event_time=interval->start_time;
							} break;
							case EVENT_QRS_WAVE_PEAK_OR_TROUGH:
							{
								current_event_time=interval->peak_or_trough_time;
							} break;
							case EVENT_QRS_WAVE_END:
							{
								current_event_time=interval->end_time;
							} break;
						}
						create_Electrical_imaging_events_from_time(trace,current_event_time,step);
					}
				} break;
				case EVENT_T_WAVE_START:
				case EVENT_T_WAVE_PEAK_OR_TROUGH:
				case EVENT_T_WAVE_END:
				{
					if (interval=find_Cardiac_interval_in_list_given_type(trace->first_interval,
						T_WAVE_INTERVAL))
					{
						switch (eimaging_time_dialog->reference_event)
						{
							case EVENT_T_WAVE_START:
							{
								current_event_time=interval->start_time;
							} break;
							case EVENT_T_WAVE_PEAK_OR_TROUGH:
							{
								current_event_time=interval->peak_or_trough_time;
							} break;
							case EVENT_T_WAVE_END:
							{
								current_event_time=interval->end_time;
							} break;
						}
						create_Electrical_imaging_events_from_time(trace,current_event_time,step);
					}
				} break;
				case EVENT_CURRENT:
				{
					found=0;
					if (event=*trace->first_eimaging_event)
					{
						/* find the current event */
						while (event&&(!found))
						{
							if (event->is_current_event)
							{
								found=1;
							}
							else
							{
								event=event->next;
							}
						}
						if (!found)
						{
							/* if no current event, make the first event the current event */
							event=*trace->first_eimaging_event;
							event->is_current_event=1;
						}
						/*store the current event*/
						current_event_time=event->time;
						create_Electrical_imaging_events_from_time(trace,current_event_time,
							step);
					}
				} break;
				case EVENT_CLEAR:
				{
					/* just destroy events  */
					destroy_Electrical_imaging_event_list(trace->first_eimaging_event);
				} break;
			}/* switch (eimaging_time_dialog->reference_event) */
			/*if we have a map, update it */
			if ((analysis->mapping_window)&&(analysis->mapping_window->map))
			{
				update_map_from_manual_time_update(analysis->mapping_window);
			}
		}/* if (eimaging_time_dialog->settings_changed) */
		/* we've now dealt with any changes. */
		eimaging_time_dialog->settings_changed=0;
		if (widget==eimaging_time_dialog->ok_button)
		{
			/* close the eimaging_time_dialog  */
			close_eimaging_time_dialog((Widget)NULL,(XtPointer)eimaging_time_dialog,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
		}
		if (widget==eimaging_time_dialog->apply_button)
		{
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			configure_eimaging_time_dialog_marker_menu(trace);
		}
	}
	LEAVE;
} /* update_eimaging_evnts_frm_dlg */

static void select_trace_1_drawing_area(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2003

DESCRIPTION :
???DB.  Update comment ?
The callback for modifying the search interval in the trace enlarge drawing
area.
???DB.  This should be in trace_window.c and the analysis_work_area calls
should be done as a callback from the trace_window.
==============================================================================*/
{
	char edit_box;
	Cursor cursor;
	Display *display;
	enum Event_detection_algorithm detection;
	enum Moving_status moving;
	float frequency,x_scale;
	int axes_bottom,axes_left,axes_right,axes_top,axes_width,box_range,
		datum,*divisions,end_analysis_interval,event_number,i,initial_marker,
		left_box,left_edit_box,marker,minimum_box_range,
		number_of_events,pointer_sensitivity,pointer_x,pointer_y,
		potential_time,*previous_divisions,previous_left_box,previous_left_edit_box,
		previous_marker,previous_pointer_x,previous_right_box,
		previous_right_edit_box,right_box,right_edit_box,start_analysis_interval,
		temp,*temp_divisions,temp_2,*times,width;
	GC marker_graphics_context;
	/*???Needed because of problem with X driver for the Color Graphics Display
		Adapter (#2770) */
	Pixmap pixel_map;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Event *event;
	struct Map *map;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Time_keeper *time_keeper;
	struct Trace_window_area_1 *trace_area_1;
	struct User_interface *user_interface;
	unsigned int working_button;
	Window working_window;
	XButtonEvent *button_event;
	XEvent xevent;
	XmDrawingAreaCallbackStruct *callback;

	ENTER(select_trace_1_drawing_area);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information)&&
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device))&&(times=buffer->times)&&
		(analysis->trace))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				switch (*(analysis->trace->analysis_mode))
				{
					case ELECTRICAL_IMAGING:
					{
						move_Cardiac_interval(callback,highlight_device,analysis->trace,
							analysis->signal_drawing_information,
							analysis->user_interface,analysis->pointer_sensitivity);
					} break;
					case EVENT_DETECTION: case BEAT_AVERAGING:
					{
						if ((callback->event)&&(ButtonPress==callback->event->type))
						{
							display=User_interface_get_display(user_interface);
							pointer_sensitivity=analysis->pointer_sensitivity;
							detection=analysis->detection;
							button_event= &(callback->event->xbutton);
							pointer_x=button_event->x;
							pointer_y=button_event->y;
							working_button=button_event->button;
							trace_area_1= &(analysis->trace->area_1);
							axes_left=trace_area_1->axes_left;
							axes_width=trace_area_1->axes_width;
							axes_right=axes_left+axes_width-1;
							axes_top=trace_area_1->axes_top;
							axes_bottom=axes_top+(trace_area_1->axes_height)-1;
							divisions=(int *)NULL;
							previous_divisions=(int *)NULL;
							if ((pointer_x>=axes_left-pointer_sensitivity)&&
								(pointer_x<=axes_right+pointer_sensitivity)&&
								(pointer_y>=axes_top-pointer_sensitivity)&&
								(pointer_y<=axes_bottom+pointer_sensitivity))
							{
								datum=analysis->datum;
								potential_time=analysis->potential_time;
								start_analysis_interval=buffer->start;
								end_analysis_interval=buffer->end;
								x_scale=SCALE_FACTOR(end_analysis_interval-
									start_analysis_interval,axes_right-axes_left);
								frequency=buffer->frequency;
								/* determine if datum has been selected */
								initial_marker=SCALE_X(datum,start_analysis_interval,axes_left,
									x_scale);
								if ((pointer_x>=initial_marker-pointer_sensitivity)&&
									(pointer_x<=initial_marker+pointer_sensitivity))
								{
									moving=MOVING_DATUM_MARKER;
									marker_graphics_context=(signal_drawing_information->
										graphics_context).datum_colour;
								}
								else
								{
									/* determine if the potential time has been selected */
									initial_marker=SCALE_X(potential_time,start_analysis_interval,
										axes_left,x_scale);
									if ((pointer_x>=initial_marker-pointer_sensitivity)&&
										(pointer_x<=initial_marker+pointer_sensitivity))
									{
										moving=MOVING_POTENTIAL_TIME_MARKER;
										marker_graphics_context=(signal_drawing_information->
											graphics_context).potential_time_colour;
									}
									else
									{
										moving=MOVING_NONE;
									}
								}
								if ((MOVING_DATUM_MARKER==moving)||
									(MOVING_POTENTIAL_TIME_MARKER==moving))
								{

									cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
									/* change the cursor */
									XDefineCursor(display,XtWindow(trace_area_1->drawing_area),cursor);
									XmUpdateDisplay(trace_area_1->drawing_area);
									XWarpPointer(display,None,None,0,0,0,0,
										initial_marker-pointer_x,0);
									pointer_x=initial_marker;
									marker=initial_marker;
									working_window=XtWindow(trace_area_1->drawing_area);
									pixel_map=trace_area_1->drawing->pixel_map;
									while (MOVING_NONE!=moving)
									{
										XNextEvent(display,&xevent);
										switch (xevent.type)
										{
											case MotionNotify:
											{
												previous_marker=marker;
												/* reduce the number of motion events displayed */
												while (True==XCheckMaskEvent(display,ButtonMotionMask,
													&xevent));
												pointer_x=xevent.xmotion.x;
												pointer_y=xevent.xmotion.y;
												if ((xevent.xmotion.window==working_window)&&
													(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
												{
													if (pointer_x<axes_left)
													{
														marker=axes_left;
													}
													else
													{
														if (pointer_x>axes_right)
														{
															marker=axes_right;
														}
														else
														{
															marker=pointer_x;
														}
													}
													if (marker!=previous_marker)
													{
														/* clear the old marker */
														XDrawLine(display,pixel_map,
															marker_graphics_context,previous_marker,
															axes_top,previous_marker,axes_bottom);
														XDrawLine(display,working_window,
															marker_graphics_context,previous_marker,
															axes_top,previous_marker,axes_bottom);
														/* draw the new marker */
														XDrawLine(display,pixel_map,
															marker_graphics_context,marker,axes_top,marker,
															axes_bottom);
														XDrawLine(display,working_window,
															marker_graphics_context,marker,axes_top,
															marker,axes_bottom);
													}
												}
											} break;
											case ButtonPress:
											{
												if (xevent.xbutton.button==working_button)
												{
													display_message(ERROR_MESSAGE,
														"select_trace_1_drawing_area.  Unexpected button press");
													moving=MOVING_NONE;
												}
											} break;
											case ButtonRelease:
											{
												if (xevent.xbutton.button==working_button)
												{
													if (xevent.xbutton.window==working_window)
													{
														if (marker!=initial_marker)
														{
															switch (moving)
															{
																case MOVING_DATUM_MARKER:
																{
																	datum=SCALE_X(marker,axes_left,
																		start_analysis_interval,1/x_scale);
																} break;
																case MOVING_POTENTIAL_TIME_MARKER:
																{
																	potential_time=SCALE_X(marker,axes_left,
																		start_analysis_interval,1/x_scale);
																} break;
															}

															XDrawLine(display,pixel_map,
																marker_graphics_context,marker,axes_top,
																marker,axes_bottom);
															XDrawLine(display,working_window,
																marker_graphics_context,marker,axes_top,
																marker,axes_bottom);
																/* update time objects */
															switch (moving)
															{
																case MOVING_POTENTIAL_TIME_MARKER:
																{
																	/*???SAB.  This conversion to time_keeper
																		time should be much more robust.  The
																		frequency is not guaranteed to divide the
																		potential time and this takes no account
																		of time transformations */
																	analysis->trace_update_flags |=
																		TRACE_1_NO_POTENTIAL_ERASE;
																	Time_keeper_request_new_time(
																		Time_object_get_time_keeper(
																		analysis->potential_time_object),
																		((double)times[potential_time]/frequency));
																} break;
																case MOVING_DATUM_MARKER:
																{
																	analysis->trace_update_flags |=
																		TRACE_1_NO_DATUM_ERASE;
																	Time_object_set_current_time_privileged(
																		analysis->datum_time_object,
																		(double)datum);
																	Time_object_notify_clients_privileged(
																		analysis->datum_time_object);
																} break;
															}
														}
													}
													else
													{
														if (marker!=initial_marker)
														{
																/* clear the old marker */
															XDrawLine(display,pixel_map,
																marker_graphics_context,marker,axes_top,
																marker,axes_bottom);
															XDrawLine(display,working_window,
																marker_graphics_context,marker,axes_top,
																marker,axes_bottom);
																/* draw the new marker */
															XDrawLine(display,pixel_map,
																marker_graphics_context,initial_marker,
																axes_top,initial_marker,axes_bottom);
															XDrawLine(display,working_window,
																marker_graphics_context,initial_marker,
																axes_top,initial_marker,axes_bottom);
														}
													}
													moving=MOVING_NONE;
												}
											} break;
											default:
											{
												XtDispatchEvent(&xevent);
											}
										}
									}
									XUndefineCursor(display,XtWindow(trace_area_1->drawing_area));
									XFreeCursor(display,cursor);
								}/* if ((MOVING_DATUM_MARKER==moving)||(MOVING_POTENTIAL_TIME_MARKER==moving))*/
								else
								{
									left_box=trace_area_1->enlarge.left_box;
									right_box=trace_area_1->enlarge.right_box;
									left_edit_box=trace_area_1->enlarge.left_edit_box;
									right_edit_box=trace_area_1->enlarge.right_edit_box;
									number_of_events=analysis->number_of_events;
									if (trace_area_1->enlarge.divisions)
									{
										divisions=(int *)NULL;
										previous_divisions=(int *)NULL;
										if (ALLOCATE(divisions,int,number_of_events-1)&&
											ALLOCATE(previous_divisions,int,number_of_events-1))
										{
											for (i=number_of_events-2;i>=0;i--)
											{
												divisions[i]=(trace_area_1->enlarge.divisions)[i];
												previous_divisions[i]=divisions[i];
											}
										}
										else
										{
											DEALLOCATE(divisions);
											DEALLOCATE(previous_divisions);
										}
									}
									box_range=right_box-left_box;
									edit_box=0;
									if ((pointer_x>=left_box-pointer_sensitivity)&&
										(pointer_x<=right_box+pointer_sensitivity))
									{
										if (pointer_x<=left_box+pointer_sensitivity)
										{
											moving=MOVING_LEFT;
											XWarpPointer(display,None,None,0,0,0,0,left_box-pointer_x,
												0);
											pointer_x=left_box;
											cursor=XCreateFontCursor(display,XC_left_side);
										}
										else
										{
											if (pointer_x>=right_box-pointer_sensitivity)
											{
												moving=MOVING_RIGHT;
												XWarpPointer(display,None,None,0,0,0,0,
													right_box-pointer_x,0);
												pointer_x=right_box;
												cursor=XCreateFontCursor(display,XC_right_side);
											}
											else
											{
												if ((pointer_y<=axes_top+pointer_sensitivity)||
													(pointer_y>=axes_bottom-pointer_sensitivity))
												{
													moving=MOVING_BOX;
													cursor=XCreateFontCursor(display,XC_dotbox);
												}
												else
												{
													if ((EDA_INTERVAL==detection)||
														(BEAT_AVERAGING==*(analysis->trace->analysis_mode)))
													{
														temp=pointer_x-left_box;
														event_number=
															((temp*number_of_events)/box_range)+1;
														if ((temp<=(((event_number-1)*box_range)/
															number_of_events)+pointer_sensitivity)||(temp>=
																((event_number*box_range)/number_of_events)-
																pointer_sensitivity))
														{
															moving=MOVING_NONE;
														}
														else
														{
															moving=MOVING_EVENT_MARKER;
															analysis->event_number=event_number;
															trace_change_event(analysis->trace);
														}
													}
												}
											}
										}
									}
									else
									{
										moving=MOVING_NONE;
									}
									if ((MOVING_NONE==moving)&&((EDA_LEVEL==detection)||
										(EDA_THRESHOLD==detection)))
									{
										if (highlight_device->signal)
										{
											event=highlight_device->signal->first_event;
										}
										else
										{
											event=(struct Event *)NULL;
										}
										while (event&&(pointer_x>(initial_marker=
											SCALE_X(event->time,start_analysis_interval,
												axes_left,x_scale))+pointer_sensitivity))
										{
											event=event->next;
										}
										if (event&&(pointer_x>=initial_marker-pointer_sensitivity))
										{
											moving=MOVING_EVENT_MARKER;
											analysis->event_number=event->number;
											trace_change_event(analysis->trace);
										}
										else
										{
											left_box=trace_area_1->enlarge.left_edit_box;
											right_box=trace_area_1->enlarge.right_edit_box;
											box_range=right_box-left_box;
											edit_box=1;
											if ((pointer_x>=left_box-pointer_sensitivity)&&
												(pointer_x<=right_box+pointer_sensitivity))
											{
												if (pointer_x<=left_box+pointer_sensitivity)
												{
													moving=MOVING_LEFT;
													XWarpPointer(display,None,None,0,0,0,0,
														left_box-pointer_x,0);
													pointer_x=left_box;
													cursor=XCreateFontCursor(display,XC_left_side);
												}
												else
												{
													if (pointer_x>=right_box-pointer_sensitivity)
													{
														moving=MOVING_RIGHT;
														XWarpPointer(display,None,None,0,0,0,0,
															right_box-pointer_x,0);
														pointer_x=right_box;
														cursor=XCreateFontCursor(display,XC_right_side);
													}
													else
													{
														moving=MOVING_BOX;
														cursor=XCreateFontCursor(display,XC_dotbox);
													}
												}
											}
										}
									}
									if ((MOVING_LEFT==moving)||(MOVING_RIGHT==moving)||
										(MOVING_BOX==moving))
									{
										event_number=analysis->event_number;
										number_of_events=analysis->number_of_events;
										/* stop the time keeper */
										time_keeper=(struct Time_keeper *)NULL;
										if ((analysis->potential_time_object)&&(time_keeper=
											Time_object_get_time_keeper(analysis->
											potential_time_object)))
										{
											if (Time_keeper_is_playing(time_keeper))
											{
												Time_keeper_stop(time_keeper);
											}
										}
										/*change the cursor*/
										XDefineCursor(display,XtWindow(trace_area_1->drawing_area),
											cursor);
										XmUpdateDisplay(trace_area_1->drawing_area);
										minimum_box_range=2*pointer_sensitivity+1;
										working_window=XtWindow(trace_area_1->drawing_area);
										while (moving!=MOVING_NONE)
										{
											XNextEvent(display,&xevent);
											switch (xevent.type)
											{
												case MotionNotify:
												{
													previous_left_box=left_box;
													previous_right_box=right_box;
													previous_left_edit_box=left_edit_box;
													previous_right_edit_box=right_edit_box;
													temp_divisions=previous_divisions;
													previous_divisions=divisions;
													divisions=temp_divisions;
													previous_pointer_x=pointer_x;
													/* reduce the number of motion events displayed */
													while (True==XCheckMaskEvent(display,
														ButtonMotionMask,&xevent));
													pointer_x=xevent.xmotion.x;
													pointer_y=xevent.xmotion.y;
													if ((xevent.xmotion.window==working_window)&&
														(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
													{
														switch (moving)
														{
															case MOVING_LEFT:
															{
																if (pointer_x<axes_left)
																{
																	left_box=axes_left;
																}
																else
																{
																	if (pointer_x>
																		(temp=right_box-minimum_box_range))
																	{
																		left_box=temp;
																	}
																	else
																	{
																		left_box=pointer_x;
																	}
																}
																if (divisions)
																{
																	for (i=number_of_events-2;i>=0;i--)
																	{
																		if (left_box>previous_divisions[i])
																		{
																			divisions[i]=left_box;
																		}
																		else
																		{
																			divisions[i]=previous_divisions[i];
																		}
																	}
																}
															} break;
															case MOVING_RIGHT:
															{
																if (pointer_x>axes_right)
																{
																	right_box=axes_right;
																}
																else
																{
																	if (pointer_x<
																		(temp=left_box+minimum_box_range))
																	{
																		right_box=temp;
																	}
																	else
																	{
																		right_box=pointer_x;
																	}
																}
																if (divisions)
																{
																	for (i=number_of_events-2;i>=0;i--)
																	{
																		if (right_box<previous_divisions[i])
																		{
																			divisions[i]=right_box;
																		}
																		else
																		{
																			divisions[i]=previous_divisions[i];
																		}
																	}
																}
															} break;
															case MOVING_BOX:
															{
																if (pointer_x<axes_left)
																{
																	pointer_x=axes_left;
																	left_box=axes_left;
																	right_box=left_box+box_range;
																}
																else
																{
																	if (pointer_x>axes_right)
																	{
																		pointer_x=axes_right;
																		right_box=axes_right;
																		left_box=right_box-box_range;
																	}
																	else
																	{
																		left_box=
																			left_box+pointer_x-previous_pointer_x;
																		right_box=left_box+box_range;
																		if (left_box<axes_left)
																		{
																			left_box=axes_left;
																			right_box=left_box+box_range;
																		}
																		else
																		{
																			if (right_box>axes_right)
																			{
																				right_box=axes_right;
																				left_box=right_box-box_range;
																			}
																		}
																	}
																}
																if (divisions)
																{
																	for (i=number_of_events-2;i>=0;i--)
																	{
																		divisions[i]=previous_divisions[i]-
																			left_box-previous_left_box;
																	}
																}
															} break;
														}
														if ((left_box!=previous_left_box)||
															(right_box!=previous_right_box))
														{
															if (divisions)
															{
																if (event_number>1)
																{
																	left_edit_box=divisions[event_number-2];
																}
																else
																{
																	left_edit_box=left_box;
																}
																if (event_number<number_of_events)
																{
																	right_edit_box=divisions[event_number-1];
																}
																else
																{
																	right_edit_box=right_box;
																}
															}
															else
															{
																width=right_box-left_box;
																left_edit_box=left_box+((event_number-1)*
																	width)/number_of_events;
																right_edit_box=left_box+(event_number*width)/
																	number_of_events;
															}
															if (EDA_INTERVAL==detection)
															{
																/* clear the old box */
																width=previous_right_box-previous_left_box,
																	draw_search_box(previous_left_box,axes_top,
																		width,trace_area_1->axes_height,detection,
																		number_of_events,previous_divisions,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																draw_highlight_event_box(
																	previous_left_edit_box,axes_top,
																	previous_right_edit_box-
																	previous_left_edit_box,
																	trace_area_1->axes_height,detection,
																	trace_area_1->drawing_area,
																	trace_area_1->drawing,
																	signal_drawing_information);
																/* draw the new box */
																width=right_box-left_box;
																draw_search_box(left_box,axes_top,width,
																	trace_area_1->axes_height,detection,
																	number_of_events,divisions,
																	trace_area_1->drawing_area,
																	trace_area_1->drawing,
																	signal_drawing_information);
																draw_highlight_event_box(left_edit_box,
																	axes_top,right_edit_box-left_edit_box,
																	trace_area_1->axes_height,detection,
																	trace_area_1->drawing_area,
																	trace_area_1->drawing,
																	signal_drawing_information);
															}
															else
															{
																if (edit_box)
																{
																	/* clear the old box */
																	draw_highlight_event_box(previous_left_box,
																		axes_top,
																		previous_right_box-previous_left_box,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																	/* draw the new box */
																	draw_highlight_event_box(left_box,axes_top,
																		right_box-left_box,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																}
																else
																{
																	/* clear the old box */
																	draw_search_box(previous_left_box,axes_top,
																		previous_right_box-previous_left_box,
																		trace_area_1->axes_height,detection,
																		number_of_events,previous_divisions,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																	/* draw the new box */
																	draw_search_box(left_box,axes_top,
																		right_box-left_box,
																		trace_area_1->axes_height,detection,
																		number_of_events,divisions,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																}
															}
														}
													}
												} break;
												case ButtonPress:
												{
													if (xevent.xbutton.button==working_button)
													{
														display_message(ERROR_MESSAGE,
															"select_trace_1_drawing_area.  Unexpected button press");
														moving=MOVING_NONE;
													}
												} break;
												case ButtonRelease:
												{
													if (xevent.xbutton.button==working_button)
													{
														if (xevent.xbutton.window==working_window)
														{
															if ((EDA_INTERVAL==detection)||!edit_box)
															{
																if ((left_box!=trace_area_1->enlarge.
																	left_box)||(right_box!=trace_area_1->
																		enlarge.right_box))
																{
																	if (left_box!=trace_area_1->enlarge.
																		left_box)
																	{
																		temp=SCALE_X(left_box,axes_left,
																			start_analysis_interval,1/x_scale);
																		if (analysis->search_interval_divisions)
																		{
																			if (MOVING_BOX==moving)
																			{
																				temp_2=temp-
																					analysis->start_search_interval;
																				for (i=number_of_events-2;
																						i>=0;i--)
																				{
																					(analysis->
																						search_interval_divisions)[i] +=
																						temp_2;
																				}
																			}
																			else
																			{
																				/* must be MOVING_LEFT */
																				i=0;
																				while ((i<number_of_events-1)&&
																					((analysis->
																						search_interval_divisions)[i]<temp))
																				{
																					(analysis->
																						search_interval_divisions)[i]=
																						temp;
																					i++;
																				}
																			}
																		}
																		analysis->start_search_interval=temp;
																		trace_area_1->enlarge.left_box=left_box;
																	}
																	if (right_box!=
																		trace_area_1->enlarge.right_box)
																	{
																		temp=SCALE_X(right_box,axes_left,
																			start_analysis_interval,1/x_scale);
																		if (analysis->search_interval_divisions)
																		{
																			/* have already done adjusting (above)
																				for MOVING_BOX */
																			if (MOVING_BOX!=moving)
																			{
																				/* must be MOVING_RIGHT */
																				i=number_of_events-2;
																				while ((i>=0)&&((analysis->
																					search_interval_divisions)[i]>temp))
																				{
																					(analysis->
																						search_interval_divisions)[i]=
																						temp;
																					i--;
																				}
																			}
																		}
																		analysis->end_search_interval=temp;
																		trace_area_1->enlarge.right_box=right_box;
																	}
																	if (divisions&&
																		(trace_area_1->enlarge.divisions))
																	{
																		for (i=number_of_events-2;i>=0;i--)
																		{
																			(trace_area_1->enlarge.divisions)[i]=
																				divisions[i];
																		}
																	}
																	if (EDA_INTERVAL==detection)
																	{
																		if (analysis->search_interval_divisions)
																		{
																			temp=analysis->start_search_interval;
																			temp_2=(analysis->end_search_interval)-
																				temp;
																			if (event_number>1)
																			{
																				trace_area_1->enlarge.left_edit_box=
																					(trace_area_1->enlarge.left_box)+
																					(((analysis->
																						search_interval_divisions)[
																							event_number-2]-temp)*
																						((trace_area_1->enlarge.right_box)-
																							(trace_area_1->enlarge.left_box)))/
																					temp_2;
																			}
																			else
																			{
																				trace_area_1->enlarge.left_edit_box=
																					trace_area_1->enlarge.left_box;
																			}
																			if (event_number<number_of_events)
																			{
																				trace_area_1->enlarge.right_edit_box=
																					(trace_area_1->enlarge.left_box)+
																					(((analysis->
																						search_interval_divisions)[
																							event_number-1]-temp)*
																						((trace_area_1->enlarge.right_box)-
																							(trace_area_1->enlarge.left_box)))/
																					temp_2;
																			}
																			else
																			{
																				trace_area_1->enlarge.right_edit_box=
																					trace_area_1->enlarge.right_box;
																			}
																		}
																		else
																		{
																			trace_area_1->enlarge.left_edit_box=
																				(trace_area_1->enlarge.left_box)+
																				((event_number-1)*
																					((trace_area_1->enlarge.right_box)-
																						(trace_area_1->enlarge.left_box)))/
																				number_of_events;
																			trace_area_1->enlarge.right_edit_box=
																				(trace_area_1->enlarge.left_box)+
																				(event_number*
																					((trace_area_1->enlarge.right_box)-
																						(trace_area_1->enlarge.left_box)))/
																				number_of_events;
																		}
																	}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
																	if (EDA_INTERVAL==detection)
																	{
																		/* clear the present markers */
																		draw_all_markers(0,0,analysis);
																		destroy_all_events(analysis->rig);
																	}
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
																	if ((
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
																		((EDA_INTERVAL==analysis->detection)&&
																		((SINGLE_ACTIVATION==analysis->map_type)||
																		(MULTIPLE_ACTIVATION==analysis->map_type)||
																		(ACTIVATION_POTENTIAL==
																		analysis->map_type)))||
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
																		(INTEGRAL==analysis->map_type))&&
																		(analysis->mapping_window)&&
																		(map=analysis->mapping_window->map))
																	{
																		/* update the mapping window */
																		analysis->map_type=NO_MAP_FIELD;
																		map->colour_option=HIDE_COLOUR;
																		map->contours_option=HIDE_CONTOURS;
																		map->electrodes_label_type=
																			SHOW_ELECTRODE_NAMES;
																		/* clear the colour map */
																		map->activation_front= -1;
																		update_mapping_drawing_area(
																			analysis->mapping_window,2);
																		update_mapping_colour_or_auxili(
																			analysis->mapping_window);
																		XtSetSensitive(analysis
																			->mapping_window->animate_button,
																			False);
																	}
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
																	if (EDA_INTERVAL==detection)
																	{
																		XtSetSensitive(analysis->window->
																			file_menu.save_times_button,False);
																		XtSetSensitive(analysis->window->map_menu.
																			single_activation_button,False);
																		XtSetSensitive(analysis->window->map_menu.
																			multiple_activation_button,False);
																		XtSetSensitive(analysis->window->map_menu.
																			activation_potential_button,False);
																		analysis->calculate_events=0;
																	}
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
																}
															}
															else
															{
																if ((left_box!=
																	trace_area_1->enlarge.left_edit_box)||
																	(right_box!=
																		trace_area_1->enlarge.right_edit_box))
																{
																	if (left_box!=
																		trace_area_1->enlarge.left_edit_box)
																	{
																		analysis->trace->area_3.edit.first_data=
																			SCALE_X(left_box,axes_left,
																				start_analysis_interval,1/x_scale);
																		trace_area_1->enlarge.left_edit_box=
																			left_box;
																	}
																	if (right_box!=
																		trace_area_1->enlarge.right_edit_box)
																	{
																		analysis->trace->area_3.edit.last_data=
																			SCALE_X(right_box,axes_left,
																				start_analysis_interval,1/x_scale);
																		trace_area_1->enlarge.right_edit_box=
																			right_box;
																	}
																}
															}
															if (EDA_INTERVAL==detection)
															{
																trace_update_edit_interval(analysis->trace);
															}
															redraw_trace_3_drawing_area((Widget)NULL,
																(XtPointer)(analysis->trace),
																(XtPointer)NULL);
															trace_update_signal_controls(analysis->trace);
														}
														else
														{
															if ((EDA_INTERVAL==detection)||!edit_box)
															{
																if ((left_box!=trace_area_1->enlarge.
																	left_box)||(right_box!=trace_area_1->
																		enlarge.right_box))
																{
																	/* clear the new box */
																	width=right_box-left_box;
																	draw_search_box(left_box,axes_top,width,
																		trace_area_1->axes_height,detection,
																		number_of_events,divisions,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																	if (EDA_INTERVAL==detection)
																	{
																		draw_highlight_event_box(left_edit_box,
																			axes_top,right_edit_box-left_edit_box,
																			trace_area_1->axes_height,detection,
																			trace_area_1->drawing_area,
																			trace_area_1->drawing,
																			signal_drawing_information);
																	}
																	/* draw the old box */
																	draw_search_box(
																		trace_area_1->enlarge.left_box,axes_top,
																		trace_area_1->enlarge.right_box-
																		trace_area_1->enlarge.left_box,
																		trace_area_1->axes_height,detection,
																		number_of_events,
																		trace_area_1->enlarge.divisions,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																	if (EDA_INTERVAL==detection)
																	{
																		draw_highlight_event_box(
																			trace_area_1->enlarge.left_edit_box,
																			axes_top,
																			trace_area_1->enlarge.right_edit_box-
																			trace_area_1->enlarge.left_edit_box,
																			trace_area_1->axes_height,detection,
																			trace_area_1->drawing_area,
																			trace_area_1->drawing,
																			signal_drawing_information);
																	}
																}
															}
															else
															{
																if ((left_box!=
																	trace_area_1->enlarge.left_edit_box)||
																	(right_box!=
																		trace_area_1->enlarge.right_edit_box))
																{
																	/* clear the new box */
																	draw_highlight_event_box(left_box,axes_top,
																		right_box-left_box,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																	draw_highlight_event_box(
																		trace_area_1->enlarge.left_edit_box,
																		axes_top,
																		trace_area_1->enlarge.right_edit_box-
																		trace_area_1->enlarge.left_edit_box,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
																}
															}
														}
														moving=MOVING_NONE;
													}
												} break;
												default:
												{
													XtDispatchEvent(&xevent);
												}
											}
										}
										XUndefineCursor(display,
											XtWindow(trace_area_1->drawing_area));
										XFreeCursor(display,cursor);
										if (time_keeper&&(analysis->mapping_window))
										{
											mapping_window_update_time_limits(
												analysis->mapping_window);
										}
									}
								}
							}
							DEALLOCATE(divisions);
							DEALLOCATE(previous_divisions);
						}
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_trace_1_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_trace_1_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_trace_1_drawing_area.  analysis work area missing");
	}
	LEAVE;
} /* select_trace_1_drawing_area */

static void select_trace_3_drawing_area(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 April 2002

DESCRIPTION :
???DB.  Change comment ?
The callback for moving the event marker in the trace edit drawing area.
???DB.  This should be in trace_window.c and the analysis_work_area calls
should be done as a callback from the trace_window.
==============================================================================*/
{
	char number_string[20],number_string2[20];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float frequency,initial_value,new_max,new_min,signal_max,signal_min,x_scale,
		y_scale;
	int ascent,axes_bottom,axes_left,axes_right,axes_top,axes_width,datum,descent,
		device_number,direction,drawing_height,drawing_width,end_analysis_interval,
		event_number,event_time,first_data,i,initial_marker,
		last_data,length,length2,marker,number_of_columns,number_of_rows,
		pointer_sensitivity,pointer_x,pointer_y,potential_time,
		previous_marker,signals_axes_height,signals_axes_left,
		signals_axes_top,signals_axes_width,start_analysis_interval,*times,
		x_axis_y_marker,xpos,x_string,x_string2,y_limit,ypos,y_string,y_string2;
	GC event_graphics_context,marker_graphics_context;
	/*???Needed because of problem with X driver for the Color Graphics Display
		Adapter (#2770) */
	GC event_graphics_context_text,marker_graphics_context_text;
	Pixmap pixel_map,signals_pixel_map;
	struct Analysis_work_area *analysis;
	struct Channel *channel;
	struct Device **device,*highlight_device,*trace_area_3_device;
	struct Event *current_event,*event,*event_temp;
	struct Region *current_region;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Signals_area *signals;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;
	struct User_interface *user_interface;
	unsigned int working_button;
	Window signals_window,working_window;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XFontStruct *font;
	XmDrawingAreaCallbackStruct *callback;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *signal_maximum_field,*signal_minimum_field;
	struct FE_field_component component;
	struct FE_node_order_info *rig_node_order_info;
	struct Signal_drawing_package	*signal_drawing_package;
#if defined (NEW_CODE)
	struct FE_node 	*highlight_device_node;
	struct FE_node *device_node,*trace_area_3_device_node;
#endif /* defined (NEW_CODE) */
#endif /* defined (UNEMAP_USE_NODES)*/

	ENTER(select_trace_3_drawing_area);
	USE_PARAMETER(widget);
#if defined (UNEMAP_USE_NODES)
	rig_node_order_info=(struct FE_node_order_info *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	rig_node_order_info=(struct FE_node_order_info *)NULL;
	signal_drawing_package=(struct Signal_drawing_package	*)NULL;
#endif /* defined (UNEMAP_USE_NODES)*/
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->trace)&&(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				switch (*(analysis->trace->analysis_mode))
				{
					case ELECTRICAL_IMAGING:
					{
						move_add_remove_Electrical_imaging_event(callback,
							analysis->trace,analysis->signal_drawing_information,
							analysis->user_interface,analysis->pointer_sensitivity,
							analysis->mapping_window);
					} break;
					case EVENT_DETECTION: case BEAT_AVERAGING:
					{
						/* if we're not in beat averaging mode with toggle on, */
						/* update the potential time */
						if (!((BEAT_AVERAGING==(*analysis->trace->analysis_mode))&&
							(True==XmToggleButtonGadgetGetState((analysis->trace->area_3).
							beat_averaging.beat_averaging_toggle))))
						{
							if (analysis->highlight)
							{
								highlight_device= *(analysis->highlight);
							}
							else
							{
								highlight_device=(struct Device *)NULL;
							}
							if ((BEAT_AVERAGING== *(analysis->trace->analysis_mode))&&
								(analysis->trace->valid_processing))
							{
								trace_area_3_device=analysis->trace->processed_device;
							}
							else
							{
								trace_area_3_device=highlight_device;
							}
							if (highlight_device&&trace_area_3_device&&
								(buffer=get_Device_signal_buffer(highlight_device))&&
								(times=buffer->times))
							{
								signal=trace_area_3_device->signal;
								if ((callback->event)&&(ButtonPress==callback->event->type))
								{
									display=User_interface_get_display(user_interface);
									font=signal_drawing_information->font;
									button_event= &(callback->event->xbutton);
									pointer_x=button_event->x;
									pointer_y=button_event->y;
									pointer_sensitivity=analysis->pointer_sensitivity;
									working_button=button_event->button;
									trace_area_3= &(analysis->trace->area_3);
									axes_top=trace_area_3->axes_top;
									axes_bottom=axes_top+(trace_area_3->axes_height)-1;
									axes_left=trace_area_3->axes_left;
									axes_width=trace_area_3->axes_width;
									axes_right=axes_left+axes_width-1;
									first_data=trace_area_3->edit.first_data;
									last_data=trace_area_3->edit.last_data;
									x_scale=
										SCALE_FACTOR(last_data-first_data,axes_right-axes_left);
									event_number=analysis->event_number;
									if (signal)
									{
										current_event=signal->first_event;
										while (current_event&&(current_event->number<event_number))
										{
											current_event=current_event->next;
										}
										if (current_event&&(event_number!=current_event->number))
										{
											current_event=(struct Event *)NULL;
										}
									}
									else
									{
										current_event=(struct Event *)NULL;
									}
									datum=analysis->datum;
									potential_time=analysis->potential_time;
									frequency=buffer->frequency;
									if ((pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
									{
										/* determine if an event has been selected */
										if (signal)
										{
											event=signal->first_event;
											while (event&&(pointer_x>(initial_marker=
												SCALE_X(event->time,first_data,axes_left,x_scale))+
												pointer_sensitivity))
											{
												event=event->next;
											}
										}
										else
										{
											event=(struct Event *)NULL;
										}
										if (event&&(pointer_x>=initial_marker-pointer_sensitivity))
										{
											if (((EDA_LEVEL==analysis->detection)||
												(EDA_THRESHOLD==analysis->detection))&&
												(Button3==working_button))
											{
												/* delete the event */
												moving=MOVING_NONE;
												/* clear the marker */
												draw_event_marker(event,event_number,datum,times,
													frequency,EDIT_AREA_DETAIL,first_data,last_data,
													trace_area_3_device->signal_display_minimum,
													trace_area_3_device->signal_display_maximum,axes_left,
													axes_top,axes_width,trace_area_3->axes_height,
													XtWindow(trace_area_3->drawing_area),
													trace_area_3->drawing->pixel_map,
													signal_drawing_information,user_interface);
												trace_area_1= &(analysis->trace->area_1);
												start_analysis_interval=buffer->start;
												end_analysis_interval=buffer->end;
												draw_event_marker(event,event_number,datum,times,
													frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
													end_analysis_interval,
													highlight_device->signal_display_minimum,
													highlight_device->signal_display_maximum,
													trace_area_1->axes_left,trace_area_1->axes_top,
													trace_area_1->axes_width,trace_area_1->axes_height,
													XtWindow(trace_area_1->drawing_area),
													trace_area_1->drawing->pixel_map,
													signal_drawing_information,user_interface);
												if ((analysis->window)&&(analysis->rig)&&
													(signals= &(analysis->window->signals)))
												{
													signals_axes_left=signals->axes_left;
													signals_axes_top=signals->axes_top;
													signals_axes_width=signals->axes_width;
													signals_axes_height=signals->axes_height;
													signals_window=XtWindow(signals->drawing_area);
													signals_pixel_map=signals->drawing->pixel_map;
													drawing_width=signals->drawing->width;
													drawing_height=signals->drawing->height;
													number_of_rows=signals->number_of_rows;
													number_of_columns=signals->number_of_columns;
													device_number=0;
													device=analysis->rig->devices;
													current_region=get_Rig_current_region(analysis->rig);
													for (i=(analysis->highlight)-device;i>0;
															i--)
													{
														if (!current_region||(current_region==
															(*device)->description->region))
														{
															device_number++;
														}
													}
													switch (signals->layout)
													{
														case SEPARATE_LAYOUT:
														{
															xpos=((device_number/number_of_rows)*
																drawing_width)/number_of_columns;
															ypos=((device_number%number_of_rows)*
																drawing_height)/number_of_rows;
														} break;
														case OVERLAP_LAYOUT:
														{
															xpos=((device_number/number_of_rows)*
																drawing_width)/number_of_columns;
															ypos=((device_number%number_of_rows)*
																drawing_height)/(number_of_rows+3);
														} break;
													}
													xpos += signals_axes_left;
													ypos += signals_axes_top;
													draw_event_marker(event,event_number,datum,times,
														frequency,SIGNAL_AREA_DETAIL,
														start_analysis_interval,end_analysis_interval,
														highlight_device->signal_display_minimum,
														highlight_device->signal_display_maximum,xpos,ypos,
														signals_axes_width,signals_axes_height,
														signals_window,signals_pixel_map,
														signal_drawing_information,user_interface);
												}
												/* free the memory for the event */
												if (event->previous)
												{
													event->previous->next=event->next;
												}
												else
												{
													signal->first_event=event->next;
												}
												if (event_temp=event->next)
												{
													event_temp->previous=event->previous;
													while (event_temp)
													{
														if ((event_temp->number==event_number)||
															(event_temp->number==event_number+1))
														{
															draw_event_marker(event_temp,event_number,datum,
																times,frequency,EDIT_AREA_DETAIL,first_data,
																last_data,trace_area_3_device->signal_display_minimum,
																trace_area_3_device->signal_display_maximum,axes_left,
																axes_top,axes_width,trace_area_3->axes_height,
																XtWindow(trace_area_3->drawing_area),
																trace_area_3->drawing->pixel_map,
																signal_drawing_information,user_interface);
															(event_temp->number)--;
															draw_event_marker(event_temp,event_number,datum,
																times,frequency,EDIT_AREA_DETAIL,first_data,
																last_data,trace_area_3_device->signal_display_minimum,
																trace_area_3_device->signal_display_maximum,axes_left,
																axes_top,axes_width,trace_area_3->axes_height,
																XtWindow(trace_area_3->drawing_area),
																trace_area_3->drawing->pixel_map,
																signal_drawing_information,user_interface);
														}
														else
														{
															(event_temp->number)--;
														}
														event_temp=event_temp->next;
													}
													event->next=(struct Event *)NULL;
												}
												destroy_Event_list(&event);
												trace_update_signal_controls(analysis->trace);
											}
											else
											{
												moving=MOVING_EVENT_MARKER;
												if (event->number!=event_number)
												{
													if (current_event)
													{
														draw_event_marker(current_event,event_number,datum,
															times,frequency,EDIT_AREA_DETAIL,first_data,
															last_data,trace_area_3_device->signal_display_minimum,
															trace_area_3_device->signal_display_maximum,axes_left,
															axes_top,axes_width,trace_area_3->axes_height,
															XtWindow(trace_area_3->drawing_area),
															trace_area_3->drawing->pixel_map,
															signal_drawing_information,user_interface);
													}
													event_number=event->number;
													analysis->event_number=event_number;
													if (current_event)
													{
														draw_event_marker(current_event,event_number,datum,
															times,frequency,EDIT_AREA_DETAIL,first_data,
															last_data,trace_area_3_device->signal_display_minimum,
															trace_area_3_device->signal_display_maximum,axes_left,
															axes_top,axes_width,trace_area_3->axes_height,
															XtWindow(trace_area_3->drawing_area),
															trace_area_3->drawing->pixel_map,
															signal_drawing_information,user_interface);
														current_event=(struct Event *)NULL;
													}
												}
												switch (event->status)
												{
													case ACCEPTED:
													{
														marker_graphics_context=(signal_drawing_information->
															graphics_context).accepted_colour;
														marker_graphics_context_text=
															(signal_drawing_information->graphics_context).
															accepted_colour_text;
													} break;
													case REJECTED:
													{
														marker_graphics_context=(signal_drawing_information->
															graphics_context).rejected_colour;
														marker_graphics_context_text=
															(signal_drawing_information->graphics_context).
															rejected_colour_text;
													} break;
													case UNDECIDED:
													{
														marker_graphics_context=(signal_drawing_information->
															graphics_context).undecided_colour;
														marker_graphics_context_text=
															(signal_drawing_information->graphics_context).
															undecided_colour_text;
													} break;
													default:
													{
														display_message(ERROR_MESSAGE,
															"select_trace_3_drawing_area.  Invalid event status");
														marker_graphics_context=(signal_drawing_information->
															graphics_context).undecided_colour;
														marker_graphics_context_text=
															(signal_drawing_information->graphics_context).
															undecided_colour_text;
													} break;
												}
												event_graphics_context=marker_graphics_context;
												event_graphics_context_text=marker_graphics_context_text;
											}
										}
										else
										{
											if (((EDA_LEVEL==analysis->detection)||
												(EDA_THRESHOLD==analysis->detection))&&
												(Button3==working_button)&&signal)
											{
												/* add another event */
												moving=MOVING_NONE;
												/* calculate the event time */
												event_time=
													SCALE_X(pointer_x,axes_left,first_data,1./x_scale);
												/* determine the position in the event list */
												if ((event_temp=signal->first_event)&&
													(event_temp->time<event_time))
												{
													while ((event_temp->next)&&
														(event_temp->next->time<event_time))
													{
														event_temp=event_temp->next;
													}
													if (event=create_Event(event_time,
														(event_temp->number)+1,UNDECIDED,event_temp,
														event_temp->next))
													{
														if (event_temp->next)
														{
															event_temp->next->previous=event;
														}
														event_temp->next=event;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"select_trace_1_drawing_area.  Could not create event");
													}
												}
												else
												{
													if (event=create_Event(event_time,1,UNDECIDED,
														(struct Event *)NULL,event_temp))
													{
														if (event_temp)
														{
															event_temp->previous=event;
														}
														signal->first_event=event;
													}
												}
												if (current_event)
												{
													draw_event_marker(current_event,event_number,datum,
														times,frequency,EDIT_AREA_DETAIL,first_data,last_data,
														trace_area_3_device->signal_display_minimum,
														trace_area_3_device->signal_display_maximum,axes_left,
														axes_top,axes_width,trace_area_3->axes_height,
														XtWindow(trace_area_3->drawing_area),
														trace_area_3->drawing->pixel_map,
														signal_drawing_information,user_interface);
												}
												event_temp=event->next;
												while (event_temp)
												{
													(event_temp->number)++;
													event_temp=event_temp->next;
												}
												event_number=event->number;
												analysis->event_number=event_number;
												if (current_event)
												{
													draw_event_marker(current_event,event_number,datum,
														times,frequency,EDIT_AREA_DETAIL,first_data,last_data,
														trace_area_3_device->signal_display_minimum,
														trace_area_3_device->signal_display_maximum,axes_left,
														axes_top,axes_width,trace_area_3->axes_height,
														XtWindow(trace_area_3->drawing_area),
														trace_area_3->drawing->pixel_map,
														signal_drawing_information,user_interface);
												}
												/* draw the marker */
												draw_event_marker(event,event_number,datum,times,
													frequency,EDIT_AREA_DETAIL,first_data,last_data,
													trace_area_3_device->signal_display_minimum,
													trace_area_3_device->signal_display_maximum,axes_left,axes_top,
													axes_width,trace_area_3->axes_height,
													XtWindow(trace_area_3->drawing_area),
													trace_area_3->drawing->pixel_map,
													signal_drawing_information,user_interface);
												trace_area_1= &(analysis->trace->area_1);
												start_analysis_interval=buffer->start;
												end_analysis_interval=buffer->end;
												draw_event_marker(event,event_number,datum,times,
													frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
													end_analysis_interval,highlight_device->signal_display_minimum,
													highlight_device->signal_display_maximum,
													trace_area_1->axes_left,trace_area_1->axes_top,
													trace_area_1->axes_width,trace_area_1->axes_height,
													XtWindow(trace_area_1->drawing_area),
													trace_area_1->drawing->pixel_map,
													signal_drawing_information,user_interface);
												if ((analysis->window)&&(analysis->rig)&&
													(signals= &(analysis->window->signals)))
												{
													signals_axes_left=signals->axes_left;
													signals_axes_top=signals->axes_top;
													signals_axes_width=signals->axes_width;
													signals_axes_height=signals->axes_height;
													signals_window=XtWindow(signals->drawing_area);
													signals_pixel_map=signals->drawing->pixel_map;
													drawing_width=signals->drawing->width;
													drawing_height=signals->drawing->height;
													number_of_rows=signals->number_of_rows;
													number_of_columns=signals->number_of_columns;
													device_number=0;
													device=analysis->rig->devices;
													current_region=get_Rig_current_region(analysis->rig);
													for (i=(analysis->highlight)-device;i>0;
															i--)
													{
														if (!current_region||(current_region==
															(*device)->description->region))
														{
															device_number++;
														}
													}
													switch (signals->layout)
													{
														case SEPARATE_LAYOUT:
														{
															xpos=((device_number/number_of_rows)*
																drawing_width)/number_of_columns;
															ypos=((device_number%number_of_rows)*
																drawing_height)/number_of_rows;
														} break;
														case OVERLAP_LAYOUT:
														{
															xpos=((device_number/number_of_rows)*
																drawing_width)/number_of_columns;
															ypos=((device_number%number_of_rows)*
																drawing_height)/(number_of_rows+3);
														} break;
													}
													xpos += signals_axes_left;
													ypos += signals_axes_top;
													draw_event_marker(event,event_number,datum,times,
														frequency,SIGNAL_AREA_DETAIL,
														start_analysis_interval,end_analysis_interval,
														highlight_device->signal_display_minimum,
														highlight_device->signal_display_maximum,xpos,ypos,
														signals_axes_width,signals_axes_height,
														signals_window,signals_pixel_map,
														signal_drawing_information,user_interface);
												}
												trace_update_signal_controls(analysis->trace);
											}
											else
											{
												/* determine if the potential time has been selected */
												initial_marker=SCALE_X(potential_time,first_data,
													axes_left,x_scale);
												if ((pointer_x>=initial_marker-pointer_sensitivity)&&
													(pointer_x<=initial_marker+pointer_sensitivity))
												{
													moving=MOVING_POTENTIAL_TIME_MARKER;
													marker_graphics_context=(signal_drawing_information->
														graphics_context).potential_time_colour;
													marker_graphics_context_text=
														(signal_drawing_information->graphics_context).
														potential_time_colour_text;
												}
												else
												{
													/* determine if datum has been selected */
													initial_marker=
														SCALE_X(datum,first_data,axes_left,x_scale);
													if ((pointer_x>=initial_marker-pointer_sensitivity)&&
														(pointer_x<=initial_marker+pointer_sensitivity))
													{
														moving=MOVING_DATUM_MARKER;
														event=current_event;
														marker_graphics_context=(signal_drawing_information->
															graphics_context).datum_colour;
														if (event)
														{
															switch (event->status)
															{
																case ACCEPTED:
																{
																	event_graphics_context=
																		(signal_drawing_information->
																			graphics_context).accepted_colour;
																	event_graphics_context_text=
																		(signal_drawing_information->
																			graphics_context).accepted_colour_text;
																} break;
																case REJECTED:
																{
																	event_graphics_context=
																		(signal_drawing_information->
																			graphics_context).rejected_colour;
																	event_graphics_context_text=
																		(signal_drawing_information->
																			graphics_context).rejected_colour_text;
																} break;
																case UNDECIDED:
																{
																	event_graphics_context=
																		(signal_drawing_information->
																			graphics_context).undecided_colour;
																	event_graphics_context_text=
																		(signal_drawing_information->
																			graphics_context).undecided_colour_text;
																} break;
																default:
																{
																	display_message(ERROR_MESSAGE,
																		"select_trace_3_drawing_area.  Invalid event status");
																	event_graphics_context=
																		(signal_drawing_information->
																			graphics_context).undecided_colour;
																	event_graphics_context_text=
																		(signal_drawing_information->
																			graphics_context).undecided_colour_text;
																} break;
															}
														}
													}
													else
													{
														/* determine if the y axis has been selected */
														if ((pointer_x>=axes_left-pointer_sensitivity)&&
															(pointer_x<=axes_left+pointer_sensitivity))
														{
															/* determine if the positive or the negative y
																axis has been selected */
															signal_min=
																trace_area_3_device->signal_display_minimum;
															signal_max=
																trace_area_3_device->signal_display_maximum;
															if (signal_max==signal_min)
															{
																signal_max += 1;
																signal_min -= 1;
															}
															initial_marker=pointer_y;
															y_scale=SCALE_FACTOR(signal_max-signal_min,
																trace_area_3->axes_height-1);
															if ((signal_max<(float)0)||(signal_min>(float)0))
															{
																x_axis_y_marker=(axes_bottom+axes_top)/2;
																initial_value=signal_max+(float)(axes_top-
																	x_axis_y_marker)/y_scale;
															}
															else
															{
																x_axis_y_marker=SCALE_Y((float)0,signal_max,
																	axes_top,y_scale);
																initial_value=(float)0;
															}
															if (initial_marker<=(y_limit=x_axis_y_marker-
																pointer_sensitivity))
															{
																moving=SCALING_Y_AXIS_POSITIVE;
																marker_graphics_context=
																	(signal_drawing_information->
																	graphics_context).scaling_signal_colour;
															}
															else
															{
																if (initial_marker>=(y_limit=x_axis_y_marker+
																	pointer_sensitivity))
																{
																	moving=SCALING_Y_AXIS_NEGATIVE;
																	marker_graphics_context=
																		(signal_drawing_information->
																			graphics_context).scaling_signal_colour;
																}
																else
																{
																	moving=MOVING_NONE;
																}
															}
														}
														else
														{
															moving=MOVING_NONE;
														}
													}
												}
											}
										}
									}
									else
									{
										moving=MOVING_NONE;
									}
									if ((MOVING_DATUM_MARKER==moving)||
										(MOVING_EVENT_MARKER==moving)||
										(MOVING_POTENTIAL_TIME_MARKER==moving)||
										(SCALING_Y_AXIS_NEGATIVE==moving)||
										(SCALING_Y_AXIS_POSITIVE==moving))
									{
										/* grab the pointer */
										cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
										/* set the cursor */
										XDefineCursor(display,XtWindow(trace_area_3->drawing_area),cursor);
										XmUpdateDisplay(trace_area_3->drawing_area);
										working_window=XtWindow(trace_area_3->drawing_area);
										pixel_map=trace_area_3->drawing->pixel_map;
										switch (moving)
										{
											case MOVING_DATUM_MARKER:
											case MOVING_EVENT_MARKER:
											{
												XWarpPointer(display,None,None,0,0,0,0,
													initial_marker-pointer_x,0);
												pointer_x=initial_marker;
												marker=initial_marker;
												if (current_event)
												{
													/* clear the delay time */
													sprintf(number_string,"%d",
														(int)((float)(times[current_event->time]-
															times[datum])*1000./frequency));
													length=strlen(number_string);
													XTextExtents(font,number_string,length,&direction,
														&ascent,&descent,&bounds);
													x_string=SCALE_X(current_event->time,first_data,
														axes_left,x_scale)+(bounds.lbearing-
															bounds.rbearing+1)/2;
													if (x_string+bounds.rbearing>=axes_left+axes_width)
													{
														x_string=axes_left+axes_width-bounds.rbearing;
													}
													if (x_string-bounds.lbearing<axes_left)
													{
														x_string=axes_left+bounds.lbearing;
													}
													y_string=axes_top-descent;
													XDrawString(display,pixel_map,
														event_graphics_context_text,x_string,y_string,
														number_string,length);
													XDrawString(display,working_window,
														event_graphics_context,x_string,y_string,
														number_string,length);
												}
											} break;
											case MOVING_POTENTIAL_TIME_MARKER:
											{
												XWarpPointer(display,None,None,0,0,0,0,
													initial_marker-pointer_x,0);
												pointer_x=initial_marker;
												marker=initial_marker;
												/* clear the time */
												sprintf(number_string,"%d",
													(int)((float)(times[potential_time])*1000./
														frequency));
												length=strlen(number_string);
												XTextExtents(font,number_string,length,&direction,
													&ascent,&descent,&bounds);
												x_string=initial_marker+
													(bounds.lbearing-bounds.rbearing+1)/2;
												if (x_string+bounds.rbearing>=axes_left+axes_width)
												{
													x_string=axes_left+axes_width-bounds.rbearing;
												}
												if (x_string-bounds.lbearing<axes_left)
												{
													x_string=axes_left+bounds.lbearing;
												}
												y_string=axes_top-descent;
												XDrawString(display,pixel_map,
													marker_graphics_context_text,x_string,y_string,
													number_string,length);
												XDrawString(display,working_window,
													marker_graphics_context,x_string,y_string,
													number_string,length);
												/* clear the potential */
												if (signal&&(channel=trace_area_3_device->channel))
												{
													switch (buffer->value_type)
													{
														case SHORT_INT_VALUE:
														{
															sprintf(number_string2,"%.3g",
																((float)(buffer->signals.short_int_values)
																[potential_time*(buffer->number_of_signals)+
																(signal->index)]-(channel->offset))*
																(channel->gain));
														} break;
														case FLOAT_VALUE:
														{
															sprintf(number_string2,"%.3g",
																((buffer->signals.float_values)
																[potential_time*(buffer->number_of_signals)+
																(signal->index)]-(channel->offset))*
																(channel->gain));
														} break;
													}
													length2=strlen(number_string2);
													XTextExtents(font,number_string2,length2,&direction,
														&ascent,&descent,&bounds);
													x_string2=axes_left-bounds.rbearing-4;
													y_string2=axes_top-descent;
													XDrawString(display,pixel_map,
														marker_graphics_context_text,x_string2,y_string2,
														number_string2,length2);
													XDrawString(display,working_window,
														marker_graphics_context,x_string2,y_string2,
														number_string2,length2);
												}
											} break;
											case SCALING_Y_AXIS_POSITIVE:
											case SCALING_Y_AXIS_NEGATIVE:
											{
												marker=initial_marker;
												/* draw a horizontal line */
												XDrawLine(display,pixel_map,marker_graphics_context,
													axes_left,initial_marker,axes_right,initial_marker);
												XDrawLine(display,working_window,
													marker_graphics_context,axes_left,initial_marker,
													axes_right,initial_marker);
											} break;
										}
										while (MOVING_NONE!=moving)
										{
											XNextEvent(display,&xevent);
											switch (xevent.type)
											{
												case MotionNotify:
												{
													previous_marker=marker;
													/* reduce the number of motion events displayed */
													while (XCheckMaskEvent(display,ButtonMotionMask,
														&xevent));
													pointer_x=xevent.xmotion.x;
													pointer_y=xevent.xmotion.y;
													switch (moving)
													{
														case MOVING_EVENT_MARKER:
														case MOVING_DATUM_MARKER:
														case MOVING_POTENTIAL_TIME_MARKER:
														{
															if ((xevent.xmotion.window==working_window)&&
																(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
															{
																if (pointer_x<axes_left)
																{
																	marker=axes_left;
																}
																else
																{
																	if (pointer_x>axes_right)
																	{
																		marker=axes_right;
																	}
																	else
																	{
																		marker=pointer_x;
																	}
																}
																if (marker!=previous_marker)
																{
																/* clear the old marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,previous_marker,
																		axes_top,previous_marker,axes_bottom);
																	XDrawLine(display,working_window,
																		marker_graphics_context,previous_marker,
																		axes_top,previous_marker,axes_bottom);
																/* draw the new marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,marker,axes_top,
																		marker,axes_bottom);
																	XDrawLine(display,working_window,
																		marker_graphics_context,marker,axes_top,
																		marker,axes_bottom);
																}
															}
														} break;
														case SCALING_Y_AXIS_NEGATIVE:
														{
															if (xevent.xmotion.window==working_window)
															{
																if (pointer_y<y_limit)
																{
																	marker=y_limit;
																}
																else
																{
																	if (pointer_y>axes_bottom)
																	{
																		marker=axes_bottom;
																	}
																	else
																	{
																		marker=pointer_y;
																	}
																}
																if (marker!=previous_marker)
																{
																/* clear the old marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,axes_left,
																		previous_marker,axes_right,previous_marker);
																	XDrawLine(display,working_window,
																		marker_graphics_context,axes_left,
																		previous_marker,axes_right,previous_marker);
																/* draw the new marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																	XDrawLine(display,working_window,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																}
															}
														} break;
														case SCALING_Y_AXIS_POSITIVE:
														{
															if (xevent.xmotion.window==working_window)
															{
																if (pointer_y>y_limit)
																{
																	marker=y_limit;
																}
																else
																{
																	if (pointer_y<axes_top)
																	{
																		marker=axes_top;
																	}
																	else
																	{
																		marker=pointer_y;
																	}
																}
																if (marker!=previous_marker)
																{
																/* clear the old marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,axes_left,
																		previous_marker,axes_right,previous_marker);
																	XDrawLine(display,working_window,
																		marker_graphics_context,axes_left,
																		previous_marker,axes_right,previous_marker);
																/* draw the new marker */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																	XDrawLine(display,working_window,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																}
															}
														} break;
													}
												} break;
												case ButtonPress:
												{
													if (xevent.xbutton.button==working_button)
													{
														display_message(ERROR_MESSAGE,
															"select_trace_3_drawing_area.  Unexpected button press");
														moving=MOVING_NONE;
													}
												} break;
												case ButtonRelease:
												{
													if (xevent.xbutton.button==working_button)
													{
														if (xevent.xbutton.window==working_window)
														{
															if (marker==initial_marker)
															{
																switch (moving)
																{
																	case SCALING_Y_AXIS_POSITIVE:
																	case SCALING_Y_AXIS_NEGATIVE:
																	{
																		/* clear the horizontal line */
																		XDrawLine(display,pixel_map,
																			marker_graphics_context,axes_left,
																			initial_marker,axes_right,initial_marker);
																		XDrawLine(display,working_window,
																			marker_graphics_context,axes_left,
																			initial_marker,axes_right,initial_marker);
																	} break;
																	case MOVING_DATUM_MARKER:
																	{
																		if (event)
																		{
																			/* draw the new delay time */
																			sprintf(number_string,"%d",
																				(int)((float)(times[event->time]-
																					times[datum])*1000./frequency));
																			length=strlen(number_string);
																			XTextExtents(font,number_string,length,
																				&direction,&ascent,&descent,&bounds);
																			x_string=SCALE_X(event->time,first_data,
																				axes_left,x_scale)+
																				(bounds.lbearing-bounds.rbearing+1)/2;
																			if (x_string+bounds.rbearing>=
																				axes_left+axes_width)
																			{
																				x_string=axes_left+axes_width-
																					bounds.rbearing;
																			}
																			if (x_string-bounds.lbearing<axes_left)
																			{
																				x_string=axes_left+bounds.lbearing;
																			}
																			y_string=axes_top-descent;
																			XDrawString(display,pixel_map,
																				event_graphics_context_text,x_string,
																				y_string,number_string,length);
																			XDrawString(display,working_window,
																				event_graphics_context,x_string,
																				y_string,number_string,length);
																		}
																	} break;
																	case MOVING_POTENTIAL_TIME_MARKER:
																	{
																		/* write the time */
																		sprintf(number_string,"%d",
																			(int)((float)(times[potential_time])*
																				1000./frequency));
																		length=strlen(number_string);
																		XTextExtents(font,number_string,length,
																			&direction,&ascent,&descent,&bounds);
																		x_string=initial_marker+
																			(bounds.lbearing-bounds.rbearing+1)/2;
																		if (x_string+bounds.rbearing>=axes_left+axes_width)
																		{
																			x_string=axes_left+axes_width-bounds.rbearing;
																		}
																		if (x_string-bounds.lbearing<axes_left)
																		{
																			x_string=axes_left+bounds.lbearing;
																		}
																		y_string=axes_top-descent;
																		XDrawString(display,pixel_map,
																			marker_graphics_context_text,
																			x_string,y_string,number_string,length);
																		XDrawString(display,working_window,
																			marker_graphics_context,
																			x_string,y_string,number_string,length);
																		if (signal&&channel)
																		{
																			/* write the potential */
																			switch (buffer->value_type)
																			{
																				case SHORT_INT_VALUE:
																				{
																					sprintf(number_string2,"%.3g",
																						((float)(buffer->signals.
																						short_int_values)[potential_time*
																						(buffer->number_of_signals)+
																						(signal->index)]-(channel->offset))*
																						(channel->gain));
																				} break;
																				case FLOAT_VALUE:
																				{
																					sprintf(number_string2,"%.3g",
																						((buffer->signals.float_values)
																						[potential_time*
																						(buffer->number_of_signals)+
																						(signal->index)]-(channel->offset))*
																						(channel->gain));
																				} break;
																			}
																			length2=strlen(number_string2);
																			XTextExtents(font,number_string2,length2,
																				&direction,&ascent,&descent,&bounds);
																			x_string2=axes_left-bounds.rbearing-4;
																			y_string2=axes_top-descent;
																			XDrawString(display,pixel_map,
																				marker_graphics_context_text,x_string2,
																				y_string2,number_string2,length2);
																			XDrawString(display,working_window,
																				marker_graphics_context,x_string2,
																				y_string2,number_string2,length2);
																		}/* if (signal&&channel)*/
																	} break;
																}/* switch (moving) */
															}/*if (marker==initial_marker)*/
															else
															{
																start_analysis_interval=buffer->start;
																end_analysis_interval=buffer->end;
																/* clear the old markers */
																trace_area_1= &(analysis->trace->area_1);
																switch (moving)
																{
																	case MOVING_EVENT_MARKER:
																	{
																		draw_event_marker(event,event_number,
																			datum,times,frequency,ENLARGE_AREA_DETAIL,
																			start_analysis_interval,
																			end_analysis_interval,
																			highlight_device->signal_display_minimum,
																			highlight_device->signal_display_maximum,
																			trace_area_1->axes_left,
																			trace_area_1->axes_top,
																			trace_area_1->axes_width,
																			trace_area_1->axes_height,
																			XtWindow(trace_area_1->drawing_area),
																			trace_area_1->drawing->pixel_map,
																			signal_drawing_information,
																			user_interface);
																	} break;
																}/* switch (moving)*/
																/* clear the signals drawing area */
																if ((analysis->window)&&(analysis->rig)&&
																	(signals= &(analysis->window->signals)))
																{
																	signals_axes_left=signals->axes_left;
																	signals_axes_top=signals->axes_top;
																	signals_axes_width=signals->axes_width;
																	signals_axes_height=signals->axes_height;
																	signals_window=
																		XtWindow(signals->drawing_area);
																	signals_pixel_map=signals->drawing->pixel_map;
																	drawing_width=signals->drawing->width;
																	drawing_height=signals->drawing->height;
																	number_of_rows=signals->number_of_rows;
																	number_of_columns=signals->number_of_columns;
																	switch (moving)
																	{
																		case MOVING_EVENT_MARKER:
																		case SCALING_Y_AXIS_POSITIVE:
																		case SCALING_Y_AXIS_NEGATIVE:
																		{
																			device_number=0;
																			device=analysis->rig->devices;
																			current_region=get_Rig_current_region(
																				analysis->rig);
#if defined (UNEMAP_USE_NODES)
																			rig_node_order_info=
																				get_Analysis_window_rig_node_order_info(
																					analysis->window);
																			device_number=
																				get_FE_node_order_info_current_node_number(
																					rig_node_order_info);
#else
																			for (i=(analysis->highlight)-device;i>0;
																					i--)
																			{
																				if (!current_region||(current_region==
																					(*device)->description->region))
																				{
																					device_number++;
																				}
																				device++;
																			}
#endif /* defined (UNEMAP_USE_NODES) */
																			switch (signals->layout)
																			{
																				case SEPARATE_LAYOUT:
																				{
																					xpos=((device_number/number_of_rows)*
																						drawing_width)/number_of_columns;
																					ypos=((device_number%number_of_rows)*
																						drawing_height)/number_of_rows;
																				} break;
																				case OVERLAP_LAYOUT:
																				{
																					xpos=((device_number/number_of_rows)*
																						drawing_width)/number_of_columns;
																					ypos=((device_number%number_of_rows)*
																						drawing_height)/(number_of_rows+3);
																				} break;
																			}
																			if (MOVING_EVENT_MARKER==moving)
																			{
																				xpos += signals_axes_left;
																				ypos += signals_axes_top;
																				draw_event_marker(event,event_number,
																					datum,times,frequency,
																					SIGNAL_AREA_DETAIL,
																					start_analysis_interval,
																					end_analysis_interval,
																					highlight_device->
																					signal_display_minimum,
																					highlight_device->
																					signal_display_maximum,xpos,
																					ypos,signals_axes_width,
																					signals_axes_height,signals_window,
																					signals_pixel_map,
																					signal_drawing_information,
																					user_interface);
																			}
																		} break;
																	}/* switch (moving) */
																}/* if ((analysis->window)&&(analysis->rig)&& */
																else
																{
																	signals=(struct Signals_area *)NULL;
																}
																/* change the values */
																switch (moving)
																{
																	case MOVING_EVENT_MARKER:
																	{
																		event->time=SCALE_X(marker,axes_left,
																			first_data,1/x_scale);
																		initial_marker=SCALE_X(event->time,
																			first_data,axes_left,x_scale);
																	} break;
																	case MOVING_DATUM_MARKER:
																	{
																		datum=SCALE_X(marker,axes_left,first_data,
																			1/x_scale);
																	} break;
																	case MOVING_POTENTIAL_TIME_MARKER:
																	{
																		potential_time=SCALE_X(marker,axes_left,
																			first_data,1/x_scale);
																	} break;
																	case SCALING_Y_AXIS_POSITIVE:
																	{
																		new_max=initial_value+
																			(float)(x_axis_y_marker-axes_top)*
																			(float)(marker-x_axis_y_marker)/(y_scale*
																			(float)(initial_marker-x_axis_y_marker));
#if defined (UNEMAP_USE_NODES)
																		signal_drawing_package=
																			analysis->signal_drawing_package;
																		signal_maximum_field=
																			get_Signal_drawing_package_signal_maximum_field(
																			signal_drawing_package);
																		/* set the new signal_maximum*/
																		component.number=0;
																		component.field=signal_maximum_field;
																		/*??JW should be copying out of and into
																			node with MANAGER_MODIFY */
																		set_FE_nodal_FE_value_value(
																			analysis->highlight_rig_node,
																			&component,0,FE_NODAL_VALUE,/*time*/0,
																			new_max);
#endif /*	defined (UNEMAP_USE_NODES) */
																		highlight_device->signal_display_maximum=
																			new_max;
																		if (highlight_device!=trace_area_3_device)
																		{
																			trace_area_3_device->
																				signal_display_maximum=
																				highlight_device->
																				signal_display_maximum;
																		}
																	} break;
																	case SCALING_Y_AXIS_NEGATIVE:
																	{
																		new_min=initial_value+
																			(float)(x_axis_y_marker-axes_bottom)*
																			(float)(marker-x_axis_y_marker)/(y_scale*
																			(float)(initial_marker-x_axis_y_marker));
#if defined (UNEMAP_USE_NODES)
																		signal_drawing_package=
																			analysis->signal_drawing_package;
																		signal_minimum_field=
																			get_Signal_drawing_package_signal_minimum_field(
																			signal_drawing_package);
																		/* set the new signal_minimum*/
																		component.number=0;
																		component.field=signal_minimum_field;
																		/*??JW should be copying out of and into
																			node with MANAGER_MODIFY */
																		set_FE_nodal_FE_value_value(
																			analysis->highlight_rig_node,&component,0,
																			FE_NODAL_VALUE,/*time*/0,new_min);
#endif /*	defined (UNEMAP_USE_NODES) */
																		highlight_device->signal_display_minimum=
																			new_min;
																		if (highlight_device!=trace_area_3_device)
																		{
																			trace_area_3_device->
																				signal_display_minimum=
																				highlight_device->
																				signal_display_minimum;
																		}
																	} break;
																}/* switch (moving) */
																/* draw the new markers */
																/* update drawing area 3 */
																switch (moving)
																{
																	case MOVING_POTENTIAL_TIME_MARKER:
																	case MOVING_DATUM_MARKER:
																	{
																		XDrawLine(display,pixel_map,
																			marker_graphics_context,
																			marker,axes_top,marker,axes_bottom);
																		XDrawLine(display,working_window,
																			marker_graphics_context,
																			marker,axes_top,marker,axes_bottom);
																	} break;
																	case MOVING_EVENT_MARKER:
																	{
																		if (marker!=initial_marker)
																		{
																			XDrawLine(display,pixel_map,
																				marker_graphics_context,
																				marker,axes_top,marker,axes_bottom);
																			XDrawLine(display,working_window,
																				marker_graphics_context,
																				marker,axes_top,marker,axes_bottom);
																			XDrawLine(display,pixel_map,
																				marker_graphics_context,initial_marker,
																				axes_top,initial_marker,axes_bottom);
																			XDrawLine(display,working_window,
																				marker_graphics_context,initial_marker,
																				axes_top,initial_marker,axes_bottom);
																		}
																	} break;
																	case SCALING_Y_AXIS_NEGATIVE:
																	case SCALING_Y_AXIS_POSITIVE:
																	{
																		redraw_trace_3_drawing_area((Widget)NULL,
																			(XtPointer)(analysis->trace),
																			(XtPointer)NULL);
																	} break;
																}/* switch (moving)*/
																/* update time objects */
																switch (moving)
																{
																	case MOVING_POTENTIAL_TIME_MARKER:
																	{
																		analysis->trace_update_flags |=
																			TRACE_3_NO_POTENTIAL_ERASE;
																		/* this conversion to time_keeper time
																			should be much more robust.  The frequency
																			is not guaranteed to divide the potential
																			time and this takes no account of time
																			transformations */
																		Time_keeper_request_new_time(
																			Time_object_get_time_keeper(
																			analysis->potential_time_object),
																			((double)times[potential_time]/
																			frequency));
																	} break;
																	case MOVING_DATUM_MARKER:
																	{
																		analysis->trace_update_flags |=
																			TRACE_3_NO_DATUM_ERASE;
																		Time_object_set_current_time_privileged(
																			analysis->datum_time_object,
																			(double)datum);
																		Time_object_notify_clients_privileged(
																			analysis->datum_time_object);
																	} break;
																}/* switch (moving) */
																/* update the drawing area 1 of the trace
																	window */
																switch (moving)
																{
																	case MOVING_EVENT_MARKER:
																	{
																		draw_event_marker(event,event_number,datum,
																			times,frequency,ENLARGE_AREA_DETAIL,
																			start_analysis_interval,
																			end_analysis_interval,
																			highlight_device->signal_display_minimum,
																			highlight_device->signal_display_maximum,
																			trace_area_1->axes_left,
																			trace_area_1->axes_top,
																			trace_area_1->axes_width,
																			trace_area_1->axes_height,
																			XtWindow(trace_area_1->drawing_area),
																			trace_area_1->drawing->pixel_map,
																			signal_drawing_information,
																			user_interface);
																	} break;
																	case SCALING_Y_AXIS_NEGATIVE:
																	case SCALING_Y_AXIS_POSITIVE:
																	{
																		redraw_trace_1_drawing_area((Widget)NULL,
																			(XtPointer)(analysis->trace),
																			(XtPointer)NULL);
																	} break;
																}/* switch (moving) */
																if (signals)
																{
																	switch (moving)
																	{
																		case MOVING_EVENT_MARKER:
																		{
																			draw_event_marker(event,event_number,
																				datum,times,frequency,
																				SIGNAL_AREA_DETAIL,
																				start_analysis_interval,
																				end_analysis_interval,
																				highlight_device->signal_display_minimum,
																				highlight_device->signal_display_maximum,xpos,
																				ypos,signals_axes_width,
																				signals_axes_height,signals_window,
																				signals_pixel_map,
																				signal_drawing_information,
																				user_interface);
																		} break;
																		case SCALING_Y_AXIS_NEGATIVE:
																		case SCALING_Y_AXIS_POSITIVE:
																		{
																			XFillRectangle(display,signals_pixel_map,
																				(signal_drawing_information->
																					graphics_context).
																				background_drawing_colour,xpos,ypos,
																				signals->signal_width,
																				signals->signal_height);
#if defined (UNEMAP_USE_NODES)
																			draw_signal(
																				analysis->highlight_rig_node,
																				analysis->signal_drawing_package,
																				(struct Device *)NULL,
																				SIGNAL_AREA_DETAIL,1,0,
																				&start_analysis_interval,
																				&end_analysis_interval,xpos,ypos,
																				signals->signal_width,
																				signals->signal_height,
																				signals_pixel_map,&signals_axes_left,
																				&signals_axes_top,&signals_axes_width,
																				&signals_axes_height,
																				signal_drawing_information,
																				user_interface);
#else
																			draw_signal(
																				(struct FE_node *)NULL,
																				(struct Signal_drawing_package *)NULL,
																				highlight_device,
																				SIGNAL_AREA_DETAIL,1,0,
																				&start_analysis_interval,
																				&end_analysis_interval,xpos,ypos,
																				signals->signal_width,
																				signals->signal_height,
																				signals_pixel_map,&signals_axes_left,
																				&signals_axes_top,&signals_axes_width,
																				&signals_axes_height,
																				signal_drawing_information,
																				user_interface);
#endif /* defined (UNEMAP_USE_NODES)*/
																			draw_device_markers(highlight_device,
																				start_analysis_interval,
																				end_analysis_interval,datum,1,
																				potential_time,1,SIGNAL_AREA_DETAIL,0,
																				signals_axes_left,signals_axes_top,
																				signals_axes_width,signals_axes_height,
																				(Window)NULL,signals_pixel_map,
																				signal_drawing_information,
																				user_interface);
																			XCopyArea(display,signals_pixel_map,
																				signals_window,
																				(signal_drawing_information->
																					graphics_context).copy,xpos,ypos,
																				signals->signal_width,
																				signals->signal_height,xpos,ypos);
																		} break;
																	}
																	/* clear the interval drawing area */
																	switch (moving)
																	{
																		case SCALING_Y_AXIS_NEGATIVE:
																		case SCALING_Y_AXIS_POSITIVE:
																		{
																			update_interval_drawing_area(
																				analysis->window);
																		} break;
																	}
																}/* if (signals) */
															}/* if (marker==initial_marker) */
															switch (moving)
															{
																case MOVING_EVENT_MARKER:
																{
																	if (event)
																	{
																		/* draw the new delay time */
																		sprintf(number_string,"%d",
																			(int)((float)(times[event->time]-
																				times[datum])*1000./frequency));
																		length=strlen(number_string);
																		XTextExtents(font,number_string,length,
																			&direction,&ascent,&descent,&bounds);
																		x_string=SCALE_X(event->time,first_data,
																			axes_left,x_scale)+
																			(bounds.lbearing-bounds.rbearing+1)/2;
																		if (x_string+bounds.rbearing>=
																			axes_left+axes_width)
																		{
																			x_string=axes_left+axes_width-
																				bounds.rbearing;
																		}
																		if (x_string-bounds.lbearing<axes_left)
																		{
																			x_string=axes_left+bounds.lbearing;
																		}
																		y_string=axes_top-descent;
																		XDrawString(display,pixel_map,
																			event_graphics_context_text,x_string,
																			y_string,number_string,length);
																		XDrawString(display,working_window,
																			event_graphics_context,x_string,y_string,
																			number_string,length);
																	}
																} break;
															}
														}
														else
														{
															switch (moving)
															{
																case MOVING_DATUM_MARKER:
																case MOVING_EVENT_MARKER:
																case MOVING_POTENTIAL_TIME_MARKER:
																{
																	if (marker!=initial_marker)
																	{
																		/* clear the new marker */
																		XDrawLine(display,pixel_map,
																			marker_graphics_context,marker,axes_top,
																			marker,axes_bottom);
																		XDrawLine(display,working_window,
																			marker_graphics_context,marker,axes_top,
																			marker,axes_bottom);
																		/* draw the old marker */
																		XDrawLine(display,pixel_map,
																			marker_graphics_context,initial_marker,
																			axes_top,initial_marker,axes_bottom);
																		XDrawLine(display,working_window,
																			marker_graphics_context,initial_marker,
																			axes_top,initial_marker,axes_bottom);
																	}
																/* redraw the time */
																	if ((MOVING_EVENT_MARKER==moving)&&
																		(event!=current_event))
																	{
																		/* draw the new delay time */
																		sprintf(number_string,"%d",
																			(int)((float)(times[event->time]-
																				times[datum])*1000./frequency));
																		length=strlen(number_string);
																		XTextExtents(font,number_string,length,
																			&direction,&ascent,&descent,&bounds);
																		x_string=SCALE_X(event->time,first_data,
																			axes_left,x_scale)+
																			(bounds.lbearing-bounds.rbearing+1)/2;
																		if (x_string+bounds.rbearing>=
																			axes_left+axes_width)
																		{
																			x_string=axes_left+axes_width-
																				bounds.rbearing;
																		}
																		if (x_string-bounds.lbearing<axes_left)
																		{
																			x_string=axes_left+bounds.lbearing;
																		}
																		y_string=axes_top-descent;
																	}
																	XDrawString(display,pixel_map,
																		marker_graphics_context_text,x_string,
																		y_string,number_string,length);
																	XDrawString(display,working_window,
																		marker_graphics_context,x_string,y_string,
																		number_string,length);
																	if (MOVING_POTENTIAL_TIME_MARKER==moving)
																	{
																		/* redraw the value */
																		XDrawString(display,pixel_map,
																			marker_graphics_context_text,x_string2,
																			y_string2,number_string2,length2);
																		XDrawString(display,working_window,
																			marker_graphics_context,x_string2,
																			y_string2,number_string2,length2);
																	}
																} break;
																case SCALING_Y_AXIS_NEGATIVE:
																case SCALING_Y_AXIS_POSITIVE:
																{
																/* clear the horizontal line */
																	XDrawLine(display,pixel_map,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																	XDrawLine(display,working_window,
																		marker_graphics_context,axes_left,marker,
																		axes_right,marker);
																} break;
															}
														}
														moving=MOVING_NONE;
													}
												} break;
												default:
												{
													XtDispatchEvent(&xevent);
												}
											}
										}
										XUndefineCursor(display,
											XtWindow(trace_area_3->drawing_area));
										XFreeCursor(display,cursor);
									}
								}
							}
						} break;
					}
				}
				/* get signal min,max put in range widget */
				update_signal_range_widget_from_highlight_signal(
					&(analysis->window->interval),
#if defined (UNEMAP_USE_NODES)
					analysis->highlight_rig_node,analysis->signal_drawing_package
#else
					*(analysis->highlight)
#endif /* defined (UNEMAP_USE_NODES)*/
					);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_trace_3_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_trace_3_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_trace_3_drawing_area.  Missing analysis work area");
	}
	LEAVE;
} /* select_trace_3_drawing_area */

static struct Rig *create_processed_rig(struct Rig *raw_rig)
/*******************************************************************************
LAST MODIFIED : 4 September 2002

DESCRIPTION :
Duplicates the raw rig, except that
1 The created rig has FLOAT_VALUEs
2 The offset and gain operation has been performed on the signal values (offset
	0 and gain 1 for all channels)
==============================================================================*/
{
	float channel_gain,channel_offset,*raw_float_value,*value;
	int buffer_offset,device_number,i,j,number_of_electrodes,*raw_time,
		region_number,*time;
	short int *raw_short_int_value;
	Linear_transformation *linear_transformation,*raw_linear_transformation;
	struct Auxiliary_properties *auxiliary,*raw_auxiliary;
	struct Channel *channel;
	struct Device **device,**raw_device;
	struct Device_description *description;
	struct Device_list_item *device_item,**device_item_address,*device_list,
		*raw_device_item;
	struct Page_list_item **page_item_address,*raw_page_item;
	struct Region *raw_region,*region;
	struct Region_list_item *raw_region_item,*region_item,**region_item_address;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *raw_signal_buffer,*signal_buffer;
#if defined (UNEMAP_USE_3D)
	struct FE_region *all_devices_rig_node_group,*rig_node_group,
		*unrejected_node_group;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(create_processed_rig);
#if defined (UNEMAP_USE_3D)
	all_devices_rig_node_group=(struct FE_region *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	unrejected_node_group=(struct FE_region *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	if (raw_rig&&(0<raw_rig->number_of_devices)&&(raw_rig->devices)&&
		(*(raw_rig->devices))&&
		(raw_signal_buffer=get_Device_signal_buffer(*(raw_rig->devices))))
	{
		if (rig=create_Rig(raw_rig->name,raw_rig->monitoring,raw_rig->experiment,0,
			(struct Device **)NULL,(struct Page_list_item *)NULL,0,
			(struct Region_list_item *)NULL,(struct Region *)NULL
#if defined (UNEMAP_USE_3D)
			,get_Rig_unemap_package(raw_rig)
#endif /* defined (UNEMAP_USE_3D) */
				))
		{
#if defined (UNEMAP_USE_3D)
			/* set the all_devices_rig_node_group pointer */
			if (all_devices_rig_node_group=
				get_Rig_all_devices_rig_node_group(raw_rig))
			{
				set_Rig_all_devices_rig_node_group(rig,all_devices_rig_node_group);
			}
#endif /* defined (UNEMAP_USE_3D) */
			if (raw_rig->signal_file_name)
			{
				/* assign the signal file name */
				if (ALLOCATE(rig->signal_file_name,char,
					strlen(raw_rig->signal_file_name)+1))
				{
					strcpy(rig->signal_file_name,raw_rig->signal_file_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"create_processed_rig.  Could not allocate memory for signal file name");
				}
			}
			if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,
				raw_signal_buffer->number_of_signals,
				raw_signal_buffer->number_of_samples,raw_signal_buffer->frequency))
			{
				signal_buffer->start=raw_signal_buffer->start;
				signal_buffer->end=raw_signal_buffer->end;
				/* duplicate the times */
				time=signal_buffer->times;
				raw_time=raw_signal_buffer->times;
				for (i=signal_buffer->number_of_samples;i>0;i--)
				{
					*time= *raw_time;
					time++;
					raw_time++;
				}
				/* copy the values */
				value=(signal_buffer->signals).float_values;
				switch (raw_signal_buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						raw_short_int_value=(raw_signal_buffer->signals).short_int_values;
						for (i=(signal_buffer->number_of_samples)*
							(signal_buffer->number_of_signals);i>0;i--)
						{
							*value=(float)(*raw_short_int_value);
							value++;
							raw_short_int_value++;
						}
					} break;
					case FLOAT_VALUE:
					{
						raw_float_value=(raw_signal_buffer->signals).float_values;
						for (i=(signal_buffer->number_of_samples)*
							(signal_buffer->number_of_signals);i>0;i--)
						{
							*value= *raw_float_value;
							value++;
							raw_float_value++;
						}
					} break;
				}
				/* duplicate the regions */
				rig->region_list=(struct Region_list_item *)NULL;
				rig->current_region=(struct Region *)NULL;
				rig->number_of_regions=0;
				region_item_address= &(rig->region_list);
				raw_region_item=raw_rig->region_list;
				while (rig&&raw_region_item)
				{
					if (raw_region=raw_region_item->region)
					{
						if ((*region_item_address=create_Region_list_item(create_Region(
							raw_region->name,raw_region->type,raw_region->number,0
#if defined (UNEMAP_USE_3D)
							,get_Rig_unemap_package(raw_rig)
#endif /* defined (UNEMAP_USE_3D) */
							),(struct Region_list_item *)NULL))&&
							((*region_item_address)->region))
						{
#if defined (UNEMAP_USE_3D)
							if (rig_node_group=get_Region_rig_node_group(raw_region))
							{
								set_Region_rig_node_group((*region_item_address)->region,
									rig_node_group);
							}
							if (unrejected_node_group=
								get_Region_unrejected_node_group(raw_region))
							{
								set_Region_unrejected_node_group((*region_item_address)->region,
									unrejected_node_group);
							}
#endif /* defined (UNEMAP_USE_3D) */
							switch (raw_region->type)
							{
								case SOCK:
								{
									((*region_item_address)->region->properties).sock.focus=
										(raw_region->properties).sock.focus;
									if (raw_linear_transformation=
										(raw_region->properties).sock.linear_transformation)
									{
										if (ALLOCATE(linear_transformation,Linear_transformation,1))
										{
											((*region_item_address)->region->properties).sock.
												linear_transformation=linear_transformation;
											linear_transformation->translate_x=
												raw_linear_transformation->translate_x;
											linear_transformation->translate_y=
												raw_linear_transformation->translate_y;
											linear_transformation->translate_z=
												raw_linear_transformation->translate_z;
											linear_transformation->txx=raw_linear_transformation->txx;
											linear_transformation->txy=raw_linear_transformation->txy;
											linear_transformation->txz=raw_linear_transformation->txz;
											linear_transformation->tyx=raw_linear_transformation->tyx;
											linear_transformation->tyy=raw_linear_transformation->tyy;
											linear_transformation->tyz=raw_linear_transformation->tyz;
											linear_transformation->tzx=raw_linear_transformation->tzx;
											linear_transformation->tzy=raw_linear_transformation->tzy;
											linear_transformation->tzz=raw_linear_transformation->tzz;
										}
										else
										{
											display_message(ERROR_MESSAGE,
							"create_processed_rig.  Could not create linear transformation");
											destroy_Rig(&rig);
										}
									}
								} break;
							}
							if (rig)
							{
								if (raw_region==raw_rig->current_region)
								{
									rig->current_region=(*region_item_address)->region;
								}
								region_item_address= &((*region_item_address)->next);
								(rig->number_of_regions)++;
								raw_region_item=raw_region_item->next;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_processed_rig.  Could not create processed region");
							destroy_Rig(&rig);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_processed_rig.  Missing raw_region");
						destroy_Rig(&rig);
					}
				}
				if (rig)
				{
					/* duplicate the devices */
					i=raw_rig->number_of_devices;
					if (ALLOCATE(device,struct Device *,i))
					{
						rig->devices=device;
						rig->number_of_devices=0;
						raw_device=raw_rig->devices;
						while ((i>0)&&rig)
						{
							if ((*raw_device)&&((*raw_device)->description))
							{
								if ((*raw_device)->channel)
								{
									if (!(channel=create_Channel((*raw_device)->channel->number,
										(float)0,(float)1)))
									{
										destroy_Rig(&rig);
									}
								}
								else
								{
									channel=(struct Channel *)NULL;
								}
								if (rig)
								{
									if ((*raw_device)->signal)
									{
										if (!(signal=create_Signal((*raw_device)->signal->index,
											signal_buffer,(*raw_device)->signal->status,
											(*raw_device)->signal->number)))
										{
											destroy_Rig(&rig);
										}
									}
									else
									{
										signal=(struct Signal *)NULL;
									}
									if (rig)
									{
										if ((*raw_device)->description->region)
										{
											region_number=(*raw_device)->description->region->number;
											region_item=rig->region_list;
											while (region_item&&(region=region_item->region)&&
												(region_number!=region->number))
											{
												region_item=region_item->next;
											}
											if (!region_item)
											{
												display_message(ERROR_MESSAGE,
													"create_processed_rig.  Missing region");
												destroy_Rig(&rig);
											}
										}
										else
										{
											region=(struct Region *)NULL;
										}
										if (rig)
										{
											if ((description=create_Device_description(
												(*raw_device)->description->name,
												(*raw_device)->description->type,region))&&
												(*device=create_Device((*raw_device)->number,
												description,channel,signal)))
											{
												(rig->number_of_devices)++;
												(region->number_of_devices)++;
												switch ((*raw_device)->description->type)
												{
													case AUXILIARY:
													{
#if defined (DEVICE_EXPRESSIONS)
														auxiliary= &((description->properties).auxiliary);
														raw_auxiliary= &(((*raw_device)->description->
															properties).auxiliary);
														auxiliary->type=raw_auxiliary->type;
														switch (raw_auxiliary->type)
														{
															case AUXILIARY_DEVICE_CHANNEL:
															{
																/* nothing to do */
															} break;
															case AUXILIARY_DEVICE_EXPRESSION:
															{
																if (((raw_auxiliary->combination).expression.
																	device_expression_string)&&ALLOCATE(
																	(auxiliary->combination).expression.
																	device_expression_string,char,strlen(
																	(raw_auxiliary->combination).expression.
																	device_expression_string)+1))
																{
																	strcpy((auxiliary->combination).
																		expression.device_expression_string,
																		(raw_auxiliary->combination).expression.
																		device_expression_string);
																	(auxiliary->combination).expression.
																		device_expression=
																		parse_device_expression_string(
																		(auxiliary->combination).expression.
																		device_expression_string,
																		(struct Device_list_item *)NULL,
																		(rig->number_of_devices)-1,rig->devices);
																	if (!((raw_auxiliary->combination).expression.
																		device_expression))
																	{
																		display_message(ERROR_MESSAGE,
																			"create_processed_rig.  Could not parse "
																			"auxiliary device expression string");
																		destroy_Rig(&rig);
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
																		"create_processed_rig.  Could not allocate "
																		"memory for an auxiliary device expression "
																		"string");
																	destroy_Rig(&rig);
																}
															} break;
															case AUXILIARY_DEVICE_SUM:
															{
																if (0<(number_of_electrodes=(raw_auxiliary->
																	combination).sum.number_of_electrodes))
																{
																	ALLOCATE(
																		(auxiliary->combination).sum.electrodes,
																		struct Device *,number_of_electrodes);
																	ALLOCATE((auxiliary->combination).sum.
																		electrode_coefficients,float,
																		number_of_electrodes);
																	if (((auxiliary->combination).sum.
																		electrodes)&&((auxiliary->combination).sum.
																		electrode_coefficients))
																	{
																		(auxiliary->combination).sum.
																			number_of_electrodes=number_of_electrodes;
																		j=0;
																		while ((j<number_of_electrodes)&&rig)
																		{
																			if (((auxiliary->combination).sum.
																				electrodes)[j]=create_Device(
																				(((raw_auxiliary->combination).sum.
																				electrodes)[j])->number,
																				(struct Device_description *)NULL,
																				(struct Channel *)NULL,
																				(struct Signal *)NULL))
																			{
																				((auxiliary->combination).sum.
																					electrode_coefficients)[j]=
																					((raw_auxiliary->combination).sum.
																					electrode_coefficients)[j];
																				j++;
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
																					"create_processed_rig.  Could not "
																					"create electrode for an auxiliary "
																					"device that is a linear "
																					"combination");
																				while (j>0)
																				{
																					j--;
																					destroy_Device(((auxiliary->
																						combination).sum.electrodes)+j);
																				}
																				destroy_Rig(&rig);
																			}
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
																			"create_processed_rig.  Could not "
																			"allocate memory for an auxiliary device "
																			"that is a linear combination");
																		DEALLOCATE((auxiliary->combination).sum.
																			electrodes);
																		DEALLOCATE((auxiliary->combination).sum.
																			electrode_coefficients);
																		destroy_Rig(&rig);
																	}
																}
															} break;
														}
#else /* defined (DEVICE_EXPRESSIONS) */
														auxiliary= &((description->properties).auxiliary);
														raw_auxiliary= &(((*raw_device)->description->
															properties).auxiliary);
														if (0<(number_of_electrodes=raw_auxiliary->
															number_of_electrodes))
														{
															ALLOCATE(auxiliary->electrodes,struct Device *,
																number_of_electrodes);
															ALLOCATE(auxiliary->electrode_coefficients,float,
																number_of_electrodes);
															if ((auxiliary->electrodes)&&
																(auxiliary->electrode_coefficients))
															{
																auxiliary->number_of_electrodes=
																	number_of_electrodes;
																j=0;
																while ((j<number_of_electrodes)&&rig)
																{
																	if ((auxiliary->electrodes)[j]=create_Device(
																		((raw_auxiliary->electrodes)[j])->number,
																		(struct Device_description *)NULL,
																		(struct Channel *)NULL,
																		(struct Signal *)NULL))
																	{
																		(auxiliary->electrode_coefficients)[j]=
																			(raw_auxiliary->
																			electrode_coefficients)[j];
																		j++;
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
																			"create_processed_rig.  Could not create "
																			"electrode for an auxiliary device that "
																			"is a linear combination");
																		while (j>0)
																		{
																			j--;
																			destroy_Device((auxiliary->electrodes)+j);
																		}
																		destroy_Rig(&rig);
																	}
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"create_processed_rig.  Could not allocate "
																	"memory for an auxiliary device that is a "
																	"linear combination");
																DEALLOCATE(auxiliary->electrodes);
																DEALLOCATE(auxiliary->electrode_coefficients);
																destroy_Rig(&rig);
															}
														}
#endif /* defined (DEVICE_EXPRESSIONS) */
													} break;
													case ELECTRODE:
													{
														(description->properties).electrode.position.x=
															((*raw_device)->description->properties).
															electrode.position.x;
														(description->properties).electrode.position.y=
															((*raw_device)->description->properties).
															electrode.position.y;
														(description->properties).electrode.position.z=
															((*raw_device)->description->properties).
															electrode.position.z;
													} break;
												}
												if (rig)
												{
													if (channel&&signal)
													{
														/* perform offset and gain */
														channel_offset=(*raw_device)->channel->offset;
														channel_gain=(*raw_device)->channel->gain;
														if ((0!=channel_offset)||(1!=channel_gain))
														{
															buffer_offset=signal_buffer->number_of_signals;
															value=(signal_buffer->signals.float_values)+
																(signal->index);
															for (j=signal_buffer->number_of_samples;j>0;j--)
															{
																*value=channel_gain*((*value)-channel_offset);
																value += buffer_offset;
															}
														}
													}
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"create_processed_rig.  Could not create device");
												if (description)
												{
													destroy_Device_description(&description);
												}
												if (channel)
												{
													destroy_Channel(&channel);
												}
												if (signal)
												{
													destroy_Signal(&signal);
												}
												destroy_Rig(&rig);
											}
										}
										else
										{
											if (channel)
											{
												destroy_Channel(&channel);
											}
											if (signal)
											{
												destroy_Signal(&signal);
											}
										}
									}
									else
									{
										if (channel)
										{
											destroy_Channel(&channel);
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_processed_rig.  Invalid raw_device");
								destroy_Rig(&rig);
							}
							i--;
							device++;
							raw_device++;
						}
						if (rig)
						{
							/* finish assigning auxiliary devices that are linear
								combinations of electrodes */
							device=rig->devices;
							for (i=0;i<rig->number_of_devices;i++)
							{
#if defined (DEVICE_EXPRESSIONS)
								if (device[i]&&(device[i]->description)&&(AUXILIARY==
									device[i]->description->type)&&(AUXILIARY_DEVICE_SUM==
									(auxiliary=&((device[i]->description->properties).
									auxiliary))->type))
								{
									for (j=0;j<(auxiliary->combination).sum.number_of_electrodes;
										j++)
									{
										device_number=(((auxiliary->combination).sum.
											electrodes)[j])->number;
										destroy_Device(((auxiliary->combination).sum.
											electrodes)+j);
										((auxiliary->combination).sum.electrodes)[j]=
											device[device_number];
									}
								}
#else /* defined (DEVICE_EXPRESSIONS) */
								if (device[i]&&(device[i]->description)&&(AUXILIARY==
									device[i]->description->type)&&(0<(auxiliary=
									&((device[i]->description->properties).auxiliary))->
									number_of_electrodes))
								{
									for (j=0;j<auxiliary->number_of_electrodes;j++)
									{
										device_number=((auxiliary->electrodes)[j])->number;
										destroy_Device((auxiliary->electrodes)+j);
										(auxiliary->electrodes)[j]=device[device_number];
									}
								}
#endif /* defined (DEVICE_EXPRESSIONS) */
							}
							/* duplicate the pages */
							rig->page_list=(struct Page_list_item *)NULL;
							page_item_address= &(rig->page_list);
							raw_page_item=raw_rig->page_list;
							while (rig&&raw_page_item)
							{
								if (raw_page_item->page)
								{
									/* duplicate the device list */
									device_list=(struct Device_list_item *)NULL;
									device_item=(struct Device_list_item *)NULL;
									device_item_address= &device_list;
									raw_device_item=raw_page_item->page->device_list;
									while (rig&&raw_device_item)
									{
										if (raw_device_item->device)
										{
											i=raw_rig->number_of_devices;
											device=rig->devices;
											raw_device=raw_rig->devices;
											while ((i>0)&&(raw_device_item->device!= *raw_device))
											{
												device++;
												raw_device++;
												i--;
											}
											if (i>0)
											{
												if (*device_item_address=create_Device_list_item(
													*device,device_item,(struct Device_list_item *)NULL))
												{
													device_item= *device_item_address;
													device_item_address= &(device_item->next);
												}
												else
												{
													display_message(ERROR_MESSAGE,
												"create_processed_rig.  Could not create device item");
													destroy_Device_list(&device_list,0);
													destroy_Rig(&rig);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"create_processed_rig.  Unknown page device");
												destroy_Device_list(&device_list,0);
												destroy_Rig(&rig);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"create_processed_rig.  Invalid page");
											destroy_Device_list(&device_list,0);
											destroy_Rig(&rig);
										}
										raw_device_item=raw_device_item->next;
									}
									if (rig)
									{
										if (ALLOCATE(*page_item_address,struct Page_list_item,1)&&
											((*page_item_address)->page=create_Page(
											raw_page_item->page->name,device_list)))
										{
											(*page_item_address)->next=(struct Page_list_item *)NULL;
											page_item_address= &((*page_item_address)->next);
										}
										else
										{
											if (*page_item_address)
											{
												DEALLOCATE(*page_item_address);
											}
											display_message(ERROR_MESSAGE,
												"create_processed_rig.  Invalid page");
											destroy_Device_list(&device_list,0);
											destroy_Rig(&rig);
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_processed_rig.  Invalid page item");
									destroy_Rig(&rig);
								}
								raw_page_item=raw_page_item->next;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_processed_rig.  Could not create devices array");
						destroy_Rig(&rig);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_processed_rig.  Could not create processed signal buffer");
				destroy_Rig(&rig);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_processed_rig.  Could not create processed rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_processed_rig.  Invalid raw_rig");
		rig=(struct Rig *)NULL;
	}
	LEAVE;

	return (rig);
} /* create_processed_rig */

static void analysis_reset_to_raw_signals(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Resets the signals being analysed back to the raw data.
==============================================================================*/
{
	int i;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Rig *rig;

	ENTER(analysis_reset_to_raw_signals);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if (rig=analysis->raw_rig)
		{
			/* reset the highlighted device */
			if (highlight= *(analysis->highlight))
			{
				device=analysis->rig->devices;
				i=analysis->rig->number_of_devices;
				while ((i>0)&&(*device!=highlight))
				{
					i--;
					device++;
				}
				if (i>0)
				{
					analysis->highlight=(rig->devices)+
						(device-(analysis->rig->devices));
					(*(analysis->highlight))->highlight=1;
				}
				else
				{
					*(analysis->highlight)=(struct Device *)NULL;
				}
			}
			destroy_Rig(&(analysis->rig));
			analysis->rig=rig;
			analysis->raw_rig=(struct Rig *)NULL;
			if (analysis->trace)
			{
				/* recalculate the power spectrum minimum and maximum */
				(analysis->trace->power_spectra).maximum_frequency= -1;
				(analysis->trace->power_spectra).minimum_frequency= -1;
			}
			/* ghost the reset button */
			XtSetSensitive(analysis->window->interval.reset_button,False);
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
			/* update the mapping window */
			if ((NO_MAP_FIELD==analysis->map_type)&&
				(mapping=analysis->mapping_window)&&(map=mapping->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(mapping,2);
				update_mapping_colour_or_auxili(mapping);
				XtSetSensitive(mapping->animate_button,False);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_reset_to_raw_signals.  Unexpected call of this function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_reset_to_raw_signals.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_reset_to_raw_signals */

static void analysis_set_baseline(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 2002

DESCRIPTION :
For every electrode signal in the current region, the baseline is shifted to the
value at the potential time.
==============================================================================*/
{
	float channel_offset,*value;
	int average_width,average_width_after,average_width_before,buffer_offset,i,j,
		potential_time;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal_buffer *buffer;

	ENTER(analysis_set_baseline);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(rig=analysis->rig))
	{
		/* create processed rig */
		if (analysis->raw_rig)
		{
			/* reset the signal range to automatic */
			device=rig->devices;
			for (i=rig->number_of_devices;i>0;i--)
			{
				(*device)->signal_display_maximum=0;
				(*device)->signal_display_minimum=1;
				device++;
			}
		}
		else
		{
			if (rig=create_processed_rig(analysis->rig))
			{
				analysis->raw_rig=analysis->rig;
				analysis->rig=rig;
				/* reset the highlighted device */
				if (highlight= *(analysis->highlight))
				{
					highlight->highlight=0;
					device=analysis->raw_rig->devices;
					i=analysis->raw_rig->number_of_devices;
					while ((i>0)&&(*device!=highlight))
					{
						i--;
						device++;
					}
					if (i>0)
					{
						analysis->highlight=(rig->devices)+
							(device-(analysis->raw_rig->devices));
						(*(analysis->highlight))->highlight=1;
					}
					else
					{
						*(analysis->highlight)=(struct Device *)NULL;
					}
				}
				/* unghost the reset button */
				XtSetSensitive(analysis->window->interval.reset_button,True);
			}
		}
		/* check that rig and signals exist */
		if (rig&&(device=rig->devices)&&(*device)&&
			(buffer=get_Device_signal_buffer(*device)))
		{
			potential_time=analysis->potential_time;
			buffer_offset=buffer->number_of_signals;
			current_region=get_Rig_current_region(rig);
			/* run through all the signals */
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					/* calculate offset,an average of samples_to_average_across signals
						values around the potential_time */
					/*???DB.  Keep consistent with calculate_moving_average in
						analysis_calculate */
					average_width=analysis->average_width;
					if (average_width<1)
					{
						average_width=1;
					}
					average_width_before=average_width/2;
					average_width_after=average_width-1-average_width_before;
					value=(buffer->signals.float_values)+((*device)->signal->index);
					channel_offset=0;
					if (potential_time<average_width_before)
					{
						channel_offset += (float)(average_width_before-potential_time)*
							(*value);
						average_width_before=potential_time;
					}
					else
					{
						value += (potential_time-average_width_before)*buffer_offset;
					}
					for (j=average_width_before;j>0;j--)
					{
						channel_offset += *value;
						value += buffer_offset;
					}
					channel_offset += *value;
					if (potential_time+average_width_after>=buffer->number_of_samples)
					{
						channel_offset += (float)(potential_time+average_width_after+1-
							(buffer->number_of_samples))*(buffer->signals.float_values)[
							((buffer->number_of_samples)-1)*buffer_offset+
							((*device)->signal->index)];
						average_width_after=(buffer->number_of_samples)-1-potential_time;
					}
					for (j=average_width_after;j>0;j--)
					{
						value += buffer_offset;
						channel_offset += *value;
					}
					channel_offset /= (float)average_width;
#if defined (OLD_CODE)
					channel_offset=(buffer->signals.float_values)[potential_time*
						buffer_offset+(*device)->signal->index];
					samples_to_average_across=analysis->average_width;
					channel_offset=0;
					sample_shift=(samples_to_average_across/2);
					/* ensure not averaging off the start/end of the samples */
					if (potential_time>sample_shift)
					{
						if ((buffer->number_of_samples-potential_time)<sample_shift)
						{
							sample_shift=buffer->number_of_samples-potential_time;
							samples_to_average_across=2*sample_shift;
						}
					}
					else
					{
						sample_shift=potential_time;
						samples_to_average_across=2*sample_shift;
					}
					the_time=potential_time-sample_shift;
					for (count=0;count<samples_to_average_across;count++)
					{
						channel_offset+=(buffer->signals.float_values)[the_time*
							buffer_offset+(*device)->signal->index];
						the_time++;
					}
					channel_offset/=samples_to_average_across;
#endif /* defined (OLD_CODE) */
					/* offset signals */
					value=(buffer->signals.float_values)+(*device)->signal->index;
					for (j=buffer->number_of_samples;j>0;j--)
					{
						*value -= channel_offset;
						value += buffer_offset;
					}
				}
				device++;
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			analysis->trace->calculate_rms=1;
			trace_change_signal(analysis->trace);
			/* update the mapping window */
			if ((NO_MAP_FIELD==analysis->map_type)&&
				(mapping=analysis->mapping_window)&&(map=mapping->map))
			{
				analysis->map_type=NO_MAP_FIELD;
				map->colour_option=HIDE_COLOUR;
				map->contours_option=HIDE_CONTOURS;
				map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(mapping,2);
				update_mapping_colour_or_auxili(mapping);
				XtSetSensitive(mapping->animate_button,False);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_baseline.  Invalid analysis_work_area");
	}
	LEAVE;
} /* analysis_set_baseline */

#if defined (UNEMAP_USE_NODES)
static void analysis_unrange_highlighted(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Unrange all the highlighted device signal, i.e range to it's own min/max.
c.f. analysis_set_range_frm_win.
==============================================================================*/
{

	float maximum,minimum;
	struct Analysis_work_area *analysis;
	struct FE_node *rig_node;
	struct FE_field	*display_start_time_field,*display_end_time_field,
		*signal_minimum_field,*signal_maximum_field;
	struct FE_field_component component;
	struct Signal_drawing_package *signal_drawing_package;

	ENTER(analysis_unrange_highlighted);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	rig_node=(struct FE_node *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	display_start_time_field=(struct FE_field *)NULL;
	display_end_time_field=(struct FE_field *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		signal_drawing_package=analysis->signal_drawing_package;
		display_start_time_field=
			get_Signal_drawing_package_display_start_time_field(
			signal_drawing_package);
		display_end_time_field=get_Signal_drawing_package_display_end_time_field(
			signal_drawing_package);
		signal_minimum_field=get_Signal_drawing_package_signal_minimum_field(
			signal_drawing_package);
		signal_maximum_field=get_Signal_drawing_package_signal_maximum_field(
			signal_drawing_package);
		rig_node=analysis->highlight_rig_node;
		/*get the signal min,max for this node */
		get_rig_node_signal_min_max(rig_node,display_start_time_field,
			display_end_time_field,(struct FE_field *)NULL,&minimum,&maximum,
			(enum Event_signal_status *)NULL,	1/*time_range*/);
		/* set the new signal_minimum,signal_maximum*/
		component.field=signal_minimum_field;
		/*??JW should be copying out of and into node with MANAGER_MODIFY */
		set_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,/*time*/0,
			minimum);
		component.field=signal_maximum_field;
		set_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,/*time*/0,
			maximum);
		/* update the display */
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		trace_change_signal(analysis->trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_unrange_highlighted.  Missing analysis_work_area");
	}
	LEAVE;
}/* analysis_unrange_highlighted */
#else
static void analysis_unrange_highlighted(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
Unrange all the highlighted device signal, i.e range to it's own min/max.
c.f. analysis_set_range_frm_win.
==============================================================================*/
{
	float channel_gain,channel_offset,maximum,minimum;
	struct Analysis_work_area *analysis;
	struct Device *highlight;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(analysis_unrange_highlighted);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and highlight exist */
		if ((rig=analysis->rig)&&(analysis->highlight)&&
			(highlight= *(analysis->highlight)))
		{
			current_region=get_Rig_current_region(rig);
			/* for the electrodes in the current region */
			if (((ELECTRODE==highlight->description->type)||
				(AUXILIARY==highlight->description->type))&&(!current_region||
				(current_region==highlight->description->region)))
			{
				/* set the signal minimum and maximum */
				if ((highlight->channel)&&(highlight->signal))
				{
					Signal_get_min_max(highlight->signal,&minimum,&maximum,
						1/*time_range*/);
					channel_offset=highlight->channel->offset;
					channel_gain=highlight->channel->gain;
					if (0<channel_gain)
					{
						highlight->signal_display_minimum=
							channel_gain*((float)(minimum)-channel_offset);
						highlight->signal_display_maximum=
							channel_gain*((float)(maximum)-channel_offset);
					}
					else
					{
						highlight->signal_display_minimum=
							channel_gain*((float)(maximum)-channel_offset);
						highlight->signal_display_maximum=
							channel_gain*((float)(minimum)-channel_offset);
					}
				}
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_unrange_highlighted.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_unrange_highlighted */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static void analysis_unrange_all(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
For every electrode signal in the current region, the range is changed to it's
own min/max
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct FE_field *display_start_time_field,*display_end_time_field,
		*signal_minimum_field,*signal_maximum_field;
	struct FE_region *rig_node_group;
	struct Min_max_iterator *min_max_iterator;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal_drawing_package *signal_drawing_package;

	ENTER(analysis_unrange_all);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	display_start_time_field=(struct FE_field *)NULL;
	display_end_time_field=(struct FE_field *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	current_region=(struct Region *)NULL;
	rig=(struct Rig *)NULL;
	min_max_iterator=(struct Min_max_iterator *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if (rig=analysis->rig)
		{
			if (min_max_iterator=CREATE(Min_max_iterator)())
			{
				signal_drawing_package=analysis->signal_drawing_package;
				display_start_time_field=
					get_Signal_drawing_package_display_start_time_field(
					signal_drawing_package);
				display_end_time_field=
					get_Signal_drawing_package_display_end_time_field(
					signal_drawing_package);
				signal_minimum_field=get_Signal_drawing_package_signal_minimum_field(
					signal_drawing_package);
				signal_maximum_field=get_Signal_drawing_package_signal_maximum_field(
					signal_drawing_package);
				if (current_region=get_Rig_current_region(rig))
				{
					rig_node_group=get_Region_rig_node_group(current_region);
				}
				else
				{
					rig_node_group=get_Rig_all_devices_rig_node_group(rig);
				}
				set_Min_max_iterator_signal_minimum_field(min_max_iterator,signal_minimum_field);
				set_Min_max_iterator_signal_maximum_field(min_max_iterator,signal_maximum_field);
				set_Min_max_iterator_display_start_time_field(min_max_iterator,
					display_start_time_field);
				set_Min_max_iterator_display_end_time_field(min_max_iterator,
					display_end_time_field);
				/* run through all the nodes setting signals min, max */
				FE_region_for_each_FE_node(rig_node_group,
					iterative_unrange_rig_node_signal, (void *)min_max_iterator);
				DESTROY(Min_max_iterator)(&min_max_iterator);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_unrange_all. CREATE(Min_max_iterarot failed)");
			}
		}	/* if (rig=analysis->rig)*/
		/* update the display */
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		trace_change_signal(analysis->trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_unrange_all.  Missing analysis_work_area");
	}
	LEAVE;
}/* analysis_unrange_all */
#else
static void analysis_unrange_all(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
For every electrode signal in the current region, the range is changed to it's
own min/max
==============================================================================*/
{
	float channel_gain,channel_offset,maximum,minimum;
	int i;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(analysis_unrange_all);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			((*device)->signal))
		{
			/* run through all the signals */
			current_region=get_Rig_current_region(rig);
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if (((ELECTRODE==(*device)->description->type)||
					(AUXILIARY==(*device)->description->type))&&(!current_region||
					(current_region==(*device)->description->region))&&
					((*device)->channel)&&((*device)->signal))
				{
					/* set the signal minimum and maximum */
					Signal_get_min_max((*device)->signal,&minimum,&maximum,
						1/*time_range*/);
					channel_offset=(*device)->channel->offset;
					channel_gain=(*device)->channel->gain;
					if (0<channel_gain)
					{
						(*device)->signal_display_minimum=
							channel_gain*((float)(minimum)-channel_offset);
						(*device)->signal_display_maximum=
							channel_gain*((float)(maximum)-channel_offset);
					}
					else
					{
						(*device)->signal_display_minimum=
							channel_gain*((float)(maximum)-channel_offset);
						(*device)->signal_display_maximum=
							channel_gain*((float)(minimum)-channel_offset);
					}
				}
				device++;
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_unrange_all.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_unrange_all */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static void anal_set_range_all_accep_undec(Widget widget,
	XtPointer analysis_work_area,	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Finds the range (min/max) of all ACCEPTED/UNDECIDED signals,
then sets all signals to this range.
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct FE_field *display_start_time_field,*display_end_time_field,
		*signal_minimum_field,*signal_maximum_field,*signal_status_field;
	struct FE_region *rig_node_group;
	struct Min_max_iterator *min_max_iterator;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal_drawing_package *signal_drawing_package;

	ENTER(anal_set_range_all_accep_undec);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	signal_status_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	display_start_time_field=(struct FE_field *)NULL;
	display_end_time_field=(struct FE_field *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	current_region=(struct Region *)NULL;
	min_max_iterator=(struct Min_max_iterator *)NULL;
	rig=(struct Rig *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if (rig=analysis->rig)
		{
			if (min_max_iterator=CREATE(Min_max_iterator)())
			{
				signal_drawing_package=analysis->signal_drawing_package;
				display_start_time_field=
					get_Signal_drawing_package_display_start_time_field(
					signal_drawing_package);
				display_end_time_field=
					get_Signal_drawing_package_display_end_time_field(
					signal_drawing_package);
				signal_minimum_field=get_Signal_drawing_package_signal_minimum_field(
					signal_drawing_package);
				signal_maximum_field=get_Signal_drawing_package_signal_maximum_field(
					signal_drawing_package);
				signal_status_field=get_Signal_drawing_package_signal_status_field(
					signal_drawing_package);
				if (current_region=get_Rig_current_region(rig))
				{
					rig_node_group=get_Region_rig_node_group(current_region);
				}
				else
				{
					rig_node_group=get_Rig_all_devices_rig_node_group(rig);
				}
				set_Min_max_iterator_count(min_max_iterator,0);
				set_Min_max_iterator_started(min_max_iterator,0);
				set_Min_max_iterator_signal_status_field(min_max_iterator,signal_status_field);
				set_Min_max_iterator_display_start_time_field(min_max_iterator,
					display_start_time_field);
				set_Min_max_iterator_display_end_time_field(min_max_iterator,
					display_end_time_field);
				/* run through all the nodes to get accepted,undecided signal's min, max */
				FE_region_for_each_FE_node(rig_node_group,
					iterative_get_rig_node_accepted_undecided_signal_min_max,
						(void *)min_max_iterator);
				/*min_max_iterator.max,min_max_iterator.min now set  */
				set_Min_max_iterator_count(min_max_iterator,0);
				set_Min_max_iterator_signal_minimum_field(min_max_iterator,signal_minimum_field);
				set_Min_max_iterator_signal_maximum_field(min_max_iterator,signal_maximum_field);
				set_Min_max_iterator_display_start_time_field(min_max_iterator,
					display_start_time_field);
				set_Min_max_iterator_display_end_time_field(min_max_iterator,
					display_end_time_field);
				/* run through all the nodes setting signals min, max */
				FE_region_for_each_FE_node(rig_node_group,
					iterative_set_rig_node_signal_min_max,
					(void *)min_max_iterator);
				DESTROY(Min_max_iterator)(&min_max_iterator);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"anal_set_range_all_accep_undec. CREATE(Min_max_iterator failed)");
			}
		}	/* if (rig=analysis->rig)*/
		/* update the display */
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		trace_change_signal(analysis->trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"anal_set_range_all_accep_undec.  Missing analysis_work_area");
	}
	LEAVE;
}/* anal_set_range_all_accep_undec */
#else
static void anal_set_range_all_accep_undec(Widget widget,
	XtPointer analysis_work_area,	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
Finds the range (min/max) of all ACCEPTED/UNDECIDED signals,
then sets all signals to this range.
==============================================================================*/
{
	float all_signals_min,all_signals_max,channel_gain,channel_offset,maximum,
		minimum;
	int i,started;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(anal_set_range_all_accep_undec);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	started=0;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			((*device)->signal))
		{
			/* run through all the ACCEPTED/UNDECIDED electrodes in the current*/
			/* region to get the absolute min/max*/
			current_region=get_Rig_current_region(rig);
			for (i=rig->number_of_devices;i>0;i--)
			{
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region))&&
					(((*device)->signal->status==ACCEPTED)||
					((*device)->signal->status==UNDECIDED))&&((*device)->channel)&&
					((*device)->signal))
				{
					/* get the minimum,maximum */
					Signal_get_min_max((*device)->signal,&minimum,&maximum,
						1/*time_range*/);
					channel_gain=(*device)->channel->gain;
					channel_offset=(*device)->channel->offset;
					if (0<channel_gain)
					{
						minimum=channel_gain*((float)(minimum)-channel_offset);
						maximum=channel_gain*((float)(maximum)-channel_offset);
					}
					else
					{
						minimum=channel_gain*((float)(maximum)-channel_offset);
						maximum=channel_gain*((float)(minimum)-channel_offset);
					}
					if (!started)
					{
						/*initialise the all_signals_min,all_signals_max*/
						all_signals_min=minimum;
						all_signals_max=maximum;
						started=1;
					}
					else
					{
						/*check/set min/max*/
						if (maximum>all_signals_max)
						{
							all_signals_max=maximum;
						}
						if (minimum<all_signals_min)
						{
							all_signals_min=minimum;
						}
					}
				}
				device++;
			}
			/* now set the signal_minimum/signal_maximum of all signals*/
			device=rig->devices;
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					(*device)->signal_display_minimum=all_signals_min;
					(*device)->signal_display_maximum=all_signals_max;
				}
				device++;
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"anal_set_range_all_accep_undec.  Missing analysis_work_area");
	}
	LEAVE;
} /* anal_set_range_all_accep_undec */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static void analysis_set_range_frm_win(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
For every electrode signal in the current region, the range is changed to be the
same as the range for the current signal.
==============================================================================*/
{
	float  maximum,minimum;
	struct Analysis_work_area *analysis;
	struct FE_node *rig_node;
	struct FE_field *signal_minimum_field,*signal_maximum_field;
	struct FE_field_component component;
	struct FE_region *rig_node_group;
	struct Min_max_iterator *min_max_iterator;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal_drawing_package *signal_drawing_package;

	ENTER(analysis_set_range_frm_win);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	rig_node=(struct FE_node *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	current_region=(struct Region *)NULL;
	rig=(struct Rig *)NULL;
	min_max_iterator=(struct Min_max_iterator *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if (rig=analysis->rig)
		{
			if (min_max_iterator=CREATE(Min_max_iterator)())
			{
				signal_drawing_package=analysis->signal_drawing_package;
				signal_minimum_field=get_Signal_drawing_package_signal_minimum_field(
					signal_drawing_package);
				signal_maximum_field=get_Signal_drawing_package_signal_maximum_field(
					signal_drawing_package);
				if (current_region=get_Rig_current_region(rig))
				{
					rig_node_group=get_Region_rig_node_group(current_region);
				}
				else
				{
					rig_node_group=get_Rig_all_devices_rig_node_group(rig);
				}
				rig_node=analysis->highlight_rig_node;
				/*get the channel gain and offset */
				component.number=0;
				component.field=signal_minimum_field;
				get_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,
					/*time*/0,&minimum);
				component.field=signal_maximum_field;
				get_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,
					/*time*/0,&maximum);
				set_Min_max_iterator_max(min_max_iterator,maximum);
				set_Min_max_iterator_min(min_max_iterator,minimum);
				set_Min_max_iterator_signal_minimum_field(min_max_iterator,
					signal_minimum_field);
				set_Min_max_iterator_signal_maximum_field(min_max_iterator,
					signal_maximum_field);
				/* run through all the nodes setting signals min, max */
				FE_region_for_each_FE_node(rig_node_group,
					iterative_set_rig_node_signal_min_max, (void *)min_max_iterator);
				DESTROY(Min_max_iterator)(&min_max_iterator);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_set_range_frm_win. CREATE(Min_max_iterator failed)");
			}
		}	/* if (rig=analysis->rig)*/
		/* update the display */
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		trace_change_signal(analysis->trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_range_frm_win.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_set_range_frm_win */
#else
static void analysis_set_range_frm_win(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 February 2001

DESCRIPTION :
For every electrode signal in the current region, the range is changed to be the
same as the range for the current signal. Note that this is not necessarily the
min,max of the signal, but the display range currently stored.
See also analysis_set_range_frm_sig
==============================================================================*/
{
	float maximum,minimum;
#if defined(NEW_CODE)
	channel_gain,channel_offset;
#endif
	int i;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(analysis_set_range_frm_win);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			((*device)->signal)&&(analysis->highlight)&&
			(highlight= *(analysis->highlight)))
		{
			/* look up the stored range for the highlighed signal */
			minimum=highlight->signal_display_minimum;
			maximum=highlight->signal_display_maximum;
			/* run through all the signals */
			current_region=get_Rig_current_region(rig);
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					(*device)->signal_display_minimum=minimum;
					(*device)->signal_display_maximum=maximum;
				}
				device++;
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_range_frm_win.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_set_range_frm_win */
#endif /* defined (UNEMAP_USE_NODES)*/

static void analysis_set_range_frm_sig(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
For every electrode signal in the current region, the range is changed to be the
same as the min,max for the current signal. See also analysis_set_range_frm_win.
==============================================================================*/
{
	float channel_gain,channel_offset,maximum,minimum;
	int i;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(analysis_set_range_frm_sig);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			((*device)->signal)&&(analysis->highlight)&&
			(highlight= *(analysis->highlight)))
		{
			/*find the current signal min and max*/
			channel_gain=highlight->channel->gain;
			channel_offset=highlight->channel->offset;
			Signal_get_min_max(highlight->signal,&minimum,&maximum,1/*time_range*/);
			if (0<channel_gain)
			{
				minimum=channel_gain*((float)(minimum)-channel_offset);
				maximum=channel_gain*((float)(maximum)-channel_offset);
			}
			else
			{
				minimum=channel_gain*((float)(maximum)-channel_offset);
				maximum=channel_gain*((float)(minimum)-channel_offset);
			}
			/* run through all the signals */
			current_region=get_Rig_current_region(rig);
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					(*device)->signal_display_minimum=minimum;
					(*device)->signal_display_maximum=maximum;
				}
				device++;
			}
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_range_frm_sig.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_set_range_frm_sig */

static int analysis_overlay_signal_file(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Reads in a signals file and adds the signals to the devices in the current rig.
???DB.  UNEMAP_USE_NODES needs doing.  Do as a separate callback ?
==============================================================================*/
{
	FILE *input_file;
	float *float_values,frequency,gain,offset,*temp_float_values,temp_frequency,
		temp_gain,temp_offset,temp_time,temp_time2,time;
	int found,i,index,j,k,l,maximum_signal_number,number_of_common_devices,
		number_of_samples,number_of_signals,return_code,temp_number_of_samples,
		temp_number_of_signals;
	short int *short_int_values,*temp_short_int_values;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight,**temp_device;
	struct Rig *rig,*temp_rig;
	struct Signal *signal;
	struct Signal_buffer *buffer,*temp_buffer;

	ENTER(analysis_overlay_signal_file);
	return_code=0;
	/* check the arguments */
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if (rig=analysis->rig)
		{
			/* open the input file */
			temp_rig=(struct Rig *)NULL;
			input_file=(FILE *)NULL;
			if ((input_file=fopen(file_name,"rb"))&&
				read_signal_file(input_file,&(temp_rig)
#if defined (UNEMAP_USE_3D)
			,analysis->unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
				))
			{
				/* assume that analysis is not done on overlay signals, so don't read
					the event detection settings (see
					analysis_work_area_read_signal_file) */
				/* check for common devices and determine the maximum signal number */
				number_of_common_devices=0;
				maximum_signal_number= -1;
				if ((device=rig->devices)&&(temp_rig->devices))
				{
					for (i=rig->number_of_devices;i>0;i--)
					{
						if ((*device)&&(signal=(*device)->signal))
						{
							do
							{
								if (signal->number>maximum_signal_number)
								{
									maximum_signal_number=signal->number;
								}
								signal=signal->next;
							} while (signal);
							temp_device=temp_rig->devices;
							j=temp_rig->number_of_devices;
							found=0;
							while ((j>0)&&!found)
							{
								if ((*temp_device)&&!strcmp((*device)->description->name,
									(*temp_device)->description->name))
								{
									found=1;
								}
								else
								{
									temp_device++;
									j--;
								}
							}
							if (found&&((*temp_device)->signal)&&
								get_Device_signal_buffer(*temp_device))
							{
								number_of_common_devices++;
							}
						}
						device++;
					}
				}
				if (0<number_of_common_devices)
				{
					/* create processed rig */
					if (analysis->raw_rig)
					{
						/* reset the signal range to automatic */
						device=rig->devices;
						for (i=rig->number_of_devices;i>0;i--)
						{
							(*device)->signal_display_maximum=0;
							(*device)->signal_display_minimum=1;
							device++;
						}
					}
					else
					{
						if (rig=create_processed_rig(analysis->rig))
						{
							analysis->raw_rig=analysis->rig;
							analysis->rig=rig;
							/* reset the highlighted device */
							if (highlight= *(analysis->highlight))
							{
								highlight->highlight=0;
								device=analysis->raw_rig->devices;
								i=analysis->raw_rig->number_of_devices;
								while ((i>0)&&(*device!=highlight))
								{
									i--;
									device++;
								}
								if (i>0)
								{
									analysis->highlight=(rig->devices)+
										(device-(analysis->raw_rig->devices));
									(*(analysis->highlight))->highlight=1;
								}
								else
								{
									*(analysis->highlight)=(struct Device *)NULL;
								}
							}
							/* unghost the reset button */
							XtSetSensitive(analysis->window->interval.reset_button,True);
						}
					}
					if (rig&&(device=rig->devices)&&(*device)&&
						(buffer=get_Device_signal_buffer(*device)))
					{
						/* extend the buffer to include the overlay signals */
						number_of_signals=buffer->number_of_signals;
						number_of_samples=buffer->number_of_samples;
						switch (buffer->value_type)
						{
							case SHORT_INT_VALUE:
							{
								if (REALLOCATE(short_int_values,(buffer->signals).
									short_int_values,short int,number_of_samples*
									(number_of_signals+number_of_common_devices)))
								{
									(buffer->signals).short_int_values=short_int_values;
									/* expand signals */
									short_int_values += number_of_signals*number_of_samples;
									temp_short_int_values=short_int_values+number_of_samples*
										number_of_common_devices;
									for (i=number_of_samples;i>0;i--)
									{
										for (j=number_of_common_devices;j>0;j--)
										{
											temp_short_int_values--;
											*temp_short_int_values=(short int)0;
										}
										for (j=number_of_signals;j>0;j--)
										{
											temp_short_int_values--;
											short_int_values--;
											*temp_short_int_values= *short_int_values;
										}
									}
									return_code=1;
								}
							} break;
							case FLOAT_VALUE:
							{
								if (REALLOCATE(float_values,(buffer->signals).float_values,
									float,number_of_samples*(number_of_signals+
									number_of_common_devices)))
								{
									(buffer->signals).float_values=float_values;
									/* expand signals */
									float_values += number_of_signals*number_of_samples;
									temp_float_values=float_values+number_of_samples*
										number_of_common_devices;
									for (i=number_of_samples;i>0;i--)
									{
										for (j=number_of_common_devices;j>0;j--)
										{
											temp_float_values--;
											*temp_float_values=(float)0;
										}
										for (j=number_of_signals;j>0;j--)
										{
											temp_float_values--;
											float_values--;
											*temp_float_values= *float_values;
										}
									}
									return_code=1;
								}
							} break;
						}
						if (return_code)
						{
							/* add the new signals */
							index=number_of_signals;
							number_of_signals += number_of_common_devices;
							buffer->number_of_signals=number_of_signals;
							maximum_signal_number++;
							frequency=buffer->frequency;
							device=rig->devices;
							i=rig->number_of_devices;
							while (return_code&&(i>0))
							{
								if ((*device)&&(signal=(*device)->signal))
								{
									temp_device=temp_rig->devices;
									j=temp_rig->number_of_devices;
									found=0;
									while (return_code&&(j>0)&&!found)
									{
										if ((*temp_device)&&!strcmp((*device)->description->name,
											(*temp_device)->description->name))
										{
											found=1;
										}
										else
										{
											temp_device++;
											j--;
										}
									}
									if (return_code&&found&&((*temp_device)->signal)&&
										(temp_buffer=get_Device_signal_buffer(*temp_device)))
									{
										while (signal->next)
										{
											signal=signal->next;
										}
										if (signal->next=create_Signal(index,buffer,REJECTED,
											maximum_signal_number))
										{
											/* fill in values */
											switch (buffer->value_type)
											{
												case SHORT_INT_VALUE:
												{
													short_int_values=((buffer->signals).short_int_values)+
														index;
												} break;
												case FLOAT_VALUE:
												{
													float_values=((buffer->signals).float_values)+index;
												} break;
											}
											switch (temp_buffer->value_type)
											{
												case SHORT_INT_VALUE:
												{
													temp_short_int_values=((temp_buffer->signals).
														short_int_values)+((*temp_device)->signal->index);
												} break;
												case FLOAT_VALUE:
												{
													temp_float_values=((temp_buffer->signals).
														float_values)+((*temp_device)->signal->index);
												} break;
											}
											offset=(*device)->channel->offset;
											gain=(*device)->channel->gain;
											temp_offset=(*temp_device)->channel->offset;
											temp_gain=(*temp_device)->channel->gain;
											temp_frequency=temp_buffer->frequency;
											temp_number_of_signals=temp_buffer->number_of_signals;
											temp_number_of_samples=temp_buffer->number_of_samples;
											k=0;
											time=(float)(buffer->times)[0]/frequency;
											l=0;
											temp_time=(float)(temp_buffer->times)[0]/temp_frequency;
											while ((k<buffer->number_of_samples)&&
												((float)(buffer->times)[k]/frequency<temp_time))
											{
												k++;
											}
											while ((k<number_of_samples)&&
												(l<temp_number_of_samples-1))
											{
												time=(float)(buffer->times)[k]/frequency;
												while ((l<temp_number_of_samples-1)&&
													((float)(temp_buffer->times)[l+1]/temp_frequency<
													time))
												{
													l++;
												}
												if (l<temp_number_of_samples-1)
												{
													temp_time=(float)(temp_buffer->times)[l]/
														temp_frequency;
													temp_time2=(float)(temp_buffer->times)[l+1]/
														temp_frequency;
													switch (buffer->value_type)
													{
														case SHORT_INT_VALUE:
														{
															switch (temp_buffer->value_type)
															{
																case SHORT_INT_VALUE:
																{
																	short_int_values[k*number_of_signals]=
																		(short int)(offset+(temp_gain/gain)*
																		(((temp_time2-time)*
																		(float)temp_short_int_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		(float)temp_short_int_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset));
																} break;
																case FLOAT_VALUE:
																{
																	short_int_values[k*number_of_signals]=
																		(short int)(offset+(temp_gain/gain)*
																		(((temp_time2-time)*temp_float_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		temp_float_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset));
																} break;
															}
														} break;
														case FLOAT_VALUE:
														{
															switch (temp_buffer->value_type)
															{
																case SHORT_INT_VALUE:
																{
																	float_values[k*number_of_signals]=offset+
																		(temp_gain/gain)*(((temp_time2-time)*
																		(float)temp_short_int_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		(float)temp_short_int_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset);
																} break;
																case FLOAT_VALUE:
																{
																	float_values[k*number_of_signals]=offset+
																		(temp_gain/gain)*(((temp_time2-time)*
																		temp_float_values[l*
																		temp_number_of_signals]+(time-temp_time)*
																		temp_float_values[(l+1)*
																		temp_number_of_signals])/
																		(temp_time2-temp_time)-temp_offset);
																} break;
															}
														} break;
													}
												}
												k++;
											}
											index++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
							"analysis_overlay_signal_file.  Could not create overlay signal");
											return_code=0;
										}
									}
								}
								device++;
								i--;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"analysis_overlay_signal_file.  Error merging signals");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"analysis_overlay_signal_file.  Could not extend buffer");
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No common devices");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_overlay_signal_file.  Invalid file: %s",file_name);
				return_code=0;
			}
			if (temp_rig)
			{
				destroy_Rig(&temp_rig);
			}
			if (input_file)
			{
				fclose(input_file);
			}
			/* update the drawing areas */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_overlay_signal_file.  Missing rig");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_overlay_signal_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_overlay_signal_file */

static int highlight_analysis_perform_highlighting(
	struct Analysis_work_area *analysis,unsigned int multiple_selection,
	struct Device **new_highlight,int new_device_number,int new_electrode_number,
	int new_auxiliary_number)
/*******************************************************************************
LAST MODIFIED : 19 April 2002

DESCRIPTION :
If the highlight is part of a multiple selection
then
	If the <device> is not highlighted
	then
		highlight it and make it THE highlighted device for the analysis work area
	else
		if it is the only highlighted device
		then
			do nothing
		else
			dehighlight it
			if it is THE highlighted device for the analysis work area
			then
				make the first highlighted device THE highlighted device
else
	highlight it and dehighlight all other devices
	make it THE highlighted device for the analysis work area
*******************************************************************************/
{
	enum Device_type device_type;
	int i,old_auxiliary_number,old_device_number,old_electrode_number,return_code,
			start_analysis_interval,end_analysis_interval;
	struct Device **old_highlight,**temp_device;
	struct Device_description *description;
	struct Interval_area *interval;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;

	ENTER(highlight_analysis_perform_highlighting);
	if (analysis&&(analysis->rig)&&(analysis->rig->devices)&&
		(*(analysis->rig->devices))&&
		(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
	{
		if (new_highlight)
		{
			return_code=1;
			current_region=get_Rig_current_region(analysis->rig);
			if (analysis->window)
			{
				signals= &(analysis->window->signals);
				interval= &(analysis->window->interval);
			}
			else
			{
				signals=(struct Signals_area *)NULL;
				interval=(struct Interval_area *)NULL;
			}
			if ((analysis->mapping_window)&&(analysis->mapping_window->map))
			{
				mapping=analysis->mapping_window;
				map=mapping->map;
			}
			else
			{
				mapping=(struct Mapping_window *)NULL;
				map=(struct Map *)NULL;
			}
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			/* if the highlight is part of a multiple selection */
			if (multiple_selection)
			{
				/* if the device is highlighted */
				if ((*new_highlight)->highlight)
				{
					/* determine whether or not the device is the only highlighted
						device */
					if (new_highlight==analysis->highlight)
					{
						temp_device=analysis->rig->devices;
						i=analysis->rig->number_of_devices;
						while ((i>0)&&
							(!((*temp_device)->highlight)||(temp_device==new_highlight)))
						{
							temp_device++;
							i--;
						}
						if (i>0)
						{
							analysis->highlight=temp_device;
							if (interval)
							{
								update_interval_drawing_area(analysis->window);
							}
							/* update the trace window */
							trace_change_signal(analysis->trace);
						}
					}
					/* if it is not the only highlighted device */
					if (new_highlight!=analysis->highlight)
					{
						/* dehighlight the selected device */
						(*new_highlight)->highlight=0;
						highlight_signal(*new_highlight,
#if defined (UNEMAP_USE_NODES)
							(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
							new_device_number,start_analysis_interval,end_analysis_interval,
							analysis->datum,analysis->potential_time,signals,
							analysis->signal_drawing_information,analysis->user_interface,
							&(analysis->window->interval));
						highlight_electrode_or_auxiliar(*new_highlight,
#if defined (UNEMAP_USE_NODES)
							(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
							new_electrode_number,new_auxiliary_number,map,mapping);
					}
				}
				else
				{
					/* highlight it and make it THE highlighted device for the analysis
						work area */
					analysis->highlight=new_highlight;
					(*new_highlight)->highlight=1;
					highlight_signal(*new_highlight,
#if defined (UNEMAP_USE_NODES)
						(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
						new_device_number,start_analysis_interval,end_analysis_interval,
						analysis->datum,analysis->potential_time,signals,
						analysis->signal_drawing_information,analysis->user_interface,
						&(analysis->window->interval));
					if (interval)
					{
						update_interval_drawing_area(analysis->window);
					}
					/* update the trace window */
					trace_change_signal(analysis->trace);
					highlight_electrode_or_auxiliar(*new_highlight,
#if defined (UNEMAP_USE_NODES)
						(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
						new_electrode_number,new_auxiliary_number,map,mapping);
				}
			}
			else
			{
				/* highlight the device and dehighlight all other devices */
				old_highlight=analysis->rig->devices;
				old_device_number=0;
				old_electrode_number=0;
				old_auxiliary_number=0;
				for (i=analysis->rig->number_of_devices;i>0;i--)
				{
					if ((current_region==(description=(*old_highlight)->description)->
						region)||(!current_region))
					{
						if (ELECTRODE==(device_type=description->type))
						{
							if ((*old_highlight)->highlight)
							{
								(*old_highlight)->highlight=0;
								highlight_signal(*old_highlight,
#if defined (UNEMAP_USE_NODES)
									(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
									old_device_number,start_analysis_interval,
									end_analysis_interval,analysis->datum,
									analysis->potential_time,signals,
									analysis->signal_drawing_information,analysis->user_interface,
									&(analysis->window->interval));
								highlight_electrode_or_auxiliar(*old_highlight,
#if defined (UNEMAP_USE_NODES)
									(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
									old_electrode_number,-1,map,mapping);
							}
							old_electrode_number++;
						}
						else
						{
							if (AUXILIARY==device_type)
							{
								if ((*old_highlight)->highlight)
								{
									(*old_highlight)->highlight=0;
									highlight_signal(*old_highlight,
#if defined (UNEMAP_USE_NODES)
										(struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
										old_device_number,start_analysis_interval,
										end_analysis_interval,analysis->datum,
										analysis->potential_time,signals,
										analysis->signal_drawing_information,
										analysis->user_interface,
										&(analysis->window->interval));
									highlight_electrode_or_auxiliar(*old_highlight,
#if defined (UNEMAP_USE_NODES)
										(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
										-1,old_auxiliary_number,map,mapping);
								}
								old_auxiliary_number++;
							}
						}
						old_device_number++;
					}
					else
					{
						if ((*old_highlight)->highlight)
						{
							(*old_highlight)->highlight=0;
						}
					}
					old_highlight++;
				}
				/* highlight the new device */
				analysis->highlight=new_highlight;
				if (new_highlight)
				{
					(*new_highlight)->highlight=1;
					highlight_signal(*new_highlight,
#if defined (UNEMAP_USE_NODES)
						(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
						new_device_number,start_analysis_interval,end_analysis_interval,
						analysis->datum,analysis->potential_time,signals,
						analysis->signal_drawing_information,analysis->user_interface,
						&(analysis->window->interval));
					if (interval)
					{
						update_interval_drawing_area(analysis->window);
					}
					/* update the trace window */
					trace_change_signal(analysis->trace);
					highlight_electrode_or_auxiliar(*new_highlight,
#if defined (UNEMAP_USE_NODES)
						(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
						new_electrode_number,	new_auxiliary_number,map,mapping);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"highlight_analysis_perform_highlighting.  "
				"Must always be a highlighted device");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"highlight_analysis_perform_highlighting. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* highlight_analysis_perform_highlighting */

static int accept_signal(struct Analysis_work_area *analysis,
	struct Device **signal_device)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION : accept the analysis signal.
==============================================================================*/
{
	char *value_string;
	float level,*objective_values;
	int average_width,device_number,i,minimum_separation,
		number_of_objective_values,objective_values_step,return_code,
		threshold_percentage,xpos,ypos;
	struct Device **device;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;
#if defined(UNEMAP_USE_3D)
	struct Cmiss_region *root_cmiss_region;
	struct FE_field *device_name_field,*signal_status_field;
	struct FE_node *rig_node,*node;
	struct FE_region *rig_node_group,*unrejected_node_group;
	struct FE_region *root_fe_region;
#endif /* defined(UNEMAP_USE_3D) */

	ENTER(accept_signal);
#if defined(UNEMAP_USE_3D)
	device_name_field=(struct FE_field *)NULL;
	signal_status_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	unrejected_node_group=(struct FE_region *)NULL;
#endif /* defined(UNEMAP_USE_3D) */
	if (analysis&&analysis->trace)
	{
		return_code=1;
		if ((rig=analysis->rig)&&(signal=(*signal_device)->signal)&&
			(buffer=signal->buffer))
		{
			if (signal->status!=ACCEPTED)
			{
				if ((analysis->calculate_events)&&(REJECTED==signal->status))
				{
					/* calculate events for the device */
					average_width=analysis->average_width;
					switch (analysis->detection)
					{
						case EDA_LEVEL:
						{
							value_string=(char *)NULL;
							XtVaGetValues(analysis->trace->area_1.enlarge.level_value,
								XmNvalue,&value_string,
								NULL);
							if (1==sscanf(value_string,"%f",&level))
							{
								if (level<0)
								{
									level=0;
								}
							}
							else
							{
								level=analysis->level;
							}
							analysis->level=level;
							XtFree(value_string);
							sprintf(global_temp_string,"%g",level);
							XtVaSetValues(analysis->trace->area_1.enlarge.level_value,
								XmNvalue,global_temp_string,
								NULL);
							value_string=(char *)NULL;
							XtVaGetValues(analysis->trace->menu.average_width_txt,
								XmNvalue,&value_string,
								NULL);
							if (1==sscanf(value_string,"%d",&average_width))
							{
								if (average_width<1)
								{
									average_width=1;
								}
							}
							else
							{
								average_width=analysis->average_width;
							}
							analysis->average_width=average_width;
							XtFree(value_string);
							sprintf(global_temp_string,"%d",average_width);
							XtVaSetValues(analysis->trace->menu.average_width_txt,
								XmNvalue,global_temp_string,
								NULL);
						} break;
						case EDA_THRESHOLD:
						{
							XtVaGetValues(analysis->trace->area_1.enlarge.threshold_scroll,
								XmNvalue,&threshold_percentage,
								NULL);
							XtVaGetValues(
								analysis->trace->area_1.enlarge.minimum_separation_scroll,
								XmNvalue,&minimum_separation,
								NULL);
						} break;
					}
					if ((analysis->trace)&&(analysis->trace->processed_device)&&
						(analysis->trace->processed_device->signal)&&
						(analysis->trace->processed_device->signal->buffer)&&
						(objective_values=analysis->trace->processed_device->signal->
						buffer->signals.float_values)&&(0<(objective_values_step=analysis->
						trace->processed_device->signal->buffer->number_of_signals))&&
						(0<(number_of_objective_values=analysis->trace->processed_device->
						signal->buffer->number_of_samples)))
					{
						calculate_device_objective(*signal_device,analysis->detection,
							analysis->objective,objective_values,number_of_objective_values,
							objective_values_step,average_width);
						calculate_device_event_markers(*signal_device,
							analysis->start_search_interval,analysis->end_search_interval,
							analysis->detection,objective_values,number_of_objective_values,
							objective_values_step,analysis->number_of_events,
							threshold_percentage,minimum_separation,level);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"accept_signal.  Invalid processed_device");
					}
					/* draw the events */
					if (analysis->window)
					{
						signals= &(analysis->window->signals);
						/* determine the number of the signal_device signal */
						device=rig->devices;
						current_region=get_Rig_current_region(rig);
						device_number=0;
						for (i=signal_device-device;i>0;i--)
						{
							if ((current_region==((*device)->description->region))||
								(!current_region))
							{
								device_number++;
							}
							device++;
						}
						switch (signals->layout)
						{
							case SEPARATE_LAYOUT:
							{
								xpos=((device_number/(signals->number_of_rows))*
									(signals->drawing->width))/(signals->number_of_columns);
								ypos=((device_number%(signals->number_of_rows))*
									(signals->drawing->height))/(signals->number_of_rows);
							} break;
							case OVERLAP_LAYOUT:
							{
								xpos=((device_number/(signals->number_of_rows))*
									(signals->drawing->width))/(signals->number_of_columns);
								ypos=((device_number%(signals->number_of_rows))*
									(signals->drawing->height))/(signals->number_of_rows+3);
							} break;
						}
						draw_device_markers(*signal_device,buffer->start,buffer->end,
							analysis->datum,1,analysis->potential_time,1,SIGNAL_AREA_DETAIL,0,
							xpos+signals->axes_left,ypos+signals->axes_top,
							signals->axes_width,signals->axes_height,
							XtWindow(signals->drawing_area),signals->drawing->pixel_map,
							analysis->signal_drawing_information,analysis->user_interface);
					}
				}
				/* change the signal status */
				signal->status=ACCEPTED;
#if defined (UNEMAP_USE_3D)
				/*???JW.  Reject the corresponding node (until have a complete nodal
					version of analysis_accept_signal) */
				/* get the rig_node corresponding to the device */
				device_name_field=get_unemap_package_device_name_field(
					analysis->unemap_package);
				rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
				rig_node=find_rig_node_given_device(*signal_device,rig_node_group,
					device_name_field);
				signal_status_field=
					get_unemap_package_signal_status_field(analysis->unemap_package);
				root_cmiss_region =
					get_unemap_package_root_Cmiss_region(analysis->unemap_package);
				root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
				/* create a node to work with */
				if (node = CREATE(FE_node)(get_FE_node_identifier(rig_node),
					(struct FE_region *)NULL, rig_node))
				{
					ACCESS(FE_node)(node);
					set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
						"ACCEPTED");
					/* merge it into the FE_region */
					FE_region_merge_FE_node(root_fe_region, node);
					/* clean up the working node */
					DEACCESS(FE_node)(&node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"accept_signal.  Could not make working node");
				}
				/* add node to unrejected group, as it's now  visible*/
				current_region=get_Rig_current_region(rig);
				if (!current_region)
				/* add node to all regions unrejected_node_groups */
				{
					struct Region_list_item *region_item=(struct Region_list_item *)NULL;
					struct Region *region=(struct Region *)NULL;

					region_item=get_Rig_region_list(rig);
					while (region_item)
					{
						region=get_Region_list_item_region(region_item);
						if (unrejected_node_group=get_Region_unrejected_node_group(region))
						{
							if (!(FE_region_contains_FE_node(unrejected_node_group,rig_node)))
							{
								FE_region_merge_FE_node(unrejected_node_group,rig_node);
							}
						}
						region_item=get_Region_list_item_next(region_item);
					}/* while (region_item)*/
				}
				else
				/* just one region */
				{
					unrejected_node_group=get_Region_unrejected_node_group(
						current_region);
					if (!(FE_region_contains_FE_node(unrejected_node_group,rig_node)))
					{
						FE_region_merge_FE_node(unrejected_node_group,rig_node);
					}
				}
#endif /* defined(UNEMAP_USE_3D) */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"accept_signal.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* accept_signal */

static int reject_signal(struct Analysis_work_area *analysis,
	struct Device **signal_device)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Reject the  <signal> in <analysis>
==============================================================================*/
{
	int device_number,i,return_code,xpos,ypos;
	struct Device **device;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;
#if defined(UNEMAP_USE_3D)
	struct Cmiss_region *root_cmiss_region;
	struct FE_field *device_name_field,*signal_status_field;
	struct FE_node *rig_node,*node;
	struct FE_region *rig_node_group,*unrejected_node_group;
	struct FE_region *root_fe_region;
#endif /* defined(UNEMAP_USE_3D) */

	ENTER(reject_signal);
#if defined(UNEMAP_USE_3D)
	device_name_field=(struct FE_field *)NULL;
	signal_status_field=(struct FE_field *)NULL;
	rig_node=(struct FE_node *)NULL;
	node=(struct FE_node *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	unrejected_node_group=(struct FE_region *)NULL;
#endif /* defined(UNEMAP_USE_3D) */
	if (analysis&&signal_device&&(*signal_device))
	{
		return_code=1;
		if ((rig=analysis->rig)&&(signal=(*signal_device)->signal)&&
			(buffer=signal->buffer))
		{
			if (signal->status!=REJECTED)
			{
				if (analysis->calculate_events)
				{
					/* clear the event markers */
					if (analysis->window)
					{
						signals= &(analysis->window->signals);
						/* determine the number of the  signal */
						device=rig->devices;
						current_region=get_Rig_current_region(rig);
						device_number=0;
						for (i=signal_device-device;i>0;i--)
						{
							if ((current_region==((*device)->description->region))||
								(!current_region))
							{
								device_number++;
							}
							device++;
						}
						switch (signals->layout)
						{
							case SEPARATE_LAYOUT:
							{
								xpos=((device_number/(signals->number_of_rows))*
									(signals->drawing->width))/(signals->number_of_columns);
								ypos=((device_number%(signals->number_of_rows))*
									(signals->drawing->height))/(signals->number_of_rows);
							} break;
							case OVERLAP_LAYOUT:
							{
								xpos=((device_number/(signals->number_of_rows))*
									(signals->drawing->width))/(signals->number_of_columns);
								ypos=((device_number%(signals->number_of_rows))*
									(signals->drawing->height))/(signals->number_of_rows+3);
							} break;
						}
						/* redraw the signal */
						draw_device_markers(*signal_device,buffer->start,buffer->end,
							analysis->datum,1,analysis->potential_time,1,SIGNAL_AREA_DETAIL,0,
							xpos+signals->axes_left,ypos+signals->axes_top,
							signals->axes_width,signals->axes_height,
							XtWindow(signals->drawing_area),signals->drawing->pixel_map,
							analysis->signal_drawing_information,analysis->user_interface);
					}
					destroy_Event_list(&(signal->first_event));
				}
				/* change the signal status */
				signal->status=REJECTED;
#if defined(UNEMAP_USE_3D)
				/*???JW.  Reject the corresponding node (until have a complete nodal
					version of reject_signal) */
				/* get the rig_node corresponding to the device */
				device_name_field=get_unemap_package_device_name_field(
					analysis->unemap_package);
				rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
				rig_node=find_rig_node_given_device(*signal_device,rig_node_group,
					device_name_field);
				signal_status_field=get_unemap_package_signal_status_field(
					analysis->unemap_package);
				root_cmiss_region =
					get_unemap_package_root_Cmiss_region(analysis->unemap_package);
				root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
				if (node = CREATE(FE_node)(get_FE_node_identifier(rig_node),
					(struct FE_region *)NULL, rig_node))
				{
					ACCESS(FE_node)(node);
					set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
						"REJECTED");
					/* merge it into the FE_region */
					FE_region_merge_FE_node(root_fe_region, node);
					/* clean up the working node */
					DEACCESS(FE_node)(&node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"reject_signal.  Could not make working node");
				}
#if defined (OLD_CODE)
				/* unselect the node, as we don't want it to be visible */
				FE_node_selection_unselect_node(
					get_unemap_package_FE_node_selection(analysis->unemap_package),
					rig_node);
#endif /* defined (OLD_CODE) */
				/* remove node from  unrejected group */
				current_region=get_Rig_current_region(rig);
				if (!current_region)
				/* remove node from all regions unrejected_node_groups */
				{
					struct Region_list_item *region_item=(struct Region_list_item *)NULL;
					struct Region *region=(struct Region *)NULL;

					region_item=get_Rig_region_list(rig);
					while (region_item)
					{
						region=get_Region_list_item_region(region_item);
						if (unrejected_node_group=get_Region_unrejected_node_group(region))
						{
							if (FE_region_contains_FE_node(unrejected_node_group,rig_node))
							{
								FE_region_remove_FE_node(unrejected_node_group,rig_node);
							}
						}
						region_item=get_Region_list_item_next(region_item);
					}/* while (region_item)*/
				}
				else
				{
					/* just one region */
					unrejected_node_group=get_Region_unrejected_node_group(
						current_region);
					if (FE_region_contains_FE_node(unrejected_node_group,rig_node))
					{
						FE_region_remove_FE_node(unrejected_node_group,rig_node);
					}
				}
#endif /* defined(UNEMAP_USE_3D) */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reject_signal.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* reject_signal */

static int analysis_accept_or_reject_signal(struct Analysis_work_area *analysis,
	int accept_reject)
/*******************************************************************************
LAST MODIFIED : 8 May 2003

DESCRIPTION :
Accept or reject the <analysis> highlighted signals.  <accept_reject> a flag,
1=accept, 0 reject.
==============================================================================*/
{
	int i,return_code;
	struct Device **device;
	struct Mapping_window *mapping;
	struct Rig *rig;
#if defined(UNEMAP_USE_3D)
	struct Cmiss_region *root_cmiss_region;
	struct FE_node_selection *node_selection;
	struct FE_region *root_fe_region;
#endif/* defined(UNEMAP_USE_3D)*/

	ENTER(analysis_accept_or_reject_signal);
	device=(struct Device **)NULL;
	mapping=(struct Mapping_window *)NULL;
	rig=(struct Rig *)NULL;
	if (analysis)
	{
		return_code=1;
		if (rig=analysis->rig)
		{
#if defined (UNEMAP_USE_3D)
			node_selection=(struct FE_node_selection *)NULL;
			root_cmiss_region =
				get_unemap_package_root_Cmiss_region(analysis->unemap_package);
			root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
			node_selection=get_unemap_package_FE_node_selection(
				analysis->unemap_package);
			FE_node_selection_begin_cache(node_selection);
			FE_region_begin_change(root_fe_region);
#endif /* defined (UNEMAP_USE_3D) */
			/* accept/reject all the highlighted signals  */
			device=rig->devices;
			for (i=0;i<rig->number_of_devices;i++)
			{
				if ((*device)->highlight)
				{
					if (accept_reject)
					{
						accept_signal(analysis,device);
					}
					else
					{
						reject_signal(analysis,device);
					}
				}
				device++;
			}
			/* redraw the signal */
			update_interval_drawing_area(analysis->window);
			trace_change_signal_status(analysis->trace);
			/* to recalculate RMS_signal. Perhaps should do trace_change_signal for
				all modes */
			if (*analysis->trace->analysis_mode==ELECTRICAL_IMAGING)
			{
				analysis->trace->calculate_rms=1;
				trace_change_signal(analysis->trace);
			}
			mapping=analysis->mapping_window;
			if (mapping&&(mapping->map))
			{
				/* we've accepted or rejected an signal so set the flag  */
				if (mapping->map->drawing_information)
				{
					set_map_drawing_information_electrodes_accepted_or_rejected
						(mapping->map->drawing_information,1);
				}
#if defined(UNEMAP_USE_3D)
				update_mapping_drawing_area(mapping,0);
				update_mapping_colour_or_auxili(mapping);
#else/* defined(UNEMAP_USE_3D) */
				update_mapping_drawing_area(mapping,0);
#endif /* defined(UNEMAP_USE_3D) */
			}	/* if (mapping&&(mapping->map)) */
#if defined (OLD_CODE)
#if !defined (UNEMAP_USE_3D)
			if (!accept_reject)
			{
				/* for rejection, unhighlight all the devices (this is already done for
					UNEMAP_USE_3D in rig_node_group_node_selection_change */
				highlight_analysis_perform_highlighting(analysis,
					0/*multiple_selection*/,(struct Device **)NULL,0/*new_device_number*/,
					0/* new_electrode_number*/,0/* new_auxiliary_number*/);
			}
#endif /* defined (UNEMAP_USE_3D) */
#endif /* defined (OLD_CODE) */
#if defined(UNEMAP_USE_3D)
			FE_region_end_change(root_fe_region);
			FE_node_selection_end_cache(node_selection);
#endif/* defined(UNEMAP_USE_3D)*/
		}/* if (rig=analysis->rig)*/
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"analysis_accept_or_reject_signal.  invalid argument");
	}
	LEAVE;

	return (return_code);
} /* analysis_accept_or_reject_signal */

static void analysis_accept_signal(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 November 2001

DESCRIPTION : accept the analysis highlighted signals
==============================================================================*/
{
	struct Analysis_work_area *analysis;

	ENTER(analysis_accept_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	analysis=(struct Analysis_work_area *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		analysis_accept_or_reject_signal(analysis,1/*accept_reject*/);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_accept_signal.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_accept_signal */

static void analysis_reject_signal(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 November 2001

DESCRIPTION : reject the analysis highlighted signals
==============================================================================*/
{
	struct Analysis_work_area *analysis;

	ENTER(analysis_reject_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	analysis=(struct Analysis_work_area *)NULL;
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		analysis_accept_or_reject_signal(analysis,0/*accept_reject*/);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_reject_signal.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_reject_signal */

static void analysis_previous_signal(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 October 1992

DESCRIPTION : move to the previous analysis signal
==============================================================================*/
{
	struct Analysis_work_area *analysis;
#if defined (UNEMAP_USE_NODES)
	char *device_type_string,*signal_status_string;
	struct FE_field *device_type_field,*signal_status_field;
	struct FE_node *highlight_rig_node;
	struct Signal_drawing_package *signal_drawing_package;
	struct FE_node_order_info *rig_node_order_info;
#else
	struct Device **highlight;
#endif /* defined (UNEMAP_USE_NODES)*/

	ENTER(analysis_previous_signal);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
#if defined (UNEMAP_USE_NODES)
		/*??JW until have analysis_accept_signal for nodes*/
		USE_PARAMETER(widget);
		USE_PARAMETER(call_data);
		device_type_string=(char *)NULL;
		signal_status_string=(char *)NULL;
		device_type_field=(struct FE_field *)NULL;
		signal_status_field=(struct FE_field *)NULL;
		signal_drawing_package=(struct Signal_drawing_package *)NULL;
		rig_node_order_info=(struct FE_node_order_info *)NULL;
		highlight_rig_node=(struct FE_node *)NULL;
		if ((analysis->rig)&&(highlight_rig_node=analysis->highlight_rig_node)&&
			(signal_drawing_package=analysis->signal_drawing_package)&&
			(device_type_field=
			get_Signal_drawing_package_device_type_field(signal_drawing_package))&&
			(signal_status_field=
			get_Signal_drawing_package_signal_status_field(signal_drawing_package))&&
			(rig_node_order_info=get_Analysis_window_rig_node_order_info(
			analysis->window)))
		{
			get_FE_nodal_string_value(highlight_rig_node,device_type_field,0,0,
						FE_NODAL_VALUE,&device_type_string);
			if (!strcmp(device_type_string,"ELECTRODE"))
			{
				/* accept the current signal if undecided */
				get_FE_nodal_string_value(highlight_rig_node,signal_status_field,0,0,
						FE_NODAL_VALUE,&signal_status_string);
				if (!strcmp(device_type_string,"UNDECIDED"))
				{
					/*??JW need a nodal version of this */
					/*
					analysis_accept_signal(widget,analysis_work_area,call_data);
					*/
				}
				DEALLOCATE(signal_status_string);
			}
			DEALLOCATE(device_type_string);
			/* move to the prev signal */
			highlight_rig_node=get_FE_node_order_info_prev_node(rig_node_order_info);
			if (highlight_rig_node)
			{
				highlight_analysis_device_node(0,highlight_rig_node,(int *)NULL,
					(int *)NULL,(int *)NULL,analysis);
			}
		}
#else
		if ((analysis->rig)&&(highlight=analysis->highlight))
		{
			if (ELECTRODE==(*highlight)->description->type)
			{
				/* accept the current signal if undecided */
				if (((*highlight)->signal)&&(UNDECIDED==(*highlight)->signal->status))
				{
					analysis_accept_signal(widget,analysis_work_area,call_data);
				}
			}
			/* move to the previous signal */
			highlight--;
			highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
				(int *)NULL,analysis);
		}
#endif /* defined (UNEMAP_USE_NODES)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_previous_signal.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_previous_signal */

static void analysis_next_signal(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION : move to the next analysis signal
==============================================================================*/
{
	struct Analysis_work_area *analysis;
#if defined (UNEMAP_USE_NODES)
	char *device_type_string,*signal_status_string;
	struct FE_field *device_type_field,*signal_status_field;
	struct FE_node *highlight_rig_node;
	struct Signal_drawing_package *signal_drawing_package;
	struct FE_node_order_info *rig_node_order_info;
#else
	struct Device **highlight;
#endif /* defined (UNEMAP_USE_NODES)*/

	ENTER(analysis_next_signal);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
#if defined (UNEMAP_USE_NODES)
		/*??JW until have analysis_accept_signal for nodes*/
		USE_PARAMETER(widget);
		USE_PARAMETER(call_data);
		device_type_string=(char *)NULL;
		signal_status_string=(char *)NULL;
		device_type_field=(struct FE_field *)NULL;
		signal_status_field=(struct FE_field *)NULL;
		signal_drawing_package=(struct Signal_drawing_package *)NULL;
		rig_node_order_info=(struct FE_node_order_info *)NULL;
		highlight_rig_node=(struct FE_node *)NULL;
		if ((analysis->rig)&&(highlight_rig_node=analysis->highlight_rig_node)&&
			(signal_drawing_package=analysis->signal_drawing_package)&&
			(device_type_field=
			get_Signal_drawing_package_device_type_field(signal_drawing_package))&&
			(signal_status_field=
			get_Signal_drawing_package_signal_status_field(signal_drawing_package))&&
			(rig_node_order_info=get_Analysis_window_rig_node_order_info(
			analysis->window)))
		{
			get_FE_nodal_string_value(highlight_rig_node,device_type_field,0,0,
						FE_NODAL_VALUE,&device_type_string);
			if (!strcmp(device_type_string,"ELECTRODE"))
			{
				/* accept the current signal if undecided */
				get_FE_nodal_string_value(highlight_rig_node,signal_status_field,0,0,
						FE_NODAL_VALUE,&signal_status_string);
				if (!strcmp(device_type_string,"UNDECIDED"))
				{
					/*??JW need a nodal version of this */
					/*
					analysis_accept_signal(widget,analysis_work_area,call_data);
					*/
				}
				DEALLOCATE(signal_status_string);
			}
			DEALLOCATE(device_type_string);
			/* move to the next signal */
			highlight_rig_node=get_FE_node_order_info_next_node(rig_node_order_info);
			if (highlight_rig_node)
			{
				highlight_analysis_device_node(0,highlight_rig_node,(int *)NULL,
					(int *)NULL,(int *)NULL,analysis);
			}
		}
#else
		if ((analysis->rig)&&(highlight=analysis->highlight))
		{
			if (ELECTRODE==(*highlight)->description->type)
			{
				/* accept the current signal if undecided */
				if (((*highlight)->signal)&&(UNDECIDED==(*highlight)->signal->status))
				{
					analysis_accept_signal(widget,analysis_work_area,call_data);
				}
			}
			/* move to the next signal */
			highlight++;
			highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,
				(int *)NULL,analysis);
		}
#endif /* defined (UNEMAP_USE_NODES)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_next_signal.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_next_signal */

static void trace_analysis_mode_apply(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Applies the current analysis mode settings to all signals.
==============================================================================*/
{
	float beat_width,*imaginary_value,*processed_value,*real_value,*value,
		value_end,value_start,*value_1,*value_2,x,y;
	int average_end,average_start,average_width,*beat_count,*beat_counts,beat_end,
		beat_number,beat_start,buffer_offset_1,buffer_offset_2,*divisions,end,
		high_pass,i,j,low_pass,notch,max_times,number_of_beats,number_of_samples,
		*processed_time,start,start_time,*time,*time_offset,transform_buffer_offset,
		transform_number_of_samples;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Event *event;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Signal_buffer *buffer,*transform_buffer;
	struct Trace_window *trace;
	XmString xm_string;

	ENTER(trace_analysis_mode_apply);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(rig=analysis->rig)&&(trace=analysis->trace))
	{
		busy_cursor_on((Widget)NULL,analysis->user_interface);
		average_width=analysis->average_width;
		switch (*trace->analysis_mode)
		{
			case BEAT_AVERAGING:
			{
				/* create processed rig */
				if (analysis->raw_rig)
				{
					/* reset the signal range to automatic */
					device=rig->devices;
					for (i=rig->number_of_devices;i>0;i--)
					{
						(*device)->signal_display_maximum=0;
						(*device)->signal_display_minimum=1;
						device++;
					}
				}
				else
				{
					if (rig=create_processed_rig(analysis->rig))
					{
						analysis->raw_rig=analysis->rig;
						analysis->rig=rig;
						/* reset the highlighted device */
						if (highlight= *(analysis->highlight))
						{
							highlight->highlight=0;
							device=analysis->raw_rig->devices;
							i=analysis->raw_rig->number_of_devices;
							while ((i>0)&&(*device!=highlight))
							{
								i--;
								device++;
							}
							if (i>0)
							{
								analysis->highlight=(rig->devices)+
									(device-(analysis->raw_rig->devices));
								(*(analysis->highlight))->highlight=1;
							}
							else
							{
								*(analysis->highlight)=(struct Device *)NULL;
							}
						}
						/* unghost the reset button */
						XtSetSensitive(analysis->window->interval.reset_button,True);
					}
				}
				/* check that rig and signals exist */
				if (rig&&(device=rig->devices)&&(*device)&&
					(buffer=get_Device_signal_buffer(*device))&&
					(0<(number_of_samples=buffer->number_of_samples)))
				{
					buffer_offset_1=buffer->number_of_signals;
					if (True==XmToggleButtonGadgetGetState((trace->area_3).
						beat_averaging.baseline_toggle))
					{
						device=rig->devices;
						start= *(trace->event_detection.start_search_interval);
						divisions= *(trace->event_detection.search_interval_divisions);
						end= *(trace->event_detection.end_search_interval);
						number_of_beats= *(trace->event_detection.number_of_events);
						/* update the times */
						if (0<start)
						{
							processed_time=buffer->times;
							time=processed_time+start;
							for (i=end-start;i>=0;i--)
							{
								*processed_time= *time;
								time++;
								processed_time++;
							}
						}
						/* run through all the signals */
						for (j=rig->number_of_devices;j>0;j--)
						{
							if ((*device)->signal)
							{
								beat_start=start;
								average_start=beat_start-average_width/2;
								if (average_start<0)
								{
									average_start=0;
								}
								average_end=average_start+average_width;
								if (average_end>number_of_samples)
								{
									average_end=number_of_samples;
								}
								value=((buffer->signals).float_values)+
									(((*device)->signal->index)+average_start*buffer_offset_1);
								value_end=0;
								for (i=average_end-average_start;i>0;i--)
								{
									value_end += *value;
									value += buffer_offset_1;
								}
								value_end /= (float)(average_end-average_start);
								beat_end=beat_start-1;
								processed_value=((buffer->signals).float_values)+
									((*device)->signal->index);
								for (beat_number=1;beat_number<=number_of_beats;beat_number++)
								{
									value_start=value_end;
									beat_start=beat_end+1;
									if (beat_number<number_of_beats)
									{
										if (divisions)
										{
											beat_end=divisions[beat_number-1]-1;
										}
										else
										{
											beat_end=start+(int)((float)((end-start)*beat_number)/
												(float)number_of_beats+0.5)-1;
										}
									}
									else
									{
										beat_end=end;
									}
									average_start=beat_end-average_width/2;
									if (average_start<0)
									{
										average_start=0;
									}
									average_end=average_start+average_width;
									if (average_end>number_of_samples)
									{
										average_end=number_of_samples;
									}
									value=((buffer->signals).float_values)+
										(((*device)->signal->index)+average_start*buffer_offset_1);
									value_end=0;
									for (i=average_end-average_start;i>0;i--)
									{
										value_end += *value;
										value += buffer_offset_1;
									}
									value_end /= (float)(average_end-average_start);
									value=((buffer->signals).float_values)+
										(((*device)->signal->index)+beat_start*buffer_offset_1);
									beat_width=(float)(beat_end-beat_start);
									for (i=beat_end-beat_start;i>=0;i--)
									{
										*processed_value=(*value)-((beat_width-(float)i)*value_end+
											(float)i*value_start)/beat_width;
										value += buffer_offset_1;
										processed_value += buffer_offset_1;
									}
								}
							}
							device++;
						}
						end -= start;
						analysis->datum -= start;
						if ((analysis->datum<0)||(end<analysis->datum))
						{
							analysis->datum=end/3;
						}
						analysis->potential_time -= start;
						if ((analysis->potential_time<0)||(end<analysis->potential_time))
						{
							analysis->potential_time=(2*end)/3;
						}
						(trace->area_3).edit.first_data -= start;
						(trace->area_3).edit.last_data -= start;
						if (divisions)
						{
							for (i=number_of_beats-2;i>=0;i--)
							{
								divisions[i] -= start;
							}
						}
						start=0;
						*(trace->event_detection.start_search_interval)=0;
						*(trace->event_detection.end_search_interval)=end;
						if (divisions)
						{
							for (i=number_of_beats-2;i>=0;i--)
							{
								divisions[i] -= start;
							}
						}
						buffer->number_of_samples=end+1;
						buffer->start=0;
						buffer->end=end;
					}
					if (True==XmToggleButtonGadgetGetState((trace->area_3).
						beat_averaging.beat_averaging_toggle))
					{
						device=rig->devices;
						start= *(trace->event_detection.start_search_interval);
						divisions= *(trace->event_detection.search_interval_divisions);
						end= *(trace->event_detection.end_search_interval);
						number_of_beats= *(trace->event_detection.number_of_events);
						beat_start=start;
						if (divisions)
						{
							beat_end=divisions[0];
							max_times=beat_end-beat_start;
							if (max_times<end-divisions[number_of_beats-2]+1)
							{
								max_times=end-divisions[number_of_beats-2]+1;
							}
							for (i=1;i<number_of_beats-1;i++)
							{
								if (max_times<divisions[i]-divisions[i-1])
								{
									max_times=divisions[i]-divisions[i-1];
								}
							}
						}
						else
						{
							beat_end=beat_start+(end-start)/number_of_beats;
							max_times=(end-start)/number_of_beats+1;
						}
						if (ALLOCATE(beat_counts,int,max_times))
						{
							if ((0<start)||(0<(buffer->times)[start]))
							{
								processed_time=buffer->times;
								time=processed_time+start;
								/* averaged beat starts from time 0 */
								start_time= *time;
								for (i=max_times;i>0;i--)
								{
									*processed_time=(*time)-start_time;
									processed_time++;
									time++;
								}
							}
							/* run through all the signals */
							for (j=rig->number_of_devices;j>0;j--)
							{
								if ((*device)->signal)
								{
									event=(*device)->signal->first_event;
									beat_count=beat_counts;
									for (i=max_times;i>0;i--)
									{
										*beat_count=0;
										beat_count++;
									}
									beat_end=start-1;
									for (beat_number=1;beat_number<=number_of_beats;beat_number++)
									{
										beat_start=beat_end+1;
										if (beat_number<number_of_beats)
										{
											if (divisions)
											{
												beat_end=divisions[beat_number-1]-1;
											}
											else
											{
												beat_end=start+(int)((float)((end-start)*beat_number)/
													(float)number_of_beats+0.5)-1;
											}
										}
										else
										{
											beat_end=end;
										}
										/* check if the beat has been rejected by checking the
											status of the first event in the beat interval */
										while (event&&(event->time<beat_start))
										{
											event=event->next;
										}
										if (!event||(event->time>beat_end)||
											(REJECTED!=event->status))
										{
											processed_value=(buffer->signals).float_values+
												((*device)->signal->index);
											value=processed_value+(beat_start*buffer_offset_1);
											beat_count=beat_counts;
											for (i=beat_end-beat_start;i>0;i--)
											{
												if (0< *beat_count)
												{
													*processed_value += *value;
												}
												else
												{
													*processed_value= *value;
												}
												(*beat_count)++;
												beat_count++;
												processed_value += buffer_offset_1;
												value += buffer_offset_1;
											}
										}
									}
									processed_value=(buffer->signals).float_values+
										((*device)->signal->index);
									beat_count=beat_counts;
									for (i=max_times;i>0;i--)
									{
										if (0< *beat_count)
										{
											*processed_value /= (float)(*beat_count);
										}
										else
										{
											*processed_value=(float)0;
										}
										processed_value += buffer_offset_1;
										beat_count++;
									}
								}
								device++;
							}
							DEALLOCATE(beat_counts);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"trace_analysis_mode_apply.  Could not allocate beat_counts");
						}
						buffer->start=0;
						buffer->end=max_times-1;
						buffer->number_of_samples=max_times;
						if ((analysis->datum<0)||(buffer->end<analysis->datum))
						{
							analysis->datum=(buffer->end)/3;
						}
						analysis->potential_time -= start;
						if ((analysis->potential_time<0)||
							(buffer->end<analysis->potential_time))
						{
							analysis->potential_time=(2*(buffer->end))/3;
						}
						(trace->area_3).edit.first_data=buffer->start;
						(trace->area_3).edit.last_data=buffer->end;
						analysis->start_search_interval=buffer->start;
						analysis->end_search_interval=buffer->end;
						DEALLOCATE(analysis->search_interval_divisions);
						if (1!=analysis->number_of_events)
						{
							analysis->number_of_events=1;
							/*???DB.  Should this \/ be here ? */
							XtUnmanageChild(
								trace->area_1.enlarge.number_of_events.down_arrow);
							XtUnmanageChild(
								trace->area_1.beat_averaging.number_of_beats.down_arrow);
							XtManageChild(trace->area_1.enlarge.number_of_events.up_arrow);
							XtManageChild(
								trace->area_1.beat_averaging.number_of_beats.up_arrow);
							xm_string=XmStringCreate("1",XmSTRING_DEFAULT_CHARSET);
							XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
								XmNlabelString,xm_string,
								NULL);
							XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
								XmNlabelString,xm_string,
								NULL);
							XmStringFree(xm_string);
							analysis->event_number=1;
						}
					}
					/* update the display */
					update_signals_drawing_area(analysis->window);
					update_interval_drawing_area(analysis->window);
					trace_change_signal(analysis->trace);
					/* update the mapping window */
					if ((NO_MAP_FIELD==analysis->map_type)&&
						(mapping=analysis->mapping_window)&&(map=mapping->map))
					{
						analysis->map_type=NO_MAP_FIELD;
						map->colour_option=HIDE_COLOUR;
						map->contours_option=HIDE_CONTOURS;
						map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
						/* clear the colour map */
						map->activation_front= -1;
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						XtSetSensitive(mapping->animate_button,False);
					}
				}
			} break;
			case FILTERING:
			{
				/* create processed rig */
				if (analysis->raw_rig)
				{
					/* reset the signal range to automatic */
					device=rig->devices;
					for (i=rig->number_of_devices;i>0;i--)
					{
						(*device)->signal_display_maximum=0;
						(*device)->signal_display_minimum=1;
						device++;
					}
				}
				else
				{
					if (rig=create_processed_rig(analysis->rig))
					{
						analysis->raw_rig=analysis->rig;
						analysis->rig=rig;
						/* reset the highlighted device */
						if (highlight= *(analysis->highlight))
						{
							highlight->highlight=0;
							device=analysis->raw_rig->devices;
							i=analysis->raw_rig->number_of_devices;
							while ((i>0)&&(*device!=highlight))
							{
								i--;
								device++;
							}
							if (i>0)
							{
								analysis->highlight=(rig->devices)+
									(device-(analysis->raw_rig->devices));
								(*(analysis->highlight))->highlight=1;
							}
							else
							{
								*(analysis->highlight)=(struct Device *)NULL;
							}
						}
						/* unghost the reset button */
						XtSetSensitive(analysis->window->interval.reset_button,True);
					}
				}
				/* check that rig and signals exist */
				if (rig&&(device=rig->devices)&&(*device)&&
					(buffer=get_Device_signal_buffer(*device)))
				{
					buffer_offset_1=buffer->number_of_signals;
					buffer_offset_2=
						trace->processed_device->signal->buffer->number_of_signals;
					transform_number_of_samples=(buffer->end)-(buffer->start)+1;
					transform_buffer=trace->real_device_1->signal->buffer;
					transform_buffer_offset=transform_buffer->number_of_signals;
					/* run through all the signals */
					for (i=rig->number_of_devices;i>0;i--)
					{
						if (((*device)->channel)&&((*device)->signal))
						{
							if (fourier_transform(SQUARE_WINDOW,*device,(struct Device *)NULL,
								trace->real_device_1,trace->imaginary_device_1))
							{
								/* perform filtering */
								real_value=((transform_buffer->signals).float_values)+
									(trace->real_device_1->signal->index);
								imaginary_value=((transform_buffer->signals).float_values)+
									(trace->imaginary_device_1->signal->index);
								/* remove DC */
								*real_value=0.;
								*imaginary_value=0.;
								if ((trace->filtering).notch_on)
								{
									notch=(int)floor(((trace->filtering).notch_frequency)*
										(transform_buffer->frequency));
									if ((0<=notch)&&
										(notch<(transform_buffer->number_of_samples)-1))
									{
										real_value[notch*transform_buffer_offset]=0;
										imaginary_value[notch*transform_buffer_offset]=0;
										real_value[(notch+1)*transform_buffer_offset]=0;
										imaginary_value[(notch+1)*transform_buffer_offset]=0;
									}
								}
								low_pass=(int)(((trace->filtering).low_pass_frequency)*
									(transform_buffer->frequency)+0.5);
								if (low_pass<0)
								{
									low_pass=0;
								}
								else
								{
									if (low_pass>transform_buffer->number_of_samples)
									{
										low_pass=transform_buffer->number_of_samples;
									}
								}
								high_pass=(int)(((trace->filtering).high_pass_frequency)*
									(transform_buffer->frequency)+0.5);
								if (high_pass<0)
								{
									high_pass=0;
								}
								else
								{
									if (high_pass>transform_buffer->number_of_samples)
									{
										high_pass=transform_buffer->number_of_samples;
									}
								}
								if (high_pass<low_pass)
								{
									for (j=high_pass;j>0;j--)
									{
										*real_value=0.;
										real_value += transform_buffer_offset;
										*imaginary_value=0.;
										imaginary_value += transform_buffer_offset;
									}
									real_value += (low_pass-high_pass)*transform_buffer_offset;
									imaginary_value +=
										(low_pass-high_pass)*transform_buffer_offset;
									for (j=(transform_buffer->number_of_samples)-low_pass-1;j>0;
										j--)
									{
										*real_value=0.;
										real_value += transform_buffer_offset;
										*imaginary_value=0.;
										imaginary_value += transform_buffer_offset;
									}
								}
								else
								{
									real_value += low_pass*transform_buffer_offset;
									imaginary_value += low_pass*transform_buffer_offset;
									for (j=high_pass-low_pass-2;j>0;j--)
									{
										*real_value=0.;
										real_value += transform_buffer_offset;
										*imaginary_value=0.;
										imaginary_value += transform_buffer_offset;
									}
								}
								/* transform back */
								if (inverse_fourier_transform(trace->real_device_1,
									trace->imaginary_device_1,trace->processed_device,
									(struct Device *)NULL))
								{
									(*device)->channel->gain=
										trace->processed_device->channel->gain;
									(*device)->channel->offset=
										trace->processed_device->channel->offset;
									/* the number of samples for the processed device will be the
										smallest power of 2 >= transform_number_of_samples */
									value_1=((buffer->signals).float_values)+
										((*device)->signal->index);
									value_2=((trace->processed_device->signal->buffer->signals).
										float_values)+(trace->processed_device->signal->index);
									for (j=transform_number_of_samples;j>0;j--)
									{
										*value_1= *value_2;
										value_1 += buffer_offset_1;
										value_2 += buffer_offset_2;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
	"trace_analysis_mode_apply.  Could not calculate inverse Fourier transform");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
					"trace_analysis_mode_apply.  Could not calculate Fourier transform");
							}
						}
						device++;
					}
					if (0<buffer->start)
					{
						time_offset=buffer->times;
						time=time_offset+(buffer->start);
						for (i=transform_number_of_samples;i>0;i--)
						{
							*time_offset= *time;
							time_offset++;
							time++;
						}
					}
					buffer->start=0;
					buffer->end=transform_number_of_samples-1;
					buffer->number_of_samples=transform_number_of_samples;
					analysis->datum=(buffer->end)/3;
					analysis->potential_time=(2*(buffer->end))/3;
					(trace->area_3).edit.first_data=buffer->start;
					(trace->area_3).edit.last_data=buffer->end;
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					if (1!=analysis->number_of_events)
					{
						analysis->number_of_events=1;
						/*???DB.  Should this \/ be here ? */
						XtUnmanageChild(
							trace->area_1.enlarge.number_of_events.down_arrow);
						XtUnmanageChild(
							trace->area_1.beat_averaging.number_of_beats.down_arrow);
						XtManageChild(trace->area_1.enlarge.number_of_events.up_arrow);
						XtManageChild(
							trace->area_1.beat_averaging.number_of_beats.up_arrow);
						xm_string=XmStringCreate("1",XmSTRING_DEFAULT_CHARSET);
						XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
							XmNlabelString,xm_string,
							NULL);
						XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
							XmNlabelString,xm_string,
							NULL);
						XmStringFree(xm_string);
						analysis->event_number=1;
					}
					/* update the display */
					update_signals_drawing_area(analysis->window);
					update_interval_drawing_area(analysis->window);
					trace_change_signal(analysis->trace);
					/* update the mapping window */
					if ((NO_MAP_FIELD==analysis->map_type)&&
						(mapping=analysis->mapping_window)&&(map=mapping->map))
					{
						analysis->map_type=NO_MAP_FIELD;
						map->colour_option=HIDE_COLOUR;
						map->contours_option=HIDE_CONTOURS;
						map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
						/* clear the colour map */
						map->activation_front= -1;
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						XtSetSensitive(mapping->animate_button,False);
					}
				}
			} break;
			case POWER_SPECTRA:
			{
				/* create processed rig */
				if (analysis->raw_rig)
				{
					/* reset the signal range to automatic */
					device=rig->devices;
					for (i=rig->number_of_devices;i>0;i--)
					{
						(*device)->signal_display_maximum=0;
						(*device)->signal_display_minimum=1;
						device++;
					}
				}
				else
				{
					if (rig=create_processed_rig(analysis->rig))
					{
						analysis->raw_rig=analysis->rig;
						analysis->rig=rig;
						/* reset the highlighted device */
						if (highlight= *(analysis->highlight))
						{
							highlight->highlight=0;
							device=analysis->raw_rig->devices;
							i=analysis->raw_rig->number_of_devices;
							while ((i>0)&&(*device!=highlight))
							{
								i--;
								device++;
							}
							if (i>0)
							{
								analysis->highlight=(rig->devices)+
									(device-(analysis->raw_rig->devices));
								(*(analysis->highlight))->highlight=1;
							}
							else
							{
								*(analysis->highlight)=(struct Device *)NULL;
							}
						}
						/* unghost the reset button */
						XtSetSensitive(analysis->window->interval.reset_button,True);
					}
				}
				/* check that rig and signals exist */
				if (rig&&(device=rig->devices)&&(*device)&&
					(buffer=get_Device_signal_buffer(*device))&&
					(0<(number_of_samples=buffer->number_of_samples)))
				{
					buffer_offset_1=buffer->number_of_signals;
					transform_buffer=trace->real_device_1->signal->buffer;
					transform_buffer_offset=transform_buffer->number_of_signals;
					transform_number_of_samples=transform_buffer->number_of_samples;
					/* set frequency and times */
					buffer->frequency=transform_buffer->frequency;
					time=buffer->times;
					for (i=0;i<transform_number_of_samples;i++)
					{
						*time=i;
						time++;
					}
					/* run through all the signals */
					for (i=rig->number_of_devices;i>0;i--)
					{
						if ((*device)->signal)
						{
							if (fourier_transform(SQUARE_WINDOW,*device,(struct Device *)NULL,
								trace->real_device_1,trace->imaginary_device_1))
							{
								/* calculate power spectrum (square of magnitude of FT, which is
									the FT of the auto-correlation) */
								real_value=((transform_buffer->signals).float_values)+
									(trace->real_device_1->signal->index);
								imaginary_value=((transform_buffer->signals).float_values)+
									(trace->imaginary_device_1->signal->index);
								value_1=((buffer->signals).float_values)+
									((*device)->signal->index);
								/* remove DC */
								*value_1=0.;
								/* the number of samples for the processed device will be the
									largest power of 2 <= (buffer->end)-(buffer->start)+1 */
								for (j=transform_number_of_samples-1;j>0;j--)
								{
									value_1 += buffer_offset_1;
									real_value += transform_buffer_offset;
									imaginary_value += transform_buffer_offset;
									x= *real_value;
									y= *imaginary_value;
									*value_1=x*x+y*y;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
					"trace_analysis_mode_apply.  Could not calculate Fourier transform");
							}
						}
						device++;
					}
					buffer->start=0;
					buffer->end=transform_number_of_samples-1;
					buffer->number_of_samples=transform_number_of_samples;
					(analysis->trace->power_spectra).maximum_frequency= -1;
					(analysis->trace->power_spectra).minimum_frequency= -1;
					analysis->datum=(buffer->end)/3;
					analysis->potential_time=(2*(buffer->end))/3;
					(trace->area_3).edit.first_data=buffer->start;
					(trace->area_3).edit.last_data=buffer->end;
					analysis->start_search_interval=buffer->start;
					analysis->end_search_interval=buffer->end;
					DEALLOCATE(analysis->search_interval_divisions);
					if (1!=analysis->number_of_events)
					{
						analysis->number_of_events=1;
						/*???DB.  Should this \/ be here ? */
						XtUnmanageChild(
							trace->area_1.enlarge.number_of_events.down_arrow);
						XtUnmanageChild(
							trace->area_1.beat_averaging.number_of_beats.down_arrow);
						XtManageChild(trace->area_1.enlarge.number_of_events.up_arrow);
						XtManageChild(
							trace->area_1.beat_averaging.number_of_beats.up_arrow);
						xm_string=XmStringCreate("1",XmSTRING_DEFAULT_CHARSET);
						XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
							XmNlabelString,xm_string,
							NULL);
						XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
							XmNlabelString,xm_string,
							NULL);
						XmStringFree(xm_string);
						analysis->event_number=1;
					}
					/* update the display */
					update_signals_drawing_area(analysis->window);
					update_interval_drawing_area(analysis->window);
					trace_change_signal(analysis->trace);
					/* update the mapping window */
					if ((NO_MAP_FIELD==analysis->map_type)&&
						(mapping=analysis->mapping_window)&&(map=mapping->map))
					{
						analysis->map_type=NO_MAP_FIELD;
						map->colour_option=HIDE_COLOUR;
						map->contours_option=HIDE_CONTOURS;
						map->electrodes_label_type=SHOW_ELECTRODE_NAMES;
						/* clear the colour map */
						map->activation_front= -1;
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
						XtSetSensitive(mapping->animate_button,False);
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"trace_analysis_mode_apply.  Invalid analysis mode");
			} break;
		}
		busy_cursor_off((Widget)NULL,analysis->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"trace_analysis_mode_apply.  Invalid analysis_work_area");
	}
	LEAVE;
} /* trace_analysis_mode_apply */

#if defined (OLD_CODE)
static void signal_hot_key_handler(Widget widget,
	XtPointer analysis_work_area_structure,XEvent *event,
	Boolean *continue_to_dispatch)
/*******************************************************************************
LAST MODIFIED : 24 December 1996

DESCRIPTION :
The hot key handler for the <widget>.
==============================================================================*/
{
	Boolean sensitive;
	Display *display;
	static KeyCode a_key_code=0,n_key_code,p_key_code,r_key_code;
	struct Analysis_window *analysis_window;
	struct User_interface *user_interface;
	XKeyEvent *key_press;

	if ((key_press=(XKeyEvent *)event)&&analysis_work_area_structure&&
		(analysis_window=
		((struct Analysis_work_area *)analysis_work_area_structure)->window)&&
		(user_interface=
		((struct Analysis_work_area *)analysis_work_area_structure)->
		user_interface))
	{
		if (!((key_press->state)&(~(ShiftMask|LockMask))))
		{
			display=User_interface_get_display(user_interface);
			if (0==a_key_code)
			{
				a_key_code=(unsigned int)XKeysymToKeycode(display,97);
				n_key_code=(unsigned int)XKeysymToKeycode(display,110);
				p_key_code=(unsigned int)XKeysymToKeycode(display,112);
				r_key_code=(unsigned int)XKeysymToKeycode(display,114);
			}
			if (a_key_code==key_press->keycode)
			{
				XtVaGetValues((analysis_window->interval).accept_button,
					XmNsensitive,&sensitive,
					NULL);
				if (True==sensitive)
				{
					analysis_accept_signal((analysis_window->interval).accept_button,
						analysis_work_area_structure,(XtPointer)NULL);
				}
			}
			else
			{
				if (n_key_code==key_press->keycode)
				{
					XtVaGetValues((analysis_window->interval).next_button,
						XmNsensitive,&sensitive,
						NULL);
					if (True==sensitive)
					{
						analysis_next_signal((analysis_window->interval).next_button,
							analysis_work_area_structure,(XtPointer)NULL);
					}
				}
				else
				{
					if (p_key_code==key_press->keycode)
					{
						XtVaGetValues((analysis_window->interval).previous_button,
							XmNsensitive,&sensitive,
							NULL);
						if (True==sensitive)
						{
							analysis_previous_signal(
								(analysis_window->interval).previous_button,
								analysis_work_area_structure,(XtPointer)NULL);
						}
					}
					else
					{
						if (r_key_code==key_press->keycode)
						{
							XtVaGetValues((analysis_window->interval).reject_button,
								XmNsensitive,&sensitive,
								NULL);
							if (True==sensitive)
							{
								analysis_reject_signal(
									(analysis_window->interval).reject_button,
									analysis_work_area_structure,(XtPointer)NULL);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"signal_hot_key_handler.  Missing event or analysis window");
	}
} /* signal_hot_key_handler */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void add_signal_hot_key_handler(Widget *widget_id,
	XtPointer analysis_work_area_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 April 1996

DESCRIPTION :
Adds the hot key handler to the widget.
==============================================================================*/
{
	ENTER(add_signal_hot_key_handler);
	if (analysis_work_area_structure)
	{
		XtAddEventHandler(*widget_id,KeyPressMask,False,signal_hot_key_handler,
			analysis_work_area_structure);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_signal_hot_key_handler.  Missing analysis_window_structure");
	}
	LEAVE;
} /* add_signal_hot_key_handler */
#endif /* defined (OLD_CODE) */

static double analysis_potential_time_next_time_callback(double time_after,
	enum Time_keeper_play_direction play_direction,void *analysis_void)
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
Calculates the next desired update callback from the time object.
==============================================================================*/
{
	double next_time;
	int number_of_spectrum_colours, time_set;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Map_drawing_information *drawing_information;
	struct Signal_buffer *buffer;

	ENTER(analysis_potential_time_next_time_callback);
	if ((analysis=(struct Analysis_work_area *)analysis_void)&&
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device)))
	{
		time_set=0;
		if ((mapping=analysis->mapping_window)&&(map=mapping->map)&&
			(drawing_information=map->drawing_information))
		{
			switch (analysis->map_type)
			{
				case POTENTIAL:
				{
					switch (map->interpolation_type)
					{
						case BICUBIC_INTERPOLATION:
						{
							if (map->end_time>map->start_time)
							{
								/* valid frames */
								switch (play_direction)
								{
									case TIME_KEEPER_PLAY_FORWARD:
									{
										next_time=((double)map->start_time+
											(double)(map->end_time-map->start_time)/
											(double)(map->number_of_frames-1)*
											floor((double)(map->number_of_frames-1)*
												((time_after*1000.0-(double)map->start_time)/
													(double)(map->end_time-map->start_time))+1.0))/1000.0;
										time_set=1;
									} break;
									case TIME_KEEPER_PLAY_BACKWARD:
									{
										next_time=((double)map->start_time+
											(double)(map->end_time - map->start_time)/
											(double)(map->number_of_frames-1)*
											ceil((double)(map->number_of_frames-1)*
												((time_after*1000.0-(double)map->start_time)/
													(double)(map->end_time-map->start_time))-1.0))/1000.0;
										time_set=1;
									} break;
								}
							}
						} break;
						case NO_INTERPOLATION:
						case DIRECT_INTERPOLATION:
						default:
						{
							/*do nothing */
							;
						} break;
					}/* switch (map->interpolation_type) */
				} break;
				case SINGLE_ACTIVATION:
				{
					number_of_spectrum_colours=
						drawing_information->number_of_spectrum_colours;
					/* valid frames */
					switch (play_direction)
					{
						case TIME_KEEPER_PLAY_FORWARD:
						{
							next_time=((float)buffer->times[analysis->datum]/
								buffer->frequency+map->minimum_value+
								(map->maximum_value-map->minimum_value)/
								(float)(number_of_spectrum_colours-1)*
								floor(1.0001+(float)(number_of_spectrum_colours-1)*
								(time_after*1000.0-(float)buffer->times[analysis->datum]/
								buffer->frequency-map->minimum_value)/
								(map->maximum_value-map->minimum_value)))/1000.0;
							time_set=1;
						} break;
						case TIME_KEEPER_PLAY_BACKWARD:
						{
							next_time=((float)buffer->times[analysis->datum]/
								buffer->frequency+map->minimum_value+
								(map->maximum_value-map->minimum_value)/
								(float)(number_of_spectrum_colours-1)*
								ceil(-1.0001+(float)(number_of_spectrum_colours-1)*
								(time_after*1000.0-(float)buffer->times[analysis->datum]/
								buffer->frequency-map->minimum_value)/
								(map->maximum_value-map->minimum_value)))/1000.0;
							time_set=1;
						} break;
					}
				} break;
			}
		}
		if (!time_set)
		{
			switch (play_direction)
			{
				case TIME_KEEPER_PLAY_FORWARD:
				{
					next_time=(1.0+floor(time_after*buffer->frequency))/
						buffer->frequency;
				} break;
				case TIME_KEEPER_PLAY_BACKWARD:
				{
					next_time=(-1.0+ceil(time_after*buffer->frequency))/
						buffer->frequency;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"analysis_potential_time_next_time_callback.  "
						"Unknown play direction");
					next_time=0;
				} break;
			}
		}
#if defined (DEBUG)
		/*???debug */
		printf("analysis_potential_time_next_time_callback.  "
			"time_after=%g, play_direction=%d, next_time=%g\n",time_after,
			play_direction,next_time);
#endif /* defined (DEBUG) */
	}
	LEAVE;

	return (next_time);
} /* analysis_potential_time_next_time_callback */

static int analysis_potential_time_update_callback(
	struct Time_object *time_object,double current_time,void *analysis_void)
/*******************************************************************************
LAST MODIFIED : 30 October 2001

DESCRIPTION :
Responds to update callbacks from the time object.
???DB.  Assume current_time is in milli-seconds
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	float contour_maximum,contour_minimum,frequency,maximum_value,minimum_value,
		map_potential_time,number_of_spectrum_colours;
	int cell_number,datum,frame_number,i,number_of_contours,potential_time,
		previous_potential_time,return_code;
	Pixel *spectrum_pixels;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Map *map;
	struct Map_drawing_information *drawing_information;
	struct Mapping_window *mapping;
	struct Signal_buffer *buffer;
	XColor colour, spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER(analysis_potential_time_update_callback);
	USE_PARAMETER(time_object);
	return_code=0;
	if ((analysis=(struct Analysis_work_area *)analysis_void)&&
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device)))
	{
		frequency=buffer->frequency;
		previous_potential_time=analysis->potential_time;
		potential_time=(int)(current_time*frequency)-(buffer->times)[0];
		if (potential_time<0)
		{
			potential_time=0;
		}
		if (potential_time>=buffer->number_of_samples)
		{
			potential_time=buffer->number_of_samples-1;
		}
#if defined (DEBUG)
		/*???debug */
		printf("analysis_potential_time_update_callback.  %g %g\n",current_time,
			frequency);
		printf("  initial potential_time %d (%g)\n",
			potential_time,(buffer->times[potential_time])/frequency);
#endif /* defined (DEBUG) */
		if ((float)(buffer->times)[potential_time]<current_time*frequency-0.5)
		{
			while ((potential_time<buffer->number_of_samples)&&
				((float)(buffer->times)[potential_time]<current_time*frequency-0.5))
			{
				potential_time++;
			}
		}
		else
		{
			if ((float)(buffer->times)[potential_time]>current_time*frequency+0.5)
			{
				while ((potential_time>=0)&&
					((float)(buffer->times)[potential_time]>current_time*frequency+0.5))
				{
					potential_time--;
				}
			}
		}
#if defined (DEBUG)
		/*???debug */
		printf("  final potential_time %d (%g)\n",
			potential_time,(buffer->times[potential_time])/frequency);
#endif /* defined (DEBUG) */
		if (potential_time<0)
		{
			display_message(ERROR_MESSAGE,"analysis_potential_time_update_callback.  "
				"Potential time less than minimum");
		}
		else
		{
			if (potential_time>=buffer->number_of_samples)
			{
				display_message(ERROR_MESSAGE,
					"analysis_potential_time_update_callback.  "
					"Potential time greater than maximum");
			}
			else
			{
				analysis->potential_time=potential_time;
				analysis_window_update_interval_area_time(analysis->window,
					potential_time,previous_potential_time,
					&analysis->analysis_update_flags);
				analysis_window_update_signal_area_time(analysis->window,
					potential_time,previous_potential_time);
				trace_update_potential_time(analysis->trace,
					potential_time,previous_potential_time,&analysis->trace_update_flags);
				if ((mapping=analysis->mapping_window)&&(map=mapping->map)&&
					(drawing_information=map->drawing_information))
				{
					/* if we're in ELECTRICAL_IMAGING mode and have events, do nothing, as
						in this mode we make maps from the eimaging_events, not the
						map_potential_time */
					if (!((map->first_eimaging_event&&*map->first_eimaging_event)&&
						(ELECTRICAL_IMAGING==*map->analysis_mode)))
					{
						switch (analysis->map_type)
						{
							case POTENTIAL:
							{
								map_potential_time=current_time*1000.0;
								switch (map->interpolation_type)
								{
									case BICUBIC_INTERPOLATION:
									{
										if (-1!=map->activation_front)
										{
											/* playing movie */
											/* ??JW fix the range when playing the movie? IF so need
												to update map dialog as well as map->fixed_range=1 */
#if defined (UNEMAP_USE_3D)
											/* 3d map */
											if (map->projection_type==THREED_PROJECTION)
											{
												/* recalculate not used for 3d maps */
												update_mapping_drawing_area(mapping,1/*recalculate*/);
												update_mapping_colour_or_auxili(mapping);
											}
											else
											{
												/*2d map */
#endif /* defined (UNEMAP_USE_3D) */
												/* 2d map */
												if (map->end_time>map->start_time)
												{
													frame_number=(int)((float)(map->number_of_frames-1)*
														((map_potential_time-map->start_time)/
															(map->end_time-map->start_time)));
													map->sub_map_number=frame_number;
													update_mapping_drawing_area(mapping,0);
													update_mapping_colour_or_auxili(mapping);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"analysis_potential_time_update_callback.  "
														"End time greater or equal to start time");
													return_code=0;
												}
#if defined (UNEMAP_USE_3D)
											}/* if (map->projection_type==THREED_PROJECTION) */
#endif /* defined (UNEMAP_USE_3D) */
										}/* if (-1!=map->activation_front) */
										else
										{
											if ((map_potential_time>=map->start_time)&&
												(map_potential_time<=map->end_time))
											{
												if (map->start_time<map->end_time)
												{
													frame_number=(int)((float)(map->number_of_frames-1)*
														((map_potential_time-map->start_time)/
															(map->end_time-map->start_time)));
												}
												else
												{
													frame_number=0;
												}
												map->sub_map_number=frame_number;
												update_mapping_drawing_area(mapping,0);
												update_mapping_colour_or_auxili(mapping);
											}/* if ((map_potential_time>=map->start_time)&&*/
											else
											{
#if defined (UNEMAP_USE_3D)
												/* 3d map */
												if (map->projection_type==THREED_PROJECTION)
												{
													/* 3d map */
													/* recalculate not used for 3d maps */
													update_mapping_drawing_area(mapping,1/*recalculate*/);
													update_mapping_colour_or_auxili(mapping);
													map->start_time=map_potential_time;
													map->end_time=map->start_time;
												}
												else
												{
#endif /* defined (UNEMAP_USE_3D) */
													/* 2d map */
													update_map_from_manual_time_update(mapping);
													map->start_time=map_potential_time;
													map->end_time=map->start_time;
#if defined (UNEMAP_USE_3D)
												}/* if (map->projection_type==THREED_PROJECTION)*/
#endif /* defined (UNEMAP_USE_3D) */
											}/* if ((map_potential_time>=map->start_time)&&*/
										}/* if (-1!=map->activation_front) */
									} break;
									case DIRECT_INTERPOLATION:
									{
										map->start_time=map_potential_time;
										map->end_time=map->start_time;
										/*???DB.  Where I'm up to */
										/*???DB.  -1!= means playing movie? */
										/*???DB.  Check out update_map_from_manual_time_update */
										if (map->activation_front==0)
										{
											update_mapping_drawing_area(mapping,2/*recalculate*/);
											update_mapping_colour_or_auxili(mapping);
										}
										else
										{
											update_map_from_manual_time_update(mapping);
										}
									} break;
									case NO_INTERPOLATION:
									default:
									{
										map->start_time=map_potential_time;
										map->end_time=map->start_time;
										update_mapping_drawing_area(mapping,1);
										update_mapping_colour_or_auxili(mapping);
									} break;
								}/*map->interpolation_type*/
							} break;
							case SINGLE_ACTIVATION:
							{
								number_of_spectrum_colours=
									drawing_information->number_of_spectrum_colours;
								map->activation_front=(float)(number_of_spectrum_colours-1)*
									(current_time*1000.0-(float)buffer->times[analysis->datum]*
									1000.0/frequency-map->minimum_value)/(map->maximum_value-
									map->minimum_value);
#if defined (DEBUG)
								printf("analysis_potential_time_update_callback.  "
									"front %d current %f  datum %f  minimum %f maximum %f\n",
									map->activation_front,current_time,
									(float)buffer->times[analysis->datum]/frequency,
									map->minimum_value,map->maximum_value);
#endif /* defined (DEBUG) */
								if ((0<=map->activation_front)&&
									(map->activation_front<number_of_spectrum_colours))
								{
									if (drawing_information->read_only_colour_map)
									{
										update_mapping_drawing_area(mapping,0);
										update_mapping_colour_or_auxili(mapping);
									}
									else
									{
										display=User_interface_get_display(
											drawing_information->user_interface);
										colour_map=drawing_information->colour_map;
										spectrum_pixels=drawing_information->spectrum_colours;
										/* use background drawing colour for the whole spectrum */
										colour.pixel=drawing_information->background_drawing_colour;
										XQueryColor(display,colour_map,&colour);
										for (i=0;i<number_of_spectrum_colours;i++)
										{
											spectrum_rgb[i].pixel=spectrum_pixels[i];
											spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
											spectrum_rgb[i].red=colour.red;
											spectrum_rgb[i].blue=colour.blue;
											spectrum_rgb[i].green=colour.green;
										}
										if ((SHOW_CONTOURS==map->contours_option)&&
											(VARIABLE_THICKNESS==map->contour_thickness))
										{
											colour.pixel=drawing_information->contour_colour;
											XQueryColor(display,colour_map,&colour);
											number_of_contours=map->number_of_contours;
											maximum_value=map->maximum_value;
											minimum_value=map->minimum_value;
											contour_maximum=map->contour_maximum;
											contour_minimum=map->contour_minimum;
											number_of_contours=map->number_of_contours;
											for (i=0;i<number_of_contours;i++)
											{
												cell_number=(int)(((contour_maximum*(float)i+
													contour_minimum*(float)(number_of_contours-1-i))/
													(float)(number_of_contours-1)-minimum_value)/
													(maximum_value-minimum_value)*
													(float)(number_of_spectrum_colours-1)+0.5);
												spectrum_rgb[cell_number].pixel=
													spectrum_pixels[cell_number];
												spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
												spectrum_rgb[cell_number].red=colour.red;
												spectrum_rgb[cell_number].blue=colour.blue;
												spectrum_rgb[cell_number].green=colour.green;
											}
										}
										/* show the activation front */
										colour.pixel=drawing_information->contour_colour;
										XQueryColor(display,colour_map,&colour);
										i=map->activation_front;
										spectrum_rgb[i].pixel=spectrum_pixels[i];
										spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
										spectrum_rgb[i].red=colour.red;
										spectrum_rgb[i].blue=colour.blue;
										spectrum_rgb[i].green=colour.green;
										XStoreColors(display,colour_map,spectrum_rgb,
											number_of_spectrum_colours);
										/* show the map boundary */
										if (drawing_information->boundary_colour)
										{
											colour.pixel=drawing_information->boundary_colour;
											colour.flags=DoRed|DoGreen|DoBlue;
											XStoreColor(display,colour_map,&colour);
										}
									}
								}
								else
								{
									if (Time_keeper_is_playing(Time_object_get_time_keeper(
										analysis->potential_time_object)))
									{
										display_message(ERROR_MESSAGE,
											"analysis_potential_time_update_callback.  "
											"Time outside range of single activation");
									}
									else
									{
										map->activation_front= -1;
										update_mapping_drawing_area(mapping,0);
										update_mapping_colour_or_auxili(mapping);
									}
								}
							} break;
							case MULTIPLE_ACTIVATION:
							{
								datum=analysis->datum;
								analysis->datum=potential_time;
#if defined (DEBUG)
								printf("MULTIPLE_ACTIVATION.  datum %d (%d)\n",analysis->datum,
									analysis->start_search_interval);
#endif /* defined (DEBUG) */
								update_mapping_drawing_area(mapping,2);
								update_mapping_colour_or_auxili(mapping);
								analysis->datum=datum;
							} break;
						} /*switch (analysis->map_type)*/
					}/* if (!((eimaging_event=*map->first_eimaging_event)&& */
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"analysis_potential_time_update_callback.  "
			"Missing analysis_window_structure");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_potential_time_update_callback */

static int analysis_time_keeper_callback(struct Time_keeper *time_keeper,
	enum Time_keeper_event event, void *analysis_void)
/*******************************************************************************
LAST MODIFIED : 6 December 2001

DESCRIPTION : time keeper callback for analysis
==============================================================================*/
{
	int return_code;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Mapping_window *mapping;

	ENTER(analysis_time_keeper_callback);
	return_code=0;
	if ((analysis=(struct Analysis_work_area *)analysis_void)&&
		(mapping=analysis->mapping_window)&&(map=mapping->map)&&time_keeper)
	{
		switch (event)
		{
			case TIME_KEEPER_STARTED:
			{
				/* only one animation at a time */
				map->activation_front=0;
			} break;
			case TIME_KEEPER_STOPPED:
			{
				/*update the time again*/
				analysis_potential_time_update_callback(analysis->potential_time_object,
					Time_object_get_current_time(analysis->potential_time_object),
					analysis_void);
				/*change to no animation*/
				map->activation_front= -1;
#if defined (OLD_CODE)
				map->activation_front= -1;
				analysis_potential_time_update_callback(analysis->potential_time_object,
					Time_object_get_current_time(analysis->potential_time_object),
					analysis_void);
#endif
			} break;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_time_keeper_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_time_keeper_callback */

static int analysis_datum_time_update_callback(struct Time_object *time_object,
	double current_time, void *analysis_void)
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
Responds to update callbacks from the time object.
==============================================================================*/
{
	int datum,previous_datum,return_code;
	struct Analysis_work_area *analysis;
#if defined (NEW_CODE)
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
#endif /* defined (NEW_CODE) */

	ENTER(analysis_datum_time_update_callback);
	USE_PARAMETER(time_object);
	return_code=0;
	if ((analysis=(struct Analysis_work_area *)analysis_void)&&
		(analysis->highlight)
#if defined (NEW_CODE)
		&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device))
#endif /* defined (NEW_CODE) */
		)
	{
		datum=current_time;
		previous_datum=analysis->datum;
		analysis->datum=datum;
		analysis_window_update_signal_area_datum(analysis->window,datum,
			previous_datum);
		trace_update_datum(analysis->trace,datum,previous_datum,
			analysis->event_number,analysis->potential_time,
			&analysis->trace_update_flags);
#if defined (NEW_CODE)
		if ((POTENTIAL==analysis->map_type)&&(mapping=analysis->mapping_window)&&
			(map=mapping->map))
		{
			map_potential_time=(float)((buffer->times)[potential_time])*1000./
				(buffer->frequency);
			map->frame_start_time=map_potential_time;
			map->frame_end_time=map_potential_time;
			map->number_of_frames=1;
			map->frame_number=0;
			update_mapping_drawing_area(mapping,2);
			update_mapping_colour_or_auxili(mapping);
		}
#endif /* defined (NEW_CODE) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"analysis_datum_time_update_callback.  "
			"Missing analysis_window_structure");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_datum_time_update_callback */

#if defined (UNEMAP_USE_NODES)
static int iterative_set_highlight_field(struct FE_node *node,
	void *set_highlight_iterator_void)
/*******************************************************************************
LAST MODIFIED :

DESCRIPTION : iteratively set the highlight_field of the node
==============================================================================*/
{
	int return_code;
	struct Set_highlight_iterator *set_highlight_iterator;
	struct FE_field_component component;

	ENTER(iterative_set_highlight_field);
	return_code=1;
	if (node&&set_highlight_iterator_void)
	{
		set_highlight_iterator=(struct Set_highlight_iterator *)set_highlight_iterator_void;
		if (set_highlight_iterator)
		{
			if (FE_field_is_defined_at_node(set_highlight_iterator->highlight_field,node))
				/* nothing to do, but NOT an error if no signal at node*/
			{
				component.number=0;
				component.field=set_highlight_iterator->highlight_field;
				/*??JW should be copying out of and into node with MANAGER_MODIFY */
				set_FE_nodal_int_value(node,&component,/*version*/0,FE_NODAL_VALUE,/*time*/0,
					set_highlight_iterator->highlight);
				set_highlight_iterator->count++;
			}/* if (FE_field_is_defined_at_node*/
		}	/* if (set_highlight_iterator */
		else
		{
			display_message(ERROR_MESSAGE,
				"iterative_set_highlight_field.  set_highlight_iterator NULL ");
			return_code=0;
		}
	}/* if ((node)&&(set_highlight_iterator_void )) */
	else
	{
		display_message(ERROR_MESSAGE,
			"iterative_set_highlight_field. Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* iterative_set_highlight_field */
#endif /* defined (UNEMAP_USE_NODES) */

static int analysis_get_numbers_from_device(struct Analysis_work_area *analysis,
	struct Device **device,int *the_new_device_number,
	int *the_new_electrode_number,int *the_new_auxiliary_number)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION
Given  <device> returns the corresponding number  information in
<the_new_device_number>,<the_new_electrode_number> <the_new_auxiliary_number>.
Pass in (struct Device **) rather than (struct Device *), as do pointer
arithmetic on it.
*******************************************************************************/
{
	enum Device_type device_type;
	int i,new_auxiliary_number,new_device_number,new_electrode_number,return_code;
	struct Device **temp_device;
	struct Device_description *description;
	struct Region *current_region;

	ENTER(analysis_get_numbers_from_device);
	temp_device=(struct Device **)NULL;
	current_region=(struct Region *)NULL;
	description=(struct Device_description *)NULL;
	if (analysis&&(analysis->rig)&&(analysis->rig->devices)&&
		(*(analysis->rig->devices))&&the_new_device_number&&
		the_new_electrode_number&&the_new_auxiliary_number)
	{
		current_region=get_Rig_current_region(analysis->rig);
		/* have specified a device */
		/* determine the device, electrode and auxiliary numbers */
		temp_device=analysis->rig->devices;
		new_device_number=0;
		new_electrode_number=0;
		new_auxiliary_number=0;
		for (i=device-temp_device;i>0;i--)
		{
			if ((current_region==(description=(*temp_device)->description)->
				region)||(!current_region))
			{
				new_device_number++;
				if (ELECTRODE==(device_type=description->type))
				{
					new_electrode_number++;
				}
				else
				{
					if (AUXILIARY==device_type)
					{
						new_auxiliary_number++;
					}
				}
			}
			temp_device++;
		}
		if (ELECTRODE==(device_type=(*(temp_device))->description->type))
		{
			new_auxiliary_number= -1;
			return_code=1;
		}
		else
		{
			if (AUXILIARY==device_type)
			{
				new_electrode_number= -1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_get_numbers_from_device.  Invalid device type in rig");
				return_code=0;
			}
		}
		/*set things up for return */
		*the_new_device_number=new_device_number;
		*the_new_electrode_number=new_electrode_number;
		*the_new_auxiliary_number=new_auxiliary_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_get_numbers_from_device. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_get_numbers_from_device*/

static int analysis_get_device_and_numbers_from_number(
	struct Analysis_work_area *analysis,struct Device ***the_new_device,
	int *device_number,int *the_new_device_number,int *electrode_number,
	int *the_new_electrode_number,int *auxiliary_number,
	int *the_new_auxiliary_number)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Given one of <device_number>,<electrode_number>,<auxiliary_number>
returns the corresponding Device <the_new_device> and ALL the number
information in <the_new_device_number>,<the_new_electrode_number>
<the_new_auxiliary_number>.
Can't pass in and return info in same variables as want input variables to
remain unchanged.
*******************************************************************************/
{
	enum Device_type device_type;
	int i,new_auxiliary_number,new_device_number,new_electrode_number,return_code;
	struct Device_description *description;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Signals_area *signals;
	struct Device **new_device;

	ENTER(analysis_get_device_and_numbers_from_number);
	map=(struct Map *)NULL;
	mapping=(struct Mapping_window *)NULL;
	current_region=(struct Region *)NULL;
	signals=(struct Signals_area *)NULL;
	description=(struct Device_description *)NULL;
	if ((analysis)&&(analysis->rig)&&(analysis->rig->devices)&&
		(*(analysis->rig->devices))&&the_new_device&&
		(device_number||electrode_number||auxiliary_number))
	{
		current_region=get_Rig_current_region(analysis->rig);
		if (analysis->window)
		{
			signals= &(analysis->window->signals);
		}
		else
		{
			signals=(struct Signals_area *)NULL;
		}
		if ((analysis->mapping_window)&&(analysis->mapping_window->map))
		{
			mapping=analysis->mapping_window;
			map=mapping->map;
		}
		else
		{
			mapping=(struct Mapping_window *)NULL;
			map=(struct Map *)NULL;
		}
		if (signals&&device_number&&((new_device_number= *device_number)>=0)&&
			(new_device_number<signals->number_of_signals))
		{
			/* have specified a device_number */
			/* determine the device and the electrode and auxiliary numbers */
			new_device=analysis->rig->devices;
			new_electrode_number=0;
			new_auxiliary_number=0;
			i=new_device_number;
			while (((current_region!=(description=(*new_device)->description)->
				region)&&current_region)||(i>0))
			{
				if (!current_region||(current_region==description->region))
				{
					i--;
					if (ELECTRODE==(device_type=description->type))
					{
						new_electrode_number++;
					}
					else
					{
						if (AUXILIARY==device_type)
						{
							new_auxiliary_number++;
						}
					}
				}
				new_device++;
			}
			if (ELECTRODE==(device_type=description->type))
			{
				new_auxiliary_number= -1;
				return_code=1;
			}
			else
			{
				if (AUXILIARY==device_type)
				{
					new_electrode_number= -1;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"analysis_get_device_and_numbers_from_number.  Invalid device type in rig");
					return_code=0;
				}
			}
			if (return_code&&
				((electrode_number&&!(*electrode_number==new_electrode_number))||
					(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
			{
				display_message(ERROR_MESSAGE,"analysis_get_device_and_numbers_from_number.  %s",
					"device number incompatible with electrode and auxiliary numbers");
				return_code=0;
			}
		}
		else
		{
			if (mapping&&map )
			{
				if (electrode_number&&((new_electrode_number= *electrode_number)>=0)&&
					(new_electrode_number<map->number_of_electrodes))
				{
					/* have specified an electrode number */
					/* determine the device and the device and auxiliary numbers */
					new_device=analysis->rig->devices;
					new_device_number=0;
					new_auxiliary_number= -1;
					i=new_electrode_number;
					while ((ELECTRODE!=(device_type=(description=(*new_device)->
						description)->type))||(current_region&&(current_region!=
							description->region))||(i>0))
					{
						if (!current_region||(current_region==description->region))
						{
							new_device_number++;
							if (ELECTRODE==device_type)
							{
								i--;
							}
						}
						new_device++;
					}
					return_code=1;
				}
				else
				{
					if (auxiliary_number&&
						((new_auxiliary_number= *auxiliary_number)>=0)&&
						(new_auxiliary_number<map->number_of_auxiliary))
					{
						/* have specified an auxiliary number */
						/* determine the device and the device and electrode numbers */
						new_device=analysis->rig->devices;
						new_device_number=0;
						new_electrode_number= -1;
						i=new_auxiliary_number;
						while ((AUXILIARY!=(device_type=(description=(*new_device)->
							description)->type))||(current_region&&(current_region!=
								description->region))||(i>0))
						{
							if (!current_region||(current_region==description->region))
							{
								new_device_number++;
								if (AUXILIARY==device_type)
								{
									i--;
								}
							}
							new_device++;
						}
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
		}
		/*set things up for return */
		*the_new_device=new_device;
		*the_new_device_number=new_device_number;
		*the_new_electrode_number=new_electrode_number;
		*the_new_auxiliary_number=new_auxiliary_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_get_device_and_numbers_from_number. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_get_device_and_numbers_from_number */

static int analysis_get_device_and_numbers(struct Analysis_work_area *analysis,
	struct Device **device,struct Device ***the_new_device,
	int *device_number,int *the_new_device_number,
	int *electrode_number,int *the_new_electrode_number,
	int *auxiliary_number,int *the_new_auxiliary_number)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Given one of <device>, <device_number>, <electrode_number> or <auxiliary_number>
returns all the corresponding device and number  information in <the_new_device>,
<the_new_device_number> <the_new_electrode_number> <the_new_auxiliary_number>.
ie supply with a <device>, get returned <the_new_device> (same as device),
<the_new_electrode_number> and <the_new_auxiliary_number>.
Calls analysis_get_numbers_from_device, analysis_get_device to do
guts of function.
Can't pass in and return info in same variables as want input variables to
remain unchanged.
*******************************************************************************/
{
	int new_auxiliary_number,new_device_number,new_electrode_number,return_code;

	ENTER(analysis_get_device_and_numbers);
	if ((analysis)&&(analysis->rig)&&(analysis->rig->devices)&&
		(*(analysis->rig->devices))&&the_new_device&&the_new_device_number&&
		the_new_electrode_number&&the_new_auxiliary_number&&
		(device||device_number||electrode_number||auxiliary_number))
	{
		if (device)
		{
			return_code=analysis_get_numbers_from_device(analysis,device,
				&new_device_number,&new_electrode_number,&new_auxiliary_number);
			*the_new_device=device;
			if (return_code&&((device_number&&!(*device_number==new_device_number))||
				(electrode_number&&!(*electrode_number==new_electrode_number))||
				(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
			{
				display_message(ERROR_MESSAGE,
					"analysis_get_device_and_numbers.  device item %s",
					"incompatible with device, electrode and auxiliary numbers");
				return_code=0;
			}
		}
		else
		{
			return_code=analysis_get_device_and_numbers_from_number(analysis,
				the_new_device,device_number,&new_device_number,electrode_number,
				&new_electrode_number,auxiliary_number,&new_auxiliary_number);
		}
		/*set things up for return */
		*the_new_device_number=new_device_number;
		*the_new_electrode_number=new_electrode_number;
		*the_new_auxiliary_number=new_auxiliary_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_get_device_and_numbers. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_get_device_and_numbers */

#if defined (UNEMAP_USE_3D)
int rig_node_highlight_change(struct FE_node *node,void *change_data_void)
/*******************************************************************************
LAST MODIFIED : 3 October 2000

DESCRIPTION :
Change the highlight status of the node/device
==============================================================================*/
{
	int auxiliary_number,device_number,electrode_number,return_code;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct FE_field *device_name_field;
	struct FE_region *all_devices_rig_node_group;
	struct Rig *rig;
	struct rig_node_selection_change_data *data;
	struct Map *map;
	struct Mapping_window *mapping;

	ENTER(rig_node_highlight_change);
	device_name_field=(struct FE_field *)NULL;
	device=(struct Device **)NULL;
	map=(struct Map *)NULL;
	mapping=(struct Mapping_window *)NULL;
	if (node&&(data=(struct rig_node_selection_change_data *)change_data_void)
		&&(analysis=data->analysis_work_area)
		&&(rig=analysis->rig)&&(all_devices_rig_node_group=
		get_Rig_all_devices_rig_node_group(rig)))
	{
		return_code=1;
		if (FE_region_contains_FE_node(all_devices_rig_node_group,node))
		{
			/* find the device corresponding to the node */
			device_name_field=get_unemap_package_device_name_field(
				analysis->unemap_package);
			device=find_device_given_rig_node(node,device_name_field,analysis->rig);
			/* set the highlight */
			(*device)->highlight=data->highlight;
			/* highlight the electrode on 2D map */
			if ((mapping=analysis->mapping_window)&&(map=mapping->map))
			{
				analysis_get_numbers_from_device(analysis,device,&device_number,
					&electrode_number,&auxiliary_number);
				highlight_electrode_or_auxiliar(*device,
#if defined (UNEMAP_USE_NODES)
					(struct FE_node *)NULL,
#endif /* defined (UNEMAP_USE_NODES) */
					electrode_number,auxiliary_number,map,mapping);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"rig_node_highlight_change. node not in rig_node_group");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"rig_node_highlight_change.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* rig_node_highlight_change */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static void rig_node_group_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *analysis_work_area_void)
/*******************************************************************************
LAST MODIFIED : 19 April 2002

DESCRIPTION :
Callback for change in the  node selection. Checks to see if nodes  are in
the rig_node group. If are highlights them.
==============================================================================*/
{
#if !defined (UNEMAP_USE_NODES)
	int auxiliary_number,device_number,electrode_number,end_analysis_interval,
		node_in_current_group,	num_selected,num_unselected,start_analysis_interval;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct FE_node *node;
	struct FE_field *device_name_field;
	struct FE_region *rig_node_group;
	struct Region *current_region;
	struct rig_node_selection_change_data data;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;
#endif /* !defined (UNEMAP_USE_NODES)*/

	ENTER(rig_node_group_node_selection_change);
#if defined (UNEMAP_USE_NODES)
	/*???JW.  Need to update the code to work entirely with nodes i.e to use
		highlight field of nodes and not use devices at all.  See also
		highlight_analysis_device_node (needs updating too) */
	USE_PARAMETER(node_selection);
	USE_PARAMETER(changes);
	USE_PARAMETER(analysis_work_area_void);
	display_message(ERROR_MESSAGE,
			"rig_node_group_node_selection_change. Update to work with nodes! ");
#else /* if defined (UNEMAP_USE_NODES) */
	device=(struct Device **)NULL;
	node=(struct FE_node *)NULL;
	device_name_field=(struct FE_field *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	signals=(struct Signals_area *)NULL;
	current_region=(struct Region *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	if (node_selection&&changes&&
		(analysis=(struct Analysis_work_area *)analysis_work_area_void))
	{
		num_selected=NUMBER_IN_LIST(FE_node)(changes->newly_selected_node_list);
		num_unselected=NUMBER_IN_LIST(FE_node)(changes->newly_unselected_node_list);
		data.analysis_work_area=analysis;
		/* data.multiple_selection flag not used for rig_node_highlight_change */
		data.highlight=0;
		FOR_EACH_OBJECT_IN_LIST(FE_node)(rig_node_highlight_change,(void *)&data,
			changes->newly_unselected_node_list);
		/* highlight the selected nodes/devices */
		data.highlight=1;
		FOR_EACH_OBJECT_IN_LIST(FE_node)(rig_node_highlight_change,(void *)&data,
			changes->newly_selected_node_list);
		/* get the first (un)selected node */
		if (!(node=FIRST_OBJECT_IN_LIST_THAT(FE_node)(
			(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
			changes->newly_selected_node_list)))
		{
			node=FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
				changes->newly_unselected_node_list);
		}
		/* find the device corresponding to the node */
		device_name_field=
			get_unemap_package_device_name_field(analysis->unemap_package);
		device=find_device_given_rig_node(node,device_name_field,analysis->rig);
		/* make it THE highlighted device */
		/*???JW. Pperhaps should set to NULL for node from
			newly_unselected_node_list */
		analysis->highlight=device;
		/* this is an xor; if there's just one electrode/signal (un)selected
			draw (via highlight_signal) just this signal, else  */
		if (((num_selected==1)&&(num_unselected==0))||
			((num_unselected==1)&&(num_selected==0)))
		{
			signals= &(analysis->window->signals);
			if ((signals->number_of_rows)&&(signals->number_of_columns))
			{
				current_region=get_Rig_current_region(analysis->rig);
				node_in_current_group=0;
				if (current_region&&
					(rig_node_group=get_Region_rig_node_group(current_region)))
				{
					if (FE_region_contains_FE_node(rig_node_group,node))
					{
						node_in_current_group=1;
					}
				}
				/* highlight signal if in current region*/
				if ((!current_region)||(node_in_current_group))
				{
					buffer=get_Device_signal_buffer(*(analysis->rig->devices));
					start_analysis_interval=buffer->start;
					end_analysis_interval=buffer->end;
					analysis_get_numbers_from_device(analysis,device,&device_number,
						&electrode_number,&auxiliary_number);
					highlight_signal(*device,
#if defined (UNEMAP_USE_NODES)
						(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,
#endif /* defined (UNEMAP_USE_NODES)*/
						device_number,start_analysis_interval,end_analysis_interval,
						analysis->datum,analysis->potential_time,signals,
						analysis->signal_drawing_information,analysis->user_interface,
						&(analysis->window->interval));
				}
			}
		}
		else
		{
			/* draw all the signals */
			update_signals_drawing_area(analysis->window);
		}
		/* if we're selecting a signal(s) the update windows */
		if (num_selected)
		{
			/* update the  windows*/
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"rig_node_group_node_selection_change.  Invalid argument(s)");
	}
#endif /* defined (UNEMAP_USE_NODES)*/
	LEAVE;
} /* rig_node_group_node_selection_change */
#endif /* defined (UNEMAP_USE_3D) */

static void analysis_set_highlight_min(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION :
Sets the analysis_work_area's highlighted signal's mimimum, from the
contents of the analysis_window->interval.maximum_value XmTextField widget
c.f update_signal_range_widget_from_highlight_signal
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	char *value_string;
	float minimum;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *signal_minimum_field;
	struct FE_field_component component;
	struct FE_node *rig_node;
	struct Signal_drawing_package *signal_drawing_package;
#else
	struct Device *device;
#endif /*	defined (UNEMAP_USE_NODES) */

	ENTER(analysis_set_highlight_min);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	value_string=(char *)NULL;
#if defined (UNEMAP_USE_NODES)
	rig_node=(struct FE_node *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
#else
	device=(struct Device *)NULL;
#endif /*	defined (UNEMAP_USE_NODES) */
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
#if defined (UNEMAP_USE_NODES)
		rig_node=analysis->highlight_rig_node;
		signal_drawing_package=analysis->signal_drawing_package;
#else
		device=*(analysis->highlight);
#endif /*	defined (UNEMAP_USE_NODES) */
		/* extract the value from the widget*/
		XtVaGetValues((analysis->window->interval.minimum_value),
			XmNvalue,&value_string,NULL);
		if (1==sscanf(value_string,"%f",&minimum))
		{
			/* now have the value to set */
#if defined (UNEMAP_USE_NODES)
			signal_drawing_package=analysis->signal_drawing_package;
			signal_minimum_field=get_Signal_drawing_package_signal_minimum_field(
				signal_drawing_package);
			/* set the new signal_minimum */
			component.number=0;
			component.field=signal_minimum_field;
			/*??JW should be copying out of and into node with MANAGER_MODIFY */
			set_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,
				/*time*/0,minimum);
#else
			device->signal_display_minimum=minimum;
#endif /*	defined (UNEMAP_USE_NODES) */
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
		XtFree(value_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_highlight_min.  Missing analysis_work_area");
	}
	LEAVE;
}/* analysis_set_highlight_min */

static void analysis_set_highlight_max(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION :
Sets the analysis_work_area's highlighted signal's maximum, from the
contents of the analysis_window->interval.maximum_value XmTextField widget
c.f update_signal_range_widget_from_highlight_signal
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	char *value_string;
	float maximum;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *signal_maximum_field;
	struct FE_field_component component;
	struct FE_node *rig_node;
	struct Signal_drawing_package *signal_drawing_package;
#else
	struct Device *device;
#endif /*	defined (UNEMAP_USE_NODES) */

	ENTER(analysis_set_highlight_max);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	value_string=(char *)NULL;
#if defined (UNEMAP_USE_NODES)
	rig_node=(struct FE_node *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
#else
	device=(struct Device *)NULL;
#endif /*	defined (UNEMAP_USE_NODES) */
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
#if defined (UNEMAP_USE_NODES)
		rig_node=analysis->highlight_rig_node;
		signal_drawing_package=analysis->signal_drawing_package;
#else
		device=*(analysis->highlight);
#endif /*	defined (UNEMAP_USE_NODES) */
		/* extract the value from the widget*/
		XtVaGetValues((analysis->window->interval.maximum_value),
			XmNvalue,&value_string,NULL);
		if (1==sscanf(value_string,"%f",&maximum))
		{
			/* now have the value to set */
#if defined (UNEMAP_USE_NODES)
			signal_drawing_package=analysis->signal_drawing_package;
			signal_maximum_field=get_Signal_drawing_package_signal_maximum_field(
				signal_drawing_package);
			/* set the new signal_maximum*/
			component.number=0;
			component.field=signal_maximum_field;
			/*??JW should be copying out of and into node with MANAGER_MODIFY */
			set_FE_nodal_FE_value_value(rig_node,&component,0,FE_NODAL_VALUE,
				/*time*/0,maximum);
#else
			device->signal_display_maximum=maximum;
#endif /*	defined (UNEMAP_USE_NODES) */
			/* update the display */
			update_signals_drawing_area(analysis->window);
			update_interval_drawing_area(analysis->window);
			trace_change_signal(analysis->trace);
		}
		XtFree(value_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_set_highlight_max.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_set_highlight_max */

/*
Global functions
----------------
*/
int highlight_analysis_device(unsigned int multiple_selection,
	struct Device **device,	int *device_number,int *electrode_number,
	int *auxiliary_number,struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Perform highlighting of Device specified by <device> or <device_number> or
<electrode_number> or <auxiliary_number>.
Guts of highlighting happens in highlight_analysis_perform_highlighting
==============================================================================*/
{
	int new_auxiliary_number,new_device_number,new_electrode_number,return_code;
	struct Device ***highlight_device,**new_highlight;
#if defined(UNEMAP_USE_3D)
	struct FE_field *device_name_field;
	struct FE_node *rig_node;
	struct FE_node_selection *node_selection;
	struct FE_region *rig_node_group;
#endif /* defined(UNEMAP_USE_3D) */

	ENTER(highlight_analysis_device);
#if defined(UNEMAP_USE_3D)
	device_name_field=(struct FE_field *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	rig_node=(struct FE_node *)NULL;
	node_selection=(struct FE_node_selection *)NULL;
#endif /* defined(UNEMAP_USE_3D) */
	if (analysis)
	{
		/* need to allocate space for  highlight_device (a pointer {to a pointer...}) */
		ALLOCATE(highlight_device,struct Device **,1);
		/*get the device/numbers */
		return_code=analysis_get_device_and_numbers(analysis,device,
			highlight_device,device_number,&new_device_number,electrode_number,
			&new_electrode_number,auxiliary_number,&new_auxiliary_number);
		new_highlight=*highlight_device;
		DEALLOCATE(highlight_device);

		if (return_code)
		{
			if (device)
			{
				*device= *new_highlight;
			}
			if (device_number)
			{
				*device_number=new_device_number;
			}
			if (electrode_number)
			{
				*electrode_number=new_electrode_number;
			}
			if (auxiliary_number)
			{
				*auxiliary_number=new_auxiliary_number;
			}
#if defined(UNEMAP_USE_3D)
			if (analysis->unemap_package)
			{
				node_selection=get_unemap_package_FE_node_selection(
					analysis->unemap_package);
				/* if the multiple_selection flag NOT set, unselect everything */
				if (!multiple_selection)
				{
					FE_node_selection_clear(node_selection);
				}
				/*get the rig_node corresponding to the device */
				device_name_field=get_unemap_package_device_name_field(
					analysis->unemap_package);
				rig_node_group=get_Rig_all_devices_rig_node_group(analysis->rig);
				rig_node=find_rig_node_given_device(*new_highlight,rig_node_group,
					device_name_field);
				/*trigger the selction callback*/
				/* if it wasn't highlighted, highlight it (and vice versa)*/
				if (!((*new_highlight)->highlight))
				{
					FE_node_selection_select_node(node_selection,rig_node);
				}
				else
				{
					FE_node_selection_unselect_node(node_selection,rig_node);
				}
			}
			else
			{
				/*highlight the device */
				return_code=highlight_analysis_perform_highlighting(analysis,
					multiple_selection,new_highlight,new_device_number,
					new_electrode_number,new_auxiliary_number);
			}
#else
			/*highlight the device */
			return_code=highlight_analysis_perform_highlighting(analysis,
				multiple_selection,new_highlight,new_device_number,new_electrode_number,
				new_auxiliary_number);
#endif /* defined(UNEMAP_USE_3D) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"highlight_analysis_device.  Missing arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* highlight_analysis_device */

#if defined (UNEMAP_USE_NODES)
/* need to alter to use callbacks, cf highlight_analysis_device*/
/* see also rig_node_group_node_selection_change */
int highlight_analysis_device_node(unsigned int multiple_selection,
	struct FE_node *device_node,	int *device_number,int *electrode_number,
	int *auxiliary_number,struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 5 September 2000

DESCRIPTION :
If the highlight is part of a multiple selection
then
	If the <device_node> is not highlighted
	then
		highlight it and make it THE highlighted device for the analysis work area
	else
		if it is the only highlighted device
		then
			do nothing
		else
			dehighlight it
			if it is THE highlighted device for the analysis work area
			then
				make the first highlighted device THE highlighted device
else
	highlight it and dehighlight all other devices
	make it THE highlighted device for the analysis work area

cf highlight_analysis_device

==============================================================================*/
{
	char *device_type_string;
	int i,count,highlighted,new_auxiliary_number,new_device_number,new_electrode_number,
		number_of_devices,old_auxiliary_number,old_device_number,old_electrode_number,
		return_code,success;
	struct FE_field_component component;
	struct FE_node *new_highlight_rig_node,*old_highlight_rig_node,
		*temp_device_rig_node;
	struct Interval_area *interval;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct FE_region *rig_node_group,*all_devices_rig_node_group;
	struct FE_node_order_info *rig_node_order_info,*all_devices_rig_node_order_info;
	struct Signals_area *signals;
	struct FE_field *device_type_field;
	struct Signal_drawing_package *signal_drawing_package;

	ENTER(highlight_analysis_device_node);
	rig_node_order_info=(struct FE_node_order_info *)NULL;
	all_devices_rig_node_order_info=(struct FE_node_order_info *)NULL;
	map=(struct Map *)NULL;
	mapping=(struct Mapping_window *)NULL;
	current_region=(struct Region *)NULL;
	signals=(struct Signals_area *)NULL;
	new_highlight_rig_node=(struct FE_node *)NULL;
	old_highlight_rig_node=(struct FE_node *)NULL;
	temp_device_rig_node=(struct FE_node *)NULL;
	interval=(struct Interval_area *)NULL;
	rig_node_group=(struct FE_region *)NULL;
	all_devices_rig_node_group=(struct FE_region *)NULL;
	device_type_string=(char *)NULL;
	device_type_field=(struct FE_field *)NULL;
	signal_drawing_package=(struct Signal_drawing_package *)NULL;
	if (analysis&&(analysis->rig)&&(signal_drawing_package=
		analysis->signal_drawing_package)&&(device_type_field=
		get_Signal_drawing_package_device_type_field(signal_drawing_package))&&
		(device_node||device_number||electrode_number||auxiliary_number))
	{
		current_region=get_Rig_current_region(analysis->rig);
		rig_node_order_info=get_Analysis_window_rig_node_order_info(
			analysis->window);
		if (rig_node_order_info)
		{
			if (analysis->window)
			{
				signals= &(analysis->window->signals);
				interval= &(analysis->window->interval);
			}
			else
			{
				signals=(struct Signals_area *)NULL;
				interval=(struct Interval_area *)NULL;
			}
			if ((analysis->mapping_window)&&(analysis->mapping_window->map))
			{
				mapping=analysis->mapping_window;
				map=mapping->map;
			}
			else
			{
				mapping=(struct Mapping_window *)NULL;
				map=(struct Map *)NULL;
			}
			if (new_highlight_rig_node=device_node)
			{
				/* have specified a device_node */
				/* determine the device, electrode and auxiliary numbers */
				temp_device_rig_node=get_FE_node_order_info_node(rig_node_order_info,0);
				new_device_number=0;
				new_electrode_number=0;
				new_auxiliary_number=0;
				number_of_devices=get_FE_node_order_info_number_of_nodes(rig_node_order_info);
				i=0;
				success=0;
				while ((i<number_of_devices)&&(!success))
				{
					temp_device_rig_node=get_FE_node_order_info_node(rig_node_order_info,i);
					if (temp_device_rig_node==device_node)
					{
						success=1;
					}
					else
					{
						new_device_number++;
						get_FE_nodal_string_value(temp_device_rig_node,device_type_field,0,0,
							FE_NODAL_VALUE,&device_type_string);
						if (!strcmp(device_type_string,"ELECTRODE"))
						{
							new_electrode_number++;
						}
						else if (!strcmp(device_type_string,"AUXILIARY"))
						{
							new_auxiliary_number++;
						}
						DEALLOCATE(device_type_string);
					}
					i++;
				}
				/* success should ALWAYS be 1 when we get here*/
				if (return_code=success)
				{
					get_FE_nodal_string_value(device_node,device_type_field,0,0,
						FE_NODAL_VALUE,&device_type_string);
					if (!strcmp(device_type_string,"ELECTRODE"))
					{
						new_auxiliary_number= -1;
						return_code=1;
					}
					else if (!strcmp(device_type_string,"AUXILIARY"))
					{
						new_electrode_number= -1;
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"highlight_analysis_device_node.  Invalid device type in rig");
						return_code=0;
					}
					/*finished with this*/
					DEALLOCATE(device_type_string);
				}
				if (return_code&&
					((device_number&&!(*device_number==new_device_number))||
						(electrode_number&&!(*electrode_number==new_electrode_number))||
						(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
				{
					display_message(ERROR_MESSAGE,
						"highlight_analysis_device_node.  device item %s",
						"incompatible with device, electrode and auxiliary numbers");
					return_code=0;
				}
			}
			else
			{
				/* have not specified the device */
				if (device_number&&signals&&((new_device_number= *device_number)>=0)&&
					(new_device_number<signals->number_of_signals))
				{
					/* have specified a device_number */
					/* determine the device and the electrode and auxiliary numbers */
					new_electrode_number=0;
					new_auxiliary_number=0;
					for (i=0;i<new_device_number;i++)
					{
						temp_device_rig_node=get_FE_node_order_info_node(
							rig_node_order_info,i);
						get_FE_nodal_string_value(temp_device_rig_node,device_type_field,0,
							0,FE_NODAL_VALUE,&device_type_string);
						if (!strcmp(device_type_string,"ELECTRODE"))
						{
							new_electrode_number++;
						}
						else if (!strcmp(device_type_string,"AUXILIARY"))
						{
							new_auxiliary_number++;
						}
						DEALLOCATE(device_type_string);
					}
					new_highlight_rig_node=get_FE_node_order_info_node(
						rig_node_order_info,new_device_number);
					get_FE_nodal_string_value(new_highlight_rig_node,device_type_field,0,
						0,FE_NODAL_VALUE,&device_type_string);
					if (!strcmp(device_type_string,"ELECTRODE"))
					{
						new_auxiliary_number= -1;
						return_code=1;
					}
					else if (!strcmp(device_type_string,"AUXILIARY"))
					{
						new_electrode_number= -1;
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"highlight_analysis_device_node.  Invalid device type in rig");
						return_code=0;
					}
					/*finished with this*/
					DEALLOCATE(device_type_string);
					if (return_code&&
						((electrode_number&&!(*electrode_number==new_electrode_number))||
							(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
					{
						display_message(ERROR_MESSAGE,"highlight_analysis_device_node.  %s",
							"device number incompatible with electrode and auxiliary numbers");
						return_code=0;
					}
				}/* if (device_number */
				else
				{
					if (mapping&&map)
					{
						if (electrode_number&&((new_electrode_number= *electrode_number)>=0)&&
							(new_electrode_number<map->number_of_electrodes))
						{
							/* have specified an electrode number */
							/* determine the device and the device and auxiliary numbers */
							new_device_number=0;
							new_auxiliary_number= -1;
							i=0;
							number_of_devices=get_FE_node_order_info_number_of_nodes(
								rig_node_order_info);
							success=0;
							while ((new_device_number<number_of_devices)&&(!success))
							{
								temp_device_rig_node=get_FE_node_order_info_node(rig_node_order_info,
									new_device_number);
								get_FE_nodal_string_value(temp_device_rig_node,device_type_field,0,0,
									FE_NODAL_VALUE,&device_type_string);
								if (!strcmp(device_type_string,"ELECTRODE"))
								{
									i++;
								}
								DEALLOCATE(device_type_string);
								if (i>new_electrode_number)
								{
									success=1;
								}
								else
								{
									new_device_number++;
								}
							}
							if (i<new_electrode_number)
								/* weren't enough electodes*/
							{
								display_message(ERROR_MESSAGE,"highlight_analysis_device_node.",
									"bad electrode number");
								new_highlight_rig_node=(struct FE_node *)NULL;
								return_code=0;
							}
							else
							{
								new_highlight_rig_node=get_FE_node_order_info_node(
									rig_node_order_info,new_device_number);
								return_code=1;
							}
						}/* if (electrode_number&*/
						else
						{
							if (auxiliary_number&&
								((new_auxiliary_number= *auxiliary_number)>=0)&&
								(new_auxiliary_number<map->number_of_auxiliary))
							{
								/* have specified an auxiliary number */
								/* determine the device and the device and electrode numbers */
								new_device_number=0;
								new_electrode_number= -1;
								i=0;
								number_of_devices=get_FE_node_order_info_number_of_nodes(
									rig_node_order_info);
								success=0;
								while ((new_device_number<number_of_devices)&&(!success))
								{
									temp_device_rig_node=get_FE_node_order_info_node(rig_node_order_info,
										new_device_number);
									get_FE_nodal_string_value(temp_device_rig_node,device_type_field,0,0,
										FE_NODAL_VALUE,&device_type_string);
									if (!strcmp(device_type_string,"AUXILIARY"))
									{
										i++;
									}
									DEALLOCATE(device_type_string);
									if (i>new_auxiliary_number)
									{
										success=1;
									}
									else
									{
										new_device_number++;
									}
								}
								if (i<new_auxiliary_number)
									/* weren't enough auxiliarys*/
								{
									display_message(ERROR_MESSAGE,"highlight_analysis_device_node.",
										"bad auxiliary number");
									new_highlight_rig_node=(struct FE_node *)NULL;
									return_code=0;
								}
								else
								{
									new_highlight_rig_node=get_FE_node_order_info_node(
										rig_node_order_info,new_device_number);
									return_code=1;
								}
							}/* if (auxiliary_number&& */
							else
							{
								return_code=0;
							}
						}/* if (electrode_number& */
					}/* if (mapping&&map) */
				}/* if (device_number */
			}/* if (new_highlight_rig_node=devic */

			if (return_code)
			{
				if (device_node)
				{
					device_node=new_highlight_rig_node;
				}
				if (device_number)
				{
					*device_number=new_device_number;
				}
				if (electrode_number)
				{
					*electrode_number=new_electrode_number;
				}
				if (auxiliary_number)
				{
					*auxiliary_number=new_auxiliary_number;
				}
				component.field=signal_drawing_package->highlight_field;
				component.number=0;
				/*update the node_order_info current node to the highlighted node */
				/*??JW should be copying out of and into node with MANAGER_MODIFY */
				set_FE_node_order_info_current_node_number(rig_node_order_info,
					new_device_number);
				/* if the highlight is part of a multiple selection */
				if (multiple_selection)
				{
					/* if the device is highlighted */
					get_FE_nodal_int_value(new_highlight_rig_node,&component,0,
						FE_NODAL_VALUE,/*time*/0,&highlighted);
					if (highlighted)
					{
						/* determine whether or not the device is the only highlighted
							device */
						if (new_highlight_rig_node==analysis->highlight_rig_node)
						{
							all_devices_rig_node_group=
								get_Rig_all_devices_rig_node_group(analysis->rig);
							all_devices_rig_node_order_info=
								create_and_sort_FE_node_order_info_from_rig_node_group(
									all_devices_rig_node_group,analysis->signal_order,
									signal_drawing_package);
							count=0;
							number_of_devices=get_FE_node_order_info_number_of_nodes(
								all_devices_rig_node_order_info);
							temp_device_rig_node=get_FE_node_order_info_node(
								all_devices_rig_node_order_info,count);
							get_FE_nodal_int_value(temp_device_rig_node,&component,
								0,FE_NODAL_VALUE,/*time*/0,&highlighted);
							count++;
							while ((count<number_of_devices)&&(!(highlighted)||
								(temp_device_rig_node==new_highlight_rig_node)))
							{
								temp_device_rig_node=get_FE_node_order_info_node(
									all_devices_rig_node_order_info,count);
								get_FE_nodal_int_value(temp_device_rig_node,&component,
									0,FE_NODAL_VALUE,/*time*/0,&highlighted);
								count++;
							}
							/*finished with this */
							DESTROY(FE_node_order_info)(&all_devices_rig_node_order_info);
							if (count<number_of_devices)
							{
								analysis->highlight_rig_node=temp_device_rig_node;
								if (interval)
								{
									update_interval_drawing_area(analysis->window);
								}
								/* update the trace window */
								trace_change_signal(analysis->trace);
							}
						}
						/* if it is not the only highlighted device */
						if (new_highlight_rig_node!=analysis->highlight_rig_node)
						{
							/* dehighlight the selected device */
							/*??JW should be copying out of and into node with MANAGER_MODIFY */
							set_FE_nodal_int_value(new_highlight_rig_node,&component,
								/*version*/0,FE_NODAL_VALUE,/*time*/0,0/*highlight*/);
							highlight_signal((struct Device *)NULL,new_highlight_rig_node,
								analysis->signal_drawing_package,new_device_number,0,0,
								analysis->datum,analysis->potential_time,signals,
								analysis->signal_drawing_information,analysis->user_interface,
								&(analysis->window->interval));
							highlight_electrode_or_auxiliar((struct Device *)NULL,
								new_highlight_rig_node,new_electrode_number,
								new_auxiliary_number,map,mapping);
						}
					}
					else
					{
						/* highlight it and make it THE highlighted device for the analysis
							work area */
						analysis->highlight_rig_node=new_highlight_rig_node;
						/*??JW should be copying out of and into node with MANAGER_MODIFY */
						set_FE_nodal_int_value(new_highlight_rig_node,&component,
							/*version*/0,FE_NODAL_VALUE,/*time*/0,1/*highlight*/);
						highlight_signal((struct Device *)NULL,new_highlight_rig_node,
							analysis->signal_drawing_package,new_device_number,
							0,0,analysis->datum,analysis->potential_time,signals,
							analysis->signal_drawing_information,analysis->user_interface,
							&(analysis->window->interval));
						if (interval)
						{
							update_interval_drawing_area(analysis->window);
						}
						/* update the trace window */
						trace_change_signal(analysis->trace);
						highlight_electrode_or_auxiliar((struct Device *)NULL,
							new_highlight_rig_node,new_electrode_number,
							new_auxiliary_number,map,mapping);
					}
				}/* if (multiple_selection) */
				else
				{
					/* highlight the new device and dehighlight all other devices */
					old_device_number=0;
					old_electrode_number=0;
					old_auxiliary_number=0;
					number_of_devices=get_FE_node_order_info_number_of_nodes
						(rig_node_order_info);
					for (i=0;i<number_of_devices;i++)
					{
						old_highlight_rig_node=get_FE_node_order_info_node(
							rig_node_order_info,i);
						get_FE_nodal_string_value(old_highlight_rig_node,device_type_field,
							0,0,FE_NODAL_VALUE,&device_type_string);
						get_FE_nodal_int_value(old_highlight_rig_node,&component,0,
							FE_NODAL_VALUE,/*time*/0,&highlighted);
						if (!strcmp(device_type_string,"ELECTRODE"))
						{
							if (highlighted)
							{
								/*??JW should be copying out of and into node with MANAGER_MODIFY */
								set_FE_nodal_int_value(old_highlight_rig_node,&component,
									/*version*/0,FE_NODAL_VALUE,/*time*/0,0/*highlight*/);
								highlight_signal((struct Device *)NULL,old_highlight_rig_node,
									analysis->signal_drawing_package,old_device_number,
									0,0,analysis->datum,analysis->potential_time,signals,
									analysis->signal_drawing_information,analysis->user_interface,
									&(analysis->window->interval));
								highlight_electrode_or_auxiliar((struct Device *)NULL,
									old_highlight_rig_node,old_electrode_number,-1,
									map,mapping);
							}
							old_electrode_number++;
						}
						else if (!strcmp(device_type_string,"AUXILIARY"))
						{
							if (highlighted)
							{
								/*??JW should be copying out of and into node with MANAGER_MODIFY */
								set_FE_nodal_int_value(old_highlight_rig_node,&component,
									/*version*/0,FE_NODAL_VALUE,/*time*/0,0/*highlight*/);
								highlight_signal((struct Device *)NULL,old_highlight_rig_node,
									analysis->signal_drawing_package,old_device_number,
									0,0,analysis->datum,analysis->potential_time,signals,
									analysis->signal_drawing_information,analysis->user_interface,
									&(analysis->window->interval));
								highlight_electrode_or_auxiliar((struct Device *)NULL,
									old_highlight_rig_node,-1,old_auxiliary_number,
									map,mapping);
							}
							old_auxiliary_number++;
						}
						DEALLOCATE(device_type_string);
						old_device_number++;
					}
					/* unhighlight devices that AREN'T in current region*/
					if (current_region)
					/*if !current region, previous rig_node_order_info is for all devices */
					/*so nothing to do*/
					{
						struct Region_list_item *region_item=(struct Region_list_item *)NULL;
						struct Region *region=(struct Region *)NULL;
						struct Set_highlight_iterator highlight_iterator;

						region_item=get_Rig_region_list(analysis->rig);
						while (region_item)
						{
							region=get_Region_list_item_region(region_item);
							if (region!=current_region)
							{
								rig_node_group=get_Region_rig_node_group(region);
								if (rig_node_group)
								{
									highlight_iterator.highlight=0;
									highlight_iterator.count=0;
									highlight_iterator.highlight_field=
										signal_drawing_package->highlight_field;
									FE_region_for_each_FE_node(rig_node_group,
										iterative_set_highlight_field,
										(void *)(&highlight_iterator));
								}
							}
							region_item=get_Region_list_item_next(region_item);
						}/* while (region_item)*/
					}
					/* highlight the new device */
					analysis->highlight_rig_node=new_highlight_rig_node;
					if (new_highlight_rig_node)
					{
						/*??JW should be copying out of and into node with MANAGER_MODIFY */
						set_FE_nodal_int_value(new_highlight_rig_node,&component,
							/*version*/0,FE_NODAL_VALUE,/*time*/0,1/*highlight*/);
						highlight_signal((struct Device *)NULL,new_highlight_rig_node,
							analysis->signal_drawing_package,new_device_number,
							0,0,analysis->datum,analysis->potential_time,signals,
							analysis->signal_drawing_information,analysis->user_interface,
							&(analysis->window->interval));
						if (interval)
						{
							update_interval_drawing_area(analysis->window);
						}
						/* update the trace window */
						trace_change_signal(analysis->trace);
						highlight_electrode_or_auxiliar((struct Device *)NULL,
							new_highlight_rig_node,new_electrode_number,
							new_auxiliary_number,map,mapping);
					}/* if (new_highlight_rig_node) */
				}/* if (multiple_selection) */
			}	/* if (return_code) */
		}/* if (rig_node_order_info) */
		else
		{
			display_message(ERROR_MESSAGE,
				"highlight_analysis_device_node. couln't create rig_node_order_info");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"highlight_analysis_device_node. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* highlight_analysis_device_node */
#endif /* defined (UNEMAP_USE_NODES) */

void set_mapping_analysis_region(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Called when a new rig region is selected from the mapping window in the
analysis work area.
==============================================================================*/
{
	char found;
	int number_of_devices;
	struct Device **highlight;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Region *current_region;
	struct Region_list_item *region_item;
	Widget *region,selected_region;

	ENTER(set_mapping_analysis_region);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((mapping=analysis->mapping_window)&&(rig=analysis->rig))
		{
			/* determine the new region selected */
			selected_region=((XmRowColumnCallbackStruct *)call_data)->widget;
			if (selected_region!=mapping->current_region)
			{
				found=0;
				region=mapping->regions;
				/* check for "all regions" */
				if (mapping->number_of_regions>1)
				{
					if (selected_region== *region)
					{
						/* update the analysis rig */
						current_region=(struct Region *)NULL;
						found=1;
					}
					else
					{
						region++;
					}
				}
				if (!found)
				{
					region_item=get_Rig_region_list(rig);
					while (region_item&&(selected_region!= *region))
					{
						region_item=get_Region_list_item_next(region_item);
						region++;
					}
					if (region_item)
					{
						found=1;
						/* update the analysis rig */
						current_region=get_Region_list_item_region(region_item);
					}
				}
				if (found)
				{
					set_Rig_current_region(rig,current_region);
					mapping->current_region=selected_region;
					/* update the highlight device */
					if ((*(analysis->highlight))&&(current_region)&&
						((*(analysis->highlight))->description->region!=current_region))
					{
						/* pick the first highlighted device in the region */
						highlight=rig->devices;
						number_of_devices=rig->number_of_devices;
						while ((number_of_devices>0)&&
							(((*highlight)->description->region!=current_region)||
							!((*highlight)->highlight)))
						{
							number_of_devices--;
							highlight++;
						}
						if (number_of_devices<=0)
						{
							/* pick the first device in the region */
							highlight=rig->devices;
							number_of_devices=rig->number_of_devices;
							while ((number_of_devices>0)&&
								((*highlight)->description->region!=current_region))
							{
								number_of_devices--;
								highlight++;
							}
						}
						if (number_of_devices>0)
						{
/*???pre multiple select							(*(analysis->highlight))->highlight=0;*/
							analysis->highlight=highlight;
							(*highlight)->highlight=1;
							/* update the trace window */
							trace_change_signal(analysis->trace);
							/* update the analysis window */
							update_interval_drawing_area(analysis->window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_mapping_analysis_region.  Error updating highlight");
						}
					}
					else
					{
						trace_update_signal_controls(analysis->trace);
					}
					/* update the analysis window */
					if (analysis_window=analysis->window)
					{
						/* update the region choice in the analysis window */
						analysis_window->current_region=
							(analysis_window->regions)[region-mapping->regions];
						XtVaSetValues(analysis_window->region_choice,
							XmNmenuHistory,analysis_window->current_region,
							NULL);
						update_signals_drawing_area(analysis_window);
					}
					/* update the mapping window */
					update_mapping_window_menu(mapping);
					update_mapping_drawing_area(mapping,2);
					update_mapping_colour_or_auxili(mapping);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_mapping_analysis_region.  Invalid region");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_mapping_analysis_region.  Missing mapping window or rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_mapping_analysis_region.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_mapping_analysis_region */

void analysis_select_auxiliary_drawi(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
The callback for selecting a device in the analysis work area (signals drawing
area, mapping drawing area, colour bar or auxiliary devices drawing area).
==============================================================================*/
{
	char value_string[11];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float x_scale;
	GC graphics_context;
	int ascent,auxiliary_number,*auxiliary_x,*auxiliary_y,colour_bar_left,
		colour_bar_right,descent,direction,marker_bottom,marker_top,
		pointer_sensitivity,pointer_x,pointer_y,previous_spectrum,
		spectrum,spectrum_bound,spectrum_initial,spectrum_left,spectrum_right,
		string_length,text_x,text_y,widget_spacing;
	struct Analysis_work_area *analysis;
	struct Drawing_2d *drawing;
	struct Map *map;
	struct Mapping_window *mapping;
	unsigned int working_button;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XFontStruct *font;
	XmDrawingAreaCallbackStruct *callback;
	Window working_window;

	ENTER(analysis_select_auxiliary_drawi);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface)&&(mapping=analysis->mapping_window)&&
		(drawing=mapping->colour_or_auxiliary_drawing))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if (callback->event)
				{
					if (ButtonPress==callback->event->type)
					{
						display=User_interface_get_display(analysis->user_interface);
						widget_spacing=User_interface_get_widget_spacing(analysis->user_interface);
						button_event= &(callback->event->xbutton);
						pointer_x=button_event->x;
						pointer_y=button_event->y;
						pointer_sensitivity=analysis->pointer_sensitivity;
						working_button=button_event->button;
						if (map=mapping->map)
						{
							if ((SHOW_COLOUR==map->colour_option)||
								(SHOW_CONTOURS==map->contours_option))
							{
								colour_bar_left=map->colour_bar_left;
								colour_bar_right=map->colour_bar_right;
								x_scale=SCALE_FACTOR(map->maximum_value-map->minimum_value,
									colour_bar_right-colour_bar_left);
								/* check if the start or the end of the colour bar has been
									selected */
								if ((pointer_y>=((map->colour_bar_top)-pointer_sensitivity))&&
									(pointer_y<=((map->colour_bar_bottom)+pointer_sensitivity)))
								{
									spectrum_left=SCALE_X(map->contour_minimum,
										map->minimum_value,colour_bar_left,x_scale);
									spectrum_right=SCALE_X(map->contour_maximum,
										map->minimum_value,colour_bar_left,x_scale);
									if ((pointer_x>=spectrum_left-pointer_sensitivity)&&
										(pointer_x<=spectrum_left+pointer_sensitivity))
									{
										moving=MOVING_LEFT;
										spectrum=spectrum_left;
										spectrum_bound=spectrum_right-2*pointer_sensitivity;
										if (spectrum_bound<colour_bar_left)
										{
											spectrum_bound=colour_bar_left;
										}
									}
									else
									{
										if ((pointer_x>=spectrum_right-pointer_sensitivity)&&
											(pointer_x<=spectrum_right+pointer_sensitivity))
										{
											moving=MOVING_RIGHT;
											spectrum=spectrum_right;
											spectrum_bound=spectrum_left+2*pointer_sensitivity;
											if (spectrum_bound>colour_bar_right)
											{
												spectrum_bound=colour_bar_right;
											}
										}
										else
										{
											moving=MOVING_NONE;
										}
									}
									if ((MOVING_LEFT==moving)||(MOVING_RIGHT==moving))
									{
										cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
										/* set the cursor */
										XDefineCursor(display,XtWindow(mapping->colour_or_auxiliary_drawing_area),cursor);
										XmUpdateDisplay(mapping->colour_or_auxiliary_drawing_area);
										working_window=XtWindow(
											mapping->colour_or_auxiliary_drawing_area);
										/* move the pointer to the correct place */
										XWarpPointer(display,None,None,0,0,0,0,
											spectrum-pointer_x,0);
										pointer_x=spectrum;
										spectrum_initial=spectrum;
										/* clear the marker */
										marker_bottom=map->colour_bar_top;
										marker_top=marker_bottom-widget_spacing;
										graphics_context=(map->drawing_information->
											graphics_context).spectrum_marker_colour;
										font=map->drawing_information->font;
										XDrawLine(display,drawing->pixel_map,graphics_context,
											spectrum,marker_top,spectrum,marker_bottom);
										switch (moving)
										{
											case MOVING_LEFT:
											{
												/* write the minimum value */
												sprintf(value_string,"%.4g",map->contour_minimum);
												string_length=strlen(value_string);
												XTextExtents(font,value_string,string_length,
													&direction,&ascent,&descent,&bounds);
												text_x=spectrum_left-bounds.rbearing;
												if (text_x+bounds.lbearing<colour_bar_left)
												{
													text_x=colour_bar_left-bounds.lbearing;
												}
												text_y=widget_spacing+ascent;
											} break;
											case MOVING_RIGHT:
											{
												/* write the maximum value */
												sprintf(value_string,"%.4g",map->contour_maximum);
												string_length=strlen(value_string);
												XTextExtents(font,value_string,string_length,
													&direction,&ascent,&descent,&bounds);
												text_x=spectrum_right-bounds.lbearing;
												if (text_x+bounds.rbearing>colour_bar_right)
												{
													text_x=colour_bar_right-bounds.rbearing;
												}
												text_y=widget_spacing+ascent;
											} break;
										}
										XDrawString(display,drawing->pixel_map,
											(map->drawing_information->graphics_context).
											background_drawing_colour,text_x,text_y,value_string,
											string_length);
										XDrawString(display,working_window,
											(map->drawing_information->graphics_context).
											background_drawing_colour,text_x,text_y,value_string,
											string_length);
										/*???DB.  What about problem with text and GCs on
											RS6000 ? */
										while (moving!=MOVING_NONE)
										{
											XNextEvent(display,&xevent);
											switch (xevent.type)
											{
												case MotionNotify:
												{
													previous_spectrum=spectrum;
													/* reduce the number of motion events displayed */
													while (XCheckMaskEvent(display,ButtonMotionMask,
														&xevent));
													pointer_x=xevent.xmotion.x;
													pointer_y=xevent.xmotion.y;
													if (xevent.xmotion.window==working_window)
													{
														switch (moving)
														{
															case MOVING_LEFT:
															{
																if (pointer_x<colour_bar_left)
																{
																	spectrum=colour_bar_left;
																}
																else
																{
																	if (pointer_x>spectrum_bound)
																	{
																		spectrum=spectrum_bound;
																	}
																	else
																	{
																		spectrum=pointer_x;
																	}
																}
															} break;
															case MOVING_RIGHT:
															{
																if (pointer_x>colour_bar_right)
																{
																	spectrum=colour_bar_right;
																}
																else
																{
																	if (pointer_x<spectrum_bound)
																	{
																		spectrum=spectrum_bound;
																	}
																	else
																	{
																		spectrum=pointer_x;
																	}
																}
															} break;
														}
														if (spectrum!=previous_spectrum)
														{
																/* redraw the marker */
															XDrawLine(display,working_window,
																graphics_context,previous_spectrum,marker_top,
																previous_spectrum,marker_bottom);
															XDrawLine(display,working_window,
																graphics_context,spectrum,marker_top,spectrum,
																marker_bottom);
														}
													}
												} break;
												case ButtonPress:
												{
													if (xevent.xbutton.button==working_button)
													{
														display_message(ERROR_MESSAGE,
															"analysis_select_auxiliary_drawi.  Unexpected button press");
														moving=MOVING_NONE;
													}
												} break;
												case ButtonRelease:
												{
													if (xevent.xbutton.button==working_button)
													{
														if (xevent.xbutton.window==working_window)
														{
															if (spectrum!=spectrum_initial)
															{
																/* recalculate value start or end of colour
																	bar */
																switch (moving)
																{
																	case MOVING_LEFT:
																	{
																		map->contour_minimum=(map->minimum_value)+
																			(float)(spectrum-colour_bar_left)/
																			x_scale;

																	} break;
																	case MOVING_RIGHT:
																	{
																		map->contour_maximum=(map->minimum_value)+
																			(float)(spectrum-colour_bar_left)/
																			x_scale;
																	} break;
																}
																update_mapping_drawing_area(mapping,0);
															}
														}
														update_mapping_colour_or_auxili(mapping);
														/* redraw the marker */
														moving=MOVING_NONE;
													}
												} break;
												default:
												{
													XtDispatchEvent(&xevent);
												}
											}
										}
										XUndefineCursor(display,XtWindow(mapping->colour_or_auxiliary_drawing_area));
										XFreeCursor(display,cursor);
									}
								}
							}
							else
							{
								if (map->number_of_auxiliary>0)
								{
									/* determine the auxiliary number */
									auxiliary_number=0;
									auxiliary_x=map->auxiliary_x;
									auxiliary_y=map->auxiliary_y;
									while ((auxiliary_number<map->number_of_auxiliary)&&
										((pointer_x< *auxiliary_x-pointer_sensitivity)||
											(pointer_x> *auxiliary_x+pointer_sensitivity)||
											(pointer_y< *auxiliary_y-pointer_sensitivity)||
											(pointer_y> *auxiliary_y+pointer_sensitivity)))
									{
										auxiliary_number++;
										auxiliary_x++;
										auxiliary_y++;
									}
#if defined (UNEMAP_USE_NODES)
									highlight_analysis_device_node((button_event->state)&ShiftMask,
										(struct FE_node *)NULL,(int *)NULL,(int *)NULL,
										&auxiliary_number,analysis);
#else
									highlight_analysis_device((button_event->state)&ShiftMask,
										(struct Device **)NULL,(int *)NULL,(int *)NULL,
										&auxiliary_number,analysis);
#endif /*  defined (UNEMAP_USE_NODES) */
								}
							}
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_select_auxiliary_drawi.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_select_auxiliary_drawi.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_select_auxiliary_drawi.  analysis work area missing");
	}
	LEAVE;
} /* analysis_select_auxiliary_drawi */

void analysis_select_map_drawing_are(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 September 2001

DESCRIPTION :
The callback for selecting a device in the analysis work area (signals drawing
area, mapping drawing area, colour bar or auxiliary devices drawing area).
==============================================================================*/
{
	char *electrode_drawn;
	int electrode_number,*electrode_x,*electrode_y,found_electrode,j,
		pointer_sensitivity;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Sub_map *sub_map;
	XButtonEvent *event;
	XmDrawingAreaCallbackStruct *callback;

	ENTER(analysis_select_map_drawing_are);
	electrode_drawn=(char *)NULL;
	electrode_x=(int *)NULL;
	electrode_y=(int *)NULL;
	analysis=(struct Analysis_work_area *)NULL;
	map=(struct Map *)NULL;
	mapping=(struct Mapping_window *)NULL;
	sub_map=(struct Sub_map *)NULL;
	event=(XButtonEvent *)NULL;
	callback=(XmDrawingAreaCallbackStruct *)NULL;
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(mapping=analysis->mapping_window))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if (callback->event)
				{
					if (ButtonPress==callback->event->type)
					{
						event= &(callback->event->xbutton);
						if ((map=mapping->map)&&(map->number_of_electrodes>0))
						{
							found_electrode=0;
							j=0;
							while ((j<map->number_of_sub_maps)&&(!found_electrode))
							{
								sub_map=map->sub_map[j];
								/* the sensitivity depends on the size of the electrode
									marker */
								pointer_sensitivity=map->electrodes_marker_size;
								/* determine the electrode number */
								electrode_number=0;
								electrode_x=sub_map->electrode_x;
								electrode_y=sub_map->electrode_y;
								electrode_drawn=map->electrode_drawn;
								while ((electrode_number<map->number_of_electrodes)&&
									(!found_electrode))
								{
									if ((*electrode_drawn)&&
										((event->x>*electrode_x-pointer_sensitivity)&&
											(event->x<*electrode_x+pointer_sensitivity)&&
											(event->y>*electrode_y-pointer_sensitivity)&&
											(event->y<*electrode_y+pointer_sensitivity)))
									{
										found_electrode=1;
									}
									else
									{
										electrode_number++;
										electrode_x++;
										electrode_y++;
										electrode_drawn++;
									}
								}
								j++;
							}
							if (found_electrode)
							{
								if (!highlight_analysis_device((event->state)&ShiftMask,
									(struct Device **)NULL,(int *)NULL,&electrode_number,
									(int *)NULL,analysis))
								{
#if defined (UNEMAP_USE_3D)
									/*highlight analysis_device_node has failed, */
									/* The mouse click wasn't on an electrode. Unselect everything */
									FE_node_selection_clear(get_unemap_package_FE_node_selection
										(analysis->unemap_package));
#endif /* defined (UNEMAP_USE_3D)	*/
								} /* if (!highlight_analysis_device(*/
							}/* if (found)*/
						}/* if ((map=mapping->map) */
					}/* if (ButtonPress==callback->event->type)*/
				}/* if (callback->event)*/
			}/* if (XmCR_INPUT==callback->reason)*/
			else
			{
				display_message(ERROR_MESSAGE,
					"analysis_select_map_drawing_are.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_select_map_drawing_are.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_select_map_drawing_are.  analysis work area missing");
	}
	LEAVE;
} /* analysis_select_map_drawing_are */

int create_analysis_work_area(struct Analysis_work_area *analysis,
	Widget activation,Widget parent,int pointer_sensitivity,
	char *signal_file_extension_read,char *signal_file_extension_write,
	char *postscript_file_extension,char *configuration_file_extension,
	Pixel identifying_colour,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface, struct Time_keeper *time_keeper,
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 14 April 2004

DESCRIPTION :
Creates the windows associated with the analysis work area.
???Allocate memory for the work area ?
???system_window_structure ?
==============================================================================*/
{
	char *temp_string;
	int return_code;
	static MrmRegisterArg callback_list[]={
		{"analysis_write_interval",(XtPointer)analysis_write_interval},
		{"analysis_reset_to_raw_signals",(XtPointer)analysis_reset_to_raw_signals},
		{"analysis_set_baseline",(XtPointer)analysis_set_baseline},
		{"analysis_set_range_frm_win",(XtPointer)analysis_set_range_frm_win},
		{"analysis_set_range_frm_sig",(XtPointer)analysis_set_range_frm_sig},
		{"analysis_previous_signal",(XtPointer)analysis_previous_signal},
		{"analysis_next_signal",(XtPointer)analysis_next_signal},
		{"analysis_accept_signal",(XtPointer)analysis_accept_signal},
		{"analysis_reject_signal",(XtPointer)analysis_reject_signal},
		{"analysis_unrange_all",(XtPointer)analysis_unrange_all},
		{"analysis_unrange_highlighted",(XtPointer)analysis_unrange_highlighted},
		{"anal_set_range_all_accep_undec",
			(XtPointer)anal_set_range_all_accep_undec},
		{"analysis_read_edf_file",(XtPointer)analysis_read_edf_file},
		{"analysis_read_bdf_file",(XtPointer)analysis_read_bdf_file},
		{"trace_analysis_mode_apply",(XtPointer)trace_analysis_mode_apply},
		{"display_map_with_check",(XtPointer)display_map_with_check},
		{"set_analysis_order_event",(XtPointer)set_analysis_order_event},
		{"set_analysis_order_channel",(XtPointer)set_analysis_order_channel},
		{"select_analysis_interval",(XtPointer)select_analysis_interval},
		{"analysis_set_buffer_start",(XtPointer)analysis_set_buffer_start},
		{"analysis_set_buffer_start_time",
			(XtPointer)analysis_set_buffer_start_time},
		{"analysis_set_buffer_end",(XtPointer)analysis_set_buffer_end},
		{"analysis_set_buffer_end_time",(XtPointer)analysis_set_buffer_end_time},
		{"select_signals_drawing_area",(XtPointer)select_signals_drawing_area},
		{"calculate_all_event_markers",(XtPointer)calculate_all_event_markers},
		{"select_trace_1_drawing_area",(XtPointer)select_trace_1_drawing_area},
		{"set_detection_interval",(XtPointer)set_detection_interval},
		{"set_detection_level",(XtPointer)set_detection_level},
		{"set_detection_threshold",(XtPointer)set_detection_threshold},
		{"set_objective_absolute_slope",(XtPointer)set_objective_absolute_slope},
		{"set_objective_positive_slope",(XtPointer)set_objective_positive_slope},
		{"set_objective_negative_slope",(XtPointer)set_objective_negative_slope},
		{"set_objective_absolute_value",(XtPointer)set_objective_absolute_value},
		{"set_objective_positive_value",(XtPointer)set_objective_positive_value},
		{"set_objective_negative_value",(XtPointer)set_objective_negative_value},
		{"decrement_number_of_events",(XtPointer)decrement_number_of_events},
		{"increment_number_of_events",(XtPointer)increment_number_of_events},
		{"analysis_previous_event",(XtPointer)analysis_previous_event},
		{"analysis_next_event",(XtPointer)analysis_next_event},
		{"analysis_accept_event",(XtPointer)analysis_accept_event},
		{"analysis_reject_event",(XtPointer)analysis_reject_event},
		{"select_trace_3_drawing_area",(XtPointer)select_trace_3_drawing_area},
		{"close_analysis_work_area",(XtPointer)close_analysis_work_area},
		{"analysis_set_highlight_min",(XtPointer)analysis_set_highlight_min},
		{"analysis_set_highlight_max",(XtPointer)analysis_set_highlight_max},
		{"update_eimaging_evnts_frm_dlg",(XtPointer)update_eimaging_evnts_frm_dlg}};
	static MrmRegisterArg identifier_list[]=
	{
		{"analysis_work_area_structure",(XtPointer)NULL},
		{"read_signal_file_data",(XtPointer)NULL},
		{"read_event_times_file_data",(XtPointer)NULL},
		{"read_bard_data_file_data",(XtPointer)NULL},
		{"read_beekeeper_data_file_data",(XtPointer)NULL},
		{"read_cardiomapp_data_file_data",(XtPointer)NULL},
		{"read_neurosoft_data_file_data",(XtPointer)NULL},
		{"write_signal_file_data",(XtPointer)NULL},
		{"overlay_signal_file_data",(XtPointer)NULL}
	};
#define XmNeventsFileExtension "eventsFileExtension"
#define XmCEventsFileExtension "EventsFileExtension"
	static XtResource resources[]=
	{
		{
			XmNeventsFileExtension,
			XmCEventsFileExtension,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Analysis_work_area_settings,events_file_extension),
			XmRString,
			"events"
		}
	};
#if defined (UNEMAP_USE_3D)
	struct FE_node_selection *node_selection;
#endif /* defined (UNEMAP_USE_3D) */

	ENTER(create_analysis_work_area);
	return_code=1;
#if defined (UNEMAP_USE_3D)
	node_selection=(struct FE_node_selection *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	if (analysis&&user_interface
#if defined (UNEMAP_USE_NODES)
		&&package
#endif /* defined (UNEMAP_USE_NODES) */
		)
	{
		analysis->unemap_package=package;
#if defined (UNEMAP_USE_3D)
		if (package)
		{
			ACCESS(Unemap_package)(analysis->unemap_package);
		}
#endif /* defined (UNEMAP_USE_3D) */
		analysis->signal_drawing_package=(struct Signal_drawing_package *)NULL;
#if defined (UNEMAP_USE_NODES)
		analysis->highlight_rig_node=(struct FE_node *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
#if defined (UNEMAP_USE_3D)
		/* set up callback for selecting nodes/devices */
		if (analysis->unemap_package)
		{
			if (node_selection=get_unemap_package_FE_node_selection(
				analysis->unemap_package))
			{
				FE_node_selection_add_callback(node_selection,
					rig_node_group_node_selection_change,(void *)analysis);
			}
		}
#endif /* defined (UNEMAP_USE_3D) */
		analysis->first_eimaging_event=(struct Electrical_imaging_event *)NULL;
		analysis->analysis_mode=EVENT_DETECTION;
		analysis->activation=activation;
		analysis->map_type=NO_MAP_FIELD;
		analysis->map_type_changed=0;
		analysis->user_interface=user_interface;
		analysis->pointer_sensitivity=pointer_sensitivity;
		analysis->postscript_file_extension=postscript_file_extension;
		analysis->configuration_file_extension=configuration_file_extension;
		analysis->signal_file_extension_read=signal_file_extension_read;
		analysis->identifying_colour=identifying_colour;
		analysis->map_drawing_information=map_drawing_information;
		analysis->signal_drawing_information=create_Signal_drawing_information(
			user_interface);
		analysis->analysis_update_flags=ANALYSIS_FLAGS_CLEAR;
		analysis->trace_update_flags=TRACE_FLAGS_CLEAR;
		analysis->potential_time_object=
			ACCESS(Time_object)(CREATE(Time_object)("UNEMAP Potential Time"));
#if defined (UNEMAP_USE_3D)
		if (analysis->unemap_package)
		{
			set_unemap_package_potential_time_object(analysis->unemap_package,
				analysis->potential_time_object);
		}
#endif /* defined (UNEMAP_USE_3D) */
		Time_object_set_next_time_function(analysis->potential_time_object,
			analysis_potential_time_next_time_callback,(void *)analysis);
		if (time_keeper)
		{
			analysis->time_keeper=ACCESS(Time_keeper)(time_keeper);
			Time_object_set_time_keeper(analysis->potential_time_object,time_keeper);
			Time_keeper_add_callback(time_keeper,analysis_time_keeper_callback,
				(void *)analysis,
				(enum Time_keeper_event)(TIME_KEEPER_STARTED|TIME_KEEPER_STOPPED));
		}
		else
		{
			analysis->time_keeper=(struct Time_keeper *)NULL;
		}
		Time_object_add_callback(analysis->potential_time_object,
			analysis_potential_time_update_callback,(void *)analysis);
		/* the datum_time_object is internal and not controlled by a time_keeper */
		analysis->datum_time_object=
			ACCESS(Time_object)(CREATE(Time_object)("UNEMAP Datum Time"));
		Time_object_add_callback(analysis->datum_time_object,
			analysis_datum_time_update_callback,(void *)analysis);
		analysis->search_interval_divisions=(int *)NULL;
		/* DPN 18 June 2001 - Initialise the file selection boxes */
		analysis->read_signal_file_data=(struct File_open_data *)NULL;
		analysis->event_times_file_data=(struct File_open_data *)NULL;
		analysis->read_bard_electrode_data=(struct File_open_data *)NULL;
		analysis->read_beekeeper_eeg_fil_data=(struct File_open_data *)NULL;
		analysis->read_cardiomapp_electr_data=(struct File_open_data *)NULL;
		analysis->read_neurosoft_electro_data=(struct File_open_data *)NULL;
		analysis->write_signal_file_data=(struct File_open_data *)NULL;
		analysis->overlay_signal_file_data=(struct File_open_data *)NULL;
		/* retrieve the settings */
		XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),analysis,
			resources,XtNumber(resources),NULL);
		if (analysis->events_file_extension)
		{
			if (0<strlen(analysis->events_file_extension))
			{
				if (ALLOCATE(temp_string,char,
					strlen(analysis->events_file_extension)+2))
				{
					strcpy(temp_string,".");
					strcat(temp_string,analysis->events_file_extension);
					analysis->events_file_extension=temp_string;
				}
			}
			else
			{
				analysis->events_file_extension=(char *)NULL;
			}
		}
		/* if there is not an analysis window shell */
		if (!(analysis->window_shell))
		{
			/* create the analysis window shell */
			if (!create_analysis_window_shell(&(analysis->window_shell),parent,
				User_interface_get_screen_width(user_interface),User_interface_get_screen_height(user_interface),
				user_interface))
			{
				display_message(ERROR_MESSAGE,
					"create_analysis_work_area.  Could not create analysis window shell");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNames(callback_list,XtNumber(callback_list)))
				/*???DB.  Using global name table because the analysis and trace window
					hierarchies are not open yet.  Should one hierarchy be opened here
					and passed ? (Reduces modularity ?) */
			{
				/* assign and register the identifiers */
				/* DPN 18 June 2001 - need to keep pointers to all the file selection
					boxes */
				identifier_list[0].value=(XtPointer)analysis;
				analysis->read_signal_file_data=create_File_open_data(
					signal_file_extension_read,REGULAR,
					analysis_work_area_read_signal_file,(void *)analysis,0,
					user_interface);
				identifier_list[1].value=(XtPointer)(analysis->read_signal_file_data);
				analysis->event_times_file_data=create_File_open_data(
					analysis->events_file_extension,REGULAR,read_event_times_file,
					(XtPointer)analysis,0,user_interface);
				identifier_list[2].value=(XtPointer)(analysis->event_times_file_data);
				analysis->read_bard_electrode_data=create_File_open_data(".ele",
					REGULAR,analysis_read_bard_electrode_fi,(XtPointer)analysis,0,
					user_interface);
				identifier_list[3].value=
					(XtPointer)(analysis->read_bard_electrode_data);
				analysis->read_beekeeper_eeg_fil_data=create_File_open_data(".eeg",
					REGULAR,analysis_read_beekeeper_eeg_fil,(XtPointer)analysis,0,
					user_interface);
				identifier_list[4].value=
					(XtPointer)(analysis->read_beekeeper_eeg_fil_data);
				analysis->read_cardiomapp_electr_data=create_File_open_data(
					configuration_file_extension,REGULAR,analysis_read_cardiomapp_electr,
					(XtPointer)analysis,0,user_interface);
				identifier_list[5].value=
					(XtPointer)(analysis->read_cardiomapp_electr_data);
				analysis->read_neurosoft_electro_data=create_File_open_data(
					configuration_file_extension,REGULAR,analysis_read_neurosoft_electro,
					(XtPointer)analysis,0,user_interface);
				identifier_list[6].value=
					(XtPointer)(analysis->read_neurosoft_electro_data);
				analysis->write_signal_file_data=create_File_open_data(
					signal_file_extension_write,REGULAR,
					file_analysis_write_signal_file,(XtPointer)analysis,0,user_interface);
				identifier_list[7].value=(XtPointer)(analysis->write_signal_file_data);
				analysis->overlay_signal_file_data=create_File_open_data(
					signal_file_extension_read,REGULAR,analysis_overlay_signal_file,
					(void *)analysis,0,user_interface);
				identifier_list[8].value=
					(XtPointer)(analysis->overlay_signal_file_data);
				if (MrmSUCCESS==MrmRegisterNames(identifier_list,
					XtNumber(identifier_list)))
					/*???DB.  Using global name table because the analysis and trace
						window hierarchies are not open yet.  Should one hierarchy be opened
						here and passed ? (Reduces modularity ?) */
				{
					/* if there is not an analysis window */
					if (!(analysis->window))
					{
						/* create the analysis window */
						if (create_Analysis_window(&(analysis->window),activation,
							analysis->window_shell,&(analysis->rig),
							&(analysis->signal_drawing_package),
							&(analysis->highlight),
#if defined (UNEMAP_USE_NODES)
							&(analysis->highlight_rig_node),
#endif /* defined (UNEMAP_USE_NODES) */
							&(analysis->datum),
							&(analysis->event_number),&(analysis->number_of_events),
							&(analysis->potential_time),&(analysis->detection),
							&(analysis->threshold),&(analysis->minimum_separation),
							&(analysis->level),&(analysis->average_width),
							analysis->identifying_colour,analysis->signal_order,
							SEPARATE_LAYOUT,&(analysis->start_search_interval),
							&(analysis->search_interval_divisions),
							&(analysis->end_search_interval),User_interface_get_screen_height(user_interface),
							postscript_file_extension,analysis->events_file_extension,
							analysis->signal_drawing_information,user_interface,
							&(analysis->signal_order)))
						{
							XtAddCallback(analysis->window->region_pull_down_menu,
								XmNentryCallback,(XtCallbackProc)set_analysis_analysis_region,
								(XtPointer)analysis);
							/* manage the analysis window */
							XtManageChild(analysis->window->window);
							/* realize the analysis shell */
							XtRealizeWidget(analysis->window_shell);
							/*??? more to do ? */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_analysis_work_area.  Could not create analysis window");
							XtDestroyWidget(analysis->window_shell);
							analysis->window_shell=(Widget)NULL;
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_analysis_work_area.  Could not register identifiers");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_analysis_work_area.  Could not register callbacks");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_analysis_work_area.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_analysis_work_area */

void close_analysis_work_area(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION :
Closes the windows associated with the analysis work area.
==============================================================================*/
{
	struct Analysis_work_area *analysis;

	ENTER(close_analysis_work_area);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if (analysis->potential_time_object)
		{
			Time_object_remove_callback(analysis->potential_time_object,
				analysis_potential_time_update_callback, (void *)analysis);
			DEACCESS(Time_object)(&(analysis->potential_time_object));
		}
		if (analysis->time_keeper)
		{
			Time_keeper_remove_callback(analysis->time_keeper,
				analysis_time_keeper_callback, (void *)analysis);
			DEACCESS(Time_keeper)(&(analysis->time_keeper));
		}
		if (analysis->datum_time_object)
		{
			Time_object_remove_callback(analysis->datum_time_object,
				analysis_datum_time_update_callback, (void *)analysis);
			DEACCESS(Time_object)(&(analysis->datum_time_object));
		}
		if (analysis->window_shell)
		{
			/* close the trace window */
			if (analysis->trace)
			{
				XtPopdown(analysis->trace->shell);
			}
			/* close the analysis shell */
			XtPopdown(analysis->window_shell);
			/* unghost the analysis activation button */
			XtSetSensitive(analysis->activation,True);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"close_analysis_work_area.  analysis window shell is missing");
		}
		DEALLOCATE(analysis->search_interval_divisions);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_analysis_work_area.  Missing client_data");
	}
	LEAVE;
} /* close_analysis_work_area */

int destroy_analysis_work_area(struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Frees up the memory associated with the <analysis> work area object given. Does
NOT deallocate the actual pointer <analysis> and should only be called after a
call to close_analysis_work_area().

Created by DPN to try and fix up memory leaks when the UnEmap analysis work area
is used in Cell.
==============================================================================*/
{
	int return_code;
	struct Analysis_window *window;

	ENTER(destroy_analysis_work_area);
	return_code=0;
	if (analysis)
	{
		/* destroy all the file selection boxes */
		if (analysis->read_signal_file_data)
		{
			destroy_File_open_data(&(analysis->read_signal_file_data));
		}
		if (analysis->event_times_file_data)
		{
			destroy_File_open_data(&(analysis->event_times_file_data));
		}
		if (analysis->read_bard_electrode_data)
		{
			destroy_File_open_data(&(analysis->read_bard_electrode_data));
		}
		if (analysis->read_beekeeper_eeg_fil_data)
		{
			destroy_File_open_data(&(analysis->read_beekeeper_eeg_fil_data));
		}
		if (analysis->read_cardiomapp_electr_data)
		{
			destroy_File_open_data(&(analysis->read_cardiomapp_electr_data));
		}
		if (analysis->read_neurosoft_electro_data)
		{
			destroy_File_open_data(&(analysis->read_neurosoft_electro_data));
		}
		if (analysis->write_signal_file_data)
		{
			destroy_File_open_data(&(analysis->write_signal_file_data));
		}
		if (analysis->overlay_signal_file_data)
		{
			destroy_File_open_data(&(analysis->overlay_signal_file_data));
		}
		if (window=analysis->window)
		{
			if (window->write_times_file_open_data)
			{
				destroy_File_open_data(&(window->write_times_file_open_data));
			}
			if (window->print_all_signals_data)
			{
				destroy_File_open_data(&(window->print_all_signals_data));
			}
			if (window->print_selected_signals_data)
			{
				destroy_File_open_data(&(window->print_selected_signals_data));
			}
			if ((window->signals).drawing)
			{
				destroy_Drawing_2d(&((window->signals).drawing));
			}
			if ((window->interval).drawing)
			{
				destroy_Drawing_2d(&((window->interval).drawing));
			}
		}
		/* Clear out the rigs */
		if (analysis->rig)
		{
			destroy_Rig(&(analysis->rig));
		}
		if (analysis->raw_rig)
		{
			destroy_Rig(&(analysis->raw_rig));
		}
		/* destroy the signal drawing information */
		if (analysis->signal_drawing_information)
		{
			destroy_Signal_drawing_information(
				&(analysis->signal_drawing_information));
		}
		/* Destroy the shell widget */
		XtDestroyWidget(analysis->window_shell);
		/* and stuff */
		if (analysis->events_file_extension)
		{
			DEALLOCATE(analysis->events_file_extension);
		}
#if defined (UNEMAP_USE_3D)
		DEACCESS(Unemap_package)(&(analysis->unemap_package));
#endif /* defined (UNEMAP_USE_3D) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_analysis_work_area.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_analysis_work_area */
