/*******************************************************************************
FILE : analysis.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "general/debug.h"
#include "general/postscript.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "general/value.h"
#include "unemap/analysis.h"
#include "unemap/drawing_2d.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct Signal_drawing_information Signal_drawing_information_settings;

/*
Module functions
----------------
*/
static void calculate_divisions(float start,float end,int divisions,
	float *division_width,float *min_tick_mark,float *max_tick_mark)
/*******************************************************************************
LAST MODIFIED : 9 June 1998

DESCRIPTION :
From the <start> and <end> values and the desired number of <divisions>, this
routine calculates the <division_width> and the values for the <min_tick_mark>
and the <max_tick_mark>.
==============================================================================*/
{
	float exponent,mantissa,step;

	ENTER(calculate_divisions);
	if ((start<end)&&(0<divisions))
	{
		step=(end-start)/(5*(float)divisions);
	}
	else
	{
		step=1;
	}
	mantissa=log10(step);
	exponent=floor(mantissa);
	exponent=pow(10,exponent);
	step=5*floor(0.5+(step/exponent))*exponent;
	*min_tick_mark=step*ceil(start/step);
	*max_tick_mark=step*floor(end/step);
	*division_width=step;
	LEAVE;
} /* calculate_divisions */

static int calculate_moving_average(float *objective_values,
	int number_of_objective_values,int objective_values_step,int average_width)
