/*******************************************************************************
FILE : analysis_window.c

LAST MODIFIED : 11 February 2000

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
#include <Xm/PushBG.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/mystring.h"
#include "general/postscript.h"
#include "unemap/analysis.h"
#include "unemap/analysis_window.h"
#include "unemap/analysis_window.uidh"
#include "unemap/rig.h"
#if defined (UNEMAP_USE_NODES)
#include "unemap/rig_node.h"
#endif /* defined (UNEMAP_USE_NODES) */
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/printer.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct Analysis_window Analysis_window_settings;

/*
Global variables
----------------
*/
/*???DB.  Have to do this because analysis_work_area needs access */
#if defined (MOTIF)
int analysis_window_hierarchy_open=0;
MrmHierarchy analysis_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void identify_analysis_reset_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
Finds the id of the analysis reset button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_reset_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.reset_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_reset_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_reset_button */

static void identify_analysis_baseline_butt(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis baseline button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_baseline_butt);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.baseline_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_baseline_butt.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_baseline_butt */

static void identify_analysis_range_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis range button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_range_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.range_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_range_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_range_button */

static void identify_analysis_previous_butt(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis previous button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_previous_butt);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.previous_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_previous_butt.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_previous_butt */

#if defined (OLD_CODE)
static void identify_analysis_previous_acce(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the accelerator for the analysis previous button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_previous_acce);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.accelerator.previous_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_previous_acce.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_previous_acce */
#endif

static void identify_analysis_next_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis next button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_next_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.next_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_next_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_next_button */

#if defined (OLD_CODE)
static void identify_analysis_next_accelera(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the accelerator for the analysis next button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_next_accelera);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.accelerator.next_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_next_accelera.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_next_accelera */
#endif

static void identify_analysis_accept_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis accept button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_accept_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.accept_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_accept_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_accept_button */

#if defined (OLD_CODE)
static void identify_analysis_accept_accele(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the accelerator for the analysis accept button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_accept_accele);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.accelerator.accept_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_accept_accele.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_accept_accele */
#endif

static void identify_analysis_reject_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis reject button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_reject_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.reject_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_reject_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_reject_button */

#if defined (OLD_CODE)
static void identify_analysis_reject_accele(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the identifier for the analysis reject button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_reject_accele);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.accelerator.reject_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_reject_accele.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_reject_accele */
#endif

static void identify_analysis_close_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis close button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_close_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_close_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_close_button */

static void identify_analysis_menu(Widget *widget_id,XtPointer analysis_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis menu.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_menu);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_menu.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_menu */

static void identify_analysis_map_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis map button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_map_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_map_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_map_button */

static void identify_analysis_map_potential(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis potential button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_map_potential);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_menu.potential_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_map_potential.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_map_potential */

static void identify_analysis_map_integral(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Finds the id of the analysis integral button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_map_integral);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_menu.integral_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_map_integral.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_map_integral */

static void identify_analysis_map_gradient(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis gradient button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_map_gradient);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_menu.gradient_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_map_gradient.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_map_gradient */

static void id_analysis_map_single_activati(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Finds the id of the analysis single activation button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(id_analysis_map_single_activati);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_menu.single_activation_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_analysis_map_single_activati.  Missing analysis_window");
	}
	LEAVE;
} /* id_analysis_map_single_activati */

static void id_analysis_map_multiple_activa(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Finds the id of the analysis single activation button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(id_analysis_map_multiple_activa);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->map_menu.multiple_activation_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_analysis_map_multiple_activa.  Missing analysis_window");
	}
	LEAVE;
} /* id_analysis_map_multiple_activa */

static void identify_analysis_file_button(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_button.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_file_button */

static void identify_analysis_file_read_sig(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file read signals button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_sig);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_menu.read_signals_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_sig.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_file_read_sig */

static void identify_analysis_file_overlay(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Finds the id of the analysis file overlay signals button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_overlay);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_menu.overlay_signals_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_overlay.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_file_overlay */

static void identify_analysis_file_save_int(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file save interval button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_save_int);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_menu.save_interval_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_save_int.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_file_save_int */

static void id_analysis_file_save_int_as(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file save interval as button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(id_analysis_file_save_int_as);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_menu.save_interval_as_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"id_analysis_file_save_int_as.  client_data missing");
	}
	LEAVE;
} /* id_analysis_file_save_int_as */

static void identify_analysis_file_save_tim(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file save times button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_save_tim);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->file_menu.save_times_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_save_tim.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_file_save_tim */

