/*******************************************************************************
FILE : analysis_work_area.c

LAST MODIFIED : 25 April 2000

DESCRIPTION :
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
#include "time/time_keeper.h"
#include "unemap/analysis.h"
#include "unemap/analysis_window.h"
#include "unemap/analysis_work_area.h"
#include "unemap/bard.h"
#include "unemap/beekeeper.h"
#include "unemap/cardiomapp.h"
#include "unemap/drawing_2d.h"
#include "unemap/interpolate.h"
#include "unemap/mapping_work_area.h"
#include "unemap/neurosoft.h"
#if defined (UNEMAP_USE_NODES)
#include "unemap/rig_node.h"
#endif /* defined (UNEMAP_USE_NODES) */
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
static void display_map(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
??? colour bar ?
??? multiple beats ?
==============================================================================*/
{
	int maintain_aspect_ratio,number_of_frames;
	struct Analysis_window *analysis_window;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Map_frame *frame;
	struct Mapping_window *mapping;
	struct Signal_buffer *buffer;
	struct User_interface *user_interface;

	ENTER(display_map);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis_window=analysis->window)&&
		(user_interface=analysis->user_interface))
	{
		switch (analysis->map_type)
		{
			case SINGLE_ACTIVATION:
			case MULTIPLE_ACTIVATION:
			{
				Spectrum_set_simple_type(analysis->map_drawing_information->spectrum,
					RED_TO_BLUE_SPECTRUM);
			} break;
			default:
			{
				Spectrum_set_simple_type(analysis->map_drawing_information->spectrum,
					BLUE_TO_RED_SPECTRUM);
			} break;
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
		/*???should create mapping window and map if not present */
		if (open_mapping_window(&(analysis->mapping_window),
			*(analysis->mapping_work_area->activation),
			*(analysis->mapping_work_area->parent),
			&(analysis->mapping_work_area->window_shell),
			&(analysis->mapping_work_area->outer_form),
			&(analysis->mapping_work_area->current_mapping_window),
			&(analysis->mapping_work_area->open),
			&(analysis->mapping_work_area->associate),&(analysis->map_type),
			HIDE_COLOUR,HIDE_CONTOURS,SHOW_ELECTRODE_NAMES,HIDE_FIBRES,HIDE_LANDMARKS,
			HIDE_EXTREMA,maintain_aspect_ratio,1,HAMMER_PROJECTION,VARIABLE_THICKNESS,
			&(analysis->rig),&(analysis->event_number),&(analysis->potential_time),
			&(analysis->datum),&(analysis->start_search_interval),
			&(analysis->end_search_interval),analysis->identifying_colour,
			ANALYSIS_ASSOCIATE,(XtPointer)set_mapping_analysis_region,
			(XtPointer)analysis_select_map_drawing_are,
			(XtPointer)analysis_select_auxiliary_drawi,analysis_work_area,
			user_interface->screen_width,user_interface->screen_height,
			analysis->configuration_file_extension,
			analysis->postscript_file_extension,analysis->map_drawing_information,
			analysis->user_interface,analysis->unemap_package))
		{
			mapping=analysis->mapping_window;
			map=mapping->map;
			Mapping_window_set_potential_time_object(mapping,
				analysis->potential_time_object);
			/* determine if undecided events are accepted or rejected */
			if ((widget==analysis_window->map_menu.single_activation_button)||
				(widget==analysis_window->map_menu.multiple_activation_button)||
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
#if defined (OLD_CODE)
			switch (analysis->map_type)
			{
				case MULTIPLE_ACTIVATION:
				{
					Spectrum_set_simple_type(analysis->map_drawing_information->spectrum,
						RED_TO_BLUE_SPECTRUM);
					map->colour_option=SHOW_COLOUR;
					map->interpolation_type=NO_INTERPOLATION;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_option=SHOW_ELECTRODE_VALUES;
				} break;
				default:
				{
					map->colour_option=SHOW_COLOUR;
					map->interpolation_type=BICUBIC_INTERPOLATION;
					map->contours_option=HIDE_CONTOURS;
					map->electrodes_option=HIDE_ELECTRODES;
				} break;
			}
#endif /* defined (OLD_CODE) */
			map->colour_option=SHOW_COLOUR;
			map->contours_option=HIDE_CONTOURS;
			map->electrodes_option=SHOW_ELECTRODE_VALUES;
			if(analysis->time_keeper && Time_keeper_is_playing(analysis->time_keeper))
			{
				map->activation_front = 0;
			}
			else
			{
				map->activation_front= -1;
			}
			if (!(map->fixed_range))
			{
				map->minimum_value=1;
				map->maximum_value=0;
			}
			/* make sure that only one frame */
			if ((1<(number_of_frames=map->number_of_frames))&&(frame=map->frames))
			{
				number_of_frames--;
				while (number_of_frames>0)
				{
					number_of_frames--;
					frame++;
					DEALLOCATE(frame->contour_x);
					DEALLOCATE(frame->contour_y);
					DEALLOCATE(frame->pixel_values);
					DEALLOCATE(frame->image->data);
					XFree((char *)(frame->image));
				}
				if (REALLOCATE(frame,map->frames,struct Map_frame,1))
				{
					map->frames=frame;
					map->frame_number=0;
					map->number_of_frames=1;
				}
				else
				{
					frame=map->frames;
					DEALLOCATE(frame->contour_x);
					DEALLOCATE(frame->contour_y);
					DEALLOCATE(frame->pixel_values);
					DEALLOCATE(frame->image->data);
					XFree((char *)(frame->image));
					DEALLOCATE(map->frames);
					map->frame_number=0;
					map->number_of_frames=0;
					display_message(ERROR_MESSAGE,
						"display_map.  Could not reallocate frames");
				}
			}
			/* set the start and end frame times based on the potential time */
			if ((analysis->rig)&&(analysis->rig->devices)&&
				(*(analysis->rig->devices))&&
				(buffer=get_Device_signal_buffer(*(analysis->rig->devices))))
			{
				map->frame_start_time=
					(float)((buffer->times)[analysis->potential_time])*1000./
					(buffer->frequency);
				map->frame_end_time=map->frame_start_time;
			}
			else
			{
				map->frame_start_time=0;
				map->frame_end_time=0;
			}
			update_mapping_drawing_area(mapping,2);
			update_mapping_colour_or_auxili(mapping);
			if ((map=mapping->map)&&(map->type)&&
				((SINGLE_ACTIVATION== *(map->type)||
				((MULTIPLE_ACTIVATION== *(map->type))&&
				(NO_INTERPOLATION==map->interpolation_type))||
				((POTENTIAL== *(map->type))&&
				(NO_INTERPOLATION==map->interpolation_type)))))
			{
				XtSetSensitive(mapping->animate_button,True);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,True);
			}
			else
			{
				XtSetSensitive(mapping->animate_button,False);
				XtSetSensitive(mapping->print_menu.animate_rgb_button,False);
			}
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
LAST MODIFIED : 19 June 1998

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
					map_type=INTEGRAL;
					start_search_interval=analysis->start_search_interval;
					end_search_interval=analysis->end_search_interval;
				}
			}
		}
		analysis->map_type=map_type;
		current_region=rig->current_region;
		no_event=1;
		no_undecided=1;
		no_accepted=1;
		switch (map_type)
		{
			case SINGLE_ACTIVATION:
			{
				while ((number_of_devices>0)&&(no_undecided||no_accepted))
				{
					if ((ELECTRODE==(description=(*device)->description)->type)&&
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
						parent=user_interface->application_shell;
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
LAST MODIFIED : 24 May 1997

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
	struct Map *map;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Region *current_region;
	struct Region_list_item *region_item;
	Widget *region,selected_region;

	ENTER(set_analysis_analysis_region);
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
					region_item=rig->region_list;
					while (region_item&&(selected_region!= *region))
					{
						region_item=region_item->next;
						region++;
					}
					if (region_item)
					{
						found=1;
						/* update the analysis rig */
						current_region=region_item->region;
					}
				}
				if (found)
				{
					analysis_window->current_region=selected_region;
					rig->current_region=current_region;
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
						if ((map=mapping->map)&&!(map->fixed_range))
						{
							map->minimum_value=1;
							map->maximum_value=0;
						}
						update_mapping_drawing_area(mapping,2);
						update_mapping_colour_or_auxili(mapping);
					}
					/* update the analysis window */
					update_signals_drawing_area(analysis_window);
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

DESCRIPTION :
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
					interval= &(analysis->window->interval);
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
				current_region=rig->current_region;
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
LAST MODIFIED : 22 February 2000

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
			widget_spacing=analysis->user_interface->widget_spacing;
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
				/* clear the colour map */
				map->activation_front= -1;
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				XtSetSensitive(analysis->mapping_window->animate_button,False);
			}
			XtSetSensitive(analysis->window->file_menu.save_times_button,False);
			XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
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
LAST MODIFIED : 23 February 2000

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
			widget_spacing=analysis->user_interface->widget_spacing;
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
LAST MODIFIED : 23 February 2000

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
			widget_spacing=analysis->user_interface->widget_spacing;
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
LAST MODIFIED : 21 February 2000

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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
LAST MODIFIED : 21 February 2000

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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
LAST MODIFIED : 21 February 2000

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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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

static void set_objective_value(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2000

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

	ENTER(set_objective_value);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->user_interface))
	{
		if (VALUE_OBJECTIVE!=analysis->objective)
		{
			analysis->objective=VALUE_OBJECTIVE;
			if (analysis->trace)
			{
				/* enlarge area */
				enlarge= &(analysis->trace->area_1.enlarge);
				XtVaSetValues(enlarge->objective_choice,
					XmNmenuHistory,enlarge->objective.value_button,
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			analysis->calculate_events=0;
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_objective_value.  Missing analysis_work_area");
	}
	LEAVE;
} /* set_objective_value */

static int analysis_write_signal_file(char *file_name,void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 9 December 1999

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	char *temp_string;
	FILE *output_file;
	float signal_maximum,signal_minimum;
	int buffer_end,buffer_start,event_number,event_time,i,new_datum,
		new_end_search_interval,new_potential_time,new_start_search_interval,
		number_of_events,return_code,temp_int;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Event *event,*start_event;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title;

	ENTER(analysis_write_signal_file);
	/* check the arguments */
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(rig=analysis->rig))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			if (return_code=write_signal_file(output_file,rig))
			{
				if ((device=rig->devices)&&(*device)&&((i=rig->number_of_devices)>0)&&
					(buffer=get_Device_signal_buffer(*device)))
				{
					buffer_end=buffer->end;
					buffer_start=buffer->start;
					/* write the event detection settings */
					new_datum=(analysis->datum)-buffer_start;
					new_potential_time=(analysis->potential_time)-buffer_start;
					new_start_search_interval=
						(analysis->start_search_interval)-buffer_start;
					new_end_search_interval=(analysis->end_search_interval)-buffer_start;
					if ((1==BINARY_FILE_WRITE((char *)&(new_datum),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->calculate_events),
						sizeof(char),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->detection),
						sizeof(enum Event_detection_algorithm),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->event_number),sizeof(int),
						1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->number_of_events),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_potential_time),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->minimum_separation),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->threshold),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->datum_type),
						sizeof(enum Datum_type),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->edit_order),
						sizeof(enum Edit_order),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->signal_order),
						sizeof(enum Signal_order),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_start_search_interval),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_end_search_interval),
						sizeof(int),1,output_file)))
					{
						if (EDA_LEVEL==analysis->detection)
						{
							/*???DB.  In case need to change the format later */
							temp_int=1;
							if (!((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
								output_file))&&(1==BINARY_FILE_WRITE((char *)&(analysis->level),
								sizeof(float),1,output_file))&&(1==BINARY_FILE_WRITE(
								(char *)&(analysis->average_width),sizeof(int),1,output_file))))
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
							"analysis_write_signal_file.  Error writing EDA_LEVEL settings");
							}
						}
						/* for each signal write the status, range and events */
						while (return_code&&(i>0))
						{
							/* write the status and range */
							/*???DB.  Originally the unscaled maximum and minimum were
								stored.  This has to be maintained for backward compatability */
							signal_minimum=(*device)->signal_minimum;
							signal_maximum=(*device)->signal_maximum;
							if (0!=((*device)->channel)->gain)
							{
								signal_minimum=(((*device)->channel)->offset)+
									signal_minimum/(((*device)->channel)->gain);
								signal_maximum=(((*device)->channel)->offset)+
									signal_maximum/(((*device)->channel)->gain);
							}
							if ((1==BINARY_FILE_WRITE((char *)&((*device)->signal->status),
								sizeof(enum Event_signal_status),1,output_file))&&
								(1==BINARY_FILE_WRITE((char *)&signal_minimum,
								sizeof(float),1,output_file))&&
								(1==BINARY_FILE_WRITE((char *)&signal_maximum,
								sizeof(float),1,output_file)))
							{
								/* write the events */
								start_event=(*device)->signal->first_event;
								while (start_event&&(start_event->time<buffer_start))
								{
									start_event=start_event->next;
								}
								event=start_event;
								number_of_events=0;
								while (event&&(event->time<=buffer_end))
								{
									number_of_events++;
									event=event->next;
								}
								if (1==BINARY_FILE_WRITE((char *)&number_of_events,sizeof(int),
									1,output_file))
								{
									event=start_event;
									while (return_code&&event&&(event->time<=buffer_end))
									{
										event_number=(event->number)-(start_event->number)+1;
										event_time=(event->time)-buffer_start;
										if ((1==BINARY_FILE_WRITE((char *)&(event_time),sizeof(int),
											1,output_file))&&
											(1==BINARY_FILE_WRITE((char *)&(event_number),sizeof(int),
											1,output_file))&&
											(1==BINARY_FILE_WRITE((char *)&(event->status),
											sizeof(enum Event_signal_status),1,output_file)))
										{
											event=event->next;
										}
										else
										{
											return_code=0;
											display_message(ERROR_MESSAGE,
												"analysis_write_signal_file.  Error writing event");
										}
									}
								}
								else
								{
									return_code=0;
									display_message(ERROR_MESSAGE,
								"analysis_write_signal_file.  Error writing number of events");
								}
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"analysis_write_signal_file.  Error writing signal range");
							}
							device++;
							i--;
						}
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"analysis_write_signal_file.  Error writing analysis settings");
					}
				}
			}
			fclose(output_file);
			if (return_code)
			{
				if (!(analysis->rig->signal_file_name)||
					strcmp(analysis->rig->signal_file_name,file_name))
				{
					XtVaGetValues(analysis->window->window,
						XmNdialogTitle,&old_dialog_title,
						NULL);
					/* assign the signal file name */
					if (ALLOCATE(temp_string,char,strlen(file_name)+1))
					{
						DEALLOCATE(analysis->rig->signal_file_name);
						analysis->rig->signal_file_name=temp_string;
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
"analysis_write_signal_file.  Could not allocate memory for signal file name");
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
		"analysis_write_signal_file.  Could not allocate memory for window title");
					}
					XtVaSetValues(analysis->window->window,
						XmNdialogTitle,new_dialog_title,
						NULL);
					XmStringFree(old_dialog_title);
				}
			}
			else
			{
				remove(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_write_signal_file.  Invalid file: %s",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_write_signal_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_write_signal_file */

static void analysis_write_interval(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 January 2000

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
			analysis_write_signal_file(analysis->rig->signal_file_name,
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

static int analysis_read_signal_file(char *file_name,void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
Sets up the analysis work area for analysing a set of signals.
==============================================================================*/
{
	char calculate_events,*temp_string,value_string[10];
	enum Datum_type datum_type;
	enum Edit_order edit_order;
	enum Event_detection_algorithm detection;
	enum Event_signal_status event_status;
	enum Signal_order signal_order;
	FILE *input_file;
	float level;
	int average_width,buffer_end,buffer_start,datum,end_search_interval,
		event_number,event_time,i,minimum_separation,number_of_events,
		potential_time,return_code,start_search_interval,threshold,temp_int;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Event *event,**event_address;
	struct Map *map=(struct Map *)NULL;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;
	XmString new_dialog_title,old_dialog_title,value_xmstring;
#if defined (UNEMAP_USE_NODES)
	struct FE_field *field;
#endif /* defined (UNEMAP_USE_NODES) */

	ENTER(analysis_read_signal_file);
	return_code=0;
	/* check the arguments */
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
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
			destroy_Rig(&(analysis->rig));
		}
		/* initialize the new analysis */
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
			}
		}
		/* get the analysis window title */
		XtVaGetValues(analysis->window->window,
			XmNdialogTitle,&old_dialog_title,
			NULL);
#if defined (UNEMAP_USE_NODES)
		/* read the signal file into nodes */
			/*???DB.  Would be better to be another callback from the same button ? */
		if (analysis->draw_package)
		{
			DEACCESS(Draw_package)(&(analysis->draw_package));
		}
		if (analysis->rig_node_group=file_read_signal_FE_node_group(file_name,
			analysis->unemap_package))
		{
			ACCESS(Unemap_package)(analysis->unemap_package);
			/* create the draw_package, store it, set it up */
			if(!analysis->draw_package)
			{
				analysis->draw_package=CREATE(Draw_package)();
				ACCESS(Draw_package)(analysis->draw_package);
			}
			field=get_unemap_package_device_name_field(analysis->unemap_package);
			set_Draw_package_device_name_field(analysis->draw_package,field);
			field=get_unemap_package_device_type_field(analysis->unemap_package);
			set_Draw_package_device_type_field(analysis->draw_package,field);
			field=get_unemap_package_channel_number_field(analysis->unemap_package);
			set_Draw_package_channel_number_field(analysis->draw_package,field);
			field=get_unemap_package_signal_field(analysis->unemap_package);
			set_Draw_package_signal_field(analysis->draw_package,field);
			field=get_unemap_package_channel_offset_field(analysis->unemap_package);
			set_Draw_package_channel_offset_field(analysis->draw_package,field);
			field=get_unemap_package_channel_gain_field(analysis->unemap_package);
			set_Draw_package_channel_gain_field(analysis->draw_package,field);
			field=get_unemap_package_signal_minimum_field(analysis->unemap_package);
			set_Draw_package_signal_minimum_field(analysis->draw_package,field);
			field=get_unemap_package_signal_maximum_field(analysis->unemap_package);
			set_Draw_package_signal_maximum_field(analysis->draw_package,field);
			/* for the moment assuming only one signal per node, ie only one
				signal_field */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_read_signal_file. file_read_signal_FE_node_group failed ");
		}
#endif /* defined (UNEMAP_USE_NODES) */
		/* open the input file */
		if ((input_file=fopen(file_name,"rb"))&&
			(read_signal_file(input_file,&(analysis->rig)))&&(rig=analysis->rig)&&
			(rig->devices)&&(*(rig->devices))&&
			(buffer=get_Device_signal_buffer(*(rig->devices))))
		{
			/* read the event detection settings */
			buffer_start=buffer->start;
			buffer_end=buffer->end;
			if ((1==BINARY_FILE_READ((char *)&datum,sizeof(int),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&calculate_events,sizeof(char),1,
				input_file))&&(1==BINARY_FILE_READ((char *)&detection,
				sizeof(enum Event_detection_algorithm),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&event_number,sizeof(int),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&number_of_events,sizeof(int),1,
				input_file))&&(1==BINARY_FILE_READ((char *)&potential_time,sizeof(int),
				1,input_file))&&(1==BINARY_FILE_READ((char *)&minimum_separation,
				sizeof(int),1,input_file))&&(1==BINARY_FILE_READ((char *)&threshold,
				sizeof(int),1,input_file))&&(1==BINARY_FILE_READ((char *)&datum_type,
				sizeof(enum Datum_type),1,input_file))&&(1==BINARY_FILE_READ(
				(char *)&edit_order,sizeof(enum Edit_order),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&signal_order,sizeof(enum Signal_order),1,
				input_file))&&(1==BINARY_FILE_READ((char *)&start_search_interval,
				sizeof(int),1,input_file))&&(1==BINARY_FILE_READ(
				(char *)&end_search_interval,sizeof(int),1,input_file)))
			{
				return_code=1;
				level=analysis->level;
				average_width=analysis->average_width;
				if (EDA_LEVEL==detection)
				{
					if (!((1==BINARY_FILE_READ((char *)&temp_int,sizeof(int),1,
						input_file))&&(1==BINARY_FILE_READ((char *)&level,sizeof(float),1,
						input_file))&&(1==BINARY_FILE_READ((char *)&average_width,
						sizeof(int),1,input_file))))
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"analysis_read_signal_file.  Could not read level");
					}
				}
				/* check the event detection settings */
				if (return_code&&((EDA_INTERVAL==detection)||(EDA_LEVEL==detection)||
					(EDA_THRESHOLD==detection))&&((AUTOMATIC_DATUM==datum_type)||
					(FIXED_DATUM==datum_type))&&((DEVICE_ORDER==edit_order)||
					(BEAT_ORDER==edit_order))&&((EVENT_ORDER==signal_order)||
					(CHANNEL_ORDER==signal_order)))
				{
					if (datum>=buffer_start)
					{
						if (datum>buffer_end)
						{
							datum=buffer_end;
						}
					}
					else
					{
						datum=buffer_start;
					}
					if (potential_time>=buffer_start)
					{
						if (potential_time>buffer_end)
						{
							potential_time=buffer_end;
						}
					}
					else
					{
						potential_time=buffer_start;
					}
					if (minimum_separation<0)
					{
						minimum_separation=0;
					}
					if (level<0)
					{
						level=0;
					}
					if (average_width<0)
					{
						average_width=0;
					}
					if (number_of_events<1)
					{
						number_of_events=1;
					}
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
						"analysis_read_signal_file.  Invalid event detection settings");
				}
				if (return_code)
				{
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
							XtVaSetValues(trace->area_1.enlarge.average_width,
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
							XtSetSensitive(analysis->window->file_menu.save_times_button,
								True);
						}
						else
						{
							XtSetSensitive(
								analysis->window->map_menu.single_activation_button,False);
							XtSetSensitive(
								analysis->window->map_menu.multiple_activation_button,False);
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
					/* for each signal read the status, range and events */
					if ((device=rig->devices)&&((i=rig->number_of_devices)>0))
					{
						while (return_code&&(i>0))
						{
							/* read the status and range */
							if ((1==BINARY_FILE_READ((char *)&event_status,
								sizeof(enum Event_signal_status),1,input_file))&&
								(1==BINARY_FILE_READ((char *)&((*device)->signal_minimum),
								sizeof(float),1,input_file))&&(1==BINARY_FILE_READ(
								(char *)&((*device)->signal_maximum),sizeof(float),1,
								input_file)))
							{
								if ((ACCEPTED==event_status)||(REJECTED==event_status)||
									(UNDECIDED==event_status))
								{
									(*device)->signal->status=event_status;
									if ((*device)->signal_minimum<=(*device)->signal_maximum)
									{
										/*???DB.  Originally the unscaled maximum and minimum were
											stored.  This has to be maintained for backward
											compatability */
										(*device)->signal_minimum=(((*device)->channel)->gain)*
											(((*device)->signal_minimum)-
											(((*device)->channel)->offset));
										(*device)->signal_maximum=(((*device)->channel)->gain)*
											(((*device)->signal_maximum)-
											(((*device)->channel)->offset));
									}
									/* read the events */
									if (1==BINARY_FILE_READ((char *)&number_of_events,sizeof(int),
										1,input_file))
									{
										event_address= &((*device)->signal->first_event);
										event=(struct Event *)NULL;
										while (return_code&&(number_of_events>0))
										{
											if ((1==BINARY_FILE_READ((char *)&(event_time),
												sizeof(int),1,input_file))&&
												(1==BINARY_FILE_READ((char *)&(event_number),
												sizeof(int),1,input_file))&&
												(1==BINARY_FILE_READ((char *)&(event_status),
												sizeof(enum Event_signal_status),1,input_file))&&
												((ACCEPTED==event_status)||(REJECTED==event_status)||
												(UNDECIDED==event_status)))
											{
												if (event=create_Event(event_time,event_number,
													event_status,event,(struct Event *)NULL))
												{
													*event_address=event;
													event_address= &(event->next);
													number_of_events--;
												}
												else
												{
													return_code=0;
													display_message(ERROR_MESSAGE,
														"analysis_read_signal_file.  Error creating event");
												}
											}
											else
											{
												return_code=0;
												display_message(ERROR_MESSAGE,
													"analysis_read_signal_file.  Error reading event");
											}
										}
									}
									else
									{
										return_code=0;
										display_message(ERROR_MESSAGE,
									"analysis_read_signal_file.  Error reading number of events");
									}
								}
								else
								{
									return_code=0;
									display_message(ERROR_MESSAGE,
										"analysis_read_signal_file.  Invalid signal status");
								}
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
							"analysis_read_signal_file.  Error reading signal range/status");
							}
							device++;
							i--;
						}
					}
					else
					{
						return_code=0;
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
			fclose(input_file);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"analysis_read_signal_file.  Invalid file: %s",file_name);
			if (input_file)
			{
				fclose(input_file);
			}
		}
		if (return_code)
		{
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
				display_message(ERROR_MESSAGE,
"analysis_read_signal_file.  Could not allocate memory for signal file name");
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
			"analysis_read_signal_file.  Could not allocate memory for window title");
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
				analysis->identifying_colour,EVENT_DETECTION,&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),&(analysis->rig),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				analysis->user_interface->screen_width,
				analysis->user_interface->screen_height,
				analysis->signal_drawing_information,analysis->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_signal_file.  Could not open trace window");
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
			/* ghost the display potential map button */
			XtSetSensitive(analysis->window->map_menu.potential_button,False);
			/* ghost the display single activation map button */
			XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
			/* ghost the display multiple activation map button */
			XtSetSensitive(analysis->window->map_menu.multiple_activation_button,
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
		update_mapping_window_menu(analysis->mapping_window);
		/* update the drawing areas */
		update_mapping_drawing_area(analysis->mapping_window,2);
		update_mapping_colour_or_auxili(analysis->mapping_window);
		update_signals_drawing_area(analysis->window);
		update_interval_drawing_area(analysis->window);
		/* free the old analysis window title */
		XmStringFree(old_dialog_title);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_signal_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_signal_file */

static int read_event_times_file(char *file_name,void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 11 February 2000

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
		previous_accepted_event_time,return_code,start_search_interval,threshold,
		*times;
	struct Analysis_work_area *analysis;
	struct Event *event,**event_next,*next_event;
	struct Device **device;
	struct Map *map;
	struct Rig *rig;
	struct Signal_buffer *buffer;
	XmString new_dialog_title,old_dialog_title,value_xmstring;

	ENTER(read_event_times_file);
	if (file_name&&(analysis=(struct Analysis_work_area *)analysis_work_area))
	{
		if (input_file=fopen(file_name,"r"))
		{
			/* read the signal file name */
			fscanf(input_file,"signal file name : ");
			if (read_string(input_file,"[^ \n]",&signal_file_name))
			{
				fscanf(input_file,"\n");
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
					destroy_Rig(&(analysis->rig));
				}
				/* initialize the new analysis */
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
						map->electrodes_option=SHOW_ELECTRODE_NAMES;
					}
				}
				/* get the analysis window title */
				XtVaGetValues(analysis->window->window,
					XmNdialogTitle,&old_dialog_title,
					NULL);
				if ((signal_input_file=fopen(signal_file_name,"rb"))&&
					read_signal_file(signal_input_file,&(analysis->rig)))
				{
					return_code=1;
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
						trace_change_rig(analysis->trace);
						/* initialize the potential time */
						analysis->potential_time=buffer->start+
							(buffer->end-buffer->start)/3;
						/* start with all signals rejected */
						number_of_devices=rig->number_of_devices;
						for (i=number_of_devices;i>0;i--)
						{
							(*device)->signal->status=REJECTED;
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
																analysis->trace->area_1.enlarge.average_width,
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
									if ((i>0)&&!((*device)->signal->first_event))
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
									while (' '== *device_line_position)
									{
										device_line_position++;
									}
									device_name_length=device_name_end-device_line_position;
									while ((device_name_end>device_line_position)&&
										(' '== *(device_name_end-1)))
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
										!((*device)->signal->first_event))
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
							analysis->identifying_colour,EVENT_DETECTION,
							&(analysis->detection),&(analysis->objective),
							&(analysis->datum_type),&(analysis->edit_order),
							&(analysis->highlight),&(analysis->rig),&(analysis->datum),
							&(analysis->potential_time),&(analysis->event_number),
							&(analysis->number_of_events),&(analysis->threshold),
							&(analysis->minimum_separation),&(analysis->level),
							&(analysis->average_width),&(analysis->start_search_interval),
							&(analysis->search_interval_divisions),
							&(analysis->end_search_interval),
							analysis->user_interface->screen_width,
							analysis->user_interface->screen_height,
							analysis->signal_drawing_information,analysis->user_interface))
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
					display_message(ERROR_MESSAGE,
						"read_event_times_file.  Invalid signal file: %s",signal_file_name);
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
				update_analysis_window_menu(analysis->window);
				update_mapping_window_menu(analysis->mapping_window);
				/* update the drawing areas */
				update_mapping_drawing_area(analysis->mapping_window,2);
				update_mapping_colour_or_auxili(analysis->mapping_window);
				update_signals_drawing_area(analysis->window);
				update_interval_drawing_area(analysis->window);
				trace_change_signal(analysis->trace);
				/* free the old analysis window title */
				XmStringFree(old_dialog_title);
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
LAST MODIFIED : 3 January 2000

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
				destroy_Rig(&(analysis->rig));
			}
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
					map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,EVENT_DETECTION,&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),&(analysis->rig),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				analysis->user_interface->screen_width,
				analysis->user_interface->screen_height,
				analysis->signal_drawing_information,analysis->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_bard_signal_file.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
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
LAST MODIFIED : 3 January 2000

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
				destroy_Rig(&(analysis->rig));
			}
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
					map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,EVENT_DETECTION,&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),&(analysis->rig),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				analysis->user_interface->screen_width,
				analysis->user_interface->screen_height,
				analysis->signal_drawing_information,analysis->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_beekeeper_eeg_fil.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
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

