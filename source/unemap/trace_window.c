/*******************************************************************************
FILE : trace_window.c

LAST MODIFIED : 23 July 2004

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/analysis.h"
#include "unemap/analysis_drawing.h"
#include "unemap/eimaging_time_dialog.h"
#include "unemap/drawing_2d.h"
#include "unemap/rig.h"
#include "unemap/spectral_methods.h"
#include "unemap/trace_window.h"
#if defined (MOTIF)
#include "unemap/trace_window.uidh"
#endif /* defined (MOTIF) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int trace_window_hierarchy_open=0;
static MrmHierarchy trace_window_hierarchy;
#endif /* defined (MOTIF) */

XmString auto_correlation_string,cross_correlation_string;

/*
Module functions
----------------
*/

static struct Cardiac_interval *create_Cardiac_interval(GC graphics_context,
	int peak_or_trough_time,int start_time,int end_time,
	enum Cardiac_interval_type type)
/*******************************************************************************
LAST MODIFIED : 23 March 2001

DESCRIPTION : create a cardiac Interval
==============================================================================*/
{
	struct Cardiac_interval *interval;

	ENTER(create_Cardiac_interval);
	if (ALLOCATE(interval,struct Cardiac_interval,1))
	{
		interval->type=type;
		interval->graphics_context=graphics_context;
		interval->peak_or_trough_time=peak_or_trough_time;
		interval->start_time=start_time;
		interval->end_time=end_time;
		interval->previous=(struct Cardiac_interval *)NULL;
		interval->next=(struct Cardiac_interval *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cardiac_interval.  Could not allocate memory");
	}
	LEAVE;
	return (interval);
} /* create_Cardiac_interval */

int print_Cardiac_interval_list(struct Cardiac_interval *first_list_interval)
/*******************************************************************************
LAST MODIFIED :  31 May 2001

DESCRIPTION : Debugging function.
==============================================================================*/
{
	int return_code;
	struct Cardiac_interval *interval;

	ENTER(print_Cardiac_interval_list);
	interval=(struct Cardiac_interval *)NULL;
	if (interval=first_list_interval)
	{
		return_code=1;
		while (interval)
		{
			printf("Cardiac interval: start_time = %d peak_or_trough_time = %d "
				"end_time= %d\n",interval->start_time,interval->peak_or_trough_time,
				interval->end_time);
			interval=interval->next;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"print_Cardiac_interval_list. no list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* print_Cardiac_interval_list */

static int add_Cardiac_interval_to_sorted_list(
	struct Cardiac_interval **first_list_interval,
	struct Cardiac_interval *new_interval)
/*******************************************************************************
LAST MODIFIED :  5 April 2001

DESCRIPTION : adds <new_interval> to the interval list <first_list_interval>,
inserting it so that the list is in order of interval->peak_or_trough_time
==============================================================================*/
{
	int placed,return_code;
	struct Cardiac_interval *interval,*next_interval;

	ENTER(add_Cardiac_interval_to_sorted_list);
	interval=(struct Cardiac_interval *)NULL;
	next_interval=(struct Cardiac_interval *)NULL;
	if (new_interval)
	{
		return_code=1;
		placed=0;
		if (interval=*first_list_interval)
		{
			/* find place to insert interval */
			/*at beginning of list?*/
			if (new_interval->peak_or_trough_time<interval->peak_or_trough_time)
			{
				new_interval->previous=(struct Cardiac_interval *)NULL;
				new_interval->next=interval;
				interval->previous=new_interval;
				*first_list_interval=new_interval;
				placed=1;
			}
			else
			{
				/*in middle or at end of list?*/
				do
				{
					if (new_interval->peak_or_trough_time>interval->peak_or_trough_time)
					{
						if (!interval->next)
						{
							/* add the new one on the end of list */
							interval->next=new_interval;
							new_interval->previous=interval;
							new_interval->next=(struct Cardiac_interval *)NULL;
							placed=1;
						}
						else
						{
							next_interval=interval->next;
							if (new_interval->peak_or_trough_time<next_interval->peak_or_trough_time)
							{
								/* insert in the middle of list */
								interval->next=new_interval;
								new_interval->previous=interval;
								new_interval->next=next_interval;
								next_interval->previous=new_interval;
								placed=1;
							}
						}
					}
				}while ((interval=interval->next)&&(!placed));
			}
		}
		else
		{
			/* first entry in the list */
			*first_list_interval=new_interval;
			new_interval->previous=(struct Cardiac_interval *)NULL;
			new_interval->next=(struct Cardiac_interval *)NULL;
			placed=1;
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Cardiac_interval_to_sorted_list. invalid arguments");
	}
	if (!placed)
	{
		return_code=0;
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Cardiac_interval_to_sorted_list. error adding to list");
	}
	LEAVE;
	
	return (return_code);
} /* add_Cardiac_interval_to_sorted_list */

int remove_Cardiac_interval_from_list(
	struct Cardiac_interval **first_list_interval,
	struct Cardiac_interval *interval_to_remove)
/*******************************************************************************
LAST MODIFIED :  14 June 2001

DESCRIPTION :
Remove the <interval_to_remove> from the list of intervals whose first interval
is <first_list_interval>.
==============================================================================*/
{
	int return_code,found;
	struct Cardiac_interval *interval,*next_interval,*prev_interval;

	ENTER(remove_Cardiac_interval_from_list);
	interval=(struct Cardiac_interval *)NULL;
	next_interval=(struct Cardiac_interval *)NULL;
	prev_interval=(struct Cardiac_interval *)NULL;
	if (interval_to_remove&&(first_list_interval)&&
		(interval=*first_list_interval))
	{
		return_code=0;
		found=0;
		/*find interval in list*/
		while (interval&&(!found))
		{
			if (interval_to_remove==interval)
			{
				found=1;
			}
			interval=interval->next;
		}
		if (!found)
		{
			/* should this be an error? I think so.*/
			return_code=0;
			display_message(ERROR_MESSAGE,
				"remove_Cardiac_interval_from_list. interval not in list ");
		}
		else
		{
			return_code=1;
			/* remove interval from list */
			prev_interval=interval_to_remove->previous;
			next_interval=interval_to_remove->next;
			if (prev_interval==NULL)
			{
				/* first interval in list*/
				if (next_interval)
				{
					next_interval->previous=(struct Cardiac_interval *)NULL;
				}
				*first_list_interval=next_interval;
			}
			else if (next_interval==NULL)
			{
				/* last interval in list*/
				prev_interval->next=(struct Cardiac_interval *)NULL;
			}
			else
			{
				/* in the middle of the list */
				prev_interval->next=next_interval;
				next_interval->previous=prev_interval;
			}
			DEALLOCATE(interval_to_remove);
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"remove_Cardiac_interval_from_list. invalid arguments");
	}
	LEAVE;
	
	return (return_code);
} /* remove_Cardiac_interval_from_list */

int add_Cardiac_interval_to_unsorted_list(
	struct Cardiac_interval **first_list_interval,
	struct Cardiac_interval *new_interval)
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :adds <new_interval> to the end of the interval list
<first_list_interval>. See also add_Cardiac_interval_to_sorted_list
==============================================================================*/
{
	int return_code;
	struct Cardiac_interval *interval;

	ENTER(add_Cardiac_interval_to_unsorted_list);
	interval=(struct Cardiac_interval *)NULL;
	if (new_interval)
	{
		return_code=1;
		if (interval=*first_list_interval)
		{
			/* find last interval*/
			while (interval->next)
			{
				interval=interval->next;
			}
			/* add the new one on the end */
			interval->next=new_interval;
			new_interval->previous=interval;
		}
		else
		{
			/* first entry in the list */
			*first_list_interval=new_interval;
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Cardiac_interval_to_unsorted_list. invalid arguments");
	}
	LEAVE;
	
	return (return_code);
} /* add_Cardiac_interval_to_unsorted_list */

static int destroy_Cardiac_interval_list(struct Cardiac_interval **first_interval)
/*******************************************************************************
LAST MODIFIED : 11 April 2001

DESCRIPTION :
This function frees the memory associated with the interval list starting at
<**first_interval> and sets <*first_interval> to NULL.
==============================================================================*/
{
	int return_code;
	struct Cardiac_interval *interval,*next_interval;

	ENTER(destroy_Cardiac_interval_list);
	return_code=1;
	if (first_interval&&(interval= *first_interval))
	{
		while (interval)
		{
			next_interval=interval->next;
			DEALLOCATE(interval);
			interval=next_interval;
		}
		*first_interval=(struct Cardiac_interval *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Cardiac_interval_list */

static void destroy_Trace_window(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
This function expects <trace_window> to be a pointer to an trace window.  If
the <address> field of the trace window is not NULL, <*address> is set to NULL.
If the <activation> field is not NULL, the <activation> widget is unghosted.
The function frees the memory associated with the fields of the trace window and
frees the memory associated with the trace window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(destroy_Trace_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (trace->first_interval)
		{
			destroy_Cardiac_interval_list(&trace->first_interval);
		}
		if (trace->first_eimaging_event)
		{
			destroy_Electrical_imaging_event_list(trace->first_eimaging_event);
		}
		/* set the pointer to the trace window to NULL */
		if (trace->address)
		{
			*(trace->address)=(struct Trace_window *)NULL;
		}
		/* unghost the activation button */
		if (trace->activation)
		{
			XtSetSensitive(trace->activation,True);
		}
		/* free the area 1 drawing */
		destroy_Drawing_2d(&(trace->area_1.drawing));
		/* free the area 2 drawing */
		destroy_Drawing_2d(&(trace->area_2.drawing));
		/* free the area 3 drawing */
		destroy_Drawing_2d(&(trace->area_3.drawing));
		/* free the trace window memory */
		DEALLOCATE(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Trace_window.  trace window missing");
	}
	LEAVE;
} /* destroy_Trace_window */


static int two_end_baseline(struct Signal_buffer *buffer,int number_of_samples,
	int average_width,int number_of_beats,int start,int end,int *divisions)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Perform a baseline on the <buffer> values between <start> and <end>.
If want to do whole signal, without regard to divisions or beats, use
<number_of_beats> =1, <divisions> = (int *)NULL;
==============================================================================*/
{
	int return_code;
	int average_end,average_start,beat_end,beat_number,beat_start,i;
	float *value,value_end,value_start,beat_width;

	ENTER(two_end_baseline);
	if (buffer)
	{
		return_code=1;
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
		value=((buffer->signals).float_values)+average_start;
		value_end=0;
		for (i=average_end-average_start;i>0;i--)
		{
			value_end += *value;
			value++;
		}
		value_end /= (float)(average_end-average_start);
		beat_end=beat_start-1;
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
			value=
				((buffer->signals).float_values)+average_start;
			value_end=0;
			for (i=average_end-average_start;i>0;i--)
			{
				value_end += *value;
				value++;
			}
			value_end /= (float)(average_end-average_start);
			value=((buffer->signals).float_values)+beat_start;
			beat_width=(float)(beat_end-beat_start);
			for (i=beat_end-beat_start;i>=0;i--)
			{
				*value -= ((beat_width-(float)i)*value_end+
					(float)i*value_start)/beat_width;
				value++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"two_end_baseline .invalid argument");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* two_end_baseline */

static void redraw_trace_2_drawing_area(Widget widget,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 2.
==============================================================================*/
{
	Display *display;
	int axes_height,axes_left,axes_top,axes_width,end_analysis_interval,height,
		redraw,start_analysis_interval,width;
	struct Device *device;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window *trace;
	struct Trace_window_area_2 *trace_area_2;
	struct User_interface *user_interface;
	XExposeEvent *expose_event;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(redraw_trace_2_drawing_area);
	USE_PARAMETER(widget);
	if ((trace=(struct Trace_window *)trace_window)&&
		(user_interface=trace->user_interface)&&
		(signal_drawing_information=trace->signal_drawing_information))
	{
		trace_area_2= &(trace->area_2);
		if (trace_area_2->drawing_area)
		{
			display=User_interface_get_display(user_interface);
			if (callback=(XmDrawingAreaCallbackStruct *)call_data)
			{
				if ((XmCR_EXPOSE==callback->reason)&&(callback->event)&&
					(Expose==callback->event->type))
				{
					expose_event= &(callback->event->xexpose);
					redraw=0;
				}
				else
				{
					if (XmCR_RESIZE==callback->reason)
					{
						/*??? during creation there are resize callbacks without windows */
						if (callback->window)
						{
							expose_event=(XExposeEvent *)NULL;
							redraw=0;
							/* find the size of the old rectangle */
							if (trace_area_2->drawing)
							{
								width=trace_area_2->drawing->width;
								height=trace_area_2->drawing->height;
								/* destroy the old pixmap */
								destroy_Drawing_2d(&(trace_area_2->drawing));
							}
							else
							{
								width=0;
								height=0;
							}
						}
						else
						{
							redraw= -1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_2_drawing_area.  Invalid call data");
						redraw= -1;
					}
				}
			}
			else
			{
				redraw=1;
			}
			if (redraw>=0)
			{
				if (!(trace_area_2->drawing))
				{
					/* determine the size of the drawing area */
					XGetWindowAttributes(display,XtWindow(trace_area_2->drawing_area),
						&attributes);
					/* create a pixel map */
					if (trace_area_2->drawing=create_Drawing_2d(
						trace_area_2->drawing_area,attributes.width,attributes.height,
						NO_DRAWING_IMAGE,trace->user_interface))
					{
						redraw=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_2_drawing_area.  Could not create drawing");
					}
				}
				if (trace_area_2->drawing)
				{
					if (redraw)
					{
						/* clear the drawing area */
						XFillRectangle(display,trace_area_2->drawing->pixel_map,
							(signal_drawing_information->graphics_context).
							background_drawing_colour,0,0,trace_area_2->drawing->width,
							trace_area_2->drawing->height);
						switch (*trace->analysis_mode)
						{
							case CROSS_CORRELATION:
							{
								if ((device=trace->correlation.device_2)&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									/* draw the active signal */
									draw_signal(
										(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,device,
										EDIT_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,0,trace_area_2->drawing->width,
										trace_area_2->drawing->height,
										trace_area_2->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_2->axes_left=axes_left;
									trace_area_2->axes_top=axes_top;
									trace_area_2->axes_width=axes_width;
									trace_area_2->axes_height=axes_height;
								}
							} break;
							case ELECTRICAL_IMAGING:
							{
								/* nothing to do here */
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
								"redraw_trace_2_drawing_area.  Not in cross correlation mode");
							} break;
						}
					}
					/* redisplay the specified part of the pixmap */
					if (callback)
					{
						if (expose_event)
						{
							/* expose callback */
							XCopyArea(display,trace_area_2->drawing->pixel_map,
								XtWindow(trace_area_2->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								expose_event->x,expose_event->y,expose_event->width,
								expose_event->height,expose_event->x,expose_event->y);
						}
						else
						{
							/* resize callback */
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							XCopyArea(display,trace_area_2->drawing->pixel_map,
								XtWindow(trace_area_2->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								0,0,width,height,0,0);
						}
					}
					else
					{
						/* update */
						XCopyArea(display,trace_area_2->drawing->pixel_map,
							XtWindow(trace_area_2->drawing_area),
							(signal_drawing_information->graphics_context).copy,0,0,
							trace_area_2->drawing->width,trace_area_2->drawing->height,0,0);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"redraw_trace_2_drawing_area.  Missing drawing area");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"redraw_trace_2_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* redraw_trace_2_drawing_area */

static void identify_event_detection_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace event detection button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_event_detection_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.event_detection_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_event_detection_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_event_detection_button */

static void reset_trace_window_after_cross_correlation(
	struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 5 March 2001

DESCRIPTION :
Reset dimensions of things in the trace window after it has been set to cross
correlation. This is necessary as  CROSS_CORRELATION (and ELECTRICAL_IMAGING)
displays trace->area_2.pane, but other trace->analysis_mode types don't.
==============================================================================*/
{
	Dimension paned_window_height;

	ENTER(reset_trace_window_after_cross_correlation);
	if (trace)
	{
		/* unmanage pane2 as it's not used */
		XtUnmanageChild(trace->area_2.pane);
		/* unmanage pane3 so it'll be resized when it's remanaged*/
		XtUnmanageChild(trace->area_3.pane);
		/*get height of whole paned window*/
		XtVaGetValues(trace->paned_window,XmNheight,&paned_window_height,NULL);
		/* manage and unmanage things that are/aren't used*/
		/* unmanage pane1 so can adjust it's height*/
		XtUnmanageChild(trace->area_1.pane);
		/* adust height to 36% of paned_window, as when created in
			create_Trace_window*/
		XtVaSetValues(trace->area_1.pane,
			XmNheight,paned_window_height*9/25,NULL);
		/* remanage panes*/
		XtManageChild(trace->area_3.pane);
		XtManageChild(trace->area_1.pane);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reset_trace_window_after_cross_correlation.  Missing trace_window");
	}
	LEAVE;
}/*reset_trace_window_after_cross_correlation */

static void reset_trace_window_after_eimaging(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 19 February 2001

DESCRIPTION : Reset things in the trace window after it has been
set to electrical imaging.
==============================================================================*/
{
	ENTER(reset_trace_window_after_eimaging);
	if (trace)
	{
		/* swap  interval menu for edit menu */
		XtUnmanageChild(trace->area_3.interval.menu);
		XtVaSetValues(trace->area_3.drawing_area,
			XmNtopWidget,trace->area_3.edit.menu,NULL);
		XtManageChild(trace->area_3.edit.menu);
		/*reattach top of drawing_area */
		XtVaSetValues(trace->area_1.drawing_area,
			XmNtopAttachment,XmATTACH_FORM,NULL);
		/* manage and unmanage things that are/aren't used*/
		XtUnmanageChild(trace->area_1.inverse.menu);
		XtUnmanageChild(trace->area_1.calculate.menu);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reset_trace_window_after_eimaging.  Missing trace_window");
	}
	LEAVE;
}/*reset_trace_window_after_eimaging */

static void set_analysis_event_detection(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
Sets the analysis mode to event detection.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_event_detection);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in event detection mode */
		if (EVENT_DETECTION!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.power_spectra.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set event detection mode */
			*trace->analysis_mode=EVENT_DETECTION;
			XtVaSetValues(trace->area_1.drawing_area,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopWidget,trace->area_1.enlarge.menu,
				NULL);
			XtManageChild(trace->area_1.enlarge.menu);
			XtVaSetValues(trace->area_3.drawing_area,
				XmNtopWidget,trace->area_3.edit.menu,
				NULL);
			XtManageChild(trace->area_3.edit.menu);
			/* go back to constant width divisions */
			DEALLOCATE(*(trace->event_detection.search_interval_divisions));
			DEALLOCATE(trace->area_1.enlarge.divisions);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_event_detection.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_event_detection */

static void identify_frequency_domain_butto(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace frequency domain button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_frequency_domain_butto);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.frequency_domain_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_frequency_domain_butto.  Missing trace_window");
	}
	LEAVE;
} /* identify_frequency_domain_butto */

static void set_analysis_frequency_domain(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
Sets the analysis mode to frequency domain.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_frequency_domain);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in frequency domain mode */
		if (FREQUENCY_DOMAIN!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtUnmanageChild(trace->area_3.edit.menu);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.edit.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.power_spectra.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set frequency domain mode */
			*trace->analysis_mode=FREQUENCY_DOMAIN;
			trace->valid_processing=0;
			XtVaSetValues(trace->area_3.drawing_area,
				XmNtopWidget,trace->area_3.frequency_domain.menu,
				NULL);
			XtManageChild(trace->area_3.frequency_domain.menu);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_frequency domain.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_frequency_domain */

static void identify_power_spectra_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace power spectra button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_power_spectra_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.power_spectra_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_power_spectra_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_power_spectra_button */

static void set_analysis_power_spectra(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
Sets the analysis mode to power spectra.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_power_spectra);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in power spectra mode */
		if (POWER_SPECTRA!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set power spectra mode */
			*trace->analysis_mode=POWER_SPECTRA;
			trace->valid_processing=0;
			XtVaSetValues(trace->area_3.drawing_area,
				XmNtopWidget,trace->area_3.power_spectra.menu,
				NULL);
			XtManageChild(trace->area_3.power_spectra.menu);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_power_spectra.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_power_spectra */

static void identify_cross_correlation_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace cross correlation button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_cross_correlation_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.cross_correlation_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_cross_correlation_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_cross_correlation_butt */

static void set_analysis_cross_correlation(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
Sets the analysis mode to cross correlation.
==============================================================================*/
{
	Dimension paned_window_height;
	struct Trace_window *trace;

	ENTER(set_analysis_cross_correlation);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in cross correlation mode */
		if (CROSS_CORRELATION!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.power_spectra.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.filtering.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopWidget,trace->area_1.correlation_time_domain.menu,
						NULL);
					XtManageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
			}
			/* set cross correlation mode */
			*trace->analysis_mode=CROSS_CORRELATION;
			trace->valid_processing=0;
			XtVaSetValues(trace->area_3.correlation.label,
				XmNlabelString,cross_correlation_string,
				NULL);
			/*get value to resize things from*/
			XtVaGetValues(trace->paned_window,XmNheight,&paned_window_height,NULL);
			/* unmanage areas 1 and 3 for resizing (2 already unmanaged)*/
			XtUnmanageChild(trace->area_3.pane);
			XtUnmanageChild(trace->area_1.pane);
			/*set the height of  panes */
			XtVaSetValues(trace->area_1.pane,
							XmNheight,paned_window_height*1/3,NULL);
			XtVaSetValues(trace->area_2.pane,
					XmNheight,paned_window_height*1/3,NULL);
			XtVaSetValues(trace->area_3.pane,
					XmNheight,paned_window_height*1/3,NULL);
			/* order of re managing is important!*/
			XtManageChild(trace->area_2.pane);
			XtManageChild(trace->area_1.pane);
			XtManageChild(trace->area_3.pane);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_2_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_cross_correlation.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_cross_correlation */

static void identify_eimaging_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 February 2001

DESCRIPTION :
Saves the id of the trace electrical imaging button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_eimaging_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.eimaging_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_eimaging_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_eimaging_butt */

static void set_analysis_eimaging(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 February 2001

DESCRIPTION :
Sets the analysis mode to electrical imaging.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_eimaging);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in electrical imaging mode */
		if (ELECTRICAL_IMAGING!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.power_spectra.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set electrical imaging mode */
			*trace->analysis_mode=ELECTRICAL_IMAGING;
			trace->valid_processing=0;
			XtManageChild(trace->area_1.inverse.menu);
			XtManageChild(trace->area_1.calculate.menu);
			XtVaSetValues(trace->area_1.drawing_area,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopWidget,trace->area_1.calculate.menu,NULL);
			/* swap edit menu for interval menu */
			XtUnmanageChild(trace->area_3.edit.menu);
			XtVaSetValues(trace->area_3.drawing_area,
				XmNtopWidget,trace->area_3.interval.menu,
				NULL);
			XtManageChild(trace->area_3.interval.menu);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
			trace->calculate_rms=1;
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_eimaging.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_eimaging */


static void identify_auto_correlation_butto(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace auto correlation button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_auto_correlation_butto);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.auto_correlation_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_auto_correlation_butto.  Missing trace_window");
	}
	LEAVE;
} /* identify_auto_correlation_butto */

static void set_analysis_auto_correlation(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
Sets the analysis mode to auto correlation.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_auto_correlation);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in cross correlation mode */
		if (AUTO_CORRELATION!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->area_3.power_spectra.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_3.filtering.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.analysis_mode_choice,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
			}
			/* set auto correlation mode */
			*trace->analysis_mode=AUTO_CORRELATION;
			trace->valid_processing=0;
			XtVaSetValues(trace->area_3.correlation.label,
				XmNlabelString,auto_correlation_string,
				NULL);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_auto_correlation.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_auto_correlation */

static void identify_filtering_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace filtering button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_filtering_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.filtering_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_filtering_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_filtering_button */

static void set_analysis_filtering(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Sets the analysis mode to filtering.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_filtering);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in filtering mode */
		if (FILTERING!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->area_3.power_spectra.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
				} break;
			}
			/* set filtering mode */
			*trace->analysis_mode=FILTERING;
			trace->valid_processing=0;
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_filtering.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_filtering */

static void identify_beat_averaging_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 August 1997

DESCRIPTION :
Saves the id of the trace beat averaging button in the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_beat_averaging_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode.beat_averaging_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_beat_averaging_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_beat_averaging_button */

static void set_analysis_beat_averaging(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Sets the analysis mode to beat averaging.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_beat_averaging);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in filtering mode */
		if (BEAT_AVERAGING!=*trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					reset_trace_window_after_eimaging(trace);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.edit.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case POWER_SPECTRA:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.power_spectra.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
				} break;
				case CROSS_CORRELATION:
				{
					reset_trace_window_after_cross_correlation(trace);
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case AUTO_CORRELATION:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
					XtVaSetValues(trace->menu.average_width,
						XmNleftWidget,trace->menu.apply_button,
						NULL);
				} break;
				case FILTERING:
				{
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_WIDGET,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.filtering.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set beat averaging mode */
			*trace->analysis_mode=BEAT_AVERAGING;
			trace_update_signal_controls(trace);
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_beat_averaging.  Missing trace_window");
	}
	LEAVE;
} /* set_analysis_beat_averaging */

static void identify_trace_electrodes_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the electrodes choice  menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_electrodes_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.electrodes_choice_mode= *widget_id;
		/* to match first entry in uil */
		trace->inverse_electrodes_mode=ELECTRODES_UNREJECTED;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_electrodes_ch.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_electrodes_ch */

static void identify_trace_waves_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_waves_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice_mode= *widget_id;
		trace->inverse_wave_mode=PQRST_WAVE; /* to match first entry in uil file*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_waves_ch.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_waves_ch */

static void identify_trace_pot_act_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the potential/activation  menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_pot_act_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.pot_act_choice_mode= *widget_id;
		/* to match first entry in uil file*/
		trace->inverse_pot_act_mode=INVERSE_POTENTIAL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_pot_act_ch.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_pot_act_ch */

static void id_electrodes_accepted_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace electrodes_accepted button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_electrodes_accepted_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.electrodes_choice.accepted_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_electrodes_accepted_button. Missing trace_window");
	}
	LEAVE;
} /*id_electrodes_accepted_button */

static void id_electrodes_unrejected_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace electrodes_unrejected button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_electrodes_unrejected_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.electrodes_choice.unrejected_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_electrodes_unrejected_button. Missing trace_window");
	}
	LEAVE;
} /* id_electrodes_unrejected_button */

static void id_electrodes_all_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace electrodes_all button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_electrodes_all_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.electrodes_choice.all_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_electrodes_all_button. Missing trace_window");
	}
	LEAVE;
} /* id_electrodes_all_button */

static void set_inv_electrodes_accepted(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse electrodes to accepted
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inv_electrodes_accepted);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already inACCEPTED  mode */
		if (ELECTRODES_ACCEPTED!=trace->inverse_electrodes_mode)
		{
			trace->inverse_electrodes_mode=ELECTRODES_ACCEPTED;
			trace->calculate_rms=1;
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inv_electrodes_accepted.  Missing trace_window");
	}
	LEAVE;
} /* set_inv_electrodes_accepted */

static void set_inv_electrodes_unrejected(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse electrodes to unrejected
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inv_electrodes_unrejected);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already inACCEPTED  mode */
		if (ELECTRODES_UNREJECTED!=trace->inverse_electrodes_mode)
		{
			trace->inverse_electrodes_mode=ELECTRODES_UNREJECTED;
			trace->calculate_rms=1;
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inv_electrodes_unrejected.  Missing trace_window");
	}
	LEAVE;
} /* set_inv_electrodes_unrejected */

static void set_inv_electrodes_all(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse electrodes to all
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inv_electrodes_all);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already inACCEPTED  mode */
		if (ELECTRODES_ALL!=trace->inverse_electrodes_mode)
		{
			trace->inverse_electrodes_mode=ELECTRODES_ALL;
			trace->calculate_rms=1;
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inv_electrodes_all.  Missing trace_window");
	}
	LEAVE;
} /* set_inv_electrodes_all */

static void identify_p_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace p_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_p_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.p_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_p_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_p_wave_button */

static void identify_qrs_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace qrs_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_qrs_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.qrs_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_qrs_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_qrs_wave_button */

static void identify_t_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace t_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_t_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.t_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_t_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_t_wave_button */

static void identify_pqrs_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace pqrs_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_pqrs_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.pqrs_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_pqrs_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_pqrs_wave_button */

static void identify_pt_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace pt_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_pt_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.pt_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_pt_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_pt_wave_button */

static void identify_qrst_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace qrst_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_qrst_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.qrst_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_qrst_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_qrst_wave_button */

static void identify_pqrst_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace pqrst_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_pqrst_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.pqrst_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_pqrst_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_pqrst_wave_button */

static void identify_none_wave_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace none_wave_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_none_wave_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.wave_choice.none_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_none_wave_button. Missing trace_window");
	}
	LEAVE;
} /*identify_none_wave_button */

static void set_inverse_p_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to p_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_p_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in P_WAVE  mode */
		if (P_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=P_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_p_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_p_wave */

static void set_inverse_qrs_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to qrs_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_qrs_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in QRS_WAVE  mode */
		if (QRS_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=QRS_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_qrs_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_qrs_wave */

static void set_inverse_t_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to t_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_t_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in T_WAVE  mode */
		if (T_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=T_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_t_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_t_wave */

static void set_inverse_pqrs_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to pqrs_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_pqrs_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in PQRS_WAVE  mode */
		if (PQRS_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=PQRS_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_pqrs_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_pqrs_wave */

static void set_inverse_pt_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to pt_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_pt_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in PT_WAVE  mode */
		if (PT_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=PT_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_pt_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_pt_wave */

static void set_inverse_qrst_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to qrst_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_qrst_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in QRST_WAVE  mode */
		if (QRST_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=QRST_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_qrst_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_qrst_wave */

static void set_inverse_pqrst_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to pqrst_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_pqrst_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in PQRST_WAVE  mode */
		if (PQRST_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=PQRST_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_pqrst_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_pqrst_wave */

static void set_inverse_none_wave(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Sets the inverse wave to none_wave
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_none_wave);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in NO_WAVE  mode */
		if (NO_WAVE!=trace->inverse_wave_mode)
		{
			trace->inverse_wave_mode=NO_WAVE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_none_wave.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_none_wave */

static void identify_pot_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace pot_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_pot_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.pot_act_choice.potential_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_pot_button. Missing trace_window");
	}
	LEAVE;
} /*identify_pot_button */

static void identify_act_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Saves the id of the trace act_button
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_act_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.pot_act_choice.activation_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_act_button. Missing trace_window");
	}
	LEAVE;
} /*identify_act_button */


static void set_inverse_potential(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to potential
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_potential);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in INVERSE_POTENTIAL  mode */
		if (INVERSE_POTENTIAL!=trace->inverse_pot_act_mode)
		{
			trace->inverse_pot_act_mode=INVERSE_POTENTIAL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_potential.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_potential */

static void set_inverse_activation(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Sets the inverse wave to activation
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_inverse_activation);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in ACTIVATION  mode */
		if (INVERSE_ACTIVATION!=trace->inverse_pot_act_mode)
		{
			trace->inverse_pot_act_mode=INVERSE_ACTIVATION;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_inverse_activation.  Missing trace_window");
	}
	LEAVE;
} /* set_inverse_activation */

static void id_trace_inverse_load_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace inverse load button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_inverse_load_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.load_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_inverse_load_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_inverse_load_butt */

static void load_inverse(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Load the inverse exnode,exelem,transfer matrix files
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(load_inverse);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to bring up a load dialog box here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"load_inverse.  Missing trace_window");
	}
	LEAVE;
} /* load_inverse */

static void id_trace_calculate_inverse_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace inversebutton.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_calculate_inverse_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.inverse_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_calculate_inverse_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_calculate_inverse_butt */

static void calculate_inverse(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
calculates the inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_inverse);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to perform inverse here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_inverse.  Missing trace_window");
	}
	LEAVE;
} /* calculate_inverse */

static void id_trace_inverse_forward_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace inverse forward  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_inverse_forward_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.forward_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_inverse_forward_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_inverse_forward_butt */

static void forward_inverse(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate inverse forward
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(forward_inverse);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to calculate forward here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"forward_inverse.  Missing trace_window");
	}
	LEAVE;
} /* forward_inverse */

static void id_trace_inverse_improve_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace inverse improve  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_inverse_improve_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.improve_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_inverse_improve_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_inverse_improve_butt */

static void improve_inverse(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate inverse improve
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(improve_inverse);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to calculate improve here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"improve_inverse.  Missing trace_window");
	}
	LEAVE;
} /* improve_inverse */

static void identify_trace_analysis_mode_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the analysis mode menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_analysis_mode_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.analysis_mode_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_analysis_mode_ch.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_analysis_mode_ch */

static void identify_trace_analysis_mode_ap(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace close button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_analysis_mode_ap);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.apply_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_analysis_mode_ap.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_analysis_mode_ap */

static void identify_trace_close_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace close button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_close_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_close_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_close_button */

static void close_trace_window(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Closes the trace window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(close_trace_window);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (trace->shell)
		{
			/* close the trace shell */
			XtPopdown(trace->shell);
			trace->open=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,"close_trace_window.  Missing shell");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_trace_window.  Missing trace_window");
	}
	LEAVE;
} /* close_trace_window */

static void identify_trace_paned_win(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Saves the id of the trace paned window
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_paned_win);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->paned_window= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_paned_win.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_paned_win */

static void identify_trace_1_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace 1 area pane.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_1_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.pane= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_1_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_1_area */

static void identify_trace_1_inverse_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 February 2001

DESCRIPTION :
Saves the id of the trace inverse menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_1_inverse_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.inverse.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_1_inverse_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_1_inverse_menu */

static void identify_trace_enlarge_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_menu */

static void identify_trace_enlarge_calculat(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge calculate button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_calculat);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.calculate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_calculat.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_calculat */

static void identify_trace_enlarge_detectio(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge detection choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_detectio);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.detection_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_detectio.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_detectio */

static void identify_detection_interval_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge detection interval button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_detection_interval_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.detection.interval_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_detection_interval_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_detection_interval_but */

static void identify_detection_level_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Saves the id of the trace enlarge detection level button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_detection_level_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.detection.level_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_detection_level_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_detection_level_button */

static void identify_detection_threshold_bu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge detection threshold button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_detection_threshold_bu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.detection.threshold_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_detection_threshold_bu.  Missing trace_window");
	}
	LEAVE;
} /* identify_detection_threshold_bu */

static void identify_trace_enlarge_objectiv(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_objectiv);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_objectiv.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_objectiv */

static void identify_obj_absolute_slope_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective absolute slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_absolute_slope_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.absolute_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_absolute_slope_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_absolute_slope_but */

static void identify_obj_positive_slope_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective positive slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_positive_slope_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.positive_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_positive_slope_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_positive_slope_but */

static void identify_obj_negative_slope_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective negative slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_negative_slope_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.negative_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_negative_slope_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_negative_slope_but */

static void identify_obj_absolute_value_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective absolute value button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_absolute_value_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.absolute_value_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_absolute_value_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_absolute_value_but */

static void identify_obj_positive_value_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective positive value button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_positive_value_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.positive_value_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_positive_value_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_positive_value_but */

static void identify_obj_negative_value_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 April 2001

DESCRIPTION :
Saves the id of the trace enlarge objective negative value button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_obj_negative_value_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.negative_value_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_obj_negative_value_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_obj_negative_value_but */

static void identify_trace_enlarge_datum_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge datum choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_datum_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.datum_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_datum_ch.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_datum_ch */

static void identify_datum_automatic_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge datum automatic button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_datum_automatic_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.datum.automatic_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_datum_automatic_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_datum_automatic_button */

static void set_datum_automatic(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the datum calculation method to automatic.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_datum_automatic);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (AUTOMATIC_DATUM!= *(trace->event_detection.datum_type))
		{
			*(trace->event_detection.datum_type)=AUTOMATIC_DATUM;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_datum_automatic.  Missing trace_window");
	}
	LEAVE;
} /* set_datum_automatic */

static void identify_datum_fixed_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge datum fixed button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_datum_fixed_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.datum.fixed_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_datum_fixed_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_datum_fixed_button */

static void set_datum_fixed(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the datum calculation method to fixed.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_datum_fixed);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (FIXED_DATUM!= *(trace->event_detection.datum_type))
		{
			*(trace->event_detection.datum_type)=FIXED_DATUM;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_datum_fixed.  Missing trace_window");
	}
	LEAVE;
} /* set_datum_fixed */

static void identify_trace_enlarge_events_f(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge events form.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_events_f);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.number_of_events_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_events_f.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_events_f */

static void identify_trace_enlarge_events_d(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge events down arrow.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_events_d);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.number_of_events.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_events_d.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_events_d */

static void identify_trace_enlarge_events_l(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge events label.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_events_l);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.number_of_events.label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_events_l.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_events_l */

static void identify_trace_enlarge_events_u(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge events up arrow.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_events_u);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.number_of_events.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_events_u.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_events_u */

static void identify_trace_enlarge_level_va(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Saves the id of the trace enlarge level value.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_level_va);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.level_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_level_va.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_level_va */

static void id_trace_average_width_txt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Saves the id of the trace enlarge level width.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_average_width_txt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.average_width_txt= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_average_width_txt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_average_width_txt */

static void id_trace_average_width(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Saves the id of the trace enlarge level width.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_average_width);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->menu.average_width= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_average_width.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_average_width */

static void change_average_width(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 10 May 2002

DESCRIPTION :
Called when the average width is changed by the level width text field.
==============================================================================*/
{
	char *value_string;
	int average_width;
	struct Device *processed_device;
	struct Signal_buffer *processed_buffer;
	struct Trace_window *trace;
	XmAnyCallbackStruct *any_data;

	ENTER(change_average_width);
	USE_PARAMETER(widget_id);
	if ((any_data=(XmAnyCallbackStruct *)call_data)&&
		((XmCR_ACTIVATE==any_data->reason)||(XmCR_LOSING_FOCUS==any_data->reason)))
	{
		if ((trace=(struct Trace_window *)trace_window)&&
			((trace->event_detection).average_width))
		{
			value_string=(char *)NULL;
			XtVaGetValues(trace->menu.average_width_txt,
				XmNvalue,&value_string,
				NULL);
			if (1==sscanf(value_string,"%d",&average_width))
			{
				if (average_width<1)
				{
					average_width=1;
				}
				if (average_width!= *((trace->event_detection).average_width))
				{
					if ((processed_device=trace->processed_device)&&
						(processed_device->signal)&&
						(processed_buffer=processed_device->signal->buffer)&&
						(processed_device->channel))
					{
						if (processed_device->signal->next)
						{
						/* calculate objective function */
						calculate_device_objective(processed_device,
							*((trace->event_detection).detection),
							*((trace->event_detection).objective),
							((processed_buffer->signals).float_values)+
							(processed_device->signal->next->index),
							processed_buffer->number_of_samples,
							processed_buffer->number_of_signals,average_width);
						}
						if (True==XmToggleButtonGadgetGetState((trace->area_3).edit.
							objective_toggle))
						{
							redraw_trace_3_drawing_area((Widget)NULL,trace_window,
								(XtPointer)NULL);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"change_average_width.  Missing processed_device");
					}
				}
			}
			else
			{
				average_width= *((trace->event_detection).average_width);
			}
			*((trace->event_detection).average_width)=average_width;
			XtFree(value_string);
			sprintf(global_temp_string,"%d",average_width);
			XtVaSetValues(trace->menu.average_width_txt,
				XmNvalue,global_temp_string,
				NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"change_average_width.  Missing trace_window");
		}
	}
	LEAVE;
} /* change_average_width */

static void identify_trace_enlarge_thresh_s(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge threshold scroll bar.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_thresh_s);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.threshold_scroll= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_thresh_s.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_thresh_s */

static void change_threshold(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Called when the threshold is changed by the threshold scroll bar.
==============================================================================*/
{
	char value_string[10];
	struct Trace_window *trace;
	XmScrollBarCallbackStruct *scroll_data;
	XmString value_xmstring;

	ENTER(change_threshold);
	USE_PARAMETER(widget_id);
	if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
		(XmCR_VALUE_CHANGED==scroll_data->reason))
	{
		if (trace=(struct Trace_window *)trace_window)
		{
			*(trace->event_detection.threshold)=scroll_data->value;
			sprintf(value_string,"%3d%%",scroll_data->value);
			value_xmstring=XmStringCreateSimple(value_string);
			XtVaSetValues(trace->area_1.enlarge.threshold_label,
				XmNlabelString,value_xmstring,
				NULL);
			XmStringFree(value_xmstring);
		}
		else
		{
			display_message(ERROR_MESSAGE,"change_threshold.  Missing trace_window");
		}
	}
	LEAVE;
} /* change_threshold */

static void identify_trace_enlarge_thresh_l(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge threshold label.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_thresh_l);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.threshold_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_thresh_l.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_thresh_l */

static void identify_trace_enlarge_separ_sc(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge minimum separation scroll bar.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_separ_sc);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.minimum_separation_scroll= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_separ_sc.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_separ_sc */

static void change_minimum_separation(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Called when the minimum separation is changed by the minimum separation scroll
bar.
==============================================================================*/
{
	char value_string[10];
	struct Trace_window *trace;
	XmScrollBarCallbackStruct *scroll_data;
	XmString value_xmstring;

	ENTER(change_minimum_separation);
	USE_PARAMETER(widget_id);
	if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
		(XmCR_VALUE_CHANGED==scroll_data->reason))
	{
		if (trace=(struct Trace_window *)trace_window)
		{
			*(trace->event_detection.minimum_separation)=scroll_data->value;
			sprintf(value_string,"%3d ms",scroll_data->value);
			value_xmstring=XmStringCreateSimple(value_string);
			XtVaSetValues(trace->area_1.enlarge.minimum_separation_label,
				XmNlabelString,value_xmstring,
				NULL);
			XmStringFree(value_xmstring);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"change_minimum_separation.  Missing trace_window");
		}
	}
	LEAVE;
} /* change_minimum_separation */

static void identify_trace_enlarge_separ_la(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge minimum separation label.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_separ_la);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.minimum_separation_label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_separ_la.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_separ_la */

static void identify_trace_enlarge_all_curr(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge all/current choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_all_curr);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.all_current_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_all_curr.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_all_curr */

static void identify_trace_enlarge_all_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge all button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_all_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.all_current.all_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_all_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_all_butt */

static void set_calculate_all(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Set the calculate button so that events are calculated for all signals.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_calculate_all);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.calculate_all_events=1;
		trace_update_signal_controls(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_calculate_all.  Missing trace_window");
	}
	LEAVE;
} /* set_calculate_all */

static void identify_trace_enlarge_current(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace enlarge current button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_enlarge_current);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.all_current.current_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_enlarge_current.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_current */

static void set_calculate_current(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Set the calculate button so that events are calculated for current signal only.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_calculate_current);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.calculate_all_events=0;
		trace_update_signal_controls(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_calculate_current.  Missing trace_window");
	}
	LEAVE;
} /* set_calculate_current */

static void identify_time_window_hamming_bu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the Hamming button in the data window type choice menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_time_window_hamming_bu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.window_type_menu.hamming_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_time_window_hamming_bu.  Missing trace_window");
	}
	LEAVE;
} /* identify_time_window_hamming_bu */