/*******************************************************************************
LAST MODIFIED : 30 September 2001

DESCRIPTION :
Calculates the moving average of the <objective_values>.
==============================================================================*/
{
	float average_after,*average_after_value,average_before,first_value,
		last_value,*objective_value,*save_value,*save_values,temp_value;
	int average_width_after,average_width_before,i,return_code;

	ENTER(calculate_moving_average);
	return_code=0;
	if (objective_values&&(0<number_of_objective_values)&&
		(0<objective_values_step)&&(0<average_width))
	{
		if (average_width>1)
		{
			average_width_before=average_width/2;
			average_width_after=average_width-1-average_width_before;
			if (ALLOCATE(save_values,float,average_width_before))
			{
				objective_value=objective_values;
				first_value=objective_values[0];
				last_value=objective_values[(number_of_objective_values-1)*
					objective_values_step];
				save_value=save_values;
				for (i=average_width_before;i>0;i--)
				{
					*save_value=first_value;
					save_value++;
				}
				save_value=save_values;
				average_before=first_value*(float)average_width_before;
				average_after=0;
				if (average_width_after<number_of_objective_values)
				{
					for (i=average_width_after;i>0;i--)
					{
						objective_value += objective_values_step;
						average_after += *objective_value;
					}
					objective_value=objective_values;
					average_after_value=objective_value+
						(average_width_after*objective_values_step);
					for (i=number_of_objective_values-average_width_after-1;i>0;i--)
					{
						temp_value=average_after+(*objective_value)+average_before;
						average_before += (*objective_value)-(*save_value);
						*save_value= *objective_value;
						save_value++;
						if (save_value-save_values>=average_width_before)
						{
							save_value=save_values;
						}
						*objective_value=temp_value;
						objective_value += objective_values_step;
						average_after_value += objective_values_step;
						average_after += (*average_after_value)-(*objective_value);
					}
					for (i=average_width_after+1;i>0;i--)
					{
						temp_value=average_after+(*objective_value)+average_before;
						average_before += (*objective_value)-(*save_value);
						*save_value= *objective_value;
						save_value++;
						if (save_value-save_values>=average_width_before)
						{
							save_value=save_values;
						}
						*objective_value=temp_value;
						objective_value += objective_values_step;
						average_after += last_value-(*objective_value);
					}
				}
				else
				{
					for (i=number_of_objective_values-1;i>0;i--)
					{
						objective_value += objective_values_step;
						average_after += *objective_value;
					}
					average_after += last_value*
						(float)(average_width_after-(number_of_objective_values-1));
					objective_value=objective_values;
					for (i=number_of_objective_values;i>0;i--)
					{
						temp_value=average_after+(*objective_value)+average_before;
						average_before += (*objective_value)-first_value;
						*objective_value=temp_value;
						objective_value += objective_values_step;
						average_after += last_value-(*objective_value);
					}
				}
				objective_value=objective_values;
				temp_value=(float)average_width;
				for (i=number_of_objective_values;i>0;i--)
				{
					*objective_value /= temp_value;
					objective_value += objective_values_step;
				}
				DEALLOCATE(save_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_moving_average.  Could not allocate save_values");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_moving_average.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_moving_average */

/*
Global functions
----------------
*/
int calculate_device_objective(struct Device *device,
	enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,float *objective_values,
	int number_of_objective_values,int objective_values_step,int average_width)
/*******************************************************************************
LAST MODIFIED : 30 September 2001

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the <device>.
Storing the values in the array (<objective_values> every
<objective_values_step>) provided.
==============================================================================*/
{
	float average_after,*average_after_value,average_before,first_value,
		*float_value,gain,last_value,objective_maximum,objective_minimum,
		*objective_value,offset,*save_value,*save_values,scale,signal_maximum,
		signal_minimum,temp_value;
	int i,number_of_samples,number_of_signals,return_code;
	short *short_value;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(calculate_device_objective);
	number_of_samples=0;
	return_code=0;
	if (device&&(signal=device->signal)&&(buffer=signal->buffer)&&
		(0<(number_of_samples=buffer->number_of_samples))&&
		(((SHORT_INT_VALUE==buffer->value_type)&&
		(buffer->signals.short_int_values))||((FLOAT_VALUE==buffer->value_type)&&
		(buffer->signals.float_values)))&&objective_values&&
		(number_of_samples<=number_of_objective_values)&&(0<objective_values_step)&&
		(0<average_width)&&(device->channel))
	{
		if (ALLOCATE(save_values,float,average_width))
		{
			number_of_signals=signal->buffer->number_of_signals;
			objective_value=objective_values;
			switch (buffer->value_type)
			{
				case SHORT_INT_VALUE:
				{
					short_value=(buffer->signals.short_int_values)+(signal->index);
					for (i=number_of_samples;i>0;i--)
					{
						*objective_value=(float)(*short_value);
						short_value += number_of_signals;
						objective_value += objective_values_step;
					}
				} break;
				case FLOAT_VALUE:
				{
					float_value=(buffer->signals.float_values)+(signal->index);
					for (i=number_of_samples;i>0;i--)
					{
						*objective_value=(float)(*float_value);
						float_value += number_of_signals;
						objective_value += objective_values_step;
					}
				} break;
			}
			objective_value=objective_values;
			gain=device->channel->gain;
			offset=device->channel->offset;
			temp_value=gain*((*objective_value)-offset);
			signal_maximum=temp_value;
			signal_minimum=signal_maximum;
			for (i=number_of_samples-1;i>0;i--)
			{
				objective_value += objective_values_step;
				temp_value=gain*((*objective_value)-offset);
				if (temp_value>signal_maximum)
				{
					signal_maximum=temp_value;
				}
				else
				{
					if (temp_value<signal_minimum)
					{
						signal_minimum=temp_value;
					}
				}
			}
			/* calculate objective function */
			objective_value=objective_values;
			switch (detection)
			{
				case EDA_INTERVAL:
				case EDA_THRESHOLD:
				{
					if ((ABSOLUTE_VALUE==objective)||(POSITIVE_VALUE==objective)||
						(NEGATIVE_VALUE==objective))
					{
						switch (objective)
						{
							case ABSOLUTE_VALUE:
							{
								objective_value=objective_values;
								for (i=number_of_samples;i>0;i--)
								{
									if (*objective_value<0)
									{
										*objective_value= -(*objective_value);
									}
									objective_value += objective_values_step;
								}
							} break;
							case NEGATIVE_VALUE:
							{
								objective_value=objective_values;
								for (i=number_of_samples;i>0;i--)
								{
									*objective_value= -(*objective_value);
									objective_value += objective_values_step;
								}
							} break;
						}
						/* take moving average */
						return_code=calculate_moving_average(objective_values,
							number_of_samples,objective_values_step,average_width);
					}
					else
					{
						first_value=objective_values[0];
						last_value=
							objective_values[(number_of_samples-1)*objective_values_step];
						save_value=save_values;
						for (i=average_width;i>0;i--)
						{
							*save_value=first_value;
							save_value++;
						}
						save_value=save_values;
						average_before=first_value*(float)average_width;
						average_after=0;
						if (average_width<number_of_samples)
						{
							for (i=average_width;i>0;i--)
							{
								objective_value += objective_values_step;
								average_after += *objective_value;
							}
							objective_value=objective_values;
							average_after_value=objective_value+
								(average_width*objective_values_step);
							for (i=number_of_samples-average_width-1;i>0;i--)
							{
								temp_value=average_after-average_before;
								average_before += (*objective_value)-(*save_value);
								*save_value= *objective_value;
								save_value++;
								if (save_value-save_values>=average_width)
								{
									save_value=save_values;
								}
								*objective_value=temp_value;
								objective_value += objective_values_step;
								average_after_value += objective_values_step;
								average_after += (*average_after_value)-(*objective_value);
							}
							for (i=average_width+1;i>0;i--)
							{
								temp_value=average_after-average_before;
								average_before += (*objective_value)-(*save_value);
								*save_value= *objective_value;
								save_value++;
								if (save_value-save_values>=average_width)
								{
									save_value=save_values;
								}
								*objective_value=temp_value;
								objective_value += objective_values_step;
								average_after += last_value-(*objective_value);
							}
						}
						else
						{
							for (i=number_of_samples-1;i>0;i--)
							{
								objective_value += objective_values_step;
								average_after += *objective_value;
							}
							average_after +=
								last_value*(float)(average_width-(number_of_samples-1));
							objective_value=objective_values;
							for (i=number_of_samples;i>0;i--)
							{
								temp_value=average_after-average_before;
								average_before += (*objective_value)-first_value;
								*objective_value=temp_value;
								objective_value += objective_values_step;
								average_after += last_value-(*objective_value);
							}
						}
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								objective_value=objective_values;
								objective_minimum=0;
								objective_maximum=0;
								for (i=number_of_samples;i>0;i--)
								{
									if (*objective_value<0)
									{
										*objective_value= -(*objective_value);
									}
									if (*objective_value>objective_maximum)
									{
										objective_maximum= *objective_value;
									}
									objective_value += objective_values_step;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								objective_value=objective_values;
								objective_minimum= -(*objective_value);
								objective_maximum=objective_minimum;
								for (i=number_of_samples;i>0;i--)
								{
									*objective_value= -(*objective_value);
									if (*objective_value>objective_maximum)
									{
										objective_maximum= *objective_value;
									}
									else
									{
										if (*objective_value<objective_minimum)
										{
											objective_minimum= *objective_value;
										}
									}
									objective_value += objective_values_step;
								}
							} break;
							case POSITIVE_SLOPE:
							{
								objective_value=objective_values;
								objective_minimum= *objective_value;
								objective_maximum=objective_minimum;
								for (i=number_of_samples;i>0;i--)
								{
									if (*objective_value>objective_maximum)
									{
										objective_maximum= *objective_value;
									}
									else
									{
										if (*objective_value<objective_minimum)
										{
											objective_minimum= *objective_value;
										}
									}
									objective_value += objective_values_step;
								}
							} break;
						}
						if (signal_maximum==signal_minimum)
						{
							signal_minimum -= 1;
							signal_maximum += 1;
						}
						if (objective_maximum==objective_minimum)
						{
							objective_value=objective_values;
							for (i=number_of_samples;i>0;i--)
							{
								*objective_value=signal_minimum;
								objective_value += objective_values_step;
							}
						}
						else
						{
							objective_value=objective_values;
							scale=(signal_maximum-signal_minimum)/
								(objective_maximum-objective_minimum);
							for (i=number_of_samples;i>0;i--)
							{
								*objective_value=signal_minimum+
									scale*((*objective_value)-objective_minimum);
								objective_value += objective_values_step;
							}
						}
					}
				} break;
				case EDA_LEVEL:
				{
					/* take absolute value */
					for (i=number_of_samples;i>0;i--)
					{
						if (*objective_value<0)
						{
							*objective_value= -(*objective_value);
						}
						objective_value += objective_values_step;
					}
					/* take moving average */
					return_code=calculate_moving_average(objective_values,
						number_of_samples,objective_values_step,average_width);
				} break;
			}
			DEALLOCATE(save_values);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_device_objective.  Could not allocate save_values");
			return_code=0;
		}
	}
	else
	{
		if (device)
		{
			if (device->signal)
			{
				if (device->signal->buffer)
				{
					display_message(ERROR_MESSAGE,
"calculate_device_objective.  Invalid argument(s).  %d (%d %d) %p %p %p %d %d %d %p",
						buffer->value_type,SHORT_INT_VALUE,FLOAT_VALUE,
						buffer->signals.short_int_values,buffer->signals.float_values,
						objective_values,number_of_objective_values,objective_values_step,
						number_of_samples,device->channel);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_device_objective.  Missing signal buffer");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_device_objective.  Missing signal");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_device_objective.  Missing device");
		}
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_device_objective */

int calculate_device_event_markers(struct Device *device,
	int start_search,int end_search,enum Event_detection_algorithm detection,
	float *objective_values,int number_of_objective_values,
	int objective_values_step,int number_of_events,int threshold_percentage,
	int minimum_separation_milliseconds,float level)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Calculate the positions of the event markers for a signal/<device>/<device_node>
based upon the the start and end times, the number of events, the <detection> 
algorithm and the <objective_values>.
==============================================================================*/
{
	float maximum_objective,minimum_objective,*objective_value,threshold;
	int event_number,interval_end,maximum,minimum_separation,no_maximum,present,
		return_code;
	struct Event *event,**event_next;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(calculate_device_event_markers);
	signal=(struct Signal *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if ((0<=start_search)&&(start_search<=end_search)&&
		device&&(signal=device->signal)&&(buffer=signal->buffer)&&
		(end_search<buffer->number_of_samples)&&
		(((SHORT_INT_VALUE==buffer->value_type)&&
		(buffer->signals.short_int_values))||((FLOAT_VALUE==buffer->value_type)&&
		(buffer->signals.float_values)))&&
		(((EDA_INTERVAL==detection)&&
		(0<number_of_events))||((EDA_LEVEL==detection)&&(0<=level))||
		((EDA_THRESHOLD==detection)&&(0<=threshold_percentage)&&
		(threshold_percentage<=100)&&(0<minimum_separation_milliseconds)))&&
		objective_values&&(0<objective_values_step)
		&&(buffer->number_of_samples<=number_of_objective_values))
	{
		/* free the previous events */
		destroy_Event_list(&(signal->first_event));
		objective_value=objective_values+(start_search*objective_values_step);
		switch (detection)
		{
			case EDA_INTERVAL:
			{
				present=start_search;
				event=(struct Event *)NULL;
				event_next= &(signal->first_event);
				event_number=1;
				no_maximum=0;
				do
				{
					maximum_objective= *objective_value;
					maximum=present;
					interval_end=SCALE_X(event_number,0,start_search,
						SCALE_FACTOR(number_of_events,end_search-start_search));
					while (present<interval_end)
					{
						present++;
						objective_value += objective_values_step;
						if ((maximum_objective< *objective_value)||no_maximum)
						{
							maximum_objective= *objective_value;;
							maximum=present;
							no_maximum=0;
						}
					}
					if (event=create_Event(maximum,event_number,UNDECIDED,event,
						(struct Event *)NULL))
					{
						*event_next=event;
						event_next= &(event->next);
						event_number++;
						no_maximum=1;
					}
				} /* do */
				while (event&&(event_number<=number_of_events));
				if (event_number>number_of_events)
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_device_event_markers.  Could not allocate event");
					destroy_Event_list(&(signal->first_event));
					return_code=0;
				}
			} break;
			case EDA_LEVEL:
			{
				present=start_search;
				while ((present<end_search)&&(*objective_value<level))
				{
					present++;
					objective_value += objective_values_step;
				}
				if (present<end_search)
				{
					/* found event */
					if (event=create_Event(present,1,UNDECIDED,(struct Event *)NULL,
						(struct Event *)NULL))
					{
						signal->first_event=event;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_device_event_markers.  Could not allocate event");
						destroy_Event_list(&(signal->first_event));
						return_code=0;
					}
				}
			} break;
			case EDA_THRESHOLD:
			{
				minimum_separation=(int)(((float)minimum_separation_milliseconds*
					(buffer->frequency))/1000.);
				/* determine the maximum */
				present=start_search;
				maximum_objective= *objective_value;
				minimum_objective=maximum_objective;
				while (present<end_search)
				{
					present++;
					objective_value += objective_values_step;
					if (maximum_objective< *objective_value)
					{
						maximum_objective= *objective_value;
					}
					if (*objective_value<minimum_objective)
					{
						minimum_objective= *objective_value;
					}
				}
				threshold=(threshold_percentage*maximum_objective+
					(100-threshold_percentage)*minimum_objective)/100;
				/* determine the events */
				present=start_search;
				objective_value=objective_values+(start_search*objective_values_step);
				if (*objective_value>=threshold)
				{
					maximum_objective= *objective_value;
					maximum=present;
					no_maximum=0;
				}
				else
				{
					maximum=start_search-1;
					no_maximum=1;
				}
				event_number=1;
				event=(struct Event *)NULL;
				event_next= &(signal->first_event);
				return_code=1;
				while (return_code&&(present<end_search))
				{
					present++;
					objective_value += objective_values_step;
					if (*objective_value>=threshold)
					{
						if (!no_maximum&&(maximum>=start_search)&&
							(minimum_separation<present-maximum))
						{
							if (event=create_Event(maximum,event_number,UNDECIDED,event,
								(struct Event *)NULL))
							{
								*event_next=event;
								event_next= &(event->next);
								event_number++;
								maximum=present;
								maximum_objective= *objective_value;
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"calculate_device_event_markers.  Could not allocate event");
								destroy_Event_list(&(signal->first_event));
								return_code=0;
							}
						}
						else
						{
							if ((maximum_objective< *objective_value)||no_maximum)
							{
								maximum=present;
								maximum_objective= *objective_value;
								no_maximum=0;
							}
						}
					}
				}
				if (return_code&&(maximum>=start_search))
				{
					if (event=create_Event(maximum,event_number,UNDECIDED,event,
						(struct Event *)NULL))
					{
						*event_next=event;
						event_next= &(event->next);
						event_number++;
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"calculate_device_event_markers.  Could not allocate event");
						destroy_Event_list(&(signal->first_event));
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
			"calculate_device_event_markers.  Invalid event detection algorithm");
				return_code=0;
			} break;
		}
	}
	else
	{
		if (buffer)
		{
			display_message(ERROR_MESSAGE,
				"calculate_device_event_markers.  Invalid argument(s).  %p %p %d %d",
				signal,buffer,buffer->number_of_samples,number_of_objective_values);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_device_event_markers.  Invalid argument(s).  %p %p %d",
				signal,buffer,number_of_objective_values);
		}
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_device_event_markers */

int draw_signal(struct FE_node *device_node,
	struct Signal_drawing_package *signal_drawing_package,struct Device *device,
	enum Signal_detail detail,int number_of_data_intervals,
	int current_data_interval,int *first_data,int *last_data,int x_pos,int y_pos,
	int width,int height,Pixmap pixel_map,int *axes_left,int *axes_top,
	int *axes_width,int *axes_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 August 2000

DESCRIPTION :
Draws the <device> signal in the <pixel_map> at the specified position
(<x_pos>, <y_pos>), size (<width>, <height>) and <detail>.
NB.  0<=current_data_interval<number_of_data_intervals 
???missing data ? times ?
???DB.  Needs more checking and more on return_code
???DB.  Change first_data and last_data to times
==============================================================================*/
{
	char **name,number_string[20],ticks_above;
	Display *display;
	enum Event_signal_status **signals_status;
	float postscript_page_bottom,postscript_page_left,postscript_page_height,
		postscript_page_width,signal_maximum,signal_minimum,signal_ref,
		*signals_maximum,*signals_minimum,**signals_values,*signal_value,*time,
		time_max,time_min,time_ref,**times,time_scale,time_tick,time_tick_max,
		time_tick_min,time_tick_width,value_float,value_scale,value_tick,
		value_tick_max,value_tick_min,value_tick_width,world_height,world_left,
		world_top,world_width,x_scale,y_scale;
	GC graphics_context;
	int ascent,descent,*highlight,direction,i,j,k,length,*number_of_points,
		number_of_segments,*number_of_signals,number_of_ticks,return_code,x_marker,
		x_max,x_min,x_ref,x_string,x_tick,x_tick_length=3,y_marker,y_max,y_min,
		y_ref,y_string,y_tick,y_tick_length=3;
	short int value_short_int,x;
	XCharStruct bounds;
	XFontStruct *font;
	XRectangle clip_rectangle;
	XSegment *segment,*segments;
	XPoint *point,*points;

	ENTER(draw_signal);
#if defined (DEBUG)
	/*???debug */
	printf("draw_signal.  %d %d %d %d %d %d\n",first_data,last_data,x_pos,y_pos,
		width,height);
#endif /* defined (DEBUG) */
	return_code=0; 
	if (user_interface&&signal_drawing_information&&(0<=current_data_interval)&&
		(current_data_interval<number_of_data_intervals)&&first_data&&last_data)
	{
		ALLOCATE(signals_status,enum Event_signal_status *,
			number_of_data_intervals);
		ALLOCATE(signals_values,float *,number_of_data_intervals);
		ALLOCATE(times,float *,number_of_data_intervals);
		ALLOCATE(name,char *,number_of_data_intervals);
		ALLOCATE(number_of_signals,int,number_of_data_intervals);
		ALLOCATE(number_of_points,int,number_of_data_intervals);
		ALLOCATE(highlight,int,number_of_data_intervals);
		ALLOCATE(signals_minimum,float,number_of_data_intervals);
		ALLOCATE(signals_maximum,float,number_of_data_intervals);
		if (signals_status&&signals_values&&times&&name&&number_of_signals&&
			number_of_points&&highlight&&signals_minimum&&signals_maximum)
		{
			for (i=0;i<number_of_data_intervals;i++)
			{
				signals_status[i]=(enum Event_signal_status *)NULL;
				signals_values[i]=(float *)NULL;
				times[i]=(float *)NULL;
				name[i]=(char *)NULL;
			}
			i=0;		
			while ((i<number_of_data_intervals)&&extract_signal_information(
				device_node,signal_drawing_package,device,0,first_data[i],last_data[i],
				number_of_signals+i,number_of_points+i,times+i,signals_values+i,
				signals_status+i,name+i,highlight+i,signals_minimum+i,
				signals_maximum+i))
			{
				i++;
			}
			if (i>=number_of_data_intervals)
			{
				/* local variables initialised OK.  Use them only from now on */
				font=signal_drawing_information->font;
				display=user_interface->display;
				graphics_context=
					(signal_drawing_information->graphics_context).axis_colour;
				/* calculate the data time range from the current interval of the first
					signal.  Offset other intervals to start at the same time as the
					current interval */
				time_min=(times[current_data_interval])[0];
				time_max=(times[current_data_interval])[number_of_points[
					current_data_interval]-1];
				for (i=0;i<number_of_data_intervals;i++)
				{
					if (i!=current_data_interval)
					{
						time=times[i];
						for (j=number_of_points[i]-1;j>=0;j--)
						{
							time[j] += time_min-time[0];
						}
						if (time[number_of_points[i]-1]>time_max)
						{
							time_max=time[number_of_points[i]-1];
						}
					}
				}
				/* determine the time tick marks */
				calculate_divisions(time_min,time_max,5,&time_tick_width,&time_tick_min,
					&time_tick_max);
				/* scale the time range to fit in the specified width on the screen */
				switch (detail)
				{
					case SIGNAL_AREA_DETAIL:
					{
						/* leave enough room on the left for displaying a 3 character
							name */
							/*???DB.  Can I do better than this ?  The axes only need to be
								calculated once when drawing all the signals */
						XTextExtents(font,"BBB",3,&direction,&ascent,&descent,&bounds);
						x_min=x_pos+bounds.lbearing+bounds.rbearing+3;
						x_max=x_pos+width-1;
					} break;
					case INTERVAL_AREA_DETAIL: case ENLARGE_AREA_DETAIL:
					{
						/* leave enough room on the left for displaying a 3 character
							name */
							/*???DB.  Can I do better than this ?  The axes only need to be
								calculated once when drawing all the signals */
						XTextExtents(font,"BBB",3,&direction,&ascent,&descent,&bounds);
						x_min=x_pos+bounds.lbearing+bounds.rbearing+3;
						x_max=x_pos+width-3;
					} break;
					case EDIT_AREA_DETAIL: case PRINTER_DETAIL:
					{
						/* leave enough room on the left for displaying a 3 character name,
							a 3 digit voltage and a tick mark */
						XTextExtents(font,"BBB -888",8,&direction,&ascent,&descent,&bounds);
						x_min=x_pos+bounds.lbearing+bounds.rbearing+6;
						x_max=x_pos+width-3;
					} break;
				}
				if (x_min>=x_max)
				{
					x_min=x_max-1;
				}
				time_scale=SCALE_FACTOR(time_max-time_min,x_max-x_min);
				/* determine the unscaled points/lines to be drawn */
				signal_minimum=signals_minimum[current_data_interval];
				signal_maximum=signals_maximum[current_data_interval];
				if (signal_maximum<signal_minimum)
				{
					/* calculate the maximum and minimum signal values */
					signal_minimum=(signals_values[0])[0];
					signal_maximum=signal_minimum;
					for (j=number_of_data_intervals-1;j>=0;j--)
					{
						signal_value=signals_values[j];
						for (i=number_of_signals[j]*number_of_points[j];i>0;i--)
						{
							value_float= *signal_value;
							if (value_float<signal_minimum)
							{
								signal_minimum=value_float;
							}
							else
							{
								if (value_float>signal_maximum)
								{
									signal_maximum=value_float;
								}
							} 
							signal_value++;
						}
					}
				}
				/* allow for constant signals */
				if (signal_minimum==signal_maximum)
				{
					signal_maximum += 1;
					signal_minimum -= 1;
				}	
				/* These max and min are  scaled and offset (by channel_gain,*/
				/* channel_offset) values*/
				if (device)
				{
					device->signal_display_maximum=signal_maximum;
					device->signal_display_minimum=signal_minimum;
				}
#if defined (UNEMAP_USE_NODES)
				else
				{
					struct FE_field_component component;
					component.number=0;				
					/* These max and min are  scaled and offset (by channel_gain,*/
					/* channel_offset) values*/
					component.field=get_Signal_drawing_package_signal_minimum_field(
						signal_drawing_package); 
					/*??JW should be copying out of and into node with MANAGER_MODIFY */
					set_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						signal_minimum);
					component.number=0;
					component.field=get_Signal_drawing_package_signal_maximum_field(
						signal_drawing_package); 
					set_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						signal_maximum);
				}
#endif /* defined (UNEMAP_USE_NODES) */
				/* determine the value tick marks */
				calculate_divisions(signal_minimum,signal_maximum,5,&value_tick_width,
					&value_tick_min,&value_tick_max);
				/* scale the value range to fit in the specified height on the screen */
				switch (detail)
				{
					case SIGNAL_AREA_DETAIL:
					{
						/* leave no space above or below */
						y_min=y_pos;
						y_max=y_pos+height-1;
					} break;
					case EDIT_AREA_DETAIL: case INTERVAL_AREA_DETAIL:
					case ENLARGE_AREA_DETAIL: case PRINTER_DETAIL:
					{
						/* leave space for putting marker values above */
						sprintf(number_string,"%.3g",time_tick_min);
						length=strlen(number_string);
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
							&bounds);
						y_min=y_pos+ascent+descent;
						y_max=y_pos+height-3;
					} break;
				}
				value_scale=SCALE_FACTOR(signal_maximum-signal_minimum,y_max-y_min);
				/* draw axes */
				/* draw the x axis */
				if ((signal_maximum<=0)||(signal_minimum>=0))
				{
					y_marker=y_max;
				}
				else
				{
					y_marker=SCALE_Y(0,signal_maximum,y_min,value_scale);
				}
				XPSDrawLine(display,pixel_map,graphics_context,x_min,y_marker,x_max,
					y_marker);
				/* draw the x axis markings - not for low detail */
				switch (detail)
				{
					case INTERVAL_AREA_DETAIL: case EDIT_AREA_DETAIL:
					case ENLARGE_AREA_DETAIL: case PRINTER_DETAIL:
					{
						/* determine if the ticks are above or below the axis */
						if (y_marker>(y_min+y_max)/2)
						{
							/* ticks above */
							ticks_above=1;
							y_tick=y_marker-y_tick_length;
#if !defined (NO_ALIGNMENT)
							SET_VERTICAL_ALIGNMENT(TOP_ALIGNMENT);
#endif /* !defined (NO_ALIGNMENT) */
						}
						else
						{
							/* ticks below */
							ticks_above=0;
							y_tick=y_marker+y_tick_length;
#if !defined (NO_ALIGNMENT)
							SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif /* !defined (NO_ALIGNMENT) */
						}
#if !defined (NO_ALIGNMENT)
						SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif /* !defined (NO_ALIGNMENT) */
						number_of_ticks=
							(int)((time_tick_max-time_tick_min)/time_tick_width+0.5);
						time_tick=time_tick_min;
						/* draw the left tick mark */
						x_marker=SCALE_X(time_tick,time_min,x_min,time_scale);
						if (ticks_above)
						{
							XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_tick,
								x_marker,y_marker);
						}
						else
						{
							XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_marker,
								x_marker,y_tick);
						}
						/* write the left tick value */
						sprintf(number_string,"%.3g",time_tick);
						length=strlen(number_string);
#if defined (NO_ALIGNMENT)
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
							&bounds);
						x_string=x_marker+(bounds.lbearing-bounds.rbearing+1)/2;
						if (x_string-bounds.lbearing<x_min)
						{
							x_string=x_min+bounds.lbearing;
						}
						if (ticks_above)
						{
							y_string=y_tick-bounds.descent;
						}
						else
						{
							y_string=y_tick+bounds.ascent;
						}
						/* write the tick value */
						XPSDrawString(display,pixel_map,graphics_context,x_string,y_string,
							number_string,length);
#else /* !defined (NO_ALIGNMENT) */
						XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
							number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
						/* move to the next tick */
						time_tick += time_tick_width;
						for (i=number_of_ticks-1;i>0;i--)
						{
							x_marker=SCALE_X(time_tick,time_min,x_min,time_scale);
							/* draw the tick mark */
							if (ticks_above)
							{
								XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_tick,
									x_marker,y_marker);
							}
							else
							{
								XPSDrawLine(display,pixel_map,graphics_context,x_marker,
									y_marker,x_marker,y_tick);
							}
							if (PRINTER_DETAIL!=detail)
							{
								/* write the tick value */
								sprintf(number_string,"%.3g",time_tick);
								length=strlen(number_string);
#if defined (NO_ALIGNMENT)
								XTextExtents(font,number_string,length,&direction,&ascent,
									&descent,&bounds);
								x_string=x_marker+(bounds.lbearing-bounds.rbearing+1)/2;
								if (x_string-bounds.lbearing<x_min)
								{
									x_string=x_min+bounds.lbearing;
								}
								if (ticks_above)
								{
									y_string=y_tick-bounds.descent;
								}
								else
								{
									y_string=y_tick+bounds.ascent;
								}
								/* write the tick value */
								XPSDrawString(display,pixel_map,graphics_context,x_string,
									y_string,number_string,length);
#else /* !defined (NO_ALIGNMENT) */
								XPSDrawString(display,pixel_map,graphics_context,x_marker,
									y_tick,number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
							}
							/* move to the next tick */
							time_tick += time_tick_width;
						}
						/* draw the right tick mark */
						x_marker=SCALE_X(time_tick,time_min,x_min,time_scale);
						if (ticks_above)
						{
							XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_tick,
								x_marker,y_marker);
						}
						else
						{
							XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_marker,
								x_marker,y_tick);
						}
						/* write the right tick value */
						sprintf(number_string,"%.3g",time_tick);
						length=strlen(number_string);
#if defined (NO_ALIGNMENT)
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
							&bounds);
						x_string=x_marker+(bounds.lbearing-bounds.rbearing+1)/2;
						if (x_string-bounds.lbearing<x_min)
						{
							x_string=x_min+bounds.lbearing;
						}
						if (ticks_above)
						{
							y_string=y_tick-bounds.descent;
						}
						else
						{
							y_string=y_tick+bounds.ascent;
						}
						XPSDrawString(display,pixel_map,graphics_context,x_string,y_string,
							number_string,length);
#else /* !defined (NO_ALIGNMENT) */
						XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
							number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
					} break;
				}
				/* draw the y axis */
				XPSDrawLine(display,pixel_map,graphics_context,x_min,y_min,x_min,y_max);
				/* draw the y axis markings - not for low detail */
				switch (detail)
				{
					case EDIT_AREA_DETAIL:
					/*???DB.  Temp ?*/case PRINTER_DETAIL:
					{
#if !defined (NO_ALIGNMENT)
						SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
						SET_VERTICAL_ALIGNMENT(CENTRE_VERTICAL_ALIGNMENT);
#endif /* !defined (NO_ALIGNMENT) */
						x_tick=x_min-x_tick_length;
						number_of_ticks=
							(int)((value_tick_max-value_tick_min)/value_tick_width+0.5);
						value_tick=value_tick_min;
						y_marker=SCALE_Y(value_tick,signal_maximum,y_min,value_scale);
						/* draw the minimum tick mark */
						XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,
							x_min,y_marker);
						/* write the minimum tick value */
						sprintf(number_string,"%.3g",value_tick);
						length=strlen(number_string);
#if defined (NO_ALIGNMENT)
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
							&bounds);
						x_string=x_tick-bounds.rbearing;
						if (x_string-bounds.lbearing<x_pos)
						{
							x_string=x_pos+bounds.lbearing;
						}
						y_string=y_marker+(bounds.ascent-bounds.descent)/2;
						if (y_string-bounds.ascent<y_min)
						{
							y_string=y_min+bounds.ascent;
						}
						if (y_string+bounds.descent>y_max)
						{
							y_string=y_max-bounds.descent;
						}
						XPSDrawString(display,pixel_map,graphics_context,x_string,y_string,
							number_string,length);
#else /* !defined (NO_ALIGNMENT) */
						XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
							number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
						/* move to the next tick */
						value_tick += value_tick_width;
						for (i=number_of_ticks-1;i>0;i--)
						{
							y_marker=SCALE_Y(value_tick,signal_maximum,y_min,value_scale);
							/* draw the tick mark */
							XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,
								x_min,y_marker);
							if (PRINTER_DETAIL!=detail)
							{
								/* write the tick value */
								sprintf(number_string,"%.3g",value_tick);
								length=strlen(number_string);
#if defined (NO_ALIGNMENT)
								XTextExtents(font,number_string,length,&direction,&ascent,
									&descent,&bounds);
								x_string=x_tick-bounds.rbearing;
								if (x_string-bounds.lbearing<x_pos)
								{
									x_string=x_pos+bounds.lbearing;
								}
								y_string=y_marker+(bounds.ascent-bounds.descent)/2;
								if (y_string-bounds.ascent<y_min)
								{
									y_string=y_min+bounds.ascent;
								}
								if (y_string+bounds.descent>y_max)
								{
									y_string=y_max-bounds.descent;
								}
								XPSDrawString(display,pixel_map,graphics_context,x_string,
									y_string,number_string,length);
#else /* !defined (NO_ALIGNMENT) */
								XPSDrawString(display,pixel_map,graphics_context,x_tick,
									y_marker,number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
							}
							/* move to the next tick */
							value_tick += value_tick_width;
						}
						/* draw the maximum tick mark */
						y_marker=SCALE_Y(value_tick,signal_maximum,y_min,value_scale);
						XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,
							x_min,y_marker);
						/* write the maximum tick value */
						sprintf(number_string,"%.3g",value_tick);
						length=strlen(number_string);
#if defined (NO_ALIGNMENT)
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
							&bounds);
						x_string=x_tick-bounds.rbearing;
						if (x_string-bounds.lbearing<x_pos)
						{
							x_string=x_pos+bounds.lbearing;
						}
						y_string=y_marker+(bounds.ascent-bounds.descent)/2;
						if (y_string-bounds.ascent<y_min)
						{
							y_string=y_min+bounds.ascent;
						}
						if (y_string+bounds.descent>y_max)
						{
							y_string=y_max-bounds.descent;
						}
						XPSDrawString(display,pixel_map,graphics_context,x_string,y_string,
							number_string,length);
#else /* !defined (NO_ALIGNMENT) */
						XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
							number_string,length);