static int analysis_read_neurosoft_sig_fil(char *file_name,
	void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 3 January 2000

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
				destroy_Rig(&(analysis->rig));
			}
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
					map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,EVENT_DETECTION,&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),&(analysis->rig),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				analysis->user_interface->screen_width,
				analysis->user_interface->screen_height,
				analysis->signal_drawing_information,analysis->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_neurosoft_sig_fil.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
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
		if (read_configuration_file(file_name,&neurosoft_rig))
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
LAST MODIFIED : 30 November 1999

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
				destroy_Rig(&(analysis->rig));
			}
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
					map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			trace_change_rig(analysis->trace);
			/* open the trace window */
			if (!open_trace_window(&(analysis->trace),analysis->window_shell,
				analysis->identifying_colour,EVENT_DETECTION,&(analysis->detection),
				&(analysis->objective),&(analysis->datum_type),&(analysis->edit_order),
				&(analysis->highlight),&(analysis->rig),&(analysis->datum),
				&(analysis->potential_time),&(analysis->event_number),
				&(analysis->number_of_events),&(analysis->threshold),
				&(analysis->minimum_separation),&(analysis->level),
				&(analysis->average_width),&(analysis->start_search_interval),
				&(analysis->search_interval_divisions),&(analysis->end_search_interval),
				analysis->user_interface->screen_width,
				analysis->user_interface->screen_height,
				analysis->signal_drawing_information,analysis->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"analysis_read_cardiomapp_sig_fi.  Could not open trace window");
			}
			update_analysis_window_menu(analysis->window);
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
		if (read_configuration_file(file_name,&cardiomapp_rig))
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
								highlight_analysis_device((event->state)&ControlMask,
									(struct Device **)NULL,&signal_number,(int *)NULL,(int *)NULL,
									analysis);
								/* make sure that the trace window is open */
								open_trace_window(&(analysis->trace),analysis->window_shell,
									analysis->identifying_colour,EVENT_DETECTION,
									&(analysis->detection),&(analysis->objective),
									&(analysis->datum_type),&(analysis->edit_order),
									&(analysis->highlight),&(analysis->rig),&(analysis->datum),
									&(analysis->potential_time),&(analysis->event_number),
									&(analysis->number_of_events),&(analysis->threshold),
									&(analysis->minimum_separation),&(analysis->level),
									&(analysis->average_width),&(analysis->start_search_interval),
									&(analysis->search_interval_divisions),
									&(analysis->end_search_interval),
									analysis->user_interface->screen_width,
									analysis->user_interface->screen_height,
									analysis->signal_drawing_information,
									analysis->user_interface);
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

