/*******************************************************************************
FILE : analysis_calculate.c

LAST MODIFIED : 22 July 2004

DESCRIPTION :
The routines for calculating event times.  Separated out and without structures
so that cm can use them
==============================================================================*/

#include <stddef.h>
#include "general/debug.h"
#include "unemap/analysis_calculate.h"
#include "unemap/drawing_2d.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/
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
int calculate_time_series_objective(enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,int average_width,float gain,
	float offset,int number_of_objective_values,int objective_values_step,
	float *objective_values)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the time
series initially stored in <objective_values>.  Storing the values in the array
(<objective_values> every <objective_values_step>) provided.  <objective_values>
is assumed to have storage for at least <number_of_objective_values>*
<objective_values_step> values.

Split from function calculate_device_objective on 19 February 2002
==============================================================================*/
{
	float average_after,*average_after_value,average_before,first_value,
		last_value,objective_maximum,objective_minimum,*objective_value,*save_value,
		signal_maximum,signal_minimum,*save_values,scale,temp_value;
	int i,return_code;

	ENTER(calculate_time_series_objective);
	return_code=0;
	if ((0<number_of_objective_values)&&objective_values&&
		(0<objective_values_step))
	{
		if (ALLOCATE(save_values,float,average_width))
		{
			objective_value=objective_values;
			temp_value=gain*((*objective_value)-offset);
			signal_maximum=temp_value;
			signal_minimum=signal_maximum;
			for (i=number_of_objective_values-1;i>0;i--)
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
						/* VALUEs only */
						switch (objective)
						{
							case ABSOLUTE_VALUE:
							{
								objective_value=objective_values;
								for (i=number_of_objective_values;i>0;i--)
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
								for (i=number_of_objective_values;i>0;i--)
								{
									*objective_value= -(*objective_value);
									objective_value += objective_values_step;
								}
							} break;
						}
						/* take moving average */
						return_code=calculate_moving_average(objective_values,
							number_of_objective_values,objective_values_step,average_width);
					}
					else
					{
						/* SLOPEs only */
						first_value=objective_values[0];
						last_value=objective_values[(number_of_objective_values-1)*
							objective_values_step];
						save_value=save_values;
						for (i=average_width;i>0;i--)
						{
							*save_value=first_value;
							save_value++;
						}
						save_value=save_values;
						average_before=first_value*(float)average_width;
						average_after=0;
						if (average_width<number_of_objective_values)
						{
							for (i=average_width;i>0;i--)
							{
								objective_value += objective_values_step;
								average_after += *objective_value;
							}
							objective_value=objective_values;
							average_after_value=objective_value+
								(average_width*objective_values_step);
							for (i=number_of_objective_values-average_width-1;i>0;i--)
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
							for (i=average_width;i>0;i--)
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
							*objective_value=average_after-average_before;
						}
						else /*average width >= number_of_sample*/
						{
							for (i=number_of_objective_values-1;i>0;i--)
							{
								objective_value += objective_values_step;
								average_after += *objective_value;
							}
							average_after += last_value*
								(float)(average_width-(number_of_objective_values-1));
							objective_value=objective_values;
							for (i=number_of_objective_values;i>0;i--)
							{
								temp_value=average_after-average_before;
								average_before += (*objective_value)-first_value;
								*objective_value=temp_value;
								objective_value += objective_values_step;
								average_after += last_value-(*objective_value);
							}
						}
						/* max/min calculations */
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								objective_value=objective_values;
								objective_minimum=0;
								objective_maximum=0;
								for (i=number_of_objective_values;i>0;i--)
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
								for (i=number_of_objective_values;i>0;i--)
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
								for (i=number_of_objective_values;i>0;i--)
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
							for (i=number_of_objective_values;i>0;i--)
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
							for (i=number_of_objective_values;i>0;i--)
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
					for (i=number_of_objective_values;i>0;i--)
					{
						if (*objective_value<0)
						{
							*objective_value= -(*objective_value);
						}
						objective_value += objective_values_step;
					}
					/* take moving average */
					return_code=calculate_moving_average(objective_values,
						number_of_objective_values,objective_values_step,average_width);
				} break;
			}
			DEALLOCATE(save_values);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_time_series_objective.  Could not allocate save_values");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_time_series_objective.  Invalid argument(s).  %d %p %d",
			number_of_objective_values,objective_values,objective_values_step);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_time_series_objective */

int calculate_time_series_event_markers(int start_search,int end_search,
	enum Event_detection_algorithm detection,float *objective_values,
	int number_of_objective_values,int objective_values_step,
	int number_of_interval_events,int *interval_divisions,
	int threshold_percentage,int minimum_separation_milliseconds,float level,
	float frequency,int *number_of_events_address,int **events_address)
