/*******************************************************************************
FILE : analysis.c

LAST MODIFIED : 14 December 1999

DESCRIPTION :
==============================================================================*/

#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "general/debug.h"
#include "general/postscript.h"
#include "general/mystring.h"
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

/*
Global functions
----------------
*/
int calculate_device_event_markers(struct Device *device,int start_search,
	int end_search,enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,int number_of_events,
	int threshold_percentage,int minimum_separation_milliseconds,float level,
	int gradient_average_width)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Calculate the positions of the event markers for a signal/<device> based upon
the the start and end times, the number of events and the search algorithm.
==============================================================================*/
{
	int average_length,end,event_number,i,interval_end,maximum,minimum_separation,
		number_of_samples,number_of_signals,offset,present,return_code,start;
	struct Event *event,**event_next;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(calculate_device_event_markers);
	if ((0<=start_search)&&(start_search<=end_search)&&device&&
		(signal=device->signal)&&(buffer=signal->buffer)&&
		(end_search<(number_of_samples=buffer->number_of_samples))&&
		(((SHORT_INT_VALUE==buffer->value_type)&&
		(buffer->signals.short_int_values))||((FLOAT_VALUE==buffer->value_type)&&
		(buffer->signals.float_values)))&&(((EDA_INTERVAL==detection)&&
		(0<number_of_events))||((EDA_LEVEL==detection)&&(0<=level))||
		((EDA_THRESHOLD==detection)&&(0<=threshold_percentage)&&
		(threshold_percentage<=100)&&(0<minimum_separation_milliseconds))))
	{
		/* free the previous events */
		destroy_Event_list(&(signal->first_event));
		number_of_signals=signal->buffer->number_of_signals;
		switch (buffer->value_type)
		{
			case SHORT_INT_VALUE:
			{
				int average_after,average_before,maximum_slope,minimum_slope,
					no_maximum,slope,threshold;
				short int first_value,last_value,*signals,*value,*value_after,
					*value_before;

				signals=buffer->signals.short_int_values;
				switch (detection)
				{
					case EDA_INTERVAL:
					{
						average_length=gradient_average_width;
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						offset=average_length*number_of_signals;
						event=(struct Event *)NULL;
						event_next= &(signal->first_event);
						event_number=1;
						no_maximum=0;
						do
						{
							maximum_slope=slope;
							maximum=present;
							interval_end=SCALE_X(event_number,0,start_search,
								SCALE_FACTOR(number_of_events,end_search-start_search));
							while ((present<interval_end)&&(present<start))
							{
								present++;
								average_before += *value_before-first_value;
								value_before += number_of_signals;
								if (present>end)
								{
									average_after += last_value- *value_before;
								}
								else
								{
									value_after += number_of_signals;
									average_after += *value_after- *(value_after-offset);
								}
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
									maximum=present;
									no_maximum=0;
								}
							}
							while ((present<interval_end)&&(present<end))
							{
								present++;
								average_before += *value_before- *(value_before-offset);
								value_before += number_of_signals;
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
									maximum=present;
									no_maximum=0;
								}
							}
							while (present<interval_end)
							{
								present++;
								average_before += *value_before- *(value_before-offset);
								value_before += number_of_signals;
								average_after += last_value- *value_before;
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
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
						}
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
						/*???DB.  Could average signal over gradient_average_width */
						value=signals+(start_search*number_of_signals+(signal->index));
						average_length=gradient_average_width;
						present=start_search;
						while ((present<end_search)&&((float)(*value)<level)&&
							(-(float)(*value)<level))
						{
							value += number_of_signals;
							present++;
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
						average_length=gradient_average_width;
						offset=average_length*number_of_signals;
						minimum_separation=(int)(((float)minimum_separation_milliseconds*
							(buffer->frequency))/1000.);
						/* determine the maximum slope */
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						maximum_slope=slope;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								minimum_slope=0;
							} break;
							default:
							{
								minimum_slope=maximum_slope;
							} break;
						}
						while ((present<end_search)&&(present<start))
						{
							present++;
							average_before += *value_before-first_value;
							value_before += number_of_signals;
							if (present>end)
							{
								average_after += last_value- *value_before;
							}
							else
							{
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
							}
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						while ((present<end_search)&&(present<end))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							value_after += number_of_signals;
							average_after += *value_after- *(value_after-offset);
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						while (present<end_search)
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							average_after += last_value- *value_before;
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						threshold=(threshold_percentage*maximum_slope+
							(100-threshold_percentage)*minimum_slope)/100;
						/* determine the events */
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						if (slope>=threshold)
						{
							maximum_slope=slope;
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
						while (return_code&&(present<end_search)&&(present<start))
						{
							present++;
							average_before += *value_before-first_value;
							value_before += number_of_signals;
							if (present>end)
							{
								average_after += last_value- *value_before;
							}
							else
							{
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
							}
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
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
										maximum_slope=slope;
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
									if ((maximum_slope<slope)||no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
										no_maximum=0;
									}
								}
							}
						}
						while (return_code&&(present<end_search)&&(present<end))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							value_after += number_of_signals;
							average_after += *value_after- *(value_after-offset);
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
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
										maximum_slope=slope;
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
									if ((maximum_slope<slope)||no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
										no_maximum=0;
									}
								}
							}
						}
						while (return_code&&(present<end_search))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							average_after += last_value- *value_before;
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
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
										maximum_slope=slope;
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
									if ((maximum_slope<slope)||no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
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
			} break;
			case FLOAT_VALUE:
			{
				float average_after,average_before,maximum_slope,minimum_slope,slope,
					threshold;
				float first_value,last_value,*signals,*value,*value_after,
					*value_before;
				int no_maximum;

				signals=buffer->signals.float_values;
				switch (detection)
				{
					case EDA_INTERVAL:
					{
						average_length=gradient_average_width;
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						offset=average_length*number_of_signals;
						event=(struct Event *)NULL;
						event_next= &(signal->first_event);
						event_number=1;
						no_maximum=0;
						do
						{
							maximum_slope=slope;
							maximum=present;
							interval_end=SCALE_X(event_number,0,start_search,
								SCALE_FACTOR(number_of_events,end_search-start_search));
							while ((present<interval_end)&&(present<start))
							{
								present++;
								average_before += *value_before-first_value;
								value_before += number_of_signals;
								if (present>end)
								{
									average_after += last_value- *value_before;
								}
								else
								{
									value_after += number_of_signals;
									average_after += *value_after- *(value_after-offset);
								}
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
									maximum=present;
									no_maximum=0;
								}
							}
							while ((present<interval_end)&&(present<end))
							{
								present++;
								average_before += *value_before- *(value_before-offset);
								value_before += number_of_signals;
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
									maximum=present;
									no_maximum=0;
								}
							}
							while (present<interval_end)
							{
								present++;
								average_before += *value_before- *(value_before-offset);
								value_before += number_of_signals;
								average_after += last_value- *value_before;
								slope=average_after-average_before;
								switch (objective)
								{
									case ABSOLUTE_SLOPE:
									{
										if (slope<0)
										{
											slope= -slope;
										}
									} break;
									case NEGATIVE_SLOPE:
									{
										slope= -slope;
									} break;
								}
								if ((maximum_slope<slope)||no_maximum)
								{
									maximum_slope=slope;
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
						}
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
						/*???DB.  Could average signal over gradient_average_width */
						value=signals+(start_search*number_of_signals+(signal->index));
						average_length=gradient_average_width;
						present=start_search;
						while ((present<end_search)&&(*value<level)&&(-(*value)<level))
						{
							value += number_of_signals;
							present++;
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
						average_length=gradient_average_width;
						offset=average_length*number_of_signals;
						minimum_separation=(int)(((float)minimum_separation_milliseconds*
							(buffer->frequency))/1000.);
						/* determine the minimum and maximum slopes */
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						maximum_slope=slope;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								minimum_slope=0;
							} break;
							default:
							{
								minimum_slope=maximum_slope;
							} break;
						}
						while ((present<end_search)&&(present<start))
						{
							present++;
							average_before += *value_before-first_value;
							value_before += number_of_signals;
							if (present>end)
							{
								average_after += last_value- *value_before;
							}
							else
							{
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
							}
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						while ((present<end_search)&&(present<end))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							value_after += number_of_signals;
							average_after += *value_after- *(value_after-offset);
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						while (present<end_search)
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							average_after += last_value- *value_before;
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (maximum_slope<slope)
							{
								maximum_slope=slope;
							}
							if (slope<minimum_slope)
							{
								minimum_slope=slope;
							}
						}
						threshold=(threshold_percentage*maximum_slope+
							(100-threshold_percentage)*minimum_slope)/100;
						/* determine the events */
						if (start_search<average_length)
						{
							start=average_length;
							if (start>end_search)
							{
								start=end_search;
							}
							value_before=signals+signal->index;
							first_value= *value_before;
							average_before=first_value*(average_length-start_search);
							for (i=0;i<start_search;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						else
						{
							start=start_search;
							value_before=signals+
								(start_search-average_length)*number_of_signals+signal->index;
							average_before=0;
							for (i=0;i<average_length;i++)
							{
								average_before += *value_before;
								value_before += number_of_signals;
							}
						}
						present=start_search;
						value_after=value_before;
						if (start_search>(end=number_of_samples-average_length-1))
						{
							average_after=0;
							for (i=start_search+1;i<number_of_samples;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
							average_after += (*value_after)*(start_search-end);
						}
						else
						{
							average_after=0;
							for (i=0;i<average_length;i++)
							{
								value_after += number_of_signals;
								average_after += *value_after;
							}
						}
						if (end_search>end)
						{
							last_value=
								signals[(number_of_samples-1)*number_of_signals+signal->index];
						}
						else
						{
							end=end_search;
						}
						slope=average_after-average_before;
						switch (objective)
						{
							case ABSOLUTE_SLOPE:
							{
								if (slope<0)
								{
									slope= -slope;
								}
							} break;
							case NEGATIVE_SLOPE:
							{
								slope= -slope;
							} break;
						}
						if (slope>=threshold)
						{
							maximum_slope=slope;
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
						while (return_code&&(present<end_search)&&(present<start))
						{
							present++;
							average_before += *value_before-first_value;
							value_before += number_of_signals;
							if (present>end)
							{
								average_after += last_value- *value_before;
							}
							else
							{
								value_after += number_of_signals;
								average_after += *value_after- *(value_after-offset);
							}
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
							{
								if (!no_maximum||(maximum>=start_search)&&
									(minimum_separation<present-maximum))
								{
									if (event=create_Event(maximum,event_number,UNDECIDED,event,
										(struct Event *)NULL))
									{
										*event_next=event;
										event_next= &(event->next);
										event_number++;
										maximum=present;
										maximum_slope=slope;
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
									if ((maximum_slope<slope)||no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
										no_maximum=0;
									}
								}
							}
						}
						while (return_code&&(present<end_search)&&(present<end))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							value_after += number_of_signals;
							average_after += *value_after- *(value_after-offset);
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
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
										maximum_slope=slope;
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
									if ((maximum_slope<slope)||no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
										no_maximum=0;
									}
								}
							}
						}
						while (return_code&&(present<end_search))
						{
							present++;
							average_before += *value_before- *(value_before-offset);
							value_before += number_of_signals;
							average_after += last_value- *value_before;
							slope=average_after-average_before;
							switch (objective)
							{
								case ABSOLUTE_SLOPE:
								{
									if (slope<0)
									{
										slope= -slope;
									}
								} break;
								case NEGATIVE_SLOPE:
								{
									slope= -slope;
								} break;
							}
							if (slope>=threshold)
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
										maximum_slope=slope;
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
									if ((maximum_slope<slope)&&no_maximum)
									{
										maximum=present;
										maximum_slope=slope;
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
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_device_event_markers.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_device_event_markers */

int draw_signal(struct FE_node *device_node,
	struct Draw_package *draw_package,struct Device *device,
	enum Signal_detail detail,int first_data,int last_data,int x_pos,int y_pos,
	int width,int height,Pixmap pixel_map,int *axes_left,int *axes_top,
	int *axes_width,int *axes_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
Draws the <device> signal in the <pixel_map> at the specified position
(<x_pos>, <y_pos>), size (<width>, <height>) and <detail>.
???missing data ? times ?
???DB.  Needs more checking and more on return_code
???DB.  Change first_data and last_data to times
==============================================================================*/
{
	char *name,number_string[20],ticks_above;
	Display *display;
	enum Event_signal_status *signals_status;
	float postscript_page_bottom,postscript_page_left,postscript_page_height,
		postscript_page_width,signal_maximum,signal_minimum,signal_ref,
		*signals_values,*signal_value,*time,time_max,time_min,time_ref,*times,
		time_scale,time_tick,time_tick_max,time_tick_min,time_tick_width,
		value_float,value_scale,value_tick,value_tick_max,value_tick_min,
		value_tick_width,world_height,world_left,world_top,world_width,x_scale,
		y_scale;
	GC graphics_context;
	int ascent,descent,highlight,direction,i,j,length,
		number_of_points,number_of_segments,
		number_of_signals,number_of_ticks,return_code,x_marker,x_max,x_min,
		x_ref,x_string,x_tick,x_tick_length=3,y_marker,y_max,y_min,y_ref,y_string,
		y_tick,y_tick_length=3;
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
	if (user_interface&&signal_drawing_information)
	{
		signals_status=(enum Event_signal_status *)NULL;
		signals_values=(float *)NULL;
		times=(float *)NULL;
		name=(char *)NULL;
		if (extract_signal_information(device_node,draw_package,device,0,first_data,
			last_data,&number_of_signals,&number_of_points,&times,&signals_values,
			&signals_status,&name,&highlight,&signal_minimum,&signal_maximum))
		{
			/* local variables initialised OK.  Use them only from now on */
			font=signal_drawing_information->font;
			display=user_interface->display;
			graphics_context=
				(signal_drawing_information->graphics_context).axis_colour;
			/* calculate the data time range from the first signal */
			time_min=times[0];
			time_max=times[number_of_points-1];
			/* determine the time tick marks */
			calculate_divisions(time_min,time_max,5,&time_tick_width,&time_tick_min,
				&time_tick_max);
			/* scale the time range to fit in the specified width on the screen */
			switch (detail)
			{
				case SIGNAL_AREA_DETAIL:
				{
					/* leave enough room on the left for displaying a 3 character name */
						/*???DB.  Can I do better than this ?  The axes only need to be
							calculated once when drawing all the signals */
					XTextExtents(font,"BBB",3,&direction,&ascent,&descent,&bounds);
					x_min=x_pos+bounds.lbearing+bounds.rbearing+3;
					x_max=x_pos+width-1;
				} break;
				case INTERVAL_AREA_DETAIL: case ENLARGE_AREA_DETAIL:
				{
					/* leave enough room on the left for displaying a 3 character name */
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
			if (signal_maximum<signal_minimum)
			{
				/* calculate the maximum and minimum signal values */
				signal_value=signals_values;
				signal_minimum= *signal_value;
				signal_maximum=signal_minimum;
				for (i=number_of_signals*number_of_points-1;i>0;i--)
				{
					signal_value++;
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
				}
			}
			/* allow for constant signals */
			if (signal_minimum==signal_maximum)
			{
				signal_maximum += 1;
				signal_minimum -= 1;
			}
			if (device)
			{
				device->signal_maximum=signal_maximum;
				device->signal_minimum=signal_minimum;
			}
#if defined (UNEMAP_USE_NODES)
			else
			{
				struct FE_field_component component;
				component.number=0;
				component.field=get_Draw_package_signal_minimum_field(draw_package); 
				set_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
					signal_minimum);
				component.number=0;
				component.field=get_Draw_package_signal_maximum_field(draw_package); 
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
#endif
					}
					else
					{
						/* ticks below */
						ticks_above=0;
						y_tick=y_marker+y_tick_length;
#if !defined (NO_ALIGNMENT)
						SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif
					}
#if !defined (NO_ALIGNMENT)
					SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif
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
#else
					XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
						number_string,length);
#endif
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
							XPSDrawLine(display,pixel_map,graphics_context,x_marker,y_marker,
								x_marker,y_tick);
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
#else
							XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
								number_string,length);
#endif
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
#else
					XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
						number_string,length);
#endif
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
#endif
					x_tick=x_min-x_tick_length;
					number_of_ticks=
						(int)((value_tick_max-value_tick_min)/value_tick_width+0.5);
					value_tick=value_tick_min;
					y_marker=SCALE_Y(value_tick,signal_maximum,y_min,value_scale);
					/* draw the minimum tick mark */
					XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,x_min,
						y_marker);
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
#else
					XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
						number_string,length);
#endif
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
#else
							XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
								number_string,length);
#endif
						}
						/* move to the next tick */
						value_tick += value_tick_width;
					}
					/* draw the maximum tick mark */
					y_marker=SCALE_Y(value_tick,signal_maximum,y_min,value_scale);
					XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,x_min,
						y_marker);
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
#else
					XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
						number_string,length);
#endif
				} break;
			}
			if (name)
			{
				/* draw the signal name */
				length=strlen(name);
#if defined (NO_ALIGNMENT)
				XTextExtents(font,name,length,&direction,&ascent,&descent,&bounds);
#endif
				switch (detail)
				{
					case SIGNAL_AREA_DETAIL:
					{
						/* right justified */
#if defined (NO_ALIGNMENT)
						x_string=x_min-(bounds.rbearing+2);
#else
						SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
						x_string=x_min;
#endif
					} break;
					case EDIT_AREA_DETAIL: case INTERVAL_AREA_DETAIL:
					case ENLARGE_AREA_DETAIL: case PRINTER_DETAIL:
					{
						/* left justified */
#if defined (NO_ALIGNMENT)
						x_string=x_pos+(bounds.lbearing+2);
#else
						SET_HORIZONTAL_ALIGNMENT(LEFT_ALIGNMENT);
						x_string=x_pos;
#endif
					} break;
				}
#if defined (NO_ALIGNMENT)
				y_string=(y_min+y_max+ascent-descent)/2;
#else
				SET_VERTICAL_ALIGNMENT(CENTRE_VERTICAL_ALIGNMENT);
				y_string=(y_min+y_max)/2;
#endif
				if (highlight)
				{
					XPSDrawString(display,pixel_map,
						(signal_drawing_information->graphics_context).highlighted_colour,
						x_string,y_string,name,length);
				}
				else
				{
					XPSDrawString(display,pixel_map,
						(signal_drawing_information->graphics_context).device_name_colour,
						x_string,y_string,name,length);
				}
			}
			/* use full resolution for printer */
			if (PRINTER_DETAIL==detail)
			{
				x_scale=SCALE_FACTOR(x_max-x_min,time_max-time_min);
				time_scale=1;
				y_scale=SCALE_FACTOR(y_max-y_min,signal_maximum-signal_minimum);
				value_scale=1;
				/* set the printer transformation */
				if (get_postscript_display_transfor(&postscript_page_left,
					&postscript_page_bottom,&postscript_page_width,
					&postscript_page_height,&world_left,&world_top,&world_width,
					&world_height))
				{
					set_postscript_display_transfor(postscript_page_left,
						postscript_page_bottom,postscript_page_width,postscript_page_height,
						(float)time_min+(float)(x_pos-x_min)*x_scale,
						(float)(y_pos-y_min)*y_scale-signal_maximum,x_scale*(float)width,
						y_scale*(float)height);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"draw_signal.  Could not get_postscript_page_size");
				}
				x_min=time_min;
				x_max=time_max;
				y_min= -signal_maximum;
				y_max= -signal_minimum;
				time_ref=0;
				x_ref=0;
				signal_ref=0;
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
			for (j=0;j<number_of_signals;j++)
			{
				/* determine the signal colour */
				if ((highlight)&&(detail==SIGNAL_AREA_DETAIL))
				{
					graphics_context=
						(signal_drawing_information->graphics_context).highlighted_colour;
				}
				else
				{
					switch (signals_status[j])
					{
						case ACCEPTED:
						{
							graphics_context=(signal_drawing_information->graphics_context).
								signal_accepted_colour;
						} break;
						case REJECTED:
						{
							if ((j>0)&&(signal_drawing_information->
								number_of_signal_overlay_colours>0))
							{
								graphics_context=(signal_drawing_information->graphics_context).
									signal_overlay_colour;
								XSetForeground(display,graphics_context,
									(signal_drawing_information->signal_overlay_colours)[(j-1)%
									(signal_drawing_information->
									number_of_signal_overlay_colours)]);
							}
							else
							{
								graphics_context=(signal_drawing_information->graphics_context).
									signal_rejected_colour;
							}
#if defined (OLD_CODE)
							/*???DB.  For dfn.  Needs generalizing */
							/*???DB.  Stops drawing if I use different contexts.  Why ? */
							graphics_context=
								(signal_drawing_information->graphics_context).spectrum;
							switch (j%6)
							{
								case 0:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->signal_rejected_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).signal_rejected_colour;*/
								} break;
								case 1:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->signal_undecided_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).signal_undecided_colour;*/
								} break;
								case 2:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->signal_accepted_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).signal_accepted_colour;*/
								} break;
								case 3:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->rejected_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).rejected_colour;*/
								} break;
								case 4:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->undecided_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).undecided_colour;*/
								} break;
								case 5:
								{
									XSetForeground(display,graphics_context,
										signal_drawing_information->accepted_colour);
/*									graphics_context=(signal_drawing_information->
										graphics_context).accepted_colour;*/
								} break;
							}