DESCRIPTION :
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
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,xpos,ypos,signals->axes_width,
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
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				trace_draw_event_marker(event,analysis->trace);
			}
			if (((SINGLE_ACTIVATION==analysis->map_type)||
				(MULTIPLE_ACTIVATION==analysis->map_type))&&
				(mapping=analysis->mapping_window)&&(mapping->map)&&
				(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_option))
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
LAST MODIFIED : 29 November 1993

DESCRIPTION :
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
			if (event=(*highlight)->signal->first_event)
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
					while ((i>=0)&&(rig->current_region)&&
						(rig->current_region!=(*highlight)->description->region))
					{
						highlight--;
						i--;
					}
					if (i<0)
					{
						i=(rig->number_of_devices)-1;
						highlight=devices+i;
						while ((i>0)&&(rig->current_region)&&
							(rig->current_region!=(*highlight)->description->region))
						{
							highlight--;
							i--;
						}
						if (event=(*highlight)->signal->first_event)
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
						while ((i>0)&&(rig->current_region)&&
							(rig->current_region!=(*highlight)->description->region))
						{
							highlight--;
							i--;
						}
						if (event=(*highlight)->signal->first_event)
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

DESCRIPTION :
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
			if (event=(*highlight)->signal->first_event)
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
					while ((i>=0)&&(rig->current_region)&&
						(rig->current_region!=(*highlight)->description->region))
					{
						highlight++;
						i--;
					}
					if (i<0)
					{
						i=(rig->number_of_devices)-1;
						highlight=devices;
						while ((i>0)&&(rig->current_region)&&
							(rig->current_region!=(*highlight)->description->region))
						{
							highlight++;
							i--;
						}
						if (event=(*highlight)->signal->first_event)
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
						while ((i>0)&&(rig->current_region)&&
							(rig->current_region!=(*highlight)->description->region))
						{
							highlight++;
							i--;
						}
						if (event=(*highlight)->signal->first_event)
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
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,xpos,ypos,signals->axes_width,
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
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,trace_area_1->axes_left,
						trace_area_1->axes_top,trace_area_1->axes_width,
						trace_area_1->axes_height,XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,trace_area_3->axes_left,
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
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,xpos,ypos,signals->axes_width,
						signals->axes_height,XtWindow(signals->drawing_area),
						signals->drawing->pixel_map,analysis->signal_drawing_information,
						analysis->user_interface);
				}
				if (trace_area_1&&trace_area_3)
				{
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
						end_analysis_interval,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,trace_area_1->axes_left,
						trace_area_1->axes_top,trace_area_1->axes_width,
						trace_area_1->axes_height,XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					draw_event_marker(event,analysis->event_number,analysis->datum,
						times,frequency,EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,(*highlight)->signal_minimum,
						(*highlight)->signal_maximum,trace_area_3->axes_left,
						trace_area_3->axes_top,trace_area_3->axes_width,
						trace_area_3->axes_height,XtWindow(trace_area_3->drawing_area),
						trace_area_3->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
				}
				if (((SINGLE_ACTIVATION==analysis->map_type)||
					(MULTIPLE_ACTIVATION==analysis->map_type))&&
					(mapping=analysis->mapping_window)&&(mapping->map)&&
					(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_option))
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
		display=user_interface->display;
		if (EVENT_DETECTION==trace_window->analysis_mode)
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
LAST MODIFIED : 21 February 2000

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
					current_region=rig->current_region;
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
					XtVaGetValues(analysis->trace->area_1.enlarge.average_width,
						XmNvalue,&value_string,
						NULL);
					if (1==sscanf(value_string,"%d",&average_width))
					{
						if (average_width<0)
						{
							average_width=0;
						}
					}
					else
					{
						average_width=analysis->average_width;
					}
					analysis->average_width=average_width;
					XtFree(value_string);
					sprintf(global_temp_string,"%d",average_width);
					XtVaSetValues(analysis->trace->area_1.enlarge.average_width,
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
#if defined (OLD_CODE)
				/* for each device/signal calculate the event markers */
				busy_cursor_on((Widget)NULL,analysis->user_interface);
				device=rig->devices;
				number_of_devices=rig->number_of_devices;
				while (number_of_devices>0)
				{
					if ((*device)->signal->status!=REJECTED)
					{
						calculate_device_event_markers(*device,
							analysis->start_search_interval,analysis->end_search_interval,
							analysis->detection,analysis->objective,
							analysis->number_of_events,threshold_percentage,
							minimum_separation,level,average_width);
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
/*					qsort((void *)(rig->devices),rig->number_of_devices,
						sizeof(struct Device *),sort_devices_by_event_time);*/
					heapsort((void *)(rig->devices),rig->number_of_devices,
						sizeof(struct Device *),sort_devices_by_event_time);
					/* update highlight */
					if (highlight)
					{
						current_region=rig->current_region;
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
						if ((device=rig->devices)&&(event=(*device)->signal->first_event))
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
								if ((event=(*device)->signal->first_event)&&(event->time<datum))
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
#endif /* defined (OLD_CODE) */
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
						if ((*device)->signal->status!=REJECTED)
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
	/*					qsort((void *)(rig->devices),rig->number_of_devices,
							sizeof(struct Device *),sort_devices_by_event_time);*/
						heapsort((void *)(rig->devices),rig->number_of_devices,
							sizeof(struct Device *),sort_devices_by_event_time);
						/* update highlight */
						if (highlight)
						{
							current_region=rig->current_region;
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
							if ((device=rig->devices)&&
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
									if ((event=(*device)->signal->first_event)&&
										(event->time<datum))
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
#if defined (OLD_CODE)
					calculate_device_event_markers(highlight_device,
						analysis->start_search_interval,analysis->end_search_interval,
						analysis->detection,analysis->objective,analysis->number_of_events,
						threshold_percentage,minimum_separation,level,average_width);
					draw_device_markers(highlight_device,buffer->start,buffer->end,
						analysis->datum,0,analysis->potential_time,0,SIGNAL_AREA_DETAIL,0,
						xpos,ypos,signals_area->axes_width,signals_area->axes_height,
						XtWindow(signals_area->drawing_area),
						signals_area->drawing->pixel_map,
						analysis->signal_drawing_information,analysis->user_interface);
					trace_draw_markers(0,0,analysis->trace);
#endif /* defined (OLD_CODE) */
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
					current_region=rig->current_region;
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
					current_region=rig->current_region;
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

static void select_analysis_interval(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION :
The callback for modifying the analysis interval in the analysis interval
drawing area.
==============================================================================*/
{
	Boolean owner_events;
	int keyboard_mode,pointer_mode;
	Window confine_to;
	char event_interval_changed,number_string[20],search_interval_changed;
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float frequency,x_scale;
	GC potential_time_colour,potential_time_colour_text;
	int ascent,axes_bottom,axes_left,axes_right,axes_top,box_range,descent,
		direction,i,initial_potential_time,left_box,length,
		minimum_box_range,pointer_sensitivity,pointer_x,pointer_y,
		potential_time,previous_right_box,previous_pointer_x,
		previous_potential_time,previous_left_box,right_box,temp,*times,x_string,
		y_string;
	Pixmap pixel_map;
	struct Analysis_work_area *analysis;
	struct Device *highlight_device;
	struct Interval_area *interval;
	struct Map *map;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;

	unsigned int working_button;
	Window working_window;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XFontStruct *font;
	XmDrawingAreaCallbackStruct *callback;
#if defined(OLD_CODE)
	int end_analysis_interval,signal_index,start_analysis_interval;
	struct User_interface *user_interface;
#endif

	ENTER(select_analysis_interval);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
#if defined(OLD_CODE)
		(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information)&&
#endif
		(analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device))&&(times=buffer->times))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if ((callback->event)&&(ButtonPress==callback->event->type))
				{
					display=analysis->user_interface->display;
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
						/* grab the pointer */
						owner_events=True;
						pointer_mode=GrabModeAsync;
						keyboard_mode=GrabModeAsync;
						confine_to=None;
						if (GrabSuccess==XtGrabPointer(interval->drawing_area,owner_events,
							ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
							pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
						{
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
													case MOVING_LEFT: case MOVING_BOX: case MOVING_RIGHT:
													{
														if ((left_box!=interval->left_box)||
															(right_box!=interval->right_box))
														{
															if (left_box!=interval->left_box)
															{
																buffer->start=SCALE_X(left_box,axes_left,0,
																	1/x_scale);
																interval->left_box=left_box;
															}
															if (right_box!=interval->right_box)
															{
																buffer->end=SCALE_X(right_box,axes_left,0,
																	1/x_scale);
																interval->right_box=right_box;
															}
															/* update the search interval */
															temp=analysis->end_search_interval-
																analysis->start_search_interval;
															search_interval_changed=0;
															if (buffer->start>analysis->start_search_interval)
															{
																search_interval_changed=1;
																if (analysis->search_interval_divisions)
																{
																	for (i=analysis->number_of_events-2;i>=0;i--)
																	{
																		(analysis->search_interval_divisions)[i] +=
																			(buffer->start)-
																			(analysis->start_search_interval);
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
																			(buffer->end)-
																			(analysis->end_search_interval);
																	}
																}
																analysis->end_search_interval=buffer->end;
																analysis->start_search_interval=
																	analysis->end_search_interval-temp;
																if (buffer->start>analysis->
																	start_search_interval)
																{
																	if (analysis->search_interval_divisions)
																	{
																		for (i=analysis->number_of_events-2;i>=0;
																			i--)
																		{
																			(analysis->search_interval_divisions)[
																				i] += (buffer->start)-
																				(analysis->start_search_interval);
																			if ((analysis->search_interval_divisions)[
																				i]>buffer->end)
																			{
																				(analysis->search_interval_divisions)[
																					i]=buffer->end;
																			}
																		}
																	}
																	analysis->start_search_interval=buffer->start;
																}
															}
															if ((trace=analysis->trace)&&
																((EDA_LEVEL==analysis->detection)||
																(EDA_THRESHOLD==analysis->detection)))
															{
																/* update the highlight event interval */
																	/*???DB.  Should be in
																		trace_change_display_interval ? */
																temp=trace->area_3.edit.last_data-
																	trace->area_3.edit.first_data;
																event_interval_changed=0;
																if (buffer->start>
																	trace->area_3.edit.first_data)
																{
																	event_interval_changed=1;
																	trace->area_3.edit.first_data=buffer->start;
																	trace->area_3.edit.last_data=
																		analysis->trace->area_3.edit.first_data+
																		temp;
																}
																if (buffer->end<trace->area_3.edit.last_data)
																{
																	event_interval_changed=1;
																	trace->area_3.edit.last_data=buffer->end;
																	trace->area_3.edit.first_data=
																		trace->area_3.edit.last_data-temp;
																	if (buffer->start>
																		trace->area_3.edit.first_data)
																	{
																		trace->area_3.edit.first_data=buffer->start;
																	}
																}
																if (event_interval_changed)
																{
																	redraw_trace_3_drawing_area((Widget)NULL,
																		(XtPointer)trace,(XtPointer)NULL);
																}
															}
															if (search_interval_changed)
															{
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
																if (EDA_INTERVAL==analysis->detection)
																{
																	/* free the event memory */
																	destroy_all_events(analysis->rig);
																	XtSetSensitive(analysis->window->file_menu.
																		save_times_button,False);
																	XtSetSensitive(
																		analysis->window->map_menu.
																		single_activation_button,
																		False);
																	XtSetSensitive(
																		analysis->window->map_menu.
																		multiple_activation_button,
																		False);
																	analysis->calculate_events=0;
																}
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
																trace_change_search_interval(analysis->trace);
																if ((
#if defined (CLEAR_EVENTS_ON_SEARCH_CHANGE)
																	((EDA_INTERVAL==analysis->detection)&&
																	((SINGLE_ACTIVATION==analysis->map_type)||
																	(MULTIPLE_ACTIVATION==analysis->map_type)))||
#endif /* defined (CLEAR_EVENTS_ON_SEARCH_CHANGE) */
																	(INTEGRAL==analysis->map_type))&&
																	(analysis->mapping_window)&&
																	(map=analysis->mapping_window->map))
																{
																	analysis->map_type=NO_MAP_FIELD;
																	map->colour_option=HIDE_COLOUR;
																	map->contours_option=HIDE_CONTOURS;
																	map->electrodes_option=SHOW_ELECTRODE_NAMES;
																	/* clear the colour map */
																	map->activation_front= -1;
																	update_mapping_drawing_area(
																		analysis->mapping_window,2);
																	update_mapping_colour_or_auxili(
																		analysis->mapping_window);
																	XtSetSensitive(analysis->mapping_window->
																		animate_button,False);
																}
															}
															trace_change_display_interval(analysis->trace);
															update_signals_drawing_area(analysis->window);
														}
													} break;
													case MOVING_POTENTIAL_TIME_MARKER:
													{
														if (potential_time!=initial_potential_time)
														{
#if defined (OLD_CODE)
															start_analysis_interval=buffer->start;
															end_analysis_interval=buffer->end;

															/* clear the old markers */
															trace_draw_potential_time(analysis->trace);
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
																device_number=analysis->rig->number_of_devices;
																device=analysis->rig->devices;
																switch (signals->layout)
																{
																	case SEPARATE_LAYOUT:
																	{
																		rows_divisor=number_of_rows;
																	} break;
																	case OVERLAP_LAYOUT:
																	{
																		rows_divisor=number_of_rows+3;
																	} break;
																}
																xpos=signals_axes_left;
																ypos=signals_axes_top;
																i=0;
																j=0;
																while (device_number>0)
																{
																	if ((*device)->signal)
																	{
																		signal_index=(*device)->signal->index;
																	}
																	else
																	{
																		signal_index= -1;
																	}
																	draw_potential_time_marker(
																		analysis->potential_time,buffer,
																		(*device)->channel,signal_index,
																		SIGNAL_AREA_DETAIL,start_analysis_interval,
																		end_analysis_interval,xpos,ypos,
																		signals_axes_width,signals_axes_height,
																		signals_window,signals_pixel_map,
																		signal_drawing_information,user_interface);
																	i++;
																	if (i>=number_of_rows)
																	{
																		i=0;
																		ypos=signals_axes_top;
																		j++;
																		xpos=signals_axes_left+
																			(j*drawing_width)/number_of_columns;
																	}
																	else
																	{
																		ypos=signals_axes_top+
																			(i*drawing_height)/rows_divisor;
																	}
																	device_number--;
																	device++;
																}
															}
															else
															{
																signals=(struct Signals_area *)NULL;
															}
#endif /* defined (OLD_CODE) */
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
																((double)times[potential_time]*1000.0/
																frequency));
#if defined (OLD_CODE)
															/* write the potential time */
															sprintf(number_string,"%d",
																(int)((float)(times[analysis->potential_time])*
																1000./frequency));
															length=strlen(number_string);
															XTextExtents(font,number_string,length,&direction,
																&ascent,&descent,&bounds);
															x_string=potential_time+
																(bounds.lbearing-bounds.rbearing+1)/2;
															if (x_string+bounds.rbearing>=axes_right)
															{
																x_string=axes_right-bounds.rbearing;
															}
															if (x_string-bounds.lbearing<axes_left)
															{
																x_string=axes_left+bounds.lbearing;
															}
															y_string=axes_top-descent;
															XDrawString(display,pixel_map,
																potential_time_colour_text,
																x_string,y_string,number_string,length);
															XDrawString(display,working_window,
																potential_time_colour,
																x_string,y_string,number_string,length);
															/* draw the new markers */
															trace_draw_potential_time(analysis->trace);
															if (signals)
															{
																device_number=analysis->rig->number_of_devices;
																device=analysis->rig->devices;
																xpos=signals_axes_left;
																ypos=signals_axes_top;
																i=0;
																j=0;
																while (device_number>0)
																{
																	if ((*device)->signal)
																	{
																		signal_index=(*device)->signal->index;
																	}
																	else
																	{
																		signal_index= -1;
																	}
																	draw_potential_time_marker(
																		analysis->potential_time,buffer,
																		(*device)->channel,signal_index,
																		SIGNAL_AREA_DETAIL,start_analysis_interval,
																		end_analysis_interval,xpos,ypos,
																		signals_axes_width,signals_axes_height,
																		signals_window,signals_pixel_map,
																		signal_drawing_information,user_interface);
																	i++;
																	if (i>=number_of_rows)
																	{
																		i=0;
																		ypos=signals_axes_top;
																		j++;
																		xpos=signals_axes_left+
																			(j*drawing_width)/number_of_columns;
																	}
																	else
																	{
																		ypos=signals_axes_top+
																			(i*drawing_height)/rows_divisor;
																	}
																	device_number--;
																	device++;
																}
															}
															/* update the mapping window */
															if ((POTENTIAL==analysis->map_type)&&
																(mapping=analysis->mapping_window)&&
																(map=mapping->map))
															{
																if (NO_INTERPOLATION==map->interpolation_type)
																{
																	update_mapping_drawing_area(mapping,2);
																	update_mapping_colour_or_auxili(mapping);
																}
																else
																{
																	analysis->map_type=NO_MAP_FIELD;
																	map->colour_option=HIDE_COLOUR;
																	map->contours_option=HIDE_CONTOURS;
																	map->electrodes_option=SHOW_ELECTRODE_NAMES;
																	/* clear the colour map */
																	map->activation_front= -1;
																	update_mapping_drawing_area(mapping,2);
																	update_mapping_colour_or_auxili(mapping);
																	XtSetSensitive(mapping->animate_button,False);
																}
															}
#endif /* defined (OLD_CODE) */
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
							/* release the pointer */
							XtUngrabPointer(interval->drawing_area,CurrentTime);
						}
						XFreeCursor(display,cursor);
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

static void decrement_number_of_events(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 January 2000

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
			if (BEAT_AVERAGING==trace->analysis_mode)
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
			map->electrodes_option=SHOW_ELECTRODE_NAMES;
			/* clear the colour map */
			map->activation_front= -1;
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			XtSetSensitive(analysis->mapping_window->animate_button,False);
		}
		XtSetSensitive(analysis->window->file_menu.save_times_button,False);
		XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.multiple_activation_button,False);
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
LAST MODIFIED : 14 January 2000

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
			if (BEAT_AVERAGING==trace->analysis_mode)
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
			map->electrodes_option=SHOW_ELECTRODE_NAMES;
			/* clear the colour map */
			map->activation_front= -1;
			update_mapping_drawing_area(analysis->mapping_window,2);
			update_mapping_colour_or_auxili(analysis->mapping_window);
			XtSetSensitive(analysis->mapping_window->animate_button,False);
		}
		XtSetSensitive(analysis->window->file_menu.save_times_button,False);
		XtSetSensitive(analysis->window->map_menu.single_activation_button,False);
		XtSetSensitive(analysis->window->map_menu.multiple_activation_button,False);
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

static void select_trace_1_drawing_area(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION :
???DB.  Update comment ?
The callback for modifying the search interval in the trace enlarge drawing
area.
???DB.  This should be in trace_window.c and the analysis_work_area calls
should be done as a callback from the trace_window.
==============================================================================*/
{
	Boolean owner_events;
	char edit_box;
	Cursor cursor;
	Display *display;
	enum Event_detection_algorithm detection;
	enum Moving_status moving;
	float frequency,x_scale;
	int axes_bottom,axes_left,axes_right,axes_top,axes_width,box_range,
		datum,*divisions,end_analysis_interval,event_number,i,initial_marker,
		keyboard_mode,left_box,left_edit_box,marker,minimum_box_range,
		number_of_events,pointer_mode,pointer_sensitivity,pointer_x,pointer_y,
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
	struct Trace_window_area_1 *trace_area_1;
	struct User_interface *user_interface;
	unsigned int working_button;
	Window confine_to,working_window;
	XButtonEvent *button_event;
	XEvent xevent;
#if defined(OLD_CODE)
	XFontStruct *font;
#endif
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
				switch (analysis->trace->analysis_mode)
				{
					case EVENT_DETECTION: case BEAT_AVERAGING:
					{
						if ((callback->event)&&(ButtonPress==callback->event->type))
						{
							display=user_interface->display;
#if defined(OLD_CODE)
							font=signal_drawing_information->font;
#endif
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
									/* grab the pointer */
									cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
									owner_events=True;
									pointer_mode=GrabModeAsync;
									keyboard_mode=GrabModeAsync;
									confine_to=None;
									if (GrabSuccess==XtGrabPointer(trace_area_1->drawing_area,
										owner_events,
										ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
										pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
									{
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
																			((double)times[potential_time]*1000.0/
																			frequency));
																	} break;
																	case MOVING_DATUM_MARKER:
																	{
																		analysis->trace_update_flags |=
																			TRACE_1_NO_DATUM_ERASE;
																		Time_object_set_current_time_privileged(
																			analysis->datum_time_object,
																			(double)datum);
																	} break;
																}
#if defined (OLD_CODE)
																/* draw the new markers */
																switch (moving)
																{
																	case MOVING_DATUM_MARKER:
																	{
																		draw_device_markers(highlight_device,
																			start_analysis_interval,
																			end_analysis_interval,datum,0,
																			potential_time,0,ENLARGE_AREA_DETAIL,
																			event_number,trace_area_1->axes_left,
																			trace_area_1->axes_top,
																			trace_area_1->axes_width,
																			trace_area_1->axes_height,
																			XtWindow(trace_area_1->drawing_area),
																			trace_area_1->drawing->pixel_map,
																			signal_drawing_information,
																			user_interface);
																	} break;
																}
																trace_area_3= &(analysis->trace->area_3);
																switch (moving)
																{
																	case MOVING_DATUM_MARKER:
																	{
																		draw_datum_marker(datum,EDIT_AREA_DETAIL,
																			trace_area_3->edit.first_data,
																			trace_area_3->edit.last_data,
																			trace_area_3->axes_left,
																			trace_area_3->axes_top,
																			trace_area_3->axes_width,
																			trace_area_3->axes_height,
																			XtWindow(trace_area_3->drawing_area),
																			trace_area_3->drawing->pixel_map,
																			signal_drawing_information,
																			user_interface);
																		/* write delay time */
																		if (event)
																		{
																			/* draw the new delay time */
																			sprintf(delay_time_string,"%d",
																				(int)((float)(times[event->time]-
																				times[datum])*1000./frequency));
																			length=strlen(delay_time_string);
																			XTextExtents(font,delay_time_string,
																				length,&direction,&ascent,&descent,
																				&bounds);
																			x_string=SCALE_X(event->time,
																				trace_area_3->edit.first_data,
																				trace_area_3->axes_left,
																				SCALE_FACTOR(trace_area_3->edit.
																				last_data-trace_area_3->edit.first_data,
																				(trace_area_3->axes_width)-1))+
																				(bounds.lbearing-bounds.rbearing+1)/2;
																			if (x_string+bounds.rbearing>=
																				trace_area_3->axes_left+
																				trace_area_3->axes_width)
																			{
																				x_string=trace_area_3->axes_left+
																					trace_area_3->axes_width-
																					bounds.rbearing;
																			}
																			if (x_string-bounds.lbearing<
																				trace_area_3->axes_left)
																			{
																				x_string=trace_area_3->axes_left+
																					bounds.lbearing;
																			}
																			y_string=trace_area_3->axes_top-descent;
																			XDrawString(display,
																				trace_area_3->drawing->pixel_map,
																				event_graphics_context_text,x_string,
																				y_string,delay_time_string,length);
																			XDrawString(display,
																				XtWindow(trace_area_3->drawing_area),
																				event_graphics_context,x_string,
																				y_string,delay_time_string,length);
																		}
																	} break;
																}
#endif /* defined (OLD_CODE) */
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
										/* release the pointer */
										XtUngrabPointer(trace_area_1->drawing_area,CurrentTime);
									}
									XFreeCursor(display,cursor);
								}
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
														(BEAT_AVERAGING==analysis->trace->analysis_mode))
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
										/* grab the pointer */
										owner_events=True;
										pointer_mode=GrabModeAsync;
										keyboard_mode=GrabModeAsync;
										confine_to=None;
										if (GrabSuccess==XtGrabPointer(trace_area_1->drawing_area,
											owner_events,
											ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
											pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
										{
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
#if defined (OLD_CODE)
																	draw_highlight_event_box(
																		previous_left_box+((event_number-1)*width)/
																		number_of_events,axes_top,
																		(event_number*width)/number_of_events-
																		((event_number-1)*width)/number_of_events,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
																	draw_highlight_event_box(
																		left_box+((event_number-1)*width)/
																		number_of_events,axes_top,
																		(event_number*width)/number_of_events-
																		((event_number-1)*width)/number_of_events,
																		trace_area_1->axes_height,detection,
																		trace_area_1->drawing_area,
																		trace_area_1->drawing,
																		signal_drawing_information);
#endif /* defined (OLD_CODE) */
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
																			(MULTIPLE_ACTIVATION==
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
																			map->electrodes_option=
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
#if defined (OLD_CODE)
																			draw_highlight_event_box(
																				left_box+((event_number-1)*width)/
																				number_of_events,axes_top,
																				(event_number*width)/number_of_events-
																				((event_number-1)*width)/
																				number_of_events,
																				trace_area_1->axes_height,detection,
																				trace_area_1->drawing_area,
																				trace_area_1->drawing,
																				signal_drawing_information);
#endif /* defined (OLD_CODE) */
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
											/* release the pointer */
											XtUngrabPointer(trace_area_1->drawing_area,CurrentTime);
										}
										XFreeCursor(display,cursor);
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
LAST MODIFIED : 14 January 2000

DESCRIPTION :
???DB.  Change comment ?
The callback for moving the event marker in the trace edit drawing area.
???DB.  This should be in trace_window.c and the analysis_work_area calls
should be done as a callback from the trace_window.
==============================================================================*/
{
	Boolean owner_events;
	char number_string[20],number_string2[20];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float initial_value,frequency,signal_max,signal_min,x_scale,y_scale;
	int ascent,axes_bottom,axes_left,axes_right,axes_top,axes_width,datum,descent,
		device_number,direction,drawing_height,drawing_width,end_analysis_interval,
		event_number,event_time,first_data,i,initial_marker,keyboard_mode,
		last_data,length,length2,marker,number_of_columns,number_of_rows,
		pointer_mode,pointer_sensitivity,pointer_x,pointer_y,potential_time,
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
	Window confine_to,signals_window,working_window;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XFontStruct *font;
	XmDrawingAreaCallbackStruct *callback;
#if defined(OLD_CODE)
	float datum_change;
#endif

	ENTER(select_trace_3_drawing_area);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->trace)&&(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information))
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				switch (analysis->trace->analysis_mode)
				{
					case EVENT_DETECTION: case BEAT_AVERAGING:
					{
						if (analysis->highlight)
						{
							highlight_device= *(analysis->highlight);
						}
						else
						{
							highlight_device=(struct Device *)NULL;
						}
						if ((BEAT_AVERAGING==analysis->trace->analysis_mode)&&
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
								display=user_interface->display;
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
								x_scale=SCALE_FACTOR(last_data-first_data,axes_right-axes_left);
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
												trace_area_3_device->signal_minimum,
												trace_area_3_device->signal_maximum,axes_left,axes_top,
												axes_width,trace_area_3->axes_height,
												XtWindow(trace_area_3->drawing_area),
												trace_area_3->drawing->pixel_map,
												signal_drawing_information,user_interface);
											trace_area_1= &(analysis->trace->area_1);
											start_analysis_interval=buffer->start;
											end_analysis_interval=buffer->end;
											draw_event_marker(event,event_number,datum,times,
												frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
												end_analysis_interval,highlight_device->signal_minimum,
												highlight_device->signal_maximum,
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
												current_region=analysis->rig->current_region;
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
													frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
													end_analysis_interval,
													highlight_device->signal_minimum,
													highlight_device->signal_maximum,xpos,ypos,
													signals_axes_width,signals_axes_height,signals_window,
													signals_pixel_map,signal_drawing_information,
													user_interface);
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
															last_data,trace_area_3_device->signal_minimum,
															trace_area_3_device->signal_maximum,axes_left,
															axes_top,axes_width,trace_area_3->axes_height,
															XtWindow(trace_area_3->drawing_area),
															trace_area_3->drawing->pixel_map,
															signal_drawing_information,user_interface);
														(event_temp->number)--;
														draw_event_marker(event_temp,event_number,datum,
															times,frequency,EDIT_AREA_DETAIL,first_data,
															last_data,trace_area_3_device->signal_minimum,
															trace_area_3_device->signal_maximum,axes_left,
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
														last_data,trace_area_3_device->signal_minimum,
														trace_area_3_device->signal_maximum,axes_left,
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
														last_data,trace_area_3_device->signal_minimum,
														trace_area_3_device->signal_maximum,axes_left,
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
													trace_area_3_device->signal_minimum,
													trace_area_3_device->signal_maximum,axes_left,
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
													trace_area_3_device->signal_minimum,
													trace_area_3_device->signal_maximum,axes_left,
													axes_top,axes_width,trace_area_3->axes_height,
													XtWindow(trace_area_3->drawing_area),
													trace_area_3->drawing->pixel_map,
													signal_drawing_information,user_interface);
											}
											/* draw the marker */
											draw_event_marker(event,event_number,datum,times,
												frequency,EDIT_AREA_DETAIL,first_data,last_data,
												trace_area_3_device->signal_minimum,
												trace_area_3_device->signal_maximum,axes_left,axes_top,
												axes_width,trace_area_3->axes_height,
												XtWindow(trace_area_3->drawing_area),
												trace_area_3->drawing->pixel_map,
												signal_drawing_information,user_interface);
											trace_area_1= &(analysis->trace->area_1);
											start_analysis_interval=buffer->start;
											end_analysis_interval=buffer->end;
											draw_event_marker(event,event_number,datum,times,
												frequency,ENLARGE_AREA_DETAIL,start_analysis_interval,
												end_analysis_interval,highlight_device->signal_minimum,
												highlight_device->signal_maximum,
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
												current_region=analysis->rig->current_region;
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
													frequency,SIGNAL_AREA_DETAIL,start_analysis_interval,
													end_analysis_interval,
													highlight_device->signal_minimum,
													highlight_device->signal_maximum,xpos,ypos,
													signals_axes_width,signals_axes_height,signals_window,
													signals_pixel_map,signal_drawing_information,
													user_interface);
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
													"select_trace_1_drawing_area.  Invalid event status");
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
														/* determine if the positive or the negative y axis
															has been selected */
														signal_min=trace_area_3_device->signal_minimum;
														signal_max=trace_area_3_device->signal_maximum;
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
#if defined (OLD_CODE)
/*???DB.  Changed when adding floats for signal values */
															initial_value=SCALE_Y((float)x_axis_y_marker,
																(float)axes_top,signal_max,1/y_scale);
#endif
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
																(signal_drawing_information->graphics_context).
																scaling_signal_colour;
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
									owner_events=True;
									pointer_mode=GrabModeAsync;
									keyboard_mode=GrabModeAsync;
									confine_to=None;
									if (GrabSuccess==XtGrabPointer(trace_area_3->drawing_area,
										owner_events,
										ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
										pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
									{
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
												if (x_string+bounds.rbearing>=axes_right)
												{
													x_string=axes_right-bounds.rbearing;
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
																		if (x_string+bounds.rbearing>=axes_right)
																		{
																			x_string=axes_right-bounds.rbearing;
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
																		}
																	} break;
																}
															}
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
																			highlight_device->signal_minimum,
																			highlight_device->signal_maximum,
																			trace_area_1->axes_left,
																			trace_area_1->axes_top,
																			trace_area_1->axes_width,
																			trace_area_1->axes_height,
																			XtWindow(trace_area_1->drawing_area),
																			trace_area_1->drawing->pixel_map,
																			signal_drawing_information,
																			user_interface);
																	} break;
																}
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
																			current_region=
																				analysis->rig->current_region;
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
																					highlight_device->signal_minimum,
																					highlight_device->signal_maximum,xpos,
																					ypos,signals_axes_width,
																					signals_axes_height,signals_window,
																					signals_pixel_map,
																					signal_drawing_information,
																					user_interface);
																			}
																		} break;
																	}
																}
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
#if defined(OLD_CODE)
																		datum_change=(float)(times[datum]-
																			times[analysis->datum])*1000/frequency;
#endif
																	} break;
																	case MOVING_POTENTIAL_TIME_MARKER:
																	{
																		potential_time=SCALE_X(marker,axes_left,
																			first_data,1/x_scale);
																	} break;
																	case SCALING_Y_AXIS_POSITIVE:
																	{
#if defined (OLD_CODE)
/*???DB.  Changed when adding floats for signal values */
																		highlight_device->signal_maximum=
																			SCALE_Y(axes_top,x_axis_y_marker,
																			initial_value,SCALE_FACTOR(
																			initial_marker-x_axis_y_marker,
																			marker-x_axis_y_marker)/y_scale);
#endif
																		highlight_device->signal_maximum=
																			initial_value+
																			(float)(x_axis_y_marker-axes_top)*
																			(float)(marker-x_axis_y_marker)/(y_scale*
																			(float)(initial_marker-x_axis_y_marker));
																		if (highlight_device!=trace_area_3_device)
																		{
																			trace_area_3_device->signal_maximum=
																				highlight_device->signal_maximum;
																		}
																	} break;
																	case SCALING_Y_AXIS_NEGATIVE:
																	{
#if defined (OLD_CODE)
/*???DB.  Changed when adding floats for signal values */
																		highlight_device->signal_minimum=
																			SCALE_Y(axes_bottom,x_axis_y_marker,
																			initial_value,SCALE_FACTOR(
																			initial_marker-x_axis_y_marker,
																			marker-x_axis_y_marker)/y_scale);
#endif
																		highlight_device->signal_minimum=
																			initial_value+
																			(float)(x_axis_y_marker-axes_bottom)*
																			(float)(marker-x_axis_y_marker)/(y_scale*
																			(float)(initial_marker-x_axis_y_marker));
																		if (highlight_device!=trace_area_3_device)
																		{
																			trace_area_3_device->signal_minimum=
																				highlight_device->signal_minimum;
																		}
																	} break;
																}
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
																}
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
																			((double)times[potential_time]*1000.0/
																			frequency));
																	} break;
																	case MOVING_DATUM_MARKER:
																	{
																		analysis->trace_update_flags |=
																			TRACE_3_NO_DATUM_ERASE;
																		Time_object_set_current_time_privileged(
																			analysis->datum_time_object,
																			(double)datum);
																	} break;

																}
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
																			highlight_device->signal_minimum,
																			highlight_device->signal_maximum,
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
																}
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
																				highlight_device->signal_minimum,
																				highlight_device->signal_maximum,xpos,
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
																			draw_signal(
																				(struct FE_node *)NULL,
																				(struct Draw_package *)NULL,
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
																}
