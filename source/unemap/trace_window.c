/*******************************************************************************
FILE : trace_window.c

LAST MODIFIED : 6 December 1999

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <math.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
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
#include "unemap/analysis.h"
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
			display=user_interface->display;
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
						switch (trace->analysis_mode)
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
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_2->drawing->width,
										trace_area_2->drawing->height,
										trace_area_2->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_2->drawing->width,
										trace_area_2->drawing->height,
										trace_area_2->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
									trace_area_2->axes_left=axes_left;
									trace_area_2->axes_top=axes_top;
									trace_area_2->axes_width=axes_width;
									trace_area_2->axes_height=axes_height;
								}
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

static void set_analysis_event_detection(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

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
		if (EVENT_DETECTION!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->area_3.power_spectra.menu);
				} break;
				case CROSS_CORRELATION:
				{
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtUnmanageChild(trace->area_2.pane);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set event detection mode */
			trace->analysis_mode=EVENT_DETECTION;
			XtVaSetValues(trace->area_1.drawing_area,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopWidget,trace->area_1.enlarge.menu,
				NULL);
			XtManageChild(trace->area_1.enlarge.menu);
			XtVaSetValues(trace->area_3.drawing_area,
				XmNtopWidget,trace->area_3.edit.menu,
				NULL);
			XtManageChild(trace->area_3.edit.menu);
			redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
				(XtPointer)NULL);
			trace_update_signal_controls(trace);
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
LAST MODIFIED : 13 August 1997

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
		if (FREQUENCY_DOMAIN!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
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
					XtUnmanageChild(trace->area_3.power_spectra.menu);
				} break;
				case CROSS_CORRELATION:
				{
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_2.pane);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set frequency domain mode */
			trace->analysis_mode=FREQUENCY_DOMAIN;
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
LAST MODIFIED : 13 August 1997

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
		if (POWER_SPECTRA!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
				case EVENT_DETECTION:
				{
					XtUnmanageChild(trace->area_1.enlarge.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_3.edit.menu);
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
				} break;
				case CROSS_CORRELATION:
				{
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_2.pane);
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtUnmanageChild(trace->area_3.filtering.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_3.beat_averaging.menu);
				} break;
			}
			/* set power spectra mode */
			trace->analysis_mode=POWER_SPECTRA;
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
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Sets the analysis mode to cross correlation.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(set_analysis_cross_correlation);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (trace=(struct Trace_window *)trace_window)
	{
		/* if not already in cross correlation mode */
		if (CROSS_CORRELATION!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
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
			trace->analysis_mode=CROSS_CORRELATION;
			trace->valid_processing=0;
			XtManageChild(trace->area_2.pane);
			XtVaSetValues(trace->area_3.correlation.label,
				XmNlabelString,cross_correlation_string,
				NULL);
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
LAST MODIFIED : 13 August 1997

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
		if (AUTO_CORRELATION!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
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
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_2.pane);
				} break;
				case FILTERING:
				{
					XtUnmanageChild(trace->menu.apply_button);
					XtUnmanageChild(trace->area_3.filtering.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.correlation.menu,
						NULL);
					XtManageChild(trace->area_3.correlation.menu);
				} break;
				case BEAT_AVERAGING:
				{
					XtUnmanageChild(trace->menu.apply_button);
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
			trace->analysis_mode=AUTO_CORRELATION;
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
		if (FILTERING!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
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
				} break;
				case FREQUENCY_DOMAIN:
				{
					XtUnmanageChild(trace->area_3.frequency_domain.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
				} break;
				case POWER_SPECTRA:
				{
					XtUnmanageChild(trace->area_3.power_spectra.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
				} break;
				case CROSS_CORRELATION:
				{
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopAttachment,XmATTACH_FORM,
						NULL);
					XtUnmanageChild(trace->area_2.pane);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
				} break;
				case AUTO_CORRELATION:
				{
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.filtering.menu,
						NULL);
					XtManageChild(trace->area_3.filtering.menu);
					XtManageChild(trace->menu.apply_button);
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
			trace->analysis_mode=FILTERING;
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
		if (BEAT_AVERAGING!=trace->analysis_mode)
		{
			/* clear the previous mode */
			switch (trace->analysis_mode)
			{
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
					XtManageChild(trace->menu.apply_button);
				} break;
				case CROSS_CORRELATION:
				{
					XtUnmanageChild(trace->area_1.correlation_time_domain.menu);
					XtVaSetValues(trace->area_1.drawing_area,
						XmNtopWidget,trace->area_1.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_1.beat_averaging.menu);
					XtUnmanageChild(trace->area_2.pane);
					XtUnmanageChild(trace->area_3.correlation.menu);
					XtVaSetValues(trace->area_3.drawing_area,
						XmNtopWidget,trace->area_3.beat_averaging.menu,
						NULL);
					XtManageChild(trace->area_3.beat_averaging.menu);
					XtManageChild(trace->menu.apply_button);
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
			trace->analysis_mode=BEAT_AVERAGING;
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
LAST MODIFIED : 20 August 1997

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

static void identify_objective_absolute_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 August 1997

DESCRIPTION :
Saves the id of the trace enlarge objective absolute slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_objective_absolute_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.absolute_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_objective_absolute_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_objective_absolute_but */

static void identify_objective_positive_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 August 1997

DESCRIPTION :
Saves the id of the trace enlarge objective positive slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_objective_positive_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.positive_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_objective_positive_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_objective_positive_but */

static void identify_objective_negative_but(Widget *widget_id,
	XtPointer trace_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 August 1997

DESCRIPTION :
Saves the id of the trace enlarge objective negative slope button.
==============================================================================*/
{
	struct Trace_window *trace;

	ENTER(identify_objective_negative_but);
	USE_PARAMETER(call_data);
	if (trace=(struct Trace_window *)trace_window)
	{
		trace->area_1.enlarge.objective.negative_slope_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_objective_negative_but.  Missing trace_window");
	}
	LEAVE;
} /* identify_objective_negative_but */

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

static struct Trace_window *create_Trace_window(
	struct Trace_window **address,Widget activation,Widget parent,
	Pixel identifying_colour,enum Signal_analysis_mode analysis_mode,
	enum Event_detection_algorithm *detection,
	enum Event_detection_objective *objective,enum Datum_type *datum_type,
	enum Edit_order *edit_order,struct Device ***highlight,struct Rig **rig,
	int *datum,int *potential_time,int *event_number,int *number_of_events,
	int *threshold,int *minimum_separation,float *level,
	int *start_search_interval,int *end_search_interval,
	int screen_height,
		/*???DB.  height of interval drawing area ? */
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 December 1999

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
		{"identify_trace_1_area",(XtPointer)identify_trace_1_area},
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
		{"identify_objective_absolute_but",
			(XtPointer)identify_objective_absolute_but},
		{"identify_objective_positive_but",
			(XtPointer)identify_objective_positive_but},
		{"identify_objective_negative_but",
			(XtPointer)identify_objective_negative_but},
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
		{"identify_trace_1_drawing_area",(XtPointer)identify_trace_1_drawing_area},
		{"expose_trace_1_drawing_area",(XtPointer)redraw_trace_1_drawing_area},
		{"resize_trace_1_drawing_area",(XtPointer)redraw_trace_1_drawing_area},
		{"identify_trace_2_area",(XtPointer)identify_trace_2_area},
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
		{"identify_trace_edit_menu",(XtPointer)identify_trace_edit_menu},
		{"identify_trace_edit_previous_bu",
			(XtPointer)identify_trace_edit_previous_bu},
#if defined (OLD_CODE)
		{"identify_trace_edit_previous_ac",
			(XtPointer)identify_trace_edit_previous_ac},
#endif
		{"identify_trace_edit_next_button",
			(XtPointer)identify_trace_edit_next_button},
#if defined (OLD_CODE)
		{"identify_trace_edit_next_accele",
			(XtPointer)identify_trace_edit_next_accele},
#endif
		{"identify_trace_edit_accept_butt",
			(XtPointer)identify_trace_edit_accept_butt},
#if defined (OLD_CODE)
		{"identify_trace_edit_accept_acce",
			(XtPointer)identify_trace_edit_accept_acce},
#endif
		{"identify_trace_edit_reject_butt",
			(XtPointer)identify_trace_edit_reject_butt},
#if defined (OLD_CODE)
		{"identify_trace_edit_reject_acce",
			(XtPointer)identify_trace_edit_reject_acce},
#endif
		{"identify_trace_edit_order_choic",
			(XtPointer)identify_trace_edit_order_choic},
		{"identify_edit_order_device_butt",
			(XtPointer)identify_edit_order_device_butt},
		{"set_edit_order_device",(XtPointer)set_edit_order_device},
		{"identify_edit_order_beat_button",
			(XtPointer)identify_edit_order_beat_button},
		{"set_edit_order_beat",(XtPointer)set_edit_order_beat},
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
		{"identify_trace_3_drawing_area",(XtPointer)identify_trace_3_drawing_area},
		{"expose_trace_3_drawing_area",(XtPointer)redraw_trace_3_drawing_area},
		{"resize_trace_3_drawing_area",(XtPointer)redraw_trace_3_drawing_area}};
	static MrmRegisterArg identifier_list[]=
	{
		{"trace_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL}
	};
	struct Channel *imaginary_channel_1,*imaginary_channel_2,*processed_channel,
		*real_channel_1,*real_channel_2;
	struct Device **device,*imaginary_device_1,*imaginary_device_2,
		*processed_device,*real_device_1,*real_device_2;
	struct Device_description *imaginary_description_1,*imaginary_description_2,
		*processed_description,*real_description_1,*real_description_2;
	struct Signal *imaginary_signal_1,*imaginary_signal_2,*processed_signal,
		*real_signal_1,*real_signal_2;
	struct Signal_buffer *frequency_domain_buffer_1,*frequency_domain_buffer_2,
		*processed_buffer;
	struct Region *current_region;
	struct Signal *signal;
	struct Trace_window *trace;
	Widget child_widget;
	XmString value_xmstring;

	ENTER(create_Trace_window);
	/* check arguments */
	if (signal_drawing_information&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(trace_window_uidh,
			&trace_window_hierarchy,&trace_window_hierarchy_open))
		{
			/* allocate memory */
			if ((cross_correlation_string=XmStringCreateSimple("Cross correlation"))&&
				(auto_correlation_string=XmStringCreateSimple("Auto correlation"))&&
				ALLOCATE(trace,struct Trace_window,1))
			{
				widget_spacing=user_interface->widget_spacing;
				/* assign fields */
				trace->open=0;
				trace->address=address;
				trace->activation=activation;
				trace->shell=parent;
				trace->signal_drawing_information=signal_drawing_information;
				trace->user_interface=user_interface;
				trace->window=(Widget)NULL;
#if defined (SPECTRAL_TOOLS)
				trace->analysis_mode=analysis_mode;
#else
				trace->analysis_mode=EVENT_DETECTION;
#endif
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
				trace->area_1.enlarge.calculate_button=(Widget)NULL;
				trace->area_1.enlarge.detection_choice=(Widget)NULL;
				trace->area_1.enlarge.detection.interval_button=(Widget)NULL;
				trace->area_1.enlarge.detection.level_button=(Widget)NULL;
				trace->area_1.enlarge.detection.threshold_button=(Widget)NULL;
				trace->area_1.enlarge.objective_choice=(Widget)NULL;
				trace->area_1.enlarge.objective.absolute_slope_button=(Widget)NULL;
				trace->area_1.enlarge.objective.positive_slope_button=(Widget)NULL;
				trace->area_1.enlarge.objective.negative_slope_button=(Widget)NULL;
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
				trace->area_1.pane=(Widget)NULL;
				trace->area_1.drawing_area=(Widget)NULL;
				trace->area_1.drawing=(struct Drawing_2d *)NULL;
				trace->area_1.axes_left=0;
				trace->area_1.axes_top=0;
				trace->area_1.axes_width=0;
				trace->area_1.axes_height=0;
				trace->area_2.correlation_time_domain.menu=(Widget)NULL;
				trace->area_2.correlation_time_domain.toggle=(Widget)NULL;
				trace->area_2.pane=(Widget)NULL;
				trace->area_2.drawing_area=(Widget)NULL;
				trace->area_2.drawing=(struct Drawing_2d *)NULL;
				trace->area_2.axes_left=0;
				trace->area_2.axes_top=0;
				trace->area_2.axes_width=0;
				trace->area_2.axes_height=0;
				trace->area_3.edit.previous_button=(Widget)NULL;
#if defined (OLD_CODE)
				trace->area_3.edit.accelerator.previous_button=(Widget)NULL;
#endif
				trace->area_3.edit.next_button=(Widget)NULL;
#if defined (OLD_CODE)
				trace->area_3.edit.accelerator.next_button=(Widget)NULL;
#endif
				trace->area_3.edit.accept_button=(Widget)NULL;
#if defined (OLD_CODE)
				trace->area_3.edit.accelerator.accept_button=(Widget)NULL;
#endif
				trace->area_3.edit.reject_button=(Widget)NULL;
#if defined (OLD_CODE)
				trace->area_3.edit.accelerator.reject_button=(Widget)NULL;
#endif
				trace->area_3.edit.order_choice=(Widget)NULL;
				trace->area_3.edit.order.device_button=(Widget)NULL;
				trace->area_3.edit.order.beat_button=(Widget)NULL;
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
				trace->rig=rig;
				trace->event_detection.datum=datum;
				trace->event_detection.potential_time=potential_time;
				trace->event_detection.event_number=event_number;
				trace->event_detection.number_of_events=number_of_events;
				trace->event_detection.threshold=threshold;
				trace->event_detection.minimum_separation=minimum_separation;
				trace->event_detection.level=level;
				trace->event_detection.start_search_interval=start_search_interval;
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
				/* allocate memory for spectral analysis */
				if (
					(frequency_domain_buffer_1=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
					(frequency_domain_buffer_2=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
					(processed_buffer=create_Signal_buffer(FLOAT_VALUE,1,1,1)))
				{
					if ((real_channel_1=create_Channel(0,0,1))&&
						(imaginary_channel_1=create_Channel(1,0,1))&&
						(real_channel_2=create_Channel(0,0,1))&&
						(imaginary_channel_2=create_Channel(1,0,1))&&
						(processed_channel=create_Channel(1,0,1)))
					{
						if ((real_signal_1=create_Signal(0,frequency_domain_buffer_1,
							UNDECIDED,0))&&(imaginary_signal_1=create_Signal(1,
							frequency_domain_buffer_1,UNDECIDED,0))&&(real_signal_2=
							create_Signal(0,frequency_domain_buffer_2,UNDECIDED,0))&&
							(imaginary_signal_2=create_Signal(1,frequency_domain_buffer_2,
							UNDECIDED,0))&&(processed_signal=create_Signal(0,processed_buffer,
							UNDECIDED,0)))
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
									processed_channel,processed_signal)))
								{
									trace->real_device_1=real_device_1;
									trace->imaginary_device_1=imaginary_device_1;
									trace->real_device_2=real_device_2;
									trace->imaginary_device_2=imaginary_device_2;
									trace->processed_device=processed_device;
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
									destroy_Signal(&real_signal_1);
									destroy_Signal(&imaginary_signal_1);
									destroy_Signal(&real_signal_2);
									destroy_Signal(&imaginary_signal_2);
									destroy_Signal(&processed_signal);
									destroy_Channel(&real_channel_1);
									destroy_Channel(&imaginary_channel_1);
									destroy_Channel(&real_channel_2);
									destroy_Channel(&imaginary_channel_2);
									destroy_Channel(&processed_channel);
									destroy_Signal_buffer(&frequency_domain_buffer_1);
									destroy_Signal_buffer(&frequency_domain_buffer_2);
									destroy_Signal_buffer(&processed_buffer);
									trace->real_device_1=(struct Device *)NULL;
									trace->imaginary_device_1=(struct Device *)NULL;
									trace->real_device_2=(struct Device *)NULL;
									trace->imaginary_device_2=(struct Device *)NULL;
									trace->processed_device=(struct Device *)NULL;
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
								destroy_Signal(&processed_signal);
								destroy_Channel(&real_channel_1);
								destroy_Channel(&imaginary_channel_1);
								destroy_Channel(&real_channel_2);
								destroy_Channel(&imaginary_channel_2);
								destroy_Channel(&processed_channel);
								destroy_Signal_buffer(&frequency_domain_buffer_1);
								destroy_Signal_buffer(&frequency_domain_buffer_2);
								destroy_Signal_buffer(&processed_buffer);
								trace->real_device_1=(struct Device *)NULL;
								trace->imaginary_device_1=(struct Device *)NULL;
								trace->real_device_2=(struct Device *)NULL;
								trace->imaginary_device_2=(struct Device *)NULL;
								trace->processed_device=(struct Device *)NULL;
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
										}
									}
								}
							}
							destroy_Channel(&real_channel_1);
							destroy_Channel(&imaginary_channel_1);
							destroy_Channel(&real_channel_2);
							destroy_Channel(&imaginary_channel_2);
							destroy_Channel(&processed_channel);
							destroy_Signal_buffer(&frequency_domain_buffer_1);
							destroy_Signal_buffer(&frequency_domain_buffer_2);
							destroy_Signal_buffer(&processed_buffer);
							trace->real_device_1=(struct Device *)NULL;
							trace->imaginary_device_1=(struct Device *)NULL;
							trace->real_device_2=(struct Device *)NULL;
							trace->imaginary_device_2=(struct Device *)NULL;
							trace->processed_device=(struct Device *)NULL;
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
						trace->real_device_1=(struct Device *)NULL;
						trace->imaginary_device_1=(struct Device *)NULL;
						trace->real_device_2=(struct Device *)NULL;
						trace->imaginary_device_2=(struct Device *)NULL;
						trace->processed_device=(struct Device *)NULL;
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
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(trace_window_hierarchy,
					callback_list,XtNumber(callback_list)))
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
							switch (analysis_mode)
							{
								case EVENT_DETECTION:
								{
#else
									XtUnmanageChild(trace->menu.analysis_mode_choice);
#endif
									XtUnmanageChild(trace->menu.apply_button);
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
									XtUnmanageChild(trace->menu.apply_button);
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
										XmNtopWidget,trace->area_1.beat_averaging.menu,
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
#endif
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							/* adjust the enlarge detection choice */
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
#if defined (OLD_CODE)
									XtUnmanageChild(trace->area_1.enlarge.threshold_scroll);
									XtUnmanageChild(trace->area_1.enlarge.threshold_label);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_scroll);
									XtUnmanageChild(trace->area_1.enlarge.
										minimum_separation_label);
									XtUnmanageChild(trace->area_1.enlarge.all_current_choice);
									XtUnmanageChild(trace->area_1.enlarge.level_value);
#endif /* defined (OLD_CODE) */
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
/*???DB.								XmNcascadePixmap,no_cascade_pixmap,*/
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
#if defined (OLD_CODE)
							if (1== *number_of_events)
							{
								XtUnmanageChild(
									trace->area_1.enlarge.number_of_events.down_arrow);
							}
							else
							{
								XtManageChild(
									trace->area_1.enlarge.number_of_events.down_arrow);
							}
							if (9== *number_of_events)
							{
								XtUnmanageChild(
									trace->area_1.enlarge.number_of_events.up_arrow);
							}
							else
							{
								XtManageChild(trace->area_1.enlarge.number_of_events.up_arrow);
							}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.accept_button,
										True);
#endif
									XtSetSensitive(trace->area_3.edit.reject_button,True);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.reject_button,
										True);
#endif
								}
								else
								{
									/* ghost the accept and reject buttons */
									XtSetSensitive(trace->area_3.edit.accept_button,False);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.accept_button,
										False);
#endif
									XtSetSensitive(trace->area_3.edit.reject_button,False);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.reject_button,
										False);
#endif
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
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.next_button,
										False);
#endif
								}
								else
								{
									XtSetSensitive(trace->area_3.edit.next_button,True);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.next_button,
										True);
#endif
								}
								if ((0==device_number)&&
									(1== *(trace->event_detection.event_number)))
								{
									XtSetSensitive(trace->area_3.edit.previous_button,False);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.previous_button,
										False);