static int write_times_undecided_accepted(char *file_name,
	void *analysis_window)
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
==============================================================================*/
{
	FILE *output_file;
	float frequency;
	int event_number,i,number_of_devices,number_of_events,return_code,*times;
	struct Analysis_window *analysis;
	struct Device **device;
	struct Event *event;
	struct Rig *rig;

	ENTER(write_times_undecided_accepted);
	if ((analysis=(struct Analysis_window *)analysis_window)&&
		(rig= *(analysis->rig))&&(device=rig->devices)&&(*device)&&
		((number_of_devices=rig->number_of_devices)>0))
	{
		i=number_of_devices;
		while ((i>0)&&!((*device)->signal))
		{
			device++;
			i--;
		}
		if ((i>0)&&((*device)->signal)&&((*device)->signal->buffer)&&
			(times=(*device)->signal->buffer->times))
		{
			/* determine the number of activations that are being output */
			number_of_events=0;
			while (i>0)
			{
				if (((*device)->signal)&&(event=(*device)->signal->first_event))
				{
					while (event->next)
					{
						event=event->next;
					}
					if (number_of_events<event->number)
					{
						number_of_events=event->number;
					}
				}
				i--;
				device++;
			}
			if (0<number_of_events)
			{
				if (output_file=fopen(file_name,"w"))
				{
					device=rig->devices;
					frequency=(*device)->signal->buffer->frequency;
					/* write the signal file name */
					fprintf(output_file,"signal file name : %s\n",rig->signal_file_name);
					/* write the table format */
					fprintf(output_file,"table format : comma separated\n");
					/* write the detection method */
					fprintf(output_file,"detection method :");
					switch (*(analysis->detection))
					{
						case EDA_INTERVAL:
						{
							fprintf(output_file," interval\n");
							/* write the start search and end search sample numbers and the
								number of events being searched for */
							fprintf(output_file,
								"search start : %.0f, search end : %.0f, search events : %d\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency,*(analysis->number_of_events));
						} break;
						case EDA_LEVEL:
						{
							fprintf(output_file," level\n");
							/* write the start search and end search sample numbers */
							fprintf(output_file,"search start : %.0f, search end : %.0f\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency);
							/* write the level */
							fprintf(output_file,"level : %g, width : %d\n",
								*(analysis->level),*(analysis->average_width));
						} break;
						case EDA_THRESHOLD:
						{
							fprintf(output_file," threshold\n");
							/* write the start search and end search sample numbers */
							fprintf(output_file,"search start : %.0f, search end : %.0f\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency);
							/* write the threshold and minimum separation */
							fprintf(output_file,
								"threshold : %d%%, minimum separation : %d ms\n",
								*(analysis->threshold),*(analysis->minimum_separation));
						} break;
					}
					/* write the number of activations that are being output */
					fprintf(output_file,"number of events : %d\n\n",number_of_events);
					/* write the heading line */
					fprintf(output_file,"Electrode");
					for (i=1;i<=number_of_events;i++)
					{
						fprintf(output_file,",%d",i);
					}
					fprintf(output_file,"\n");
					/* write the channel names and event times */
					for (i=number_of_devices;i>0;i--)
					{
						if (((*device)->signal)&&(REJECTED!=(*device)->signal->status))
						{
							/* write the device name */
							fprintf(output_file,"%s",(*device)->description->name);
							event=(*device)->signal->first_event;
							for (event_number=1;event_number<=number_of_events;event_number++)
							{
								if (event&&(event_number==event->number))
								{
									switch (event->status)
									{
										case ACCEPTED: case UNDECIDED:
										{
											fprintf(output_file,",%g",
												(float)(times[event->time])*1000./frequency);
										} break;
										case REJECTED:
										{
											fprintf(output_file,",n/a");
										} break;
									}
									event=event->next;
								}
								else
								{
									fprintf(output_file,",none");
								}
							}
							fprintf(output_file,"\n");
						}
						device++;
					}
					/* write the datum */
					fprintf(output_file,"Reference %g\n",
						(float)(times[*(analysis->datum)])*1000./frequency);
#if defined (OLD_CODE)
					fprintf(output_file,"Electrode  ");
					for (i=1;i<=number_of_events;i++)
					{
						fprintf(output_file,"%-6d",i);
					}
					fprintf(output_file,"\n");
					/* write the channel names and event times */
					for (i=number_of_devices;i>0;i--)
					{
						if (REJECTED!=(*device)->signal->status)
						{
							/* write the device name */
							fprintf(output_file,"%-10.10s ",(*device)->description->name);
							event=(*device)->signal->first_event;
							for (event_number=1;event_number<=number_of_events;event_number++)
							{
								if (event&&(event_number==event->number))
								{
									switch (event->status)
									{
										case ACCEPTED: case UNDECIDED:
										{
											fprintf(output_file,"%-6.0f",
												(float)(times[event->time])*1000./frequency);
										} break;
										case REJECTED:
										{
											fprintf(output_file,"n/a   ");
										} break;
									}
									event=event->next;
								}
								else
								{
									fprintf(output_file,"none  ");
								}
							}
							fprintf(output_file,"\n");
						}
						device++;
					}
					/* write the datum */
					fprintf(output_file,"Reference  %-6.0f\n",
						(float)(times[*(analysis->datum)])*1000./frequency);
#endif /* defined (OLD_CODE) */
					fclose(output_file);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_times_undecided_accepted.  Invalid file: %s",file_name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_times_undecided_accepted.  No events");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_times_undecided_accepted.  Invalid rig");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_times_undecided_accepted.  Invalid rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_times_undecided_accepted */

static int write_times_undecided_rejected(char *file_name,
	void *analysis_window)
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
==============================================================================*/
{
	FILE *output_file;
	float frequency;
	int event_number,i,number_of_devices,number_of_events,return_code,*times;
	struct Analysis_window *analysis;
	struct Device **device;
	struct Event *event;
	struct Rig *rig;

	ENTER(write_times_undecided_rejected);
	if ((analysis=(struct Analysis_window *)analysis_window)&&
		(rig= *(analysis->rig))&&(device=rig->devices)&&(*device)&&
		((number_of_devices=rig->number_of_devices)>0))
	{
		/* determine the number of activations that are being output */
		i=number_of_devices;
		while ((i>0)&&!((*device)->signal))
		{
			device++;
			i--;
		}
		if ((i>0)&&((*device)->signal)&&((*device)->signal->buffer)&&
			(times=(*device)->signal->buffer->times))
		{
			/* determine the number of activations that are being output */
			number_of_events=0;
			while (i>0)
			{
				if (((*device)->signal)&&(event=(*device)->signal->first_event))
				{
					while (event->next)
					{
						event=event->next;
					}
					if (number_of_events<event->number)
					{
						number_of_events=event->number;
					}
				}
				i--;
				device++;
			}
			if (0<number_of_events)
			{
				if (output_file=fopen(file_name,"w"))
				{
					device=rig->devices;
					frequency=(*device)->signal->buffer->frequency;
					/* write the signal file name */
					fprintf(output_file,"signal file name : %s\n",rig->signal_file_name);
					/* write the table format */
					fprintf(output_file,"table format : comma separated\n");
					/* write the detection method */
					fprintf(output_file,"detection method :");
					switch (*(analysis->detection))
					{
						case EDA_INTERVAL:
						{
							fprintf(output_file," interval\n");
							/* write the start search and end search sample numbers and the
								number of events being searched for */
							fprintf(output_file,
								"search start : %.0f, search end : %.0f, search events : %d\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency,*(analysis->number_of_events));
						} break;
						case EDA_LEVEL:
						{
							fprintf(output_file," level\n");
							/* write the start search and end search sample numbers */
							fprintf(output_file,"search start : %.0f, search end : %.0f\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency);
							/* write the level */
							fprintf(output_file,"level : %g, width : %d\n",
								*(analysis->level),*(analysis->average_width));
						} break;
						case EDA_THRESHOLD:
						{
							fprintf(output_file," threshold\n");
							/* write the start search and end search sample numbers */
							fprintf(output_file,"search start : %.0f, search end : %.0f\n",
								(float)times[*(analysis->start_search_interval)]*1000./
								frequency,(float)times[*(analysis->end_search_interval)]*1000./
								frequency);
							/* write the threshold and minimum separation */
							fprintf(output_file,
								"threshold : %d%%, minimum separation : %d ms\n",
								*(analysis->threshold),*(analysis->minimum_separation));
						} break;
					}
					/* write the number of activations that are being output */
						/*???DB.  Change number of events ? */
					fprintf(output_file,"number of events : %d\n\n",number_of_events);
					/* write the heading line */
					fprintf(output_file,"Electrode");
					for (i=1;i<=number_of_events;i++)
					{
						fprintf(output_file,",%d",i);
					}
					fprintf(output_file,"\n");
					/* write the channel names and event times */
					device=rig->devices;
					frequency=(*device)->signal->buffer->frequency;
					for (i=number_of_devices;i>0;i--)
					{
						if (((*device)->signal)&&(REJECTED!=(*device)->signal->status))
						{
							/* write the device name */
							fprintf(output_file,"%s",(*device)->description->name);
							event=(*device)->signal->first_event;
							for (event_number=1;event_number<=number_of_events;event_number++)
							{
								if (event&&(event_number==event->number))
								{
									switch (event->status)
									{
										case ACCEPTED:
										{
											fprintf(output_file,",%g",
												(float)(times[event->time])*1000./frequency);
										} break;
										case REJECTED: case UNDECIDED:
										{
											fprintf(output_file,",n/a");
										} break;
									}
									event=event->next;
								}
								else
								{
									fprintf(output_file,",none");
								}
							}
							fprintf(output_file,"\n");
						}
						device++;
					}
					/* write the datum */
					fprintf(output_file,"Reference %g\n",
						(float)(times[*(analysis->datum)])*1000./frequency);
#if defined (OLD_CODE)
					fprintf(output_file,"Electrode  ");
					for (i=1;i<=number_of_events;i++)
					{
						fprintf(output_file,"%-6d",i);
					}
					fprintf(output_file,"\n");
					/* write the channel names and event times */
					device=rig->devices;
					frequency=(*device)->signal->buffer->frequency;
					for (i=number_of_devices;i>0;i--)
					{
						if (REJECTED!=(*device)->signal->status)
						{
							/* write the device name */
							fprintf(output_file,"%-10.10s ",(*device)->description->name);
							event=(*device)->signal->first_event;
							for (event_number=1;event_number<=number_of_events;event_number++)
							{
								if (event&&(event_number==event->number))
								{
									switch (event->status)
									{
										case ACCEPTED:
										{
											fprintf(output_file,"%-6.0f",
												(float)(times[event->time])*1000./frequency);
										} break;
										case REJECTED: case UNDECIDED:
										{
											fprintf(output_file,"n/a   ");
										} break;
									}
									event=event->next;
								}
								else
								{
									fprintf(output_file,"none  ");
								}
							}
							fprintf(output_file,"\n");
						}
						device++;
					}
					/* write the datum */
					fprintf(output_file,"Reference  %-6.0f\n",
						(float)(times[*(analysis->datum)])*1000./frequency);
#endif /* defined (OLD_CODE) */
					fclose(output_file);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_times_undecided_rejected.  Invalid file: %s",file_name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_times_undecided_rejected.  No events");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_times_undecided_rejected.  Invalid rig");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_times_undecided_rejected.  Invalid rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_times_undecided_rejected */

static void set_write_times_undecided_accepted(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(set_write_times_undecided_accepted);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (analysis->write_times_file_open_data)
		{
			busy_cursor_off(analysis->write_event_times_warning_box_s,
				analysis->user_interface);
			analysis->write_times_file_open_data->operation=
				write_times_undecided_accepted;
			open_file_and_write(analysis->file_menu.save_times_button,
				(XtPointer)analysis->write_times_file_open_data,(XtPointer)NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"set_write_times_undecided_accepted.  Missing write_times_file_open_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_write_times_undecided_accepted.  Missing analysis window");
	}
	LEAVE;
} /* set_write_times_undecided_accepted */

static void set_write_times_undecided_rejected(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(set_write_times_undecided_rejected);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (analysis->write_times_file_open_data)
		{
			busy_cursor_off(analysis->write_event_times_warning_box_s,
				analysis->user_interface);
			analysis->write_times_file_open_data->operation=
				write_times_undecided_rejected;
			open_file_and_write(analysis->file_menu.save_times_button,
				(XtPointer)analysis->write_times_file_open_data,(XtPointer)NULL);
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"set_write_times_undecided_rejected.  Missing write_times_file_open_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_write_times_undecided_rejected.  Missing analysis window");
	}
	LEAVE;
} /* set_write_times_undecided_rejected */

static void close_write_event_times_warning(Widget widget,
	XtPointer warning_box_widget,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
==============================================================================*/
{
	Widget warning_box;

	ENTER(close_write_event_times_warning);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (warning_box=(Widget)warning_box_widget)
	{
		XtUnmanageChild(warning_box);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_write_event_times_warning.  warning_box_widget is missing");
	}
	LEAVE;
} /* close_write_event_times_warning */

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
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis_window=(struct Analysis_window *)analysis_window_void)
	{
		busy_cursor_off(analysis_window->write_event_times_warning_box_s,
			analysis_window->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"busy_cursor_off_warning_box_shell.  Missing file_open_data");
	}

	LEAVE;
} /* busy_cursor_off_warning_box_shell */

static void write_event_times_file(Widget widget,XtPointer analysis_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Check the analysis rig for events and for undecided events.  Prompt user for
continuation.  Write the file.
==============================================================================*/
{
	char no_event,no_undecided;
	int number_of_devices;
	MrmType warning_box_class;
	struct Analysis_window *analysis;
	struct Device **device;
	struct Event *event;
	struct Rig *rig;
	Widget parent,warning_box,warning_box_shell;

	ENTER(write_event_times_file);
	USE_PARAMETER(call_data);
	if ((analysis=(struct Analysis_window *)analysis_window)&&
		(rig= *(analysis->rig))&&(device=rig->devices)&&
		((number_of_devices=rig->number_of_devices)>0))
	{
		no_event=1;
		no_undecided=1;
		while ((number_of_devices>0)&&no_undecided)
		{
			if (event=(*device)->signal->first_event)
			{
				no_event=0;
				while (event&&no_undecided)
				{
					if (UNDECIDED==event->status)
					{
						no_undecided=0;
					}
					event++;
				}
			}
			device++;
			number_of_devices--;
		}
		if (no_event)
		{
			display_message(WARNING_MESSAGE,"No events to write");
		}
		else
		{
			if (no_undecided)
			{
				open_file_and_write(analysis->file_menu.save_times_button,
					(XtPointer)analysis->write_times_file_open_data,
					(XtPointer)NULL);
			}
			else
			{
				if (!(warning_box_shell=analysis->write_event_times_warning_box_s))
				{
					if ((!(parent=widget))||(True!=XtIsWidget(parent)))
					{
						parent=analysis->user_interface->application_shell;
					}
					if (warning_box_shell=XtVaCreatePopupShell(
						"write_event_times_warning_shell",
						xmDialogShellWidgetClass,parent,
						XmNtitle,"Warning",
						NULL))
					{
						analysis->write_event_times_warning_box_s=warning_box_shell;
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
							analysis->write_event_times_warning_box=warning_box;
							/* add the accept all undecided callback */
							XtAddCallback(warning_box,XmNokCallback,
								set_write_times_undecided_accepted,analysis_window);
							/* add the reject all undecided callback */
							XtAddCallback(warning_box,XmNcancelCallback,
								set_write_times_undecided_rejected,analysis_window);
							/* add the cancel callbacks */
							XtAddCallback(warning_box,XmNhelpCallback,
								close_write_event_times_warning,(XtPointer)warning_box);
							XtAddCallback(warning_box,XmNhelpCallback,
								busy_cursor_off_warning_box_shell,(XtPointer)analysis);
							/* add the destroy callbacks */
							XtAddCallback(warning_box,XmNdestroyCallback,destroy_window_shell,
								(XtPointer)create_Shell_list_item(&(analysis->
								write_event_times_warning_box_s),analysis->user_interface));
							XtAddCallback(warning_box,XmNdestroyCallback,
								busy_cursor_off_warning_box_shell,(XtPointer)analysis);
						}
						else
						{
							XtDestroyWidget(warning_box_shell);
							warning_box_shell=(Widget)NULL;
							display_message(ERROR_MESSAGE,
								"write_event_times_file.  Could not retrieve warning box");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_event_times_file.  Could not create warning box shell");
					}
				}
				else
				{
					warning_box=analysis->write_event_times_warning_box;
				}
				if (warning_box_shell)
				{
					busy_cursor_on(warning_box_shell,analysis->user_interface);
					/* pop up the warning */
					XtManageChild(warning_box);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_event_times_file.  Missing or empty rig");
	}
	LEAVE;
} /* write_event_times_file */

static void identify_analysis_file_read_tim(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file read times button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_tim);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.read_times_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_tim.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_read_tim */

static void identify_analysis_file_save_pot(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file save potentials button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_save_pot);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.save_potentials_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_save_pot.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_save_pot */

static void identify_analysis_file_read_bar(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis file read bard files button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_bar);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.read_bard_data_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_bar.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_read_bar */

static void identify_analysis_file_read_bee(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 July 1997

DESCRIPTION :
Finds the id of the analysis file read beekeeper files button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_bee);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.read_beekeeper_data_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_bee.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_read_bee */

static void identify_analysis_file_read_car(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 August 1997

DESCRIPTION :
Finds the id of the analysis file read CardioMapp files button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_car);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.read_cardiomapp_data_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_car.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_read_car */

static void identify_analysis_file_read_neu(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 August 1997

DESCRIPTION :
Finds the id of the analysis file read Neurosoft files button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_file_read_neu);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->file_menu.read_neurosoft_data_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_file_read_neu.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_file_read_neu */

static void identify_analysis_print_button(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis print button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_print_button);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->print_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_print_button.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_print_button */

static void identify_analysis_print_all_but(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis print all button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_print_all_but);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->print_menu.all_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_print_all_but.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_print_all_but */

static int print_signals(char all,char *file_name,void *analysis_window)
/*******************************************************************************
LAST MODIFIED : 11 January 2000

DESCRIPTION :
Writes the PostScript for drawing either <all> the signals from the
<analysis_window> or selected signals to the specified file.
==============================================================================*/
{
	Display *display;
	float postscript_page_height,postscript_page_width,scale_point_to_pixel_x,
		scale_point_to_pixel_y,signal_gap,signal_height_point,signal_width_point,
		y_position_point;
	int axes_height,axes_left,axes_top,axes_width,first_data,i,j,last_data,
		number_of_devices,return_code,screen,signal_height_pixel,signals_per_page,
		signal_width_pixel;
	struct Analysis_window *analysis;
	struct Device **device;
	struct Printer printer;
	struct Rig *rig;
	struct Region *current_region;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	XWindowAttributes window_attributes;

	ENTER(print_signals);
	/* check that the signals are present */
	if ((analysis=(struct Analysis_window *)analysis_window)&&
		(signal_drawing_information=analysis->signal_drawing_information)&&
		(rig= *(analysis->rig))&&(device=rig->devices)&&(*device)&&
		(buffer=get_Device_signal_buffer(*device))&&
		((number_of_devices=rig->number_of_devices)>0)&&
		((signals_per_page=analysis->signals_per_printer_page)>0))
	{
		display=analysis->user_interface->display;
		first_data=buffer->start;
		last_data=buffer->end;
		current_region=rig->current_region;
		if (open_printer(&printer,analysis->user_interface))
		{
			if (XGetWindowAttributes(display,
				XtWindow((analysis->signals).drawing_area),&window_attributes))
			{
				/* open the postscript file */
				if (open_postscript(file_name,LANDSCAPE,window_attributes.colormap,
					(Pixel *)NULL,0,printer.background_colour_pixel,
					&(printer.background_colour),printer.foreground_colour_pixel,
					signal_drawing_information->font,(float)(printer.page_width_mm),
					(float)(printer.page_height_mm),(float)(printer.page_left_margin_mm),
					(float)(printer.page_right_margin_mm),
					(float)(printer.page_top_margin_mm),
					(float)(printer.page_bottom_margin_mm),analysis->user_interface))
				{
					get_postscript_page_size(&postscript_page_width,
						&postscript_page_height);
					/* determine the page layout */
					signal_height_point=postscript_page_height/(float)signals_per_page;
					signal_width_point=postscript_page_width;
					screen=DefaultScreen(display);
					scale_point_to_pixel_x=(25.4/72)*((float)DisplayWidth(display,screen)/
						(float)DisplayWidthMM(display,screen));
					scale_point_to_pixel_y=
						(25.4/72)*((float)DisplayHeight(display,screen)/
						(float)DisplayHeightMM(display,screen));
					signal_height_pixel=(int)(scale_point_to_pixel_y*signal_height_point);
					signal_width_pixel=(int)(scale_point_to_pixel_x*signal_width_point);
					signal_gap=signal_height_point;
					signal_height_point=(float)signal_height_pixel/scale_point_to_pixel_y;
					signal_width_point=(float)signal_width_pixel/scale_point_to_pixel_x;
					y_position_point=postscript_page_height-signal_height_point;
					/* print the signals with a single colour */
					XSetForeground(display,
						(signal_drawing_information->graphics_context).axis_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).device_name_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).highlighted_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						unhighlighted_colour,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).accepted_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).rejected_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).undecided_colour,
						printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						accepted_colour_text,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						rejected_colour_text,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						undecided_colour_text,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						signal_accepted_colour,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						signal_rejected_colour,printer.foreground_colour_pixel);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).
						signal_undecided_colour,printer.foreground_colour_pixel);
					/* print all the signals with the specified number of signals per
						page */
					i=number_of_devices;
					j=signals_per_page;
					return_code=1;
					while (return_code&&(i>0))
					{
						if (j<=0)
						{
							/* start a new printer page */
							new_postscript_page();
							j=signals_per_page;
							y_position_point=postscript_page_height-signal_height_point;
						}
						if ((all&&(!current_region||(current_region==
							(*device)->description->region)))||(!all&&((*device)->highlight)))
						{
							/* set the page window */
							set_postscript_display_transfor(0,y_position_point,
								signal_width_point,signal_height_point,0,
								(float)signal_height_pixel,(float)signal_width_pixel,
								(float)signal_height_pixel);
							/* print the signal */
							draw_signal(
								(struct FE_node *)NULL,(struct Draw_package *)NULL,*device,
								PRINTER_DETAIL,1,0,&first_data,&last_data,0,signal_height_pixel,
								signal_width_pixel,signal_height_pixel,(Pixmap)NULL,&axes_left,
								&axes_top,&axes_width,&axes_height,
								analysis->signal_drawing_information,analysis->user_interface);
							/* print the markers */
							draw_device_markers(*device,first_data,last_data,0,0,0,0,
								PRINTER_DETAIL,0,axes_left,axes_top,axes_width,axes_height,
								(Window)NULL,(Pixmap)NULL,analysis->signal_drawing_information,
								analysis->user_interface);
							j--;
							y_position_point -= signal_gap;
						}
						/* move to the next signal */
						device++;
						i--;
					}
					close_postscript();
					/* reset signal component colours */
					XSetForeground(display,
						(signal_drawing_information->graphics_context).axis_colour,
						signal_drawing_information->axis_colour);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).device_name_colour,
						signal_drawing_information->device_name_colour);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).highlighted_colour,
						signal_drawing_information->highlighted_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).unhighlighted_colour,
						signal_drawing_information->unhighlighted_colour);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).accepted_colour,
						signal_drawing_information->accepted_colour);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).rejected_colour,
						signal_drawing_information->rejected_colour);
					XSetForeground(display,
						(signal_drawing_information->graphics_context).undecided_colour,
						signal_drawing_information->undecided_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).accepted_colour_text,
						signal_drawing_information->accepted_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).rejected_colour_text,
						signal_drawing_information->rejected_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).undecided_colour_text,
						signal_drawing_information->undecided_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).signal_accepted_colour,
						signal_drawing_information->signal_accepted_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).signal_rejected_colour,
						signal_drawing_information->signal_rejected_colour);
					XSetForeground(display,(signal_drawing_information->
						graphics_context).signal_undecided_colour,
						signal_drawing_information->signal_undecided_colour);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"print_signals.  Could not open postscript file");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"print_signals.  Could not get colour map");
				return_code=0;
			}
			close_printer(&printer);
		}
		else
		{
			display_message(ERROR_MESSAGE,"print_signals.  Could not open printer");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"print_signals.  Invalid signal information");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* print_signals */