#if defined (OLD_CODE)
																if (((SINGLE_ACTIVATION==analysis->map_type)||
																	(MULTIPLE_ACTIVATION==analysis->map_type))&&
																	(MOVING_DATUM_MARKER==moving)&&
																	(mapping=analysis->mapping_window)&&
																	(map=mapping->map))
																{
																	if (NO_INTERPOLATION==map->interpolation_type)
																	{
																		update_mapping_drawing_area(mapping,2);
																		update_mapping_colour_or_auxili(mapping);
																	}
																	else
																	{
																		analysis->map_type=NO_MAP_FIELD;
																		map->colour_option=HIDE_COLOUR;
																		map->contours_option=HIDE_CONTOURS;
																		map->electrodes_option=SHOW_ELECTRODE_NAMES;
																		/* clear the colour map */
																		map->activation_front= -1;
																		update_mapping_drawing_area(mapping,2);
																		update_mapping_colour_or_auxili(mapping);
																		XtSetSensitive(mapping->animate_button,
																			False);
#if defined (OLD_CODE)
/*???DB.  Could subtract datum change off everything, but probably not worth
	it */
																		/* update the map */
																		map->contour_maximum -= datum_change;
																		map->maximum_value -= datum_change;
																		map->minimum_value -= datum_change;
																		map->contour_minimum -= datum_change;
																		region_item=analysis->rig->region_list;
																		while (region_item)
																		{
																			if (interpolation_function=region_item->
																				region->interpolation_function)
																			{
																				interpolation_function->f_max -=
																					datum_change;
																				interpolation_function->f_min -=
																					datum_change;
																				f=interpolation_function->f;
																				for (i=interpolation_function->
																					number_of_nodes;i>0;i--)
																				{
																					*f -= datum_change;
																					f++;
																				}
																			}
																			region_item=region_item->next;
																		}
																		update_mapping_colour_or_auxili(mapping);
																		if (SHOW_ELECTRODE_VALUES==
																			map->electrodes_option)
																		{
																			update_mapping_drawing_area(mapping,0);
																		}
#endif /* defined (OLD_CODE) */
																	}
																}
																else
																{
																	if ((POTENTIAL==analysis->map_type)&&
																		((MOVING_EVENT_MARKER==moving)||
																		(MOVING_POTENTIAL_TIME_MARKER==moving))&&
																		(mapping=analysis->mapping_window)&&
																		(map=mapping->map))
																	{
																		if (NO_INTERPOLATION==
																			map->interpolation_type)
																		{
																			update_mapping_drawing_area(mapping,2);
																			update_mapping_colour_or_auxili(mapping);
																		}
																		else
																		{
																			analysis->map_type=NO_MAP_FIELD;
																			map->colour_option=HIDE_COLOUR;
																			map->contours_option=HIDE_CONTOURS;
																			map->electrodes_option=
																				SHOW_ELECTRODE_NAMES;
																			/* clear the colour map */
																			map->activation_front= -1;
																			update_mapping_drawing_area(mapping,2);
																			update_mapping_colour_or_auxili(mapping);
																			XtSetSensitive(mapping->animate_button,
																				False);
																		}
																	}
																	else
																	{
																		if (((SINGLE_ACTIVATION==
																			analysis->map_type)||
																			(MULTIPLE_ACTIVATION==
																			analysis->map_type))&&
																			(MOVING_EVENT_MARKER==moving)&&
																			(mapping=analysis->mapping_window)&&
																			(map=mapping->map)&&
																			(SHOW_ELECTRODE_VALUES==
																			map->electrodes_option))
																		{
																			update_mapping_drawing_area(mapping,0);
																		}
																	}
																}