#endif /* !defined (NO_ALIGNMENT) */
					} break;
				}
				if (name[current_data_interval])
				{
					/* draw the signal name */
					length=strlen(name[current_data_interval]);
#if defined (NO_ALIGNMENT)
					XTextExtents(font,name[current_data_interval],length,&direction,
						&ascent,&descent,&bounds);
#endif /* !defined (NO_ALIGNMENT) */
					switch (detail)
					{
						case SIGNAL_AREA_DETAIL:
						{
							/* right justified */
#if defined (NO_ALIGNMENT)
							x_string=x_min-(bounds.rbearing+2);
#else /* !defined (NO_ALIGNMENT) */
							SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
							x_string=x_min;
#endif /* !defined (NO_ALIGNMENT) */
						} break;
						case EDIT_AREA_DETAIL: case INTERVAL_AREA_DETAIL:
						case ENLARGE_AREA_DETAIL: case PRINTER_DETAIL:
						{
							/* left justified */
#if defined (NO_ALIGNMENT)
							x_string=x_pos+(bounds.lbearing+2);
#else /* !defined (NO_ALIGNMENT) */
							SET_HORIZONTAL_ALIGNMENT(LEFT_ALIGNMENT);
							x_string=x_pos;
#endif /* !defined (NO_ALIGNMENT) */
						} break;
					}
#if defined (NO_ALIGNMENT)
					y_string=(y_min+y_max+ascent-descent)/2;
#else /* !defined (NO_ALIGNMENT) */
					SET_VERTICAL_ALIGNMENT(CENTRE_VERTICAL_ALIGNMENT);
					y_string=(y_min+y_max)/2;
#endif /* !defined (NO_ALIGNMENT) */
					/* only highlight signal name in the signals window */
					if (highlight&&(SIGNAL_AREA_DETAIL==detail))
					{
						XPSDrawString(display,pixel_map,
							(signal_drawing_information->graphics_context).highlighted_colour,
							x_string,y_string,name[current_data_interval],length);
					}
					else
					{
						XPSDrawString(display,pixel_map,
							(signal_drawing_information->graphics_context).device_name_colour,
							x_string,y_string,name[current_data_interval],length);
					}
				}
				/* use full resolution for printer */
				if (PRINTER_DETAIL==detail)
				{
					x_scale=1;
					y_scale=1;
					x_scale *= (float)MAXSHORT/(float)(x_max-x_min);
					time_scale *= (float)MAXSHORT/(float)(x_max-x_min);
					y_scale *= (float)MAXSHORT/(float)(y_max-y_min);
					value_scale *= (float)MAXSHORT/(float)(y_max-y_min);
					/* set the printer transformation */
					if (get_postscript_display_transfor(&postscript_page_left,
						&postscript_page_bottom,&postscript_page_width,
						&postscript_page_height,&world_left,&world_top,&world_width,
						&world_height))
					{
						set_postscript_display_transfor(postscript_page_left,
							postscript_page_bottom,postscript_page_width,
							postscript_page_height,(float)(x_pos-x_min)*x_scale,
							(float)(y_pos-y_min)*y_scale,x_scale*(float)width,
							y_scale*(float)height);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"draw_signal.  Could not get_postscript_page_size");
					}
					x_min=0;
					x_max=MAXSHORT;
					y_min=0;
					y_max=MAXSHORT;
					time_ref=time_min;
					x_ref=0;
					signal_ref=signal_maximum;
					y_ref=0;
				}
				else
				{
					time_ref=time_min;
					x_ref=x_min;
					signal_ref=signal_maximum;
					y_ref=y_min;
				}
				*axes_left=x_min;
				*axes_width=x_max-x_min+1;
				*axes_top=y_min;
				*axes_height=y_max-y_min+1;