static void set_time_window_hamming(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the data windowing to be the Hamming window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_time_window_hamming);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (HAMMING_WINDOW!=trace->frequency_domain.window_type)
		{
			trace->frequency_domain.window_type=HAMMING_WINDOW;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_time_window_hamming.  Missing trace_window");
	}
	LEAVE;
} /* set_time_window_hamming */

static void identify_time_window_parzen_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the Parzen button in the data window type choice menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_time_window_parzen_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.window_type_menu.parzen_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_time_window_parzen_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_time_window_parzen_but */

static void set_time_window_parzen(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the data windowing to be the Parzen window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_time_window_parzen);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (PARZEN_WINDOW!=trace->frequency_domain.window_type)
		{
			trace->frequency_domain.window_type=PARZEN_WINDOW;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_time_window_parzen.  Missing trace_window");
	}
	LEAVE;
} /* set_time_window_parzen */

static void identify_time_window_square_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the square button in the data window type choice menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_time_window_square_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.window_type_menu.square_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_time_window_square_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_time_window_square_but */

static void set_time_window_square(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the data windowing to be the square window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_time_window_square);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (SQUARE_WINDOW!=trace->frequency_domain.window_type)
		{
			trace->frequency_domain.window_type=SQUARE_WINDOW;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_time_window_square.  Missing trace_window");
	}
	LEAVE;
} /* set_time_window_square */

static void identify_time_window_welch_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the Welch button in the data window type choice menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_time_window_welch_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.window_type_menu.welch_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_time_window_welch_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_time_window_welch_butt */

static void set_time_window_welch(Widget *widget_id,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the data windowing to be the Welch window.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_time_window_welch);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (WELCH_WINDOW!=trace->frequency_domain.window_type)
		{
			trace->frequency_domain.window_type=WELCH_WINDOW;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_time_window_welch.  Missing trace_window");
	}
	LEAVE;
} /* set_time_window_welch */

static void identify_trace_time_window_choi(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the time window type choice for trace area 1.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_time_window_choi);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.window_type_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_time_window_choi.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_time_window_choi */

static void identify_trace_correlation_1_me(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the correlation time domain menu for trace area 1.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_1_me);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.correlation_time_domain.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_1_me.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_1_me */

static void identify_trace_correlation_1_to(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the toggle button for the correlation menu in trace area 1.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_1_to);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.correlation_time_domain.toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_1_to.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_1_to */

static void toggle_correlation_signal_1_inp(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Toggles whether signal 1 or signal 2 is the signal which is updated when
trace_update_signal is called.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(toggle_correlation_signal_1_inp);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (trace->correlation.signal_1_input)
		{
			trace->correlation.signal_1_input=0;
			XtVaSetValues(trace->area_2.correlation_time_domain.toggle,
				XmNset,True,
				NULL);
		}
		else
		{
			trace->correlation.signal_1_input=1;
			XtVaSetValues(trace->area_2.correlation_time_domain.toggle,
				XmNset,False,
				NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_correlation_signal_1_inp.  Missing trace_window");
	}
	LEAVE;
} /* toggle_correlation_signal_1_inp */

static void id_trace_beat_averaging_1_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the beat averaging menu for trace area 1.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_beat_averaging_1_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_beat_averaging_1_menu.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_beat_averaging_1_menu */

static void identify_trace_beat_averaging_f(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the trace beat averaging beats form.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_beat_averaging_f);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.number_of_beats_form= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_beat_averaging_f.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_beat_averaging_f */

static void identify_trace_beat_averaging_d(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the trace beat averaging beats down arrow.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_beat_averaging_d);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.number_of_beats.down_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_beat_averaging_d.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_beat_averaging_d */

static void identify_trace_beat_averaging_l(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the trace beat averaging beats label.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_beat_averaging_l);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.number_of_beats.label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_beat_averaging_l.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_beat_averaging_l */

static void identify_trace_beat_averaging_u(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the trace beat averaging beats up arrow.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_beat_averaging_u);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.number_of_beats.up_arrow= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_beat_averaging_u.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_beat_averaging_u */

static void identify_trace_beat_averaging_a(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
Saves the id of the trace beat averaging align button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_beat_averaging_a);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.beat_averaging.align_with_events_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_beat_averaging_a.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_beat_averaging_a */

static void identify_trace_1_drawing_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the drawing area in trace area 1.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_1_drawing_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_1_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_1_drawing_area */

static void identify_trace_2_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace 2 area pane.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_2_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_2.pane= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_2_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_2_area */

static void identify_trace_1_calculate_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 March 2001

DESCRIPTION :
Saves the id of the trace calculate menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_1_calculate_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_1_calculate_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_1_calculate_menu */

static void id_trace_calculate_apply_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace calculate apply  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_calculate_apply_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.apply_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_calculate_apply_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_calculate_apply_butt */

static void calculate_apply(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate apply for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_apply);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to calculate apply here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_apply.  Missing trace_window");
	}
	LEAVE;
} /* calculate_apply */

static void id_trace_calculate_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the calculate calculate  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_calculate_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.calculate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_calculate_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_calculate_butt */

int write_marker_time(char *time_str,int x_marker,int axes_left,int axes_width,int top,
	XFontStruct *font,GC graphics_context,Display *display,Widget drawing_area,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : writes the time at the top of a marker.

DESCRIPTION : 4 April 2001
==============================================================================*/
{
	int ascent,descent,direction,length,return_code,x_string,y_string;
	XCharStruct bounds;

	ENTER(write_marker_time);
	if (time_str&&display&&drawing)
	{
		/* write the peak or trough time */
		length=strlen(time_str);
		XTextExtents(font,time_str,length,&direction,&ascent,&descent,
			&bounds);
		x_string=x_marker+(bounds.lbearing-bounds.rbearing+1)/2;
		if (x_string+bounds.rbearing>=axes_left+axes_width)
		{
			x_string=axes_left+axes_width-bounds.rbearing;
		}
		if (x_string-bounds.lbearing<axes_left)
		{
			x_string=axes_left+bounds.lbearing;
		}
		y_string=top-descent;
		XDrawString(display,drawing->pixel_map,graphics_context,x_string,
			y_string,time_str,length);
		XDrawString(display,XtWindow(drawing_area),graphics_context,
			x_string,y_string,time_str,length);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/*write_marker_time*/

static int draw_Cardiac_interval_box(struct Cardiac_interval *interval,
	int end_analysis_interval,int start_analysis_interval,
	int top,int height,int axes_left,int axes_width,
	Widget drawing_area,struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information,
	struct Signal_buffer *buffer)
/*******************************************************************************
LAST MODIFIED :  10 April 2001

DESCRIPTION : draws a box showing the cardiac interval
==============================================================================*/
{
	char time_string[20];
	Display *display;
	float x_scale;
	GC box_colour,graphics_context;
	int interval_start,interval_width,peak_or_trough,return_code,right,time;

	ENTER(draw_Cardiac_interval_box);
	display=(Display *)NULL;
	if (buffer&&interval&&drawing_area&&drawing&&(drawing->user_interface)&&
		(drawing->pixel_map)&&signal_drawing_information&&
		(signal_drawing_information->user_interface==drawing->user_interface))
	{
		box_colour=interval->graphics_context;
		x_scale=SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
			(axes_width)-1);
		interval_start=SCALE_X(interval->start_time,
			start_analysis_interval,axes_left,x_scale);
		right=SCALE_X(interval->end_time,
			start_analysis_interval,axes_left,x_scale);
		peak_or_trough=SCALE_X(interval->peak_or_trough_time,
			start_analysis_interval,axes_left,x_scale);
		interval_width=right-interval_start;
		time=(int)((float)((buffer->times)[interval->peak_or_trough_time])*1000./
			(buffer->frequency));
		sprintf(time_string,"%d",time);

		/* draw the box */		
		display=User_interface_get_display(signal_drawing_information->user_interface);		
		XDrawLine(display,XtWindow(drawing_area),box_colour,interval_start,
			top,interval_start,top+height);
		XDrawLine(display,drawing->pixel_map,box_colour,interval_start,
			top,interval_start,top+height);
		XDrawLine(display,XtWindow(drawing_area),box_colour,interval_start+interval_width,
			top,interval_start+interval_width,top+height);
		XDrawLine(display,drawing->pixel_map,box_colour,interval_start+interval_width,
			top,interval_start+interval_width,top+height);
		XDrawLine(display,XtWindow(drawing_area),box_colour,interval_start+1,
			top,interval_start+interval_width-1,top);
		XDrawLine(display,drawing->pixel_map,box_colour,interval_start+1,
			top,interval_start+interval_width-1,top);
		XDrawLine(display,XtWindow(drawing_area),box_colour,interval_start+1,
			top+height,interval_start+interval_width-1,top+height);
		XDrawLine(display,drawing->pixel_map,box_colour,interval_start+1,
			top+height,interval_start+interval_width-1,top+height);
		/* draw the centre line */
		graphics_context=
			(signal_drawing_information->graphics_context).cardiac_interval_colour;
		XDrawLine(display,XtWindow(drawing_area),graphics_context,peak_or_trough,
			top+1,peak_or_trough,top+height-1);
		XDrawLine(display,drawing->pixel_map,graphics_context,peak_or_trough,
			top+1,peak_or_trough,top+height-1);
		/* write the peak or trough time */
		return_code=write_marker_time(time_string,peak_or_trough,axes_left,
			axes_width,top,signal_drawing_information->font,
			(signal_drawing_information->graphics_context).	potential_time_colour_text,
			display,drawing_area,drawing);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_Cardiac_interval_box.  Invalid drawing/drawing area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_Cardiac_interval_box */

static int draw_Cardiac_interval_markers(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED :  28 March 2001

DESCRIPTION : draws the markers on the trace window.
==============================================================================*/
{

	int end_analysis_interval,return_code,start_analysis_interval;

	struct Cardiac_interval *interval;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;

	ENTER(draw_Cardiac_interval_markers);
	interval=(struct Cardiac_interval *)NULL;
	highlight_device=(struct Device *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	trace_area_1=(struct Trace_window_area_1 *)NULL;
	if (trace)
	{
		return_code=1;
		if ((interval=trace->first_interval)&&(trace->highlight)&&
			(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			trace_area_1= &(trace->area_1);
			/* draw all interval markers */
			while (interval)
			{
				draw_Cardiac_interval_box(interval,end_analysis_interval,
					start_analysis_interval,trace_area_1->axes_top,
					trace_area_1->axes_height,trace_area_1->axes_left,
					trace_area_1->axes_width,trace_area_1->drawing_area,
					trace_area_1->drawing,trace->signal_drawing_information,buffer);
				interval=interval->next;
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_Cardiac_interval_markers.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* draw_Cardiac_interval_markers */

int reconcile_Electrical_imaging_event_marker(int *marker,
	struct Electrical_imaging_event *event,float x_scale,int axes_left,
	int start_analysis_interval)
/*******************************************************************************
LAST MODIFIED : 13 June 2001

DESCRIPTION : ensure that that the  <event>'s time matches the <marker>.
Set <event> to <marker> and then <marker> to <event> to avoid
rounding error in SCALE_X(..1/x_scale...) followed by SCALE_X(..x_scale...)
==============================================================================*/
{
	int return_code;

	ENTER(reconcile_Electrical_imaging_event_marker);
	if (marker)
	{
		return_code=1;
		event->time=SCALE_X(*marker,axes_left,
			start_analysis_interval,1/x_scale);
		*marker=SCALE_X(event->time,start_analysis_interval,
			axes_left,x_scale);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reconcile_Electrical_imaging_event_marker. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* reconcile_Electrical_imaging_event_marker */

int draw_Electrical_imaging_event_marker(struct Electrical_imaging_event *event,
	int end_analysis_interval,int start_analysis_interval,
	int top,int height,int axes_left,int axes_width,
	Widget drawing_area,struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information,
	struct Signal_buffer *buffer)
/*******************************************************************************
LAST MODIFIED :  1 June 2001

DESCRIPTION : Draws the marker of the Electrical_imaging_event <event>
==============================================================================*/
{
	char time_string[20];
	Display *display;
	float frequency,x_scale;
	GC graphics_context;
	int event_time,marker,return_code,time,*times;

	ENTER(draw_Electrical_imaging_event_marker);
	display=(Display *)NULL;
	if (buffer&&event&&drawing_area&&drawing&&(drawing->user_interface)&&
		(drawing->pixel_map)&&signal_drawing_information&&(times=buffer->times)&&
		(signal_drawing_information->user_interface==drawing->user_interface))
	{
		x_scale=SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
			(axes_width)-1);
		event_time=SCALE_X(event->time,start_analysis_interval,
			axes_left,x_scale);
		/* draw the line */
		display=User_interface_get_display(signal_drawing_information->user_interface);		
		graphics_context=(signal_drawing_information->graphics_context).
			eimaging_event_colour;
		XDrawLine(display,XtWindow(drawing_area),graphics_context,event_time,
			top+1,event_time,top+height-1);
		XDrawLine(display,drawing->pixel_map,graphics_context,event_time,
			top+1,event_time,top+height-1);
		/*  if required, write the time text */
		if (event->is_current_event)
		{
			frequency=buffer->frequency;
			marker=SCALE_X(event->time,start_analysis_interval,
				axes_left,x_scale);
			reconcile_Electrical_imaging_event_marker(&marker,event,x_scale,
				axes_left,start_analysis_interval);
			time=(int)((float)((times)[event->time])
				*1000./frequency+0.5);
			sprintf(time_string,"%d",time);
			write_marker_time(time_string,marker,axes_left,
				axes_width,top,signal_drawing_information->font,
				graphics_context,display,drawing_area,drawing);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_Electrical_imaging_event_marker.  Invalid drawing/drawing area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_Electrical_imaging_event_marker */

int draw_Electrical_imaging_event_markers(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED :1 June 2001

DESCRIPTION : draws the markers on the trace window.
==============================================================================*/
{

	int end_analysis_interval,return_code,start_analysis_interval;

	struct Electrical_imaging_event *event;
	struct Device *cardiac_interval_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(draw_Electrical_imaging_event_markers);
	event=(struct Electrical_imaging_event *)NULL;
	cardiac_interval_device=(struct Device *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	trace_area_3=(struct Trace_window_area_3 *)NULL;
	if (trace)
	{
		return_code=1;
		if ((event=*trace->first_eimaging_event)&&
			(cardiac_interval_device=trace->cardiac_interval_device)&&
			(buffer=get_Device_signal_buffer(cardiac_interval_device)))
		{
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			trace_area_3= &(trace->area_3);
			/* draw all event markers */
			while (event)
			{
				draw_Electrical_imaging_event_marker(event,end_analysis_interval,
					start_analysis_interval,trace_area_3->axes_top,
					trace_area_3->axes_height,trace_area_3->axes_left,
					trace_area_3->axes_width,trace_area_3->drawing_area,
					trace_area_3->drawing,trace->signal_drawing_information,buffer);
				event=event->next;
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_Electrical_imaging_event_markers.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* draw_Electrical_imaging_event_markers */

static int calculate_Cardiac_interval_device(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Calculate the <trace's> cardiac_interval_device.
This is a device that contains a signal derived from trace->highlight
or trace->processed_device.
If trace->calculate_signal_mode==CURRENT_SIGNAL, it derives from
trace->highlight;
if trace->calculate_signal_mode==RMS_SIGNAL or RMS_AND_CURRENT_SIGNAL it derives
from trace->processed_device .
If the trace's cardiac intervals have been calculated,
trace->cardiac_interval_device's signal will be calculated  from the start of
the first cardiac interval to the end of the last. If they haven't been
calculated, it contains the entire signal.
==============================================================================*/
{
	int buffer_end,buffer_offset,buffer_start,*cardiac_interval_time,j,
		number_of_signals,number_of_samples,return_code,signal_number;
	float *current_times,*current_values,frequency,*cardiac_interval_value,
		maximum_value,minimum_value,*time_float,*value;
	struct Cardiac_interval *interval;
	struct Device *cardiac_interval_device,*source_device;
	struct Signal_buffer *cardiac_interval_buffer;
	struct Signal *signal_next,*signal_next_new;
#if defined (DEVICE_EXPRESSIONS)
	struct Signal_buffer *buffer;
#else /* defined (DEVICE_EXPRESSIONS) */
	struct Device **electrodes;
	struct Device *device;
#endif /* defined (DEVICE_EXPRESSIONS) */

	ENTER(calculate_Cardiac_interval_device);
	return_code=0;
#if !defined (DEVICE_EXPRESSIONS)
	electrodes=(struct Device **)NULL;
#endif /* !defined (DEVICE_EXPRESSIONS) */
	interval=(struct Cardiac_interval *)NULL;
	source_device=(struct Device *)NULL;
	cardiac_interval_device=(struct Device *)NULL;
	current_values=(float *)NULL;
	current_times=(float *)NULL;
	cardiac_interval_value=(float *)NULL;
	time_float=(float *)NULL;
	value=(float *)NULL;
	cardiac_interval_time=(int *)NULL;
	signal_next=(struct Signal *)NULL;
	signal_next_new=(struct Signal *)NULL;
	cardiac_interval_buffer=(struct Signal_buffer *)NULL;
	if (trace&&(cardiac_interval_device=trace->cardiac_interval_device)&&
		(cardiac_interval_device->signal)&&(cardiac_interval_device->channel)&&
		(cardiac_interval_buffer=cardiac_interval_device->signal->buffer))
	{
		if (trace->calculate_signal_mode==CURRENT_SIGNAL)
		{
			source_device=**(trace->highlight);
		}
		else
		{	/* RMS/RMS_AND_CURRENT_SIGNAL	*/
			source_device=trace->processed_device;
		}
		if (source_device)
		{
			if (trace->calculate_signal_mode==RMS_AND_CURRENT_SIGNAL)
			{
				/* set up the signal->next */
				if (signal_next=cardiac_interval_device->signal->next)
				{
					signal_next_new=(struct Signal *)NULL;
				}
				else
				{
					signal_next_new=create_Signal(1,cardiac_interval_buffer,REJECTED,2);
					signal_next=signal_next_new;
				}
				cardiac_interval_device->signal->next=signal_next;
				signal_next->buffer=cardiac_interval_buffer;
			}
			else
			{
				destroy_Signal(&(cardiac_interval_device->signal->next));
			}
			buffer_start=0;
#if defined (DEVICE_EXPRESSIONS)
			if (buffer=get_Device_signal_buffer(source_device))
			{
				buffer_end=(buffer->number_of_samples)-1;
				frequency=buffer->frequency;
			}
			else
			{
				buffer_end=0;
				frequency=1;
			}
#else /* defined (DEVICE_EXPRESSIONS) */
			if (AUXILIARY==source_device->description->type)
			{
				electrodes=(source_device->description->properties).auxiliary.
					electrodes;
				if (electrodes)
				{
					device=(*electrodes);
				}
				else
				{
					device=source_device;
				}
			}
			else
			{
				device=source_device;
			}
			buffer_end=device->signal->buffer->number_of_samples-1;
			frequency=device->signal->buffer->frequency;
#endif /* defined (DEVICE_EXPRESSIONS) */
			/* extract all the signals (signal and any signal->next)*/
			signal_number=0;
			if (extract_signal_information((struct FE_node *)NULL,
				(struct Signal_drawing_package *)NULL,source_device,signal_number,
				buffer_start,buffer_end,&number_of_signals,&number_of_samples,
				&current_times,&current_values,(enum Event_signal_status **)NULL,
				(char **)NULL,(int *)NULL,&minimum_value,&maximum_value))
			{
				/* realloc Signal_buffer for cardiac_interval_device */
				if (cardiac_interval_buffer=reallocate_Signal_buffer(
					cardiac_interval_buffer,FLOAT_VALUE,number_of_signals,
					number_of_samples,frequency))
				{
					time_float=current_times;
					cardiac_interval_time=cardiac_interval_buffer->times;
					buffer_offset=cardiac_interval_buffer->number_of_signals;
					cardiac_interval_device->channel->offset=0;
					cardiac_interval_device->channel->gain=1;
					cardiac_interval_device->signal_display_maximum=0;
					cardiac_interval_device->signal_display_minimum=1;
					/*copy the times*/
					for (j=number_of_samples;j>0;j--)
					{
						*cardiac_interval_time=(int)((*time_float)*frequency+0.5);
						cardiac_interval_time++;
						time_float++;
					}
					/* put source signal (rms or current) in the
						cardiac_interval_device->signal */
					value=current_values;
					cardiac_interval_value=
						((cardiac_interval_buffer->signals).float_values)+
						(cardiac_interval_device->signal->index);
					for (j=number_of_samples;j>0;j--)
					{
						*cardiac_interval_value= *value;
						cardiac_interval_value += buffer_offset;
#if defined (OLD_CODE)
						/*???DB.  Have reversed the order, signal number now varies
							fastest */
						value++;
#endif /* defined (OLD_CODE) */
						value += number_of_signals;
					}
					if (trace->calculate_signal_mode==RMS_AND_CURRENT_SIGNAL)
					{
						/* put rms signal in the cardiac_interval_device->signal->next */
#if defined (OLD_CODE)
						/*???DB.  Have reversed the order, signal number now varies
							fastest */
						value=current_values+number_of_samples;
#endif /* defined (OLD_CODE) */
						value=current_values+1;
						cardiac_interval_value=
							((cardiac_interval_buffer->signals).float_values)+
							(cardiac_interval_device->signal->next->index);
						for (j=number_of_samples;j>0;j--)
						{
							*cardiac_interval_value= *value;
							cardiac_interval_value += buffer_offset;
#if defined (OLD_CODE)
							/*???DB.  Have reversed the order, signal number now varies
								fastest */
							value++;
#endif /* defined (OLD_CODE) */
							value += number_of_signals;
						}
					}
					/* ensure the range of rms is 0 to max; min to max looks confusing, as
						rms is all positive.*/
					if (trace->calculate_signal_mode==RMS_SIGNAL)
					{
						cardiac_interval_device->signal_display_minimum=0;
						cardiac_interval_device->signal_display_maximum=maximum_value;
					}
					trace->valid_processing=1;
					/* view from  the first cariac interval's start_time to last
						interval's  end_time (else we'll see al the signal) */
					if (trace->first_interval)
					{
						interval=trace->first_interval;
						cardiac_interval_buffer->start=interval->start_time;
						/* find last interval*/
						while (interval->next)
						{
							interval=interval->next;
						}
						cardiac_interval_buffer->end=interval->end_time;
					}
					else
					{
#if defined (DEVICE_EXPRESSIONS)
						if (buffer=get_Device_signal_buffer(source_device))
						{
							cardiac_interval_buffer->start=buffer->start;
							cardiac_interval_buffer->end=buffer->end;
						}
						else
						{
							cardiac_interval_buffer->start=0;
							cardiac_interval_buffer->end=0;
						}
#else /* defined (DEVICE_EXPRESSIONS) */
						if (AUXILIARY==source_device->description->type)
						{
							electrodes=(source_device->description->properties).auxiliary.
								electrodes;
							if (electrodes)
							{
								device=(*electrodes);
							}
							else
							{
								device=source_device;
							}
						}
						else
						{
							device=source_device;
						}
						cardiac_interval_buffer->start=device->signal->buffer->start;
						cardiac_interval_buffer->end=device->signal->buffer->end;
#endif /* defined (DEVICE_EXPRESSIONS) */
					}
					return_code=1;
				} /* if (cardiac_interval_buffer=reallocate_Signal_buffer */
				else
				{
					display_message(ERROR_MESSAGE,"calculate_Cardiac_interval_device.  "
						"reallocate_Signal_buffer failed");
					return_code=0;
				}
			}	/* if (extract_signal_information*/
			else
			{
				display_message(ERROR_MESSAGE,"calculate_Cardiac_interval_device.  "
					"extract_signal_information failed");
				return_code=0;
			}
			if (current_times)
			{
				DEALLOCATE(current_times);
			}
			if (current_values)
			{
				DEALLOCATE(current_values);
			}
		}/* if (source_device)	*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_Cardiac_interval_device. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* calculate_Cardiac_interval_device */

static void calculate_calculate(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Calculate  intervals (P QRS T) for inverse.
Does this by finding the signal maximum, then the adjacent zero crossings
on each side of this max. Interval is left zero x-ing to right zero x-ing.
Then search the signal for new intervals, EXCLUDING the area region of the
previous intervals.

==============================================================================*/
{
	/* can have at most 3 cardiac_intervals, p,qrs,t*/
	enum Cardiac_interval_type cardiac_interval_type[3];
	GC graphics_context[3];
	int buffer_start,buffer_end,*dest_time,end_time,excluded,
		gc_index,i,j,number_of_intervals,number_of_samples,
		number_of_signals,offset,start_time,success,time_max;
	float frequency,max,*source_time,*time,*times,*values,*value;
	struct Device *device;
	struct Cardiac_interval *interval,*new_interval;
	struct Signal *intervals_signal;
	struct Signal_buffer *intervals_buffer;
	struct Trace_window *trace;
#if defined (DEBUG)
	int max_time;
#endif

	ENTER(calculate_calculate);
	dest_time=(int *)NULL;
	source_time=(float *)NULL;
	time=(float *)NULL;
	times=(float *)NULL;
	values=(float *)NULL;
	value=(float *)NULL;
	device=(struct Device *)NULL;
	interval=(struct Cardiac_interval *)NULL;
	new_interval=(struct Cardiac_interval *)NULL;
	intervals_signal=(struct Signal *)NULL;
	intervals_buffer=(struct Signal_buffer *)NULL;
	trace=(struct Trace_window *)NULL;
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* erase any existing intervals graphics */
		draw_Cardiac_interval_markers(trace);
		/* destroy any existing intervals */
		if (trace->first_interval)
		{
			destroy_Cardiac_interval_list(&trace->first_interval);
		}
		/*for NO_WAVE, all we do is destroy the existing intervals */
		if (trace->inverse_wave_mode!=NO_WAVE)
		{
			intervals_buffer=create_Signal_buffer(FLOAT_VALUE,2,1,1);
			intervals_signal=create_Signal(1,intervals_buffer,REJECTED,2);
			number_of_signals=1;
			/* get the current device */
			if ((trace->highlight)&&(*(trace->highlight))&&
				(device= **(trace->highlight))&&(device->signal)&&
				(device->signal->buffer))
			{
				offset=device->signal->buffer->start;
				/* use the processed_device for rms signal */
				if (trace->calculate_signal_mode==RMS_SIGNAL)
				{
					if (trace->processed_device)
					{
						device=trace->processed_device;
						success=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_calculate. no processed_device for RMS!");
						success=0;
					}
				}
				else
				{
					success=1;
				}
				/* concerned with buffer->start to buffer->end, the 'selected' part of*/
				/*the signal NOT all of it ; they are the same for the RMS signal*/
				buffer_start=device->signal->buffer->start;
				buffer_end=device->signal->buffer->end;
				if (success&&extract_signal_information((struct FE_node *)NULL,
					(struct Signal_drawing_package *)NULL,device,1,buffer_start,buffer_end
					,(int *)NULL,&number_of_samples,&times,&values,
					(enum Event_signal_status **)NULL,(char **)NULL,(int *)NULL,
					(float *)NULL,(float *)NULL)&&(0<number_of_samples))
				{
					success=0;
					frequency=device->signal->buffer->frequency;
					if (intervals_buffer=reallocate_Signal_buffer(intervals_buffer,
						FLOAT_VALUE,number_of_signals,number_of_samples,frequency))
					{
						/* copy in the current signal */
						source_time=times;
						dest_time=intervals_buffer->times;
						/* just set the pointer, as it's only one signal */
						((intervals_buffer->signals).float_values)=values;
						for (j=number_of_samples;j>0;j--)
						{
							*dest_time=(int)((*source_time)*frequency+0.5);
							dest_time++;
							source_time++;
						}
						/* now find the peaks/troughs*/
						gc_index=0;
						switch (trace->inverse_wave_mode)
						{
							case P_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									pwave_colour;
								cardiac_interval_type[0]=P_WAVE_INTERVAL;
								number_of_intervals=1;
							} break;
							case QRS_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									qrswave_colour;
								cardiac_interval_type[0]=QRS_WAVE_INTERVAL;
								number_of_intervals=1;
							} break;
							case T_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									twave_colour;
								cardiac_interval_type[0]=T_WAVE_INTERVAL;
								number_of_intervals=1;
							} break;
							case PQRS_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									pwave_colour;
								graphics_context[1]=
									(trace->signal_drawing_information->graphics_context).
									qrswave_colour;
								cardiac_interval_type[0]=P_WAVE_INTERVAL;
								cardiac_interval_type[1]=QRS_WAVE_INTERVAL;
								number_of_intervals=2;
							} break;
							case PT_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									pwave_colour;
								graphics_context[1]=
									(trace->signal_drawing_information->graphics_context).
									twave_colour;
								cardiac_interval_type[0]=P_WAVE_INTERVAL;
								cardiac_interval_type[1]=T_WAVE_INTERVAL;
								number_of_intervals=2;
							} break;
							case QRST_WAVE:
							{
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									qrswave_colour;
								graphics_context[1]=
									(trace->signal_drawing_information->graphics_context).
									twave_colour;
								cardiac_interval_type[0]=QRS_WAVE_INTERVAL;
								cardiac_interval_type[1]=T_WAVE_INTERVAL;
								number_of_intervals=2;
							} break;
							case PQRST_WAVE:
							{
								number_of_intervals=3;
								graphics_context[0]=
									(trace->signal_drawing_information->graphics_context).
									pwave_colour;
								graphics_context[1]=
									(trace->signal_drawing_information->graphics_context).
									qrswave_colour;
								graphics_context[2]=
									(trace->signal_drawing_information->graphics_context).
									twave_colour;
								cardiac_interval_type[0]=P_WAVE_INTERVAL;
								cardiac_interval_type[1]=QRS_WAVE_INTERVAL;
								cardiac_interval_type[2]=T_WAVE_INTERVAL;
							} break;
							/*colours to cycle through */
						};
						for (i=0;i<number_of_intervals;i++)
						{
							value=values;
							time=times;
							max=fabs(*value);
#if defined (DEBUG)
							max_time=*times;
#endif
							time_max=0;
							success=0;
							for (j=0;j<number_of_samples;j++)
							{
								excluded=0;
								/*seach any existing intervals,to see if this point is excluded*/
								if (interval=trace->first_interval)
								{
									/* find last interval */
									while (interval)
									{
										/* exclude the point if it's inside the interval*/
										if ((j>=interval->start_time)&&(j<=interval->end_time))
										{
											excluded=1;
										}
										interval=interval->next;
									}
								}
								if ((!excluded)&&(fabs(*value)>max))
								{
									max=fabs(*value);
									time_max=j;
#if defined (DEBUG)
									max_time=*time;
#endif
									success=1;
								}
								value++;
								time++;
							} /* for (j=0;j<number_of_samples;j++) */
							if (success)
								/* we found a max */
							{
#if defined (DEBUG)
								printf("max= %f, time = %f %d\n",max,max_time,time_max);
								fflush(NULL);
#endif
								/* now find the zero crossings either side of this peak/trough
									find RH zero crossing, OR the RH end of the window */
								success=0;
								j=time_max;
								while ((j<number_of_samples)&&(!success))
								{
									if ((values[j]==0)||((values[j]>0)&&(values[j+1]<0))||
										((values[j]<0)&&(values[j+1]>0))||(j==number_of_samples-1))
									{
#if defined (DEBUG)
										printf("RH zero crossing  = %f, time = %f %d\n",values[j],
											times[j],j);fflush(NULL);
#endif
										success=1;
										end_time=j;
									}
									j++;
								} /* while ((j<(number_of_samples-time_max))&&(!success)) */
								/* find LH zero crossing OR the LH end of the window */
								if (success)
								{
									success=0;
									j=time_max;
									while ((j>=0)&&(!success))
									{
										if ((values[j]==0)||((values[j]>0)&&(values[j-1]<0))||
											((values[j]<0)&&(values[j-1]>0))||(j==0))
										{
#if defined (DEBUG)
											printf("LH zero crossing  = %f, time = %f %d\n",values[j],
												times[j],j);
											fflush(NULL);
#endif
											success=1;
											start_time=j;
										}
										j--;
									}/* while ((j>0)&&(!success))*/
								}
								/* create new interval and add to list */
								if (success)
								{
									new_interval=create_Cardiac_interval(
										graphics_context[gc_index],time_max,start_time,end_time,
										cardiac_interval_type[i]);
									add_Cardiac_interval_to_sorted_list(&trace->first_interval,
										new_interval);
									if (i>2)
									{
										display_message(ERROR_MESSAGE,"calculate_calculate.  "
											"Can't (currently) have > 3 cardiac intervals ");
									}
								}
								/* cycle through the colours */
								if (gc_index<(number_of_intervals-1))
								{
									gc_index++;
								}
								else
								{
									gc_index=0;
								}
							} /* if (success)*/
						} /* for (i=0;i<number_of_intervals;i++) */
						/* intervals are relative to the first entry of the buffer,so add */
						/* buffer_start,the offset*/
						if (interval=trace->first_interval)
						{
							while (interval)
							{
								interval->peak_or_trough_time+=offset;
								interval->start_time+=offset;
								interval->end_time+=offset;
								interval=interval->next;
							}
						}
					} /* if (intervals_buffer=reallocate_Signal_buffer */
					/* don't deallocate values or do a destroy_Signal_buffer */
					DEALLOCATE(times);
					destroy_Signal(&intervals_signal);
				}	/* if (extract_signal_information */
			} /*if ((trace->highlight)&&( */
		}/* if (trace->inverse_wave_mode!=NO_WAVE) */
		/* we've changed the cardiac intervals, so change their device*/
		calculate_Cardiac_interval_device(trace);
		/* destroy the electrical imaging events*/
		if (trace->first_eimaging_event)
		{
			destroy_Electrical_imaging_event_list(trace->first_eimaging_event);
		}
		/*update the window  */
		draw_Cardiac_interval_markers(trace);
		redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
			(XtPointer)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_calculate.  Missing trace_window");
	}
	LEAVE;
} /* calculate_calculate */

static void identify_trace_cutoff_value(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the cutoff value.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_cutoff_value);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.cutoff_value= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_cutoff_value.  Missing trace_window");
	}
	LEAVE;
} /*identify_trace_cutoff_value*/

static void change_cutoff_value(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Change cutoff value  for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(change_cutoff_value);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to change cutoff value here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_cutoff_value.  Missing trace_window");
	}
	LEAVE;
} /* change_cutoff_value */

static void identify_calculate_int_rank_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the interval rankcutoff choice  menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_calculate_int_rank_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.interval_rank_cutoff_mode_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_calculate_int_rank_ch.  Missing trace_window");
	}
	LEAVE;
} /*identify_calculate_int_rank_ch */

static void id_calculate_intervals_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the intervals button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_intervals_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.interval_rank_cutoff_choice.interval_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_intervals_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_calculate_intervals_butt */

static void calculate_intervals(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate intervals for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_intervals);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to calculate intervals here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_intervals.  Missing trace_window");
	}
	LEAVE;
} /*calculate_intervals  */

static void id_calculate_rank_cutoff_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_rank_cutoff_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.interval_rank_cutoff_choice.rank_cutoff_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_rank_cutoff_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_calculate_rank_cutoff_butt */

static void calculate_rank_cutoff(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate rank_cutoff for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_rank_cutoff);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* will need code to calculate rank_cutoff here.*/
		USE_PARAMETER(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_rank_cutoff.  Missing trace_window");
	}
	LEAVE;
} /* calculate_rank_cutoff */

static void id_calculate_rms_current_ch(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2001

DESCRIPTION :
Saves the id of the  RMS_current_mode choice  menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_rms_current_ch);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.RMS_current_mode_choice= *widget_id;
		/* to match first entry in uil file*/
		trace->calculate_signal_mode=CURRENT_SIGNAL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_rms_current_ch.  Missing trace_window");
	}
	LEAVE;
} /* id_calculate_rms_current_ch*/

static void id_calculate_rms_sig_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the rms signal  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_rms_sig_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.RMS_current_choice.RMS_signal_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_rms_sig_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_calculate_rms_sig_butt */

static void calculate_rms_signal(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Calculate rms signal for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_rms_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->calculate_signal_mode=RMS_SIGNAL;
		trace_update_signal_controls(trace);
		trace->calculate_rms=1;
		trace_change_signal(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_rms_signal.  Missing trace_window");
	}
	LEAVE;
} /* calculate_rms_signal*/

static void id_calculate_curr_sig_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the current_signal  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_curr_sig_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.RMS_current_choice.current_signal_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_curr_sig_butt.  Missing trace_window");
	}
	LEAVE;
} /* id_calculate_curr_sig_butt */

static void calculate_current_signal(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
get the current signal for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_current_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->calculate_signal_mode=CURRENT_SIGNAL;
		trace_update_signal_controls(trace);
		trace->calculate_rms=1;
		trace_change_signal(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_current_signal.  Missing trace_window");
	}
	LEAVE;
} /* calculate_current_signal */

static void id_calculate_rms_curr_sig_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the current_signal  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_calculate_rms_curr_sig_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.calculate.RMS_current_choice.rms_current_signal_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_calculate_rms_curr_sig_butt. Missing trace_window");
	}
	LEAVE;
} /*id_calculate_rms_curr_sig_butt */

static void calculate_rms_current_signal(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
get the current signal for inverse
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(calculate_rms_current_signal);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->calculate_signal_mode=RMS_AND_CURRENT_SIGNAL;
		trace_update_signal_controls(trace);
		trace->calculate_rms=1;
		trace_change_signal(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_rms_current_signal. Missing trace_window");
	}
	LEAVE;
} /* calculate_rms_current_signal */

static void identify_trace_correlation_2_me(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the correlation menu for trace area 2.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_2_me);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_2.correlation_time_domain.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_2_me.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_2_me */

static void identify_trace_correlation_2_to(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the toggle button for the correlation menu in trace area 2.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_2_to);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_2.correlation_time_domain.toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_2_to.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_2_to */

static void toggle_correlation_signal_2_inp(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Toggles whether signal 1 or signal 2 is the signal which is updated when
trace_update_signal is called.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(toggle_correlation_signal_2_inp);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (trace->correlation.signal_1_input)
		{
			trace->correlation.signal_1_input=0;
			XtVaSetValues(trace->area_1.correlation_time_domain.toggle,
				XmNset,False,
				NULL);
		}
		else
		{
			trace->correlation.signal_1_input=1;
			XtVaSetValues(trace->area_1.correlation_time_domain.toggle,
				XmNset,True,
				NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_correlation_signal_2_inp.  Missing trace_window");
	}
	LEAVE;
} /* toggle_correlation_signal_2_inp */

static void identify_trace_2_drawing_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the drawing area in trace area 2.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_2_drawing_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_2.drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_2_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_2_drawing_area */

static void select_trace_2_drawing_area(Widget widget,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
==============================================================================*/
{
	XmDrawingAreaCallbackStruct *callback;

	ENTER(select_trace_2_drawing_area);
	USE_PARAMETER(widget);
	if (trace_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_trace_2_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_trace_2_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_trace_2_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* select_trace_2_drawing_area */


static void identify_trace_3_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace 3 area pane.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_3_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.pane= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_3_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_3_area */

static void id_trace_3_interval_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Saves the id of the trace interval menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_3_interval_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.interval.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_3_interval_menu.  Missing trace_window");
	}
	LEAVE;
} /*id_trace_3_interval_menu  */

static void id_trace_times_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED :21 February 2001

DESCRIPTION :
Saves the id of the trace times  button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_times_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.interval.times_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_times_button.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_times_button*/

static void configure_eimaging_event_times(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Do time for inverse
==============================================================================*/
{
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;
	struct Trace_window *trace;
	static MrmRegisterArg identifier_list[]=
	{
		{"trace_window_structure",(XtPointer)NULL}
	};

	ENTER(configure_eimaging_event_times);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (!(eimaging_time_dialog=trace->area_3.eimaging_time_dialog))
		{
			/* assign and register the identifiers */
				/*???DB.  Have to put in global name list because the map dialog
					hierarchy may not be open */
			identifier_list[0].value=(XtPointer)trace;
			if (MrmSUCCESS==MrmRegisterNames(identifier_list,
				XtNumber(identifier_list)))
			{
				eimaging_time_dialog=create_Electrical_imaging_time_dialog(
					&(trace->area_3.eimaging_time_dialog),trace->area_3.interval.times_button,
					trace->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"configure_eimaging_event_times.  Could not register identifiers");
			}
		}
		if (eimaging_time_dialog)
		{
			open_eimaging_time_dialog(eimaging_time_dialog);
			configure_eimaging_time_dialog_marker_menu(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"configure_eimaging_event_times.  Missing trace_window");
	}
	LEAVE;
} /*configure_eimaging_event_times */

static void identify_trace_edit_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit menu.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_enlarge_menu */

static void identify_trace_edit_previous_bu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit previous button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_previous_bu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.previous_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_previous_bu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_previous_bu */

#if defined (OLD_CODE)
static void identify_trace_edit_previous_ac(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the accelerator for trace edit previous button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_previous_ac);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.accelerator.previous_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_previous_ac.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_previous_ac */
#endif

static void identify_trace_edit_next_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit next button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_next_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.next_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_next_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_next_button */

#if defined (OLD_CODE)
static void identify_trace_edit_next_accele(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the accelerator for the trace edit next button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_next_accele);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.accelerator.next_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_next_accele.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_next_accele */
#endif

static void identify_trace_edit_accept_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit accept button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_accept_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.accept_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_accept_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_accept_butt */

#if defined (OLD_CODE)
static void identify_trace_edit_accept_acce(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the accelerator for the trace edit accept button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_accept_acce);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.accelerator.accept_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_accept_acce.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_accept_acce */
#endif

static void identify_trace_edit_reject_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit reject button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_reject_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.reject_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_reject_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_reject_butt */

#if defined (OLD_CODE)
static void identify_trace_edit_reject_acce(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the accelerator for the trace edit reject button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_reject_acce);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.accelerator.reject_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_reject_acce.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_reject_acce */
#endif

static void identify_trace_edit_order_choic(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit order choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_order_choic);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.order_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_order_choic.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_order_choic */

static void identify_edit_order_device_butt(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit order device button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_edit_order_device_butt);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.order.device_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_edit_order_device_butt.  Missing trace_window");
	}
	LEAVE;
} /* identify_edit_order_device_butt */

static void set_edit_order_device(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the event editing order to device.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_edit_order_device);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		*(trace->event_detection.edit_order)=DEVICE_ORDER;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_edit_order_device.  Missing trace_window");
	}
	LEAVE;
} /* set_edit_order_device */

static void identify_edit_order_beat_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace edit order beat button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_edit_order_beat_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.order.beat_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_edit_order_beat_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_edit_order_beat_button */

static void set_edit_order_beat(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the event editing order to beat.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_edit_order_beat);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		*(trace->event_detection.edit_order)=BEAT_ORDER;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_edit_order_beat.  Missing trace_window");
	}
	LEAVE;
} /* set_edit_order_beat */

static void identify_trace_edit_objective_t(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
Saves the id of the trace edit objective toggle.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_edit_objective_t);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.edit.objective_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_edit_objective_t.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_edit_objective_t */

void change_edit_objective_toggle(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
Called when the objective toggle is changed.
==============================================================================*/
{
	ENTER(change_edit_objective_toggle);
	USE_PARAMETER(call_data);
	redraw_trace_3_drawing_area(widget,trace_window,(XtPointer)NULL);
	LEAVE;
} /* change_edit_objective_toggle */

static void identify_trace_frequency_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the frequency domain menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_frequency_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_frequency_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_frequency_menu */

static void identify_trace_frequency_choice(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the frequency domain amplitude/phase or real/imaginary choice
for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_frequency_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.display_mode_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_frequency_choice.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_frequency_choice */

static void identify_amplitude_phase_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the amplitude/phase button frequency domain in the frequency
domain display mode choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_amplitude_phase_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.display_mode.amplitude_phase_button=
			*widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_amplitude_phase_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_amplitude_phase_button */

static void set_frequency_amplitude_phase(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the display mode in the frequency domain to amplitude/phase.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_frequency_amplitude_phase);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (AMPLITUDE_PHASE!=trace->frequency_domain.display_mode)
		{
			trace->frequency_domain.display_mode=AMPLITUDE_PHASE;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_frequency_amplitude_phase.  Missing trace_window");
	}
	LEAVE;
} /* set_frequency_amplitude_phase */

static void identify_real_imaginary_button(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the real/imaginary button frequency domain in the frequency
domain display mode choice.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_real_imaginary_button);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.frequency_domain.display_mode.real_imaginary_button=
			*widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_real_imaginary_button.  Missing trace_window");
	}
	LEAVE;
} /* identify_real_imaginary_button */

static void set_frequency_real_imaginary(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Sets the display mode in the frequency domain to real/imaginary.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_frequency_real_imaginary);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		if (REAL_IMAGINARY!=trace->frequency_domain.display_mode)
		{
			trace->frequency_domain.display_mode=REAL_IMAGINARY;
			/*???DB.  Don't need to redraw area 1, but don't want to repeat filtering.
				New function ? */
			trace_change_signal(trace);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_frequency_real_imaginary.  Missing trace_window");
	}
	LEAVE;
} /* set_frequency_real_imaginary */