#endif /* defined (OLD_CODE) */
															}
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
										/* release the pointer */
										XtUngrabPointer(trace_area_3->drawing_area,CurrentTime);
									}
									XFreeCursor(display,cursor);
								}
							}
						}
					} break;
				}
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
LAST MODIFIED : 13 October 1999

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

	ENTER(create_processed_rig);
	if (raw_rig&&(0<raw_rig->number_of_devices)&&(raw_rig->devices)&&
		(*(raw_rig->devices))&&
		(raw_signal_buffer=get_Device_signal_buffer(*(raw_rig->devices))))
	{
		if (rig=create_Rig(raw_rig->name,raw_rig->monitoring,raw_rig->experiment,0,
			(struct Device **)NULL,(struct Page_list_item *)NULL,0,
			(struct Region_list_item *)NULL,(struct Region *)NULL))
		{
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
							raw_region->name,raw_region->type,raw_region->number,0),
							(struct Region_list_item *)NULL))&&
							((*region_item_address)->region))
						{
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
"create_processed_rig.  Could not create electrode for an auxiliary device that is a linear combination");
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
"create_processed_rig.  Could not allocate memory for an auxiliary device that is a linear combination");
																DEALLOCATE(auxiliary->electrodes);
																DEALLOCATE(auxiliary->electrode_coefficients);
																destroy_Rig(&rig);
															}
														}
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
LAST MODIFIED : 19 June 1998

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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
LAST MODIFIED : 19 June 1998