#endif
								}
								else
								{
									XtSetSensitive(trace->area_3.edit.previous_button,True);
#if defined (OLD_CODE)
									XtSetSensitive(trace->area_3.edit.accelerator.previous_button,
										True);
#endif
								}
							}
							else
							{
								/* ghost the accept and reject buttons */
								XtSetSensitive(trace->area_3.edit.accept_button,False);
#if defined (OLD_CODE)
								XtSetSensitive(trace->area_3.edit.accelerator.accept_button,
									False);
#endif
								XtSetSensitive(trace->area_3.edit.reject_button,False);
#if defined (OLD_CODE)
								XtSetSensitive(trace->area_3.edit.accelerator.reject_button,
									False);
#endif
								/* ghost the next and previous buttons */
								XtSetSensitive(trace->area_3.edit.next_button,False);
#if defined (OLD_CODE)
								XtSetSensitive(trace->area_3.edit.accelerator.next_button,
									False);
#endif
								XtSetSensitive(trace->area_3.edit.previous_button,False);
#if defined (OLD_CODE)
								XtSetSensitive(trace->area_3.edit.accelerator.previous_button,
									False);
#endif
							}
							install_accelerators(trace->window, trace->window);
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

/*
Global functions
----------------
*/
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
		display=signal_drawing_information->user_interface->display;
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
	enum Event_detection_algorithm detection,int number_of_events,
	Widget drawing_area,struct Drawing_2d *drawing,
	struct Signal_drawing_information *signal_drawing_information)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

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
		display=signal_drawing_information->user_interface->display;
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
				x_division=left+(i*width)/number_of_events;
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
	Pixel identifying_colour,enum Signal_analysis_mode analysis_mode,
	enum Event_detection_algorithm *detection,
	enum Event_detection_objective *objective,enum Datum_type *datum_type,
	enum Edit_order *edit_order,struct Device ***highlight,struct Rig **rig,
	int *datum,int *potential_time,int *event_number,int *number_of_events,
	int *threshold,int *minimum_separation,float *level,
	int *start_search_interval,int *end_search_interval,int screen_width,
	int screen_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

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
					objective,datum_type,edit_order,highlight,rig,datum,potential_time,
					event_number,number_of_events,threshold,minimum_separation,level,
					start_search_interval,end_search_interval,screen_height,
					signal_drawing_information,user_interface))
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
					/*???DB.  Temp */
					XtMapWidget(trace->area_1.enlarge.detection_choice);
					XtMapWidget(trace->area_1.enlarge.objective_choice);
					XtMapWidget(trace->area_1.enlarge.number_of_events_form);
					XtMapWidget(trace->area_1.enlarge.calculate_button);
					XtMapWidget(trace->area_1.enlarge.datum_choice);
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

