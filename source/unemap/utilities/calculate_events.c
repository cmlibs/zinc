/*******************************************************************************
FILE : calculate_events.c

LAST MODIFIED : 19 November 2000

DESCRIPTION :
Reads in a signal file, calculates events and writes out the signal file with
events.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "general/debug.h"
#include "unemap/analysis.h"
#include "unemap/rig.h"
#include "user_interface/message.h"

int main(int argc,char *argv[])
{
	enum Event_detection_algorithm detection;
	enum Event_detection_objective objective;
	float level;
	FILE *signal_file;
	float *objective_values;
	int arg_number,average_width,end_search_interval,minimum_separation,
		number_of_devices,number_of_events,number_of_objective_values,
		objective_values_step,return_code,start_search_interval,threshold;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *buffer;

	/* check arguments */
	return_code=0;
	if (4<=argc)
	{
		arg_number=0;
		/* read the signal file */
		rig=(struct Rig *)NULL;
		arg_number++;
		if ((signal_file=fopen(argv[arg_number],"rb"))&&
			read_signal_file(signal_file,&rig))
		{
			fclose(signal_file);
			/* determine the detection method */
			arg_number++;
			if (0==strcmp("interval",argv[arg_number]))
			{
				detection=EDA_INTERVAL;
				if (7==argc)
				{
					/* determine objective */
					arg_number++;
					if (0==strcmp("absolute_slope",argv[arg_number]))
					{
						objective=ABSOLUTE_SLOPE;
						return_code=1;
					}
					else if (0==strcmp("negative_slope",argv[arg_number]))
					{
						objective=NEGATIVE_SLOPE;
						return_code=1;
					}
					else if (0==strcmp("positive_slope",argv[arg_number]))
					{
						objective=POSITIVE_SLOPE;
						return_code=1;
					}
					else if (0==strcmp("value",argv[arg_number]))
					{
						objective=VALUE_OBJECTIVE;
						return_code=1;
					}
					if (return_code)
					{
						/* read average width */
						arg_number++;
						if ((1==sscanf(argv[arg_number],"%d",&average_width))&&
							(0<average_width))
						{
							/* read the number of intervals */
							arg_number++;
							if (!((1==sscanf(argv[arg_number],"%d",&number_of_events))&&
								(0<number_of_events)))
							{
								printf("ERROR.  Invalid number of intervals: %s\n",
									argv[arg_number]);
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Invalid average width: %s\n",argv[arg_number]);
							return_code=0;
						}
					}
					else
					{
						printf("ERROR.  Invalid objective: %s\n",argv[arg_number]);
					}
				}
				else
				{
					printf("ERROR.  Invalid number of arguments for interval method\n");
				}
			}
			else
			{
				printf("ERROR.  Invalid method: %s\n",argv[arg_number]);
			}
			if (return_code)
			{
				/* for each device/signal calculate the event markers */
				device=rig->devices;
				number_of_devices=rig->number_of_devices;
				objective_values_step=1;
				threshold=0;
				minimum_separation=10;
				level=0;
				start_search_interval=0;
				end_search_interval= -1;
				while (number_of_devices>0)
				{
					if (((*device)->signal)&&((*device)->signal->status!=REJECTED)&&
						(buffer=(*device)->signal->buffer)&&(0<(number_of_objective_values=
						buffer->number_of_samples)))
					{
						if ((end_search_interval<0)||
							(number_of_objective_values>end_search_interval))
						{
							end_search_interval=number_of_objective_values;
						}
						if (ALLOCATE(objective_values,float,number_of_objective_values))
						{
							calculate_device_objective(*device,detection,objective,
								objective_values,number_of_objective_values,
								objective_values_step,average_width);
							calculate_device_event_markers(*device,start_search_interval,
								number_of_objective_values-1,detection,objective_values,
								number_of_objective_values,objective_values_step,
								number_of_events,threshold,minimum_separation,level);
							DEALLOCATE(objective_values);
						}
					}
					device++;
					number_of_devices--;
				}
				if (0<end_search_interval)
				{
					end_search_interval--;
					/* write the signal file */
					arg_number++;
					if (!(return_code=analysis_write_signal_file(argv[arg_number],rig,0,
						(start_search_interval+end_search_interval)/2,start_search_interval,
						end_search_interval,(char)0,detection,1,number_of_events,
						minimum_separation,threshold,AUTOMATIC_DATUM,DEVICE_ORDER,
						CHANNEL_ORDER,level,average_width)))
					{
						printf("ERROR.  Writing signal file %s\n",argv[arg_number]);
					}
				}
				else
				{
					printf("ERROR.  Calculating events\n");
				}
			}
		}
		else
		{
			printf("ERROR.  Could not read signal file.  %s\n",argv[arg_number]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: calculate_events in_signal_file method <method dependent options> out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to calculate events for\n");
		printf("  method is the detection method to be used.  Allowable options: interval\n");
		printf("  <method dependent options>\n");
		printf("    for interval method\n");
		printf("    objective  can be absolute_slope, negative_slope, positive_slope or value\n");
		printf("    average_width  number of values before and after to average over\n");
		printf("    number_of_intervals  look for an event in each interval\n");
		printf("  out_signal_file is the name of the signal file with the calculated events\n");
		return_code=0;
	}

	return (return_code);
} /* main */