DESCRIPTION :
For every electrode signal in the current region, the baseline is shifted to the
value at the potential time.
==============================================================================*/
{
	float channel_offset,*value;
	int buffer_offset,i,j,potential_time;
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
				(*device)->signal_maximum=0;
				(*device)->signal_minimum=1;
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
			current_region=rig->current_region;
			/* run through all the signals */
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					channel_offset=(buffer->signals.float_values)[potential_time*
						buffer_offset+(*device)->signal->index];
					value=(buffer->signals.float_values)+(*device)->signal->index;
					for (j=buffer->number_of_samples;j>0;j--)
					{
						*value -= channel_offset;
						value += buffer_offset;
					}
				}
				device++;
			}
#if defined (OLD_CODE)
			switch (buffer->value_type)
			{
				case SHORT_INT_VALUE:
				{
					for (i=rig->number_of_devices;i>0;i--)
					{
						/* for the electrodes in the current region */
						if ((ELECTRODE==(*device)->description->type)&&(!current_region||
							(current_region==(*device)->description->region)))
						{
							/* reset the channel offset */
							(*device)->channel->offset=
								(float)(buffer->signals.short_int_values)
								[potential_time*buffer_offset+(*device)->signal->index];
						}
						device++;
					}
				} break;
				case FLOAT_VALUE:
				{
					for (i=rig->number_of_devices;i>0;i--)
					{
						/* for the electrodes in the current region */
						if ((ELECTRODE==(*device)->description->type)&&(!current_region||
							(current_region==(*device)->description->region)))
						{
							/* reset the channel offset */
							(*device)->channel->offset=(buffer->signals.float_values)
								[potential_time*buffer_offset+(*device)->signal->index];
						}
						device++;
					}
				} break;
			}
#endif /* defined (OLD_CODE) */
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
				map->electrodes_option=SHOW_ELECTRODE_NAMES;
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