#endif /* defined (OLD_CODE) */
						} break;
						case UNDECIDED:
						{
							graphics_context=(signal_drawing_information->graphics_context).
								signal_undecided_colour;
						} break;
					}
				}
				XPSSetClipRectangles(display,graphics_context,0,0,&clip_rectangle,1,
					Unsorted);
				if ((PRINTER_DETAIL==detail)||(number_of_points<=4*(*axes_width)))
				{
					/* draw all the data points with line segments joining them */
					if (ALLOCATE(points,XPoint,number_of_points))
					{
						/* calculate the points */
						point=points;
						time=times;
						signal_value=signals_values+(j*number_of_points);
						for (i=number_of_points;i>0;i--)
						{
							point->x=SCALE_X(*time,time_ref,x_ref,time_scale);
							point->y=SCALE_Y(*signal_value,signal_ref,y_ref,value_scale);
							point++;
							signal_value++;
							time++;
						}	
						/* draw */
						XPSDrawLines(display,pixel_map,graphics_context,points,
							number_of_points,CoordModeOrigin);
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
						time=times;
						signal_value=signals_values+(j*number_of_points);
						value_short_int=SCALE_Y(*signal_value,signal_ref,y_ref,value_scale);
						segment->x1=x_min;
						segment->y1=value_short_int;
						segment->x2=x_min;
						segment->y2=value_short_int;
						number_of_segments=1;
						i=number_of_points-1;
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
		else
		{
			display_message(ERROR_MESSAGE,"draw_signal.  Could not extract signal");
			return_code=0;
		}	
		DEALLOCATE(name);	
		DEALLOCATE(signals_status);
		DEALLOCATE(signals_values);
		DEALLOCATE(times);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_signal.  Invalid arguments");
		return_code=0;
	}	
	LEAVE;

	return (return_code);
} /* draw_signal */