static int print_all_signals(char *file_name,void *analysis_window)
/*******************************************************************************
LAST MODIFIED : 8 May 1993

DESCRIPTION :
Writes the PostScript for drawing all the signals from the <analysis_window> to
the specified file.
==============================================================================*/
{
	int return_code;

	ENTER(print_all_signals);
	return_code=print_signals(1,file_name,analysis_window);
	LEAVE;

	return (return_code);
} /* print_all_signals */

static int print_selected_signals(char *file_name,void *analysis_window)
/*******************************************************************************
LAST MODIFIED : 8 May 1993

DESCRIPTION :
Writes the PostScript for drawing selected signals to the specified file.
==============================================================================*/
{
	int return_code;
;
	ENTER(print_selected_signals);
	return_code=print_signals(0,file_name,analysis_window);
	LEAVE;

	return (return_code);
} /* print_selected_signals */

static void identify_analysis_print_page_bu(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis print page button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_print_page_bu);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->print_menu.page_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_print_page_bu.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_print_page_bu */

static void identify_analysis_print_selecte(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis print selected button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_print_selecte);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->print_menu.selected_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_print_selecte.  Missing analysis_window");
	}
	LEAVE;
} /* identify_analysis_print_selecte */

static void identify_analysis_order_choice(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis order choice toggle.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_order_choice);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->order_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_order_choice.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_order_choice */

static void identify_analysis_order_event_b(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis order event button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_order_event_b);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->order.event_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_order_event_b.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_order_event_b */

static void identify_analysis_order_channel(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis order channel button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_order_channel);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->order.channel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_order_channel.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_order_channel */

static void identify_analysis_layout_choice(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis layout choice toggle.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_layout_choice);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->layout_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_layout_choice.  client_data missing");
	}
	LEAVE;
} /* identify_analysis_layout_choice */

static void identify_analysis_layout_separa(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis layout separate button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_layout_separa);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->layout.separate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_layout_separa.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_layout_separa */

static void set_analysis_layout_separate(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Sets the way the signals are displayed to separate.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(set_analysis_layout_separate);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (SEPARATE_LAYOUT!=analysis->signals.layout)
		{
			analysis->signals.layout=SEPARATE_LAYOUT;
			/* redraw the signals */
			update_signals_drawing_area(analysis);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_layout_separate.  analysis_window missing");
	}
	LEAVE;
} /* set_analysis_layout_separate */

static void identify_analysis_layout_overla(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis layout overlap button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_layout_overla);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->layout.overlap_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_layout_overla.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_layout_overla */

static void set_analysis_layout_overlap(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Sets the way the signals are displayed to overlap.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(set_analysis_layout_overlap);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget_id);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (OVERLAP_LAYOUT!=analysis->signals.layout)
		{
			analysis->signals.layout=OVERLAP_LAYOUT;
			/* redraw the signals */
			update_signals_drawing_area(analysis);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_analysis_layout_overlap.  analysis_window missing");
	}
	LEAVE;
} /* set_analysis_layout_overlap */

static void identify_analysis_region_choice(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis region choice button.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_region_choice);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->region_choice= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_region_choice.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_region_choice */

static void identify_analysis_region_pull_d(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis region pull down menu.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_region_pull_d);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->region_pull_down_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_region_pull_d.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_region_pull_d */

static void identify_analysis_region_place(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
In order for the region choice pull-down to be in the right place, there needs
to be a button in the pull-down when the analysis window is first realized.  A
place holder is used.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_analysis_region_place);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (ALLOCATE(analysis->regions,Widget,1))
		{
			analysis->number_of_regions=1;
			*(analysis->regions)= *widget_id;
			analysis->current_region= *widget_id;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"identify_analysis_region_place.  Insufficient memory for region list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_analysis_region_place.  analysis_window missing");
	}
	LEAVE;
} /* identify_analysis_region_place */

static void identify_signals_drawing_area(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the analysis drawing area.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_signals_drawing_area);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)client_data)
	{
		analysis->signals.drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_signals_drawing_area.  client_data missing");
	}
	LEAVE;
} /* identify_signals_drawing_area */