static void analysis_set_range(Widget widget,XtPointer analysis_work_area,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 December 1996

DESCRIPTION :
For every electrode signal in the current region, the range is changed to be the
same as the range for the current signal.
==============================================================================*/
{
	float channel_gain,channel_offset,maximum,minimum;
	int i;
	struct Analysis_work_area *analysis;
	struct Device **device,*highlight;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(analysis_set_range);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		/* check that rig and signals exist */
		if ((rig=analysis->rig)&&(device=rig->devices)&&(*device)&&
			((*device)->signal)&&(analysis->highlight)&&
			(highlight= *(analysis->highlight)))
		{
			/* calculate the range for the current signal */
			channel_gain=highlight->channel->gain;
			channel_offset=highlight->channel->offset;
			minimum=channel_gain*((float)(highlight->signal_minimum)-channel_offset);
			maximum=channel_gain*((float)(highlight->signal_maximum)-channel_offset);
			/* run through all the signals */
			current_region=rig->current_region;
			for (i=rig->number_of_devices;i>0;i--)
			{
				/* for the electrodes in the current region */
				if ((ELECTRODE==(*device)->description->type)&&(!current_region||
					(current_region==(*device)->description->region)))
				{
					/* set the signal minimum and maximum */
					if (0<(channel_gain=(*device)->channel->gain))
					{
						channel_offset=(*device)->channel->offset;
						(*device)->signal_minimum=channel_offset+minimum/channel_gain;
						(*device)->signal_maximum=channel_offset+maximum/channel_gain;
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
			"analysis_set_range.  Missing analysis_work_area");
	}
	LEAVE;
} /* analysis_set_range */

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
				read_signal_file(input_file,&(temp_rig)))
			{
				/* assume that analysis is not done on overlay signals, so don't read
					the event detection settings (see analysis_read_signal_file) */
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
							(*device)->signal_maximum=0;
							(*device)->signal_minimum=1;
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

static void analysis_accept_signal(Widget widget,
	XtPointer analysis_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
==============================================================================*/
{
	char *value_string;
	float level,*objective_values;
	int average_width,device_number,i,minimum_separation,
		number_of_objective_values,objective_values_step,threshold_percentage,xpos,
		ypos;
	struct Analysis_work_area *analysis;
	struct Device **device,**highlight;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;

	ENTER(analysis_accept_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(analysis->trace))
	{
		if ((rig=analysis->rig)&&(highlight=analysis->highlight)&&(*highlight)&&
			(signal=(*highlight)->signal)&&(buffer=signal->buffer))
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
							XtVaGetValues(analysis->trace->area_1.enlarge.average_width,
								XmNvalue,&value_string,
								NULL);
							if (1==sscanf(value_string,"%d",&average_width))
							{
								if (average_width<0)
								{
									average_width=0;
								}
							}
							else
							{
								average_width=analysis->average_width;
							}
							analysis->average_width=average_width;
							XtFree(value_string);
							sprintf(global_temp_string,"%d",average_width);
							XtVaSetValues(analysis->trace->area_1.enlarge.average_width,
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
#if defined (OLD_CODE)
					calculate_device_event_markers(*highlight,
						analysis->start_search_interval,analysis->end_search_interval,
						analysis->detection,analysis->objective,analysis->number_of_events,
						threshold_percentage,minimum_separation,level,average_width);
#endif /* defined (OLD_CODE) */
					if ((analysis->trace)&&(analysis->trace->processed_device)&&
						(analysis->trace->processed_device->signal)&&
						(analysis->trace->processed_device->signal->buffer)&&
						(objective_values=analysis->trace->processed_device->signal->
						buffer->signals.float_values)&&(0<(objective_values_step=analysis->
						trace->processed_device->signal->buffer->number_of_signals))&&
						(0<(number_of_objective_values=analysis->trace->processed_device->
						signal->buffer->number_of_samples)))
					{
						calculate_device_objective(*highlight,analysis->detection,
							analysis->objective,objective_values,number_of_objective_values,
							objective_values_step,average_width);
						calculate_device_event_markers(*highlight,
							analysis->start_search_interval,analysis->end_search_interval,
							analysis->detection,objective_values,number_of_objective_values,
							objective_values_step,analysis->number_of_events,
							threshold_percentage,minimum_separation,level);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"analysis_accept_signal.  Invalid processed_device");
					}
					/* draw the events */
					if (analysis->window)
					{
						signals= &(analysis->window->signals);
						/* determine the number of the highlighted signal */
						device=rig->devices;
						current_region=rig->current_region;
						device_number=0;
						for (i=highlight-device;i>0;i--)
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
						draw_device_markers(*highlight,buffer->start,buffer->end,
							analysis->datum,1,analysis->potential_time,1,SIGNAL_AREA_DETAIL,0,
							xpos+signals->axes_left,ypos+signals->axes_top,
							signals->axes_width,signals->axes_height,
							XtWindow(signals->drawing_area),signals->drawing->pixel_map,
							analysis->signal_drawing_information,analysis->user_interface);
					}
				}
				/* change the signal status */
				signal->status=ACCEPTED;
				/* redraw the signal */
				update_interval_drawing_area(analysis->window);
				trace_change_signal_status(analysis->trace);
				if ((mapping=analysis->mapping_window)&&(mapping->map)&&
					(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_option))
				{
					update_mapping_drawing_area(mapping,0);
				}
			}
		}
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
LAST MODIFIED : 24 December 1996

DESCRIPTION :
==============================================================================*/
{
	int device_number,i,xpos,ypos;
	struct Analysis_work_area *analysis;
	struct Device **device,**highlight;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;

	ENTER(analysis_reject_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((rig=analysis->rig)&&(highlight=analysis->highlight)&&(*highlight)&&
			(signal=(*highlight)->signal)&&(buffer=signal->buffer))
		{
			if (signal->status!=REJECTED)
			{
				if (analysis->calculate_events)
				{
					/* clear the event markers */
					if (analysis->window)
					{
						signals= &(analysis->window->signals);
						/* determine the number of the highlighted signal */
						device=rig->devices;
						current_region=rig->current_region;
						device_number=0;
						for (i=highlight-device;i>0;i--)
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
						draw_device_markers(*highlight,buffer->start,buffer->end,
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
				/* redraw the signal */
				/* the signals drawing area does not need doing because it is
					highlighted */
				update_interval_drawing_area(analysis->window);
				trace_change_signal_status(analysis->trace);
				if ((mapping=analysis->mapping_window)&&(mapping->map)&&
					(SHOW_ELECTRODE_VALUES==mapping->map->electrodes_option))
				{
					update_mapping_drawing_area(mapping,0);
				}
			}
		}
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

DESCRIPTION :
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Device **highlight;

	ENTER(analysis_previous_signal);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((analysis->rig)&&(highlight=analysis->highlight))
		{
			if (ELECTRODE==(*highlight)->description->type)
			{
				/* accept the current signal if undecided */
				if (UNDECIDED==(*highlight)->signal->status)
				{
					analysis_accept_signal(widget,analysis_work_area,call_data);
				}
			}
			/* move to the previous signal */
			highlight--;
			highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,(int *)NULL,
				analysis);
		}
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
LAST MODIFIED : 24 December 1996

DESCRIPTION :
==============================================================================*/
{
	struct Analysis_work_area *analysis;
	struct Device **highlight;

	ENTER(analysis_next_signal);
	if (analysis=(struct Analysis_work_area *)analysis_work_area)
	{
		if ((analysis->rig)&&(highlight=analysis->highlight))
		{
			if (ELECTRODE==(*highlight)->description->type)
			{
				/* accept the current signal if undecided */
				if (UNDECIDED==(*highlight)->signal->status)
				{
					analysis_accept_signal(widget,analysis_work_area,call_data);
				}
			}
			/* move to the next signal */
			highlight++;
			highlight_analysis_device(0,highlight,(int *)NULL,(int *)NULL,(int *)NULL,
				analysis);
		}
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
LAST MODIFIED : 21 April 2000

DESCRIPTION :
Applies the current analysis mode settings to all signals.
==============================================================================*/
{
	float beat_width,*imaginary_value,*processed_value,*real_value,*value,
		value_end,value_start,*value_1,*value_2,x,y;
	int average_end,average_start,average_width,*beat_count,*beat_counts,beat_end,
		beat_number,beat_start,buffer_offset_1,buffer_offset_2,*divisions,end,
		high_pass,i,j,low_pass,notch,max_times,number_of_beats,number_of_samples,
		*processed_time,start,start_time,*time,transform_buffer_offset,
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
		switch (trace->analysis_mode)
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
						(*device)->signal_maximum=0;
						(*device)->signal_minimum=1;
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
#if defined (OLD_CODE)
						/* update the times */
						if ((0<start)||(0<(buffer->times)[start]))
						{
							processed_time=buffer->times;
							time=processed_time+start;
							/* averaged beat starts from time 0 */
							start_time= *time;
							for (i=end-start;i>=0;i--)
							{
								*processed_time=(*time)-start_time;
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
								beat_end=beat_start+(end-start)/number_of_beats;
								if (0<beat_start)
								{
									processed_value=(buffer->signals).float_values+
										((*device)->signal->index);
									value=processed_value+(beat_start*buffer_offset_1);
#if defined (OLD_CODE)
									processed_time=buffer->times;
									time=processed_time+beat_start;
#endif /* defined (OLD_CODE) */
									for (i=beat_end-beat_start;i>0;i--)
									{
										*processed_value= *value;
#if defined (OLD_CODE)
										*processed_time= *time;
										processed_time++;
										time++;
#endif /* defined (OLD_CODE) */
										processed_value += buffer_offset_1;
										value += buffer_offset_1;
									}
									*processed_time= *time;
								}
								for (beat_number=1;beat_number<number_of_beats;beat_number++)
								{
									beat_start=start+(int)((float)((end-start)*beat_number)/
										(float)number_of_beats+0.5);
									beat_end=beat_start+(end-start)/number_of_beats;
									processed_value=(buffer->signals).float_values+
										((*device)->signal->index);
									value=processed_value+(beat_start*buffer_offset_1);
									for (i=beat_end-beat_start;i>0;i--)
									{
										*processed_value += *value;
										processed_value += buffer_offset_1;
										value += buffer_offset_1;
									}
								}
								processed_value=(buffer->signals).float_values+
									((*device)->signal->index);
								for (i=beat_end-beat_start;i>0;i--)
								{
									*processed_value /= (float)number_of_beats;
									processed_value += buffer_offset_1;
								}
							}
							device++;
						}
						buffer->start=0;
						buffer->end=beat_end-beat_start-1;
						buffer->number_of_samples=beat_end-beat_start;
#endif /* defined (OLD_CODE) */
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
						map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
						(*device)->signal_maximum=0;
						(*device)->signal_minimum=1;
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
								if ((0<=notch)&&(notch<(transform_buffer->number_of_samples)-1))
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
								imaginary_value += (low_pass-high_pass)*transform_buffer_offset;
								for (j=(transform_buffer->number_of_samples)-low_pass-1;j>0;j--)
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
								/* set the gain and offset */
								(*device)->channel->gain=trace->processed_device->channel->gain;
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
						device++;
					}
					buffer->start=0;
					buffer->end=transform_number_of_samples-1;
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
						map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
						(*device)->signal_maximum=0;
						(*device)->signal_minimum=1;
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
						map->electrodes_option=SHOW_ELECTRODE_NAMES;
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
			display=user_interface->display;
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
	enum Time_keeper_play_direction play_direction, void *analysis_void)
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
					if (NO_INTERPOLATION!=map->interpolation_type)
					{
						if (map->frame_end_time>map->frame_start_time)
						{
							/* valid frames */
							switch (play_direction)
							{
								case TIME_KEEPER_PLAY_FORWARD:
								{
									next_time=map->frame_start_time+
										(map->frame_end_time-map->frame_start_time)/
										(float)(map->number_of_frames-1)*
										floor((float)(map->number_of_frames-1)*
										((time_after-map->frame_start_time)/
										(map->frame_end_time-map->frame_start_time))+1.0);
									time_set=1;
								} break;
								case TIME_KEEPER_PLAY_BACKWARD:
								{
									next_time=map->frame_start_time+
										(map->frame_end_time - map->frame_start_time)/
										(float)(map->number_of_frames-1)*
										ceil((float)(map->number_of_frames-1)*
										((time_after-map->frame_start_time)/
										(map->frame_end_time-map->frame_start_time))-1.0);
									time_set=1;
								} break;
							}
						}
					}
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
							next_time=(float)buffer->times[analysis->datum]*1000.0/
								buffer->frequency+map->minimum_value+
								(map->maximum_value-map->minimum_value)/
								(float)(number_of_spectrum_colours-1)*
								floor(1.0001+(float)(number_of_spectrum_colours-1)*
								(time_after-(float)buffer->times[analysis->datum]*1000.0/
								buffer->frequency-map->minimum_value)/
								(map->maximum_value-map->minimum_value));
							time_set=1;
						} break;
						case TIME_KEEPER_PLAY_BACKWARD:
						{
							next_time=(float)buffer->times[analysis->datum]*1000.0/
								buffer->frequency+map->minimum_value+
								(map->maximum_value-map->minimum_value)/
								(float)(number_of_spectrum_colours-1)*
								ceil(-1.0001+(float)(number_of_spectrum_colours-1)*
								(time_after-(float)buffer->times[analysis->datum]*1000.0/
								buffer->frequency-map->minimum_value)/
								(map->maximum_value-map->minimum_value));
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
					next_time=(1.0+floor(time_after*buffer->frequency/1000.0))*1000.0/
						buffer->frequency;
				} break;
				case TIME_KEEPER_PLAY_BACKWARD:
				{
					next_time=(-1.0+ceil(time_after*buffer->frequency/1000.0))*1000.0/
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
		printf("analysis_potential_time_next_time_callback. next_time %f\n",
			next_time);
#endif /* defined (DEBUG) */
	}
	LEAVE;

	return (next_time);
} /* analysis_potential_time_next_time_callback */

static int analysis_potential_time_update_callback(
	struct Time_object *time_object,double current_time,void *analysis_void)
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
Responds to update callbacks from the time object.
???DB.  Assume current_time is in milli-seconds
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	enum Interpolation_type interpolation;
	float contour_maximum,contour_minimum, frequency, maximum_value,minimum_value,
		map_potential_time, number_of_spectrum_colours;
	int cell_number, datum, frame_number, i, number_of_contours,
		potential_time, previous_potential_time, return_code;
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
		potential_time=current_time*frequency/1000.0;
		if (potential_time<0)
		{
			potential_time=0;
		}
		if (potential_time>=buffer->number_of_samples)
		{
			potential_time=buffer->number_of_samples-1;
		}
#if defined (DEBUG)
		printf("analysis_potential_time_update_callback.  initial %d (%d)\n",
			potential_time,buffer->times[potential_time]);
#endif /* defined (DEBUG) */
		if (((float)buffer->times[potential_time]<
			(current_time-1.0)*frequency/1000.0))
		{
			while ((potential_time<buffer->number_of_samples)&&
				((float)buffer->times[potential_time]<
				(current_time-1.0)*frequency/1000.0))
			{
				potential_time++;
			}
		}
		else
		{
			if ((float)buffer->times[potential_time]>current_time*frequency/1000.0)
			{
				while ((potential_time>=0)&&
					((float)buffer->times[potential_time]>current_time*frequency/1000.0))
				{
					potential_time--;
				}
			}
		}
#if defined (DEBUG)
		printf("                                          final   %d (%d)\n",
			potential_time,buffer->times[potential_time]);
#endif /* defined (DEBUG) */
		if (potential_time<0)
		{
#if defined (OLD_CODE)
			Time_keeper_request_new_time(
				Time_object_get_time_keeper(analysis->potential_time_object),
				(float)buffer->times[buffer->number_of_samples - 1]*1000.0/frequency);
#endif /* defined (OLD_CODE) */
			display_message(ERROR_MESSAGE,"analysis_potential_time_update_callback.  "
				"Potential time less than minimum");
		}
		else
		{
			if (potential_time>=buffer->number_of_samples)
			{
#if defined (OLD_CODE)
				Time_keeper_request_new_time(
					Time_object_get_time_keeper(analysis->potential_time_object),
					(float)buffer->times[0]*1000.0/frequency);
#endif /* defined (OLD_CODE) */
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
					switch (analysis->map_type)
					{
						case POTENTIAL:
						{
							map_potential_time=current_time;
							if (NO_INTERPOLATION!=map->interpolation_type)
							{
								if (-1!=map->activation_front)
								{
									/* playing movie */
									/* ??JW fix the range when playing the movie? IF so need */
									/* to update map dialog as well as map->fixed_range=1 */
#if defined (UNEMAP_USE_NODES)
									/* 3d map */
									map->frame_start_time=map_potential_time;
									map->frame_end_time=map_potential_time;
									map->frame_number=0;
									/* recalculate not used for 3d maps */
									update_mapping_drawing_area(mapping,1/*recalculate*/);
									update_mapping_colour_or_auxili(mapping);
#else /* defined (UNEMAP_USE_NODES) */
									/* 2d map */
									if (map->frame_end_time>map->frame_start_time)
									{
										frame_number=(int)((float)(map->number_of_frames-1)*
											((map_potential_time-map->frame_start_time)/
											(map->frame_end_time-map->frame_start_time)));
										if ((frame_number>=0)&&(frame_number<map->number_of_frames))
										{
											map->frame_number=frame_number;
											update_mapping_drawing_area(mapping,0);
											update_mapping_colour_or_auxili(mapping);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"analysis_potential_time_update_callback.  "
												"Wrong time for an animated_sequence");
										}
										return_code=1;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"analysis_potential_time_update_callback.  "
											"End time greater or equal to start time");
										return_code=0;
									}
#endif /* defined (UNEMAP_USE_NODES) */
								}
								else
								{
									if ((map_potential_time>=map->frame_start_time)&&
										(map_potential_time<=map->frame_end_time))
									{
										if (map->frame_start_time<map->frame_end_time)
										{
											frame_number=(int)((float)(map->number_of_frames-1)*
												((map_potential_time-map->frame_start_time)/
												(map->frame_end_time-map->frame_start_time)));
										}
										else
										{
											frame_number=0;
										}
										map->frame_number=frame_number;
										update_mapping_drawing_area(mapping,0);
										update_mapping_colour_or_auxili(mapping);
									}
									else
									{
#if defined (UNEMAP_USE_NODES)
										/* 3d map */
										map->frame_start_time=map_potential_time;
										map->frame_end_time=map_potential_time;
										map->frame_number=0;
										/* recalculate not used for 3d maps */
										update_mapping_drawing_area(mapping,1/*recalculate*/);
										update_mapping_colour_or_auxili(mapping);
#else /* defined (UNEMAP_USE_NODES) */
										/* 2d map */
										interpolation=map->interpolation_type;
										map->interpolation_type=NO_INTERPOLATION;
										update_mapping_drawing_area(mapping,1);
										update_mapping_colour_or_auxili(mapping);
										map->interpolation_type=interpolation;
#endif /* defined (UNEMAP_USE_NODES) */
									}
								}
							}
							else
							{
#if defined (UNEMAP_USE_NODES)
								/* for 3d map, with NO_INTERPOLATION need the frame_start_time
									to get the signal min,max */
								map->frame_start_time=map_potential_time;
								map->frame_end_time=map_potential_time;
								map->frame_number=0;
#endif /* defined (UNEMAP_USE_NODES) */
								update_mapping_drawing_area(mapping,1);
								update_mapping_colour_or_auxili(mapping);
							}
#if defined (OLD_CODE)
								map->frame_start_time=map_potential_time;
								map->frame_end_time=map_potential_time;
								if (map->number_of_frames>1)
								{
									/* deallocate all the frames except one */
									i=map->number_of_frames;
									frame=map->frames+1;
									while (i>1)
									{
										DEALLOCATE(frame->contour_x);
										DEALLOCATE(frame->contour_y);
										DEALLOCATE(frame->pixel_values);
										DEALLOCATE(frame->image->data);
										XFree((char *)(frame->image));
										frame++;
										i--;
									}
									if (REALLOCATE(frame,map->frames,struct Map_frame,1))
									{
										map->frames=frame;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"analysis_potential_time_update_callback.  "
											"Could not reallocate frame array");
										return_code=0;
									}
									map->number_of_frames=1;
								}
								map->frame_number=0;
#endif /* defined (OLD_CODE) */
						} break;
						case SINGLE_ACTIVATION:
						{
							number_of_spectrum_colours=
								drawing_information->number_of_spectrum_colours;
							map->activation_front=(float)(number_of_spectrum_colours-1)*
								(current_time-(float)buffer->times[analysis->datum]*1000.0/
								frequency-map->minimum_value)/(map->maximum_value-
								map->minimum_value);
#if defined (DEBUG)
							printf("analysis_potential_time_update_callback.  front %d current %f  datum %f  minimum %f maximum %f\n",
								map->activation_front,current_time,
								(float)buffer->times[analysis->datum]*1000.0/frequency,
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
									display=drawing_information->user_interface->display;
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
									colour.pixel=drawing_information->boundary_colour;
									colour.flags=DoRed|DoGreen|DoBlue;
									XStoreColor(display,colour_map,&colour);
								}
							}
							else
							{
								if (Time_keeper_is_playing(
									Time_object_get_time_keeper(analysis->potential_time_object)))
								{
#if defined (OLD_CODE)
									if (TIME_KEEPER_PLAY_FORWARD==Time_keeper_get_play_direction(
										Time_object_get_time_keeper(analysis->
										potential_time_object)))
									{
										Time_keeper_request_new_time(Time_object_get_time_keeper(
											analysis->potential_time_object),map->minimum_value+
											(float)buffer->times[analysis->datum]*1000.0/frequency);
									}
									else
									{
										Time_keeper_request_new_time(Time_object_get_time_keeper(
											analysis->potential_time_object),map->maximum_value+
											(float)buffer->times[analysis->datum]*1000.0/frequency);
									}
#endif /* defined (OLD_CODE) */
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
							analysis->datum=(float)potential_time-
								(float)(analysis->start_search_interval);
							update_mapping_drawing_area(mapping,2);
							update_mapping_colour_or_auxili(mapping);
							analysis->datum=datum;
						} break;
					}
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
LAST MODIFIED : 28 December 1999

DESCRIPTION :
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
				map->activation_front= -1;
				analysis_potential_time_update_callback(analysis->potential_time_object,
					Time_object_get_current_time(analysis->potential_time_object),
					analysis_void);
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

/*
Global functions
----------------
*/
int highlight_analysis_device(unsigned int multiple_selection,
	struct Device **device,int *device_number,int *electrode_number,
	int *auxiliary_number,struct Analysis_work_area *analysis)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

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
==============================================================================*/
{
	enum Device_type device_type;
	int i,new_auxiliary_number,new_device_number,new_electrode_number,
		old_auxiliary_number,old_device_number,old_electrode_number,return_code,
			start_analysis_interval,end_analysis_interval;
	struct Device **new_highlight,**old_highlight,**temp_device;
	struct Device_description *description;
	struct Interval_area *interval;
	struct Map *map;
	struct Mapping_window *mapping;
	struct Region *current_region;
	struct Signal_buffer *buffer;
	struct Signals_area *signals;

	ENTER(highlight_analysis_device);
	if (analysis&&(analysis->rig)&&(analysis->rig->devices)&&
		(*(analysis->rig->devices))&&
		(buffer=get_Device_signal_buffer(*(analysis->rig->devices)))&&
		(device||device_number||electrode_number||auxiliary_number))
	{
		current_region=analysis->rig->current_region;
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
		if (new_highlight=device)
		{
			/* have specified a device */
			/* determine the device, electrode and auxiliary numbers */
			temp_device=analysis->rig->devices;
			new_device_number=0;
			new_electrode_number=0;
			new_auxiliary_number=0;
			for (i=new_highlight-temp_device;i>0;i--)
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
						"highlight_analysis_device.  Invalid device type in rig");
					return_code=0;
				}
			}
			if (return_code&&
				((device_number&&!(*device_number==new_device_number))||
				(electrode_number&&!(*electrode_number==new_electrode_number))||
				(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
			{
				display_message(ERROR_MESSAGE,
					"highlight_analysis_device.  device item %s",
					"incompatible with device, electrode and auxiliary numbers");
				return_code=0;
			}
		}
		else
		{
			/* have not specified the device */
			if (signals&&device_number&&((new_device_number= *device_number)>=0)&&
				(new_device_number<signals->number_of_signals))
			{
				/* have specified a device_number */
				/* determine the device and the electrode and auxiliary numbers */
				new_highlight=analysis->rig->devices;
				new_electrode_number=0;
				new_auxiliary_number=0;
				i=new_device_number;
				while (((current_region!=(description=(*new_highlight)->description)->
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
					new_highlight++;
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
							"highlight_analysis_device.  Invalid device type in rig");
						return_code=0;
					}
				}
				if (return_code&&
					((electrode_number&&!(*electrode_number==new_electrode_number))||
					(auxiliary_number&&!(*auxiliary_number==new_auxiliary_number))))
				{
					display_message(ERROR_MESSAGE,"highlight_analysis_device.  %s",
						"device number incompatible with electrode and auxiliary numbers");
					return_code=0;
				}
			}
			else
			{
				if (mapping&&map)
				{
					if (electrode_number&&((new_electrode_number= *electrode_number)>=0)&&
						(new_electrode_number<map->number_of_electrodes))
					{
						/* have specified an electrode number */
						/* determine the device and the device and auxiliary numbers */
						new_highlight=analysis->rig->devices;
						new_device_number=0;
						new_auxiliary_number= -1;
						i=new_electrode_number;
						while ((ELECTRODE!=(device_type=(description=(*new_highlight)->
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
							new_highlight++;
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
							new_highlight=analysis->rig->devices;
							new_device_number=0;
							new_electrode_number= -1;
							i=new_auxiliary_number;
							while ((AUXILIARY!=(device_type=(description=(*new_highlight)->
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
								new_highlight++;
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
		}
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
						highlight_signal(*new_highlight,new_device_number,
							start_analysis_interval,end_analysis_interval,analysis->datum,
							analysis->potential_time,signals,
							analysis->signal_drawing_information,analysis->user_interface);
						highlight_electrode_or_auxiliar(*new_highlight,new_electrode_number,
							new_auxiliary_number,map,mapping);
					}
				}
				else
				{
 					/* highlight it and make it THE highlighted device for the analysis
						work area */
					analysis->highlight=new_highlight;
					(*new_highlight)->highlight=1;
					highlight_signal(*new_highlight,new_device_number,
						start_analysis_interval,end_analysis_interval,analysis->datum,
						analysis->potential_time,signals,
						analysis->signal_drawing_information,analysis->user_interface);
					if (interval)
					{
						update_interval_drawing_area(analysis->window);
					}
					/* update the trace window */
					trace_change_signal(analysis->trace);
					highlight_electrode_or_auxiliar(*new_highlight,new_electrode_number,
						new_auxiliary_number,map,mapping);
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
								highlight_signal(*old_highlight,old_device_number,
									start_analysis_interval,end_analysis_interval,analysis->datum,
									analysis->potential_time,signals,
									analysis->signal_drawing_information,
									analysis->user_interface);
								highlight_electrode_or_auxiliar(*old_highlight,
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
									highlight_signal(*old_highlight,old_device_number,
										start_analysis_interval,end_analysis_interval,
										analysis->datum,analysis->potential_time,signals,
										analysis->signal_drawing_information,
										analysis->user_interface);
									highlight_electrode_or_auxiliar(*old_highlight,-1,
										old_auxiliary_number,map,mapping);
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
					highlight_signal(*new_highlight,new_device_number,
						start_analysis_interval,end_analysis_interval,analysis->datum,
						analysis->potential_time,signals,
						analysis->signal_drawing_information,analysis->user_interface);
					if (interval)
					{
						update_interval_drawing_area(analysis->window);
					}
					/* update the trace window */
					trace_change_signal(analysis->trace);
					highlight_electrode_or_auxiliar(*new_highlight,new_electrode_number,
						new_auxiliary_number,map,mapping);
				}
			}
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
					region_item=rig->region_list;
					while (region_item&&(selected_region!= *region))
					{
						region_item=region_item->next;
						region++;
					}
					if (region_item)
					{
						found=1;
						/* update the analysis rig */
						current_region=region_item->region;
					}
				}
				if (found)
				{
					rig->current_region=current_region;
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
LAST MODIFIED : 26 December 1996

DESCRIPTION :
The callback for selecting a device in the analysis work area (signals drawing
area, mapping drawing area, colour bar or auxiliary devices drawing area).
==============================================================================*/
{
	Boolean owner_events;
	char value_string[11];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float x_scale;
	GC graphics_context;
	int ascent,auxiliary_number,*auxiliary_x,*auxiliary_y,colour_bar_left,
		colour_bar_right,descent,direction,keyboard_mode,marker_bottom,marker_top,
		pointer_mode,pointer_sensitivity,pointer_x,pointer_y,previous_spectrum,
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
	Window confine_to,working_window;

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
						display=analysis->user_interface->display;
						widget_spacing=analysis->user_interface->widget_spacing;
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
										/* grab the pointer */
										owner_events=True;
										pointer_mode=GrabModeAsync;
										keyboard_mode=GrabModeAsync;
										confine_to=None;
										if (GrabSuccess==XtGrabPointer(
											mapping->colour_or_auxiliary_drawing_area,owner_events,
											ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
											pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
										{
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
#if defined (OLD_CODE)
																			map->contour_minimum=
																				SCALE_X(spectrum,colour_bar_left,
																				map->minimum_value,1/x_scale);
																			spectrum_left=
																				SCALE_X(map->contour_minimum,
																				map->minimum_value,colour_bar_left,
																				x_scale);
																			if (spectrum_left!=spectrum)
																			{
																				XDrawLine(display,working_window,
																					graphics_context,spectrum,marker_top,
																					spectrum,marker_bottom);
																				XDrawLine(display,working_window,
																					graphics_context,spectrum_left,
																					marker_top,spectrum_left,
																					marker_bottom);
																				spectrum=spectrum_left;
																			}
#endif
																		} break;
																		case MOVING_RIGHT:
																		{
																			map->contour_maximum=(map->minimum_value)+
																				(float)(spectrum-colour_bar_left)/
																				x_scale;
#if defined (OLD_CODE)
																			map->contour_maximum=
																				SCALE_X(spectrum,colour_bar_left,
																				map->minimum_value,1/x_scale);
																			spectrum_right=
																				SCALE_X(map->contour_maximum,
																				map->minimum_value,colour_bar_left,
																				x_scale);
																			if (spectrum_right!=spectrum)
																			{
																				XDrawLine(display,working_window,
																					graphics_context,spectrum,marker_top,
																					spectrum,marker_bottom);
																				XDrawLine(display,working_window,
																					graphics_context,spectrum_right,
																					marker_top,spectrum_right,
																					marker_bottom);
																				spectrum=spectrum_right;
																			}
#endif
																		} break;
																	}
#if defined (OLD_CODE)
																	if ((HIDE_COLOUR==map->colour_option)&&
																		(SHOW_CONTOURS==map->contours_option))
																	{
#endif
																		update_mapping_drawing_area(mapping,0);
#if defined (OLD_CODE)
																	}
																	else
																	{
																		update_colour_map(map);
																	}
#endif
																}
															}
#if defined (OLD_CODE)
															else
															{
																XDrawLine(display,working_window,
																	graphics_context,spectrum,marker_top,spectrum,
																	marker_bottom);
																spectrum=spectrum_initial;
																XDrawLine(display,working_window,
																	graphics_context,spectrum,marker_top,spectrum,
																	marker_bottom);
															}
#endif
															update_mapping_colour_or_auxili(mapping);
															/* redraw the marker */
#if defined (OLD_CODE)
															switch (moving)
															{
																case MOVING_LEFT:
																{
																	/* write the minimum value */
																	sprintf(value_string,"%.4g",
																		map->contour_minimum);
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
																	sprintf(value_string,"%.4g",
																		map->contour_maximum);
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
																(map->drawing_information).graphics_context.
																spectrum_text_colour,text_x,text_y,
																value_string,string_length);
															XDrawString(display,working_window,
																(map->drawing_information).graphics_context.
																spectrum_text_colour,text_x,text_y,
																value_string,string_length);
															marker_top=(map->colour_bar_top)-widget_spacing;
															marker_bottom=map->colour_bar_bottom;
															XDrawLine(display,drawing->pixel_map,
																graphics_context,spectrum,marker_top,spectrum,
																marker_bottom);
#endif
															moving=MOVING_NONE;
														}
													} break;
													default:
													{
														XtDispatchEvent(&xevent);
													}
												}
											}
											/* release the pointer */
											XtUngrabPointer(
												mapping->colour_or_auxiliary_drawing_area,CurrentTime);
										}
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
									highlight_analysis_device((button_event->state)&ControlMask,
										(struct Device **)NULL,(int *)NULL,(int *)NULL,
										&auxiliary_number,analysis);
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
LAST MODIFIED : 24 December 1996

DESCRIPTION :
The callback for selecting a device in the analysis work area (signals drawing
area, mapping drawing area, colour bar or auxiliary devices drawing area).
==============================================================================*/
{
	char *electrode_drawn;
	int electrode_number,*electrode_x,*electrode_y,pointer_sensitivity;
	struct Analysis_work_area *analysis;
	struct Map *map;
	struct Mapping_window *mapping;
	XButtonEvent *event;
	XmDrawingAreaCallbackStruct *callback;

	ENTER(analysis_select_map_drawing_are);
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
							pointer_sensitivity=analysis->pointer_sensitivity;
							/* determine the electrode number */
							electrode_number=0;
							electrode_x=map->electrode_x;
							electrode_y=map->electrode_y;
							electrode_drawn=map->electrode_drawn;
							while ((electrode_number<map->number_of_electrodes)&&
								((!(*electrode_drawn))||
								(event->x<*electrode_x-pointer_sensitivity)||
								(event->x>*electrode_x+pointer_sensitivity)||
								(event->y<*electrode_y-pointer_sensitivity)||
								(event->y>*electrode_y+pointer_sensitivity)))
							{
								electrode_number++;
								electrode_x++;
								electrode_y++;
								electrode_drawn++;
							}
							highlight_analysis_device((event->state)&ControlMask,
								(struct Device **)NULL,(int *)NULL,&electrode_number,
								(int *)NULL,analysis);
						}
					}
				}
			}
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
LAST MODIFIED : 25 April 2000

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
		{"analysis_set_range",(XtPointer)analysis_set_range},
		{"analysis_previous_signal",(XtPointer)analysis_previous_signal},
		{"analysis_next_signal",(XtPointer)analysis_next_signal},
		{"analysis_accept_signal",(XtPointer)analysis_accept_signal},
		{"analysis_reject_signal",(XtPointer)analysis_reject_signal},
		{"trace_analysis_mode_apply",(XtPointer)trace_analysis_mode_apply},
		{"display_map_with_check",(XtPointer)display_map_with_check},
		{"set_analysis_order_event",(XtPointer)set_analysis_order_event},
		{"set_analysis_order_channel",(XtPointer)set_analysis_order_channel},
		{"select_analysis_interval",(XtPointer)select_analysis_interval},
		{"select_signals_drawing_area",(XtPointer)select_signals_drawing_area},
		{"calculate_all_event_markers",(XtPointer)calculate_all_event_markers},
		{"select_trace_1_drawing_area",(XtPointer)select_trace_1_drawing_area},
		{"set_detection_interval",(XtPointer)set_detection_interval},
		{"set_detection_level",(XtPointer)set_detection_level},
		{"set_detection_threshold",(XtPointer)set_detection_threshold},
		{"set_objective_absolute_slope",(XtPointer)set_objective_absolute_slope},
		{"set_objective_positive_slope",(XtPointer)set_objective_positive_slope},
		{"set_objective_negative_slope",(XtPointer)set_objective_negative_slope},
		{"set_objective_value",(XtPointer)set_objective_value},
		{"decrement_number_of_events",(XtPointer)decrement_number_of_events},
		{"increment_number_of_events",(XtPointer)increment_number_of_events},
		{"analysis_previous_event",(XtPointer)analysis_previous_event},
		{"analysis_next_event",(XtPointer)analysis_next_event},
		{"analysis_accept_event",(XtPointer)analysis_accept_event},
		{"analysis_reject_event",(XtPointer)analysis_reject_event},
		{"select_trace_3_drawing_area",(XtPointer)select_trace_3_drawing_area},
		{"close_analysis_work_area",(XtPointer)close_analysis_work_area}};
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

	ENTER(create_analysis_work_area);
	return_code=1;
	if (analysis&&user_interface
#if defined (UNEMAP_USE_NODES)
		&&package
#endif /* defined (UNEMAP_USE_NODES) */
		)
	{
		analysis->unemap_package=package;
		analysis->draw_package=(struct Draw_package *)NULL;
#if defined (UNEMAP_USE_NODES)
		analysis->rig_node_group=(struct GROUP(FE_node) *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
		analysis->activation=activation;
		analysis->map_type=NO_MAP_FIELD;
		analysis->user_interface=user_interface;
		analysis->pointer_sensitivity=pointer_sensitivity;
		analysis->postscript_file_extension=postscript_file_extension;
		analysis->configuration_file_extension=configuration_file_extension;
		analysis->identifying_colour=identifying_colour;
		analysis->map_drawing_information=map_drawing_information;
		analysis->signal_drawing_information=create_Signal_drawing_information(
			user_interface);
		analysis->analysis_update_flags=ANALYSIS_FLAGS_CLEAR;
		analysis->trace_update_flags=TRACE_FLAGS_CLEAR;
		analysis->potential_time_object=
			ACCESS(Time_object)(CREATE(Time_object)("UNEMAP Potential Time"));
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
			analysis->time_keeper = (struct Time_keeper *)NULL;
		}
		Time_object_add_callback(analysis->potential_time_object,
			analysis_potential_time_update_callback,(void *)analysis);
		/* the datum_time_object is internal and not controlled by a time_keeper */
		analysis->datum_time_object=
			ACCESS(Time_object)(CREATE(Time_object)("UNEMAP Datum Time"));
		Time_object_add_callback(analysis->datum_time_object,
			analysis_datum_time_update_callback,(void *)analysis);
		analysis->search_interval_divisions=(int *)NULL;
		/* retrieve the settings */
		XtVaGetApplicationResources(user_interface->application_shell,analysis,
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
				user_interface->screen_width,user_interface->screen_height,
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
				identifier_list[0].value=(XtPointer)analysis;
				identifier_list[1].value=(XtPointer)create_File_open_data(
					signal_file_extension_read,REGULAR,analysis_read_signal_file,
					(void *)analysis,0,user_interface);
				identifier_list[2].value=(XtPointer)create_File_open_data(
					analysis->events_file_extension,REGULAR,read_event_times_file,
					(XtPointer)analysis,0,user_interface);
				identifier_list[3].value=(XtPointer)create_File_open_data(".ele",
					REGULAR,analysis_read_bard_electrode_fi,(XtPointer)analysis,0,
					user_interface);
				identifier_list[4].value=(XtPointer)create_File_open_data(".eeg",
					REGULAR,analysis_read_beekeeper_eeg_fil,(XtPointer)analysis,0,
					user_interface);
				identifier_list[5].value=(XtPointer)create_File_open_data(
					configuration_file_extension,REGULAR,analysis_read_cardiomapp_electr,
					(XtPointer)analysis,0,user_interface);
				identifier_list[6].value=(XtPointer)create_File_open_data(
					configuration_file_extension,REGULAR,analysis_read_neurosoft_electro,
					(XtPointer)analysis,0,user_interface);
				identifier_list[7].value=(XtPointer)create_File_open_data(
					signal_file_extension_write,REGULAR,
					analysis_write_signal_file,(XtPointer)analysis,0,user_interface);
				identifier_list[8].value=(XtPointer)create_File_open_data(
					signal_file_extension_read,REGULAR,analysis_overlay_signal_file,
					(void *)analysis,0,user_interface);
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
							&(analysis->rig_node_group),&(analysis->draw_package),
							&(analysis->highlight),&(analysis->datum),
							&(analysis->event_number),&(analysis->number_of_events),
							&(analysis->potential_time),&(analysis->detection),
							&(analysis->threshold),&(analysis->minimum_separation),
							&(analysis->level),&(analysis->average_width),
							analysis->identifying_colour,analysis->signal_order,
							SEPARATE_LAYOUT,&(analysis->start_search_interval),
							&(analysis->search_interval_divisions),
							&(analysis->end_search_interval),user_interface->screen_height,
							postscript_file_extension,analysis->events_file_extension,
							analysis->signal_drawing_information,user_interface))
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
LAST MODIFIED : 3 January 2000

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
