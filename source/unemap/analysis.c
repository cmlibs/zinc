/*******************************************************************************
FILE : analysis.c

LAST MODIFIED : 20 July 2004

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
Global functions
----------------
*/
int calculate_device_objective(struct Device *device,
	enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,float *objective_values,
	int number_of_objective_values,int objective_values_step,int average_width)
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the <device>.
Storing the values in the array (<objective_values> every
<objective_values_step>) provided.
==============================================================================*/
{
	int number_of_signals,return_code,save_number_of_objective_values;
#if defined (OLD_CODE)
	float *float_value,*objective_value;
	int i,number_of_samples,number_of_signals;
	short *short_value;
	struct Signal *signal;
	struct Signal_buffer *buffer;
#endif /* defined (OLD_CODE) */

	ENTER(calculate_device_objective);
	return_code=0;
#if defined (OLD_CODE)
	number_of_samples=0;
	if (device&&(signal=device->signal)&&(buffer=signal->buffer)&&
		(0<(number_of_samples=buffer->number_of_samples))&&
		(((SHORT_INT_VALUE==buffer->value_type)&&
		(buffer->signals.short_int_values))||((FLOAT_VALUE==buffer->value_type)&&
		(buffer->signals.float_values)))&&objective_values&&
		(number_of_samples<=number_of_objective_values)&&
		(0<objective_values_step)&&(0<average_width)&&(device->channel))
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
		return_code=calculate_time_series_objective(detection,objective,
			average_width,device->channel->gain,device->channel->offset,
			number_of_samples,objective_values_step,objective_values);
	}
	else
	{
		if (device)
		{
			if (device->signal)
			{
				if (device->signal->buffer)
				{
					display_message(ERROR_MESSAGE,"calculate_device_objective.  "
						"Invalid argument(s).  %d (%d %d) %p %p %p %d %d %d %p",
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
#endif /* defined (OLD_CODE) */
	save_number_of_objective_values=number_of_objective_values;
	number_of_signals=objective_values_step;
	if ((extract_Device_signal_information(device,1,0,
		number_of_objective_values-1,(float **)NULL,
		&objective_values,(enum Event_signal_status **)NULL,
		&number_of_signals,&save_number_of_objective_values,(char **)NULL,
		(int *)NULL,(float *)NULL,(float *)NULL))&&
		(save_number_of_objective_values==number_of_objective_values)&&
		(1==number_of_signals))
	{
		return_code=calculate_time_series_objective(detection,objective,
			average_width,(float)1,(float)0,number_of_objective_values,
			objective_values_step,objective_values);
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_device_objective.  "
			"Error extracting signal values.  %d %d %d",number_of_objective_values,
			save_number_of_objective_values,number_of_signals);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_device_objective */

int calculate_device_event_markers(struct Device *device,
	int start_search,int end_search,enum Event_detection_algorithm detection,
	float *objective_values,int number_of_objective_values,
	int objective_values_step,int number_of_events,int *search_interval_divisions,
	int threshold_percentage,int minimum_separation_milliseconds,float level)
/*******************************************************************************
LAST MODIFIED : 20 July 2004

DESCRIPTION :
Calculate the positions of the event markers for a signal/<device>/<device_node>
based upon the the start and end times, the number of events, the interval
divisions (if missing then equally spaced), the <detection> algorithm and the
<objective_values>.
==============================================================================*/
{
	int *events,i,number_of_calculated_events,return_code;
	struct Event *event,**event_next;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(calculate_device_event_markers);
	return_code=0;
	/* check arguments */
	signal=(struct Signal *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	if (device&&(signal=device->signal)&&(buffer=signal->buffer))
	{
		events=(int *)NULL;
		number_of_calculated_events=0;
		if (return_code=calculate_time_series_event_markers(start_search,end_search,
			detection,objective_values,number_of_objective_values,
			objective_values_step,number_of_events,search_interval_divisions,
			threshold_percentage,minimum_separation_milliseconds,level,
			buffer->frequency,&number_of_calculated_events,&events))
		{
			/* free the previous events */
			destroy_Event_list(&(signal->first_event));
			i=0;
			event=(struct Event *)NULL;
			event_next= &(signal->first_event);
			while (return_code&&(i<number_of_calculated_events))
			{
				if (event=create_Event(events[i],i+1,UNDECIDED,event,
					(struct Event *)NULL))
				{
					*event_next=event;
					event_next= &(event->next);
					i++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_device_event_markers.  Could not allocate event");
					destroy_Event_list(&(signal->first_event));
					return_code=0;
				}
			}
			DEALLOCATE(events);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_device_event_markers.  Invalid argument(s).  %p %p %p",device,
			signal,buffer);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_device_event_markers */

int analysis_write_signal_file(char *file_name,struct Rig *rig,int datum,
	int potential_time,int start_search_interval,int end_search_interval,
	char calculate_events,enum Event_detection_algorithm detection,
	int analysis_event_number,int analysis_number_of_events,
	int minimum_separation,int threshold,enum Datum_type datum_type,
	enum Edit_order edit_order,enum Signal_order signal_order,float level,
	int average_width)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

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
								if (0<((*device)->channel)->gain)
								{
									signal_minimum=(((*device)->channel)->offset)+
										signal_minimum/(((*device)->channel)->gain);
									signal_maximum=(((*device)->channel)->offset)+
										signal_maximum/(((*device)->channel)->gain);

								}
								else
								{
									if (0>((*device)->channel)->gain)
									{
										signal_minimum=(((*device)->channel)->offset)+
											signal_maximum/(((*device)->channel)->gain);
										signal_maximum=(((*device)->channel)->offset)+
											signal_minimum/(((*device)->channel)->gain);

									}
								}
								if ((1==BINARY_FILE_WRITE((char *)&((*device)->signal->status),
									sizeof(enum Event_signal_status),1,output_file))&&
									(1==BINARY_FILE_WRITE((char *)&signal_minimum,sizeof(float),1,
									output_file))&&
									(1==BINARY_FILE_WRITE((char *)&signal_maximum,sizeof(float),1,
									output_file)))
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
									if (1==BINARY_FILE_WRITE((char *)&number_of_events,
										sizeof(int),1,output_file))
									{
										event=start_event;
										while (return_code&&event&&(event->time<=buffer_end))
										{
											event_number=(event->number)-(start_event->number)+1;
											event_time=(event->time)-buffer_start;
											if ((1==BINARY_FILE_WRITE((char *)&(event_time),
												sizeof(int),1,output_file))&&
												(1==BINARY_FILE_WRITE((char *)&(event_number),
												sizeof(int),1,output_file))&&
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

int analysis_read_signal_file(char *file_name,struct Rig **rig_address,
	int *analysis_information,int *datum_address,char *calculate_events_address,
	enum Event_detection_algorithm *detection_address,int *event_number_address,
	int *number_of_events_address,int *potential_time_address,
	int *minimum_separation_address,int *threshold_address,
	enum Datum_type *datum_type_address,enum Edit_order *edit_order_address,
	enum Signal_order *signal_order_address,int *start_search_interval_address,
	int *end_search_interval_address,float *level_address,
	int *average_width_address
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES) */
	)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
Reads a signal file and the analysis information.  If there is analysis
information then <*analysis_information> set non-zero and the information is
set.
==============================================================================*/
{
	char calculate_events;
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
	struct Device **device;
	struct Event *event,**event_address;
	struct Rig *rig;
	struct Signal_buffer *buffer;

	ENTER(analysis_read_signal_file);
	return_code=0;
	/* check arguments */
	if (file_name&&rig_address&&analysis_information&&datum_address&&
		calculate_events_address&&detection_address&&event_number_address&&
		number_of_events_address&&potential_time_address&&
		minimum_separation_address&&threshold_address&&datum_type_address&&
		edit_order_address&&signal_order_address&&start_search_interval_address&&
		end_search_interval_address&&level_address&&average_width_address)
	{
		/* open the input file */
		rig=(struct Rig *)NULL;
		input_file=(FILE *)NULL;
		if ((input_file=fopen(file_name,"rb"))&&(read_signal_file(input_file,&rig
#if defined (UNEMAP_USE_3D)
			,unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
			))&&rig&&(rig->devices)&&(*(rig->devices))&&
			(buffer=get_Device_signal_buffer(*(rig->devices))))
		{
			*rig_address=rig;
			/* read the event detection settings */
			buffer_start=buffer->start;
			buffer_end=buffer->end;
			if ((1==BINARY_FILE_READ((char *)&datum,sizeof(int),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&calculate_events,sizeof(char),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&detection,
				sizeof(enum Event_detection_algorithm),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&event_number,sizeof(int),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&number_of_events,sizeof(int),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&potential_time,sizeof(int),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&minimum_separation,sizeof(int),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&threshold,sizeof(int),1,input_file))&&
				(1==BINARY_FILE_READ((char *)&datum_type,sizeof(enum Datum_type),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&edit_order,sizeof(enum Edit_order),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&signal_order,sizeof(enum Signal_order),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&start_search_interval,sizeof(int),1,
				input_file))&&
				(1==BINARY_FILE_READ((char *)&end_search_interval,sizeof(int),1,
				input_file)))
			{
				return_code=1;
				if (EDA_LEVEL==detection)
				{
					if (!((1==BINARY_FILE_READ((char *)&temp_int,sizeof(int),1,
						input_file))&&
						(1==BINARY_FILE_READ((char *)&level,sizeof(float),1,input_file))&&
						(1==BINARY_FILE_READ((char *)&average_width,sizeof(int),1,
						input_file))))
					{
						*level_address=level;
						*average_width_address=average_width;
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"analysis_read_signal_file.  Could not read level/average_width");
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
					if (average_width<1)
					{
						average_width=1;
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
					*threshold_address=threshold;
					*minimum_separation_address=minimum_separation;
					*datum_type_address=datum_type;
					*detection_address=detection;
					*edit_order_address=edit_order;
					*number_of_events_address=number_of_events;
					*calculate_events_address=calculate_events;
					*signal_order_address=signal_order;
					*datum_address=datum;
					*event_number_address=event_number;
					*potential_time_address=potential_time;
					*start_search_interval_address=start_search_interval;
					*end_search_interval_address=end_search_interval;
					/* for each signal read the status, range and events */
					if ((device=rig->devices)&&((i=rig->number_of_devices)>0))
					{
						while (return_code&&(i>0))
						{
							if (((*device)->channel)&&((*device)->signal))
							{
								/* read the status and range */
								/* if no signal status, a linear comb auxiliary device.  Do
									nothing */
								if ((1==BINARY_FILE_READ((char *)&event_status,
									sizeof(enum Event_signal_status),1,input_file))&&
									(1==BINARY_FILE_READ((char *)&((*device)->
									signal_display_minimum),sizeof(float),1,input_file))&&
									(1==BINARY_FILE_READ((char *)&((*device)->
									signal_display_maximum),sizeof(float),1,input_file)))
								{
									if ((ACCEPTED==event_status)||(REJECTED==event_status)||
										(UNDECIDED==event_status))
									{
										(*device)->signal->status=event_status;
										if ((*device)->signal_display_minimum<=
											(*device)->signal_display_maximum)
										{
											/*???DB.  Originally the unscaled maximum and minimum were
												stored.  This has to be maintained for backward
												compatability */
											if (0<((*device)->channel)->gain)
											{
												(*device)->signal_display_minimum=
													(((*device)->channel)->gain)*
													(((*device)->signal_display_minimum)-
													(((*device)->channel)->offset));
												(*device)->signal_display_maximum=
													(((*device)->channel)->gain)*
													(((*device)->signal_display_maximum)-
													(((*device)->channel)->offset));
											}
											else
											{
												(*device)->signal_display_minimum=
													(((*device)->channel)->gain)*
													(((*device)->signal_display_maximum)-
													(((*device)->channel)->offset));
												(*device)->signal_display_maximum=
													(((*device)->channel)->gain)*
													(((*device)->signal_display_minimum)-
													(((*device)->channel)->offset));
											}
										}
										/* read the events */
										if (1==BINARY_FILE_READ((char *)&number_of_events,
											sizeof(int),1,input_file))
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
															"analysis_read_signal_file.  "
															"Error creating event");
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
							}
							device++;
							i--;
						}
						if (return_code)
						{
							*analysis_information=1;
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
				*analysis_information=0;
			}
			fclose(input_file);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"analysis_read_signal_file.  Error reading signal file: %s",file_name);
			if (input_file)
			{
				fclose(input_file);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_read_signal_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_read_signal_file */