#if defined (DEBUG)
				/*???debug */
				if ((x_max<=x_min)||(y_max<=y_min))
				{
					printf("draw_signal.  %d %d %d %d\n",x_min,x_max,y_min,y_max);
				}
#endif /* defined (DEBUG) */
				/* set the clipping rectangle */
				clip_rectangle.x= *axes_left;
				clip_rectangle.y= *axes_top;
				clip_rectangle.width= *axes_width;
				clip_rectangle.height= *axes_height;
				/* draw the signals */
				for (k=0;k<number_of_data_intervals;k++)
				{
					for (j=0;j<number_of_signals[k];j++)
					{
						/* determine the signal colour */
						if ((highlight[k])&&(detail==SIGNAL_AREA_DETAIL))
						{
							graphics_context=(signal_drawing_information->graphics_context).
								highlighted_colour;
						}
						else
						{
							switch ((signals_status[k])[j])
							{
								case ACCEPTED:
								{
									graphics_context=(signal_drawing_information->
										graphics_context).signal_accepted_colour;
								} break;
								case REJECTED:
								{
									if ((j>0)&&(signal_drawing_information->
										number_of_signal_overlay_colours>0))
									{
										graphics_context=(signal_drawing_information->
											graphics_context).signal_overlay_colour;
										XSetForeground(display,graphics_context,
											(signal_drawing_information->signal_overlay_colours)[
											(j-1)%(signal_drawing_information->
											number_of_signal_overlay_colours)]);
									}
									else
									{
										graphics_context=(signal_drawing_information->
											graphics_context). signal_rejected_colour;
									}
								} break;
								case UNDECIDED:
								{
									graphics_context=(signal_drawing_information->
										graphics_context).signal_undecided_colour;
								} break;
							}
						}
						XPSSetClipRectangles(display,graphics_context,0,0,&clip_rectangle,1,
							Unsorted);
						if ((PRINTER_DETAIL==detail)||
							(number_of_points[k]<=4*(*axes_width)))
						{
							/* draw all the data points with line segments joining them */
							if (ALLOCATE(points,XPoint,number_of_points[k]))
							{
								/* calculate the points */
								point=points;
								time=times[k];
								signal_value=signals_values[k]+(j*number_of_points[k]);
								for (i=number_of_points[k];i>0;i--)
								{
									point->x=SCALE_X(*time,time_ref,x_ref,time_scale);
									point->y=SCALE_Y(*signal_value,signal_ref,y_ref,value_scale);
									point++;
									signal_value++;
									time++;
								}	
								/* draw */
								XPSDrawLines(display,pixel_map,graphics_context,points,
									number_of_points[k],CoordModeOrigin);
								DEALLOCATE(points);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_signal.  Insufficient memory for signal");
							}
						}
						else
						{
							/* represent the signal by a number of line segments */
							if (ALLOCATE(segments,XSegment,2*(*axes_width)-1))
							{
								segment=segments;
								time=times[k];
								signal_value=signals_values[k]+(j*number_of_points[k]);
								value_short_int=SCALE_Y(*signal_value,signal_ref,y_ref,
									value_scale);
								segment->x1=x_min;
								segment->y1=value_short_int;
								segment->x2=x_min;
								segment->y2=value_short_int;
								number_of_segments=1;
								i=number_of_points[k]-1;
								while (i>0)
								{
									time++;
									signal_value++;
									while ((i>0)&&(segment->x2==
										(x=SCALE_X(*time,time_ref,x_ref,time_scale))))
									{
										value_short_int=SCALE_Y(*signal_value,signal_ref,y_ref,
											value_scale);
										if (value_short_int<segment->y1)
										{
											segment->y1=value_short_int;
										}
										else
										{
											if (value_short_int>segment->y2)
											{
												segment->y2=value_short_int;
											}
										}
										signal_value++;
										time++;
										i--;
									}
									if (i>0)
									{
										segment[1].x1=segment->x2;
										segment++;
										number_of_segments++;
										segment->y1=value_short_int;
										segment->x2=x;
										value_short_int=SCALE_Y(*signal_value,signal_ref,y_ref,
											value_scale);
										segment->y2=value_short_int;
										segment++;
										number_of_segments++;
										segment->x1=x;
										segment->y1=value_short_int;
										segment->x2=x;
										segment->y2=value_short_int;
										i--;
									}
								}
								/* draw */			
								XDrawSegments(display,pixel_map,graphics_context,segments,
									number_of_segments);			
								DEALLOCATE(segments);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_signal.  Insufficient memory for signal");
							}
						}
						XPSSetClipMask(display,graphics_context,None);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_signal.  Could not extract signal information");
				return_code=0;
			}	
			for (i=0;i<number_of_data_intervals;i++)
			{
				DEALLOCATE(name[i]);	
				DEALLOCATE(signals_status[i]);
				DEALLOCATE(signals_values[i]);
				DEALLOCATE(times[i]);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_signal.  Could not allocate storage");
			return_code=0;
		}	
		DEALLOCATE(signals_status);
		DEALLOCATE(signals_values);
		DEALLOCATE(times);
		DEALLOCATE(name);
		DEALLOCATE(number_of_signals);
		DEALLOCATE(number_of_points);
		DEALLOCATE(highlight);
		DEALLOCATE(signals_minimum);
		DEALLOCATE(signals_maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_signal.  Invalid argument(s)");
		return_code=0;
	}	
	LEAVE;

	return (return_code);
} /* draw_signal */

