/*******************************************************************************
FILE : activation_summary.c

LAST MODIFIED : 13 November 2002

DESCRIPTION :
Writes out a summary of the activation in a signal file.
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
#include "user_interface/message.h"

int main(int argc,char *argv[])
{
	char calculate_events;
	enum Datum_type datum_type;
	enum Edit_order edit_order;
	enum Event_detection_algorithm detection;
	enum Signal_order signal_order;
	FILE *output_file;
	float activation_width,frequency,level;
	int analysis_information,arg_number,average_width,datum,end_search_interval,
		event_number,i,j,k,minimum_separation,number_of_devices,number_of_events,
		number_of_regions,number_of_samples,number_of_signals,potential_time,
		return_code,start_search_interval,*summary_time,threshold,*time;
	short int *short_value;
	struct Device **device;
	struct Event *event;
	struct Region_list_item *region_item;
	struct Rig *signal_rig,*summary_rig;
	struct Signal_buffer *buffer,*summary_buffer;

	/* check arguments */
	return_code=0;
	if ((4==argc)||(5==argc))
	{
		arg_number=0;
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		arg_number++;
		if (analysis_read_signal_file(argv[arg_number],&signal_rig,
			&analysis_information,&datum,&calculate_events,&detection,&event_number,
			&number_of_events,&potential_time,&minimum_separation,&threshold,
			&datum_type,&edit_order,&signal_order,&start_search_interval,
			&end_search_interval,&level,&average_width
#if defined (UNEMAP_USE_3D)
			,(struct Unemap_package *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
			))
		{
			device=signal_rig->devices;
			if (device&&(*device)&&((*device)->signal)&&
				(buffer=(*device)->signal->buffer)&&((*device)->channel))
			{
				number_of_regions=signal_rig->number_of_regions;
				frequency=buffer->frequency;
				number_of_devices=signal_rig->number_of_devices;
				number_of_signals=buffer->number_of_signals;
				number_of_samples=buffer->number_of_samples;
				/* create the summary rig */
				if ((summary_buffer=create_Signal_buffer(SHORT_INT_VALUE,
					number_of_regions,number_of_samples,frequency))&&
					(summary_rig=create_standard_Rig("summary",PATCH,MONITORING_OFF,
					EXPERIMENT_OFF,1,&number_of_regions,1,0,(float)1)))
				{
					/* fill in the devices and signals */
					return_code=1;
					device=summary_rig->devices;
					region_item=signal_rig->region_list;
					i=0;
					while (return_code&&(i<number_of_regions))
					{
						if (ALLOCATE((*device)->description->name,char,
							strlen(region_item->region->name)+1))
						{
							strcpy((*device)->description->name,region_item->region->name);
							if ((*device)->signal=create_Signal(i,summary_buffer,
								REJECTED,0))
							{
								i++;
								region_item=region_item->next;
								device++;
							}
							else
							{
								printf("ERROR.  Could not create region signal");
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Could not allocate region device name");
							return_code=0;
						}
					}
					if (return_code)
					{
						/* zero the summary values */
						short_value=(summary_buffer->signals).short_int_values;
						for (i=number_of_samples*number_of_regions;i>0;i--)
						{
							*short_value=(short int)0;
							short_value++;
						}
						/* set the summary times */
						time=buffer->times;
						summary_time=summary_buffer->times;
						for (i=number_of_samples;i>0;i--)
						{
							*summary_time= *time;
							time++;
							summary_time++;
						}
						/* read the activation width */
						arg_number++;
						if (1==sscanf(argv[arg_number],"%f",&activation_width))
						{
							activation_width=activation_width*frequency/(float)1000;
							/* calculate the summary */
							short_value=(summary_buffer->signals).short_int_values;
							device=signal_rig->devices;
							summary_time=summary_buffer->times;
							for (i=number_of_devices;i>0;i--)
							{
								if ((*device)&&((*device)->signal)&&
									(event=(*device)->signal->first_event))
								{
									j=0;
									region_item=signal_rig->region_list;
									while (region_item&&
										((*device)->description->region!=region_item->region))
									{
										region_item=region_item->next;
										j++;
									}
									if ((j<number_of_regions)&&
										((*device)->description->region==region_item->region))
									{
										while (event)
										{
											k=event->time;
											while ((k<number_of_samples)&&((float)summary_time[k]<=
												(float)summary_time[event->time]+activation_width))
											{
												(short_value[k*number_of_regions+j])++;
												k++;
											}
											event=event->next;
										}
									}
								}
								device++;
							}
							/* open the summary signal file */
							arg_number++;
							if (output_file=fopen(argv[arg_number],"w"))
							{
								write_signal_file(output_file,summary_rig);
								fclose(output_file);
							}
							else
							{
								printf("ERROR.  Could not open summary signal file.  %s\n",
									argv[arg_number]);
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Could not read activation width.  %s\n",
								argv[arg_number]);
							return_code=0;
						}
					}
				}
				else
				{
					printf("ERROR.  Could not create summary rig");
					return_code=0;
				}
			}
			else
			{
				printf("ERROR.  No devices");
				return_code=0;
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
		printf("usage: activation_summary in_signal_file activation_width "
			"summary_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be "
			"summarised\n");
		printf("  activation_width is the length of time, in milliseconds, after "
			"activation that an electrode is considered active\n");
		printf("  summary_signal_file is the name for the signal file that is "
			"output.  For each region, a time history of the number of active "
			"electrodes\n");
		return_code=0;
	}

	return (return_code);
} /* main */