#if defined (OLD_CODE)
int draw_signal(struct Device *device,enum Signal_detail detail,int first_data,
	int last_data,int x_pos,int y_pos,int width,int height,Pixmap pixel_map,
	int *axes_left,int *axes_top,int *axes_width,int *axes_height,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 October 1998

DESCRIPTION :
Draws the <device> signal in the <pixel_map> at the specified position
(<x_pos>, <y_pos>), size (<width>, <height>) and <detail>.
???missing data ? times ?
???DB.  Needs more checking and more on return_code
==============================================================================*/
{
	char number_string[20],ticks_above;
	Display *display;
	float channel_gain,channel_offset,*data_value_float,frequency,frequency_next,
		postscript_page_bottom,postscript_page_left,postscript_page_height,
		postscript_page_width,signal_max,signal_min,signal_ref,time_ref_next,
		time_scale,time_scale_next,time_tick,time_tick_max,time_tick_min,
		time_tick_width,value_float,value_scale,value_tick,value_tick_max,
		value_tick_min,value_tick_width,world_height,world_left,world_top,
		world_width,x_scale,y_scale;
	GC graphics_context;
	int ascent,buffer_offset,buffer_offset_next,*data_time,*data_time_next,
		descent,direction,first_data_next,i,last_data_next,length,number_of_points,
		number_of_points_next,number_of_segments,number_of_ticks,return_code,
		time_max,time_min,time_ref,x_marker,x_max,x_min,x_ref,x_string,x_tick,
		x_tick_length=3,y_marker,y_max,y_min,y_ref,y_string,y_tick,y_tick_length=3;
	short int *data_value_short_int,value_short_int,x;
	struct Signal *signal,*signal_next;
	struct Signal_buffer *buffer,*buffer_next;
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
	if (device&&(signal=device->signal)&&(buffer=signal->buffer)&&user_interface&&
		signal_drawing_information)
	{
		font=signal_drawing_information->font;
		display=user_interface->display;
		graphics_context=
			(signal_drawing_information->graphics_context).axis_colour;
		/* calculate the data time range from the first signal */
		data_time=buffer->times;
		frequency=buffer->frequency;
		time_min=data_time[first_data];
		time_max=data_time[last_data];
		/* determine the time tick marks */
		calculate_divisions(((float)time_min)/frequency,
			((float)time_max)/frequency,5,
			&time_tick_width,&time_tick_min,&time_tick_max);
		/* scale the time range to fit in the specified width on the screen */
		switch (detail)
		{

			case SIGNAL_AREA_DETAIL:
			{
				/* leave enough room on the left for displaying a 3 character electrode
					name */
					/*???Can I do better than this ?  The axes only need to be calculated
						once when drawing all the signals */
				XTextExtents(font,"BBB",3,&direction,&ascent,&descent,&bounds);
				x_min=x_pos+bounds.lbearing+bounds.rbearing+3;
				x_max=x_pos+width-1;
			} break;
			case INTERVAL_AREA_DETAIL: case ENLARGE_AREA_DETAIL:
			{
				/* leave enough room on the left for displaying a 3 character electrode
					name */
					/*???Can I do better than this ?  The axes only need to be calculated
						once when drawing all the signals */
				XTextExtents(font,"BBB",3,&direction,&ascent,&descent,&bounds);
				x_min=x_pos+bounds.lbearing+bounds.rbearing+3;
				x_max=x_pos+width-3;
			} break;
			case EDIT_AREA_DETAIL: case PRINTER_DETAIL:
			{
				/* leave enough room on the left for displaying a 3 character electrode
					name, a 3 digit voltage and a tick mark */
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
		channel_offset=device->channel->offset;
		channel_gain=device->channel->gain;
		buffer_offset=buffer->number_of_signals;
		signal_max=device->signal_maximum;
		signal_min=device->signal_minimum;
		number_of_points=last_data-first_data+1;
		if (signal_max<signal_min)
		{
			/* calculate the maximum and minimum signal values */
			switch (buffer->value_type)
			{
				case SHORT_INT_VALUE:
				{
					data_value_short_int=(buffer->signals.short_int_values)+
						(first_data*buffer_offset+signal->index);
					signal_min=(float)(*data_value_short_int);
					signal_max=signal_min;
					for (i=number_of_points;i>0;i--)
					{
						value_float=(float)(*data_value_short_int);
						if (value_float<signal_min)
						{
							signal_min=value_float;
						}
						else
						{
							if (value_float>signal_max)
							{
								signal_max=value_float;
							}
						}
						data_value_short_int += buffer_offset;
					}
				} break;
				case FLOAT_VALUE:
				{
					data_value_float=(buffer->signals.float_values)+
						(first_data*buffer_offset+signal->index);
					signal_min= *data_value_float;
					signal_max=signal_min;
					for (i=number_of_points;i>0;i--)
					{
						value_float= *data_value_float;
						if (value_float<signal_min)
						{
							signal_min=value_float;
						}
						else
						{
							if (value_float>signal_max)
							{
								signal_max=value_float;
							}
						}
						data_value_float += buffer_offset;
					}
				} break;
			}
			signal_next=signal->next;
			while (signal_next)
			{
				buffer_next=signal_next->buffer;
				data_time_next=buffer_next->times;
				frequency_next=buffer_next->frequency;
				buffer_offset_next=buffer_next->number_of_signals;
				first_data_next=0;
				last_data_next=(buffer_next->number_of_samples)-1;
				while ((first_data_next<=last_data_next)&&
					(((float)data_time_next[first_data_next])*frequency<
					time_min*frequency_next))
				{
					first_data_next++;
				}
				while ((first_data_next<=last_data_next)&&(time_max*frequency_next<
					((float)data_time_next[last_data_next])*frequency))
				{
					last_data_next--;
				}
				switch (buffer_next->value_type)
				{
					case SHORT_INT_VALUE:
					{
						data_value_short_int=(buffer_next->signals.short_int_values)+
							(first_data_next*buffer_offset_next+signal_next->index);
						for (i=last_data_next-first_data_next+1;i>0;i--)
						{
							value_float=(float)(*data_value_short_int);
							if (value_float<signal_min)
							{
								signal_min=value_float;
							}
							else
							{
								if (value_float>signal_max)
								{
									signal_max=value_float;
								}
							}
							data_value_short_int += buffer_offset_next;
						}
					} break;
					case FLOAT_VALUE:
					{
						data_value_float=(buffer_next->signals.float_values)+
							(first_data_next*buffer_offset_next+signal_next->index);
						for (i=last_data_next-first_data_next+1;i>0;i--)
						{
							value_float= *data_value_float;
							if (value_float<signal_min)
							{
								signal_min=value_float;
							}
							else
							{
								if (value_float>signal_max)
								{
									signal_max=value_float;
								}
							}
							data_value_float += buffer_offset_next;
						}
					} break;
				}
				signal_next=signal_next->next;
			}
			device->signal_maximum=signal_max;
			device->signal_minimum=signal_min;
		}
		/* allow for constant signals */
		if (signal_min==signal_max)
		{
			signal_max += 1;
			signal_min -= 1;
		}
		/* determine the value tick marks */
		if (0!=channel_gain)
		{
			calculate_divisions(channel_gain*(signal_min-channel_offset),
				channel_gain*(signal_max-channel_offset),5,&value_tick_width,
				&value_tick_min,&value_tick_max);
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_signal.  Zero gain - ignore");
			calculate_divisions(signal_min,signal_max,5,&value_tick_width,
				&value_tick_min,&value_tick_max);
		}
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
		value_scale=SCALE_FACTOR(signal_max-signal_min,y_max-y_min);
		/* draw axes */
		/* draw the x axis */
		if ((signal_max<=channel_offset)||(signal_min>=channel_offset))
		{
			y_marker=y_max;
		}
		else
		{
			y_marker=SCALE_Y(channel_offset,signal_max,y_min,value_scale);
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
#endif
				}
				else
				{
					/* ticks below */
					ticks_above=0;
					y_tick=y_marker+y_tick_length;
#if !defined (NO_ALIGNMENT)
					SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif
				}
#if !defined (NO_ALIGNMENT)
				SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif
				number_of_ticks=
					(int)((time_tick_max-time_tick_min)/time_tick_width+0.5);
				time_tick=time_tick_min;
				/* draw the left tick mark */
				x_marker=SCALE_X(time_tick*frequency,(float)time_min,x_min,time_scale);
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
#else
				XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
					number_string,length);
#endif
				/* move to the next tick */
				time_tick += time_tick_width;
				for (i=number_of_ticks-1;i>0;i--)
				{
					x_marker=
						SCALE_X(time_tick*frequency,(float)time_min,x_min,time_scale);
					/* draw the tick mark */
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
					if (PRINTER_DETAIL!=detail)
					{
						/* write the tick value */
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
#else
						XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
							number_string,length);
#endif
					}
					/* move to the next tick */
					time_tick += time_tick_width;
				}
				/* draw the right tick mark */
				x_marker=SCALE_X(time_tick*frequency,(float)time_min,x_min,time_scale);
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
#else
				XPSDrawString(display,pixel_map,graphics_context,x_marker,y_tick,
					number_string,length);
#endif
			} break;
		}
		/* draw the y axis */
		XPSDrawLine(display,pixel_map,graphics_context,x_min,y_min,x_min,y_max);
		/* draw the y axis markings - not for low detail */
		switch (detail)
		{
			case EDIT_AREA_DETAIL:
				/*???temp ?*/case PRINTER_DETAIL:
			{
#if !defined (NO_ALIGNMENT)
				SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
				SET_VERTICAL_ALIGNMENT(CENTRE_VERTICAL_ALIGNMENT);
#endif
				x_tick=x_min-x_tick_length;
				number_of_ticks=
					(int)((value_tick_max-value_tick_min)/value_tick_width+0.5);
				value_tick=value_tick_min;
				y_marker=SCALE_Y(value_tick/channel_gain+channel_offset,
					signal_max,y_min,value_scale);
				/* draw the minimum tick mark */
				XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,x_min,
					y_marker);
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
#else
				XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
					number_string,length);
#endif
				/* move to the next tick */
				value_tick += value_tick_width;
				for (i=number_of_ticks-1;i>0;i--)
				{
					y_marker=SCALE_Y(value_tick/channel_gain+channel_offset,
						signal_max,y_min,value_scale);
					/* draw the tick mark */
					XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,x_min,
						y_marker);
					if (PRINTER_DETAIL!=detail)
					{
						/* write the tick value */
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
#else
						XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
							number_string,length);
#endif
					}
					/* move to the next tick */
					value_tick += value_tick_width;
				}
				/* draw the maximum tick mark */
				y_marker=SCALE_Y(value_tick/channel_gain+channel_offset,
					signal_max,y_min,value_scale);
				XPSDrawLine(display,pixel_map,graphics_context,x_tick,y_marker,x_min,
					y_marker);
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
#else
				XPSDrawString(display,pixel_map,graphics_context,x_tick,y_marker,
					number_string,length);