void redraw_trace_1_drawing_area(Widget widget,XtPointer trace_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 1.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	Display *display;
	float x_scale;
	int analysis_range,axes_left,axes_height,axes_top,axes_width,
		end_analysis_interval,height,redraw,start_analysis_interval,width;
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
		display=user_interface->display;
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
						switch (trace->analysis_mode)
						{
							case EVENT_DETECTION: case BEAT_AVERAGING:
							{
								/* draw the active signal */
								if ((trace->highlight)&&(*(trace->highlight))&&
									(device= **(trace->highlight))&&
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										ENLARGE_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(device,ENLARGE_AREA_DETAIL,
										start_analysis_interval,end_analysis_interval,0,0,
										trace_area_1->drawing->width,trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
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
										switch (*(trace->event_detection.detection))
										{
											case EDA_INTERVAL:
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
									(buffer=get_Device_signal_buffer(device)))
								{
									start_analysis_interval=buffer->start;
									end_analysis_interval=buffer->end;
									/* draw the active signal */
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
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
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_1->drawing->width,
										trace_area_1->drawing->height,
										trace_area_1->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
									trace_area_1->axes_left=axes_left;
									trace_area_1->axes_top=axes_top;
									trace_area_1->axes_width=axes_width;
									trace_area_1->axes_height=axes_height;
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
LAST MODIFIED : 13 August 1997

DESCRIPTION :
The callback for redrawing part of the drawing area in trace area 3.
==============================================================================*/
{
	Display *display;
	int axes_height,axes_left,axes_top,axes_width,end_analysis_interval,height,
		redraw,signal_height,start_analysis_interval,width;
	struct Device *device;
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
	if ((trace=(struct Trace_window *)trace_window)&&
		(user_interface=trace->user_interface)&&
		(signal_drawing_information=trace->signal_drawing_information))
	{
		display=user_interface->display;
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
						switch (trace->analysis_mode)
						{
							case EVENT_DETECTION:
							{
								if ((trace->highlight)&&(*(trace->highlight))&&
									(device= **(trace->highlight)))
								{
									/* draw the active signal */
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
										trace_area_3->edit.last_data,0,0,
										trace_area_3->drawing->width,trace_area_3->drawing->height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,
										trace_area_3->edit.first_data,
										trace_area_3->edit.last_data,0,0,
										trace_area_3->drawing->width,trace_area_3->drawing->height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
									trace_area_3->axes_left=axes_left;
									trace_area_3->axes_top=axes_top;
									trace_area_3->axes_width=axes_width;
									trace_area_3->axes_height=axes_height;
									draw_device_markers(device,
										trace_area_3->edit.first_data,trace_area_3->edit.last_data,
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
									if (True==XmToggleButtonGadgetGetState((trace->area_3).
										beat_averaging.beat_averaging_toggle))
									{
										draw_signal(
											(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
											EDIT_AREA_DETAIL,buffer->start,buffer->end,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#if defined (OLD_CODE)
										draw_signal(device,EDIT_AREA_DETAIL,
											buffer->start,buffer->end,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#endif /* defined (OLD_CODE) */
										trace_area_3->axes_left=axes_left;
										trace_area_3->axes_top=axes_top;
										trace_area_3->axes_width=axes_width;
										trace_area_3->axes_height=axes_height;
									}
									else
									{
										/* draw the active signal */
										draw_signal(
											(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
											EDIT_AREA_DETAIL,trace_area_3->edit.first_data,
											trace_area_3->edit.last_data,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#if defined (OLD_CODE)
										draw_signal(device,EDIT_AREA_DETAIL,
											trace_area_3->edit.first_data,
											trace_area_3->edit.last_data,0,0,
											trace_area_3->drawing->width,
											trace_area_3->drawing->height,
											trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
											&axes_width,&axes_height,signal_drawing_information,
											user_interface);
#endif /* defined (OLD_CODE) */
										trace_area_3->axes_left=axes_left;
										trace_area_3->axes_top=axes_top;
										trace_area_3->axes_width=axes_width;
										trace_area_3->axes_height=axes_height;
										draw_device_markers(device,trace_area_3->edit.first_data,
											trace_area_3->edit.last_data,
											*(trace->event_detection.datum),1,
											*(trace->event_detection.potential_time),1,
											EDIT_AREA_DETAIL,*(trace->event_detection.event_number),
											axes_left,axes_top,axes_width,axes_height,(Window)NULL,
											trace_area_3->drawing->pixel_map,
											signal_drawing_information,user_interface);
									}
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
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#endif /* defined (OLD_CODE) */
									/* draw the imaginary/phase signal */
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,
										trace->imaginary_device_1,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,signal_height,
										trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#if defined (OLD_CODE)
									draw_signal(trace->imaginary_device_1,EDIT_AREA_DETAIL,
										start_analysis_interval,end_analysis_interval,0,
										signal_height,trace_area_3->drawing->width,signal_height,
										trace_area_3->drawing->pixel_map,&axes_left,&axes_top,
										&axes_width,&axes_height,signal_drawing_information,
										user_interface);
#endif /* defined (OLD_CODE) */
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
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#endif /* defined (OLD_CODE) */
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
									draw_signal(
										(struct FE_node *)NULL,(struct Draw_package *)NULL,device,
										EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#if defined (OLD_CODE)
									draw_signal(device,EDIT_AREA_DETAIL,start_analysis_interval,
										end_analysis_interval,0,0,trace_area_3->drawing->width,
										signal_height,trace_area_3->drawing->pixel_map,&axes_left,
										&axes_top,&axes_width,&axes_height,
										signal_drawing_information,user_interface);
#endif /* defined (OLD_CODE) */
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
	LEAVE;
} /* redraw_trace_3_drawing_area */

int trace_change_signal(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Called when the "highlighted_device" is changed.
???DB.  More work needed.  Pass device/signal ?
==============================================================================*/
{
	char value_string[20];
	float *averaged_value,beat_width,*imaginary_value,*imaginary_value_2,
		processed_frequency,*processed_value,*real_value,*real_value_2,*time_float,
		*times,*value,value_end,*values,value_start,x,x_2,y,y_2;
	int average_end,average_start,average_width=5,
		/*???DB.  average_width should be analysis->gradient_average_width */
		beat_number,beat_start,beat_end,buffer_offset,buffer_offset_2,end,high_pass,
		i,low_pass,notch,number_of_beats,number_of_samples,*processed_time,
		return_code,start,*time;
	struct Device *device,*processed_device;
	struct Signal_buffer *buffer,*processed_buffer;

	ENTER(trace_change_signal);
	if (trace)
	{
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION:
			{
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case BEAT_AVERAGING:
			{
				values=(float *)NULL;
				times=(float *)NULL;
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&extract_signal_information(
					(struct FE_node *)NULL,(struct Draw_package *)NULL,device,1,1,0,
					(int *)NULL,&number_of_samples,&times,&values,
					(enum Event_signal_status **)NULL,(char **)NULL,(int *)NULL,
					(float *)NULL,(float *)NULL)&&(0<number_of_samples)&&
					(processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->channel)&&
					(processed_buffer=processed_device->signal->buffer))
				{
					if (number_of_samples!=processed_buffer->number_of_samples)
					{
						if (REALLOCATE(processed_time,processed_buffer->times,int,
							number_of_samples)&&REALLOCATE(value,
							(processed_buffer->signals).float_values,float,number_of_samples))
						{
							processed_buffer->times=processed_time;
							(processed_buffer->signals).float_values=value;
							processed_buffer->number_of_samples=number_of_samples;
							processed_buffer->number_of_signals=1;
							trace->valid_processing=1;
						}
						else
						{
							DEALLOCATE(processed_time);
							display_message(ERROR_MESSAGE,
								"trace_change_signal.  Could not allocate processed buffer");
							trace->valid_processing=0;
						}
					}
					else
					{
						processed_time=processed_buffer->times;
						value=(processed_buffer->signals).float_values;
						trace->valid_processing=1;
					}
					if (trace->valid_processing)
					{
						/* copy the values */
						time_float=times;
						value=values;
						processed_time=processed_buffer->times;
						processed_value=((processed_buffer->signals).float_values)+
							(trace->processed_device->signal->index);
						buffer_offset_2=processed_buffer->number_of_signals;
						processed_device->channel->offset=0;
						processed_device->channel->gain=1;
						processed_device->signal_maximum=0;
						processed_device->signal_minimum=1;
						processed_frequency=(float)number_of_samples/
							(times[number_of_samples-1]-times[0]);
						processed_buffer->frequency=processed_frequency;
						for (i=number_of_samples;i>0;i--)
						{
							*processed_value= *value;
							processed_value += buffer_offset_2;
							value++;
							*processed_time=(int)((*time_float)*processed_frequency+0.5);
							processed_time++;
							time_float++;
						}
						/* do any processing */
						if (trace->valid_processing)
						{
							if (True==XmToggleButtonGadgetGetState((trace->area_3).
								beat_averaging.baseline_toggle))
							{
								start= *(trace->event_detection.start_search_interval);
								end= *(trace->event_detection.end_search_interval);
								number_of_beats= *(trace->event_detection.number_of_events);
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
								value=((processed_buffer->signals).float_values)+average_start;
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
									beat_end=start+(int)((float)((end-start)*beat_number)/
										(float)number_of_beats+0.5);
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
										((processed_buffer->signals).float_values)+average_start;
									value_end=0;
									for (i=average_end-average_start;i>0;i--)
									{
										value_end += *value;
										value++;
									}
									value_end /= (float)(average_end-average_start);
									value=((processed_buffer->signals).float_values)+beat_start;
									beat_width=(float)(beat_end-beat_start);
									for (i=beat_end-beat_start;i>=0;i--)
									{
										*value -= ((beat_width-(float)i)*value_end+
											(float)i*value_start)/beat_width;
										value++;
									}
								}
							}
							if (True==XmToggleButtonGadgetGetState((trace->area_3).
								beat_averaging.beat_averaging_toggle))
							{
								start= *(trace->event_detection.start_search_interval);
								end= *(trace->event_detection.end_search_interval);
								number_of_beats= *(trace->event_detection.number_of_events);
								beat_start=start;
								beat_end=beat_start+(end-start)/number_of_beats;
								if (0<beat_start)
								{
									averaged_value=(processed_buffer->signals).float_values;
									value=averaged_value+beat_start;
									processed_time=processed_buffer->times;
									time=processed_time+beat_start;
									for (i=beat_end-beat_start;i>0;i--)
									{
										*averaged_value= *value;
										*value=0;
										*processed_time= *time;
										processed_time++;
										time++;
										averaged_value++;
										value++;
									}
								}
								for (beat_number=1;beat_number<number_of_beats;beat_number++)
								{
									beat_start=start+(int)((float)((end-start)*beat_number)/
										(float)number_of_beats+0.5);
									beat_end=beat_start+(end-start)/number_of_beats;
									averaged_value=(processed_buffer->signals).float_values;
									value=averaged_value+beat_start;
									for (i=beat_end-beat_start;i>0;i--)
									{
										*averaged_value += *value;
										*value=0;
										averaged_value++;
										value++;
									}
								}
								averaged_value=(processed_buffer->signals).float_values;
								for (i=beat_end-beat_start;i>0;i--)
								{
									*averaged_value /= (float)number_of_beats;
									averaged_value++;
								}
								processed_buffer->start=0;
								processed_buffer->end=beat_end-beat_start-1;
								processed_buffer->number_of_samples=beat_end-beat_start;
							}
						}
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
#if defined (OLD_CODE)
				float channel_gain,channel_offset,*float_value;
				short int *short_int_value;

				/*???DB.  Doesn't do anything for linear combinations and probably
					shouldn't */
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(device->signal)&&(device->channel)&&
					(buffer=device->signal->buffer)&&
					(0<(number_of_samples=buffer->number_of_samples))&&
					(processed_device=trace->processed_device)&&
					(processed_device->signal)&&(processed_device->channel)&&
					(processed_buffer=processed_device->signal->buffer))
				{
					if (number_of_samples!=processed_buffer->number_of_samples)
					{
						if (REALLOCATE(processed_time,processed_buffer->times,int,
							number_of_samples)&&REALLOCATE(value,
							(processed_buffer->signals).float_values,float,number_of_samples))
						{
							processed_buffer->times=processed_time;
							(processed_buffer->signals).float_values=value;
							processed_buffer->number_of_samples=number_of_samples;
							processed_buffer->number_of_signals=1;
							trace->valid_processing=1;
						}
						else
						{
							DEALLOCATE(processed_time);
							display_message(ERROR_MESSAGE,
								"trace_change_signal.  Could not allocate processed buffer");
							trace->valid_processing=0;
						}
					}
					else
					{
						processed_time=processed_buffer->times;
						value=(processed_buffer->signals).float_values;
						trace->valid_processing=1;
					}
					if (trace->valid_processing)
					{
						/* copy the values */
						buffer_offset=buffer->number_of_signals;
						time=buffer->times;
						value=((processed_buffer->signals).float_values)+
							(trace->processed_device->signal->index);
						buffer_offset_2=processed_buffer->number_of_signals;
						channel_gain=device->channel->gain;
						channel_offset=device->channel->offset;
						processed_time=processed_buffer->times;
						processed_device->channel->offset=0;
						processed_device->channel->gain=1;
						processed_device->signal_maximum=0;
						processed_device->signal_minimum=1;
						processed_buffer->frequency=buffer->frequency;
						switch (buffer->value_type)
						{
							case SHORT_INT_VALUE:
							{
								short_int_value=((buffer->signals).short_int_values)+
									(device->signal->index);
								for (i=buffer->number_of_samples;i>0;i--)
								{
									*value=
										channel_gain*((float)(*short_int_value)-channel_offset);
									value += buffer_offset_2;
									short_int_value += buffer_offset;
									*processed_time= *time;
									processed_time++;
									time++;
								}
							} break;
							case FLOAT_VALUE:
							{
								float_value=((buffer->signals).float_values)+
									(device->signal->index);
								for (i=buffer->number_of_samples;i>0;i--)
								{
									*value=channel_gain*((*float_value)-channel_offset);
									value += buffer_offset_2;
									float_value += buffer_offset;
									*processed_time= *time;
									processed_time++;
									time++;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"trace_change_signal.  Unknown buffer type");
								trace->valid_processing=0;
							}
						}
						/* do any processing */
						if (trace->valid_processing)
						{
							if (True==XmToggleButtonGadgetGetState((trace->area_3).
								beat_averaging.baseline_toggle))
							{
								start= *(trace->event_detection.start_search_interval);
								end= *(trace->event_detection.end_search_interval);
								number_of_beats= *(trace->event_detection.number_of_events);
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
								value=((processed_buffer->signals).float_values)+average_start;
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
									beat_end=start+(int)((float)((end-start)*beat_number)/
										(float)number_of_beats+0.5);
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
										((processed_buffer->signals).float_values)+average_start;
									value_end=0;
									for (i=average_end-average_start;i>0;i--)
									{
										value_end += *value;
										value++;
									}
									value_end /= (float)(average_end-average_start);
									value=((processed_buffer->signals).float_values)+beat_start;
									beat_width=(float)(beat_end-beat_start);
									for (i=beat_end-beat_start;i>=0;i--)
									{
										*value -= ((beat_width-(float)i)*value_end+
											(float)i*value_start)/beat_width;
										value++;
									}
								}
							}
							if (True==XmToggleButtonGadgetGetState((trace->area_3).
								beat_averaging.beat_averaging_toggle))
							{
								start= *(trace->event_detection.start_search_interval);
								end= *(trace->event_detection.end_search_interval);
								number_of_beats= *(trace->event_detection.number_of_events);
								beat_start=start;
								beat_end=beat_start+(end-start)/number_of_beats;
								if (0<beat_start)
								{
									averaged_value=(processed_buffer->signals).float_values;
									value=averaged_value+beat_start;
									processed_time=processed_buffer->times;
									time=processed_time+beat_start;
									for (i=beat_end-beat_start;i>0;i--)
									{
										*averaged_value= *value;
										*value=0;
										*processed_time= *time;
										processed_time++;
										time++;
										averaged_value++;
										value++;
									}
								}
								for (beat_number=1;beat_number<number_of_beats;beat_number++)
								{
									beat_start=start+(int)((float)((end-start)*beat_number)/
										(float)number_of_beats+0.5);
									beat_end=beat_start+(end-start)/number_of_beats;
									averaged_value=(processed_buffer->signals).float_values;
									value=averaged_value+beat_start;
									for (i=beat_end-beat_start;i>0;i--)
									{
										*averaged_value += *value;
										*value=0;
										averaged_value++;
										value++;
									}
								}
								averaged_value=(processed_buffer->signals).float_values;
								for (i=beat_end-beat_start;i>0;i--)
								{
									*averaged_value /= (float)number_of_beats;
									averaged_value++;
								}
								processed_buffer->start=0;
								processed_buffer->end=beat_end-beat_start-1;
								processed_buffer->number_of_samples=beat_end-beat_start;
							}
						}
					}
				}
				else
				{
					trace->valid_processing=0;
				}
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
#endif /* defined (OLD_CODE) */
			} break;
			case FREQUENCY_DOMAIN:
			{
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
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case POWER_SPECTRA:
			{
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
					/* recalculate FFT */
						/*???DB.  Data window ? */
					if (trace->valid_processing=fourier_transform(SQUARE_WINDOW,
						device,(struct Device *)NULL,trace->real_device_1,
						trace->imaginary_device_1))
					{
						buffer=trace->real_device_1->signal->buffer;
						if ((trace->power_spectra).maximum_frequency<0)
						{
							(trace->power_spectra).maximum_frequency=
								(float)(buffer->number_of_samples)/(buffer->frequency);
							sprintf(value_string,"%g",
								(trace->power_spectra).maximum_frequency);
							XtVaSetValues(
								(trace->area_3).power_spectra.maximum_frequency_text_field,
								XmNvalue,value_string,
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
							sprintf(value_string,"%g",
								(trace->power_spectra).minimum_frequency);
							XtVaSetValues(
								(trace->area_3).power_spectra.minimum_frequency_text_field,
								XmNvalue,value_string,
								XmNeditable,True,
								NULL);
						}
						XtVaSetValues(
							(trace->area_3).power_spectra.minimum_frequency_scroll_bar,
							XmNvalue,(int)(((trace->power_spectra).minimum_frequency)*
							(buffer->frequency)*100./(buffer->number_of_samples)+0.5),
							NULL);
						/* calculate power spectrum */
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
							*real_value=sqrt(x*x+y*y);
						}
					}
				}
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case CROSS_CORRELATION:
			{
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
				if ((trace->correlation.device_1)&&(trace->correlation.device_2))
				{
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
							trace->real_device_1,trace->imaginary_device_1,
							trace->processed_device,(struct Device *)NULL);
						/* undo "wrap-around" order so that have negative and positive
							lags */
						buffer=trace->processed_device->signal->buffer;
						real_value=((buffer->signals).float_values)+
							(trace->processed_device->signal->index);
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
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case AUTO_CORRELATION:
			{
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
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
							trace->processed_device,(struct Device *)NULL);
						/* auto-correlation is symmetric */
						trace->processed_device->signal->buffer->end=(trace->
							processed_device->signal->buffer->number_of_samples)/2-1;
					}
				}
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
				redraw_trace_3_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
			} break;
			case FILTERING:
			{
				if ((trace->highlight)&&(*(trace->highlight))&&
					(device= **(trace->highlight))&&(trace->real_device_1))
				{
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
							sprintf(value_string,"%g",(trace->filtering).low_pass_frequency);
							XtVaSetValues((trace->area_3).filtering.low_pass_text_field,
								XmNvalue,value_string,
								XmNeditable,True,
								NULL);
						}
						if ((trace->filtering).high_pass_frequency<0)
						{
							(trace->filtering).high_pass_frequency=0.;
							sprintf(value_string,"%g",(trace->filtering).high_pass_frequency);
							XtVaSetValues((trace->area_3).filtering.high_pass_text_field,
								XmNvalue,value_string,
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
							sprintf(value_string,"%g",(trace->filtering).notch_frequency);
							XtVaSetValues((trace->area_3).filtering.notch_text_field,
								XmNvalue,value_string,
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
							trace->real_device_1,trace->imaginary_device_1,
							trace->processed_device,(struct Device *)NULL);
						extract_signal_information((struct FE_node *)NULL,
							(struct Draw_package *)NULL,device,1,1,0,(int *)NULL,
							&number_of_samples,(float **)NULL,(float **)NULL,
							(enum Event_signal_status **)NULL,(char **)NULL,(int *)NULL,
							(float *)NULL,(float *)NULL);
						trace->processed_device->signal->buffer->end=number_of_samples-1;
#if defined (OLD_CODE)
						buffer=device->signal->buffer;
						trace->processed_device->signal->buffer->end=
							(buffer->end)-(buffer->start);
#endif /* defined (OLD_CODE) */
					}
				}
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
LAST MODIFIED : 13 August 1997

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
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
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
		switch (trace->analysis_mode)
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
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION: case FREQUENCY_DOMAIN: case POWER_SPECTRA:
				case AUTO_CORRELATION: case BEAT_AVERAGING:
			{
				redraw_trace_1_drawing_area((Widget)NULL,(XtPointer)trace,
					(XtPointer)NULL);
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
LAST MODIFIED : 30 November 1999

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
		switch (trace->analysis_mode)
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
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Updates the selectability of the signal controls.
==============================================================================*/
{
	int device_number,event_number,i,return_code;
	struct Device **device,**highlight;
	struct Event *event;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(trace_update_signal_controls);
	if (trace)
	{
		if ((trace->rig)&&(rig= *(trace->rig))&&(trace->highlight)&&
			(highlight= *(trace->highlight)))
		{
			switch (trace->analysis_mode)
			{
				case EVENT_DETECTION:
				{
					/* ghost/unghost the calculate button */
					if (!(trace->area_1.enlarge.calculate_all_events)&&
						(REJECTED==(*highlight)->signal->status))
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
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.accept_button,True);
#endif
						XtSetSensitive(trace->area_3.edit.reject_button,True);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.reject_button,True);
#endif
					}
					else
					{
						XtSetSensitive(trace->area_3.edit.accept_button,False);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.accept_button,False);
#endif
						XtSetSensitive(trace->area_3.edit.reject_button,False);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.reject_button,False);
#endif
					}
					/* ghost/unghost the next and previous buttons */
					if (current_region=rig->current_region)
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
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.next_button,False);
#endif
					}
					else
					{
						XtSetSensitive(trace->area_3.edit.next_button,True);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.next_button,True);
#endif
					}
					if ((0==device_number)&&(!event||((event->number>=event_number)&&
						!(event->previous))))
					{
						XtSetSensitive(trace->area_3.edit.previous_button,False);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.previous_button,
							False);
#endif
					}
					else
					{
						XtSetSensitive(trace->area_3.edit.previous_button,True);
#if defined (OLD_CODE)
						XtSetSensitive(trace->area_3.edit.accelerator.previous_button,True);
#endif
					}
				} break;
			}
			return_code=1;
		}
		else
		{
			switch (trace->analysis_mode)
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
		switch (trace->analysis_mode)
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
					signal_minimum=(*highlight)->signal_minimum;
					signal_maximum=(*highlight)->signal_maximum;
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

int trace_draw_potential_time(struct Trace_window *trace)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Draws the potential time marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int end_analysis_interval,potential_time,return_code,signal_index,
		start_analysis_interval;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(trace_draw_potential_time);
	if (trace)
	{
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if ((trace->event_detection.potential_time)&&(trace->highlight)&&
					(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
					(buffer=get_Device_signal_buffer(highlight_device)))
				{
					potential_time= *(trace->event_detection.potential_time);
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
					draw_potential_time_marker(potential_time,buffer,
						highlight_device->channel,signal_index,ENLARGE_AREA_DETAIL,
						start_analysis_interval,end_analysis_interval,
						trace_area_1->axes_left,trace_area_1->axes_top,
						trace_area_1->axes_width,trace_area_1->axes_height,
						XtWindow(trace_area_1->drawing_area),
						trace_area_1->drawing->pixel_map,trace->signal_drawing_information,
						trace->user_interface);
					trace_area_3= &(trace->area_3);
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
						"trace_draw_potential_time.  Invalid potential_time");
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
} /* trace_draw_potential_time */

int trace_update_potential_time(struct Trace_window *trace, int potential_time,
	int previous_potential_time, enum Trace_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Updates the potential time marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	int end_analysis_interval,return_code,signal_index,start_analysis_interval;
	struct Device *highlight_device;
	struct Signal_buffer *buffer;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;

	ENTER(trace_draw_potential_time);
	if (trace)
	{
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
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
					if(*update_flags & TRACE_1_NO_POTENTIAL_ERASE)
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
					if((*update_flags & TRACE_3_NO_POTENTIAL_ERASE))
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
						"trace_draw_potential_time.  Invalid potential_time");
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
} /* trace_update_potential_time */

int trace_update_datum(struct Trace_window *trace, int datum,
	int previous_datum, int event_number, int potential_time,
	enum Trace_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Updates the datum marker in the <trace> window.
???DB.  Beat averaging sharing with event detection
==============================================================================*/
{
	char delay_time_string[20];
	Display *display;
	int ascent,descent,direction,end_analysis_interval,length,return_code, 
		start_analysis_interval,*times,x_string,y_string;
	float frequency;
	GC event_graphics_context, event_graphics_context_text;
	struct Device *highlight_device;
	struct Event *event;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Trace_window_area_1 *trace_area_1;
	struct Trace_window_area_3 *trace_area_3;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(trace_draw_datum);
	if (trace)
	{
		switch (trace->analysis_mode)
		{
			case EVENT_DETECTION: case BEAT_AVERAGING:
			{
				if ((trace->highlight)&&
					(signal_drawing_information=trace->signal_drawing_information)&&
					(*(trace->highlight))&&(highlight_device= **(trace->highlight))&&
					(buffer=get_Device_signal_buffer(highlight_device))&&
					(times=buffer->times))
				{
					display=trace->user_interface->display;
					start_analysis_interval=buffer->start;
					end_analysis_interval=buffer->end;
					frequency=buffer->frequency;
					font=signal_drawing_information->font;

					trace_area_1= &(trace->area_1);
					if(*update_flags & TRACE_1_NO_DATUM_ERASE)
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

					event=highlight_device->signal->first_event;
					while (event&&(event->number<event_number))
					{
						event=event->next;
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
									"trace_draw_datum.  Invalid event status");
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

					trace_area_3= &(trace->area_3);
					if(*update_flags & TRACE_3_NO_DATUM_ERASE)
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

					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"trace_draw_datum.  Invalid potential_time");
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
		switch (trace->analysis_mode)
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
LAST MODIFIED : 30 November 1999

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
					edit_start= *(trace->event_detection.start_search_interval);
					edit_diff= *(trace->event_detection.end_search_interval)-edit_start;
					trace->area_3.edit.first_data=edit_start+(int)((float)(edit_diff*
						((*(trace->event_detection.event_number))-1))/
						(float)(*(trace->event_detection.number_of_events))+0.5);
					trace->area_3.edit.last_data=edit_start+(int)((float)(edit_diff*
						(*(trace->event_detection.event_number)))/
						(float)(*(trace->event_detection.number_of_events))+0.5);
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
					if (event=(**(trace->highlight))->signal->first_event)
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
LAST MODIFIED : 30 November 1999

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
		trace_change_signal(trace);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* trace_change_rig */