static void identify_trace_power_spectra_me(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the power spectra menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_spectra_me);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_spectra_me.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_spectra_me */

static void identify_trace_power_spectra_la(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the label in the power spectra menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_spectra_la);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_spectra_la.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_spectra_la */

static void identify_trace_power_min_f_scro(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Saves the id of the minimum scroll bar in the power spectra menu for trace area
3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_min_f_scro);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.minimum_frequency_scroll_bar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_min_f_scro.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_min_f_scro */

static void identify_trace_power_min_f_valu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Saves the id of the minimum text field in the power spectra menu for trace area
3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_min_f_valu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.minimum_frequency_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_min_f_valu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_min_f_valu */

static void identify_trace_power_max_f_scro(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Saves the id of the maximum scroll bar in the power spectra menu for trace area
3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_max_f_scro);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.maximum_frequency_scroll_bar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_max_f_scro.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_max_f_scro */

static void identify_trace_power_max_f_valu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Saves the id of the maximum text field in the power spectra menu for trace area
3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_power_max_f_valu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.power_spectra.maximum_frequency_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_power_max_f_valu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_power_max_f_valu */

static void change_power_frequency_range(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 August 1999

DESCRIPTION :
Called when the frequency range to be displayed of the power spectra is changed.
==============================================================================*/
{
	char *new_value,value_string[20];
	float frequency;
	struct Power_spectra_area *power_spectra;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;
	XmAnyCallbackStruct *text_data;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(change_power_frequency_range);
	if (widget&&(trace=(struct Trace_window *)trace_window)&&
		(power_spectra=&(trace->area_3.power_spectra))&&call_data&&
		(trace->real_device_1)&&
		(buffer=get_Device_signal_buffer(trace->real_device_1)))
	{
		if (widget==power_spectra->minimum_frequency_scroll_bar)
		{
			if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
				(XmCR_VALUE_CHANGED==scroll_data->reason))
			{
				(trace->power_spectra).minimum_frequency=((float)((scroll_data->value)*
					(buffer->number_of_samples)))/(100.*(buffer->frequency));
				sprintf(value_string,"%g",(trace->power_spectra).minimum_frequency);
				XtVaSetValues(power_spectra->minimum_frequency_text_field,
					XmNvalue,value_string,
					NULL);
				trace_change_signal(trace);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"change_power_frequency_range.  Error minimum frequency scroll bar");
			}
		}
		else
		{
			if (widget==power_spectra->minimum_frequency_text_field)
			{
				if ((text_data=(XmAnyCallbackStruct *)call_data)&&
					((XmCR_ACTIVATE==text_data->reason)||
					(XmCR_LOSING_FOCUS==text_data->reason)))
				{
					XtVaGetValues(power_spectra->minimum_frequency_text_field,
						XmNvalue,&new_value,
						NULL);
					if (1==sscanf(new_value,"%f",&frequency))
					{
						if (frequency<0.)
						{
							(trace->power_spectra).minimum_frequency=0.;
						}
						else
						{
							if (frequency<(trace->power_spectra).maximum_frequency)
							{
								(trace->power_spectra).minimum_frequency=frequency;
							}
						}
					}
					sprintf(value_string,"%g",(trace->power_spectra).minimum_frequency);
					XtVaSetValues(power_spectra->minimum_frequency_text_field,
						XmNvalue,value_string,
						NULL);
					trace_change_signal(trace);
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"change_power_frequency_range.  Error minimum frequency text_field");
				}
			}
			else
			{
				if (widget==power_spectra->maximum_frequency_scroll_bar)
				{
					if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
						(XmCR_VALUE_CHANGED==scroll_data->reason))
					{
						(trace->power_spectra).maximum_frequency=
							((float)((scroll_data->value)*(buffer->number_of_samples)))/
							(100.*(buffer->frequency));
						sprintf(value_string,"%g",(trace->power_spectra).maximum_frequency);
						XtVaSetValues(power_spectra->maximum_frequency_text_field,
							XmNvalue,value_string,
							NULL);
						trace_change_signal(trace);
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"change_power_frequency_range.  Error maximum frequency scroll bar");
					}
				}
				else
				{
					if (widget==power_spectra->maximum_frequency_text_field)
					{
						if ((text_data=(XmAnyCallbackStruct *)call_data)&&
							((XmCR_ACTIVATE==text_data->reason)||
							(XmCR_LOSING_FOCUS==text_data->reason)))
						{
							XtVaGetValues(power_spectra->maximum_frequency_text_field,
								XmNvalue,&new_value,
								NULL);
							if (1==sscanf(new_value,"%f",&frequency))
							{
								if (frequency>(float)(buffer->number_of_samples)/
									(buffer->frequency))
								{
									(trace->power_spectra).maximum_frequency=
										(float)(buffer->number_of_samples)/(buffer->frequency);
								}
								else
								{
									if (frequency>(trace->power_spectra).minimum_frequency)
									{
										(trace->power_spectra).maximum_frequency=frequency;
									}
								}
							}
							sprintf(value_string,"%g",
								(trace->power_spectra).maximum_frequency);
							XtVaSetValues(power_spectra->maximum_frequency_text_field,
								XmNvalue,value_string,
								NULL);
							trace_change_signal(trace);
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"change_power_frequency_range.  Error maximum frequency text_field");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"change_power_frequency_range.  Invalid widget");
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"change_power_frequency_range.  Invalid argument(s)");
	}
	LEAVE;
} /* change_power_frequency_range */

static void identify_trace_correlation_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the correlation menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.correlation.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_menu */

static void identify_trace_correlation_labe(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the label in the correlation menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_correlation_labe);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.correlation.label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_correlation_labe.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_correlation_labe */

static void identify_trace_filtering_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the filtering menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_menu.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_menu */

static void identify_trace_filtering_label(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the label in the filtering menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_label);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.label= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_label.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_label */

static void identify_trace_filtering_high_s(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace filtering high-pass scroll bar.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_high_s);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.high_pass_scroll_bar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_high_s.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_high_s */

static void change_filtering(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
Called when the filtering is changed by the low or high pass (scroll bar or text
field) or notch (toggle or text field).
==============================================================================*/
{
	char *new_value,value_string[20];
	float frequency;
	struct Filtering_area *filtering;
	struct Signal_buffer *buffer;
	struct Trace_window *trace;
	XmAnyCallbackStruct *text_data;
	XmScrollBarCallbackStruct *scroll_data;

	ENTER(change_filtering);
	if (widget&&(trace=(struct Trace_window *)trace_window)&&
		(filtering=&(trace->area_3.filtering))&&call_data&&(trace->real_device_1)&&
		(buffer=get_Device_signal_buffer(trace->real_device_1)))
	{
		if (widget==filtering->high_pass_scroll_bar)
		{
			if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
				(XmCR_VALUE_CHANGED==scroll_data->reason))
			{
				(trace->filtering).high_pass_frequency=((float)((scroll_data->value)*
					(buffer->number_of_samples)))/(100.*(buffer->frequency));
				sprintf(value_string,"%g",(trace->filtering).high_pass_frequency);
				XtVaSetValues(filtering->high_pass_text_field,
					XmNvalue,value_string,
					NULL);
				trace_change_signal(trace);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"change_filtering.  Error high-pass scroll bar");
			}
		}
		else
		{
			if (widget==filtering->high_pass_text_field)
			{
				if ((text_data=(XmAnyCallbackStruct *)call_data)&&
					((XmCR_ACTIVATE==text_data->reason)||
					(XmCR_LOSING_FOCUS==text_data->reason)))
				{
					XtVaGetValues(filtering->high_pass_text_field,
						XmNvalue,&new_value,
						NULL);
					if (1==sscanf(new_value,"%f",&frequency))
					{
						if (frequency<0.)
						{
							(trace->filtering).high_pass_frequency=0.;
						}
						else
						{
							(trace->filtering).high_pass_frequency=
								(float)(buffer->number_of_samples)/(buffer->frequency);
							if (frequency<(trace->filtering).high_pass_frequency)
							{
								(trace->filtering).high_pass_frequency=frequency;
							}
						}
					}
					sprintf(value_string,"%g",(trace->filtering).high_pass_frequency);
					XtVaSetValues(filtering->high_pass_text_field,
						XmNvalue,value_string,
						NULL);
					trace_change_signal(trace);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"change_filtering.  Error high-pass text_field");
				}
			}
			else
			{
				if (widget==filtering->low_pass_scroll_bar)
				{
					if ((scroll_data=(XmScrollBarCallbackStruct *)call_data)&&
						(XmCR_VALUE_CHANGED==scroll_data->reason))
					{
						(trace->filtering).low_pass_frequency=((float)((scroll_data->value)*
							(buffer->number_of_samples)))/(100.*(buffer->frequency));
						sprintf(value_string,"%g",(trace->filtering).low_pass_frequency);
						XtVaSetValues(filtering->low_pass_text_field,
							XmNvalue,value_string,
							NULL);
						trace_change_signal(trace);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"change_filtering.  Error low-pass scroll bar");
					}
				}
				else
				{
					if (widget==filtering->low_pass_text_field)
					{
						if ((text_data=(XmAnyCallbackStruct *)call_data)&&
							((XmCR_ACTIVATE==text_data->reason)||
							(XmCR_LOSING_FOCUS==text_data->reason)))
						{
							XtVaGetValues(filtering->low_pass_text_field,
								XmNvalue,&new_value,
								NULL);
							if (1==sscanf(new_value,"%f",&frequency))
							{
								if (frequency<0.)
								{
									(trace->filtering).low_pass_frequency=0.;
								}
								else
								{
									(trace->filtering).low_pass_frequency=
										(float)(buffer->number_of_samples)/(buffer->frequency);
									if (frequency<(trace->filtering).low_pass_frequency)
									{
										(trace->filtering).low_pass_frequency=frequency;
									}
								}
							}
							sprintf(value_string,"%g",(trace->filtering).low_pass_frequency);
							XtVaSetValues(filtering->low_pass_text_field,
								XmNvalue,value_string,
								NULL);
							trace_change_signal(trace);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"change_filtering.  Error low-pass text_field");
						}
					}
					else
					{
						if (widget==filtering->notch_text_field)
						{
							if ((text_data=(XmAnyCallbackStruct *)call_data)&&
								((XmCR_ACTIVATE==text_data->reason)||
								(XmCR_LOSING_FOCUS==text_data->reason)))
							{
								XtVaGetValues(filtering->notch_text_field,
									XmNvalue,&new_value,
									NULL);
								if (1==sscanf(new_value,"%f",&frequency))
								{
									if (frequency<0.)
									{
										(trace->filtering).notch_frequency=0.;
									}
									else
									{
										(trace->filtering).notch_frequency=
											(float)(buffer->number_of_samples)/(buffer->frequency);
										if (frequency<(trace->filtering).notch_frequency)
										{
											(trace->filtering).notch_frequency=frequency;
										}
									}
								}
								sprintf(value_string,"%g",(trace->filtering).notch_frequency);
								XtVaSetValues(filtering->notch_text_field,
									XmNvalue,value_string,
									NULL);
								trace_change_signal(trace);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"change_filtering.  Error notch text_field");
							}
						}
						else
						{
							if (widget==filtering->notch_toggle)
							{
								if (True==XmToggleButtonGadgetGetState(filtering->notch_toggle))
								{
									(trace->filtering).notch_on=1;
								}
								else
								{
									(trace->filtering).notch_on=0;
								}
								trace_change_signal(trace);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"change_filtering.  Invalid widget");
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_filtering.  Invalid argument(s)");
	}
	LEAVE;
} /* change_filtering */

static void identify_trace_filtering_high_v(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace filtering high-pass text_field.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_high_v);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.high_pass_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_high_v.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_high_v */

static void identify_trace_filtering_low_sc(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace filtering low-pass scroll bar.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_low_sc);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.low_pass_scroll_bar= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_low_sc.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_low_sc */

static void identify_trace_filtering_low_va(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the trace filtering low-pass text_field.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_filtering_low_va);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.low_pass_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_filtering_low_va.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_filtering_low_va */

static void id_trace_filtering_notch_toggle(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
Saves the id of the trace filtering notch toggle button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_filtering_notch_toggle);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.notch_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_filtering_notch_toggle.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_filtering_notch_toggle */

static void id_trace_filtering_notch_value(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
Saves the id of the trace filtering notch text_field.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_filtering_notch_value);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.filtering.notch_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_filtering_notch_value.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_filtering_notch_value */

static void id_trace_beat_averaging_3_menu(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the beat averaging menu for trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_beat_averaging_3_menu);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.beat_averaging.menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_beat_averaging_3_menu.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_beat_averaging_3_menu */

static void id_trace_beat_averaging_baselin(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the baseline toggle button for the beat averaging menu in trace
area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_beat_averaging_baselin);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.beat_averaging.baseline_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_beat_averaging_baselin.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_beat_averaging_baselin */

static void change_beat_averaging(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 August 1997

DESCRIPTION :
Called when the beat averaging settings are changed.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(change_beat_averaging);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace_change_signal(trace);
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_beat_averaging.  Invalid argument");
	}
	LEAVE;
} /* change_beat_averaging */

static void id_trace_beat_averaging_toggle(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Saves the id of the beat averaging toggle button for the beat averaging menu in
trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_beat_averaging_toggle);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.beat_averaging.beat_averaging_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_beat_averaging_toggle.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_beat_averaging_toggle */

static void id_trace_beat_averaging_overlay(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1999

DESCRIPTION :
Saves the id of the overlay beats toggle button for the beat averaging menu in
trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(id_trace_beat_averaging_overlay);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.beat_averaging.overlay_beats_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_trace_beat_averaging_overlay.  Missing trace_window");
	}
	LEAVE;
} /* id_trace_beat_averaging_overlay */

static void identify_trace_3_drawing_area(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
Saves the id of the drawing area trace area 3.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_trace_3_drawing_area);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_3.drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_trace_3_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* identify_trace_3_drawing_area */