#endif
			} break;
		}
		if (device->description->name)
		{
			/* draw the signal name */
			length=strlen(device->description->name);
#if defined (NO_ALIGNMENT)
			XTextExtents(font,device->description->name,length,&direction,&ascent,
				&descent,&bounds);
#endif
			switch (detail)
			{
				case SIGNAL_AREA_DETAIL:
				{
					/* right justified */
#if defined (NO_ALIGNMENT)
					x_string=x_min-(bounds.rbearing+2);
#else
					SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
					x_string=x_min;
#endif
				} break;
				case EDIT_AREA_DETAIL: case INTERVAL_AREA_DETAIL:
					case ENLARGE_AREA_DETAIL: case PRINTER_DETAIL:
				{
					/* left justified */
#if defined (NO_ALIGNMENT)
					x_string=x_pos+(bounds.lbearing+2);
#else
					SET_HORIZONTAL_ALIGNMENT(LEFT_ALIGNMENT);
					x_string=x_pos;
#endif
				} break;
			}
#if defined (NO_ALIGNMENT)
			y_string=(y_min+y_max+ascent-descent)/2;
#else
			SET_VERTICAL_ALIGNMENT(CENTRE_VERTICAL_ALIGNMENT);
			y_string=(y_min+y_max)/2;
#endif
			if (device->highlight)
			{
				XPSDrawString(display,pixel_map,
					(signal_drawing_information->graphics_context).highlighted_colour,
					x_string,y_string,device->description->name,length);
			}
			else
			{
				XPSDrawString(display,pixel_map,
					(signal_drawing_information->graphics_context).device_name_colour,
					x_string,y_string,device->description->name,length);
			}
		}
		/* draw the signal */
		/* determine the signal colour */
		if ((device->highlight)&&(detail==SIGNAL_AREA_DETAIL))
		{
			graphics_context=
				(signal_drawing_information->graphics_context).highlighted_colour;
		}
		else
		{
			switch (device->signal->status)
			{
				case ACCEPTED:
				{
					graphics_context=(signal_drawing_information->graphics_context).
						signal_accepted_colour;
				} break;
				case REJECTED:
				{
					/*???DB.  For dfn.  Needs generalizing */
					/*???DB.  Stops drawing if I use different contexts.  Why ? */
					graphics_context=
						(signal_drawing_information->graphics_context).spectrum;
					switch ((signal->number)%6)
					{
						case 0:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->signal_rejected_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								signal_rejected_colour;*/
						} break;
						case 1:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->signal_undecided_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								signal_undecided_colour;*/
						} break;
						case 2:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->signal_accepted_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								signal_accepted_colour;*/
						} break;
						case 3:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->rejected_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								rejected_colour;*/
						} break;
						case 4:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->undecided_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								undecided_colour;*/
						} break;
						case 5:
						{
							XSetForeground(display,graphics_context,
								signal_drawing_information->accepted_colour);
/*							graphics_context=(signal_drawing_information->graphics_context).
								accepted_colour;*/
						} break;
					}
				} break;
				case UNDECIDED:
				{
					graphics_context=(signal_drawing_information->graphics_context).
						signal_undecided_colour;
				} break;
			}
		}
		/* use full resolution for printer */
		if (PRINTER_DETAIL==detail)
		{
			x_scale=SCALE_FACTOR(x_max-x_min,time_max-time_min);
			time_scale=1;
			switch (buffer->value_type)
			{
				case SHORT_INT_VALUE:
				{
					y_scale=SCALE_FACTOR(y_max-y_min,signal_max-signal_min);
					value_scale=1;
				} break;
				case FLOAT_VALUE:
				{
					value_scale=fabs(signal_max);
					y_scale=fabs(signal_min);
					if (value_scale>y_scale)
					{
						value_scale=(float)SHRT_MAX/value_scale;
					}
					else
					{
						value_scale=(float)SHRT_MAX/y_scale;
					}
					signal_max *= value_scale;
					signal_min *= value_scale;
					y_scale=SCALE_FACTOR(y_max-y_min,signal_max-signal_min);
				} break;
			}
			/* set the printer transformation */
			if (get_postscript_display_transfor(&postscript_page_left,
				&postscript_page_bottom,&postscript_page_width,&postscript_page_height,
				&world_left,&world_top,&world_width,&world_height))
			{
				set_postscript_display_transfor(postscript_page_left,
					postscript_page_bottom,postscript_page_width,postscript_page_height,
					(float)time_min+(float)(x_pos-x_min)*x_scale,
					(float)(y_pos-y_min)*y_scale-signal_max,x_scale*(float)width,
					y_scale*(float)height);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_signal.  Could not get_postscript_page_size");
			}
			x_min=time_min;
			x_max=time_max;
			y_min= -signal_max;
			y_max= -signal_min;
			time_ref=0;
			x_ref=0;
			signal_ref=0;
			y_ref=0;
		}
		else
		{
			time_ref=time_min;
			x_ref=x_min;
			signal_ref=signal_max;
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
		XPSSetClipRectangles(display,graphics_context,0,0,&clip_rectangle,1,
			Unsorted);
		if ((PRINTER_DETAIL==detail)||(number_of_points<=4*(*axes_width)))
		{
			/* draw all the data points with line segments joining them */
			if (ALLOCATE(points,XPoint,number_of_points))
			{
				/* calculate the points */
				point=points;
				data_time=(buffer->times)+first_data;
#if defined (DEBUG)
				/*???debug */
				printf("points.  *data_time=%d, first_data=%d, time_ref=%d\n",
					*data_time,first_data,time_ref);
				printf("x_ref=%d, time_scale=%g, time_max=%d\n",x_ref,time_scale,
					time_max);
				printf("time_min=%d, x_max=%d, x_min=%d\n",time_min,x_max,x_min);
#endif /* defined (DEBUG) */
				switch (buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						data_value_short_int=(buffer->signals.short_int_values)+
							(first_data*buffer_offset+signal->index);
						for (i=number_of_points;i>0;i--)
						{
							point->x=SCALE_X(*data_time,time_ref,x_ref,time_scale);
							point->y=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
								value_scale);
							point++;
							data_value_short_int += buffer_offset;
							data_time++;
						}
					} break;
					case FLOAT_VALUE:
					{
						data_value_float=(buffer->signals.float_values)+
							(first_data*buffer_offset+signal->index);
						for (i=number_of_points;i>0;i--)
						{
							point->x=SCALE_X(*data_time,time_ref,x_ref,time_scale);
							point->y=SCALE_Y(*data_value_float,signal_ref,y_ref,value_scale);
							point++;
							data_value_float += buffer_offset;
							data_time++;
						}
					} break;
				}
				/* draw */
				XPSDrawLines(display,pixel_map,graphics_context,points,number_of_points,
					CoordModeOrigin);
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
				data_time=(buffer->times)+first_data;
#if defined (DEBUG)
				/*???debug */
				printf("segments.  *data_time=%d, first_data=%d, time_ref=%d\n",
					*data_time,first_data,time_ref);
				printf("x_ref=%d, time_scale=%g, time_max=%d\n",x_ref,time_scale,
					time_max);
				printf("time_min=%d, x_max=%d, x_min=%d\n",time_min,x_max,x_min);
#endif /* defined (DEBUG) */
				switch (buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						data_value_short_int=(buffer->signals.short_int_values)+
							(first_data*buffer_offset+signal->index);
						value_short_int=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
							value_scale);
						segment->x1=x_min;
						segment->y1=value_short_int;
						segment->x2=x_min;
						segment->y2=value_short_int;
						number_of_segments=1;
						i=number_of_points-1;
						while (i>0)
						{
							data_time++;
							data_value_short_int += buffer_offset;
							while ((i>0)&&(segment->x2==
								(x=SCALE_X(*data_time,time_ref,x_ref,time_scale))))
							{
								value_short_int=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
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
								data_value_short_int += buffer_offset;
								data_time++;
								i--;
							}
							if (i>0)
							{
								segment[1].x1=segment->x2;
								segment++;
								number_of_segments++;
								segment->y1=value_short_int;
								segment->x2=x;
								value_short_int=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
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
					} break;
					case FLOAT_VALUE:
					{
						data_value_float=(buffer->signals.float_values)+
							(first_data*buffer_offset+signal->index);
						value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
							value_scale);
						segment->x1=x_min;
						segment->y1=value_short_int;
						segment->x2=x_min;
						segment->y2=value_short_int;
						number_of_segments=1;
						i=number_of_points-1;
						while (i>0)
						{
							data_time++;
							data_value_float += buffer_offset;
							while ((i>0)&&(segment->x2==
								(x=SCALE_X(*data_time,time_ref,x_ref,time_scale))))
							{
								value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
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
								data_value_float += buffer_offset;
								data_time++;
								i--;
							}
							if (i>0)
							{
								segment[1].x1=segment->x2;
								segment++;
								number_of_segments++;
								segment->y1=value_short_int;
								segment->x2=x;
								value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
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
					} break;
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
		signal_next=signal->next;
		while (signal_next)
		{
			/* determine the signal colour */
			if ((device->highlight)&&(detail==SIGNAL_AREA_DETAIL))
			{
				graphics_context=(signal_drawing_information->graphics_context).
					highlighted_colour;
			}
			else
			{
				switch (signal_next->status)
				{
					case ACCEPTED:
					{
						graphics_context=(signal_drawing_information->graphics_context).
							signal_accepted_colour;
					} break;
					case REJECTED:
					{
						/*???DB.  For dfn.  Needs generalizing */
						/*???DB.  Stops drawing if I use different contexts.  Why ? */
						graphics_context=(signal_drawing_information->graphics_context).
							spectrum;
						switch ((signal_next->number)%6)
						{
							case 0:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->signal_rejected_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).signal_rejected_colour;*/
							} break;
							case 1:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->signal_undecided_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).signal_undecided_colour;*/
							} break;
							case 2:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->signal_accepted_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).signal_accepted_colour;*/
							} break;
							case 3:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->rejected_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).rejected_colour;*/
							} break;
							case 4:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->undecided_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).undecided_colour;*/
							} break;
							case 5:
							{
								XSetForeground(display,graphics_context,
									signal_drawing_information->accepted_colour);
/*								graphics_context=(signal_drawing_information->
									graphics_context).accepted_colour;*/
							} break;
						}
					} break;
					case UNDECIDED:
					{
						graphics_context=(signal_drawing_information->graphics_context).
							signal_undecided_colour;
					} break;
				}
			}
			buffer_next=signal_next->buffer;
			data_time_next=buffer_next->times;
			frequency_next=buffer_next->frequency;
			buffer_offset_next=buffer_next->number_of_signals;
			first_data_next=0;
			last_data_next=(buffer_next->number_of_samples)-1;
			while ((first_data_next<=last_data_next)&&
				(((float)data_time_next[first_data_next])*frequency<
				(float)time_min*frequency_next))
			{
				first_data_next++;
			}
			while ((first_data_next<=last_data_next)&&((float)time_max*frequency_next<
				((float)data_time_next[last_data_next])*frequency))
			{
				last_data_next--;
			}
			number_of_points_next=last_data_next-first_data_next+1;
			if (0<number_of_points_next)
			{
				time_ref_next=((float)time_ref)*frequency_next/frequency;
				time_scale_next=time_scale*frequency/frequency_next;
				if ((PRINTER_DETAIL==detail)||(number_of_points_next<=4*(*axes_width)))
				{
					/* draw all the data points with line segments joining them */
					if (ALLOCATE(points,XPoint,number_of_points_next))
					{
						/* calculate the points */
						point=points;
						data_time_next=(buffer_next->times)+first_data_next;
						switch (buffer_next->value_type)
						{
							case SHORT_INT_VALUE:
							{
								data_value_short_int=(buffer_next->signals.short_int_values)+
									(first_data_next*buffer_offset_next+signal_next->index);
								for (i=number_of_points_next;i>0;i--)
								{
									point->x=SCALE_X(*data_time_next,time_ref_next,x_ref,
										time_scale_next);
									point->y=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
										value_scale);
									point++;
									data_value_short_int += buffer_offset_next;
									data_time_next++;
								}
							} break;
							case FLOAT_VALUE:
							{
								data_value_float=(buffer_next->signals.float_values)+
									(first_data_next*buffer_offset_next+signal_next->index);
								for (i=number_of_points_next;i>0;i--)
								{
									point->x=SCALE_X(*data_time_next,time_ref_next,x_ref,
										time_scale_next);
									point->y=SCALE_Y(*data_value_float,signal_ref,y_ref,
										value_scale);
									point++;
									data_value_float += buffer_offset_next;
									data_time_next++;
								}
							} break;
						}
						/* draw */
						XPSDrawLines(display,pixel_map,graphics_context,points,
							number_of_points_next,CoordModeOrigin);
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
						data_time_next=(buffer_next->times)+first_data_next;
						switch (buffer_next->value_type)
						{
							case SHORT_INT_VALUE:
							{
								data_value_short_int=(buffer_next->signals.short_int_values)+
									(first_data_next*buffer_offset_next+signal_next->index);
								value_short_int=SCALE_Y(*data_value_short_int,signal_ref,y_ref,
									value_scale);
								segment->x1=SCALE_X(*data_time_next,time_ref_next,x_ref,
									time_scale_next);
								segment->y1=value_short_int;
								segment->x2=segment->x1;
								segment->y2=value_short_int;
								number_of_segments=1;
								i=number_of_points_next-1;
								while (i>0)
								{
									data_time_next++;
									data_value_short_int += buffer_offset_next;
									while ((i>0)&&(segment->x2==(x=SCALE_X(*data_time_next,
										time_ref_next,x_ref,time_scale_next))))
									{
										value_short_int=SCALE_Y(*data_value_short_int,signal_ref,
											y_ref,value_scale);
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
										data_value_short_int += buffer_offset_next;
										data_time_next++;
										i--;
									}
									if (i>0)
									{
										segment[1].x1=segment->x2;
										segment++;
										number_of_segments++;
										segment->y1=value_short_int;
										segment->x2=x;
										value_short_int=SCALE_Y(*data_value_short_int,signal_ref,
											y_ref,value_scale);
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
							} break;
							case FLOAT_VALUE:
							{
								data_value_float=(buffer_next->signals.float_values)+
									(first_data_next*buffer_offset_next+signal_next->index);
								value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
									value_scale);
								segment->x1=SCALE_X(*data_time_next,time_ref_next,x_ref,
									time_scale_next);
								segment->y1=value_short_int;
								segment->x2=segment->x1;
								segment->y2=value_short_int;
								number_of_segments=1;
								i=number_of_points-1;
								while (i>0)
								{
									data_time_next++;
									data_value_float += buffer_offset_next;
									while ((i>0)&&(segment->x2==(x=SCALE_X(*data_time_next,
										time_ref_next,x_ref,time_scale_next))))
									{
										value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
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
										data_value_float += buffer_offset_next;
										data_time_next++;
										i--;
									}
									if (i>0)
									{
										segment[1].x1=segment->x2;
										segment++;
										number_of_segments++;
										segment->y1=value_short_int;
										segment->x2=x;
										value_short_int=SCALE_Y(*data_value_float,signal_ref,y_ref,
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
							} break;
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
			}
			signal_next=signal_next->next;
		}
		XPSSetClipMask(display,graphics_context,None);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_signal.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_signal */
#endif /* defined (OLD_CODE) */

int draw_datum_marker(int datum,enum Signal_detail detail,int first_data,
	int last_data,int axes_left,int axes_top,int axes_width,int axes_height,
	Window drawing_area_window,Pixmap pixel_map,
	struct Signal_drawing_information *signal_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 22 December 1996

DESCRIPTION :
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
LAST MODIFIED : 1 January 1997

DESCRIPTION :
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
				if (PRINTER_DETAIL==detail)
				{
					x_scale=SCALE_FACTOR(axes_width-1,last_data-first_data);
					if (signal_min<=signal_max)
					{
						y_scale=SCALE_FACTOR(axes_height-1,2);
					}
					else
					{
						y_scale=SCALE_FACTOR(axes_height-1,signal_max-signal_min);
					}
					x_marker=axes_left+(event->time-first_data);
				}
				else
				{
					x_scale=1;
					y_scale=1;
					x_marker=SCALE_X(event->time,first_data,axes_left,
						SCALE_FACTOR(last_data-first_data,axes_width-1));
				}
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
LAST MODIFIED : 4 August 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code,signal_index,*times;
	float frequency;
	struct Event *event;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(draw_device_markers);
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
							frequency,detail,first_data,last_data,device->signal_minimum,
							device->signal_maximum,axes_left,axes_top,axes_width,
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
LAST MODIFIED : 14 December 1999

DESCRIPTION :
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
#define XmNdatumColour "datumColour"
#define XmCDatumColour "DatumColour"
#define XmNdeviceNameColour "deviceNameColour"
#define XmCDeviceNameColour "DeviceNameColour"
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
		},
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
			if (overlay_colours_string&&(temp_string_1=duplicate_string(
				overlay_colours_string)))
			{
				overlay_colours_string=temp_string_1;
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
				DEALLOCATE(overlay_colours_string);
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
LAST MODIFIED : 29 November 1999

DESCRIPTION :
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
		display=signal_drawing_information->user_interface->display;
		XFreeGC(display,(signal_drawing_information->graphics_context).
			accepted_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			accepted_colour_text);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			axis_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			background_drawing_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).copy);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			datum_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			device_name_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			highlighted_box_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			highlighted_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			interval_box_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			potential_time_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			potential_time_colour_text);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			rejected_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			rejected_colour_text);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			scaling_signal_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			signal_accepted_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			signal_rejected_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			signal_overlay_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			signal_undecided_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).spectrum);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			undecided_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			undecided_colour_text);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			unhighlighted_colour);
		XFreeGC(display,(signal_drawing_information->graphics_context).
			accepted_colour);
		DEALLOCATE(signal_drawing_information->signal_overlay_colours);
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