/*******************************************************************************
LAST MODIFIED : 22 July 2004

DESCRIPTION :
Calculate the event times for a signal (<objective_values>) based upon the the
<start_search> and <end_search> times, and the <detection> algorithm.
<objective_values> is assumed to have storage for at least
<number_of_objective_values>*<objective_values_step> values.  Allocates storage
for <*event_address>.

<interval_divisions> are the <number_of_interval_events>-1 divisions between the
sub-intervals.  If missing then sub-intervals are all the same length.

Split from the function calculate_device_event_markers on 19 February 2002
==============================================================================*/
{
	float maximum_objective,minimum_objective,*objective_value,threshold;
	int event_number,*events,*events_temp,interval_end,maximum,minimum_separation,
		no_maximum,number_of_events,present,return_code;

	ENTER(calculate_time_series_event_markers);
	return_code=0;
	/* check arguments */
	if (number_of_events_address&&events_address&&(0<=start_search)&&
		(start_search<=end_search)&&(end_search<number_of_objective_values)&&
		(((EDA_INTERVAL==detection)&&(0<number_of_interval_events))||
		((EDA_LEVEL==detection)&&(0<=level))||
		((EDA_THRESHOLD==detection)&&(0<=threshold_percentage)&&(0<frequency)&&
		(threshold_percentage<=100)&&(0<minimum_separation_milliseconds)))&&
		objective_values&&(0<objective_values_step)&&(0<number_of_objective_values))
	{
		return_code=1;
		objective_value=objective_values+(start_search*objective_values_step);
		number_of_events=0;
		events=(int *)NULL;
		switch (detection)
		{
			case EDA_INTERVAL:
			{
				present=start_search;
				event_number=1;
				no_maximum=0;
				do
				{
					maximum_objective= *objective_value;
					maximum=present;
					if (interval_divisions)
					{
						if (event_number<number_of_interval_events)
						{
							interval_end=interval_divisions[event_number-1];
							if (interval_end>end_search)
							{
								interval_end=end_search;
							}
						}
						else
						{
							interval_end=end_search;
						}
					}
					else
					{
						interval_end=SCALE_X(event_number,0,start_search,
							SCALE_FACTOR(number_of_interval_events,end_search-start_search));
					}
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
					if (REALLOCATE(events_temp,events,int,number_of_events+1))
					{
						events=events_temp;
						events[number_of_events]=maximum;
						number_of_events++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_time_series_event_markers.  "
							"Could not reallocate events");
						DEALLOCATE(events);
						return_code=0;
					}
					event_number++;
				}
				while (return_code&&(event_number<=number_of_interval_events));
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
					if (REALLOCATE(events_temp,events,int,number_of_events+1))
					{
						events=events_temp;
						*(events+number_of_events)=present;
						(number_of_events)++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_time_series_event_markers.  "
							"Could not reallocate events");
						DEALLOCATE(events);
						return_code=0;
					}
				}
			} break;
			case EDA_THRESHOLD:
			{
				minimum_separation=(int)(((float)minimum_separation_milliseconds*
					frequency)/1000.);
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
				while (return_code&&(present<end_search))
				{
					present++;
					objective_value += objective_values_step;
					if (*objective_value>=threshold)
					{
						if (!no_maximum&&(maximum>=start_search)&&
							(minimum_separation<present-maximum))
						{
							if (REALLOCATE(events_temp,events,int,number_of_events+1))
							{
								events=events_temp;
								events[number_of_events]=maximum;
								number_of_events++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calculate_time_series_event_markers.  "
									"Could not reallocate events");
								DEALLOCATE(events);
								return_code=0;
							}
							event_number++;
							maximum=present;
							maximum_objective= *objective_value;
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
					if (REALLOCATE(events_temp,events,int,number_of_events+1))
					{
						events=events_temp;
						events[number_of_events]=maximum;
						number_of_events++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_time_series_event_markers.  "
							"Could not reallocate events");
						DEALLOCATE(events);
						return_code=0;
					}
					event_number++;
				}
			} break;
			default:
			{
				return_code=0;
				display_message(ERROR_MESSAGE,
					"Invalid event detection algorithm");
			} break;
		}
		if (return_code)
		{
			*events_address=events;
			*number_of_events_address=number_of_events;
		}
		else
		{
			DEALLOCATE(events);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_time_series_event_markers.  "
			"Invalid argument(s).  %p %p %d %d %d %d %g %d %g %d %p %d %d",
			number_of_events_address,events_address,start_search,end_search,detection,
			number_of_interval_events,level,threshold_percentage,frequency,
			minimum_separation_milliseconds,objective_values,objective_values_step,
			number_of_objective_values);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_time_series_event_markers */