#if defined (OLD_CODE)
static void destroy_trace_window_shell(Widget widget,
	XtPointer trace_window_shell,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
This function expects <trace_window_shell> to be a pointer to a trace window
shell widget.  It sets the widget to NULL.
==============================================================================*/
{
	Widget *shell;

	ENTER(destroy_trace_window_shell);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (shell=(Widget *)trace_window_shell)
	{
		*shell=(Widget)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_trace_window_shell.  Missing client_data");
	}
	LEAVE;
} /* destroy_trace_window_shell */
#endif /* defined (OLD_CODE) */

static void trace_align_beats_to_events(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 July 2001

DESCRIPTION :
The callback for aligning the beats (search box divisions) to the events while
beat averaging.
==============================================================================*/
{
	float x_scale;
	int axes_left,*divisions,end_analysis_interval,end_search_interval,
		event_number,i,number_of_events,offset,*search_interval_divisions,
		start_analysis_interval,start_search_interval;
	struct Enlarge_area *enlarge;
	struct Event *event;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window *trace;

	ENTER(trace_align_beats_to_events);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((trace=(struct Trace_window *)trace_window)&&
		(signal_drawing_information=trace->signal_drawing_information))
	{
		if ((trace->highlight)&&(*(trace->highlight))&&
			(**(trace->highlight))&&(buffer=get_Device_signal_buffer(
			**(trace->highlight)))&&((**(trace->highlight))->signal)&&
			(event=(**(trace->highlight))->signal->first_event))
		{
			start_search_interval= *(trace->event_detection.start_search_interval);
			end_search_interval= *(trace->event_detection.end_search_interval);
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			while (event&&(event->time<start_search_interval))
			{
				event=event->next;
			}
			if (event&&(event->time<=end_search_interval)&&
				(start_analysis_interval<end_analysis_interval))
			{
				number_of_events= *(trace->event_detection.number_of_events);
				/*nothing to do for one event*/
				if (number_of_events>1)
				{
					ALLOCATE(search_interval_divisions,int,number_of_events-1);
					ALLOCATE(divisions,int,number_of_events-1);
					if (search_interval_divisions&&divisions)
					{
						enlarge= &(trace->area_1.enlarge);
						offset=(event->time)-start_search_interval;
						x_scale=SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
							(trace->area_1.axes_width)-1);
						axes_left=trace->area_1.axes_left;
						i=0;
						event=event->next;
						while ((i<number_of_events-1)&&event&&
							((event->time)<=end_search_interval))
						{
							search_interval_divisions[i]=(event->time)-offset;
							divisions[i]=SCALE_X(search_interval_divisions[i],
								start_analysis_interval,axes_left,x_scale);
							event=event->next;
							i++;
						}
						while (i<number_of_events-1)
						{
							search_interval_divisions[i]=end_search_interval;
							divisions[i]=SCALE_X(search_interval_divisions[i],
								start_analysis_interval,axes_left,x_scale);
							i++;
						}
						/* clear the search and edit interval boxes */
						draw_search_box(enlarge->left_box,trace->area_1.axes_top,
							enlarge->right_box-enlarge->left_box,trace->area_1.axes_height,
							*(trace->event_detection.detection),number_of_events,
							enlarge->divisions,trace->area_1.drawing_area,
							trace->area_1.drawing,signal_drawing_information);
						draw_highlight_event_box(enlarge->left_edit_box,
							trace->area_1.axes_top,
							enlarge->right_edit_box-enlarge->left_edit_box,
							trace->area_1.axes_height,*(trace->event_detection.detection),
							trace->area_1.drawing_area,trace->area_1.drawing,
							signal_drawing_information);
						DEALLOCATE(*(trace->event_detection.search_interval_divisions));
						*(trace->event_detection.search_interval_divisions)=
							search_interval_divisions;
						DEALLOCATE(enlarge->divisions);
						enlarge->divisions=divisions;
						event_number= *(trace->event_detection.event_number);
						if (1<event_number)
						{
							enlarge->left_edit_box=(enlarge->divisions)[event_number-2];
						}
						else
						{
							enlarge->left_edit_box=enlarge->left_box;
						}
						if (event_number<number_of_events)
						{
							enlarge->right_edit_box=(enlarge->divisions)[event_number-1];
						}
						else
						{
							enlarge->right_edit_box=enlarge->right_box;
						}
						/* draw the new search and edit interval boxes */
						draw_search_box(enlarge->left_box,trace->area_1.axes_top,
							enlarge->right_box-enlarge->left_box,trace->area_1.axes_height,
							*(trace->event_detection.detection),number_of_events,
							enlarge->divisions,trace->area_1.drawing_area,
							trace->area_1.drawing,signal_drawing_information);
						draw_highlight_event_box(enlarge->left_edit_box,
							trace->area_1.axes_top,
							enlarge->right_edit_box-enlarge->left_edit_box,
							trace->area_1.axes_height,*(trace->event_detection.detection),
							trace->area_1.drawing_area,trace->area_1.drawing,
							signal_drawing_information);
						trace_process_device(trace);
						redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)(trace),
							(XtPointer)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"trace_align_beats_to_events.  "
							"Could not allocate search_interval_divisions/divisions %d",
							number_of_events);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"trace_align_beats_to_events.  Missing trace_window");
	}
	LEAVE;
} /* trace_align_beats_to_events */

static void trace_overlay_beats(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 January 2000

DESCRIPTION :
The callback for toggling overlaying beats while beat averaging.

Can't just be <redraw_trace_3_drawing_area> because don't want to pass <widget>
and <call_data>.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(trace_overlay_beats);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((trace=(struct Trace_window *)trace_window)&&(trace->processed_device))
	{
		trace->processed_device->signal_display_minimum=1;
		trace->processed_device->signal_display_maximum=0;
		redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace_window,
			(XtPointer)NULL);
	}
	LEAVE;
} /* trace_overlay_beats */

static struct Trace_window *create_Trace_window(
	struct Trace_window **address,Widget activation,Widget parent,
	Pixel identifying_colour,enum Signal_analysis_mode *analysis_mode,
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
	int *end_search_interval,int screen_height,
		/*???DB.  height of interval drawing area ? */
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,
	struct Electrical_imaging_event **eimaging_events)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
This function allocates the memory for an trace window and sets the fields to
the specified values (<address>, <activation>).  It then retrieves an trace
window widget with the specified <parent> and assigns the widget ids to the
appropriate fields of the structure.  If successful it returns a pointer to the
created trace window and, if <address> is not NULL, makes <*address> point to
the created trace window.  If unsuccessful, NULL is returned.
==============================================================================*/
{
	char number_string[2],value_string[10];
	Dimension left_margin,right_margin;
	int device_number,i,widget_spacing;
	MrmType trace_window_class;
	Pixmap no_cascade_pixmap;

	static MrmRegisterArg callback_list[]={
		{"destroy_Trace_window",(XtPointer)destroy_Trace_window},
		{"identify_event_detection_button",
			(XtPointer)identify_event_detection_button},
		{"set_analysis_event_detection",(XtPointer)set_analysis_event_detection},
		{"identify_frequency_domain_butto",
			(XtPointer)identify_frequency_domain_butto},
		{"set_analysis_frequency_domain",(XtPointer)set_analysis_frequency_domain},
		{"identify_power_spectra_button",(XtPointer)identify_power_spectra_button},
		{"set_analysis_power_spectra",(XtPointer)set_analysis_power_spectra},
		{"identify_cross_correlation_butt",
			(XtPointer)identify_cross_correlation_butt},
		{"set_analysis_cross_correlation",
			(XtPointer)set_analysis_cross_correlation},
		{"identify_eimaging_butt",
			(XtPointer)identify_eimaging_butt},
		{"set_analysis_eimaging",
			(XtPointer)set_analysis_eimaging},
		{"identify_auto_correlation_butto",
			(XtPointer)identify_auto_correlation_butto},
		{"set_analysis_auto_correlation",(XtPointer)set_analysis_auto_correlation},
		{"identify_filtering_button",(XtPointer)identify_filtering_button},
		{"set_analysis_filtering",(XtPointer)set_analysis_filtering},
		{"identify_beat_averaging_button",
			(XtPointer)identify_beat_averaging_button},
		{"set_analysis_beat_averaging",(XtPointer)set_analysis_beat_averaging},
		{"identify_trace_analysis_mode_ch",
			(XtPointer)identify_trace_analysis_mode_ch},
		{"identify_trace_analysis_mode_ap",
			(XtPointer)identify_trace_analysis_mode_ap},
		{"identify_trace_close_button",(XtPointer)identify_trace_close_button},
		{"close_trace_window",(XtPointer)close_trace_window},
		{"identify_trace_paned_win",(XtPointer)identify_trace_paned_win},
		{"identify_trace_1_area",(XtPointer)identify_trace_1_area},
		{"identify_trace_1_inverse_menu",(XtPointer)identify_trace_1_inverse_menu},
		{"identify_trace_electrodes_ch",
			(XtPointer)identify_trace_electrodes_ch},
		{"id_electrodes_accepted_button",
			(XtPointer)id_electrodes_accepted_button},
		{"set_inv_electrodes_accepted",
		(XtPointer)set_inv_electrodes_accepted},
		{"id_electrodes_unrejected_button",
			(XtPointer)id_electrodes_unrejected_button},
		{"set_inv_electrodes_unrejected",
		(XtPointer)set_inv_electrodes_unrejected},
		{"id_electrodes_all_button",
			(XtPointer)id_electrodes_all_button},
		{"set_inv_electrodes_all",
		(XtPointer)set_inv_electrodes_all},
		{"identify_trace_waves_ch",
			(XtPointer)identify_trace_waves_ch},
		{"identify_p_wave_button",
			(XtPointer)identify_p_wave_button},
		{"set_inverse_p_wave",
			(XtPointer)set_inverse_p_wave},
		{"identify_qrs_wave_button",
			(XtPointer)identify_qrs_wave_button},
		{"set_inverse_qrs_wave",
			(XtPointer)set_inverse_qrs_wave},
		{"identify_t_wave_button",
			(XtPointer)identify_t_wave_button},
		{"set_inverse_t_wave",
			(XtPointer)set_inverse_t_wave},
		{"identify_pqrs_wave_button",
			(XtPointer)identify_pqrs_wave_button},
		{"identify_none_wave_button",
			(XtPointer)identify_none_wave_button},
		{"set_inverse_pqrs_wave",
			(XtPointer)set_inverse_pqrs_wave},
		{"set_inverse_none_wave",
			(XtPointer)set_inverse_none_wave},
		{"identify_pt_wave_button",
			(XtPointer)identify_pt_wave_button},
		{"set_inverse_pt_wave",
			(XtPointer)set_inverse_pt_wave},
		{"identify_qrst_wave_button",
			(XtPointer)identify_qrst_wave_button},
		{"set_inverse_qrst_wave",
			(XtPointer)set_inverse_qrst_wave},
		{"identify_pqrst_wave_button",
			(XtPointer)identify_pqrst_wave_button},
		{"set_inverse_pqrst_wave",
			(XtPointer)set_inverse_pqrst_wave},
		{"identify_trace_pot_act_ch",
			(XtPointer)identify_trace_pot_act_ch},
		{"identify_pot_button",
			(XtPointer)identify_pot_button},
		{"set_inverse_potential",
			(XtPointer)set_inverse_potential},
		{"identify_act_button",
			(XtPointer)identify_act_button},
		{"set_inverse_activation",
			(XtPointer)set_inverse_activation},
		{"id_trace_inverse_load_butt",
			(XtPointer)id_trace_inverse_load_butt},
		{"load_inverse",
			(XtPointer)load_inverse},
		{"id_trace_calculate_inverse_butt",
			(XtPointer)id_trace_calculate_inverse_butt},
		{"calculate_inverse",
			(XtPointer)calculate_inverse},
		{"id_trace_inverse_forward_butt",
			(XtPointer)id_trace_inverse_forward_butt},
		{"forward_inverse",
			(XtPointer)forward_inverse},
		{"id_trace_inverse_improve_butt",
			(XtPointer)id_trace_inverse_improve_butt},
		{"improve_inverse",
			(XtPointer)improve_inverse},
		{"identify_trace_enlarge_menu",(XtPointer)identify_trace_enlarge_menu},
		{"identify_trace_enlarge_detectio",
			(XtPointer)identify_trace_enlarge_detectio},
		{"identify_detection_interval_but",
			(XtPointer)identify_detection_interval_but},
		{"identify_detection_level_button",
			(XtPointer)identify_detection_level_button},
		{"identify_detection_threshold_bu",
			(XtPointer)identify_detection_threshold_bu},
		{"identify_trace_enlarge_objectiv",
			(XtPointer)identify_trace_enlarge_objectiv},
		{"identify_obj_absolute_slope_but",
			(XtPointer)identify_obj_absolute_slope_but},
		{"identify_obj_positive_slope_but",
			(XtPointer)identify_obj_positive_slope_but},
		{"identify_obj_negative_slope_but",
			(XtPointer)identify_obj_negative_slope_but},
		{"identify_obj_absolute_value_but",
			(XtPointer)identify_obj_absolute_value_but},
		{"identify_obj_positive_value_but",
			(XtPointer)identify_obj_positive_value_but},
		{"identify_obj_negative_value_but",
			(XtPointer)identify_obj_negative_value_but},
		{"identify_trace_enlarge_calculat",
			(XtPointer)identify_trace_enlarge_calculat},
		{"identify_trace_enlarge_datum_ch",
			(XtPointer)identify_trace_enlarge_datum_ch},
		{"identify_datum_automatic_button",
			(XtPointer)identify_datum_automatic_button},
		{"set_datum_automatic",(XtPointer)set_datum_automatic},
		{"identify_datum_fixed_button",(XtPointer)identify_datum_fixed_button},
		{"set_datum_fixed",(XtPointer)set_datum_fixed},
		{"identify_trace_enlarge_events_f",
			(XtPointer)identify_trace_enlarge_events_f},
		{"identify_trace_enlarge_events_d",
			(XtPointer)identify_trace_enlarge_events_d},
		{"identify_trace_enlarge_events_l",
			(XtPointer)identify_trace_enlarge_events_l},
		{"identify_trace_enlarge_events_u",
			(XtPointer)identify_trace_enlarge_events_u},
		{"identify_trace_enlarge_level_va",
			(XtPointer)identify_trace_enlarge_level_va},
		{"id_trace_average_width",
			(XtPointer)id_trace_average_width},
		{"id_trace_average_width_txt",
			(XtPointer)id_trace_average_width_txt},
		{"change_average_width",(XtPointer)change_average_width},
		{"identify_trace_enlarge_thresh_s",
			(XtPointer)identify_trace_enlarge_thresh_s},
		{"change_threshold",(XtPointer)change_threshold},
		{"identify_trace_enlarge_thresh_l",
			(XtPointer)identify_trace_enlarge_thresh_l},
		{"identify_trace_enlarge_separ_sc",
			(XtPointer)identify_trace_enlarge_separ_sc},
		{"change_minimum_separation",(XtPointer)change_minimum_separation},
		{"identify_trace_enlarge_separ_la",
			(XtPointer)identify_trace_enlarge_separ_la},
		{"identify_trace_enlarge_all_curr",
			(XtPointer)identify_trace_enlarge_all_curr},
		{"identify_trace_enlarge_all_butt",
			(XtPointer)identify_trace_enlarge_all_butt},
		{"set_calculate_all",(XtPointer)set_calculate_all},
		{"identify_trace_enlarge_current",
			(XtPointer)identify_trace_enlarge_current},
		{"set_calculate_current",(XtPointer)set_calculate_current},
		{"identify_time_window_hamming_bu",
			(XtPointer)identify_time_window_hamming_bu},
		{"set_time_window_hamming",(XtPointer)set_time_window_hamming},
		{"identify_time_window_parzen_but",
			(XtPointer)identify_time_window_parzen_but},
		{"set_time_window_parzen",(XtPointer)set_time_window_parzen},
		{"identify_time_window_square_but",
			(XtPointer)identify_time_window_square_but},
		{"set_time_window_square",(XtPointer)set_time_window_square},
		{"identify_time_window_welch_butt",
			(XtPointer)identify_time_window_welch_butt},
		{"set_time_window_welch",(XtPointer)set_time_window_welch},
		{"identify_trace_time_window_choi",
			(XtPointer)identify_trace_time_window_choi},
		{"identify_trace_correlation_1_me",
			(XtPointer)identify_trace_correlation_1_me},
		{"identify_trace_correlation_1_to",
			(XtPointer)identify_trace_correlation_1_to},
		{"toggle_correlation_signal_1_inp",
			(XtPointer)toggle_correlation_signal_1_inp},
		{"id_trace_beat_averaging_1_menu",
			(XtPointer)id_trace_beat_averaging_1_menu},
		{"identify_trace_beat_averaging_f",
			(XtPointer)identify_trace_beat_averaging_f},
		{"identify_trace_beat_averaging_d",
			(XtPointer)identify_trace_beat_averaging_d},
		{"identify_trace_beat_averaging_l",
			(XtPointer)identify_trace_beat_averaging_l},
		{"identify_trace_beat_averaging_u",
			(XtPointer)identify_trace_beat_averaging_u},
		{"identify_trace_beat_averaging_a",
			(XtPointer)identify_trace_beat_averaging_a},
		{"trace_align_beats_to_events",(XtPointer)trace_align_beats_to_events},
		{"identify_trace_1_drawing_area",(XtPointer)identify_trace_1_drawing_area},
		{"expose_trace_1_drawing_area",(XtPointer)redraw_trace_1_drawing_area},
		{"resize_trace_1_drawing_area",(XtPointer)redraw_trace_1_drawing_area},
		{"identify_trace_2_area",(XtPointer)identify_trace_2_area},
		{"identify_trace_1_calculate_menu",
		(XtPointer)identify_trace_1_calculate_menu},
		{"id_trace_calculate_apply_butt",
		(XtPointer)id_trace_calculate_apply_butt},
		{"calculate_apply",
		(XtPointer)calculate_apply},
		{"identify_calculate_int_rank_ch",
		(XtPointer)identify_calculate_int_rank_ch},
		{"id_calculate_intervals_butt",
		(XtPointer)id_calculate_intervals_butt},
		{"calculate_intervals",
		(XtPointer)calculate_intervals},
		{"id_calculate_rank_cutoff_butt",
		(XtPointer)id_calculate_rank_cutoff_butt},
		{"calculate_rank_cutoff",
		(XtPointer)calculate_rank_cutoff},
		{"id_calculate_rms_current_ch",
		(XtPointer)id_calculate_rms_current_ch},
		{"id_calculate_rms_sig_butt",
		(XtPointer)id_calculate_rms_sig_butt},
		{"calculate_rms_signal",
		(XtPointer)calculate_rms_signal},
		{"id_calculate_curr_sig_butt",
		(XtPointer)id_calculate_curr_sig_butt},
		{"calculate_current_signal",
		(XtPointer)calculate_current_signal},
		{"id_calculate_rms_curr_sig_butt",
		(XtPointer)id_calculate_rms_curr_sig_butt},
		{"calculate_rms_current_signal",
		(XtPointer)calculate_rms_current_signal},
		{"id_trace_calculate_butt",
		(XtPointer)id_trace_calculate_butt},
		{"calculate_calculate",
		(XtPointer)calculate_calculate},
		{"identify_trace_cutoff_value",
		(XtPointer)identify_trace_cutoff_value},
		{"change_cutoff_value",
		(XtPointer)change_cutoff_value},
		{"identify_trace_correlation_2_me",
			(XtPointer)identify_trace_correlation_2_me},
		{"identify_trace_correlation_2_to",
			(XtPointer)identify_trace_correlation_2_to},
		{"toggle_correlation_signal_2_inp",
			(XtPointer)toggle_correlation_signal_2_inp},
		{"identify_trace_2_drawing_area",(XtPointer)identify_trace_2_drawing_area},
		{"expose_trace_2_drawing_area",(XtPointer)redraw_trace_2_drawing_area},
		{"resize_trace_2_drawing_area",(XtPointer)redraw_trace_2_drawing_area},
		{"select_trace_2_drawing_area",(XtPointer)select_trace_2_drawing_area},
		{"identify_trace_3_area",(XtPointer)identify_trace_3_area},
		{"id_trace_3_interval_menu",
		(XtPointer)id_trace_3_interval_menu},
		{"id_trace_times_button",
		(XtPointer)id_trace_times_button},
		{"configure_eimaging_event_times",
		(XtPointer)configure_eimaging_event_times},
		{"identify_trace_edit_menu",(XtPointer)identify_trace_edit_menu},
		{"identify_trace_edit_previous_bu",
			(XtPointer)identify_trace_edit_previous_bu},
		{"identify_trace_edit_next_button",
			(XtPointer)identify_trace_edit_next_button},
		{"identify_trace_edit_accept_butt",
			(XtPointer)identify_trace_edit_accept_butt},
		{"identify_trace_edit_reject_butt",
			(XtPointer)identify_trace_edit_reject_butt},
		{"identify_trace_edit_order_choic",
			(XtPointer)identify_trace_edit_order_choic},
		{"identify_edit_order_device_butt",
			(XtPointer)identify_edit_order_device_butt},
		{"set_edit_order_device",(XtPointer)set_edit_order_device},
		{"identify_edit_order_beat_button",
			(XtPointer)identify_edit_order_beat_button},
		{"set_edit_order_beat",(XtPointer)set_edit_order_beat},
		{"identify_trace_edit_objective_t",
			(XtPointer)identify_trace_edit_objective_t},
		{"change_edit_objective_toggle",(XtPointer)change_edit_objective_toggle},
		{"identify_trace_frequency_menu",(XtPointer)identify_trace_frequency_menu},
		{"identify_trace_frequency_choice",
			(XtPointer)identify_trace_frequency_choice},
		{"identify_amplitude_phase_button",
			(XtPointer)identify_amplitude_phase_button},
		{"set_frequency_amplitude_phase",(XtPointer)set_frequency_amplitude_phase},
		{"identify_real_imaginary_button",
			(XtPointer)identify_real_imaginary_button},
		{"set_frequency_real_imaginary",(XtPointer)set_frequency_real_imaginary},
		{"identify_trace_power_spectra_me",
			(XtPointer)identify_trace_power_spectra_me},
		{"identify_trace_power_spectra_la",
			(XtPointer)identify_trace_power_spectra_la},
		{"identify_trace_power_min_f_scro",
			(XtPointer)identify_trace_power_min_f_scro},
		{"change_power_frequency_range",(XtPointer)change_power_frequency_range},
		{"identify_trace_power_min_f_valu",
			(XtPointer)identify_trace_power_min_f_valu},
		{"identify_trace_power_max_f_scro",
			(XtPointer)identify_trace_power_max_f_scro},
		{"identify_trace_power_max_f_valu",
			(XtPointer)identify_trace_power_max_f_valu},
		{"identify_trace_correlation_menu",
			(XtPointer)identify_trace_correlation_menu},
		{"identify_trace_correlation_labe",
			(XtPointer)identify_trace_correlation_labe},
		{"identify_trace_filtering_menu",(XtPointer)identify_trace_filtering_menu},
		{"identify_trace_filtering_label",
			(XtPointer)identify_trace_filtering_label},
		{"identify_trace_filtering_high_s",
			(XtPointer)identify_trace_filtering_high_s},
		{"change_filtering",(XtPointer)change_filtering},
		{"identify_trace_filtering_high_v",
			(XtPointer)identify_trace_filtering_high_v},
		{"identify_trace_filtering_low_sc",
			(XtPointer)identify_trace_filtering_low_sc},
		{"identify_trace_filtering_low_va",
			(XtPointer)identify_trace_filtering_low_va},
		{"id_trace_filtering_notch_toggle",
			(XtPointer)id_trace_filtering_notch_toggle},
		{"id_trace_filtering_notch_value",
			(XtPointer)id_trace_filtering_notch_value},
		{"id_trace_beat_averaging_3_menu",
			(XtPointer)id_trace_beat_averaging_3_menu},
		{"id_trace_beat_averaging_baselin",
			(XtPointer)id_trace_beat_averaging_baselin},
		{"change_beat_averaging",(XtPointer)change_beat_averaging},
		{"id_trace_beat_averaging_toggle",
			(XtPointer)id_trace_beat_averaging_toggle},
		{"id_trace_beat_averaging_overlay",
			(XtPointer)id_trace_beat_averaging_overlay},
		{"trace_overlay_beats",(XtPointer)trace_overlay_beats},
		{"identify_trace_3_drawing_area",(XtPointer)identify_trace_3_drawing_area},
		{"expose_trace_3_drawing_area",(XtPointer)redraw_trace_3_drawing_area},
		{"resize_trace_3_drawing_area",(XtPointer)redraw_trace_3_drawing_area}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"trace_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL}
	};
	struct Channel *cardiac_interval_channel,*imaginary_channel_1,
		*imaginary_channel_2,*processed_channel,*real_channel_1,*real_channel_2;
	struct Device **device,*cardiac_interval_device,*imaginary_device_1,
		*imaginary_device_2,*processed_device,*real_device_1,*real_device_2;
	struct Device_description *cardiac_interval_description,
		*imaginary_description_1,*imaginary_description_2,*processed_description,
		*real_description_1,*real_description_2;
	struct Signal *cardiac_interval_signal,*imaginary_signal_1,
		*imaginary_signal_2,*processed_signal,*real_signal_1,*real_signal_2;
	struct Signal_buffer *cardiac_interval_buffer,*frequency_domain_buffer_1,
		*frequency_domain_buffer_2,*processed_buffer;
	struct Region *current_region;
	struct Signal *signal;
	struct Trace_window *trace;
	Widget child_widget;
	XmString value_xmstring;

	ENTER(create_Trace_window);
	/* check arguments */
	if (signal_drawing_information&&user_interface&&detection)
	{
		no_cascade_pixmap=User_interface_get_no_cascade_pixmap(user_interface);
		if (MrmOpenHierarchy_base64_string(trace_window_uidh,
			&trace_window_hierarchy,&trace_window_hierarchy_open))
		{
			/* allocate memory */
			if ((cross_correlation_string=XmStringCreateSimple("Cross correlation"))&&
				(auto_correlation_string=XmStringCreateSimple("Auto correlation"))&&
				ALLOCATE(trace,struct Trace_window,1))
			{
				widget_spacing=User_interface_get_widget_spacing(user_interface);
				/* assign fields */
				trace->first_interval=(struct Cardiac_interval *)NULL;
				trace->calculate_rms=0;
				trace->open=0;
				trace->address=address;
				trace->activation=activation;
				trace->shell=parent;
				trace->signal_drawing_information=signal_drawing_information;
				trace->user_interface=user_interface;
				trace->window=(Widget)NULL;
#if defined (SPECTRAL_TOOLS)
				trace->analysis_mode=analysis_mode;
#else /* defined (SPECTRAL_TOOLS) */
				trace->analysis_mode=analysis_mode;
				*trace->analysis_mode=EVENT_DETECTION;
#endif /* defined (SPECTRAL_TOOLS) */
				trace->menu.average_width=(Widget)NULL;
				trace->menu.average_width_txt=(Widget)NULL;
				trace->menu.analysis_mode_choice=(Widget)NULL;
				trace->menu.analysis_mode.event_detection_button=(Widget)NULL;
				trace->menu.analysis_mode.frequency_domain_button=(Widget)NULL;
				trace->menu.analysis_mode.power_spectra_button=(Widget)NULL;
				trace->menu.analysis_mode.cross_correlation_button=(Widget)NULL;
				trace->menu.analysis_mode.auto_correlation_button=(Widget)NULL;
				trace->menu.analysis_mode.filtering_button=(Widget)NULL;
				trace->menu.analysis_mode.beat_averaging_button=(Widget)NULL;
				trace->menu.apply_button=(Widget)NULL;
				trace->menu.close_button=(Widget)NULL;
				trace->area_1.inverse.electrodes_choice_mode=(Widget)NULL;
				trace->area_1.inverse.electrodes_choice.accepted_button=(Widget)NULL;
				trace->area_1.inverse.electrodes_choice.unrejected_button=(Widget)NULL;
				trace->area_1.inverse.electrodes_choice.all_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice_mode=(Widget)NULL;
				trace->area_1.inverse.wave_choice.p_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.qrs_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.t_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.pqrs_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.pt_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.qrst_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.pqrst_button=(Widget)NULL;
				trace->area_1.inverse.wave_choice.none_button=(Widget)NULL;
				trace->area_1.inverse.pot_act_choice_mode=(Widget)NULL;
				trace->area_1.inverse.pot_act_choice.potential_button=(Widget)NULL;
				trace->area_1.inverse.pot_act_choice.activation_button=(Widget)NULL;
				trace->area_1.inverse.load_button=(Widget)NULL;
				trace->area_1.inverse.inverse_button=(Widget)NULL;
				trace->area_1.inverse.improve_button=(Widget)NULL;
				trace->area_1.inverse.forward_button=(Widget)NULL;
				trace->area_1.enlarge.divisions=(int *)NULL;
				trace->area_1.enlarge.calculate_button=(Widget)NULL;
				trace->area_1.enlarge.detection_choice=(Widget)NULL;
				trace->area_1.enlarge.detection.interval_button=(Widget)NULL;
				trace->area_1.enlarge.detection.level_button=(Widget)NULL;
				trace->area_1.enlarge.detection.threshold_button=(Widget)NULL;
				trace->area_1.enlarge.objective_choice=(Widget)NULL;
				trace->area_1.enlarge.objective.absolute_slope_button=(Widget)NULL;
				trace->area_1.enlarge.objective.positive_slope_button=(Widget)NULL;
				trace->area_1.enlarge.objective.negative_slope_button=(Widget)NULL;
				trace->area_1.enlarge.objective.absolute_value_button=(Widget)NULL;
				trace->area_1.enlarge.objective.positive_value_button=(Widget)NULL;
				trace->area_1.enlarge.objective.negative_value_button=(Widget)NULL;
				trace->area_1.enlarge.datum_choice=(Widget)NULL;
				trace->area_1.enlarge.datum.automatic_button=(Widget)NULL;
				trace->area_1.enlarge.datum.fixed_button=(Widget)NULL;
				trace->area_1.enlarge.number_of_events_form=(Widget)NULL;
				trace->area_1.enlarge.number_of_events.down_arrow=(Widget)NULL;
				trace->area_1.enlarge.number_of_events.label=(Widget)NULL;
				trace->area_1.enlarge.number_of_events.up_arrow=(Widget)NULL;
				trace->area_1.enlarge.threshold_scroll=(Widget)NULL;
				trace->area_1.enlarge.threshold_label=(Widget)NULL;
				trace->area_1.enlarge.minimum_separation_scroll=(Widget)NULL;
				trace->area_1.enlarge.minimum_separation_label=(Widget)NULL;
				trace->area_1.enlarge.level_value=(Widget)NULL;
				trace->area_1.enlarge.all_current_choice=(Widget)NULL;
				trace->area_1.enlarge.calculate_all_events=1;
				trace->area_1.enlarge.all_current.all_button=(Widget)NULL;
				trace->area_1.enlarge.all_current.current_button=(Widget)NULL;
				trace->area_1.correlation_time_domain.menu=(Widget)NULL;
				trace->area_1.correlation_time_domain.toggle=(Widget)NULL;
				trace->area_1.beat_averaging.menu=(Widget)NULL;
				trace->area_1.beat_averaging.number_of_beats_form=(Widget)NULL;
				trace->area_1.beat_averaging.number_of_beats.down_arrow=(Widget)NULL;
				trace->area_1.beat_averaging.number_of_beats.label=(Widget)NULL;
				trace->area_1.beat_averaging.number_of_beats.up_arrow=(Widget)NULL;
				trace->area_1.beat_averaging.align_with_events_button=(Widget)NULL;
				trace->area_1.pane=(Widget)NULL;
				trace->area_1.drawing_area=(Widget)NULL;
				trace->area_1.drawing=(struct Drawing_2d *)NULL;
				trace->area_1.axes_left=0;
				trace->area_1.axes_top=0;
				trace->area_1.axes_width=0;
				trace->area_1.axes_height=0;
				trace->area_1.calculate.menu=(Widget)NULL;
				trace->area_1.calculate.apply_button=(Widget)NULL;
				trace->area_1.calculate.interval_rank_cutoff_mode_choice=(Widget)NULL;
				trace->area_1.calculate.interval_rank_cutoff_choice.interval_button
					=(Widget)NULL;
				trace->area_1.calculate.interval_rank_cutoff_choice.rank_cutoff_button
					=(Widget)NULL;
				trace->area_1.calculate.RMS_current_mode_choice=(Widget)NULL;
				trace->area_1.calculate.RMS_current_choice.RMS_signal_button
					=(Widget)NULL;
				trace->area_1.calculate.RMS_current_choice.current_signal_button
					=(Widget)NULL;
				trace->area_1.calculate.RMS_current_choice.rms_current_signal_button
					=(Widget)NULL;
				trace->area_1.calculate.calculate_button=(Widget)NULL;
				trace->area_1.calculate.cutoff_value=(Widget)NULL;
				trace->area_2.correlation_time_domain.menu=(Widget)NULL;
				trace->area_2.correlation_time_domain.toggle=(Widget)NULL;
				trace->area_2.pane=(Widget)NULL;
				trace->area_2.drawing_area=(Widget)NULL;
				trace->area_2.drawing=(struct Drawing_2d *)NULL;
				trace->area_2.axes_left=0;
				trace->area_2.axes_top=0;
				trace->area_2.axes_width=0;
				trace->area_2.axes_height=0;
				trace->area_3.interval.menu=(Widget)NULL;
				trace->area_3.interval.times_button=(Widget)NULL;
				trace->area_3.edit.previous_button=(Widget)NULL;
				trace->area_3.edit.next_button=(Widget)NULL;
				trace->area_3.edit.accept_button=(Widget)NULL;
				trace->area_3.edit.reject_button=(Widget)NULL;
				trace->area_3.edit.order_choice=(Widget)NULL;
				trace->area_3.edit.order.device_button=(Widget)NULL;
				trace->area_3.edit.order.beat_button=(Widget)NULL;
				trace->area_3.edit.objective_toggle=(Widget)NULL;
				trace->area_3.eimaging_time_dialog=
					(struct Electrical_imaging_time_dialog *)NULL;
				trace->area_3.frequency_domain.menu=(Widget)NULL;
				trace->area_3.frequency_domain.display_mode_choice=(Widget)NULL;
				trace->area_3.frequency_domain.display_mode.amplitude_phase_button=
					(Widget)NULL;
				trace->area_3.frequency_domain.display_mode.real_imaginary_button=
					(Widget)NULL;
				trace->area_3.frequency_domain.window_type_choice=(Widget)NULL;
				trace->area_3.frequency_domain.window_type_menu.hamming_button=
					(Widget)NULL;
				trace->area_3.frequency_domain.window_type_menu.parzen_button=
					(Widget)NULL;
				trace->area_3.frequency_domain.window_type_menu.square_button=
					(Widget)NULL;
				trace->area_3.frequency_domain.window_type_menu.welch_button=
					(Widget)NULL;
				trace->area_3.power_spectra.menu=(Widget)NULL;
				trace->area_3.power_spectra.label=(Widget)NULL;
				trace->area_3.power_spectra.minimum_frequency_scroll_bar=(Widget)NULL;
				trace->area_3.power_spectra.minimum_frequency_text_field=(Widget)NULL;
				trace->area_3.power_spectra.maximum_frequency_scroll_bar=(Widget)NULL;
				trace->area_3.power_spectra.maximum_frequency_text_field=(Widget)NULL;
				trace->area_3.correlation.menu=(Widget)NULL;
				trace->area_3.correlation.label=(Widget)NULL;
				trace->area_3.filtering.menu=(Widget)NULL;
				trace->area_3.filtering.label=(Widget)NULL;
				trace->area_3.filtering.high_pass_scroll_bar=(Widget)NULL;
				trace->area_3.filtering.high_pass_text_field=(Widget)NULL;
				trace->area_3.filtering.low_pass_scroll_bar=(Widget)NULL;
				trace->area_3.filtering.low_pass_text_field=(Widget)NULL;
				trace->area_3.filtering.notch_toggle=(Widget)NULL;
				trace->area_3.filtering.notch_text_field=(Widget)NULL;
				trace->area_3.beat_averaging.menu=(Widget)NULL;
				trace->area_3.beat_averaging.baseline_toggle=(Widget)NULL;
				trace->area_3.beat_averaging.beat_averaging_toggle=(Widget)NULL;
				trace->area_3.beat_averaging.overlay_beats_toggle=(Widget)NULL;
				trace->area_3.pane=(Widget)NULL;
				trace->area_3.drawing_area=(Widget)NULL;
				trace->area_3.drawing=(struct Drawing_2d *)NULL;
				trace->area_3.axes_left=0;
				trace->area_3.axes_top=0;
				trace->area_3.axes_width=0;
				trace->area_3.axes_height=0;
				trace->event_detection.detection=detection;
				trace->event_detection.objective=objective;
				trace->event_detection.datum_type=datum_type;
				trace->event_detection.edit_order=edit_order;
				trace->highlight=highlight;
#if defined (UNEMAP_USE_NODES)
				trace->highlight_rig_node=highlight_rig_node;
#endif
				trace->signal_drawing_package=signal_drawing_package;
				trace->rig=rig;
				trace->event_detection.datum=datum;
				trace->event_detection.potential_time=potential_time;
				trace->event_detection.event_number=event_number;
				trace->event_detection.number_of_events=number_of_events;
				trace->event_detection.threshold=threshold;
				trace->event_detection.minimum_separation=minimum_separation;
				trace->event_detection.level=level;
				trace->event_detection.average_width=average_width;
				trace->event_detection.start_search_interval=start_search_interval;
				trace->event_detection.search_interval_divisions=
					search_interval_divisions;
				if (search_interval_divisions&&(*search_interval_divisions)&&
					number_of_events&&(1< *number_of_events))
				{
					ALLOCATE(trace->area_1.enlarge.divisions,int,(*number_of_events)-1);
				}
				trace->event_detection.end_search_interval=end_search_interval;
				trace->frequency_domain.display_mode=AMPLITUDE_PHASE;
				trace->valid_processing=0;
				trace->power_spectra.minimum_frequency= -1.;
				trace->power_spectra.maximum_frequency= -1.;
				trace->filtering.low_pass_frequency= -1.;
				trace->filtering.high_pass_frequency= -1.;
				trace->filtering.notch_frequency= -1.;
				trace->filtering.notch_on=0;
				/*???debug */
				trace->processed_device=(struct Device *)NULL;
				trace->cardiac_interval_device=(struct Device *)NULL;
				trace->first_eimaging_event=eimaging_events;
				/* allocate memory for spectral analysis */
				if (
					(frequency_domain_buffer_1=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
					(frequency_domain_buffer_2=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
					(cardiac_interval_buffer=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
					(processed_buffer=create_Signal_buffer(FLOAT_VALUE,2,1,1)))
				{
					if ((real_channel_1=create_Channel(0,0,1))&&
						(imaginary_channel_1=create_Channel(1,0,1))&&
						(real_channel_2=create_Channel(0,0,1))&&
						(imaginary_channel_2=create_Channel(1,0,1))&&
						(cardiac_interval_channel=create_Channel(1,0,1))&&
						(processed_channel=create_Channel(1,0,1)))
					{
						if ((real_signal_1=create_Signal(0,frequency_domain_buffer_1,
							UNDECIDED,0))&&(imaginary_signal_1=create_Signal(1,
							frequency_domain_buffer_1,UNDECIDED,0))&&(real_signal_2=
							create_Signal(0,frequency_domain_buffer_2,UNDECIDED,0))&&
							(imaginary_signal_2=create_Signal(1,frequency_domain_buffer_2,
							UNDECIDED,0))&&(processed_signal=create_Signal(0,
							processed_buffer,UNDECIDED,1))&&(processed_signal->next=
							create_Signal(1,processed_buffer,REJECTED,2))&&
							(cardiac_interval_signal=create_Signal(0,cardiac_interval_buffer,
								UNDECIDED,1)))
						{
							if ((real_description_1=create_Device_description("Re",AUXILIARY,
								(struct Region *)NULL))&&(imaginary_description_1=
								create_Device_description("Im",AUXILIARY,
								(struct Region *)NULL))&&(real_description_2=
								create_Device_description((char *)NULL,AUXILIARY,
								(struct Region *)NULL))&&(imaginary_description_2=
								create_Device_description((char *)NULL,AUXILIARY,
								(struct Region *)NULL))&&(processed_description=
								create_Device_description((char *)NULL,AUXILIARY,
								(struct Region *)NULL))&&(cardiac_interval_description=
								create_Device_description((char *)NULL,AUXILIARY,
								(struct Region *)NULL)))
							{
								if ((real_device_1=create_Device(0,real_description_1,
									real_channel_1,real_signal_1))&&(imaginary_device_1=
									create_Device(1,imaginary_description_1,imaginary_channel_1,
									imaginary_signal_1))&&(real_device_2=create_Device(0,
									real_description_2,real_channel_2,real_signal_2))&&
									(imaginary_device_2=create_Device(1,imaginary_description_2,
									imaginary_channel_2,imaginary_signal_2))&&
									(processed_device=create_Device(0,processed_description,
									processed_channel,processed_signal))&&
									(cardiac_interval_device=create_Device(0,
									cardiac_interval_description,cardiac_interval_channel,
									cardiac_interval_signal)))
								{
									trace->real_device_1=real_device_1;
									trace->imaginary_device_1=imaginary_device_1;
									trace->real_device_2=real_device_2;
									trace->imaginary_device_2=imaginary_device_2;
									trace->processed_device=processed_device;
									trace->cardiac_interval_device=cardiac_interval_device;
								}
								else
								{
									display_message(ERROR_MESSAGE,
						"create_Trace_window.  Could not create spectral analysis devices");
									if (real_device_1)
									{
										destroy_Device(&real_device_1);
										if (imaginary_device_1)
										{
											destroy_Device(&imaginary_device_1);
											if (real_device_2)
											{
												destroy_Device(&real_device_2);
												if (imaginary_device_2)
												{
													destroy_Device(&imaginary_device_2);
												}
											}
										}
									}
									destroy_Device_description(&real_description_1);
									destroy_Device_description(&imaginary_description_1);
									destroy_Device_description(&real_description_2);
									destroy_Device_description(&imaginary_description_2);
									destroy_Device_description(&processed_description);
									destroy_Device_description(&cardiac_interval_description);
									destroy_Signal(&real_signal_1);
									destroy_Signal(&imaginary_signal_1);
									destroy_Signal(&real_signal_2);
									destroy_Signal(&imaginary_signal_2);
									destroy_Signal(&(processed_signal->next));
									destroy_Signal(&processed_signal);
									destroy_Signal(&cardiac_interval_signal);
									destroy_Channel(&real_channel_1);
									destroy_Channel(&imaginary_channel_1);
									destroy_Channel(&real_channel_2);
									destroy_Channel(&imaginary_channel_2);
									destroy_Channel(&processed_channel);
									destroy_Channel(&cardiac_interval_channel);
									destroy_Signal_buffer(&frequency_domain_buffer_1);
									destroy_Signal_buffer(&frequency_domain_buffer_2);
									destroy_Signal_buffer(&processed_buffer);
									destroy_Signal_buffer(&cardiac_interval_buffer);
									trace->real_device_1=(struct Device *)NULL;
									trace->imaginary_device_1=(struct Device *)NULL;
									trace->real_device_2=(struct Device *)NULL;
									trace->imaginary_device_2=(struct Device *)NULL;
									trace->processed_device=(struct Device *)NULL;
									trace->cardiac_interval_device=(struct Device *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
"create_Trace_window.  Could not create spectral analysis device descriptions");
								if (real_description_1)
								{
									destroy_Device_description(&real_description_1);
									if (imaginary_description_1)
									{
										destroy_Device_description(&imaginary_description_1);
										if (real_description_2)
										{
											destroy_Device_description(&real_description_2);
											if (imaginary_description_2)
											{
												destroy_Device_description(&imaginary_description_2);
											}
										}
									}
								}
								destroy_Signal(&real_signal_1);
								destroy_Signal(&imaginary_signal_1);
								destroy_Signal(&real_signal_2);
								destroy_Signal(&imaginary_signal_2);
								destroy_Signal(&(processed_signal->next));
								destroy_Signal(&processed_signal);
								destroy_Signal(&cardiac_interval_signal);
								destroy_Channel(&real_channel_1);
								destroy_Channel(&imaginary_channel_1);
								destroy_Channel(&real_channel_2);
								destroy_Channel(&imaginary_channel_2);
								destroy_Channel(&processed_channel);
								destroy_Channel(&cardiac_interval_channel);
								destroy_Signal_buffer(&frequency_domain_buffer_1);
								destroy_Signal_buffer(&frequency_domain_buffer_2);
								destroy_Signal_buffer(&processed_buffer);
								destroy_Signal_buffer(&cardiac_interval_buffer);
								trace->real_device_1=(struct Device *)NULL;
								trace->imaginary_device_1=(struct Device *)NULL;
								trace->real_device_2=(struct Device *)NULL;
								trace->imaginary_device_2=(struct Device *)NULL;
								trace->processed_device=(struct Device *)NULL;
								trace->cardiac_interval_device=(struct Device *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
						"create_Trace_window.  Could not create spectral analysis signals");
							if (real_signal_1)
							{
								destroy_Signal(&real_signal_1);
								if (imaginary_signal_1)
								{
									destroy_Signal(&imaginary_signal_1);
									if (real_signal_2)
									{
										destroy_Signal(&real_signal_2);
										if (imaginary_signal_2)
										{
											destroy_Signal(&imaginary_signal_2);
											if (processed_signal)
											{
												destroy_Signal(&processed_signal);
												if (cardiac_interval_signal)
												{
													destroy_Signal(&cardiac_interval_signal);
												}
											}
										}
									}
								}
							}
							destroy_Channel(&real_channel_1);
							destroy_Channel(&imaginary_channel_1);
							destroy_Channel(&real_channel_2);
							destroy_Channel(&imaginary_channel_2);
							destroy_Channel(&processed_channel);
							destroy_Channel(&cardiac_interval_channel);
							destroy_Signal_buffer(&frequency_domain_buffer_1);
							destroy_Signal_buffer(&frequency_domain_buffer_2);
							destroy_Signal_buffer(&processed_buffer);
							destroy_Signal_buffer(&cardiac_interval_buffer);
							trace->real_device_1=(struct Device *)NULL;
							trace->imaginary_device_1=(struct Device *)NULL;
							trace->real_device_2=(struct Device *)NULL;
							trace->imaginary_device_2=(struct Device *)NULL;
							trace->processed_device=(struct Device *)NULL;
							trace->cardiac_interval_device=(struct Device *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"create_Trace_window.  Could not create spectral analysis channels");
						if (real_channel_1)
						{
							destroy_Channel(&real_channel_1);
							if (imaginary_channel_1)
							{
								destroy_Channel(&imaginary_channel_1);
								if (real_channel_2)
								{
									destroy_Channel(&real_channel_2);
									if (imaginary_channel_2)
									{
										destroy_Channel(&imaginary_channel_2);
									}
								}
							}
						}
						destroy_Signal_buffer(&frequency_domain_buffer_1);
						destroy_Signal_buffer(&frequency_domain_buffer_2);
						destroy_Signal_buffer(&processed_buffer);
						destroy_Signal_buffer(&cardiac_interval_buffer);
						trace->real_device_1=(struct Device *)NULL;
						trace->imaginary_device_1=(struct Device *)NULL;
						trace->real_device_2=(struct Device *)NULL;
						trace->imaginary_device_2=(struct Device *)NULL;
						trace->processed_device=(struct Device *)NULL;
						trace->cardiac_interval_device=(struct Device *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Trace_window.  Could not create spectral analysis buffers");
					if (frequency_domain_buffer_1)
					{
						destroy_Signal_buffer(&frequency_domain_buffer_1);
						if (frequency_domain_buffer_2)
						{
							destroy_Signal_buffer(&frequency_domain_buffer_2);
						}
					}
					trace->real_device_1=(struct Device *)NULL;
					trace->imaginary_device_1=(struct Device *)NULL;
					trace->real_device_2=(struct Device *)NULL;
					trace->imaginary_device_2=(struct Device *)NULL;
					trace->processed_device=(struct Device *)NULL;
					trace->cardiac_interval_device=(struct Device *)NULL;
				}
				trace->correlation.signal_1_input=1;
				if (highlight&&(*highlight))
				{
					trace->correlation.device_1= **highlight;
					trace->correlation.device_2= **highlight;
				}
				else
				{
					trace->correlation.device_1=(struct Device *)NULL;
					trace->correlation.device_2=(struct Device *)NULL;
				}
				/* set the edit interval */
				switch (*detection)
				{
					case EDA_INTERVAL:
					{
						trace_update_edit_interval(trace);
					} break;
					case EDA_LEVEL:
					case EDA_THRESHOLD:
					{
						trace->area_3.edit.first_data= *start_search_interval;
						trace->area_3.edit.last_data= *end_search_interval;
					} break;
				}
				/* register the callbacks */
				if ((MrmSUCCESS==MrmRegisterNamesInHierarchy(trace_window_hierarchy,
					callback_list,XtNumber(callback_list))))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)trace;
					identifier_list[1].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(trace_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch the trace window widget */
						if (MrmSUCCESS==MrmFetchWidget(trace_window_hierarchy,
							"trace_window",parent,&(trace->window),&trace_window_class))
						{
#if defined (SPECTRAL_TOOLS)
							/* set the analysis mode */
							switch (*analysis_mode)
							{
								case EVENT_DETECTION:
								{
#else /* defined (SPECTRAL_TOOLS) */
									XtUnmanageChild(trace->menu.analysis_mode_choice);
#endif /* defined (SPECTRAL_TOOLS) */
									XtUnmanageChild(trace->menu.apply_button);
									XtVaSetValues(trace->menu.average_width,
										XmNleftWidget,trace->menu.analysis_mode_choice,
										NULL);
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									XtUnmanageChild(trace->area_3.beat_averaging.menu);
									/* set the height of and the top widget for the drawing
										area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNheight,screen_height/8,
										XmNtopWidget,trace->area_1.enlarge.menu,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.edit.menu,
										NULL);
#if defined (SPECTRAL_TOOLS)
								} break;
								case FREQUENCY_DOMAIN:
								{
									XtUnmanageChild(trace->menu.apply_button);
									XtVaSetValues(trace->menu.average_width,
										XmNleftWidget,trace->menu.analysis_mode_choice,
										NULL);
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									XtUnmanageChild(trace->area_3.beat_averaging.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopAttachment,XmATTACH_FORM,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.frequency_domain.menu,
										NULL);
								} break;
								case POWER_SPECTRA:
								{
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									XtUnmanageChild(trace->area_3.beat_averaging.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopAttachment,XmATTACH_FORM,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.power_spectra.menu,
										NULL);
								} break;
								case CROSS_CORRELATION:
								{
									XtUnmanageChild(trace->menu.apply_button);
									XtVaSetValues(trace->menu.average_width,
										XmNleftWidget,trace->menu.analysis_mode_choice,
										NULL);
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopWidget,trace->area_1.correlation_time_domain.menu,
										NULL);
									/* set the top widget for the drawing area 2 */
									XtVaSetValues(trace->area_2.drawing_area,
										XmNtopWidget,trace->area_2.correlation_time_domain.menu,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.correlation.menu,
										NULL);
								} break;
								case AUTO_CORRELATION:
								{
									XtUnmanageChild(trace->menu.apply_button);
									XtVaSetValues(trace->menu.average_width,
										XmNleftWidget,trace->menu.analysis_mode_choice,
										NULL);
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									XtUnmanageChild(trace->area_3.beat_averaging.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopAttachment,XmATTACH_FORM,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.correlation.menu,
										NULL);
								} break;
								case FILTERING:
								{
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_1.beat_averaging.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.beat_averaging.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopAttachment,XmATTACH_FORM,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.filtering.menu,
										NULL);
								} break;
								case BEAT_AVERAGING:
								{
									XtUnmanageChild(trace->area_1.inverse.menu);
									XtUnmanageChild(trace->area_1.calculate.menu);
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_2.pane);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopAttachment,XmATTACH_WIDGET,
										XmNtopWidget,trace->area_1.calculate.menu,NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.interval.menu,
										NULL);
								} break;
								case ELECTRICAL_IMAGING:
								{
									XtUnmanageChild(trace->area_1.enlarge.menu);
									XtUnmanageChild(trace->area_1.drawing_area);
									XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
									XtUnmanageChild(trace->area_3.edit.menu);
									XtUnmanageChild(trace->area_3.frequency_domain.menu);
									XtUnmanageChild(trace->area_3.power_spectra.menu);
									XtUnmanageChild(trace->area_3.correlation.menu);
									XtUnmanageChild(trace->area_3.filtering.menu);
									/* set the top widget for the drawing area 1 */
									XtVaSetValues(trace->area_1.drawing_area,
										XmNtopWidget,trace->area_1.inverse.menu,
										NULL);
									/* set the top widget for the drawing area 3 */
									XtVaSetValues(trace->area_3.drawing_area,
										XmNtopWidget,trace->area_3.beat_averaging.menu,
										NULL);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Trace_window.  Invalid analysis mode");
								} break;
							}
#endif /* defined (SPECTRAL_TOOLS) */
							/* set the background colour for the drawing areas */
							XtVaSetValues(trace->area_1.drawing_area,XmNbackground,
								signal_drawing_information->background_drawing_colour,NULL);
							XtVaSetValues(trace->area_2.drawing_area,XmNbackground,
								signal_drawing_information->background_drawing_colour,NULL);
							XtVaSetValues(trace->area_3.drawing_area,XmNbackground,
								signal_drawing_information->background_drawing_colour,NULL);
							/* set the filtering notch toggle */
							XtVaSetValues(trace->area_3.filtering.notch_toggle,
								XmNset,False,
								NULL);
							trace->filtering.notch_on=0;
							/* set the cross correlation toggles */
							XtVaSetValues(trace->area_1.correlation_time_domain.toggle,
								XmNset,True,
								NULL);
							XtVaSetValues(trace->area_2.correlation_time_domain.toggle,
								XmNset,False,
								NULL);
							/* adjust the analysis mode choice */
							child_widget=
								XmOptionLabelGadget(trace->menu.analysis_mode_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->menu.analysis_mode_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the inverse electrodes mode choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.inverse.electrodes_choice_mode);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.inverse.electrodes_choice_mode);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the inverse wave mode choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.inverse.wave_choice_mode);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.inverse.wave_choice_mode );
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the inverse potential/activation mode choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.inverse.pot_act_choice_mode);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.inverse.pot_act_choice_mode );
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the interval/rank cutoffmode choice */
							child_widget=
								XmOptionLabelGadget(
									trace->area_1.calculate.interval_rank_cutoff_mode_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(
									trace->area_1.calculate.interval_rank_cutoff_mode_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the RMS current signal mode choice */
							child_widget=
								XmOptionLabelGadget(
									trace->area_1.calculate.RMS_current_mode_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(
									trace->area_1.calculate.RMS_current_mode_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the frequency domain display mode choice */
							child_widget=XmOptionLabelGadget(
								trace->area_3.frequency_domain.display_mode_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=XmOptionButtonGadget(
								trace->area_3.frequency_domain.display_mode_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* set the time domain window type */
							XtVaSetValues(trace->area_3.frequency_domain.window_type_choice,
								XmNmenuHistory,
								trace->area_3.frequency_domain.window_type_menu.square_button,
								NULL);
							trace->frequency_domain.window_type=SQUARE_WINDOW;
							/* adjust the time domain window type choice */
							child_widget=XmOptionLabelGadget(
								trace->area_3.frequency_domain.window_type_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=XmOptionButtonGadget(
								trace->area_3.frequency_domain.window_type_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionLabelGadget(trace->area_1.enlarge.detection_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.enlarge.detection_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the enlarge objective choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.enlarge.objective_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.enlarge.objective_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginRight,&right_margin,
								NULL);
							if (right_margin>widget_spacing)
							{
								XtVaSetValues(trace->area_1.enlarge.number_of_events_form,
									XmNleftOffset,0,
									NULL);
								XtVaSetValues(trace->area_1.enlarge.threshold_scroll,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(trace->area_1.enlarge.number_of_events_form,
									XmNleftOffset,widget_spacing-right_margin,
									NULL);
								XtVaSetValues(trace->area_1.enlarge.threshold_scroll,
									XmNleftOffset,widget_spacing-right_margin,
									NULL);
							}
							/* adjust the all/current choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.enlarge.all_current_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.enlarge.all_current_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginRight,&right_margin,
								NULL);
							/* set the threshold */
							sprintf(value_string,"000%%");
							value_xmstring=XmStringCreateSimple(value_string);
							XtVaSetValues(trace->area_1.enlarge.threshold_label,
								XmNlabelString,value_xmstring,
								NULL);
							XmStringFree(value_xmstring);
							XtVaSetValues(trace->area_1.enlarge.threshold_label,
								XmNrecomputeSize,False,
								NULL);
							sprintf(value_string,"%3d%%",*threshold);
							value_xmstring=XmStringCreateSimple(value_string);
							XtVaSetValues(trace->area_1.enlarge.threshold_label,
								XmNlabelString,value_xmstring,
								NULL);
							XmStringFree(value_xmstring);
							XtVaSetValues(trace->area_1.enlarge.threshold_scroll,
								XmNvalue,*threshold,
								NULL);
							/* set the minimum separation */
							sprintf(value_string,"000 ms");
							value_xmstring=XmStringCreateSimple(value_string);
							XtVaSetValues(trace->area_1.enlarge.minimum_separation_label,
								XmNlabelString,value_xmstring,
								NULL);
							XmStringFree(value_xmstring);
							XtVaSetValues(trace->area_1.enlarge.minimum_separation_label,
								XmNrecomputeSize,False,
								NULL);
							sprintf(value_string,"%3d ms",*minimum_separation);
							value_xmstring=XmStringCreateSimple(value_string);
							XtVaSetValues(trace->area_1.enlarge.minimum_separation_label,
								XmNlabelString,value_xmstring,
								NULL);
							XmStringFree(value_xmstring);
							XtVaSetValues(trace->area_1.enlarge.minimum_separation_scroll,
								XmNvalue,*minimum_separation,
								NULL);
							/* set the level */
							sprintf(value_string,"%g",*level);
							XtVaSetValues(trace->area_1.enlarge.level_value,
								XmNvalue,value_string,
								NULL);
							/* set the level width */
							sprintf(value_string,"%d",*average_width);
							XtVaSetValues(trace->menu.average_width_txt,
								XmNvalue,value_string,
								NULL);
							/* adjust the enlarge datum choice */
							child_widget=
								XmOptionLabelGadget(trace->area_1.enlarge.datum_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							child_widget=
								XmOptionButtonGadget(trace->area_1.enlarge.datum_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* set the method for determining datum */
							switch (*datum_type)
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
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Trace_window.  Invalid datum method");
								} break;
							}
							/* set the event detection algorithm */
							switch (*detection)
							{
								case EDA_INTERVAL:
								{
									XtVaSetValues(trace->area_1.enlarge.detection_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.detection.interval_button,NULL);
									XtVaSetValues(trace->area_1.enlarge.calculate_button,
										XmNleftWidget,trace->area_1.enlarge.number_of_events_form,
										NULL);
									if (left_margin>widget_spacing)
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.calculate_button,
											XmNleftOffset,0,
											NULL);
									}
									else
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.calculate_button,
											XmNleftOffset,widget_spacing-left_margin,
											NULL);
									}
									XtUnmanageChild(trace->area_1.enlarge.threshold_scroll);
									XtUnmanageChild(trace->area_1.enlarge.threshold_label);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_scroll);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_label);
									XtUnmanageChild(trace->area_1.enlarge.all_current_choice);
									XtUnmanageChild(trace->area_1.enlarge.level_value);
								} break;
								case EDA_LEVEL:
								{
									XtVaSetValues(trace->area_1.enlarge.detection_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.detection.level_button,NULL);
									XtVaSetValues(trace->area_1.enlarge.calculate_button,
										XmNleftWidget,trace->area_1.enlarge.level_value,
										NULL);
									if (left_margin>widget_spacing)
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.calculate_button,
											XmNleftOffset,0,
											NULL);
									}
									else
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.calculate_button,
											XmNleftOffset,widget_spacing-left_margin,
											NULL);
									}
									XtUnmanageChild(trace->area_1.enlarge.number_of_events_form);
									XtUnmanageChild(trace->area_1.enlarge.threshold_scroll);
									XtUnmanageChild(trace->area_1.enlarge.threshold_label);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_scroll);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_label);
									XtUnmanageChild(trace->area_1.enlarge.all_current_choice);
									XtUnmanageChild(trace->area_1.enlarge.objective_choice);
								} break;
								case EDA_THRESHOLD:
								{
									XtVaSetValues(trace->area_1.enlarge.detection_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.detection.threshold_button,NULL);
									XtVaSetValues(trace->area_1.enlarge.calculate_button,
										XmNleftWidget,
										trace->area_1.enlarge.minimum_separation_label,NULL);
									if (left_margin+right_margin>widget_spacing)
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.all_current_choice,
											XmNleftOffset,0,
											NULL);
									}
									else
									{
										XtVaSetValues(trace->area_1.enlarge.datum_choice,
											XmNleftWidget,trace->area_1.enlarge.all_current_choice,
											XmNleftOffset,widget_spacing-(left_margin+right_margin),
											NULL);
									}
									XtUnmanageChild(trace->area_1.enlarge.number_of_events_form);
									XtUnmanageChild(trace->area_1.enlarge.level_value);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Trace_window.  Invalid detection algorithm");
								} break;
							}
							/* set the objective for the event detection algorithm */
							switch (*objective)
							{
								case ABSOLUTE_SLOPE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.absolute_slope_button,NULL);
								} break;
								case POSITIVE_SLOPE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.positive_slope_button,NULL);
								} break;
								case NEGATIVE_SLOPE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.negative_slope_button,NULL);
								} break;
								case ABSOLUTE_VALUE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.absolute_value_button,NULL);
								} break;
								case POSITIVE_VALUE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.positive_value_button,NULL);
								} break;
								case NEGATIVE_VALUE:
								{
									XtVaSetValues(trace->area_1.enlarge.objective_choice,
										XmNmenuHistory,
										trace->area_1.enlarge.objective.negative_value_button,NULL);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Trace_window.  Invalid objective");
								} break;
							}
							/* adjust the edit order choice */
							child_widget=XmOptionLabelGadget(trace->area_3.edit.order_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							if (left_margin>widget_spacing)
							{
								XtVaSetValues(trace->area_3.edit.order_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(trace->area_3.edit.order_choice,
									XmNleftOffset,widget_spacing-left_margin,
									NULL);
							}
							child_widget=
								XmOptionButtonGadget(trace->area_3.edit.order_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* set the edit order */
							switch (*edit_order)
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
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Trace_window.  Invalid edit order");
								} break;
							}
							/* set the number of events */
							sprintf(number_string,"%1d",*number_of_events);
							XtVaSetValues(trace->area_1.enlarge.number_of_events.label,
								XmNlabelString,XmStringCreate(number_string,
								XmSTRING_DEFAULT_CHARSET),
								NULL);
							XtVaSetValues(trace->area_1.beat_averaging.number_of_beats.label,
								XmNlabelString,XmStringCreate(number_string,
								XmSTRING_DEFAULT_CHARSET),
								NULL);
							/* set the background colour for the accept and reject buttons */
							XtVaSetValues(trace->area_3.edit.accept_button,
								XmNbackground,signal_drawing_information->accepted_colour,
								NULL);
							XtVaSetValues(trace->area_3.edit.reject_button,
								XmNbackground,signal_drawing_information->rejected_colour,
								NULL);
							if (rig&&(*rig)&&highlight&&(*highlight)&&(**highlight))
							{
								if ((signal=(**highlight)->signal)&&(signal->first_event))
								{
									/* unghost the accept and reject buttons */
									XtSetSensitive(trace->area_3.edit.accept_button,True);
									XtSetSensitive(trace->area_3.edit.reject_button,True);
								}
								else
								{
									/* ghost the accept and reject buttons */
									XtSetSensitive(trace->area_3.edit.accept_button,False);
									XtSetSensitive(trace->area_3.edit.reject_button,False);
								}
								if (current_region=(*rig)->current_region)
								{
									device_number=0;
									device=(*rig)->devices;
									for (i=(*highlight)-device;i>0;i--)
									{
										if (current_region==(*device)->description->region)
										{
											device_number++;
										}
										device++;
									}
								}
								else
								{
									device_number=(*highlight)-((*rig)->devices);
								}
								if (((!current_region&&(device_number==((*rig)->
									number_of_devices)-1))||(current_region&&(device_number==
									current_region->number_of_devices-1)))&&
									(*(trace->event_detection.event_number)==
									*(trace->event_detection.number_of_events)))
								{
									XtSetSensitive(trace->area_3.edit.next_button,False);
								}
								else
								{
									XtSetSensitive(trace->area_3.edit.next_button,True);
								}
								if ((0==device_number)&&
									(1== *(trace->event_detection.event_number)))
								{
									XtSetSensitive(trace->area_3.edit.previous_button,False);
								}
								else
								{
									XtSetSensitive(trace->area_3.edit.previous_button,True);
								}
							}
							else
							{
								/* ghost the accept and reject buttons */
								XtSetSensitive(trace->area_3.edit.accept_button,False);
								XtSetSensitive(trace->area_3.edit.reject_button,False);
								/* ghost the next and previous buttons */
								XtSetSensitive(trace->area_3.edit.next_button,False);
								XtSetSensitive(trace->area_3.edit.previous_button,False);
							}
							install_accelerators(trace->window,trace->window);
							/*??? more to do ? */
							if (address)
							{
								*address=trace;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Trace_window.  Could not fetch trace window widget");
							DEALLOCATE(trace);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Trace_window.  Could not register the identifiers");
						DEALLOCATE(trace);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Trace_window.  Could not register the callbacks");
					DEALLOCATE(trace);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Trace_window.  Could not allocate trace window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Trace_window.  Invalid argument(s)");
			trace=(struct Trace_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Trace_window.  Invalid argument(s)");
		trace=(struct Trace_window *)NULL;
	}
	LEAVE;

	return (trace);
} /* create_Trace_window */

static Widget create_trace_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for an trace window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
???If address is NULL, then it won't be added to the shell list ?
???DB.  screen_width and screen_height should be replaced by shell dimensions
and location.
==============================================================================*/
{
	Widget shell;

	ENTER(create_trace_window_shell);
	/* create and place the trace window shell */
	if (shell=XtVaCreatePopupShell("trace_window_shell",
		topLevelShellWidgetClass,parent,
		XmNallowShellResize,False,
		XmNx,0,
		XmNy,0,
		XmNwidth,screen_width/2,
		XmNheight,screen_height/2,
		XmNuserData,user_interface,
		NULL))
	{
		if (address)
		{
			*address=shell;
			/* add the destroy callback */
			XtAddCallback(shell,XmNdestroyCallback,destroy_window_shell,
				(XtPointer)create_Shell_list_item(address,user_interface));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_trace_window_shell.  Could not create the trace window shell");
	}
	LEAVE;

	return (shell);
} /* create_trace_window_shell */

static int process_eimaging(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Calculates the processed device for electrical imaging.
Set the trace->calculate_rms flag to 1 before calling this function if you
want the RMS of all the signals to be calculated. This flag is cleared
before this function is exited.
==============================================================================*/
{
	enum Calculate_signal_mode calculate_signal_mode;
	enum Event_signal_status **status_ptr,*status;
	enum Inverse_electrodes_mode electrodes_mode;
	float *current_times,*current_values,maximum_value,minimum_value,
		processed_frequency,*processed_value,*source_value,*start_value,*time_float,
		*value,*values;
	int buffer_offset,end,i,j,number_of_devices,num_valid_devices,
		number_of_samples,number_of_signals,*processed_time,return_code,start;
	struct Device *device,*processed_device,**the_device;
	struct Device_description *description;
	struct Rig *rig;
	struct Region *current_region;
	struct Signal_buffer *processed_buffer;
	struct Signal *signal_next,*signal_next_new;
#if defined (DEVICE_EXPRESSIONS)
	struct Signal_buffer *buffer;
#else /* defined (DEVICE_EXPRESSIONS) */
	struct Device *a_device,**electrodes;
#endif /* defined (DEVICE_EXPRESSIONS) */

	ENTER(process_eimaging);
	device=(struct Device *)NULL;
	processed_device=(struct Device *)NULL;
	the_device=(struct Device **)NULL;
#if !defined (DEVICE_EXPRESSIONS)
	electrodes=(struct Device **)NULL;
	a_device=(struct Device *)NULL;
#endif /* !defined (DEVICE_EXPRESSIONS) */
	values=(float *)NULL;
	current_values=(float *)NULL;
	current_times=(float *)NULL;
	rig=(struct Rig *)NULL;
	current_region=(struct Region *)NULL;
	description=(struct Device_description *)NULL;
	signal_next=(struct Signal *)NULL;
	signal_next_new=(struct Signal *)NULL;
	processed_value=(float *)NULL;
	source_value=(float *)NULL;
	start_value=(float *)NULL;
	time_float=(float *)NULL;
	value=(float *)NULL;
	processed_time=(int *)NULL;
	processed_buffer=(struct Signal_buffer *)NULL;
	return_code=0;
	if (trace)
	{
		calculate_signal_mode=trace->calculate_signal_mode;
		electrodes_mode=trace->inverse_electrodes_mode;
		if (calculate_signal_mode!=CURRENT_SIGNAL)
			/* nothing to do for CURRENT_SIGNAL case  */
		{
			if ((trace->highlight)&&(*(trace->highlight))&&
				(device= **(trace->highlight))&&
				(processed_device=trace->processed_device)&&
				(processed_device->signal)&&(processed_device->channel)&&
				(processed_buffer=processed_device->signal->buffer))
			{
				if (calculate_signal_mode==RMS_SIGNAL)
				{
					destroy_Signal(&(processed_device->signal->next));
				}
				else
				{
					/* calculate_signal_mode==RMS_CURRENT_SIGNAL*/
					if (signal_next=processed_device->signal->next)
					{
						signal_next_new=(struct Signal *)NULL;
					}
					else
					{
						signal_next_new=create_Signal(1,processed_buffer,REJECTED,2);
						signal_next=signal_next_new;
					}
					processed_device->signal->next=signal_next;
					signal_next->buffer=processed_buffer;
				}
				start=0;
#if defined (DEVICE_EXPRESSIONS)
				if (buffer=get_Device_signal_buffer(device))
				{
					end=(buffer->number_of_samples)-1;
				}
				else
				{
					end=0;
				}
#else /* defined (DEVICE_EXPRESSIONS) */
				if (AUXILIARY==device->description->type)
				{
					electrodes=(device->description->properties).auxiliary.electrodes;
					if (electrodes)
					{
						a_device=(*electrodes);
					}
					else
					{
						a_device=device;
					}
				}
				else
				{
					a_device=device;
				}
				end=a_device->signal->buffer->number_of_samples-1;
#endif /* defined (DEVICE_EXPRESSIONS) */
				/* get info from the highlighted signal */
				if (((signal_next)||(calculate_signal_mode==RMS_SIGNAL))&&
					extract_signal_information((struct FE_node *)NULL,
					(struct Signal_drawing_package *)NULL,device,1,
					start,end,(int *)NULL,&number_of_samples,&current_times,
					&current_values,(enum Event_signal_status **)NULL,(char **)NULL,
					(int *)NULL,(float *)NULL,(float *)NULL)&&(0<number_of_samples))
				{
					if (calculate_signal_mode==RMS_AND_CURRENT_SIGNAL)
					{
						/* current signal and rms signal*/
						number_of_signals=2;
					}
					else
					{ /* rms signal*/
						number_of_signals=1;
					}
					/* realloc Signal_buffer for processed_device */
					processed_frequency=(float)number_of_samples/
						(current_times[number_of_samples-1]-current_times[0]);
					if (processed_buffer=
						reallocate_Signal_buffer(processed_buffer,FLOAT_VALUE,
							number_of_signals,number_of_samples,processed_frequency))
					{
						time_float=current_times;
						processed_time=processed_buffer->times;
						buffer_offset=processed_buffer->number_of_signals;
						processed_device->channel->offset=0;
						processed_device->channel->gain=1;
						/* might have no relevant signals */
						trace->valid_processing=1;
						return_code=1;
						if (trace->calculate_rms)
						{
							if (calculate_signal_mode==RMS_AND_CURRENT_SIGNAL)
							{
								/* put the rms signal in the processed_device->signal->next*/
								start_value=((processed_buffer->signals).float_values)+
									(processed_device->signal->next->index);
							}
							else
							{
								/* RMS_SIGNAL put the rms signal in the
									processed_device->signal */
								start_value=((processed_buffer->signals).float_values)+
									(processed_device->signal->index);
							}
							/* zero all the processed values, and copy the processed times*/
							processed_value=start_value;
							for (j=number_of_samples;j>0;j--)
							{
								*processed_value=0;
								processed_value += buffer_offset;
								*processed_time=(int)((*time_float)*processed_frequency+0.5);
								processed_time++;
								time_float++;
							}
							/* find all the relevant devices, to get RMS of them */
							rig=*(trace->rig);
							current_region=get_Rig_current_region(rig);
							number_of_devices=rig->number_of_devices;
							the_device=rig->devices;
							num_valid_devices=0;
							status_ptr=&status;
							/* now need to get RMS and shove in processed signal*/
							/* sum the square of the sample values  */
							for (i=0;i<number_of_devices;i++)
							{
								description=(*the_device)->description;
								if ((!current_region||(current_region==description->region))&&
									(extract_signal_information((struct FE_node *)NULL,
									(struct Signal_drawing_package *)NULL,*the_device,1,
									start,end,(int *)NULL,&number_of_samples,(float **)NULL,
									&values,status_ptr,(char **)NULL,(int *)NULL,(float *)NULL,
									(float *)NULL)))
								{
									if (((ACCEPTED== *status)&&
										(ELECTRODES_ACCEPTED==electrodes_mode))||
										(ELECTRODES_ALL==electrodes_mode)||((REJECTED!= *status)&&
										(ELECTRODES_UNREJECTED==electrodes_mode)))
									{
										processed_value=start_value;
										source_value=values;
										for (j=number_of_samples;j>0;j--)
										{
											*processed_value += (*source_value)*(*source_value);
											processed_value += buffer_offset;
											source_value++;
										}
										num_valid_devices++;
									}
									DEALLOCATE(status);
									DEALLOCATE(values)
								}
								the_device++;
							}
							/* calculate rms, and find maximum */
							if (num_valid_devices)
							{
								processed_value=start_value;
								maximum_value=sqrt(*processed_value/num_valid_devices);
								minimum_value=maximum_value;
								for (j=number_of_samples;j>0;j--)
								{
									*processed_value/=num_valid_devices;
									*processed_value=sqrt(*processed_value);
									if (*processed_value>maximum_value)
									{
										maximum_value=*processed_value;
									}
									if (*processed_value<minimum_value)
									{
										minimum_value=*processed_value;
									}
									processed_value += buffer_offset;
								}
							}
							/* ensure the range of rms is 0 to max; min to max looks
								confusing, as rms is all positive. */
							if (calculate_signal_mode==RMS_SIGNAL)
							{
								processed_device->signal_display_minimum=0;
								processed_device->signal_display_maximum=maximum_value;
							}

						}	/* if (trace->calculate_rms)	*/
						if (calculate_signal_mode==RMS_AND_CURRENT_SIGNAL)
						/* put current signal in the processed_device->signal */
						/* (processed_device->signal->next is already the rms)*/
						{
							buffer_offset=processed_buffer->number_of_signals;
							value=current_values;
							processed_value=((processed_buffer->signals).float_values)+
								(processed_device->signal->index);
							for (j=number_of_samples;j>0;j--)
							{
								*processed_value= *value;
								processed_value += buffer_offset;
								value++;
							}

							processed_device->signal_display_maximum=0;
							processed_device->signal_display_minimum=1;
						}
#if defined (DEVICE_EXPRESSIONS)
						if (buffer=get_Device_signal_buffer(device))
						{
							processed_buffer->start=buffer->start;
							processed_buffer->end=buffer->end;
						}
						else
						{
							processed_buffer->start=0;
							processed_buffer->end=0;
						}
#else /* defined (DEVICE_EXPRESSIONS) */
						if (AUXILIARY==device->description->type)
						{
							electrodes=(device->description->properties).auxiliary.electrodes;
							if (electrodes)
							{
								a_device=(*electrodes);
							}
							else
							{
								a_device=device;
							}

						}
						else
						{
							a_device=device;
						}
						processed_buffer->start=a_device->signal->buffer->start;
						processed_buffer->end=a_device->signal->buffer->end;
#endif /* defined (DEVICE_EXPRESSIONS) */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"process_eimaging. reallocate_Signal_buffer failed");
						return_code=0;
					}
				}
				else
				{
					if (signal_next)
					{
						destroy_Signal(&signal_next);
					}
					display_message(ERROR_MESSAGE,
						"process_eimaging.  Could not reallocate processed buffer");
					trace->valid_processing=0;
				}
			} /* if ((trace->highlight)&&( */
		}/* if (calculate_signal_mode!=CURRENT_SIGNAL) */
		if (current_times)
		{
			DEALLOCATE(current_times);
		}
		if (current_values)
		{
			DEALLOCATE(current_values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_eimaging. Invalid arguments");
		return_code=0;
	}
	trace->calculate_rms=0; /*clear flag*/
	LEAVE;
	
	return (return_code);
} /* process_eimaging */

/*
Global functions
----------------
*/

int trace_process_device(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 21 April 2004

DESCRIPTION :
Calculates the processed device.
==============================================================================*/
{
	int return_code;

	ENTER(trace_process_device);
	return_code=0;
	if (trace)
	{
		switch (*(trace->analysis_mode))
		{
			case ELECTRICAL_IMAGING:
			{
				return_code=process_eimaging(trace);
			} break;
			case EVENT_DETECTION:
			{
				char *name;
				enum Event_signal_status *signals_status;
				float maximum,minimum,processed_frequency,*processed_value,*time_float,
					*times,*value,*values;
				int buffer_offset,highlight,i,number_of_samples,*processed_time;
				struct Device *device,*processed_device;
				struct Signal *signal_next,*signal_next_new;
				struct Signal_buffer *processed_buffer;

				values=(float *)NULL;
				times=(float *)NULL;
				signals_status=(enum Event_signal_status *)NULL;
				name=(char *)NULL;
				/* extract_signal_information with first_data>last_data returns the
					buffer from start to end.  Here, the whole buffer is required */
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(device->signal)&&
					(device->signal->buffer)&&extract_signal_information(
					(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,device,
					1,0,(device->signal->buffer->number_of_samples)-1,
					(int *)NULL,&number_of_samples,&times,&values,&signals_status,&name,
					&highlight,&minimum,&maximum)&&(0<number_of_samples)&&
					(processed_device=trace->processed_device)&&
					(processed_device->signal)&&
					(processed_buffer=processed_device->signal->buffer)&&
					(processed_device->channel))
				{
					if (signal_next=processed_device->signal->next)
					{
						signal_next_new=(struct Signal *)NULL;
					}
					else
					{
						signal_next_new=create_Signal(1,processed_buffer,REJECTED,2);
						signal_next=signal_next_new;
					}
					if (signal_next)
					{
						processed_frequency=(float)number_of_samples/
							(times[number_of_samples-1]-times[0]);
						if (processed_buffer=reallocate_Signal_buffer(processed_buffer,
							FLOAT_VALUE,2,number_of_samples,processed_frequency))
						{
							processed_device->signal->buffer=processed_buffer;
							processed_device->signal->next=signal_next;
							signal_next->buffer=processed_buffer;
							/* copy the values */
							time_float=times;
							value=values;
							processed_time=processed_buffer->times;
							processed_value=((processed_buffer->signals).float_values)+
								(processed_device->signal->index);
							buffer_offset=processed_buffer->number_of_signals;
							processed_device->channel->offset=0;
							processed_device->channel->gain=1;
							processed_device->signal_display_maximum=maximum;
							processed_device->signal_display_minimum=minimum;
							for (i=number_of_samples;i>0;i--)
							{
								*processed_value= *value;
								processed_value += buffer_offset;
								value++;
								*processed_time=(int)((*time_float)*processed_frequency+0.5);
								processed_time++;
								time_float++;
							}
							processed_device->description->name=name;
							processed_device->description->type=device->description->type;
							processed_device->highlight=highlight;
							processed_device->signal->status=signals_status[0];
							/* calculate objective function */
							calculate_device_objective(processed_device,
								*((trace->event_detection).detection),
								*((trace->event_detection).objective),
								((processed_buffer->signals).float_values)+
								(processed_device->signal->next->index),
								processed_buffer->number_of_samples,
								processed_buffer->number_of_signals,
								*((trace->event_detection).average_width));
							trace->valid_processing=1;
						}
						else
						{
							if (signal_next_new)
							{
								destroy_Signal(&signal_next_new);
							}
							DEALLOCATE(name);
							display_message(ERROR_MESSAGE,
								"trace_process_device.  Could not reallocate processed buffer");
							trace->valid_processing=0;
						}
					}
					else
					{
						DEALLOCATE(name);
						display_message(ERROR_MESSAGE,
							"trace_process_device.  Could not allocate objective signal");
						trace->valid_processing=0;
					}
				}
				else
				{
					trace->valid_processing=0;
				}
				DEALLOCATE(signals_status);
				DEALLOCATE(values);
				DEALLOCATE(times);
				/* put the signal and the objective function in the processed device */
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case BEAT_AVERAGING:
			{
				char *name;
				float *averaged_value,processed_frequency,*processed_value,*time_float,
					*times,*value,*values;
				int average_width,*beat_count,*beat_counts,beat_end,beat_number,
					beat_start,buffer_offset,*divisions,end,i,max_times,number_of_beats,
					number_of_samples,*processed_time,start,start_time,*time;
				struct Device *device,*processed_device;
				struct Event *event;
				struct Signal_buffer *processed_buffer;

				values=(float *)NULL;
				times=(float *)NULL;
				average_width= *((trace->event_detection).average_width);
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(device->signal)&&
					(event=device->signal->first_event)&&extract_signal_information(
					(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,device,1,
					1,0,(int *)NULL,&number_of_samples,&times,&values,
					(enum Event_signal_status **)NULL,&name,(int *)NULL,
					(float *)NULL,(float *)NULL)&&(0<number_of_samples)&&
					(processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->channel)&&
					(processed_buffer=processed_device->signal->buffer))
				{
					/* allow room for the beat averaged signal to be stored after the
						signal */
					if (processed_device->signal->next)
					{
						destroy_Signal(&(processed_device->signal->next));
					}
					processed_frequency=device->signal->buffer->frequency;
#if defined (OLD_CODE)
					processed_frequency=(float)number_of_samples/
						(times[number_of_samples-1]-times[0]);
#endif
					if (processed_buffer=reallocate_Signal_buffer(processed_buffer,
						FLOAT_VALUE,1,2*number_of_samples,processed_frequency))
					{
						/* copy the values */
						time_float=times;
						value=values;
						processed_time=processed_buffer->times;
						processed_value=((processed_buffer->signals).float_values)+
							(trace->processed_device->signal->index);
						buffer_offset=processed_buffer->number_of_signals;
						processed_device->channel->offset=0;
						processed_device->channel->gain=1;
						processed_device->signal_display_maximum=0;
						processed_device->signal_display_minimum=1;
						for (i=number_of_samples;i>0;i--)
						{
							*processed_value= *value;
							processed_value += buffer_offset;
							value++;
							*processed_time=(int)((*time_float)*processed_frequency+0.5);
							processed_time++;
							time_float++;
						}
						/* do any processing */
						if (True==XmToggleButtonGadgetGetState((trace->area_3).
							beat_averaging.baseline_toggle))
						{
							start= *(trace->event_detection.start_search_interval);
							divisions= *(trace->event_detection.search_interval_divisions);
							end= *(trace->event_detection.end_search_interval);
							number_of_beats= *(trace->event_detection.number_of_events);
							return_code=two_end_baseline(processed_buffer,
								number_of_samples,average_width,number_of_beats,start,end,
								divisions);
						}
						if (True==XmToggleButtonGadgetGetState((trace->area_3).
							beat_averaging.beat_averaging_toggle))
						{
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
								beat_count=beat_counts;
								for (i=max_times;i>0;i--)
								{
									*beat_count=0;
									beat_count++;
								}
								/* store the beat averaged signal after the signal */
								processed_time=(processed_buffer->times)+number_of_samples;
								time=(processed_buffer->times)+beat_start;
								/* averaged beat starts from time 0 */
								start_time= *time;
								for (i=max_times;i>0;i--)
								{
									*processed_time=(*time)-start_time;
									processed_time++;
									time++;
								}
								beat_end=start-1;
								event=device->signal->first_event;
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
									/* check if the beat has been rejected by checking the status
										of the first event in the beat interval */
									while (event&&(event->time<beat_start))
									{
										event=event->next;
									}
									if (!event||(event->time>beat_end)||(REJECTED!=event->status))
									{
										/* store the beat averaged signal after the signal */
										averaged_value=((processed_buffer->signals).float_values)+
											number_of_samples;
										value=((processed_buffer->signals).float_values)+beat_start;
										beat_count=beat_counts;
										for (i=beat_end-beat_start;i>0;i--)
										{
											if (0< *beat_count)
											{
												*averaged_value += *value;
											}
											else
											{
												*averaged_value= *value;
											}
											(*beat_count)++;
											beat_count++;
											averaged_value++;
											value++;
										}
									}
								}
								/* store the beat averaged signal after the signal */
								averaged_value=((processed_buffer->signals).float_values)+
									number_of_samples;
								beat_count=beat_counts;
								for (i=max_times;i>0;i--)
								{
									if (0< *beat_count)
									{
										*averaged_value /= (float)(*beat_count);
									}
									else
									{
										*averaged_value=(float)0;
									}
									averaged_value++;
									beat_count++;
								}
								processed_buffer->start=number_of_samples;
								processed_buffer->end=number_of_samples+max_times-1;
								DEALLOCATE(beat_counts);
								trace->valid_processing=1;
							}
							else
							{
								trace->valid_processing=0;
								DEALLOCATE(name);
								display_message(ERROR_MESSAGE,
									"trace_process_device.  Could not allocate beat_counts");
							}
						}
					}
					else
					{
						trace->valid_processing=0;
						DEALLOCATE(name);
						display_message(ERROR_MESSAGE,
							"trace_process_device.  Could not reallocate processed buffer");
					}
				}
				else
				{
					trace->valid_processing=0;
				}
				DEALLOCATE(values);
				DEALLOCATE(times);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case FREQUENCY_DOMAIN:
			{
				float i,*imaginary_value,*real_value,x,y;
				int buffer_offset;
				struct Device *device;
				struct Signal_buffer *buffer;

				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if (trace->valid_processing=
						fourier_transform(trace->frequency_domain.window_type,device,
						(struct Device *)NULL,trace->real_device_1,
						trace->imaginary_device_1))
					{
						/* remove DC */
						buffer=trace->real_device_1->signal->buffer;
						((buffer->signals).float_values)[trace->real_device_1->signal->
							index]=0.;
						((buffer->signals).float_values)[trace->imaginary_device_1->signal->
							index]=0.;
						switch ((trace->frequency_domain).display_mode)
						{
							case REAL_IMAGINARY:
							{
								strcpy(trace->real_device_1->description->name,"Re");
								strcpy(trace->imaginary_device_1->description->name,"Im");
							} break;
							case AMPLITUDE_PHASE:
							{
								strcpy(trace->real_device_1->description->name,"Am");
								strcpy(trace->imaginary_device_1->description->name,"Ph");
								buffer=trace->real_device_1->signal->buffer;
								real_value=((buffer->signals).float_values)+
									(trace->real_device_1->signal->index);
								imaginary_value=((buffer->signals).float_values)+
									(trace->imaginary_device_1->signal->index);
								buffer_offset=buffer->number_of_signals;
								for (i=buffer->number_of_samples;i>0;i--)
								{
									x= *real_value;
									y= *imaginary_value;
									if ((x!=0)||(y!=0))
									{
										*real_value=sqrt(x*x+y*y);
										*imaginary_value=atan2(y,x);
									}
									real_value += buffer_offset;
									imaginary_value += buffer_offset;
								}
								trace->imaginary_device_1->channel->gain=1.;
							} break;
						}
					}
				}
			} break;
			case POWER_SPECTRA:
			{
				float *imaginary_value,*real_value,x,y;
				int buffer_offset,i;
				struct Device *device;
				struct Signal_buffer *buffer;

				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if (trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						device,(struct Device *)NULL,trace->real_device_1,
						trace->imaginary_device_1))
					{
						strcpy(trace->real_device_1->description->name,"Pr");
						buffer=trace->real_device_1->signal->buffer;
						if ((trace->power_spectra).maximum_frequency<0)
						{
							(trace->power_spectra).maximum_frequency=
								(float)(buffer->number_of_samples)/(buffer->frequency);
							sprintf(global_temp_string,"%g",
								(trace->power_spectra).maximum_frequency);
							XtVaSetValues(
								(trace->area_3).power_spectra.maximum_frequency_text_field,
								XmNvalue,global_temp_string,
								XmNeditable,True,
								NULL);
						}
						XtVaSetValues(
							(trace->area_3).power_spectra.maximum_frequency_scroll_bar,
							XmNvalue,(int)(((trace->power_spectra).maximum_frequency)*
							(buffer->frequency)*100./(buffer->number_of_samples)+0.5),
							NULL);
						if ((trace->power_spectra).minimum_frequency<0)
						{
							(trace->power_spectra).minimum_frequency=0.;
							sprintf(global_temp_string,"%g",
								(trace->power_spectra).minimum_frequency);
							XtVaSetValues(
								(trace->area_3).power_spectra.minimum_frequency_text_field,
								XmNvalue,global_temp_string,
								XmNeditable,True,
								NULL);
						}
						XtVaSetValues(
							(trace->area_3).power_spectra.minimum_frequency_scroll_bar,
							XmNvalue,(int)(((trace->power_spectra).minimum_frequency)*
							(buffer->frequency)*100./(buffer->number_of_samples)+0.5),
							NULL);
						/* calculate power spectrum (square of magnitude of FT, which is the
							FT of the auto-correlation) */
						real_value=((buffer->signals).float_values)+
							(trace->real_device_1->signal->index);
						imaginary_value=((buffer->signals).float_values)+
							(trace->imaginary_device_1->signal->index);
						buffer_offset=buffer->number_of_signals;
						/* remove DC */
						*real_value=0.;
						for (i=buffer->number_of_samples-1;i>0;i--)
						{
							real_value += buffer_offset;
							imaginary_value += buffer_offset;
							x= *real_value;
							y= *imaginary_value;
							*real_value=x*x+y*y;
						}
					}
				}
			} break;
			case CROSS_CORRELATION:
			{
				float *imaginary_value,*imaginary_value_2,*real_value,*real_value_2,x,
					x_2,y,y_2;
				int buffer_offset,buffer_offset_2,i,*time;
				struct Device *processed_device;
				struct Signal_buffer *buffer;

				/* recalculate cross correlation */
				if ((processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->signal->buffer)&&
					(trace->correlation.device_1)&&(trace->correlation.device_2))
				{
					/* clear the processed device name */
					if (processed_device->description)
					{
						DEALLOCATE(processed_device->description->name);
					}
					if (processed_device->signal->next)
					{
						destroy_Signal(&(processed_device->signal->next));
						processed_device->signal->buffer->number_of_signals=1;
					}
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if ((trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						trace->correlation.device_1,(struct Device *)NULL,
						trace->real_device_1,trace->imaginary_device_1))&&
						(trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						trace->correlation.device_2,(struct Device *)NULL,
						trace->real_device_2,trace->imaginary_device_2)))
					{
						/* calculate transform_1 * conjugate(transform_2) */
						buffer=trace->real_device_1->signal->buffer;
						real_value=((buffer->signals).float_values)+
							(trace->real_device_1->signal->index);
						imaginary_value=((buffer->signals).float_values)+
							(trace->imaginary_device_1->signal->index);
						buffer_offset=buffer->number_of_signals;
						buffer=trace->real_device_2->signal->buffer;
						real_value_2=((buffer->signals).float_values)+
							(trace->real_device_2->signal->index);
						imaginary_value_2=((buffer->signals).float_values)+
							(trace->imaginary_device_2->signal->index);
						buffer_offset_2=buffer->number_of_signals;
						/* remove DC */
						*real_value=0.;
						*imaginary_value=0.;
						for (i=buffer->number_of_samples-1;i>0;i--)
						{
							real_value += buffer_offset;
							imaginary_value += buffer_offset;
							real_value_2 += buffer_offset_2;
							imaginary_value_2 += buffer_offset_2;
							x= *real_value;
							y= *imaginary_value;
							x_2= *real_value_2;
							y_2= *imaginary_value_2;
							*real_value=x*x_2+y*y_2;
							*imaginary_value=y*x_2-x*y_2;
						}
						/* transform back */
						trace->valid_processing=inverse_fourier_transform(
							trace->real_device_1,trace->imaginary_device_1,processed_device,
							(struct Device *)NULL);
						/* undo "wrap-around" order so that have negative and positive
							lags */
						buffer=processed_device->signal->buffer;
						real_value=((buffer->signals).float_values)+
							(processed_device->signal->index);
						time=buffer->times;
						buffer_offset=buffer->number_of_signals;
						buffer_offset_2=(buffer->number_of_samples)/2;
						for (i=buffer_offset_2;i>0;i--)
						{
							x= *real_value;
							*real_value=real_value[buffer_offset_2];
							real_value[buffer_offset_2]=x;
							*time -= buffer_offset_2;
							time[buffer_offset_2] -= buffer_offset_2;
							real_value += buffer_offset;
							time++;
						}
					}
				}
			} break;
			case AUTO_CORRELATION:
			{
				float *imaginary_value,*real_value,x,y;
				int buffer_offset,i;
				struct Device *device,*processed_device;
				struct Signal_buffer *buffer;

				if ((processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->signal->buffer)&&
					(trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
					if (processed_device->description)
					{
						DEALLOCATE(processed_device->description->name);
						if (device->description)
						{
							processed_device->description->name=duplicate_string(device->
								description->name);
						}
					}
					if (processed_device->signal->next)
					{
						destroy_Signal(&(processed_device->signal->next));
						processed_device->signal->buffer->number_of_signals=1;
					}
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if (trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						device,(struct Device *)NULL,trace->real_device_1,
						trace->imaginary_device_1))
					{
						/* calculate transform * conjugate(transform) (power spectrum) */
						buffer=trace->real_device_1->signal->buffer;
						real_value=((buffer->signals).float_values)+
							(trace->real_device_1->signal->index);
						imaginary_value=((buffer->signals).float_values)+
							(trace->imaginary_device_1->signal->index);
						buffer_offset=buffer->number_of_signals;
						/* remove DC */
						*real_value=0.;
						*imaginary_value=0.;
						for (i=buffer->number_of_samples-1;i>0;i--)
						{
							real_value += buffer_offset;
							imaginary_value += buffer_offset;
							x= *real_value;
							y= *imaginary_value;
							*real_value=x*x+y*y;
							*imaginary_value=0.;
						}
						/* transform back */
						trace->valid_processing=inverse_fourier_transform(
							trace->real_device_1,trace->imaginary_device_1,
							processed_device,(struct Device *)NULL);
						/* auto-correlation is symmetric */
						processed_device->signal->buffer->end=
							(processed_device->signal->buffer->number_of_samples)/2-1;
					}
				}
			} break;
			case FILTERING:
			{
				float *imaginary_value,*real_value;
				int buffer_offset,high_pass,i,low_pass,notch,number_of_samples;
				struct Device *device,*processed_device;
				struct Signal_buffer *buffer;

				if ((processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->signal->buffer)&&
					(trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
					if (processed_device->description)
					{
						DEALLOCATE(processed_device->description->name);
						if (device->description)
						{
							processed_device->description->name=duplicate_string(device->
								description->name);
						}
					}
					if (processed_device->signal->next)
					{
						destroy_Signal(&(processed_device->signal->next));
						processed_device->signal->buffer->number_of_signals=1;
					}
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if (trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						device,(struct Device *)NULL,trace->real_device_1,
						trace->imaginary_device_1))
					{
						/* perform filtering */
						buffer=trace->real_device_1->signal->buffer;
						buffer_offset=buffer->number_of_signals;
						real_value=((buffer->signals).float_values)+
							(trace->real_device_1->signal->index);
						imaginary_value=((buffer->signals).float_values)+
							(trace->imaginary_device_1->signal->index);
						/* remove DC */
						*real_value=0.;
						*imaginary_value=0.;
						if ((trace->filtering).low_pass_frequency<0)
						{
							(trace->filtering).low_pass_frequency=
								(float)(buffer->number_of_samples)/(buffer->frequency);
							sprintf(global_temp_string,"%g",
								(trace->filtering).low_pass_frequency);
							XtVaSetValues((trace->area_3).filtering.low_pass_text_field,
								XmNvalue,global_temp_string,
								XmNeditable,True,
								NULL);
						}
						if ((trace->filtering).high_pass_frequency<0)
						{
							(trace->filtering).high_pass_frequency=0.;
							sprintf(global_temp_string,"%g",
								(trace->filtering).high_pass_frequency);
							XtVaSetValues((trace->area_3).filtering.high_pass_text_field,
								XmNvalue,global_temp_string,
								XmNeditable,True,
								NULL);
						}
						if ((trace->filtering).notch_frequency<0)
						{
							(trace->filtering).notch_frequency=
								(float)(buffer->number_of_samples)/(buffer->frequency);
							if ((float)50<(trace->filtering).notch_frequency)
							{
								(trace->filtering).notch_frequency=(float)50;
							}
							sprintf(global_temp_string,"%g",
								(trace->filtering).notch_frequency);
							XtVaSetValues((trace->area_3).filtering.notch_text_field,
								XmNvalue,global_temp_string,
								XmNeditable,True,
								NULL);
							(trace->filtering).notch_on=0;
							XtVaSetValues(trace->area_3.filtering.notch_toggle,
								XmNset,False,
								NULL);
						}
						low_pass=(int)(((trace->filtering).low_pass_frequency)*
							(buffer->frequency)+0.5);
						if (low_pass<0)
						{
							low_pass=0;
						}
						else
						{
							if (low_pass>buffer->number_of_samples)
							{
								low_pass=buffer->number_of_samples;
							}
						}
						XtVaSetValues((trace->area_3).filtering.low_pass_scroll_bar,
							XmNvalue,(int)(((trace->filtering).low_pass_frequency)*
							(buffer->frequency)*100./(buffer->number_of_samples)+0.5),
							NULL);
						high_pass=(int)(((trace->filtering).high_pass_frequency)*
							(buffer->frequency)+0.5);
						if (high_pass<0)
						{
							high_pass=0;
						}
						else
						{
							if (high_pass>buffer->number_of_samples)
							{
								high_pass=buffer->number_of_samples;
							}
						}
						XtVaSetValues((trace->area_3).filtering.high_pass_scroll_bar,
							XmNvalue,(int)(((trace->filtering).high_pass_frequency)*
							(buffer->frequency)*100./(buffer->number_of_samples)+0.5),
							NULL);
						if ((trace->filtering).notch_on)
						{
/*							notch=(int)(((trace->filtering).notch_frequency)*
								(buffer->frequency)+0.5);*/
							notch=(int)floor(((trace->filtering).notch_frequency)*
								(buffer->frequency));
							if ((notch<0)||(notch>=(buffer->number_of_samples)-1))
							{
								notch= -1;
							}
						}
						else
						{
							notch= -1;
						}
						if (notch>=0)
						{
/*							real_value[(notch-1)*buffer_offset]=0;
							imaginary_value[(notch-1)*buffer_offset]=0;*/
							real_value[notch*buffer_offset]=0;
							imaginary_value[notch*buffer_offset]=0;
							real_value[(notch+1)*buffer_offset]=0;
							imaginary_value[(notch+1)*buffer_offset]=0;
						}
						if (high_pass<low_pass)
						{
							for (i=high_pass;i>0;i--)
							{
								*real_value=0.;
								real_value += buffer_offset;
								*imaginary_value=0.;
								imaginary_value += buffer_offset;
							}
							real_value += (low_pass-high_pass)*buffer_offset;
							imaginary_value += (low_pass-high_pass)*buffer_offset;
							for (i=(buffer->number_of_samples)-low_pass-1;i>0;i--)
							{
								*real_value=0.;
								real_value += buffer_offset;
								*imaginary_value=0.;
								imaginary_value += buffer_offset;
							}
						}
						else
						{
							real_value += low_pass*buffer_offset;
							imaginary_value += low_pass*buffer_offset;
							for (i=high_pass-low_pass-2;i>0;i--)
							{
								*real_value=0.;
								real_value += buffer_offset;
								*imaginary_value=0.;
								imaginary_value += buffer_offset;
							}
						}
						/* transform back */
						trace->valid_processing=inverse_fourier_transform(
							trace->real_device_1,trace->imaginary_device_1,processed_device,
							(struct Device *)NULL);
						extract_signal_information((struct FE_node *)NULL,
							(struct Signal_drawing_package *)NULL,device,1,1,0,(int *)NULL,
							&number_of_samples,(float **)NULL,(float **)NULL,
							(enum Event_signal_status **)NULL,(char **)NULL,(int *)NULL,
							(float *)NULL,(float *)NULL);
						processed_device->signal->buffer->end=number_of_samples-1;
					}
				}
			} break;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_process_device */

int draw_highlight_event_box(int left,int top,int width,int height,
	enum Event_detection_algorithm detection,Widget drawing_area,
	struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
???DB.  Maybe not needed anymore ?
==============================================================================*/
{
	Display *display;
	GC graphics_context;
	int return_code;

	ENTER(draw_highlight_event_box);
	if (drawing_area&&drawing&&(drawing->user_interface)&&(drawing->pixel_map)&&
		signal_drawing_information&&
		(signal_drawing_information->user_interface==drawing->user_interface))
	{
		display=User_interface_get_display(signal_drawing_information->user_interface);
		graphics_context=(signal_drawing_information->graphics_context).
			highlighted_box_colour;
		switch (detection)
		{
			case EDA_INTERVAL:
			{
				XFillRectangle(display,XtWindow(drawing_area),graphics_context,left+1,
					top+1,width-1,height-1);
				XFillRectangle(display,drawing->pixel_map,graphics_context,left+1,top+1,
					width-1,height-1);
			} break;
			case EDA_LEVEL:
			case EDA_THRESHOLD:
			{
				XFillRectangle(display,XtWindow(drawing_area),graphics_context,left,
					top+1,width+1,height-1);
				XFillRectangle(display,drawing->pixel_map,graphics_context,left,top+1,
					width+1,height-1);
			} break;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_highlight_event_box.  Invalid drawing/drawing area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_highlight_event_box */

int draw_search_box(int left,int top,int width,int height,
	enum Event_detection_algorithm detection,int number_of_events,int *divisions,
	Widget drawing_area,struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information)
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION :
==============================================================================*/
{
	Display *display;
	GC graphics_context;
	int bottom,i,return_code,x_division;

	ENTER(draw_search_box);
	if (drawing_area&&drawing&&(drawing->user_interface)&&(drawing->pixel_map)&&
		signal_drawing_information&&
		(signal_drawing_information->user_interface==drawing->user_interface))
	{
		display=
			User_interface_get_display(signal_drawing_information->user_interface);
		graphics_context=(signal_drawing_information->graphics_context).
			interval_box_colour;
		XDrawRectangle(display,XtWindow(drawing_area),graphics_context,left,top,
			width,height);
		XDrawRectangle(display,drawing->pixel_map,graphics_context,left,top,width,
			height);
		if (EDA_INTERVAL==detection)
		{
			bottom=top+height-1;
			for (i=number_of_events-1;i>0;i--)
			{
				if (divisions)
				{
					x_division=divisions[i-1];
				}
				else
				{
					x_division=left+(i*width)/number_of_events;
				}
				XDrawLine(display,XtWindow(drawing_area),graphics_context,x_division,
					top,x_division,bottom);
				XDrawLine(display,drawing->pixel_map,graphics_context,x_division,top,
					x_division,bottom);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_search_box.  Invalid drawing/drawing area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_search_box */

int open_trace_window(struct Trace_window **trace_address,Widget parent,
	Pixel identifying_colour,enum Signal_analysis_mode *analysis_mode,
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
	struct User_interface *user_interface,
	struct Electrical_imaging_event **first_eimaging_event)
/*******************************************************************************
LAST MODIFIED : 4 July 2001

DESCRIPTION :
If <*trace_address> is NULL, a trace window with the specified <parent> and
<identifying_colour> is created.  The trace window is opened.
==============================================================================*/
{
	int return_code;
	struct Trace_window *trace;
	Widget trace_window_shell;

	ENTER(open_trace_window);
	if (trace_address&&parent&&signal_drawing_information&&user_interface)
	{
		if (!(trace= *trace_address))
		{
			if (trace_window_shell=create_trace_window_shell((Widget *)NULL,parent,
				screen_width,screen_height,user_interface))
			{
				if (trace=create_Trace_window(trace_address,(Widget)NULL,
					trace_window_shell,identifying_colour,analysis_mode,detection,
					objective,datum_type,edit_order,highlight,
#if defined (UNEMAP_USE_NODES)
					highlight_rig_node,
#endif /* defined (UNEMAP_USE_NODES) */
					rig,signal_drawing_package,datum,potential_time,
					event_number,number_of_events,threshold,minimum_separation,level,
					average_width,start_search_interval,search_interval_divisions,
					end_search_interval,screen_height,signal_drawing_information,
					user_interface,first_eimaging_event))
				{
					/* add the destroy callback */
					XtAddCallback(trace->shell,XmNdestroyCallback,destroy_window_shell,
						(XtPointer)create_Shell_list_item(&(trace->shell),user_interface));
					/* manage the trace window */
					XtManageChild(trace->window);
					/* realize the trace shell */
					XtRealizeWidget(trace->shell);
					/* set the number of events */
					if (9== *number_of_events)
					{
						XtUnmanageChild(trace->area_1.enlarge.number_of_events.up_arrow);
						XtUnmanageChild(
							trace->area_1.beat_averaging.number_of_beats.up_arrow);
					}
					if (1== *number_of_events)
					{
						XtUnmanageChild(trace->area_1.enlarge.number_of_events.down_arrow);
						XtUnmanageChild(
							trace->area_1.beat_averaging.number_of_beats.down_arrow);
					}
					*trace_address=trace;
					trace_change_signal(trace);
					/* pop up the trace window shell */
					XtPopup(trace->shell,XtGrabNone);
					trace->open=1;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_trace_window.  Could not create window");
					XtDestroyWidget(trace_window_shell);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_trace_window.  Could not create shell");
				return_code=0;
			}
		}
		else
		{
			/* trace window already exists */
			if (!(trace->open))
			{
				trace_change_signal(trace);
				/* pop up the trace window shell */
				XtPopup(trace->shell,XtGrabNone);
				trace->open=1;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_trace_window.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_trace_window */

static int alter_Cardiac_interval_box(Window working_window,Display *display,
	Pixmap pixel_map,GC marker_graphics_context,enum Trace_moving_status moving,
	int marker,int previous_marker,int axes_top,int axes_bottom,int axes_height)
/*******************************************************************************
LAST MODIFIED : 12 June 2001

DESCRIPTION : Alter the start time or end time or peak_or_trough_time
of a graphical cardiac interval box.
==============================================================================*/
{
	int return_code;
	ENTER(alter_Cardiac_interval_box);
	if (display)
	{
		return_code=1;
		/* clear the old marker */
		XDrawLine(display,pixel_map,marker_graphics_context,
			previous_marker,axes_top+1,previous_marker,axes_bottom);
		XDrawLine(display,working_window,marker_graphics_context,
			previous_marker,axes_top+1,previous_marker,axes_bottom);
		/* draw the new marker */
		XDrawLine(display,pixel_map,marker_graphics_context,marker,
			axes_top+1,marker,axes_bottom);
		XDrawLine(display,working_window,marker_graphics_context,
			marker,axes_top+1,marker,axes_bottom);
		/* draw/erase the  change top and bottom parts */
		if (moving==TRACE_MOVING_CARDIAC_END_TIME)
		{
			if (marker>previous_marker)
			{
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker+1,axes_top,marker,axes_top);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker+1,axes_top,marker,axes_top);
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker+1,axes_top+axes_height,marker,axes_top+axes_height);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker+1,axes_top+axes_height,marker,axes_top+axes_height);
			}
			else if (marker<previous_marker)
			{
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker,axes_top,marker+1,axes_top);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker,axes_top,marker+1,axes_top);
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker,axes_top+axes_height,marker+1,axes_top+axes_height);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker,axes_top+axes_height,marker+1,axes_top+axes_height);
			}
		}
		if (moving==TRACE_MOVING_CARDIAC_START_TIME)
		{
			if (marker>previous_marker)
			{
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker,axes_top,marker-1,axes_top);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker,axes_top,marker-1,axes_top);
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker,axes_top+axes_height,marker-1,axes_top+axes_height);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker,axes_top+axes_height,marker-1,axes_top+axes_height);
			}
			else if (marker<previous_marker)
			{
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker-1,axes_top,marker,axes_top);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker-1,axes_top,marker,axes_top);
				XDrawLine(display,pixel_map,marker_graphics_context,
					previous_marker-1,axes_top+axes_height,marker,axes_top+axes_height);
				XDrawLine(display,working_window,marker_graphics_context,
					previous_marker-1,axes_top+axes_height,marker,axes_top+axes_height);
			}
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"alter_Cardiac_interval_box. Invalid argument");
	}
	LEAVE;
	
	return (return_code);
} /* alter_Cardiac_interval_box */

static int limit_Cardiac_markers(int *marker,enum Trace_moving_status moving,
	struct Cardiac_interval *interval,int axes_left,int start_analysis_interval,
	float scale )
/*******************************************************************************
LAST MODIFIED : 9 April 2001

DESCRIPTION :
Limit the allowed placement of the cardiac interval marker <interval>,<marker>.
Assumes that the cardiac interval list is sorted by time, i.e.
interval->previous < interval < interval->next
==============================================================================*/
{
	int return_code,marker_time;

	if (marker&&interval)
	{
		return_code=1;
		marker_time=SCALE_X(*marker,axes_left,start_analysis_interval,1/scale);
		switch (moving)
		{
			case TRACE_MOVING_CARDIAC_START_TIME:
			{
				/* start time limited by prev interval's end time, & its own
					peak_or_trough_time*/
				if (interval->previous)
				{
					if (marker_time<interval->previous->end_time)
					{
						*marker=SCALE_X(interval->previous->end_time,
							start_analysis_interval,axes_left,scale);
					}
				}
				if (marker_time>interval->peak_or_trough_time)
				{
					*marker=SCALE_X(interval->peak_or_trough_time,start_analysis_interval,
						axes_left,scale);
				}
			} break;
			case TRACE_MOVING_CARDIAC_END_TIME:
			{
				/* end time limited by its own peak_or_trough_time and next interval's start time */
				if (marker_time<interval->peak_or_trough_time)
				{
					*marker=SCALE_X(interval->peak_or_trough_time,start_analysis_interval,
						axes_left,scale);
				}
				if (interval->next)
				{
					if (marker_time>interval->next->start_time)
					{
						*marker=SCALE_X(interval->next->start_time,start_analysis_interval,
							axes_left,scale);
					}
				}
			} break;
			case TRACE_MOVING_CARDIAC_PT_TIME:
			{
				/* limit  the marker to its own box (start/end time) */
				if (marker_time<interval->start_time)
				{
					*marker=SCALE_X(interval->start_time,start_analysis_interval,
						axes_left,scale);
				}
				if (marker_time>interval->end_time)
				{
					*marker=SCALE_X(interval->end_time,start_analysis_interval,axes_left
						,scale);
				}
			} break;
			default:
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"limit_Cardiac_markers. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* limit_Cardiac_markers */

static int limit_Cardiac_marker_box(int *interval_diff,
	enum Trace_moving_status moving,struct Cardiac_interval *interval,
	int signal_start,int signal_end)
/*******************************************************************************
LAST MODIFIED : 9 April 2001

DESCRIPTION :
Limit the allowed placement of the cardiac <interval> box, when moving the whole
thing.
Assumes that the cardiac interval list is sorted by time, i.e.
interval->previous < interval < interval->next
==============================================================================*/
{
	int return_code;

	if (interval_diff&&interval&&(moving==TRACE_MOVING_BOX))
	{
		return_code=1;

		/* start time limited by prev interval's end time or signal_start */
		if (interval->previous)
		{
			if ((interval->start_time+*interval_diff)<interval->previous->end_time)
			{
				*interval_diff=(interval->previous->end_time-interval->start_time);
			}
		}
		else
		{
			if ((interval->start_time+*interval_diff)<signal_start)
			{
				*interval_diff=(signal_start-interval->start_time);
			}
		}
		/* end time limited by next interval's start time or signal_end */
		if (interval->next)
		{
			if (interval->end_time+*interval_diff>interval->next->start_time)
			{
				*interval_diff=(interval->next->start_time-interval->end_time);
			}
		}
		else
		{
			if ((interval->end_time+*interval_diff)>signal_end)
			{
				*interval_diff=(signal_end-interval->end_time);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"limit_Cardiac_marker_box. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* limit_Cardiac_marker_box */

static int reconcile_Cardiac_interval_and_marker(int *marker,
	struct Cardiac_interval *interval,float x_scale,
	enum Trace_moving_status moving,int axes_left,int start_analysis_interval)
/*******************************************************************************
LAST MODIFIED : 12 June 2001

DESCRIPTION : ensure that that the  <interval>'s
end_time/start_time/peak_or_trough_time matches the <marker>.
Set <interval> to <marker> and then <marker> to <interval> to avoid
rounding error in SCALE_X(..1/x_scale...) followed by SCALE_X(..x_scale...)
==============================================================================*/
{
	int return_code;

	ENTER(reconcile_Cardiac_interval_and_marker);
	if (marker)
	{
		return_code=1;
		switch (moving)
		{
			case TRACE_MOVING_CARDIAC_END_TIME:
			{
				interval->end_time=SCALE_X(*marker,axes_left,
					start_analysis_interval,1/x_scale);
				*marker=SCALE_X(interval->end_time,start_analysis_interval,
					axes_left,x_scale);
			} break;
			case TRACE_MOVING_CARDIAC_START_TIME:
			{
				interval->start_time=SCALE_X(*marker,axes_left,
					start_analysis_interval,1/x_scale);
				*marker=SCALE_X(interval->start_time,start_analysis_interval,
					axes_left,x_scale);
			} break;
			case TRACE_MOVING_CARDIAC_PT_TIME:
			{
				interval->peak_or_trough_time=SCALE_X(*marker,axes_left,
					start_analysis_interval,1/x_scale);
				*marker=SCALE_X(interval->peak_or_trough_time,start_analysis_interval,
					axes_left,x_scale);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reconcile_Cardiac_interval_and_marker. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* reconcile_Cardiac_interval_and_marker */

static int move_Cardiac_interval_markers(
	struct Trace_window_area_1 *trace_area_1,int initial_marker,
	struct Cardiac_interval *interval,float x_scale,
	struct Signal_drawing_information *signal_drawing_information,
	Display *display,int x_pointer,enum Trace_moving_status moving,int *times,
	GC marker_graphics_context,float frequency,int start_analysis_interval,
	unsigned int working_button)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION : Move the indviudal cardiac interval markers.
==============================================================================*/
{
	char time_string[20];
	Cursor cursor;
	int axes_bottom,axes_height,axes_left,axes_right,axes_top,axes_width,
		marker,pointer_x,pointer_y,previous_marker,
		return_code,time;
	Pixmap pixel_map;
	Window working_window;
	XEvent xevent;

	ENTER(move_Cardiac_interval_markers);
	if (trace_area_1&&interval&&signal_drawing_information&&display&&times)
	{
		axes_left=trace_area_1->axes_left;
		axes_width=trace_area_1->axes_width;
		axes_right=axes_left+axes_width-1;
		axes_top=trace_area_1->axes_top;
		axes_bottom=axes_top+(trace_area_1->axes_height)-1;
		axes_height=trace_area_1->axes_height;
		/* grab the pointer */
		cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
		/* set the cursor */
		XDefineCursor(display,XtWindow(trace_area_1->drawing_area),cursor);
		XmUpdateDisplay(trace_area_1->drawing_area);
		XWarpPointer(display,None,None,0,0,0,0,
			initial_marker-x_pointer,0);
		pointer_x=initial_marker;
		marker=initial_marker;
		working_window=XtWindow(trace_area_1->drawing_area);
		pixel_map=trace_area_1->drawing->pixel_map;
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
						reconcile_Cardiac_interval_and_marker(&marker,interval,x_scale,
							moving,axes_left,start_analysis_interval);
						limit_Cardiac_markers(&marker,moving,interval,axes_left,
							start_analysis_interval,x_scale);
						if (marker!=previous_marker)
						{
							alter_Cardiac_interval_box(working_window,display,pixel_map,
								marker_graphics_context,moving,marker,previous_marker,
								axes_top,axes_bottom,axes_height);
						}	/* if (marker!=previous_marker)	*/
					}
				} break;
				case ButtonRelease:
				{
					/* clear the exisitng marker */
					XDrawLine(display,pixel_map,marker_graphics_context,marker,
						axes_top+1,marker,axes_bottom);
					XDrawLine(display,working_window,marker_graphics_context,
						marker,axes_top+1,marker,axes_bottom);
					reconcile_Cardiac_interval_and_marker(&marker,interval,x_scale,
						moving,axes_left,start_analysis_interval);
					if (moving==TRACE_MOVING_CARDIAC_PT_TIME)
					{
						/*write in the new time */
						time=(int)((float)((times)[interval->peak_or_trough_time])*1000./frequency);
						sprintf(time_string,"%d",time);
						write_marker_time(time_string,marker,axes_left,
							axes_width,axes_top,signal_drawing_information->font,
							(signal_drawing_information->graphics_context).	potential_time_colour_text,
							display,trace_area_1->drawing_area,trace_area_1->drawing);
					}
					/* draw the new marker */
					XDrawLine(display,pixel_map,marker_graphics_context,
						marker,axes_top+1,marker,axes_bottom);
					XDrawLine(display,working_window,marker_graphics_context,
						marker,axes_top+1,marker,axes_bottom);
					moving=TRACE_MOVING_NONE;
				} break;
				case ButtonPress:
				{
					if (xevent.xbutton.button==working_button)
					{
						display_message(ERROR_MESSAGE,
							"move_Cardiac_interval_markers. Unexpected button press\n");
						moving=TRACE_MOVING_NONE;
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"move_Cardiac_interval_markers. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* move_Cardac_interval_markers */

static int move_Cardiac_interval_box(struct Trace_window_area_1 *trace_area_1,
	struct Cardiac_interval *interval,float x_scale,
	struct Signal_drawing_information *signal_drawing_information,
	Display *display,int x_pointer,enum Trace_moving_status moving,
	int start_analysis_interval,int end_analysis_interval,
	struct Signal_buffer *buffer,unsigned int working_button)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION : move the entire cardiac interval box.
==============================================================================*/
{
	Cursor cursor;
	int axes_bottom,axes_height,axes_left,axes_right,axes_top,axes_width,
		interval_time,interval_time_prev,pointer_x,
		pointer_y,return_code,previous_x_pos,x_pos,interval_diff;
	Window working_window;
	XEvent xevent;

	ENTER(move_Cardiac_interval_box);
	if (trace_area_1&&interval&&signal_drawing_information&&display&&
		(moving==TRACE_MOVING_BOX))
	{
		axes_left=trace_area_1->axes_left;
		axes_width=trace_area_1->axes_width;
		axes_right=axes_left+axes_width-1;
		axes_top=trace_area_1->axes_top;
		axes_bottom=axes_top+(trace_area_1->axes_height)-1;
		axes_height=trace_area_1->axes_height;
		/* grab the pointer */
		/* need to change to the square thing */
		cursor=XCreateFontCursor(display,XC_dotbox);
		XDefineCursor(display,XtWindow(trace_area_1->drawing_area),cursor);
		XmUpdateDisplay(trace_area_1->drawing_area);
		x_pos=x_pointer;
		working_window=XtWindow(trace_area_1->drawing_area);
		while (TRACE_MOVING_NONE!=moving)
		{
			XNextEvent(display,&xevent);
			switch (xevent.type)
			{
				case MotionNotify:
				{
					previous_x_pos=x_pos;
					/* reduce the number of motion events displayed */
					while (True==XCheckMaskEvent(display,ButtonMotionMask,&xevent));
					pointer_x=xevent.xmotion.x;
					pointer_y=xevent.xmotion.y;
					if ((xevent.xmotion.window==working_window)&&
						(pointer_y>=axes_top)&&(pointer_y<=axes_bottom))
					{

						if (pointer_x<axes_left)
						{
							x_pos=axes_left;
						}
						else
						{
							if (pointer_x>axes_right)
							{
								x_pos=axes_right;
							}
							else
							{
								x_pos=pointer_x;
							}
						}
						if (x_pos!=previous_x_pos)
						{
							/* clear old */
							draw_Cardiac_interval_box(interval,end_analysis_interval,
								start_analysis_interval,axes_top,axes_height,axes_left,
								axes_width,trace_area_1->drawing_area,trace_area_1->drawing,
								signal_drawing_information,buffer);
							/* alter interval */
							interval_time=SCALE_X(x_pos,axes_left,start_analysis_interval,1/x_scale);
							interval_time_prev=SCALE_X(previous_x_pos,axes_left,
								start_analysis_interval,1/x_scale);
							interval_diff=interval_time-interval_time_prev;
							limit_Cardiac_marker_box(&interval_diff,moving,interval,
								buffer->start,buffer->end);
							interval->start_time+=interval_diff;
							interval->peak_or_trough_time+=interval_diff;
							interval->end_time+=interval_diff;
							/* draw new */
							draw_Cardiac_interval_box(interval,end_analysis_interval,
								start_analysis_interval,axes_top,axes_height,axes_left,
								axes_width,trace_area_1->drawing_area,trace_area_1->drawing,
								signal_drawing_information,buffer);
						}/* (x_pos!=previous_x_pos)	*/

					}
				} break;
				case ButtonRelease:
				{
					moving=TRACE_MOVING_NONE;
				} break;
				case ButtonPress:
				{
					if (xevent.xbutton.button==working_button)
					{
						display_message(ERROR_MESSAGE,
							"move_Cardiac_interval_box. Unexpected button press\n");
						moving=TRACE_MOVING_NONE;
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"move_Cardiac_interval_box. Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* move_Cardiac_interval_box */

int configure_eimaging_time_dialog_marker_menu(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 22 June 2001

DESCRIPTION :
Configures <trace>'s eimaging_time_dialog's event_choice based upon
<trace>'s inverse_wave_mode.
<trace>'s eimaging_time_dialog MUST have been created before this function can
be called.
==============================================================================*/
{
	enum Inverse_wave_mode inverse_wave_mode;
	char flag;
	int i,num_children,return_code;
	struct Electrical_imaging_time_dialog *eimaging_time_dialog;
	Widget *list,option_widget;

	ENTER(configure_eimaging_time_dialog_marker_menu);
	eimaging_time_dialog=(struct Electrical_imaging_time_dialog *)NULL;
	list=(Widget *)NULL;
	if (trace&&(eimaging_time_dialog=trace->area_3.eimaging_time_dialog))
	{
		/*dim all the menu choices */
		XtSetSensitive(eimaging_time_dialog->current_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->clear_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->p_wave_start_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->p_wave_peak_or_trough_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->p_wave_end_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->qrs_wave_start_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->qrs_wave_peak_or_trough_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->qrs_wave_end_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->t_wave_start_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->t_wave_peak_or_trough_button,FALSE);
		XtSetSensitive(eimaging_time_dialog->t_wave_end_button,FALSE);
		/*select the relevant ones.*/
		/* "clear" is always always valid */
		XtSetSensitive(eimaging_time_dialog->clear_button,TRUE);
		/* if we have at least one electrical imaging event, we can use the current
			one */
		if (trace->first_eimaging_event)
		{
			XtSetSensitive(eimaging_time_dialog->current_button,TRUE);
		}
		if (trace->first_interval)
		{
			inverse_wave_mode=trace->inverse_wave_mode;
			if ((inverse_wave_mode==P_WAVE)||(inverse_wave_mode==PQRS_WAVE)||
				(inverse_wave_mode==PT_WAVE)||(inverse_wave_mode==PQRST_WAVE))
			{
				if (find_Cardiac_interval_in_list_given_type(trace->first_interval,
					P_WAVE_INTERVAL))
				{
					XtSetSensitive(eimaging_time_dialog->p_wave_start_button,TRUE);
					XtSetSensitive(eimaging_time_dialog->p_wave_peak_or_trough_button,
						TRUE);
					XtSetSensitive(eimaging_time_dialog->p_wave_end_button,TRUE);
				}
			}
			if ((inverse_wave_mode==QRS_WAVE)||(inverse_wave_mode==PQRS_WAVE)||
				(inverse_wave_mode==QRST_WAVE)||(inverse_wave_mode==PQRST_WAVE))
			{
				if (find_Cardiac_interval_in_list_given_type(trace->first_interval,
					QRS_WAVE_INTERVAL))
				{
					XtSetSensitive(eimaging_time_dialog->qrs_wave_start_button,TRUE);
					XtSetSensitive(eimaging_time_dialog->qrs_wave_peak_or_trough_button,
						TRUE);
					XtSetSensitive(eimaging_time_dialog->qrs_wave_end_button,TRUE);
				}
			}
			if ((inverse_wave_mode==T_WAVE)||(inverse_wave_mode==PT_WAVE)||
				(inverse_wave_mode==QRST_WAVE)||(inverse_wave_mode==PQRST_WAVE))
			{
				if (find_Cardiac_interval_in_list_given_type(trace->first_interval,
					T_WAVE_INTERVAL))
				{
					XtSetSensitive(eimaging_time_dialog->t_wave_start_button,TRUE);
					XtSetSensitive(eimaging_time_dialog->t_wave_peak_or_trough_button,
						TRUE);
					XtSetSensitive(eimaging_time_dialog->t_wave_end_button,TRUE);
				}
			}

		}
		/* find current menu selection */
		XtVaGetValues(eimaging_time_dialog->event_choice,
			XmNmenuHistory,&option_widget,NULL);
		/* is it valid (sensitive) ?*/
		XtVaGetValues(option_widget,XmNsensitive,&flag,NULL);
		/* if not, or if it's the clear button*/
		if ((flag==FALSE)||(option_widget==eimaging_time_dialog->clear_button))
		{
			/* find the first valid (sensitive) entry */
			XtVaGetValues(eimaging_time_dialog->event_pull_down,XmNchildren,
				&list,NULL);
			XtVaGetValues(eimaging_time_dialog->event_pull_down,XmNnumChildren,
				&num_children,NULL);
			i=0;
			flag=FALSE;
			while ((i<num_children)&&(flag==FALSE))
			{
				XtVaGetValues(list[i],XmNsensitive,&flag,NULL);
				if (flag==TRUE)
				{
					/* make the first valid (sensitive) entry the current choice */
					XtVaSetValues(eimaging_time_dialog->event_choice,
						XmNmenuHistory,list[i],NULL);
				}
				i++;
			}
		}	/* if (flag==FALSE)	*/
		/* find (now possibly changed) current menu selection  */
		XtVaGetValues(eimaging_time_dialog->event_choice,
			XmNmenuHistory,&option_widget,NULL);
		/* update this info */
		alter_eimaging_button_event_info(option_widget,eimaging_time_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"configure_eimaging_time_dialog_marker_menu. invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /*configure_eimaging_time_dialog_marker_menu*/

int create_Electrical_imaging_events_from_time(struct Trace_window *trace,
	int current_event_time,int step)
/*******************************************************************************
LAST MODIFIED : 22 June 2001

DESCRIPTION :
Create electrical imaging events at <current_event_time>, and up from (and down
from) this, as spacing <step>, for the length of <trace>'s cardiac interval
device.  If <step>==0, then just put the event at <current_event_time>, none up
or down.
==============================================================================*/
{
	int return_code,time;
	struct Signal_buffer *buffer;
	struct Device *cardiac_interval_device;
	struct Electrical_imaging_event  *new_event;

	ENTER(create_Electrical_imaging_events_from_time);
	new_event=(struct Electrical_imaging_event  *)NULL;
	cardiac_interval_device=(struct Device *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if (trace&&(cardiac_interval_device=trace->cardiac_interval_device)&&
		(buffer=get_Device_signal_buffer(cardiac_interval_device)))
	{
		return_code=1;
		destroy_Electrical_imaging_event_list(trace->first_eimaging_event);
		time=current_event_time;
		if (step==0)
		{
			new_event=create_Electrical_imaging_event(time);
				add_Electrical_imaging_event_to_sorted_list(trace->first_eimaging_event,
					new_event);
			new_event->is_current_event=1;
		}
		else
		{
			while (time<=buffer->end)
			{
				new_event=create_Electrical_imaging_event(time);
				if (time==current_event_time)
				{
					new_event->is_current_event=1;
				}
				add_Electrical_imaging_event_to_sorted_list(trace->first_eimaging_event,
					new_event);
				time+=step;
			}
			time=current_event_time-step;
			/* loop backward from current event, making events */
			while (time>buffer->start)
			{
				new_event=create_Electrical_imaging_event(time);
				add_Electrical_imaging_event_to_sorted_list(trace->first_eimaging_event,
					new_event);
				time-=step;
			}
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"create_Electrical_imaging_events_from_time.  Missing trace_window");
	}
	LEAVE;
	
	return (return_code);
} /* create_Electrical_imaging_events_from_time */

struct Cardiac_interval *find_Cardiac_interval_in_list_given_type(
	struct Cardiac_interval *first_list_interval,
	enum Cardiac_interval_type desired_type)
/*******************************************************************************
LAST MODIFIED : 22 June 2001

DESCRIPTION : finds  and returns the first  cardiac interval in list beginning
with <first_list_interval> that has type <desired_type>. If none is found,
returns NULL.
==============================================================================*/
{
	struct Cardiac_interval *interval,*found_interval;

	ENTER(find_Cardiac_interval_in_list_given_type);
	interval=(struct Cardiac_interval *)NULL;
	found_interval=(struct Cardiac_interval *)NULL;
	if (interval=first_list_interval)
	{
		/* find interval with matching type */
		while (interval&&(found_interval==NULL))
		{
			if (interval->type==desired_type)
			{
				found_interval=interval;
			}
			interval=interval->next;
		}
	}
	else
	{

		display_message(ERROR_MESSAGE,
			"find_Cardiac_interval_in_list_given_type. invalid arguments");
	}
	LEAVE;
	
	return (found_interval);
} /* find_Cardiac_interval_in_list_given_type */

int move_Cardiac_interval(XmDrawingAreaCallbackStruct *callback,
	struct Device *highlight_device,struct Trace_window *trace,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,int pointer_sensitivity)
/*******************************************************************************
LAST MODIFIED : 11 April 2001

DESCRIPTION : detect mouse or keyboard activity and move the cardiac intervals,
either individually or the whole box.
==============================================================================*/
{
	char time_string[20];
	enum Trace_moving_status moving;
	Display *display;
	float frequency,x_scale;
	GC marker_graphics_context;
	int axes_bottom,axes_left,axes_right,axes_top,axes_width,end_analysis_interval,
		end_marker,found,initial_marker,pointer_x,pointer_y,return_code,
		start_analysis_interval,start_marker,time,*times;
	struct Cardiac_interval *interval;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	unsigned int working_button;
	XButtonEvent *button_event;

	ENTER(move_Cardiac_interval);
	times=(int *)NULL;
	interval=(struct Cardiac_interval *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	trace_area_1=(struct Trace_window_area_1 *)NULL;
	button_event=(XButtonEvent *)NULL;
	display=(Display *)NULL;
	if (callback&&highlight_device&&(buffer=get_Device_signal_buffer(highlight_device))&&
		trace&&(trace_area_1=&(trace->area_1))&&signal_drawing_information&&
		(times=buffer->times)&&user_interface)
	{
		return_code=1;
		if ((ButtonPress==callback->event->type)&&(interval=trace->first_interval))
		{
			display=User_interface_get_display(user_interface);
			button_event= &(callback->event->xbutton);
			working_button=button_event->button;
			pointer_x=button_event->x;
			pointer_y=button_event->y;
			axes_left=trace_area_1->axes_left;
			axes_width=trace_area_1->axes_width;
			axes_right=axes_left+axes_width-1;
			axes_top=trace_area_1->axes_top;
			axes_bottom=axes_top+(trace_area_1->axes_height)-1;
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
				/* are we at a cardiac interval? */
				found=0;
				moving=TRACE_MOVING_NONE;
				while (interval&&(!found))
				{
					initial_marker=SCALE_X(interval->peak_or_trough_time,start_analysis_interval,
						axes_left,x_scale);
					if ((pointer_x>=initial_marker-pointer_sensitivity)&&
						(pointer_x<=initial_marker+pointer_sensitivity))
					{
						marker_graphics_context=(signal_drawing_information->graphics_context).
							cardiac_interval_colour;
						moving=TRACE_MOVING_CARDIAC_PT_TIME;
						/*clear the time text */
						time=(int)((float)((buffer->times)
							[interval->peak_or_trough_time])*1000./frequency);
						sprintf(time_string,"%d",time);
						write_marker_time(time_string,initial_marker,axes_left,
							axes_width,axes_top,signal_drawing_information->font,
							(signal_drawing_information->graphics_context).potential_time_colour_text,
							display,trace_area_1->drawing_area,trace_area_1->drawing);
						found=1;
					}
					else
					{
						initial_marker=SCALE_X(interval->start_time,start_analysis_interval,
							axes_left,x_scale);
						if ((pointer_x>=initial_marker-pointer_sensitivity)&&
							(pointer_x<=initial_marker+pointer_sensitivity))
						{
							marker_graphics_context=interval->graphics_context;
							moving=TRACE_MOVING_CARDIAC_START_TIME;
							found=1;
						}
						else
						{
							initial_marker=SCALE_X(interval->end_time,start_analysis_interval,
								axes_left,x_scale);
							if ((pointer_x>=initial_marker-pointer_sensitivity)&&
								(pointer_x<=initial_marker+pointer_sensitivity))
							{
								marker_graphics_context=interval->graphics_context;
								moving=TRACE_MOVING_CARDIAC_END_TIME;
								found=1;
							}
							else
							{
								start_marker=SCALE_X(interval->start_time,start_analysis_interval,
									axes_left,x_scale);
								start_marker+=pointer_sensitivity;
								end_marker=SCALE_X(interval->end_time,start_analysis_interval,
									axes_left,x_scale);
								end_marker-=pointer_sensitivity;
								/* are we moving a box? */
								if ((pointer_x>start_marker)&&(pointer_x<end_marker))
								{
									moving=TRACE_MOVING_BOX;
									found=1;
								}
								else
								{
									moving=TRACE_MOVING_NONE;
									found=0;
								}
							}
						}
					}
					if (!found)
					{
						interval=interval->next;
					}
				}
				if (found)
				{
					if (moving==TRACE_MOVING_BOX)
					{
						move_Cardiac_interval_box(trace_area_1,interval,x_scale,
							signal_drawing_information,display,pointer_x,moving,
							start_analysis_interval,end_analysis_interval,buffer,
							working_button);
					}
					else
					{
						move_Cardiac_interval_markers(trace_area_1,initial_marker,interval,
							x_scale,signal_drawing_information,display,pointer_x,moving,times,
							marker_graphics_context,frequency,start_analysis_interval,
							working_button);
					}
					/* we've changed the cardiac intervals, so change their device and
						redraw */
					calculate_Cardiac_interval_device(trace);
					redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
						(XtPointer)NULL);
				}
			}
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"move_Cardiac_interval. invalid argument");
	}
	LEAVE;
	
	return (return_code);
} /* move_Cardiac_interval */

void redraw_trace_1_drawing_area(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 1.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	Display *display;
	float x_scale;
	int analysis_range,axes_left,axes_height,axes_top,axes_width,
		end_analysis_interval,height,i,redraw,start_analysis_interval,
		valid_processing,width;
	struct Device *device;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window *trace;
	struct Trace_window_area_1 *trace_area_1;
	struct User_interface *user_interface;
	XExposeEvent *expose_event;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(redraw_trace_1_drawing_area);
	USE_PARAMETER(widget);
	if ((trace=(struct Trace_window *)trace_window)&&
		(user_interface=trace->user_interface)&&
		(signal_drawing_information=trace->signal_drawing_information))
	{
		display=User_interface_get_display(user_interface);
		trace_area_1= &(trace->area_1);
		if (trace_area_1->drawing_area)
		{
			if (callback=(XmDrawingAreaCallbackStruct *)call_data)
			{
				if ((XmCR_EXPOSE==callback->reason)&&(callback->event)&&
					(Expose==callback->event->type))
				{
					expose_event= &(callback->event->xexpose);
					redraw=0;
				}
				else
				{
					if (XmCR_RESIZE==callback->reason)
					{
						/*??? during creation there are resize callbacks without windows */
						if (callback->window)
						{
							expose_event=(XExposeEvent *)NULL;
							redraw=0;
							/* find the size of the old rectangle */
							if (trace_area_1->drawing)
							{
								width=trace_area_1->drawing->width;
								height=trace_area_1->drawing->height;
								/* destroy the old pixmap */
								destroy_Drawing_2d(&(trace_area_1->drawing));
							}
							else
							{
								width=0;
								height=0;
							}
						}
						else
						{
							redraw= -1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_1_drawing_area.  Invalid call data");
						redraw= -1;
					}
				}
			}
			else
			{
				redraw=1;
			}
			if (redraw>=0)
			{
				if (!(trace_area_1->drawing))
				{
					/* determine the size of the drawing area */
					XGetWindowAttributes(display,XtWindow(trace_area_1->drawing_area),
						&attributes);
					/* create a pixel map */
					if (trace_area_1->drawing=create_Drawing_2d(
						trace_area_1->drawing_area,attributes.width,attributes.height,
						NO_DRAWING_IMAGE,user_interface))
					{
						redraw=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_1_drawing_area.  Could not create drawing");
					}
				}
				if (trace_area_1->drawing)
				{
					if (redraw)
					{
						/* clear the drawing area */
						XFillRectangle(display,trace_area_1->drawing->pixel_map,
							(signal_drawing_information->graphics_context).
							background_drawing_colour,0,0,trace_area_1->drawing->width,
							trace_area_1->drawing->height);
						switch (*trace->analysis_mode)
						{
							case EVENT_DETECTION: case BEAT_AVERAGING:
							{
								/* draw the active signal */
								if ((trace->highlight)&&(*(trace->highlight))&&
#if defined (UNEMAP_USE_NODES)
									(*(trace->highlight_rig_node))&&
									(*(trace->signal_drawing_package))&&
#endif /* defined (UNEMAP_USE_NODES) */
									(device= **(trace->highlight))&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
#if defined (UNEMAP_USE_NODES)
									draw_signal(*(trace->highlight_rig_node),
										*(trace->signal_drawing_package),(struct Device *)NULL,
										ENLARGE_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#else /* defined (UNEMAP_USE_NODES) */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,
										ENLARGE_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (UNEMAP_USE_NODES) */
									trace_area_1->axes_left=axes_left;
									trace_area_1->axes_top=axes_top;
									trace_area_1->axes_width=axes_width;
									trace_area_1->axes_height=axes_height;
									draw_device_markers(device,start_analysis_interval,
										end_analysis_interval,*(trace->event_detection.datum),1,
										*(trace->event_detection.potential_time),1,
										ENLARGE_AREA_DETAIL,*(trace->event_detection.event_number),
										axes_left,axes_top,axes_width,axes_height,(Window)NULL,
										trace_area_1->drawing->pixel_map,signal_drawing_information,
										user_interface);
									if ((analysis_range=end_analysis_interval-
										start_analysis_interval)>0)
									{
										x_scale=SCALE_FACTOR(analysis_range,axes_width-1);
										trace_area_1->enlarge.left_box=SCALE_X(
											*(trace->event_detection.start_search_interval),
											start_analysis_interval,axes_left,x_scale);
										trace_area_1->enlarge.right_box=SCALE_X(
											*(trace->event_detection.end_search_interval),
											start_analysis_interval,axes_left,x_scale);
										if ((*(trace->event_detection.search_interval_divisions))&&
											(trace_area_1->enlarge.divisions))
										{
											for (i= *(trace->event_detection.number_of_events)-2;i>=0;
												i--)
											{
												(trace_area_1->enlarge.divisions)[i]=SCALE_X(
													(*(trace->event_detection.search_interval_divisions))[
													i],start_analysis_interval,axes_left,x_scale);
											}
										}
										switch (*(trace->event_detection.detection))
										{
											case EDA_INTERVAL:
											{
												if (*(trace->event_detection.search_interval_divisions))
												{
													if (1< *(trace->event_detection.event_number))
													{
														trace_area_1->enlarge.left_edit_box=SCALE_X(
															(*(trace->event_detection.
															search_interval_divisions))[
															*(trace->event_detection.event_number)-2],
															start_analysis_interval,axes_left,x_scale);
													}
													else
													{
														trace_area_1->enlarge.left_edit_box=
															trace_area_1->enlarge.left_box;
													}
													if (*(trace->event_detection.event_number)<
														*(trace->event_detection.number_of_events))
													{
														trace_area_1->enlarge.right_edit_box=SCALE_X(
															(*(trace->event_detection.
															search_interval_divisions))[
															*(trace->event_detection.event_number)-1],
															start_analysis_interval,axes_left,x_scale);
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
														((*(trace->event_detection.event_number)-1)*
														((trace_area_1->enlarge.right_box)-
														(trace_area_1->enlarge.left_box)))/
														(*(trace->event_detection.number_of_events));
													trace_area_1->enlarge.right_edit_box=
														(trace_area_1->enlarge.left_box)+
														((*(trace->event_detection.event_number))*
														((trace_area_1->enlarge.right_box)-
														(trace_area_1->enlarge.left_box)))/
														(*(trace->event_detection.number_of_events));
												}
											} break;
											case EDA_LEVEL:
											case EDA_THRESHOLD:
											{
												trace_area_1->enlarge.left_edit_box=SCALE_X(
													trace->area_3.edit.first_data,
													start_analysis_interval,axes_left,x_scale);
												trace_area_1->enlarge.right_edit_box=SCALE_X(
													trace->area_3.edit.last_data,
													start_analysis_interval,axes_left,x_scale);
											} break;
										}
									}
									else
									{
										trace_area_1->enlarge.left_box=axes_left;
										trace_area_1->enlarge.right_box=axes_left+axes_width-1;
										trace_area_1->enlarge.left_edit_box=axes_left;
										trace_area_1->enlarge.right_edit_box=axes_left+axes_width-1;
									}
									draw_search_box(trace_area_1->enlarge.left_box,axes_top,
										trace_area_1->enlarge.right_box-
										trace_area_1->enlarge.left_box,axes_height,
										*(trace->event_detection.detection),
										*(trace->event_detection.number_of_events),
										trace_area_1->enlarge.divisions,
										trace_area_1->drawing_area,trace_area_1->drawing,
										signal_drawing_information);
									draw_highlight_event_box(trace_area_1->enlarge.left_edit_box,
										axes_top,trace_area_1->enlarge.right_edit_box-
										trace_area_1->enlarge.left_edit_box,axes_height,
										*(trace->event_detection.detection),
										trace_area_1->drawing_area,trace_area_1->drawing,
										signal_drawing_information);
								}
								else
								{
									trace_area_1->axes_left=0;
									trace_area_1->axes_top=0;
									trace_area_1->axes_width=0;
									trace_area_1->axes_height=0;
									trace_area_1->enlarge.left_box=0;
									trace_area_1->enlarge.right_box=0;
								}
							} break;
							case FREQUENCY_DOMAIN: case POWER_SPECTRA: case AUTO_CORRELATION:
								case FILTERING:
							{
								if ((trace->highlight)&&(*(trace->highlight))&&
									(device= **(trace->highlight))&&
#if defined (UNEMAP_USE_NODES)
									(*(trace->highlight_rig_node))&&
									(*(trace->signal_drawing_package))&&
#endif /* defined (UNEMAP_USE_NODES) */
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									/* draw the active signal */
#if defined (UNEMAP_USE_NODES)
									draw_signal(*(trace->highlight_rig_node),
										*(trace->signal_drawing_package),(struct Device *)NULL,
										EDIT_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#else /* defined (UNEMAP_USE_NODES) */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,
										EDIT_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (UNEMAP_USE_NODES) */
									trace_area_1->axes_left=axes_left;
									trace_area_1->axes_top=axes_top;
									trace_area_1->axes_width=axes_width;
									trace_area_1->axes_height=axes_height;
								}
							} break;
							case CROSS_CORRELATION:
							{
								if ((device=trace->correlation.device_1)&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									/* draw the active signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_1->drawing->width,trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_1->axes_left=axes_left;
									trace_area_1->axes_top=axes_top;
									trace_area_1->axes_width=axes_width;
									trace_area_1->axes_height=axes_height;
								}
							} break;
							case ELECTRICAL_IMAGING:
							{
								if (trace->calculate_signal_mode==CURRENT_SIGNAL)
								{
									device= **(trace->highlight);
									valid_processing=1;
								}
								else
									/* RMS_SIGNAL  */
								{
									device=trace->processed_device;
									valid_processing=trace->valid_processing;
								}
								buffer=get_Device_signal_buffer(device);
								start_analysis_interval=buffer->start;
								end_analysis_interval=buffer->end;
								if (device&&buffer&&valid_processing)
								{
									/* draw the active signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_1->drawing->width,trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_1->axes_left=axes_left;
									trace_area_1->axes_top=axes_top;
									trace_area_1->axes_width=axes_width;
									trace_area_1->axes_height=axes_height;
									draw_Cardiac_interval_markers(trace);
								}
							} break;
						}
					}
					/* redisplay the specified part of the pixmap */
					if (callback)
					{
						if (expose_event)
						{
							/* expose callback */
							XCopyArea(display,trace_area_1->drawing->pixel_map,
								XtWindow(trace_area_1->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								expose_event->x,expose_event->y,expose_event->width,
								expose_event->height,expose_event->x,expose_event->y);
						}
						else
						{
							/* resize callback */
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							XCopyArea(display,trace_area_1->drawing->pixel_map,
								XtWindow(trace_area_1->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								0,0,width,height,0,0);
						}
					}
					else
					{
						/* update */
						XCopyArea(display,trace_area_1->drawing->pixel_map,
							XtWindow(trace_area_1->drawing_area),
							(signal_drawing_information->graphics_context).copy,0,0,
							trace_area_1->drawing->width,trace_area_1->drawing->height,0,0);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"redraw_trace_1_drawing_area.  Missing drawing area");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"redraw_trace_1_drawing_area.  Missing trace_window");
	}
	LEAVE;
} /* redraw_trace_1_drawing_area */

void redraw_trace_3_drawing_area(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 3.
==============================================================================*/
{
	Display *display;
	int axes_height,axes_left,axes_top,axes_width,beat_end,beat_start,
		current_data_interval,edit_start,edit_diff,end_analysis_interval,first_data,
		*first_data_array,height,i,last_data,*last_data_array,
		number_of_data_intervals,number_of_events,redraw,signal_height,
		start_analysis_interval,width;
	struct Device *device;
	struct Event *event;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window *trace;
	struct Trace_window_area_3 *trace_area_3;
	struct User_interface *user_interface;
	XExposeEvent *expose_event;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(redraw_trace_3_drawing_area);
	USE_PARAMETER(widget);
#if defined (DEBUG)
	/*???debug */
	printf("enter redraw_trace_3_drawing_area\n");
	list_memory(-1,0,0);
#endif /* defined (DEBUG) */
	if ((trace=(struct Trace_window *)trace_window)&&
		(user_interface=trace->user_interface)&&
		(signal_drawing_information=trace->signal_drawing_information))
	{
		display=User_interface_get_display(user_interface);
		trace_area_3= &(trace->area_3);
		if (trace_area_3->drawing_area)
		{
			if (callback=(XmDrawingAreaCallbackStruct *)call_data)
			{
				if ((XmCR_EXPOSE==callback->reason)&&(callback->event)&&
					(Expose==callback->event->type))
				{
					expose_event= &(callback->event->xexpose);
					redraw=0;
				}
				else
				{
					if (XmCR_RESIZE==callback->reason)
					{
						/*??? during creation there are resize callbacks without windows */
						if (callback->window)
						{
							expose_event=(XExposeEvent *)NULL;
							redraw=0;
							/* find the size of the old rectangle */
							if (trace_area_3->drawing)
							{
								width=trace_area_3->drawing->width;
								height=trace_area_3->drawing->height;
								/* destroy the old pixmap */
								destroy_Drawing_2d(&(trace_area_3->drawing));
							}
							else
							{
								width=0;
								height=0;
							}
						}
						else
						{
							redraw= -1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_3_drawing_area.  Invalid call data");
						redraw= -1;
					}
				}
			}
			else
			{
				redraw=1;
			}
			if (redraw>=0)
			{
				if (!(trace_area_3->drawing))
				{
					/* determine the size of the drawing area */
					XGetWindowAttributes(display,XtWindow(trace_area_3->drawing_area),
						&attributes);
					/* create a pixel map */
					if (trace_area_3->drawing=create_Drawing_2d(
						trace_area_3->drawing_area,attributes.width,attributes.height,
						NO_DRAWING_IMAGE,user_interface))
					{
						redraw=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"redraw_trace_3_drawing_area.  Could not create drawing");
					}
				}
				if (trace_area_3->drawing)
				{
					if (redraw)
					{
						/* clear the drawing area */
						XFillRectangle(display,trace_area_3->drawing->pixel_map,
							(signal_drawing_information->graphics_context).
							background_drawing_colour,0,0,trace_area_3->drawing->width,
							trace_area_3->drawing->height);
						switch (*trace->analysis_mode)
						{
							case ELECTRICAL_IMAGING:
							{
								device=trace->cardiac_interval_device;
								buffer=get_Device_signal_buffer(device);
								start_analysis_interval=buffer->start;
								end_analysis_interval=buffer->end;
								if (device&&buffer&&trace->valid_processing)
								{
									/* draw the cardiac_interval_device signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_3->drawing->width,trace_area_3->drawing->height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
									draw_Electrical_imaging_event_markers(trace);
								}
							} break;
							case EVENT_DETECTION:
							{
								if ((trace->highlight)&&(*(trace->highlight))&&
									(device= **(trace->highlight))
#if defined (UNEMAP_USE_NODES)
									&&(*(trace->highlight_rig_node))&&
									(*(trace->signal_drawing_package))
#endif /* defined (UNEMAP_USE_NODES) */
										)
								{
									/* draw the active signal */
									first_data=trace_area_3->edit.first_data;
									last_data=trace_area_3->edit.last_data;
									if ((True==XmToggleButtonGadgetGetState((trace->area_3).edit.
										objective_toggle))&&(trace->processed_device)&&
										(trace->valid_processing))
									{
										trace->processed_device->signal_display_minimum=
											device->signal_display_minimum;
										trace->processed_device->signal_display_maximum=
											device->signal_display_maximum;
										draw_signal((struct FE_node *)NULL,
											(struct Signal_drawing_package *)NULL,
											trace->processed_device,EDIT_AREA_DETAIL,1,0,&first_data,
											&last_data,0,0,trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
									}
									else
									{
#if defined (UNEMAP_USE_NODES)
										draw_signal(*(trace->highlight_rig_node),
											*(trace->signal_drawing_package),
											(struct Device *)NULL,EDIT_AREA_DETAIL,1,0,
											&first_data,&last_data,0,0,trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#else /* defined (UNEMAP_USE_NODES) */
										draw_signal((struct FE_node *)NULL,
											(struct Signal_drawing_package *)NULL,device,
											EDIT_AREA_DETAIL,1,0,&first_data,&last_data,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#endif /* defined (UNEMAP_USE_NODES) */
									}
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
									draw_device_markers(device,first_data,last_data,
										*(trace->event_detection.datum),1,
										*(trace->event_detection.potential_time),1,EDIT_AREA_DETAIL,
										*(trace->event_detection.event_number),axes_left,axes_top,
										axes_width,axes_height,(Window)NULL,
										trace_area_3->drawing->pixel_map,signal_drawing_information,
										user_interface);
								}
							} break;
							case BEAT_AVERAGING:
							{
								if (trace->valid_processing)
								{
									device=trace->processed_device;
								}
								else
								{
									if ((trace->highlight)&&(*(trace->highlight)))
									{
										device= **(trace->highlight);
									}
									else
									{
										device=(struct Device *)NULL;
									}
								}
								if (device&&(buffer=get_Device_signal_buffer(device)))
								{
									number_of_events=
										*((trace->event_detection).number_of_events);
									ALLOCATE(first_data_array,int,number_of_events+1);
									ALLOCATE(last_data_array,int,number_of_events+1);
									if (first_data_array&&last_data_array)
									{
										if (True==XmToggleButtonGadgetGetState((trace->area_3).
											beat_averaging.overlay_beats_toggle))
										{
											if ((trace->highlight)&&(*(trace->highlight))&&
												(**(trace->highlight))&&
												((**(trace->highlight))->signal))
											{
												event=(**(trace->highlight))->signal->first_event;
											}
											else
											{
												event=(struct Event *)NULL;
											}
											number_of_data_intervals=0;
											current_data_interval=0;
											if (*((trace->event_detection).search_interval_divisions))
											{
												beat_start=
													*((trace->event_detection).start_search_interval);
												for (i=0;i<number_of_events;i++)
												{
													if (i<number_of_events-1)
													{
														beat_end=(*((trace->event_detection).
															search_interval_divisions))[i]-1;
													}
													else
													{
														beat_end=
															*((trace->event_detection).end_search_interval);
													}
													while (event&&(event->time<beat_start))
													{
														event=event->next;
													}
													if (!event||(event->time>beat_end)||
														(REJECTED!=event->status))
													{
														if (event&&(event->number==
															*((trace->event_detection).event_number)))
														{
															current_data_interval=number_of_data_intervals;
														}
														first_data_array[number_of_data_intervals]=
															beat_start;
														last_data_array[number_of_data_intervals]=beat_end;
														number_of_data_intervals++;
													}
													beat_start=beat_end+1;
												}
											}
											else
											{
												edit_start=
													*(trace->event_detection.start_search_interval);
												edit_diff= *(trace->event_detection.
													end_search_interval)-edit_start;
												beat_start=edit_start;
												for (i=0;i<number_of_events;i++)
												{
													if (i<number_of_events-1)
													{
														beat_end=edit_start+(int)((float)(edit_diff*(i+1))/
															(float)number_of_events+0.5)-1;
													}
													else
													{
														beat_end=
															*((trace->event_detection).end_search_interval);
													}
													while (event&&!((beat_start<=event->time)&&
														(event->time<=beat_end)))
													{
														event=event->next;
													}
													if (!event||(REJECTED!=event->status))
													{
														if (event&&(event->number==
															*((trace->event_detection).event_number)))
														{
															current_data_interval=number_of_data_intervals;
														}
														first_data_array[number_of_data_intervals]=
															beat_start;
														last_data_array[number_of_data_intervals]=beat_end;
														number_of_data_intervals++;
													}
													beat_start=beat_end+1;
												}
											}
											if ((trace->valid_processing)&&
												(True==XmToggleButtonGadgetGetState((trace->area_3).
												beat_averaging.beat_averaging_toggle)))
											{
												first_data_array[number_of_data_intervals]=
													buffer->start;
												last_data_array[number_of_data_intervals]=buffer->end;
												number_of_data_intervals++;
											}
										}
										else
										{
											number_of_data_intervals=1;
											current_data_interval=0;
											if ((trace->valid_processing)&&
												(True==XmToggleButtonGadgetGetState((trace->area_3).
												beat_averaging.beat_averaging_toggle)))
											{
												*first_data_array=buffer->start;
												*last_data_array=buffer->end;
											}
											else
											{
												*first_data_array=trace_area_3->edit.first_data;
												*last_data_array=trace_area_3->edit.last_data;
											}
										}
										draw_signal((struct FE_node *)NULL,
											(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,
											number_of_data_intervals,current_data_interval,
											first_data_array,last_data_array,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
										trace_area_3->axes_left=axes_left;
										trace_area_3->axes_top=axes_top;
										trace_area_3->axes_width=axes_width;
										trace_area_3->axes_height=axes_height;
										draw_device_markers(device,*first_data_array,*last_data_array,
											*(trace->event_detection.datum),1,
											*(trace->event_detection.potential_time),1,EDIT_AREA_DETAIL,
											*(trace->event_detection.event_number),axes_left,axes_top,
											axes_width,axes_height,(Window)NULL,
											trace_area_3->drawing->pixel_map,signal_drawing_information,
											user_interface);
									}
									DEALLOCATE(first_data_array);
									DEALLOCATE(last_data_array);
								}
							} break;
							case FREQUENCY_DOMAIN:
							{
								if ((trace->valid_processing)&&(device=trace->real_device_1)&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									signal_height=((trace_area_3->drawing->height)-1)/2;
									/* draw the real/amplitude signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									/* draw the imaginary/phase signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,trace->imaginary_device_1,
										EDIT_AREA_DETAIL,1,0,&start_analysis_interval,
										&end_analysis_interval,0,signal_height,
										trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
								}
							} break;
							case POWER_SPECTRA:
							{
								if ((trace->valid_processing)&&(device=trace->real_device_1)&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=
										(trace->power_spectra).minimum_frequency*
										(buffer->frequency);
									if (start_analysis_interval<buffer->start)
									{
										start_analysis_interval=buffer->start;
									}
									end_analysis_interval=
										(trace->power_spectra).maximum_frequency*
										(buffer->frequency);
									if (end_analysis_interval>buffer->end)
									{
										end_analysis_interval=buffer->end;
									}
									signal_height=trace_area_3->drawing->height;
									/* draw the power spectrum */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
								}
							} break;
							case CROSS_CORRELATION:
							case AUTO_CORRELATION:
							case FILTERING:
							{
								if ((trace->valid_processing)&&
									(device=trace->processed_device)&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									signal_height=trace_area_3->drawing->height;
									/* draw the signal */
									draw_signal((struct FE_node *)NULL,
										(struct Signal_drawing_package *)NULL,device,EDIT_AREA_DETAIL,1,0,
										&start_analysis_interval,&end_analysis_interval,0,0,
										trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
								}
							} break;
						}
					}
					/* redisplay the specified part of the pixmap */
					if (callback)
					{
						if (expose_event)
						{
							/* expose callback */
							XCopyArea(display,trace_area_3->drawing->pixel_map,
								XtWindow(trace_area_3->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								expose_event->x,expose_event->y,expose_event->width,
								expose_event->height,expose_event->x,expose_event->y);
						}
						else
						{
							/* resize callback */
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							XCopyArea(display,trace_area_3->drawing->pixel_map,
								XtWindow(trace_area_3->drawing_area),
								(signal_drawing_information->graphics_context).copy,
								0,0,width,height,0,0);
						}
					}
					else
					{
						/* update */
						XCopyArea(display,trace_area_3->drawing->pixel_map,
							XtWindow(trace_area_3->drawing_area),
							(signal_drawing_information->graphics_context).copy,0,0,
							trace_area_3->drawing->width,trace_area_3->drawing->height,0,0);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"redraw_trace_3_drawing_area.  Missing drawing area");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"redraw_trace_3_drawing_area.  Missing trace_window");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave redraw_trace_3_drawing_area\n");
#endif /* defined (DEBUG) */
	LEAVE;
} /* redraw_trace_3_drawing_area */

int trace_change_signal(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Called when the "highlighted_device" is changed.
???DB.  More work needed.  Pass device/signal ?
==============================================================================*/
{
	int return_code;

	ENTER(trace_change_signal);
	return_code=0;
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case ELECTRICAL_IMAGING:
			{
				trace_process_device(trace);
				calculate_Cardiac_interval_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case EVENT_DETECTION:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case BEAT_AVERAGING:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case FREQUENCY_DOMAIN:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case POWER_SPECTRA:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case CROSS_CORRELATION:
			{
				struct Device *device;

				/* put the signal in the correct time domain */
				if ((trace->highlight)&&(*(trace->highlight)))
				{
					device= **(trace->highlight);
				}
				else
				{
					device=(struct Device *)NULL;
				}
				if (trace->correlation.signal_1_input)
				{
					trace->correlation.device_1=device;
					redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
						(XtPointer)NULL);
				}
				else
				{
					trace->correlation.device_2=device;
					redraw_trace_2_drawing_area((Widget)NULL,(XtPointer)trace,
						(XtPointer)NULL);
				}
				/* recalculate cross correlation */
				trace_process_device(trace);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case AUTO_CORRELATION:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case FILTERING:
			{
				trace_process_device(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
		}
		trace_update_signal_controls(trace);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_signal */

int trace_change_event(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 14 January 2000

DESCRIPTION :
Change the event without changing the device.
???DB.  The same as trace_change_signal for EVENT_DETECTION at present, but
should be able to do better.  Will have to keep track of event changed from ?
See analysis_previous_event.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int return_code;

	ENTER(trace_change_event);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if (trace->processed_device)
				{
					trace->processed_device->signal_display_minimum=1;
					trace->processed_device->signal_display_maximum=0;
				}
				trace_update_edit_interval(trace);
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				trace_update_signal_controls(trace);
			} break;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_event */

int trace_change_signal_status(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Change the signal status (accept, reject, undecided) without changing the
device.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int return_code;

	ENTER(trace_change_signal_status);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				trace_update_signal_controls(trace);
			} break;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_signal_status */

int trace_change_display_interval(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Change the signal interval displayed in the trace window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int return_code;

	ENTER(trace_change_display_interval);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case FREQUENCY_DOMAIN: case POWER_SPECTRA:
				case AUTO_CORRELATION: case BEAT_AVERAGING:
			{
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case ELECTRICAL_IMAGING:
			{
				trace->calculate_rms=1;
				trace_change_signal(trace);
			} break;
			case CROSS_CORRELATION:
			{
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_2_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_display_interval */

int trace_change_search_interval(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 29 December 1999

DESCRIPTION :
Change the search interval displayed in the trace window.
???DB.  Update end points here ?
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int return_code;

	ENTER(trace_change_search_interval);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if (trace->event_detection.detection)
				{
					switch (*(trace->event_detection.detection))
					{
						case EDA_INTERVAL:
						{
							trace_update_edit_interval(trace);
							redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
								(XtPointer)NULL);
							trace_update_signal_controls(trace);
						} break;
					}
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
#if defined (OLD_CODE)
/*???DB.  Shouldn't be here */
			case THRESHOLD:
			{
				return_code=1;
			} break;
#endif /* defined (OLD_CODE) */
			default:
			{
				return_code=0;
			} break;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_search_interval */

int trace_update_signal_controls(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Updates the selectability of the signal controls.
==============================================================================*/
{
	int device_number,event_number,i,return_code;
	struct Device **device;
	struct Device	**highlight;
	struct Event *event;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(trace_update_signal_controls)
	device=(struct Device **)NULL;
	highlight=(struct Device **)NULL;
	event=(struct Event *)NULL;
	current_region=(struct Region *)NULL;
	rig=(struct Rig *)NULL;;
	if (trace)
	{
		if ((trace->rig)&&(rig= *(trace->rig))&&(trace->highlight)&&
			(highlight= *(trace->highlight))&&(*highlight))
		{
			switch (*trace->analysis_mode)
			{
				case ELECTRICAL_IMAGING:
				{
					/*??JW  do stuff */
				} break;
				case EVENT_DETECTION:
				{
					/* ghost/unghost the calculate button */
					if (!(trace->area_1.enlarge.calculate_all_events)&&
						((*highlight)->signal)&&(REJECTED==(*highlight)->signal->status))
					{
						XtSetSensitive(trace->area_1.enlarge.calculate_button,False);
					}
					else
					{
						XtSetSensitive(trace->area_1.enlarge.calculate_button,True);
					}
					/* ghost/unghost the accept and reject buttons */
					if ((*highlight)->signal)
					{
						event=(*highlight)->signal->first_event;
						event_number= *(trace->event_detection.event_number);
						while (event&&(event->number<event_number))
						{
							event=event->next;
						}
					}
					else
					{
						event=(struct Event *)NULL;
					}
					if (event&&(event->number==event_number))
					{
						XtSetSensitive(trace->area_3.edit.accept_button,True);
						XtSetSensitive(trace->area_3.edit.reject_button,True);
					}
					else
					{
						XtSetSensitive(trace->area_3.edit.accept_button,False);
						XtSetSensitive(trace->area_3.edit.reject_button,False);
					}
					/* ghost/unghost the next and previous buttons */
					if (current_region=get_Rig_current_region(rig))
					{
						device=rig->devices;
						device_number=0;
						for (i=highlight-device;i>0;i--)
						{
							if (current_region==(*device)->description->region)
							{
								device_number++;
							}
							device++;
						}
					}
					else
					{
						device_number=highlight-rig->devices;
					}
					if (((!current_region&&(device_number==rig->number_of_devices-1))||
						(current_region&&(device_number==
						current_region->number_of_devices-1)))&&
						(!event||((event->number==event_number)&&!(event->next))))
					{
						XtSetSensitive(trace->area_3.edit.next_button,False);
					}
					else
					{
						XtSetSensitive(trace->area_3.edit.next_button,True);
					}
					if ((0==device_number)&&(!event||((event->number>=event_number)&&
						!(event->previous))))
					{
						XtSetSensitive(trace->area_3.edit.previous_button,False);

					}
					else
					{
						XtSetSensitive(trace->area_3.edit.previous_button,True);
					}
				} break;
			}
			return_code=1;
		}
		else
		{
			switch (*trace->analysis_mode)
			{
				case EVENT_DETECTION:
				{
					/* ghost the calculate button */
					XtSetSensitive(trace->area_1.enlarge.calculate_button,False);
					/* ghost the accept and reject buttons */
					XtSetSensitive(trace->area_3.edit.accept_button,False);
					XtSetSensitive(trace->area_3.edit.reject_button,False);
					/* ghost the next and previous buttons */
					XtSetSensitive(trace->area_3.edit.next_button,False);
					XtSetSensitive(trace->area_3.edit.previous_button,False);
				} break;
			}
			return_code=1;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_update_signal_controls */

int trace_draw_event_marker(struct Event *event,struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Draws the <event> marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	float frequency;
	int current_event_number,datum,end_analysis_interval,return_code,
		signal_maximum,signal_minimum,start_analysis_interval,*times;
	struct Device **highlight;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(trace_draw_event_marker);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if ((trace->event_detection.datum)&&
					(trace->event_detection.event_number)&&(trace->highlight)&&
					(highlight= *(trace->highlight))&&(*highlight)&&
					(buffer=get_Device_signal_buffer(*highlight))&&
					(times=buffer->times))
				{
					datum= *(trace->event_detection.datum);
					current_event_number= *(trace->event_detection.event_number);
					frequency=buffer->frequency;
					start_analysis_interval=buffer->start;
					end_analysis_interval=buffer->end;
					signal_minimum=(*highlight)->signal_display_minimum;
					signal_maximum=(*highlight)->signal_display_maximum;
					trace_area_1= &(trace->area_1);
					draw_event_marker(event,current_event_number,datum,times,frequency,
						ENLARGE_AREA_DETAIL,start_analysis_interval,end_analysis_interval,
						signal_minimum,signal_maximum,trace_area_1->axes_left,
						trace_area_1->axes_top,trace_area_1->axes_width,
						trace_area_1->axes_height,XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,trace->signal_drawing_information,
						trace->user_interface);
					trace_area_3= &(trace->area_3);
					draw_event_marker(event,current_event_number,datum,times,frequency,
						EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,signal_minimum,signal_maximum,
						trace_area_3->axes_left,trace_area_3->axes_top,
						trace_area_3->axes_width,trace_area_3->axes_height,
						XtWindow(trace_area_3->drawing_area),
						trace_area_3->drawing->pixel_map,trace->signal_drawing_information,
						trace->user_interface);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"trace_draw_event_marker.  Invalid event");
					return_code=0;
				}
			} break;
			default:
			{
				return_code=1;
			} break;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_draw_event_marker */

static int trace_update_pot_time_beat_event(struct Trace_window *trace,
	int potential_time,int previous_potential_time,
	enum Trace_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 16 January 2002

DESCRIPTION :
Updates the potential time marker in the <trace> window.
==============================================================================*/
{
	int end_analysis_interval,return_code,signal_index,start_analysis_interval;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(trace_update_pot_time_beat_event);
	if (trace)
	{
		if ((trace->event_detection.potential_time)&&(trace->highlight)&&
			(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
			(buffer=get_Device_signal_buffer(highlight_device)))
		{
			start_analysis_interval=buffer->start;
			end_analysis_interval=buffer->end;
			trace_area_1= &(trace->area_1);
			if (highlight_device->signal)
			{
				signal_index=highlight_device->signal->index;
			}
			else
			{
				signal_index= -1;
			}
			if (*update_flags & TRACE_1_NO_POTENTIAL_ERASE)
			{
				/* Clear just that flag */
				*update_flags &= ~TRACE_1_NO_POTENTIAL_ERASE;
			}
			else
			{
				draw_potential_time_marker(previous_potential_time,buffer,
					highlight_device->channel,signal_index,ENLARGE_AREA_DETAIL,
					start_analysis_interval,end_analysis_interval,
					trace_area_1->axes_left,trace_area_1->axes_top,
					trace_area_1->axes_width,trace_area_1->axes_height,
					XtWindow(trace_area_1->drawing_area),
					trace_area_1->drawing->pixel_map,
					trace->signal_drawing_information,trace->user_interface);
			}
			draw_potential_time_marker(potential_time,buffer,
				highlight_device->channel,signal_index,ENLARGE_AREA_DETAIL,
				start_analysis_interval,end_analysis_interval,
				trace_area_1->axes_left,trace_area_1->axes_top,
				trace_area_1->axes_width,trace_area_1->axes_height,
				XtWindow(trace_area_1->drawing_area),
				trace_area_1->drawing->pixel_map,trace->signal_drawing_information,
				trace->user_interface);
			trace_area_3= &(trace->area_3);
			if ((*update_flags & TRACE_3_NO_POTENTIAL_ERASE))
			{
				/* Clear just that flag */
				*update_flags &= ~TRACE_3_NO_POTENTIAL_ERASE;
			}
			else
			{
				draw_potential_time_marker(previous_potential_time,buffer,
					highlight_device->channel,signal_index,EDIT_AREA_DETAIL,
					trace_area_3->edit.first_data,trace_area_3->edit.last_data,
					trace_area_3->axes_left,trace_area_3->axes_top,
					trace_area_3->axes_width,trace_area_3->axes_height,
					XtWindow(trace_area_3->drawing_area),
					trace_area_3->drawing->pixel_map,
					trace->signal_drawing_information,trace->user_interface);
			}
			draw_potential_time_marker(potential_time,buffer,
				highlight_device->channel,signal_index,EDIT_AREA_DETAIL,
				trace_area_3->edit.first_data,trace_area_3->edit.last_data,
				trace_area_3->axes_left,trace_area_3->axes_top,
				trace_area_3->axes_width,trace_area_3->axes_height,
				XtWindow(trace_area_3->drawing_area),
				trace_area_3->drawing->pixel_map,trace->signal_drawing_information,
				trace->user_interface);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"trace_update_pot_time_beat_event.  Invalid potential_time");
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_update_pot_time_beat_event */

int trace_update_potential_time(struct Trace_window *trace,int potential_time,
	int previous_potential_time,enum Trace_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 16 January 2002

DESCRIPTION :
Updates the potential time marker in the <trace> window. for
Beat averaging sharing and event detection
==============================================================================*/
{
	int return_code;

	ENTER(trace_update_potential_time);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION:
			{
				return_code=trace_update_pot_time_beat_event(trace,potential_time,
					previous_potential_time,update_flags);
			} break;
			case BEAT_AVERAGING:
			{
				/* if we're not in beat averaging mode with toggle on, update the
					potential time */
				if (False==XmToggleButtonGadgetGetState((trace->area_3).beat_averaging.
					beat_averaging_toggle))
				{
					return_code=trace_update_pot_time_beat_event(trace,potential_time,
						previous_potential_time,update_flags);
				}
				else
				{
					return_code=1;
				}
			} break;
			default:
			{
				return_code=1;
			} break;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* trace_update_potential_time */

int trace_update_datum(struct Trace_window *trace,int datum,int previous_datum,
	int event_number,int potential_time,
	enum Trace_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
Updates the datum marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	char delay_time_string[20];
	Display *display;
	int ascent,descent,direction,end_analysis_interval,length,return_code,
		start_analysis_interval,*times,update_trace_area_3,x_string,y_string;
	float frequency;
	GC event_graphics_context,event_graphics_context_text;
	struct Device *device,*highlight_device;
	struct Event *event;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(trace_update_datum);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if ((trace->highlight)&&
					(signal_drawing_information=trace->signal_drawing_information)&&
					(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
					(buffer=get_Device_signal_buffer(highlight_device))&&
					(times=buffer->times))
				{

					update_trace_area_3=0;
					/* if we're not in beat averaging mode with toggle on,
						update trace_area_3 */
					if (!((*trace->analysis_mode==BEAT_AVERAGING)&&
						(True==XmToggleButtonGadgetGetState((trace->area_3).
						beat_averaging.beat_averaging_toggle))))
					{
						update_trace_area_3=1;
					}				
					display=User_interface_get_display(trace->user_interface);
					start_analysis_interval=buffer->start;
					end_analysis_interval=buffer->end;
					frequency=buffer->frequency;
					font=signal_drawing_information->font;

					trace_area_1= &(trace->area_1);
					if (*update_flags & TRACE_1_NO_DATUM_ERASE)
					{
						/* Redraw the datum so it can be erased with the event markers */
						draw_datum_marker(previous_datum,ENLARGE_AREA_DETAIL,
							start_analysis_interval,
							end_analysis_interval,
							trace_area_1->axes_left,
							trace_area_1->axes_top,
							trace_area_1->axes_width,
							trace_area_1->axes_height,
							XtWindow(trace_area_1->drawing_area),
							trace_area_1->drawing->pixel_map,
							signal_drawing_information,
							trace->user_interface);
						/* Clear just that flag */
						*update_flags &= ~TRACE_1_NO_DATUM_ERASE;
					}
					draw_device_markers(highlight_device,
						start_analysis_interval,
						end_analysis_interval,previous_datum,1,
						potential_time,0,ENLARGE_AREA_DETAIL,
						event_number,trace_area_1->axes_left,
						trace_area_1->axes_top,
						trace_area_1->axes_width,
						trace_area_1->axes_height,
						XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						signal_drawing_information,
						trace->user_interface);
					if ((BEAT_AVERAGING==*trace->analysis_mode)&&trace->valid_processing)
					{
						device=trace->processed_device;
					}
					else
					{
						device=highlight_device;
					}
					if (device->signal)
					{
						event=device->signal->first_event;
						while (event&&(event->number<event_number))
						{
							event=event->next;
						}
					}
					else
					{
						event=(struct Event *)NULL;
					}
					if (event&&(event_number==event->number))
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
										graphics_context).
									accepted_colour_text;
							} break;
							case REJECTED:
							{
								event_graphics_context=
									(signal_drawing_information->
										graphics_context).rejected_colour;
								event_graphics_context_text=
									(signal_drawing_information->
										graphics_context).
									rejected_colour_text;
							} break;
							case UNDECIDED:
							{
								event_graphics_context=
									(signal_drawing_information->
										graphics_context).undecided_colour;
								event_graphics_context_text=
									(signal_drawing_information->
										graphics_context).
									undecided_colour_text;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"trace_update_datum.  Invalid event status");
								event_graphics_context=
									(signal_drawing_information->
										graphics_context).undecided_colour;
								event_graphics_context_text=
									(signal_drawing_information->
										graphics_context).
									undecided_colour_text;
							} break;
						}
					}
					if (update_trace_area_3)
					{
						trace_area_3= &(trace->area_3);
						if (*update_flags & TRACE_3_NO_DATUM_ERASE)
						{
							/* Clear just that flag */
							*update_flags &= ~TRACE_3_NO_DATUM_ERASE;
						}
						else
						{
							draw_datum_marker(previous_datum,EDIT_AREA_DETAIL,
								trace_area_3->edit.first_data,
								trace_area_3->edit.last_data,
								trace_area_3->axes_left,
								trace_area_3->axes_top,
								trace_area_3->axes_width,
								trace_area_3->axes_height,
								XtWindow(trace_area_3->drawing_area),
								trace_area_3->drawing->pixel_map,
								signal_drawing_information,
								trace->user_interface);

							/* clear the delay time */
							if (event)
							{
								sprintf(delay_time_string,"%d",
									(int)((float)(times[event->time]-
										times[previous_datum])*1000./frequency));
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
						}
					}
					/* draw the new markers */
					draw_device_markers(highlight_device,
						start_analysis_interval,
						end_analysis_interval,datum,1,
						potential_time,0,ENLARGE_AREA_DETAIL,
						event_number,trace_area_1->axes_left,
						trace_area_1->axes_top,
						trace_area_1->axes_width,
						trace_area_1->axes_height,
						XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,
						signal_drawing_information,
						trace->user_interface);
					if (update_trace_area_3)
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
							trace->user_interface);
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
					}
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"trace_update_datum.  Invalid potential_time");
					return_code=0;
				}
			} break;
			default:
			{
				return_code=1;
			} break;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_update_datum */

int trace_draw_markers(char draw_datum,char draw_potential_time,
	struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Draws the markers in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int datum,end_analysis_interval,event_number,potential_time,return_code,
		start_analysis_interval;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(trace_draw_markers);
	if (trace)
	{
		switch (*trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if ((trace->event_detection.datum)&&
					(trace->event_detection.potential_time)&&
					(trace->event_detection.number_of_events)&&
					(trace->event_detection.event_number)&&(trace->highlight)&&
					(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
					(buffer=get_Device_signal_buffer(highlight_device)))
				{
					datum= *(trace->event_detection.datum);
					potential_time= *(trace->event_detection.potential_time);
					event_number= *(trace->event_detection.event_number);
					start_analysis_interval=buffer->start;
					end_analysis_interval=buffer->end;
					trace_area_1= &(trace->area_1);
					draw_device_markers(highlight_device,start_analysis_interval,
						end_analysis_interval,datum,draw_datum,potential_time,
						draw_potential_time,ENLARGE_AREA_DETAIL,event_number,
						trace_area_1->axes_left,trace_area_1->axes_top,
						trace_area_1->axes_width,trace_area_1->axes_height,
						XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,trace->signal_drawing_information,
						trace->user_interface);
					trace_area_3= &(trace->area_3);
					draw_device_markers(highlight_device,trace_area_3->edit.first_data,
						trace_area_3->edit.last_data,datum,draw_datum,potential_time,
						draw_potential_time,EDIT_AREA_DETAIL,event_number,
						trace_area_3->axes_left,trace_area_3->axes_top,
						trace_area_3->axes_width,trace_area_3->axes_height,
						XtWindow(trace_area_3->drawing_area),
						trace_area_3->drawing->pixel_map,trace->signal_drawing_information,
						trace->user_interface);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"trace_draw_markers.  Invalid event");
					return_code=0;
				}
			} break;
			default:
			{
				return_code=1;
			} break;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_draw_markers */

int trace_update_edit_interval(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 4 January 2000

DESCRIPTION :
???DB.  Should be a module function.  Wait until select_trace_1_drawing_area
has been moved.
==============================================================================*/
{
	int edit_diff,edit_start,event_number,return_code;
	struct Event *event;
	struct Signal_buffer *buffer;

	ENTER(trace_update_edit_interval);
	if (trace&&(trace->event_detection.detection))
	{
		switch (*(trace->event_detection.detection))
		{
			case EDA_INTERVAL:
			{
				if ((trace->event_detection.event_number)&&
					(trace->event_detection.number_of_events)&&
					(trace->event_detection.start_search_interval)&&
					(trace->event_detection.end_search_interval))
				{
					if (*(trace->event_detection.search_interval_divisions))
					{
						if (1< *(trace->event_detection.event_number))
						{
							trace->area_3.edit.first_data=
								(*(trace->event_detection.search_interval_divisions))[
								*(trace->event_detection.event_number)-2];
						}
						else
						{
							trace->area_3.edit.first_data=
								*(trace->event_detection.start_search_interval);
						}
						if (*(trace->event_detection.event_number)<
							*(trace->event_detection.number_of_events))
						{
							trace->area_3.edit.last_data=
								(*(trace->event_detection.search_interval_divisions))[
								*(trace->event_detection.event_number)-1];
						}
						else
						{
							trace->area_3.edit.last_data=
								*(trace->event_detection.end_search_interval);
						}
					}
					else
					{
						edit_start= *(trace->event_detection.start_search_interval);
						edit_diff= *(trace->event_detection.end_search_interval)-edit_start;
						trace->area_3.edit.first_data=edit_start+(int)((float)(edit_diff*
							((*(trace->event_detection.event_number))-1))/
							(float)(*(trace->event_detection.number_of_events))+0.5);
						trace->area_3.edit.last_data=edit_start+(int)((float)(edit_diff*
							(*(trace->event_detection.event_number)))/
							(float)(*(trace->event_detection.number_of_events))+0.5);
					}
					return_code=1;
				}
				else
				{
					trace->area_3.edit.first_data=0;
					trace->area_3.edit.last_data= -1;
					display_message(ERROR_MESSAGE,
						"trace_update_edit_interval.  Missing information");
					return_code=0;
				}
			} break;
			case EDA_LEVEL:
			case EDA_THRESHOLD:
			{
				if ((trace->event_detection.event_number)&&(trace->highlight)&&
					(*(trace->highlight))&&(**(trace->highlight))&&
					(buffer=get_Device_signal_buffer(**(trace->highlight))))
				{
					if (((**(trace->highlight))->signal)&&
						(event=(**(trace->highlight))->signal->first_event))
					{
						event_number= *(trace->event_detection.event_number);
						while (event&&(event->number<event_number))
						{
							event=event->next;
						}
						if (event&&(event->number==event_number))
						{
							edit_start=trace->area_3.edit.first_data;
							edit_diff=(trace->area_3.edit.last_data)-edit_start;
							if ((event->time<=edit_start+(edit_diff/8))||
								(event->time>edit_start+((7*edit_diff)/8)))
							{
								trace->area_3.edit.first_data=(event->time)-(edit_diff/2);
								if (trace->area_3.edit.first_data<buffer->start)
								{
									trace->area_3.edit.first_data=buffer->start;
								}
								trace->area_3.edit.last_data=
									trace->area_3.edit.first_data+edit_diff;
								if (trace->area_3.edit.last_data>buffer->end)
								{
									trace->area_3.edit.first_data=(buffer->end)-edit_diff;
									trace->area_3.edit.last_data=buffer->end;
								}
							}
						}
					}
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"trace_update_edit_interval.  Missing information");
					return_code=0;
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"trace_update_edit_interval.  Invalid trace window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_update_edit_interval */

int trace_change_rig(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 29 December 1999

DESCRIPTION :
Sets both the cross correlation devices to the highlight device.  Should be
called when the analysis rig is changed.
==============================================================================*/
{
	int return_code;
	struct Device *highlight_device;

	ENTER(trace_change_rig);
	if (trace)
	{
		if ((trace->highlight)&&(*(trace->highlight))&&
			(highlight_device= **(trace->highlight)))
		{
			trace->correlation.device_1=highlight_device;
			trace->correlation.device_2=highlight_device;
		}
		else
		{
			trace->correlation.device_1=(struct Device *)NULL;
			trace->correlation.device_2=(struct Device *)NULL;
		}
		/* set the edit interval */
		switch (*(trace->event_detection.detection))
		{
			case EDA_INTERVAL:
			{
				trace_update_edit_interval(trace);
			} break;
			case EDA_LEVEL:
			case EDA_THRESHOLD:
			{
				trace->area_3.edit.first_data=
					*(trace->event_detection.start_search_interval);
				trace->area_3.edit.last_data=
					*(trace->event_detection.end_search_interval);
			} break;
		}
		/* update filtering */
		(trace->filtering).low_pass_frequency= -1;
		(trace->filtering).high_pass_frequency= -1;
		/* update electrical imaging stuff */
		trace->calculate_rms=1;
		/* destroy any existing intervals */
		if (trace->first_interval)
		{
			destroy_Cardiac_interval_list(&trace->first_interval);
		}
		/* destroy any existing electrical imaging events */
		if (trace->first_eimaging_event)
		{
			destroy_Electrical_imaging_event_list(trace->first_eimaging_event);
		}
		trace_change_signal(trace);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_rig */