static void expose_signals_drawing_area(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
The callback for redrawing part of an analysis drawing area.
==============================================================================*/
{
	struct Analysis_window *analysis;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;
	struct Signals_area *signals;

	ENTER(expose_signals_drawing_area);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						event= &(callback->event->xexpose);
						signals= &(analysis->signals);
						if (signals->drawing_area)
						{
							if (!(signals->drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(analysis->user_interface->display,
									XtWindow(signals->drawing_area),&attributes);
								/* create a pixel map */
								if (signals->drawing=create_Drawing_2d(signals->drawing_area,
									attributes.width,attributes.height,NO_DRAWING_IMAGE,
									analysis->user_interface))
								{
									/* draw the signals */
#if defined (UNEMAP_USE_NODES)
									/* use rig_node_group and draw_package */
									draw_all_signals((struct Rig *)NULL,*(analysis->datum),
										*(analysis->potential_time),signals,signals->layout,
										analysis->signal_aspect_ratio,
										analysis->signal_overlap_spacing,
										analysis->signal_drawing_information,
										analysis->user_interface,*(analysis->rig_node_group),
										*(analysis->draw_package));
#else /* defined (UNEMAP_USE_NODES) */
									/* use rig */
									draw_all_signals(*(analysis->rig),*(analysis->datum),
										*(analysis->potential_time),signals,signals->layout,
										analysis->signal_aspect_ratio,
										analysis->signal_overlap_spacing,
										analysis->signal_drawing_information,
										analysis->user_interface,(struct GROUP(FE_node) *)NULL,
										(struct Draw_package *)NULL);
#endif /* defined (UNEMAP_USE_NODES) */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"expose_signals_drawing_area.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (signals->drawing)
							{
								XCopyArea(analysis->user_interface->display,
									signals->drawing->pixel_map,XtWindow(signals->drawing_area),
									(analysis->signal_drawing_information->graphics_context).
									copy,event->x,event->y,event->width,event->height,event->x,
									event->y);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_signals_drawing_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_signals_drawing_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_signals_drawing_area.  event missing");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_signals_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_signals_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_signals_drawing_area.  analysis window missing");
	}
	LEAVE;
} /* expose_signals_drawing_area */

static void resize_signals_drawing_area(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
The callback for resizing an analysis drawing area.
==============================================================================*/
{
	struct Analysis_window *analysis;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;
	int width,height;
	struct Signals_area *signals;

	ENTER(resize_signals_drawing_area);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					signals= &(analysis->signals);
					if (signals->drawing_area)
					{
						/* find the size of the old rectangle */
						if (signals->drawing)
						{
							width=signals->drawing->width;
							height=signals->drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(signals->drawing));
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(analysis->user_interface->display,
							callback->window,&attributes);
						/* create a new pixmap */
						if (signals->drawing=create_Drawing_2d(signals->drawing_area,
							attributes.width,attributes.height,NO_DRAWING_IMAGE,
							analysis->user_interface))
						{
							/* redraw the signals */
#if defined (UNEMAP_USE_NODES)
							/* use rig_node_group and draw_package */
							draw_all_signals((struct Rig *)NULL,*(analysis->datum),
								*(analysis->potential_time),signals,signals->layout,
								analysis->signal_aspect_ratio,analysis->signal_overlap_spacing,
								analysis->signal_drawing_information,analysis->user_interface,
								*(analysis->rig_node_group),*(analysis->draw_package));
#else /* defined (UNEMAP_USE_NODES) */
							/* use rig */
							draw_all_signals(*(analysis->rig),*(analysis->datum),
								*(analysis->potential_time),signals,signals->layout,
								analysis->signal_aspect_ratio,analysis->signal_overlap_spacing,
								analysis->signal_drawing_information,analysis->user_interface,
								(struct GROUP(FE_node) *)NULL,(struct Draw_package *)NULL);
#endif /* defined (UNEMAP_USE_NODES) */
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
							XCopyArea(analysis->user_interface->display,
								signals->drawing->pixel_map,XtWindow(signals->drawing_area),
								(analysis->signal_drawing_information->graphics_context).copy,
								0,0,width,height,0,0);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_signals_drawing_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_signals_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_signals_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_signals_drawing_area.  analysis window missing");
	}
	LEAVE;
} /* resize_signals_drawing_area */

static void identify_interval_drawing_area(Widget *widget_id,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
Finds the id of the interval drawing area.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(identify_interval_drawing_area);
	USE_PARAMETER(call_data);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		analysis->interval.drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_interval_drawing_area.  analysis_window missing");
	}
	LEAVE;
} /* identify_interval_drawing_area */

static void expose_interval_drawing_area(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 January 2000

DESCRIPTION :
The callback for redrawing part of an analysis interval drawing area.
==============================================================================*/
{
	Display *display;
	float x_scale;
	int first_data,last_data;
	struct Analysis_window *analysis;
	struct Device *highlight_device;
	struct Interval_area *interval;
	struct Signal_buffer *buffer;
	XExposeEvent *event;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(expose_interval_drawing_area);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						event= &(callback->event->xexpose);
						interval= &(analysis->interval);
						if (interval->drawing_area)
						{
							display=analysis->user_interface->display;
							if (!(interval->drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(display,XtWindow(interval->drawing_area),
									&attributes);
								/* create a pixel map */
								if (interval->drawing=create_Drawing_2d(interval->drawing_area,
									attributes.width,attributes.height,NO_DRAWING_IMAGE,
									analysis->user_interface))
								{
									/* clear the interval drawing area */
									XFillRectangle(display,interval->drawing->pixel_map,
										(analysis->signal_drawing_information->graphics_context).
										background_drawing_colour,0,0,interval->drawing->width,
										interval->drawing->height);
									/* draw all of the active signal */
									if ((*(analysis->highlight))&&
										(highlight_device= **(analysis->highlight))&&
										(buffer=get_Device_signal_buffer(highlight_device)))
									{
										first_data=0;
										last_data=buffer->number_of_samples-1;
										draw_signal(
											(struct FE_node *)NULL,(struct Draw_package *)NULL,
											highlight_device,
											INTERVAL_AREA_DETAIL,1,0,&first_data,&last_data,0,0,
											attributes.width,attributes.height,
											interval->drawing->pixel_map,&(interval->axes_left),
											&(interval->axes_top),&(interval->axes_width),
											&(interval->axes_height),
											analysis->signal_drawing_information,
											analysis->user_interface);
										draw_device_markers(highlight_device,first_data,last_data,
											*(analysis->datum),1,*(analysis->potential_time),1,
											INTERVAL_AREA_DETAIL,*(analysis->event_number),
											interval->axes_left,interval->axes_top,
											interval->axes_width,interval->axes_height,(Window)NULL,
											interval->drawing->pixel_map,
											analysis->signal_drawing_information,
											analysis->user_interface);
										x_scale=SCALE_FACTOR(buffer->number_of_samples-1,
											interval->axes_width-1);
										interval->left_box=
											SCALE_X(buffer->start,0,interval->axes_left,x_scale);
										interval->right_box=
											SCALE_X(buffer->end,0,interval->axes_left,x_scale);
										/* draw the interval rectangle */
										draw_interval_box(interval->left_box,interval->axes_top,
											interval->right_box-interval->left_box+1,
											interval->axes_height,interval,
											analysis->signal_drawing_information,
											analysis->user_interface);
									}
									else
									{
										interval->axes_height=0;
										interval->axes_left=0;
										interval->axes_top=0;
										interval->axes_width=0;
										interval->left_box=0;
										interval->right_box=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"expose_interval_drawing_area.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (interval->drawing)
							{
								XCopyArea(display,interval->drawing->pixel_map,
									XtWindow(interval->drawing_area),(analysis->
									signal_drawing_information->graphics_context).copy,event->x,
									event->y,event->width,event->height,event->x,event->y);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_interval_drawing_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_interval_drawing_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_interval_drawing_area.  event missing");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_interval_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_interval_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_interval_drawing_area.  analysis window missing");
	}
	LEAVE;
} /* expose_interval_drawing_area */

static void resize_interval_drawing_area(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 11 January 2000

DESCRIPTION :
The callback for resizing an analysis interval drawing area.
==============================================================================*/
{
	Display *display;
	float x_scale;
	int first_data,height,last_data,width;
	struct Analysis_window *analysis;
	struct Device *highlight_device;
	struct Interval_area *interval;
	struct Signal_buffer *buffer;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(resize_interval_drawing_area);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					interval= &(analysis->interval);
					if (interval->drawing_area)
					{
						display=analysis->user_interface->display;
						/* find the size of the old rectangle */
						if (interval->drawing)
						{
							width=interval->drawing->width;
							height=interval->drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(interval->drawing));
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(display,callback->window,&attributes);
						/* create a new pixmap */
						if (interval->drawing=create_Drawing_2d(interval->drawing_area,
							attributes.width,attributes.height,NO_DRAWING_IMAGE,
							analysis->user_interface))
						{
							/* clear the interval drawing area */
							XFillRectangle(display,interval->drawing->pixel_map,
								(analysis->signal_drawing_information->graphics_context).
								background_drawing_colour,0,0,interval->drawing->width,
								interval->drawing->height);
							/* draw all of the active signal */
							if ((*(analysis->highlight))&&
								(highlight_device= **(analysis->highlight))&&
								(buffer=get_Device_signal_buffer(highlight_device)))
							{
								first_data=0;
								last_data=buffer->number_of_samples-1;
								draw_signal((struct FE_node *)NULL,(struct Draw_package *)NULL,
									highlight_device,INTERVAL_AREA_DETAIL,1,0,&first_data,
									&last_data,0,0,attributes.width,attributes.height,
									interval->drawing->pixel_map,&(interval->axes_left),
									&(interval->axes_top),&(interval->axes_width),
									&(interval->axes_height),analysis->signal_drawing_information,
									analysis->user_interface);
								draw_device_markers(highlight_device,first_data,last_data,
									*(analysis->datum),1,*(analysis->potential_time),1,
									INTERVAL_AREA_DETAIL,*(analysis->event_number),
									interval->axes_left,interval->axes_top,interval->axes_width,
									interval->axes_height,(Window)NULL,
									interval->drawing->pixel_map,
									analysis->signal_drawing_information,
									analysis->user_interface);
								x_scale=SCALE_FACTOR(buffer->number_of_samples-1,
									interval->axes_width-1);
								interval->left_box=
									SCALE_X(buffer->start,0,interval->axes_left,x_scale);
								interval->right_box=
									SCALE_X(buffer->end,0,interval->axes_left,x_scale);
								/* draw the interval rectangle */
								draw_interval_box(interval->left_box,interval->axes_top,
									interval->right_box-interval->left_box+1,
									interval->axes_height,interval,
									analysis->signal_drawing_information,
									analysis->user_interface);
							}
							else
							{
								interval->axes_height=0;
								interval->axes_left=0;
								interval->axes_top=0;
								interval->axes_width=0;
								interval->left_box=0;
								interval->right_box=0;
							}
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
							XCopyArea(display,interval->drawing->pixel_map,
								XtWindow(interval->drawing_area),
								(analysis->signal_drawing_information->graphics_context).copy,
								0,0,width,height,0,0);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_interval_drawing_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_interval_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_interval_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_interval_drawing_area.  analysis window missing");
	}
	LEAVE;
} /* resize_interval_drawing_area */

static void destroy_Analysis_window(Widget widget,
	XtPointer analysis_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
This function expects <analysis_window> to be a pointer to an analysis window.
If the <address> field of the analysis window is not NULL, <*address> is set to
NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
analysis window and frees the memory associated with the analysis window.
==============================================================================*/
{
	struct Analysis_window *analysis;

	ENTER(destroy_Analysis_window);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	if (analysis=(struct Analysis_window *)analysis_window)
	{
		/* set the pointer to the analysis window to NULL */
		if (analysis->address)
		{
			*(analysis->address)=(struct Analysis_window *)NULL;
		}
		/* unghost the activation button */
		if (analysis->activation)
		{
			XtSetSensitive(analysis->activation,True);
		}
		/* free the signals drawing memory */
		destroy_Drawing_2d(&(analysis->signals.drawing));
		/* free the analysis window memory */
		DEALLOCATE(analysis);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Analysis_window.  analysis window missing");
	}
	LEAVE;
} /* destroy_Analysis_window */

/*
Global functions
----------------
*/
int analysis_window_update_interval_area_time(struct Analysis_window *analysis,
	int potential_time, int previous_potential_time,
	enum Analysis_window_update_flags *update_flags)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Shifts the bar in the interval area part of the analysis window to represent
the new potential time.
==============================================================================*/
{
	char number_string[20];
	Display *display;
	int ascent, axes_bottom, axes_left, axes_right, axes_top, descent,
		 direction, length, return_code, scaled_potential_time,
		scaled_previous_potential_time,*times, x_string, y_string;
	float frequency, x_scale;
	GC potential_time_colour,potential_time_colour_text;
	Pixmap pixel_map;
	struct Device *highlight_device;
	struct Interval_area *interval;
	struct Signal_buffer *buffer;
	Window working_window;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(analysis_window_update_interval_area_time);
	if ((*analysis->highlight)&&(highlight_device= **(analysis->highlight))&&
		(buffer=get_Device_signal_buffer(highlight_device))&&(times=buffer->times))
	{
		display=analysis->user_interface->display;
		frequency=buffer->frequency;
		font=analysis->signal_drawing_information->font;
		if (potential_time!=previous_potential_time)
		{
			interval= &(analysis->interval);
			axes_left=interval->axes_left;
			axes_right=axes_left+(interval->axes_width)-1;
			axes_top=interval->axes_top;
			axes_bottom=axes_top+(interval->axes_height)-1;
			x_scale=
				SCALE_FACTOR((buffer->number_of_samples)-1,axes_right-axes_left);
			pixel_map=interval->drawing->pixel_map;
			potential_time_colour=(analysis->signal_drawing_information->
				graphics_context).potential_time_colour;
			potential_time_colour_text=(analysis->signal_drawing_information->
				graphics_context).potential_time_colour_text;
			working_window=XtWindow(interval->drawing_area);
			scaled_previous_potential_time=SCALE_X(previous_potential_time,0,
				axes_left,x_scale);
			scaled_potential_time=SCALE_X(potential_time,0,axes_left,x_scale);
			if (*update_flags & ANALYSIS_INTERVAL_NO_POTENTIAL_ERASE)
			{
				/* Clear just that flag */
				*update_flags &= ~ANALYSIS_INTERVAL_NO_POTENTIAL_ERASE;

				/* draw the new marker */
				XDrawLine(display,pixel_map,
					potential_time_colour,
					scaled_potential_time,axes_top,scaled_potential_time,
					axes_bottom);
				XDrawLine(display,working_window,
					potential_time_colour,
					scaled_potential_time,axes_top,scaled_potential_time,
					axes_bottom);
			}
			else
			{
				/* clear the potential time */
				sprintf(number_string,"%d",
					(int)((float)(times[previous_potential_time])*1000./
						frequency));
				length=strlen(number_string);
				XTextExtents(font,number_string,length,&direction,&ascent,
					&descent,&bounds);
				x_string=scaled_previous_potential_time+
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
				XDrawString(display,pixel_map,potential_time_colour_text,
					x_string,y_string,number_string,length);
				XDrawString(display,working_window,potential_time_colour,
					x_string,y_string,number_string,length);
				if (scaled_potential_time!=scaled_previous_potential_time)
				{
					/* clear the old marker */
					XDrawLine(display,pixel_map,
						potential_time_colour,
						scaled_previous_potential_time,axes_top,
						scaled_previous_potential_time,axes_bottom);
					XDrawLine(display,working_window,
						potential_time_colour,
						scaled_previous_potential_time,axes_top,
						scaled_previous_potential_time,axes_bottom);
					/* draw the new marker */
					XDrawLine(display,pixel_map,
						potential_time_colour,
						scaled_potential_time,axes_top,scaled_potential_time,
						axes_bottom);
					XDrawLine(display,working_window,
						potential_time_colour,
						scaled_potential_time,axes_top,scaled_potential_time,
						axes_bottom);
				}
			}
			/* write the potential time */
			sprintf(number_string,"%d",(int)((float)(times[potential_time])*
				1000./frequency));
			length=strlen(number_string);
			XTextExtents(font,number_string,length,&direction,
				&ascent,&descent,&bounds);
			x_string=scaled_potential_time+
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
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"analysis_window_update_interval_area_time.  Missing analysis_window_structure");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* analysis_window_update_interval_area_time */

int analysis_window_update_signal_area_time(struct Analysis_window *analysis,
	int potential_time, int previous_potential_time)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Shifts the bar in the signal area part of the analysis window to represent
the new potential time.
==============================================================================*/
{
	int	device_number,drawing_height,drawing_width,end_analysis_interval,i,j,
		number_of_columns,number_of_rows,return_code,rows_divisor,
		scaled_potential_time,scaled_previous_potential_time,signal_index,
		signals_axes_height,signals_axes_left,signals_axes_top,signals_axes_width,
		start_analysis_interval,xpos,ypos;
	Pixmap signals_pixel_map;
	struct Device **device, *highlight_device;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Signals_area *signals;
	struct User_interface *user_interface;
	Window signals_window;

	ENTER(analysis_time_update_callback);
	return_code=0;
	if ((*analysis->highlight)&&(highlight_device= **(analysis->highlight))&&
		(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information)&&
		(buffer=get_Device_signal_buffer(highlight_device)))
	{
		start_analysis_interval=buffer->start;
		end_analysis_interval=buffer->end;
		if ((*analysis->rig)&&(signals= &(analysis->signals)))
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
			device_number=(*analysis->rig)->number_of_devices;
			device=(*analysis->rig)->devices;
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
			scaled_previous_potential_time=SCALE_X(previous_potential_time,
				start_analysis_interval, xpos,
				SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
				signals_axes_width-1));
			scaled_potential_time=SCALE_X(potential_time,
				start_analysis_interval, xpos,
				SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
				signals_axes_width-1));
			while (device_number>0)
			{
				if (scaled_previous_potential_time!=scaled_potential_time)
				{
					if ((*device)->signal)
					{
						signal_index=(*device)->signal->index;
					}
					else
					{
						signal_index= -1;
					}
					draw_potential_time_marker(previous_potential_time,buffer,
						(*device)->channel,signal_index,SIGNAL_AREA_DETAIL,
						start_analysis_interval,end_analysis_interval,xpos,ypos,
						signals_axes_width,signals_axes_height,signals_window,
						signals_pixel_map,signal_drawing_information,user_interface);
					draw_potential_time_marker(potential_time,buffer,
						(*device)->channel,signal_index,SIGNAL_AREA_DETAIL,
						start_analysis_interval,end_analysis_interval,xpos,ypos,
						signals_axes_width,signals_axes_height,signals_window,
						signals_pixel_map,signal_drawing_information,user_interface);
				}
				i++;
				if (i>=number_of_rows)
				{
					i=0;
					ypos=signals_axes_top;
					j++;
					xpos=signals_axes_left+
						(j*drawing_width)/number_of_columns;
					scaled_previous_potential_time=SCALE_X(previous_potential_time,
						start_analysis_interval, xpos,
						SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
						signals_axes_width-1));
					scaled_potential_time=SCALE_X(potential_time,
						start_analysis_interval, xpos,
						SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
						signals_axes_width-1));
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
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"analysis_time_update_signal_area_time.  Missing analysis_window_structure");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* analysis_time_update_signal_area_time */

int analysis_window_update_signal_area_datum(struct Analysis_window *analysis,
	int datum_marker, int previous_datum_marker)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Shifts the bar in the signal area part of the analysis window to represent
the new datum time.
==============================================================================*/
{
	int	device_number, drawing_height, drawing_width, end_analysis_interval,
		i, j, number_of_columns, number_of_rows, return_code,
		rows_divisor, scaled_datum_marker, scaled_previous_datum_marker,
		signals_axes_height,signals_axes_left,signals_axes_top,signals_axes_width,
		start_analysis_interval,xpos,ypos;
	Pixmap signals_pixel_map;
	struct Device **device, *highlight_device;
	struct Signal_buffer *buffer;
	struct Signal_drawing_information *signal_drawing_information;
	struct Signals_area *signals;
	struct User_interface *user_interface;
	Window signals_window;

	ENTER(analysis_window_update_signal_area_datum);

	if ((*analysis->highlight)&&(highlight_device= **(analysis->highlight))&&
		(user_interface=analysis->user_interface)&&
		(signal_drawing_information=analysis->signal_drawing_information)&&
		(buffer=get_Device_signal_buffer(highlight_device)))
	{
		start_analysis_interval=buffer->start;
		end_analysis_interval=buffer->end;
		if ((*analysis->rig)&&(signals= &(analysis->signals)))
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
			device_number=(*analysis->rig)->number_of_devices;
			device=(*analysis->rig)->devices;
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
			scaled_previous_datum_marker=SCALE_X(previous_datum_marker,
				start_analysis_interval, xpos,
				SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
				signals_axes_width-1));
			scaled_datum_marker=SCALE_X(datum_marker,
				start_analysis_interval, xpos,
				SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
				signals_axes_width-1));
			while (device_number>0)
			{
				if (scaled_previous_datum_marker != scaled_datum_marker)
				{
					draw_datum_marker(previous_datum_marker,
						SIGNAL_AREA_DETAIL,
						start_analysis_interval,
						end_analysis_interval,xpos,ypos,
						signals_axes_width,
						signals_axes_height,signals_window,
						signals_pixel_map,
						signal_drawing_information,
						user_interface);
					draw_datum_marker(datum_marker,
						SIGNAL_AREA_DETAIL,
						start_analysis_interval,
						end_analysis_interval,xpos,ypos,
						signals_axes_width,
						signals_axes_height,signals_window,
						signals_pixel_map,
						signal_drawing_information,
						user_interface);
				}
				i++;
				if (i>=number_of_rows)
				{
					i=0;
					ypos=signals_axes_top;
					j++;
					xpos=signals_axes_left+
						(j*drawing_width)/number_of_columns;
					scaled_previous_datum_marker=SCALE_X(previous_datum_marker,
						start_analysis_interval, xpos,
						SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
						signals_axes_width-1));
					scaled_datum_marker=SCALE_X(datum_marker,
						start_analysis_interval, xpos,
						SCALE_FACTOR(end_analysis_interval-start_analysis_interval,
						signals_axes_width-1));
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
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_window_update_signal_area_datum.  Missing analysis_window_structure");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* analysis_window_update_signal_area_datum */

int draw_interval_box(int left,int top,int width,int height,
	struct Interval_area *interval,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 December 1996

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(draw_interval_box);
	if (interval&&(interval->drawing_area)&&(interval->drawing)&&
		(interval->drawing->pixel_map)&&signal_drawing_information&&user_interface)
	{
		XDrawRectangle(user_interface->display,XtWindow(interval->drawing_area),
			(signal_drawing_information->graphics_context).interval_box_colour,
			left,top,width,height);
		XDrawRectangle(user_interface->display,interval->drawing->pixel_map,
			(signal_drawing_information->graphics_context).interval_box_colour,
			left,top,width,height);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_interval_box.  Invalid interval");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_interval_box */

struct Analysis_window *create_Analysis_window(
	struct Analysis_window **address,Widget activation,Widget parent,
	struct Rig **rig,struct GROUP(FE_node) **rig_node_group,
	struct Draw_package **draw_package,
	struct Device ***highlight,int *datum,int *event_number,
	int *number_of_events,int *potential_time,
	enum Event_detection_algorithm *detection,int *threshold,
	int *minimum_separation,float *level,int *average_width,
	Pixel identifying_colour,enum Signal_order order,enum Signal_layout layout,
	int *start_search_interval,int **search_interval_divisions,
	int *end_search_interval,int screen_height,
		/*???DB.  height of interval drawing area.  Better ? */
	char *postscript_file_extension,char *events_file_extension,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
This function allocates the memory for an analysis window and sets the fields
to the specified values (<address>, <activation>, <rig_address>).  It then
retrieves an analysis window widget with the specified <parent> and assigns the
widget ids to the appropriate fields of the structure.  If successful it returns
a pointer to the created analysis window and, if <address> is not NULL, makes
<*address> point to the created analysis window.  If unsuccessful, NULL is
returned.
==============================================================================*/
{
	Dimension left_margin,right_margin;
	int signal_aspect_ratio_percent,widget_spacing;
	MrmType analysis_window_class;
	Pixmap no_cascade_pixmap;
	static MrmRegisterArg callback_list[]={
		{"identify_analysis_reset_button",
			(XtPointer)identify_analysis_reset_button},
		{"identify_analysis_baseline_butt",
			(XtPointer)identify_analysis_baseline_butt},
		{"identify_analysis_range_button",
			(XtPointer)identify_analysis_range_button},
		{"identify_analysis_previous_butt",
			(XtPointer)identify_analysis_previous_butt},
#if defined (OLD_CODE)
		{"identify_analysis_previous_acce",
			(XtPointer)identify_analysis_previous_acce},
#endif
		{"identify_analysis_next_button",
			(XtPointer)identify_analysis_next_button},
#if defined (OLD_CODE)
		{"identify_analysis_next_accelera",
			(XtPointer)identify_analysis_next_accelera},
#endif
		{"identify_analysis_accept_button",
			(XtPointer)identify_analysis_accept_button},
#if defined (OLD_CODE)
		{"identify_analysis_accept_accele",
			(XtPointer)identify_analysis_accept_accele},
#endif
		{"identify_analysis_reject_button",
			(XtPointer)identify_analysis_reject_button},
#if defined (OLD_CODE)
		{"identify_analysis_reject_accele",
			(XtPointer)identify_analysis_reject_accele},
#endif
		{"identify_analysis_close_button",
			(XtPointer)identify_analysis_close_button},
		{"identify_analysis_menu",(XtPointer)identify_analysis_menu},
		{"identify_analysis_map_button",(XtPointer)identify_analysis_map_button},
		{"identify_analysis_map_potential",
			(XtPointer)identify_analysis_map_potential},
		{"identify_analysis_map_integral",
			(XtPointer)identify_analysis_map_integral},
		{"identify_analysis_map_gradient",
			(XtPointer)identify_analysis_map_gradient},
		{"id_analysis_map_single_activati",
			(XtPointer)id_analysis_map_single_activati},
		{"id_analysis_map_multiple_activa",
			(XtPointer)id_analysis_map_multiple_activa},
		{"identify_analysis_file_button",(XtPointer)identify_analysis_file_button},
		{"identify_analysis_file_read_sig",
			(XtPointer)identify_analysis_file_read_sig},
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"identify_analysis_file_overlay",
			(XtPointer)identify_analysis_file_overlay},
		{"identify_analysis_file_save_int",
			(XtPointer)identify_analysis_file_save_int},
		{"id_analysis_file_save_int_as",(XtPointer)id_analysis_file_save_int_as},
		{"open_file_and_write",(XtPointer)open_file_and_write},
		{"identify_analysis_file_save_tim",
			(XtPointer)identify_analysis_file_save_tim},
		{"identify_analysis_file_read_tim",
			(XtPointer)identify_analysis_file_read_tim},
		{"identify_analysis_file_save_pot",
			(XtPointer)identify_analysis_file_save_pot},
		{"identify_analysis_file_read_bar",
			(XtPointer)identify_analysis_file_read_bar},
		{"identify_analysis_file_read_bee",
			(XtPointer)identify_analysis_file_read_bee},
		{"identify_analysis_file_read_car",
			(XtPointer)identify_analysis_file_read_car},
		{"identify_analysis_file_read_neu",
			(XtPointer)identify_analysis_file_read_neu},
		{"write_event_times_file",(XtPointer)write_event_times_file},
		{"identify_analysis_print_button",
			(XtPointer)identify_analysis_print_button},
		{"identify_analysis_print_all_but",
			(XtPointer)identify_analysis_print_all_but},
		{"identify_analysis_print_page_bu",
			(XtPointer)identify_analysis_print_page_bu},
		{"identify_analysis_print_selecte",
			(XtPointer)identify_analysis_print_selecte},
		{"identify_analysis_order_choice",
			(XtPointer)identify_analysis_order_choice},
		{"identify_analysis_order_event_b",
			(XtPointer)identify_analysis_order_event_b},
		{"identify_analysis_order_channel",
			(XtPointer)identify_analysis_order_channel},
		{"identify_analysis_layout_choice",
			(XtPointer)identify_analysis_layout_choice},
		{"identify_analysis_layout_separa",
			(XtPointer)identify_analysis_layout_separa},
		{"set_analysis_layout_separate",(XtPointer)set_analysis_layout_separate},
		{"identify_analysis_layout_overla",
			(XtPointer)identify_analysis_layout_overla},
		{"set_analysis_layout_overlap",(XtPointer)set_analysis_layout_overlap},
		{"identify_analysis_region_choice",
			(XtPointer)identify_analysis_region_choice},
		{"identify_analysis_region_pull_d",
			(XtPointer)identify_analysis_region_pull_d},
		{"identify_analysis_region_place",
			(XtPointer)identify_analysis_region_place},
		{"identify_interval_drawing_area",
			(XtPointer)identify_interval_drawing_area},
		{"expose_interval_drawing_area",(XtPointer)expose_interval_drawing_area},
		{"resize_interval_drawing_area",(XtPointer)resize_interval_drawing_area},
		{"identify_signals_drawing_area",
			(XtPointer)identify_signals_drawing_area},
		{"expose_signals_drawing_area",(XtPointer)expose_signals_drawing_area},
		{"resize_signals_drawing_area",(XtPointer)resize_signals_drawing_area},
		{"destroy_Analysis_window",(XtPointer)destroy_Analysis_window}};
	static MrmRegisterArg identifier_list[]=
	{
		{"analysis_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL},
		{"print_all_signals_data",(XtPointer)NULL},
		{"print_selected_signals_data",(XtPointer)NULL},
	};
#define XmNsignalAspectRatioPercent "signalAspectRatioPercent"
#define XmCSignalAspectRatioPercent "SignalAspectRatioPercent"
#define XmNsignalOverlapSpacing "signalOverlapSpacing"
#define XmCSignalOverlapSpacing "SignalOverlapSpacing"
#define XmNsignalsPerPrinterPage "signalsPerPrinterPage"
#define XmCSignalsPerPrinterPage "SignalsPerPrinterPage"
	static XtResource
		resources_1[]=
		{
			{
				XmNsignalAspectRatioPercent,
				XmCSignalAspectRatioPercent,
				XmRInt,
				sizeof(int),
				0,
				XmRString,
				"20"
			}
		},
		resources_2[]=
		{
			{
				XmNsignalOverlapSpacing,
				XmCSignalOverlapSpacing,
				XmRInt,
				sizeof(int),
				XtOffsetOf(Analysis_window_settings,signal_overlap_spacing),
				XmRString,
				"5"
			},
			{
				XmNsignalsPerPrinterPage,
				XmCSignalsPerPrinterPage,
				XmRInt,
				sizeof(int),
				XtOffsetOf(Analysis_window_settings,signals_per_printer_page),
				XmRString,
				"12"
			}
		};
	struct Analysis_window *analysis;
	Widget child_widget;
	XmString dialog_title;
	ENTER(create_Analysis_window);
	if (signal_drawing_information&&user_interface)
	{
		no_cascade_pixmap=user_interface->no_cascade_pixmap;
		if (MrmOpenHierarchy_base64_string(analysis_window_uidh,
			&analysis_window_hierarchy,&analysis_window_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(analysis,struct Analysis_window,1))
			{
				/* assign fields */
				analysis->user_interface=user_interface;
				analysis->signal_drawing_information=signal_drawing_information;
				analysis->address=address;
				analysis->activation=activation;
				analysis->rig=rig;
				analysis->rig_node_group=rig_node_group;
				analysis->draw_package=draw_package;
				analysis->highlight=highlight;
				analysis->datum=datum;
				analysis->event_number=event_number;
				analysis->number_of_events=number_of_events;
				analysis->potential_time=potential_time;
				analysis->detection=detection;
				analysis->threshold=threshold;
				analysis->minimum_separation=minimum_separation;
				analysis->level=level;
				analysis->average_width=average_width;
				analysis->start_search_interval=start_search_interval;
				analysis->search_interval_divisions=search_interval_divisions;
				analysis->end_search_interval=end_search_interval;
				/* create the structure for writing event time files */
				analysis->write_times_file_open_data=create_File_open_data(
					events_file_extension,REGULAR,write_times_undecided_accepted,analysis,
					0,user_interface);
				XtVaGetApplicationResources(user_interface->application_shell,
					&signal_aspect_ratio_percent,resources_1,XtNumber(resources_1),NULL);
				analysis->signal_aspect_ratio=(float)(signal_aspect_ratio_percent)/100;
				XtVaGetApplicationResources(user_interface->application_shell,analysis,
					resources_2,XtNumber(resources_2),NULL);
				analysis->window=(Widget)NULL;
				analysis->menu=(Widget)NULL;
				analysis->map_button=(Widget)NULL;
				analysis->map_menu.potential_button=(Widget)NULL;
				analysis->map_menu.integral_button=(Widget)NULL;
				analysis->map_menu.gradient_button=(Widget)NULL;
				analysis->map_menu.single_activation_button=(Widget)NULL;
				analysis->map_menu.multiple_activation_button=(Widget)NULL;
				analysis->display_map_warning_accept_butt=(Widget)NULL;
				analysis->display_map_warning_reject_butt=(Widget)NULL;
				analysis->display_map_warning_box=(Widget)NULL;
				analysis->display_map_warning_box_shell=(Widget)NULL;
				analysis->file_button=(Widget)NULL;
				analysis->file_menu.read_signals_button=(Widget)NULL;
				analysis->file_menu.overlay_signals_button=(Widget)NULL;
				analysis->file_menu.save_interval_button=(Widget)NULL;
				analysis->file_menu.save_interval_as_button=(Widget)NULL;
				analysis->file_menu.read_times_button=(Widget)NULL;
				analysis->file_menu.save_times_button=(Widget)NULL;
				analysis->write_event_times_warning_box=(Widget)NULL;
				analysis->write_event_times_warning_box_s=(Widget)NULL;
				analysis->file_menu.save_potentials_button=(Widget)NULL;
				analysis->file_menu.read_bard_data_button=(Widget)NULL;
				analysis->file_menu.read_beekeeper_data_button=(Widget)NULL;
				analysis->file_menu.read_cardiomapp_data_button=(Widget)NULL;
				analysis->file_menu.read_neurosoft_data_button=(Widget)NULL;
				analysis->print_button=(Widget)NULL;
				analysis->print_menu.all_button=(Widget)NULL;
				analysis->print_menu.page_button=(Widget)NULL;
				analysis->print_menu.selected_button=(Widget)NULL;
				analysis->region_choice=(Widget)NULL;
				analysis->region_pull_down_menu=(Widget)NULL;
				analysis->number_of_regions=0;
				analysis->regions=(Widget *)NULL;
				analysis->close_button=(Widget)NULL;
				analysis->interval.reset_button=(Widget)NULL;
				analysis->interval.baseline_button=(Widget)NULL;
				analysis->interval.range_button=(Widget)NULL;
				analysis->interval.previous_button=(Widget)NULL;
#if defined (OLD_CODE)
				analysis->interval.accelerator.previous_button=(Widget)NULL;
#endif
				analysis->interval.next_button=(Widget)NULL;
#if defined (OLD_CODE)
				analysis->interval.accelerator.next_button=(Widget)NULL;
#endif
				analysis->interval.accept_button=(Widget)NULL;
#if defined (OLD_CODE)
				analysis->interval.accelerator.accept_button=(Widget)NULL;
#endif
				analysis->interval.reject_button=(Widget)NULL;
#if defined (OLD_CODE)
				analysis->interval.accelerator.reject_button=(Widget)NULL;
#endif
				analysis->interval.drawing_area=(Widget)NULL;
				analysis->interval.drawing=(struct Drawing_2d *)NULL;
				analysis->interval.axes_height=0;
				analysis->interval.axes_left=0;
				analysis->interval.axes_top=0;
				analysis->interval.axes_width=0;
				analysis->interval.left_box=0;
				analysis->interval.right_box=0;
				analysis->signals.drawing_area=(Widget)NULL;
				analysis->signals.drawing=(struct Drawing_2d *)NULL;
				analysis->signals.layout=layout;
				analysis->signals.number_of_signals=0;
				analysis->signals.number_of_rows=0;
				analysis->signals.number_of_columns=0;
				analysis->signals.signal_width=0;
				analysis->signals.signal_height=0;
				analysis->signals.axes_height=0;
				analysis->signals.axes_left=0;
				analysis->signals.axes_top=0;
				analysis->signals.axes_width=0;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(analysis_window_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)analysis;
					identifier_list[1].value=(XtPointer)identifying_colour;
					identifier_list[2].value=(XtPointer)create_File_open_data(
						postscript_file_extension,REGULAR,print_all_signals,
						(XtPointer)analysis,1,user_interface);
					identifier_list[3].value=(XtPointer)create_File_open_data(
						postscript_file_extension,REGULAR,print_selected_signals,
						(XtPointer)analysis,1,user_interface);
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(analysis_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch the analysis window widget */
						if (MrmSUCCESS==MrmFetchWidget(analysis_window_hierarchy,
							"analysis_window",parent,&(analysis->window),
							&analysis_window_class))
						{
							widget_spacing=user_interface->widget_spacing;
							/* set the dialog title */
							dialog_title=XmStringCreateSimple("Analysis");
							XtVaSetValues(analysis->window,
								XmNdialogTitle,dialog_title,
								NULL);
							/* set the height and background colour of the interval drawing
								area */
							XtVaSetValues(analysis->interval.drawing_area,XmNheight,
								screen_height/16,XmNbackground,
								signal_drawing_information->background_drawing_colour,NULL);
							/* set the background colour of the signals drawing area */
							XtVaSetValues(analysis->signals.drawing_area,XmNbackground,
								signal_drawing_information->background_drawing_colour,NULL);
							/* adjust the order choice */

							child_widget=XmOptionLabelGadget(analysis->order_choice);
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
								XtVaSetValues(analysis->order_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(analysis->order_choice,
									XmNleftOffset,widget_spacing-left_margin,
									NULL);
							}
							child_widget=XmOptionButtonGadget(analysis->order_choice);
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
							/* set the signal order */
							switch (order)
							{
								case EVENT_ORDER:
								{
									XtVaSetValues(analysis->order_choice,
										XmNmenuHistory,analysis->order.event_button,
										NULL);
								} break;
								case CHANNEL_ORDER:
								{
									XtVaSetValues(analysis->order_choice,
										XmNmenuHistory,analysis->order.channel_button,
										NULL);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Analysis_window.  Invalid signal order");
								} break;
							}
							/* adjust the layout choice */
							child_widget=XmOptionLabelGadget(analysis->layout_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							if (left_margin+right_margin>widget_spacing)
							{
								XtVaSetValues(analysis->layout_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(analysis->layout_choice,
									XmNleftOffset,widget_spacing-(left_margin+right_margin),
									NULL);
							}
							child_widget=XmOptionButtonGadget(analysis->layout_choice);
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
							/* set the signal layout */
							switch (layout)
							{
								case SEPARATE_LAYOUT:
								{
									XtVaSetValues(analysis->layout_choice,
										XmNmenuHistory,analysis->layout.separate_button,
										NULL);
								} break;
								case OVERLAP_LAYOUT:
								{
									XtVaSetValues(analysis->layout_choice,
										XmNmenuHistory,analysis->layout.overlap_button,
										NULL);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_Analysis_window.  Invalid signal layout");
								} break;
							}
							/* adjust the region choice */
							child_widget=XmOptionLabelGadget(analysis->region_choice);
							XtVaSetValues(child_widget,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							XtVaGetValues(child_widget,
								XmNmarginLeft,&left_margin,
								NULL);
							if (left_margin+right_margin>widget_spacing)
							{
								XtVaSetValues(analysis->region_choice,
									XmNleftOffset,0,
									NULL);
							}
							else
							{
								XtVaSetValues(analysis->region_choice,
									XmNleftOffset,widget_spacing-(left_margin+right_margin),
									NULL);
							}
							child_widget=XmOptionButtonGadget(analysis->region_choice);
							XtVaSetValues(child_widget,
								XmNshadowThickness,0,
								XmNhighlightThickness,0,
								XmNcascadePixmap,no_cascade_pixmap,
								XmNalignment,XmALIGNMENT_BEGINNING,
								XmNmarginLeft,0,
								XmNmarginRight,0,
								XmNmarginWidth,0,
								NULL);
							update_analysis_window_menu(analysis);							
							install_accelerators(parent, parent);
							/*??? more to do ? */
							if (address)
							{
								*address=analysis;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
						"create_Analysis_window.  Could not fetch analysis window widget");
							DEALLOCATE(analysis);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Analysis_window.  Could not register the identifiers");
						DEALLOCATE(analysis);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Analysis_window.  Could not register the callbacks");
					DEALLOCATE(analysis);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"create_Analysis_window.  Could not allocate analysis window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Analysis_window.  Could not open hierarchy");
			analysis=(struct Analysis_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Analysis_window.  Invalid argument(s)");
		analysis=(struct Analysis_window *)NULL;
	}
	LEAVE;

	return (analysis);
} /* create_Analysis_window */

Widget create_analysis_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for an analysis window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
==============================================================================*/
{
	Widget shell;

	ENTER(create_analysis_window_shell);
	/* create and place the analysis window shell */
	if (shell=XtVaCreatePopupShell("analysis_window_shell",
		topLevelShellWidgetClass,parent,
		XmNallowShellResize,False,
		XmNx,screen_width/2,
		XmNy,0,
		XmNwidth,screen_width/2,
		XmNheight,screen_height,
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
			"create_analysis_window_shell.  Could create the analysis window shell");
	}
	LEAVE;

	return (shell);
} /* create_analysis_window_shell */

int update_signals_drawing_area(struct Analysis_window *analysis)
/*******************************************************************************
LAST MODIFIED : 23 December 1996

DESCRIPTION :
The callback for redrawing the analysis drawing area.
==============================================================================*/
{
	int return_code;
	struct Signals_area *signals;

	ENTER(update_signals_drawing_area);
	if (analysis&&((analysis->signals).drawing)&&
		((analysis->signals).drawing_area))
	{
		signals= &(analysis->signals);
#if defined (UNEMAP_USE_NODES)
		/* use rig_node_group and draw_package */
		draw_all_signals((struct Rig *)NULL,*(analysis->datum),
			*(analysis->potential_time),signals,signals->layout,
			analysis->signal_aspect_ratio,analysis->signal_overlap_spacing,
			analysis->signal_drawing_information,analysis->user_interface,
			*(analysis->rig_node_group),*(analysis->draw_package));
#else /* defined (UNEMAP_USE_NODES) */
		/* use rig */
		draw_all_signals(*(analysis->rig),*(analysis->datum),
			*(analysis->potential_time),signals,signals->layout,
			analysis->signal_aspect_ratio,analysis->signal_overlap_spacing,
			analysis->signal_drawing_information,analysis->user_interface,
			(struct GROUP(FE_node) *)NULL,(struct Draw_package *)NULL);
#endif /* defined (UNEMAP_USE_NODES) */
		XCopyArea(analysis->user_interface->display,signals->drawing->pixel_map,
			XtWindow(signals->drawing_area),
			(analysis->signal_drawing_information->graphics_context).copy,
			0,0,signals->drawing->width,signals->drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_signals_drawing_area */

int update_interval_drawing_area(struct Analysis_window *analysis)
/*******************************************************************************
LAST MODIFIED : 11 January 2000

DESCRIPTION :
The function for redrawing the analysis interval drawing area.
==============================================================================*/
{
	Display *display;
	float x_scale;
	int device_number,first_data,i,last_data,return_code;
	struct Device **device,**highlight_device;
	struct Interval_area *interval;
	struct Region *current_region;
	struct Signal_buffer *buffer;

	ENTER(update_interval_drawing_area);
	if (analysis&&((analysis->interval).drawing)&&
		((analysis->interval).drawing_area))
	if (analysis)
	{
		display=analysis->user_interface->display;
		interval= &(analysis->interval);
		/* clear the drawing area */
		XFillRectangle(display,interval->drawing->pixel_map,
			(analysis->signal_drawing_information->graphics_context).
			background_drawing_colour,0,0,interval->drawing->width,
			interval->drawing->height);
		if ((analysis->highlight)&&(highlight_device= *(analysis->highlight))&&
			(*highlight_device)&&(buffer=get_Device_signal_buffer(*highlight_device)))
		{
			/* draw all of the active signal */
			first_data=0;
			last_data=buffer->number_of_samples-1;
			draw_signal((struct FE_node *)NULL,(struct Draw_package *)NULL,
				*highlight_device,INTERVAL_AREA_DETAIL,1,0,&first_data,&last_data,0,0,
				interval->drawing->width,interval->drawing->height,
				interval->drawing->pixel_map,&(interval->axes_left),
				&(interval->axes_top),&(interval->axes_width),
				&(interval->axes_height),analysis->signal_drawing_information,
				analysis->user_interface);
			draw_device_markers(*highlight_device,first_data,last_data,
				*(analysis->datum),1,*(analysis->potential_time),1,INTERVAL_AREA_DETAIL,
				*(analysis->event_number),interval->axes_left,interval->axes_top,
				interval->axes_width,interval->axes_height,(Window)NULL,
				interval->drawing->pixel_map,analysis->signal_drawing_information,
				analysis->user_interface);
			x_scale=SCALE_FACTOR(buffer->number_of_samples-1,
				interval->axes_width-1);
			interval->left_box=SCALE_X(buffer->start,0,interval->axes_left,x_scale);
			interval->right_box=SCALE_X(buffer->end,0,interval->axes_left,x_scale);
			/* draw the interval rectangle */
			draw_interval_box(interval->left_box,interval->axes_top,
				interval->right_box-interval->left_box+1,interval->axes_height,
				interval,analysis->signal_drawing_information,
				analysis->user_interface);
			/* update the buttons */
			if ((*highlight_device)&&
				(ELECTRODE==(*highlight_device)->description->type))
			{
				XtSetSensitive(interval->accept_button,True);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.accept_button,True);
#endif
				XtSetSensitive(interval->reject_button,True);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.reject_button,True);
#endif
			}
			else
			{
				XtSetSensitive(interval->accept_button,False);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.accept_button,False);
#endif
				XtSetSensitive(interval->reject_button,False);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.reject_button,False);
#endif
			}
			if (current_region=(*(analysis->rig))->current_region)
			{
				device_number=0;
				device=(*(analysis->rig))->devices;
				for (i=highlight_device-device;i>0;i--)
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
				device_number=highlight_device-((*(analysis->rig))->devices);
			}
			if ((!current_region&&(device_number==((*(analysis->rig))->
				number_of_devices)-1))||(current_region&&(device_number==
				current_region->number_of_devices-1)))
			{
				XtSetSensitive(interval->next_button,False);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.next_button,False);
#endif
			}
			else
			{
				XtSetSensitive(interval->next_button,True);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.next_button,True);
#endif
			}
			if (0==device_number)
			{
				XtSetSensitive(interval->previous_button,False);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.previous_button,False);
#endif
			}
			else
			{
				XtSetSensitive(interval->previous_button,True);
#if defined (OLD_CODE)
				XtSetSensitive(interval->accelerator.previous_button,True);
#endif
			}
		}
		XCopyArea(display,interval->drawing->pixel_map,
			XtWindow(interval->drawing_area),
			(analysis->signal_drawing_information->graphics_context).copy,0,0,
			interval->drawing->width,interval->drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_interval_drawing_area */

int draw_all_signals(struct Rig *rig,int datum,int potential_time,
	struct Signals_area *signals,enum Signal_layout layout,
	float signal_aspect_ratio,int signal_overlap_spacing,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface,struct GROUP(FE_node) *rig_node_group,
	struct Draw_package *draw_package)
/*******************************************************************************
LAST MODIFIED : 6 December 1999

DESCRIPTION :
This function draws all the signals for the <signal_event_list> in the <drawing>
using the <graphics_context>.

Pass either <rig > or (<rig_node_group> and <draw_package> ).  If pass
<rig_node_group> and <draw_package>, then we create an array of pointers to the
nodes, in an FE_node_order_info, so can increment through nodes similar
to rig->devices. This is not ideal, but is done for compatibility with the
rig->devices approach. ??JW Do differently when have removed rig->devices
Should use GROUP NEXT operator (when it's ready!)
==============================================================================*/
{
	int axes_height,axes_left,axes_top,axes_width,drawing_height,drawing_width,
		first_data,i,j,last_data,number_of_columns,number_of_rows,number_of_signals,
		return_code,signal_height,signal_width,xpos,ypos;
	struct Drawing_2d *drawing;
	struct Device **device;
	struct Region *current_region;
	struct Signal_buffer *buffer;
	Pixmap pixel_map;
#if defined (UNEMAP_USE_NODES)
	int node_number;
	struct FE_field *field;
	struct FE_node *node;
	struct FE_node_order_info *node_order_info;
#endif /* defined (UNEMAP_USE_NODES) */

	ENTER(draw_all_signals);
#if defined (UNEMAP_USE_NODES)
	field=(struct FE_field *)NULL;
	node_order_info=(struct FE_node_order_info *)NULL;
	node=(struct FE_node *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
	drawing=(struct Drawing_2d *)NULL;
	current_region=(struct Region *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	device= (struct Device **)NULL;
	if (signals&&(drawing=signals->drawing)&&signal_drawing_information&&
		user_interface)
	{
		pixel_map=drawing->pixel_map;
		drawing_width=drawing->width;
		drawing_height=drawing->height;
		/* clear the drawing area */
		XFillRectangle(user_interface->display,pixel_map,
			(signal_drawing_information->graphics_context).background_drawing_colour,
			0,0,drawing_width,drawing_height);
		/* check that rig is set XOR both the draw_package and  the rig_node_group
			are set, */
		/* OR everything is NULL */
		if ((!rig_node_group&&!draw_package)||((rig_node_group&&draw_package)&&
			!rig))
		{
			number_of_signals=0;
			return_code=1;
			if (rig_node_group) /* must extract info from node group */
			{
#if defined (UNEMAP_USE_NODES)
				/* create an array of pointers to the nodes, in node_order_info */
				/* so can increment through nodes similar to devices. */
				/* should use GROUP NEXT operator (when it's ready!) */
				if (!node_order_info)
				{
					node_order_info=CREATE(FE_node_order_info)(0);
				}
				if (node_order_info)
				{
					if (return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
						fill_FE_node_order_info,(void *)node_order_info,
						rig_node_group))
					{
						node_number=0;
						node=get_FE_node_order_info_node(node_order_info,node_number);
						number_of_signals= get_FE_node_order_info_number_of_nodes(
							node_order_info);
						first_data=0;
						/* time is stored at signal field.  For moment, assume only 1 signal
							per node */
						field=get_Draw_package_signal_field(draw_package);
						last_data=get_FE_field_number_of_times(field)-1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"draw_all_signals. "
						"CREATE(FE_node_order_info) failed");
					return_code=0;
				}
#endif /* defined (UNEMAP_USE_NODES) */
			}
			else if (rig&&(device=rig->devices)&&(*device)&&
				(buffer=get_Device_signal_buffer(*device)))
			{
				first_data=buffer->start;
				last_data=buffer->end;
				/* determine the number of signals */
				if (current_region=rig->current_region)
				{
					number_of_signals=current_region->number_of_devices;
				}
				else
				{
					number_of_signals=rig->number_of_devices;
				}
			}/*if rig( */
			if ((number_of_signals>0)&&return_code)
			{
				switch (layout)
				{
					case SEPARATE_LAYOUT:
					{
						/* First choose the signal width so that all signals are shown, the
							 signals have the specified aspect ratio and the signals are as
							 large as possible */
						signal_width=(int)sqrt((float)(drawing_height*drawing_width)/
							(signal_aspect_ratio*(float)number_of_signals));
						if (drawing_width<signal_width)
						{
							signal_width=drawing_width;
						}
						number_of_columns=(drawing_width)/signal_width;
						number_of_rows=(number_of_signals-1)/number_of_columns+1;
						/* equalize the columns */
						if ((number_of_signals>1)&&(number_of_columns>1)&&
							((i=number_of_rows*number_of_columns-number_of_signals-1)>0))
						{
							number_of_rows -= i/(number_of_columns-1);
						}
						/* make the signals fill the width and height */
						signal_width=(drawing_width)/number_of_columns;
						signal_height=(drawing_height)/number_of_rows;
						/* save signal drawing area parameters */
						signals->number_of_signals=number_of_signals;
						signals->number_of_rows=number_of_rows;
						signals->number_of_columns=number_of_columns;
						signals->signal_width=signal_width;
						signals->signal_height=signal_height;
						/* Draw each signal */
						/* if using rig, skip the devices not in the current region */
						if ((rig)&&(current_region))
						{
							while ((*device)->description->region!=current_region)
							{
								device++;
							}
						}
						i=0;
						j=0;
						xpos=0;
						ypos=0;
						draw_signal(
#if defined (UNEMAP_USE_NODES)
							node,draw_package,(struct Device *)NULL,
#else /* defined (UNEMAP_USE_NODES) */
							(struct FE_node *)NULL,(struct Draw_package *)NULL,*device,
#endif /* defined (UNEMAP_USE_NODES) */
							SIGNAL_AREA_DETAIL,1,0,&first_data,&last_data,xpos,ypos,
							signal_width,signal_height,pixel_map,&axes_left,&axes_top,
							&axes_width,&axes_height,signal_drawing_information,
							user_interface);
						signals->axes_left=axes_left;
						signals->axes_top=axes_top;
						signals->axes_width=axes_width;
						signals->axes_height=axes_height;
						/*if using rig, draw_device_markers, and inc device pointer */
						/* write code rig_node based draw_device_markers later */
						if (rig)
						{
							draw_device_markers(*device,first_data,last_data,datum,1,
								potential_time,1,SIGNAL_AREA_DETAIL,0,axes_left,axes_top,
								axes_width,axes_height,(Window)NULL,pixel_map,
								signal_drawing_information,user_interface);
							device++;
						}
#if defined (UNEMAP_USE_NODES)
						else
						{
							/* for rig_node based, get next rig node */
							node_number++;
							node=get_FE_node_order_info_node(node_order_info,node_number);
						}
#endif /* defined (UNEMAP_USE_NODES) */
						number_of_signals--;
						while (number_of_signals>0)
						{
							/* skip the devices not in the current region */
							/* if using rig, skip the devices not in the current region */
							if ((rig)&&(current_region))
							{
								while ((*device)->description->region!=current_region)
								{
									device++;
								}
							}
							i++;
							if (i>=number_of_rows)
							{
								i=0;
								ypos=0;
								j++;
								xpos=(j*drawing_width)/number_of_columns;
							}
							else
							{
								ypos=(i*drawing_height)/number_of_rows;
							}
							draw_signal(
#if defined (UNEMAP_USE_NODES)
								node,draw_package,(struct Device *)NULL,
#else /* defined (UNEMAP_USE_NODES) */
								(struct FE_node *)NULL,(struct Draw_package *)NULL,*device,
#endif /* defined (UNEMAP_USE_NODES) */
								SIGNAL_AREA_DETAIL,1,0,&first_data,&last_data,xpos,ypos,
								signal_width,signal_height,pixel_map,&axes_left,&axes_top,
								&axes_width,&axes_height,signal_drawing_information,
								user_interface);
							/*if using rig, draw_device_markers, and inc device pointer */
							/* write code rig_node based draw_device_markers later */
							if (rig)
							{
								draw_device_markers(*device,first_data,last_data,datum,1,
									potential_time,1,SIGNAL_AREA_DETAIL,0,axes_left,axes_top,
									axes_width,axes_height,(Window)NULL,pixel_map,
									signal_drawing_information,user_interface);
								device++;
							}
#if defined (UNEMAP_USE_NODES)
							else
							{
								/* for rig_node based, get next rig node*/
								node_number++;
								node=get_FE_node_order_info_node(node_order_info,node_number);
							}
#endif /* defined (UNEMAP_USE_NODES) */
							number_of_signals--;
						}
						return_code=1;
					} break;
					case OVERLAP_LAYOUT:
					{
						number_of_rows=(drawing_height/signal_overlap_spacing)-3;
						if (number_of_rows>number_of_signals)
						{
							number_of_rows=number_of_signals;
						}
						number_of_columns=(number_of_signals-1)/number_of_rows+1;
						/* equalize the columns */
						if ((number_of_signals>1)&&(number_of_columns>1)&&
							((i=number_of_rows*number_of_columns-number_of_signals-1)>0))
						{
							number_of_rows -= i/(number_of_columns-1);
						}
						/* make the signals fill the width and height */
						signal_width=(drawing_width)/number_of_columns;
						signal_height=4*signal_overlap_spacing;
						/* save signal drawing area parameters */
						signals->number_of_signals=number_of_signals;
						signals->number_of_rows=number_of_rows;
						signals->number_of_columns=number_of_columns;
						signals->signal_width=signal_width;
						signals->signal_height=signal_height;
						/* Draw each signal */
						i=0;
						j=0;
						xpos=0;
						ypos=0;
						draw_signal(
#if defined (UNEMAP_USE_NODES)
							node,draw_package,(struct Device *)NULL,
#else /* defined (UNEMAP_USE_NODES) */
							(struct FE_node *)NULL,(struct Draw_package *)NULL,*device,
#endif /* defined (UNEMAP_USE_NODES) */
							SIGNAL_AREA_DETAIL,1,0,&first_data,&last_data,xpos,ypos,
							signal_width,signal_height,pixel_map,&axes_left,&axes_top,
							&axes_width,&axes_height,signal_drawing_information,
							user_interface);
						signals->axes_left=axes_left;
						signals->axes_top=axes_top;
						signals->axes_width=axes_width;
						signals->axes_height=axes_height;
						if (rig)
						{
							/* for rig based, get next device */
							device++;
						}
#if defined (UNEMAP_USE_NODES)
						else
						{
							/* for rig_node based, get next rig node*/
							node_number++;
							node=get_FE_node_order_info_node(node_order_info,node_number);
						}
#endif /* defined (UNEMAP_USE_NODES) */
						number_of_signals--;
						while (number_of_signals>0)
						{
							i++;
							if (i>=number_of_rows)
							{
								i=0;
								ypos=0;
								j++;
								xpos=(j*drawing_width)/number_of_columns;
							}
							else
							{
								ypos=(i*drawing_height)/(number_of_rows+3);
							}
							draw_signal(
#if defined (UNEMAP_USE_NODES)
								node,draw_package,(struct Device *)NULL,
#else /* defined (UNEMAP_USE_NODES) */
								(struct FE_node *)NULL,(struct Draw_package *)NULL,*device,
#endif /* defined (UNEMAP_USE_NODES) */
								SIGNAL_AREA_DETAIL,1,0,&first_data,&last_data,xpos,ypos,
								signal_width,signal_height,pixel_map,&axes_left,&axes_top,
								&axes_width,&axes_height,signal_drawing_information,
								user_interface);
							if (rig)
							{
								/* for rig based, get next device */
								device++;
							}
#if defined (UNEMAP_USE_NODES)
							else
							{
								/* for rig_node based, get next rig node*/
								node_number++;
								node=get_FE_node_order_info_node(node_order_info,node_number);
							}
#endif /* defined (UNEMAP_USE_NODES) */
							number_of_signals--;
						}
						/* have to draw the markers separately because the signals
							 overlap */
						if (rig)
						{
							/* for rig based */
							device=rig->devices;
							number_of_signals=rig->number_of_devices;
						}
#if defined (UNEMAP_USE_NODES)
						else
						{
							/* for rig_node based, get next rig node*/
							node_number=0;
							node=get_FE_node_order_info_node(node_order_info,node_number);
							number_of_signals=get_FE_node_order_info_number_of_nodes(
								node_order_info);
						}
#endif /* defined (UNEMAP_USE_NODES) */
						i=0;
						j=0;
						axes_left=signals->axes_left;
						axes_top=signals->axes_top;
						xpos=axes_left;
						ypos=axes_top;
						while (number_of_signals>0)
						{
							if (rig)
							{
								draw_device_markers(*device,first_data,last_data,datum,1,
									potential_time,1,SIGNAL_AREA_DETAIL,0,xpos,ypos,axes_width,
									axes_height,(Window)NULL,pixel_map,signal_drawing_information,
									user_interface);
							}
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
								ypos=axes_top+(i*drawing_height)/(number_of_rows+3);
							}
							if (rig)
							{
								/* for rig based, get next device */
								device++;
							}
#if defined (UNEMAP_USE_NODES)
							else
							{
								/* for rig_node based, get next rig node*/
								node_number=0;
								node=get_FE_node_order_info_node(node_order_info,node_number);
							}
#endif /* defined (UNEMAP_USE_NODES) */
							number_of_signals--;
						}
						return_code=1;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"draw_all_signals.  Invalid signal layout");
					} break;
				} /* switch */
			}	/* if (number_of_signals>0)*/
		} /* if ((!rig_node_group&&!draw_package)|| */
		else
		{
			display_message(ERROR_MESSAGE,"draw_all_signals. Invalid arguments");
			return_code=0;
		}
	}/* if (signals&&(drawing=s */
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_all_signals.  signals missing or incomplete");
		return_code=0;
	}
#if defined (UNEMAP_USE_NODES)
	/* no longer needed */
	DEACCESS(FE_node_order_info)(&node_order_info);
#endif /* defined (UNEMAP_USE_NODES) */
	LEAVE;
	return (return_code);
} /* draw_all_signals */

int update_analysis_window_menu(struct Analysis_window *analysis)
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
Updates the analysis region menu to be consistent with the current rig.
==============================================================================*/
{
	Widget current_region;
	WidgetList regions;
	int number_of_regions,return_code;
#define NUMBER_OF_ATTRIBUTES 2
	Arg attributes[NUMBER_OF_ATTRIBUTES];
	struct Region_list_item *region_item;
	struct Rig *rig;
	Window analysis_menu_window;

	ENTER(update_analysis_window_menu);
	if (analysis)
	{
		if (analysis_menu_window=XtWindow(analysis->menu))
		{
			XtUnrealizeWidget(analysis->menu);
		}
		/* remove the current entries from the option menu */
		if (((number_of_regions=analysis->number_of_regions)>0)&&
			(regions=analysis->regions))
		{
			while (number_of_regions>0)
			{
				XtUnmanageChild(*regions);
				XtDestroyWidget(*regions);
				regions++;
				number_of_regions--;
			}
			DEALLOCATE(analysis->regions);
			analysis->number_of_regions=0;
			analysis->current_region=(Widget)NULL;
			return_code=1;
		}
		else
		{
			return_code=1;
		}
		if (return_code)
		{
			if ((rig= *(analysis->rig))&&
				((number_of_regions=rig->number_of_regions)>0))
			{
				/* put the regions for the new rig into the option menu */
				if (number_of_regions>1)
				{
					number_of_regions++;
				}
				if (ALLOCATE(regions,Widget,number_of_regions))
				{
					analysis->number_of_regions=number_of_regions;
					analysis->regions=regions;
					current_region=(Widget)NULL;
					XtSetArg(attributes[1],XmNfontList,XmFontListCreate(analysis->
						user_interface->button_font,XmSTRING_DEFAULT_CHARSET));
					if (number_of_regions>1)
					{
						XtSetArg(attributes[0],XmNlabelString,
							(XtPointer)XmStringCreate("All regions",
							XmSTRING_DEFAULT_CHARSET));
						if (*regions=XmCreatePushButtonGadget(
							analysis->region_pull_down_menu,"All regions",attributes,
							NUMBER_OF_ATTRIBUTES))
						{
							XtManageChild(*regions);
							if (!(rig->current_region))
							{
								current_region= *regions;
							}
							regions++;
						}
						else
						{
							return_code=0;
						}
					}
					region_item=rig->region_list;
					while (region_item&&return_code)
					{
						XtSetArg(attributes[0],XmNlabelString,
							(XtPointer)XmStringCreate(
							region_item->region->name,XmSTRING_DEFAULT_CHARSET));
						if (*regions=XmCreatePushButtonGadget(
							analysis->region_pull_down_menu,region_item->region->name,
							attributes,NUMBER_OF_ATTRIBUTES))
						{
							XtManageChild(*regions);
							if ((rig->current_region==region_item->region)||
								((!current_region)&&(!(rig->current_region))))
							{
								current_region= *regions;
							}
							region_item=region_item->next;
							regions++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"update_analysis_window_menu.  Could not create region PushButtonGadget");
							return_code=0;
						}
					}
					if (return_code)
					{
						XtVaSetValues(analysis->region_choice,
							XmNmenuHistory,current_region,
							NULL);
						analysis->current_region=current_region;
						XtManageChild(analysis->region_choice);
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"update_analysis_window_menu.  Could not create region PushButtonGadget");
						return_code=0;
						/*???What about destroying PushButtons already created ? */
						DEALLOCATE(analysis->regions);
						analysis->number_of_regions=0;
						analysis->current_region=(Widget)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
				"update_analysis_window_menu.  Insufficient memory for region list");
					return_code=0;
				}
			}
			else
			{
				XtUnmanageChild(analysis->region_choice);
			}
		}
		if (analysis_menu_window&&return_code)
		{
			XtRealizeWidget(analysis->menu);
			XtManageChild(analysis->menu);
		}
	}
	LEAVE;

	return (return_code);
} /* update_analysis_window_menu */

int highlight_signal(struct Device *device,int device_number,int start_data,
	int end_data,int datum,int potential_time,struct Signals_area *signals,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 30 July 1999

DESCRIPTION :
Highlights/dehighlights the <device> in the <signals> area.
==============================================================================*/
{
	int axes_height,axes_left,axes_top,axes_width,return_code,xpos,ypos;

	ENTER(highlight_signal);
	if (signals&&signal_drawing_information&&user_interface)
	{
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
		draw_device_markers(device,start_data,end_data,datum,1,potential_time,1,
			SIGNAL_AREA_DETAIL,0,xpos+signals->axes_left,
			ypos+signals->axes_top,signals->axes_width,signals->axes_height,
			(Window)NULL,signals->drawing->pixel_map,signal_drawing_information,
			user_interface);
		draw_signal((struct FE_node *)NULL,(struct Draw_package *)NULL,device,
			SIGNAL_AREA_DETAIL,1,0,&start_data,&end_data,xpos,ypos,
			signals->signal_width,signals->signal_height,signals->drawing->pixel_map,
			&axes_left,&axes_top,&axes_width,&axes_height,signal_drawing_information,
			user_interface);
		draw_device_markers(device,start_data,end_data,datum,1,potential_time,1,
			SIGNAL_AREA_DETAIL,0,xpos+signals->axes_left,
			ypos+signals->axes_top,signals->axes_width,signals->axes_height,
			(Window)NULL,signals->drawing->pixel_map,signal_drawing_information,
			user_interface);
		XCopyArea(user_interface->display,signals->drawing->pixel_map,
			XtWindow(signals->drawing_area),
			(signal_drawing_information->graphics_context).copy,xpos,ypos,
			signals->drawing->width,signals->drawing->height,xpos,ypos);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* highlight_signal */