int draw_datum_marker(int datum,enum Signal_detail detail,int first_data,
	int last_data,int axes_left,int axes_top,int axes_width,int axes_height,
	Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION : draws the datum_marker
==============================================================================*/
{
	int return_code,x_marker;

	ENTER(draw_datum_marker);
	if (user_interface&&signal_drawing_information)
	{
		if (INTERVAL_AREA_DETAIL!=detail)
		{
			if ((datum>=first_data)&&(datum<last_data))
			{
				x_marker=SCALE_X(datum,first_data,axes_left,
					SCALE_FACTOR(last_data-first_data,axes_width-1));
				XPSDrawLine(user_interface->display,pixel_map,
					(signal_drawing_information->graphics_context).datum_colour,x_marker,
					axes_top,x_marker,axes_top+axes_height);
				if (drawing_area_window)
				{
					XPSDrawLine(user_interface->display,drawing_area_window,
						(signal_drawing_information->graphics_context).datum_colour,
						x_marker,axes_top,x_marker,axes_top+axes_height);
				}
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_datum_marker */

int draw_potential_time_marker(int time,struct Signal_buffer *buffer,
	struct Channel *channel,int signal_index,enum Signal_detail detail,
	int first_data,int last_data,int axes_left,int axes_top,int axes_width,
	int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 1 January 1997

DESCRIPTION :
???DB.  Should <time> be <position> instead ?
==============================================================================*/
{
	char number_string[20];
	Display *display;
	float potential;
	GC graphics_context,graphics_context_text;
	int length,return_code,x_marker;
#if defined (NO_ALIGNMENT)
	int ascent,descent,direction,x_string,y_string;
	XCharStruct bounds;
	XFontStruct *font;
#endif /* defined (NO_ALIGNMENT) */

	ENTER(draw_potential_time_marker);
	if (user_interface&&signal_drawing_information)
	{
		if ((time>=first_data)&&(time<last_data))
		{
			display=user_interface->display;
#if defined (NO_ALIGNMENT)
			font=signal_drawing_information->font;
#endif /* defined (NO_ALIGNMENT) */
			graphics_context=
				(signal_drawing_information->graphics_context).potential_time_colour;
			graphics_context_text=(signal_drawing_information->graphics_context).
				potential_time_colour_text;
			x_marker=SCALE_X(time,first_data,axes_left,
				SCALE_FACTOR(last_data-first_data,axes_width-1));
			XPSDrawLine(display,pixel_map,graphics_context,x_marker,axes_top,x_marker,
				axes_top+axes_height-1);
			if (drawing_area_window)
			{
				XPSDrawLine(display,drawing_area_window,graphics_context,x_marker,
					axes_top,x_marker,axes_top+axes_height-1);
			}
			switch (detail)
			{
				case INTERVAL_AREA_DETAIL: case EDIT_AREA_DETAIL:
				{
					if (buffer&&(buffer->times))
					{
						/* write the time */
						sprintf(number_string,"%d",
							(int)((float)((buffer->times)[time])*1000./(buffer->frequency)));
						length=strlen(number_string);
#if defined (NO_ALIGNMENT)
						XTextExtents(font,number_string,length,&direction,&ascent,&descent,
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
						y_string=axes_top-descent;
/*						XPSDrawString(display,pixel_map,graphics_context,x_string,y_string,
							number_string,length);*/
						XPSDrawString(display,pixel_map,graphics_context_text,x_string,
							y_string,number_string,length);
#else
						SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
						SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
						XPSDrawString(display,pixel_map,graphics_context_text,x_marker,
							axes_top,number_string,length);
#endif
						if (drawing_area_window)
						{
							XPSDrawString(display,drawing_area_window,graphics_context_text,
#if defined (NO_ALIGNMENT)
								x_string,y_string,number_string,length);
#else
								x_marker,axes_top,number_string,length);
#endif
						}
						if ((EDIT_AREA_DETAIL==detail)&&channel&&(0<=signal_index)&&
							(signal_index<buffer->number_of_signals))
						{
							/* write the potential */
							switch (buffer->value_type)
							{
								case SHORT_INT_VALUE:
								{
									potential=((float)(buffer->signals.short_int_values)
										[time*(buffer->number_of_signals)+signal_index]-
										(channel->offset))*(channel->gain);
								} break;
								case FLOAT_VALUE:
								{
									potential=((buffer->signals.float_values)
										[time*(buffer->number_of_signals)+signal_index]-
										(channel->offset))*(channel->gain);
								} break;
							}
							sprintf(number_string,"%.3g",potential);
							length=strlen(number_string);
#if defined (NO_ALIGNMENT)
							XTextExtents(font,number_string,length,&direction,&ascent,
								&descent,&bounds);
							x_string=axes_left-bounds.rbearing-4;
							y_string=axes_top-descent;
/*							XPSDrawString(display,pixel_map,graphics_context,x_string,
								y_string,number_string,length);*/
							XPSDrawString(display,pixel_map,graphics_context_text,x_string,
								y_string,number_string,length);
#else
							SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
							XPSDrawString(display,pixel_map,graphics_context_text,axes_left,
								axes_top,number_string,length);
#endif
							if (drawing_area_window)
							{
								XPSDrawString(display,drawing_area_window,graphics_context,
#if defined (NO_ALIGNMENT)
									x_string,y_string,number_string,length);
#else
									axes_left,axes_top,number_string,length);
#endif
							}
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
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_potential_time_marker */

int draw_event_marker(struct Event *event,int current_event_number,
	int datum,int *times,float frequency,enum Signal_detail detail,int first_data,
	int last_data,int signal_min,int signal_max,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION : draws the event_marker
==============================================================================*/
{
	char number_string[20];
	Display *display;
	float x_scale,y_scale;
	GC event_graphics_context;
	GC event_graphics_context_text;
	int length,return_code,x_marker;
#if defined (NO_ALIGNMENT)
	int ascent,axes_right,descent,direction,x_string,y_string;
	XCharStruct bounds;
	XFontStruct *font;
#endif /* defined (NO_ALIGNMENT) */

	ENTER(draw_event_marker);
	USE_PARAMETER(signal_min);
	USE_PARAMETER(signal_max);
	/*???undecided accepted ? */
	if (event&&times&&((PRINTER_DETAIL!=detail)||(event->status==ACCEPTED)||
		(event->status==UNDECIDED))&&user_interface&&signal_drawing_information)
	{
		display=user_interface->display;
#if defined (NO_ALIGNMENT)
		font=signal_drawing_information->font;
#endif /* defined (NO_ALIGNMENT) */
		if (INTERVAL_AREA_DETAIL!=detail)
		{
			if ((event->time>=first_data)&&(event->time<last_data))
			{
				switch (event->status)
				{
					case ACCEPTED:
					{
						event_graphics_context=(signal_drawing_information->
							graphics_context).accepted_colour;
						event_graphics_context_text=(signal_drawing_information->
							graphics_context).accepted_colour_text;
					} break;
					case REJECTED:
					{
						event_graphics_context=(signal_drawing_information->
							graphics_context).rejected_colour;
						event_graphics_context_text=(signal_drawing_information->
							graphics_context).rejected_colour_text;
					} break;
					case UNDECIDED:
					{
						event_graphics_context=(signal_drawing_information->
							graphics_context).undecided_colour;
						event_graphics_context_text=(signal_drawing_information->
							graphics_context).undecided_colour_text;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"draw_event_marker.  Invalid event status");
						event_graphics_context=(signal_drawing_information->
							graphics_context).undecided_colour;
						event_graphics_context_text=(signal_drawing_information->
							graphics_context).undecided_colour_text;
					} break;
				}
				x_scale=1;
				y_scale=1;
				x_marker=SCALE_X(event->time,first_data,axes_left,
					SCALE_FACTOR(last_data-first_data,axes_width-1));
#if defined (DEBUG)
				/*???debug */
				if (NULL==event->previous)
				{
					printf("event->time=%d, first_data=%d, axes_left=%d\n",
						event->time,first_data,axes_left);
					printf("last_data=%d, axes_width=%d, x_marker=%d, %g\n",last_data,
						axes_width,x_marker,SCALE_FACTOR(last_data-first_data,
						axes_width-1));
				}
#endif /* defined (DEBUG) */
				XPSDrawLine(display,pixel_map,event_graphics_context,x_marker,axes_top,
					x_marker,axes_top+axes_height);
				if (drawing_area_window)
				{
					XPSDrawLine(display,drawing_area_window,event_graphics_context,
						x_marker,axes_top,x_marker,axes_top+axes_height);
				}
				if ((ENLARGE_AREA_DETAIL==detail)||(PRINTER_DETAIL==detail)||
					((EDIT_AREA_DETAIL==detail)&&(current_event_number==event->number)))
				{
					sprintf(number_string,"%d",
						(int)((float)(times[event->time]-times[datum])*1000./frequency));
					length=strlen(number_string);
#if defined (NO_ALIGNMENT)
					XTextExtents(font,number_string,length,&direction,&ascent,&descent,
						&bounds);
					x_string=x_marker+
						(int)(x_scale*(float)(bounds.lbearing-bounds.rbearing+1)/2);
					axes_right=axes_left+axes_width;
					if (x_string+(int)(x_scale*(float)bounds.rbearing)>=axes_right)
					{
						x_string=axes_right-(int)(x_scale*(float)bounds.rbearing);
					}
					if (x_string-(int)(x_scale*(float)bounds.lbearing)<axes_left)
					{
						x_string=axes_left+(int)(x_scale*(float)bounds.lbearing);
					}
					y_string=axes_top-(int)(y_scale*(float)descent);
/*					XPSDrawString(display,pixel_map,event_graphics_context,
						x_string,y_string,number_string,length);*/
 					XPSDrawString(display,pixel_map,event_graphics_context_text,
 						x_string,y_string,number_string,length);
#else
					SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
					SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
					XPSDrawString(display,pixel_map,event_graphics_context_text,x_marker,
						axes_top,number_string,length);
#endif
					if (drawing_area_window)
					{
						XPSDrawString(display,drawing_area_window,event_graphics_context,
#if defined (NO_ALIGNMENT)
 							x_string,y_string,
#else
							x_marker,axes_top,
#endif
							number_string,length);
					}
				}
			}
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_event_marker */

int draw_device_markers(struct Device *device,int first_data,int last_data,
	int datum,char draw_datum,int potential_time,char draw_potential_time,
	enum Signal_detail detail,int current_event_number,int axes_left,int axes_top,
	int axes_width,int axes_height,Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION : Draws the markers for the device
==============================================================================*/
{
	int return_code,signal_index,*times;
	float frequency;
	struct Event *event;	
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(draw_device_markers);
	times=(int *)NULL;
	event=(struct Event *)NULL;	
	signal=(struct Signal *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	return_code=0;
	if (device&&(buffer=get_Device_signal_buffer(device))&&(times=buffer->times))
	{
		if (signal=device->signal)
		{
			signal_index=signal->index;
		}
		else
		{
			signal_index= -1;
		}
		frequency=buffer->frequency;
		/* draw datum */
		if (draw_datum)
		{
			draw_datum_marker(datum,detail,first_data,last_data,axes_left,axes_top,
				axes_width,axes_height,drawing_area_window,pixel_map,
				signal_drawing_information,user_interface);
		}
		/* draw potential time */
		if (draw_potential_time)
		{
			draw_potential_time_marker(potential_time,buffer,device->channel,
				signal_index,detail,first_data,last_data,axes_left,axes_top,
				axes_width,axes_height,drawing_area_window,pixel_map,
				signal_drawing_information,user_interface);
		}
		if (signal&&(event=signal->first_event))
		{
			switch (detail)
			{
				case SIGNAL_AREA_DETAIL: case ENLARGE_AREA_DETAIL:
						/*???temp ?*/case PRINTER_DETAIL: case EDIT_AREA_DETAIL:
				{
					/* draw all event markers */
					while (event)
					{
						draw_event_marker(event,current_event_number,datum,times,
							frequency,detail,first_data,last_data,device->signal_display_minimum,
							device->signal_display_maximum,axes_left,axes_top,axes_width,
							axes_height,drawing_area_window,pixel_map,
							signal_drawing_information,user_interface);
						event=event->next;
					}
				} break;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_device_markers.  Invalid device");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_device_markers */

struct Signal_drawing_information *create_Signal_drawing_information(
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 12 April 2001

DESCRIPTION : creates the Signal_drawing_information
==============================================================================*/
{
	char *overlay_colours_string,temp_char,*temp_string_1,*temp_string_2;
	Display *display;
	int depth,number_of_overlay_colours;
	Pixel *overlay_colours;
	Pixmap depth_screen_drawable;
	struct Signal_drawing_information *signal_drawing_information;
#define XmNacceptedColour "acceptedColour"
#define XmCAcceptedColour "AcceptedColour"
#define XmNaxisColour "axisColour"
#define XmCAxisColour "AxisColour"
#define XmNcardiacIntervalColour "cardiacIntervalColour"
#define XmCCardiacIntervalColour "CardiacIntervalColour"
#define XmNdatumColour "datumColour"
#define XmCDatumColour "DatumColour"
#define XmNdeviceNameColour "deviceNameColour"
#define XmCDeviceNameColour "DeviceNameColour"
#define XmNeimagingEventColour "eimagingEventColour"
#define XmCEimagingEventColour "EimagingEventColour"
#define XmNdrawingBackgroundColour "drawingBackgroundColour"
#define XmCDrawingBackgroundColour "DrawingBackgroundColour"
#define XmNhighlightedColour "highlightedColour"
#define XmCHighlightedColour "HighlightedColour"
#define XmNintervalBoxColour "intervalBoxColour"
#define XmCIntervalBoxColour "IntervalBoxColour"
#define XmNoverlaySignalColours "overlaySignalColours"
#define XmCOverlaySignalColours "OverlaySignalColours"
#define XmNpotentialTimeColour "potentialTimeColour"
#define XmCPotentialTimeColour "PotentialTimeColour"
#define XmNpWaveColour "pWaveColour"
#define XmCPWaveColour "PWaveColour"
#define XmNqrsWaveColour "qrsWaveColour"
#define XmCQRSWaveColour "QRSWaveColour"
#define XmNrejectedColour "rejectedColour"
#define XmCRejectedColour "RejectedColour"
#define XmNscalingSignalColour "scalingSignalColour"
#define XmCScalingSignalColour "ScalingSignalColour"
#define XmNsignalAcceptedColour "signalAcceptedColour"
#define XmCSignalAcceptedColour "SignalAcceptedColour"
#define XmNsignalRejectedColour "signalRejectedColour"
#define XmCSignalRejectedColour "SignalRejectedColour"
#define XmNsignalUndecidedColour "signalUndecidedColour"
#define XmCSignalUndecidedColour "SignalUndecidedColour"
#define XmNtWaveColour "tWaveColour"
#define XmCTWaveColour "TWaveColour"
#define XmNundecidedColour "undecidedColour"
#define XmCUndecidedColour "UndecidedColour"
#define XmNunhighlightedColour "unhighlightedColour"
#define XmCUnhighlightedColour "UnhighlightedColour"

	static XtResource resources[]=
	{
		{
			XmNacceptedColour,
			XmCAcceptedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,accepted_colour),
			XmRString,
			"cyan"
		},
		{
			XmNaxisColour,
			XmCAxisColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,axis_colour),
			XmRString,
			"blue"
		},
		{
			XmNcardiacIntervalColour,
			XmCCardiacIntervalColour,		
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,cardiac_interval_colour),
			XmRString,
			"red"
		},
		{
			XmNeimagingEventColour,
			XmCEimagingEventColour,		
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,eimaging_event_colour),
			XmRString,
			"white"
		},
		{
			XmNdatumColour,
			XmCDatumColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,datum_colour),
			XmRString,
			"white"
		},
		{
			XmNdeviceNameColour,
			XmCDeviceNameColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,device_name_colour),
			XmRString,
			"yellow"
		},
		{
			XmNdrawingBackgroundColour,
			XmCDrawingBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,background_drawing_colour),
			XmRString,
			"lightgray"
		},
		{
			XmNhighlightedColour,
			XmCHighlightedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,highlighted_colour),
			XmRString,
			"white"
		},
		{
			XmNintervalBoxColour,
			XmCIntervalBoxColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,interval_box_colour),
			XmRString,
			"yellow"
		},
		{
			XmNpotentialTimeColour,
			XmCPotentialTimeColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,potential_time_colour),
			XmRString,
			"red"
		},
		{
			XmNpWaveColour,
			XmCPWaveColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,pwave_colour),
			XmRString,
			"white"
		},
		{
			XmNqrsWaveColour,
			XmCQRSWaveColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,qrswave_colour),
			XmRString,
			"cyan"
		},
		{
			XmNrejectedColour,
			XmCRejectedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,rejected_colour),
			XmRString,
			"orange"
		},
		{
			XmNscalingSignalColour,
			XmCScalingSignalColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,scaling_signal_colour),
			XmRString,
			"yellow"
		},
		{
			XmNsignalAcceptedColour,
			XmCSignalAcceptedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,signal_accepted_colour),
			XmRString,
			"cyan"
		},
		{
			XmNsignalRejectedColour,
			XmCSignalRejectedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,signal_rejected_colour),
			XmRString,
			"orange"
		},
		{
			XmNsignalUndecidedColour,
			XmCSignalUndecidedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,signal_undecided_colour),
			XmRString,
			"green"
		},
		{
			XmNtWaveColour,
			XmCTWaveColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,twave_colour),
			XmRString,
			"green"
		},
		{
			XmNundecidedColour,
			XmCUndecidedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,undecided_colour),
			XmRString,
			"green"
		},
		{
			XmNunhighlightedColour,
			XmCUnhighlightedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Signal_drawing_information_settings,unhighlighted_colour),
			XmRString,
			"red"
		}
	};
	static XtResource overlay_resources[]=
	{
		{
			XmNoverlaySignalColours,
			XmCOverlaySignalColours,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			"yellow,blue"
		},
	};
	unsigned long mask;
	XGCValues values;
	XrmValue from,to;

	ENTER(create_Signal_drawing_information);
	/* check arguments */
	if (user_interface)
	{
		if (ALLOCATE(signal_drawing_information,struct Signal_drawing_information,
			1))
		{
			signal_drawing_information->user_interface=user_interface;
			signal_drawing_information->font=user_interface->normal_font;
			/* retrieve_settings */
			XtVaGetApplicationResources(user_interface->application_shell,
				signal_drawing_information,resources,XtNumber(resources),NULL);
			/*???DB.  Would like to make a type converter, but there isn't a pixel
				array type */
			overlay_colours_string=(char *)NULL;
			XtVaGetApplicationResources(user_interface->application_shell,
				&overlay_colours_string,overlay_resources,XtNumber(overlay_resources),
				NULL);
			/* NB.  XtVaGetApplicationResources does not allocate memory for
				overlay_colours_string, so it does not need to be free'd */
			number_of_overlay_colours=0;
			overlay_colours=(Pixel *)NULL;
			/*???DB.  Under Linux the overlay_colours_string is in some sort of read
				only memory and can't be modified */
			if (overlay_colours_string)
			{
				overlay_colours_string=duplicate_string(overlay_colours_string);
			}
			if (temp_string_1=overlay_colours_string)
			{
				while (*temp_string_1)
				{
					/* skip leading spaces and commas */
					temp_string_1 += strspn(temp_string_1," ,");
					if (*temp_string_1)
					{
						number_of_overlay_colours++;
						/* find next string or comma */
						temp_string_1 += strcspn(temp_string_1," ,");
					}
				}
			}
			if ((0<number_of_overlay_colours)&&ALLOCATE(overlay_colours,Pixel,
				number_of_overlay_colours))
			{
				number_of_overlay_colours=0;
				temp_string_1=overlay_colours_string;
				while (*temp_string_1)
				{
					/* skip leading spaces and commas */
					temp_string_1 += strspn(temp_string_1," ,");
					if (*temp_string_1)
					{
						/* find next string or comma */
						temp_string_2=temp_string_1+strcspn(temp_string_1," ,");
						temp_char= *temp_string_2;
						*temp_string_2='\0';
						from.addr=temp_string_1;
						from.size=strlen(temp_string_1);
						to.addr=(XPointer)(overlay_colours+number_of_overlay_colours);
						to.size=sizeof(overlay_colours[number_of_overlay_colours]);
						XtConvertAndStore(user_interface->application_shell,XtRString,
							&from,XtRPixel,&to);
						*temp_string_2=temp_char;
						temp_string_1=temp_string_2;
						number_of_overlay_colours++;
					}
				}
			}
			DEALLOCATE(overlay_colours_string);
			signal_drawing_information->number_of_signal_overlay_colours=
				number_of_overlay_colours;
			signal_drawing_information->signal_overlay_colours=overlay_colours;
			/* create the graphics contexts */
			display=user_interface->display;
			/* the drawable has to have the correct depth and screen */
			XtVaGetValues(user_interface->application_shell,XmNdepth,&depth,NULL);
			depth_screen_drawable=XCreatePixmap(user_interface->display,
				XRootWindow(user_interface->display,
				XDefaultScreen(user_interface->display)),1,1,depth);
			mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
			values.font=user_interface->normal_font->fid;
			values.line_style=LineSolid;
			values.background=signal_drawing_information->background_drawing_colour;
			values.foreground=signal_drawing_information->axis_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).axis_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->device_name_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).device_name_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->highlighted_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).highlighted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->unhighlighted_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).unhighlighted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			/*???DB.  When using an RS/6000 model with the Color Graphics Display
				Adapter (#2770), GCs stop writing text to the pixel map after they've
				been used for drawing lines to the window.  So I duplicate them */
			values.foreground=signal_drawing_information->accepted_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).accepted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(signal_drawing_information->graphics_context).accepted_colour_text=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->rejected_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).rejected_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(signal_drawing_information->graphics_context).rejected_colour_text=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->undecided_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).undecided_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(signal_drawing_information->graphics_context).undecided_colour_text=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->potential_time_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).potential_time_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(signal_drawing_information->graphics_context).
				potential_time_colour_text=XCreateGC(display,depth_screen_drawable,mask,
				&values);
			values.foreground=signal_drawing_information->signal_accepted_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).signal_accepted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->signal_rejected_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).signal_rejected_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			if (signal_drawing_information->signal_overlay_colours)
			{
				values.foreground=
					(signal_drawing_information->signal_overlay_colours)[0];
			}
			(signal_drawing_information->graphics_context).signal_overlay_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->signal_undecided_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).signal_undecided_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->datum_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).datum_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->pwave_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).pwave_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->qrswave_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).qrswave_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->twave_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).twave_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->cardiac_interval_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).cardiac_interval_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->eimaging_event_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).eimaging_event_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->background_drawing_colour;
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).background_drawing_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->interval_box_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).interval_box_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->scaling_signal_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).scaling_signal_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=signal_drawing_information->highlighted_colour^
				signal_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(signal_drawing_information->graphics_context).highlighted_box_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).spectrum=XCreateGC(
				display,depth_screen_drawable,GCLineStyle|GCBackground|GCFunction,
				&values);
			values.function=GXcopy;
			(signal_drawing_information->graphics_context).copy=XCreateGC(display,
				depth_screen_drawable,GCFunction,&values);
			XFreePixmap(user_interface->display,depth_screen_drawable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Signal_drawing_information.  Could not allocate memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Signal_drawing_information.  Missing user_interface");
		signal_drawing_information=(struct Signal_drawing_information *)NULL;
	}
	LEAVE;

	return (signal_drawing_information);
} /* create_Signal_drawing_information */

int destroy_Signal_drawing_information(
	struct Signal_drawing_information **signal_drawing_information_address)
/*******************************************************************************
LAST MODIFIED : 18 June 2001

DESCRIPTION : destroys the Signal_drawing_information
==============================================================================*/
{
	Display *display;
	int return_code;
	struct Signal_drawing_information *signal_drawing_information;

	ENTER(destroy_Signal_drawing_information);
	if (signal_drawing_information_address&&
		(signal_drawing_information= *signal_drawing_information_address)&&
		(signal_drawing_information->user_interface))
	{
    /* DPN 18 June 2001 - Need to check things */
    if (display=signal_drawing_information->user_interface->display)
    {
      if ((signal_drawing_information->graphics_context).accepted_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          accepted_colour);
      }
      if ((signal_drawing_information->graphics_context).accepted_colour_text)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          accepted_colour_text);
      }
      if ((signal_drawing_information->graphics_context).axis_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          axis_colour);
      }
      if ((signal_drawing_information->graphics_context).
        background_drawing_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          background_drawing_colour);
      }
      if ((signal_drawing_information->graphics_context).copy)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).copy);
      }
      if ((signal_drawing_information->graphics_context).datum_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          datum_colour);
      }
      if ((signal_drawing_information->graphics_context).pwave_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          pwave_colour);
      }
      if ((signal_drawing_information->graphics_context).qrswave_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          qrswave_colour);
      }
      if ((signal_drawing_information->graphics_context).twave_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          twave_colour);
      }
      if ((signal_drawing_information->graphics_context).cardiac_interval_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          cardiac_interval_colour);
      }
			if ((signal_drawing_information->graphics_context).eimaging_event_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          eimaging_event_colour);
      }
      if ((signal_drawing_information->graphics_context).device_name_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          device_name_colour);
      }
      if ((signal_drawing_information->graphics_context).
        highlighted_box_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          highlighted_box_colour);
      }
      if ((signal_drawing_information->graphics_context).
        highlighted_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          highlighted_colour);
      }
      if ((signal_drawing_information->graphics_context).
        interval_box_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          interval_box_colour);
      }
      if ((signal_drawing_information->graphics_context).
        potential_time_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          potential_time_colour);
      }
      if ((signal_drawing_information->graphics_context).
        potential_time_colour_text)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          potential_time_colour_text);
      }
      if ((signal_drawing_information->graphics_context).
        rejected_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          rejected_colour);
      }
      if ((signal_drawing_information->graphics_context).
        rejected_colour_text)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          rejected_colour_text);
      }
      if ((signal_drawing_information->graphics_context).
        scaling_signal_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          scaling_signal_colour);
      }
      if ((signal_drawing_information->graphics_context).
        signal_accepted_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          signal_accepted_colour);
      }
      if ((signal_drawing_information->graphics_context).
        signal_rejected_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          signal_rejected_colour);
      }
      if ((signal_drawing_information->graphics_context).
        signal_overlay_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          signal_overlay_colour);
      }
      if ((signal_drawing_information->graphics_context).
        signal_undecided_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          signal_undecided_colour);
      }
      if ((signal_drawing_information->graphics_context).spectrum)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).spectrum);
      }
      if ((signal_drawing_information->graphics_context).
        undecided_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          undecided_colour);
      }
      if ((signal_drawing_information->graphics_context).
        undecided_colour_text)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          undecided_colour_text);
      }
      if ((signal_drawing_information->graphics_context).
        unhighlighted_colour)
      {
        XFreeGC(display,(signal_drawing_information->graphics_context).
          unhighlighted_colour);
      }
      /* DPN 10 July 2001 - This one is done above */
      /* if ((signal_drawing_information->graphics_context).
         accepted_colour)
         {
         XFreeGC(display,(signal_drawing_information->graphics_context).
         accepted_colour);
         }*/
    }
    if (signal_drawing_information->signal_overlay_colours)
    {
      DEALLOCATE(signal_drawing_information->signal_overlay_colours);
    }
		DEALLOCATE(*signal_drawing_information_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Signal_drawing_information */

int analysis_write_signal_file(char *file_name,struct Rig *rig,int datum,
	int potential_time,int start_search_interval,int end_search_interval,
	char calculate_events,enum Event_detection_algorithm detection,
	int analysis_event_number,int analysis_number_of_events,
	int minimum_separation,int threshold,enum Datum_type datum_type,
	enum Edit_order edit_order,enum Signal_order signal_order,float level,
	int average_width)
/*******************************************************************************
LAST MODIFIED : 19 November 2000

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	FILE *output_file;
	float signal_maximum,signal_minimum;
	int buffer_end,buffer_start,event_number,event_time,i,new_datum,
		new_end_search_interval,new_potential_time,new_start_search_interval,
		number_of_events,return_code,temp_int;
	struct Device **device;
	struct Event *event,*start_event;
	struct Signal_buffer *buffer;

	ENTER(analysis_write_signal_file);
	output_file=(FILE *)NULL;
	/* check the arguments */
	if (rig)
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
					new_datum=datum-buffer_start;
					new_potential_time=potential_time-buffer_start;
					new_start_search_interval=start_search_interval-buffer_start;
					new_end_search_interval=end_search_interval-buffer_start;
					if ((1==BINARY_FILE_WRITE((char *)&(new_datum),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&calculate_events,sizeof(char),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&detection,
						sizeof(enum Event_detection_algorithm),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&analysis_event_number,sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&analysis_number_of_events,
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&new_potential_time,sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&minimum_separation,sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&threshold,sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&datum_type,sizeof(enum Datum_type),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&edit_order,sizeof(enum Edit_order),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&signal_order,
						sizeof(enum Signal_order),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_start_search_interval),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_end_search_interval),
						sizeof(int),1,output_file)))
					{
						if (EDA_LEVEL==detection)
						{
							/*???DB.  In case need to change the format later */
							temp_int=1;
							if (!((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
								output_file))&&(1==BINARY_FILE_WRITE((char *)&level,
								sizeof(float),1,output_file))&&(1==BINARY_FILE_WRITE(
								(char *)&average_width,sizeof(int),1,output_file))))
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
							/* if no (*device)->channel, a linear comb auxiliary device.  Do
								nothing */
							if (((*device)->channel)&&((*device)->signal))
							{
								signal_minimum=(*device)->signal_display_minimum;
								signal_maximum=(*device)->signal_display_maximum;
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
			if (!return_code)
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
