/*******************************************************************************
FILE : combine_signals.c

LAST MODIFIED : 9 July 1998

DESCRIPTION :
Allow the user to combine two signal files.  Does not use X-windows.  Usage:
	combine_signals signal_file1 signal_file2 offset combined file
signal_file1 and signal_file2 are the names of the signal files to combine
offset is the number of samples to drop from the beginning of the second signal
	file in order to align the files
combined_file is the name of the combined signal file
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "general/debug.h"
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 13 May 1998

DESCRIPTION :
==============================================================================*/
{
	char *char_ptr;
	FILE *signal_file;
	int i,index_offset,j,number_of_devices,number_of_regions,number_of_samples,
		number_of_signals,return_code,signal_offset,*time;
	struct Channel *channel;
	struct Device **combined_devices,**device;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *combined_region_list,*combined_region_list_item,
		*region_list_item,**region_list_item_address;
	struct Rig *combined_rig,*signal_rig1,*signal_rig2;
	struct Signal *signal;
	struct Signal_buffer *signal_buffer;

	/* check arguments */
	if (5==argc)
	{
		/* read the first signal file */
		signal_rig1=(struct Rig *)NULL;
		if ((signal_file=fopen(argv[1],"rb"))&&read_signal_file(signal_file,
			&signal_rig1))
		{
			fclose(signal_file);
			/* read the second signal file */
			signal_rig2=(struct Rig *)NULL;
			if ((signal_file=fopen(argv[2],"rb"))&&read_signal_file(signal_file,
				&signal_rig2))
			{
				fclose(signal_file);
				/* check that the configurations are consistent */
				if (((*(signal_rig1->devices))->signal->buffer->frequency==
					(*(signal_rig2->devices))->signal->buffer->frequency)&&
					((*(signal_rig1->devices))->signal->buffer->value_type==
					(*(signal_rig2->devices))->signal->buffer->value_type))
				{
					/* get the signal offset */
					if ((1==sscanf(argv[3],"%d",&signal_offset))&&(0<=signal_offset))
					{
						/* create the combined signal buffer */
						number_of_signals=
							((*(signal_rig1->devices))->signal->buffer->number_of_signals)+
							((*(signal_rig2->devices))->signal->buffer->number_of_signals);
						number_of_samples=
							((*(signal_rig2->devices))->signal->buffer->number_of_samples)-
							signal_offset;
						i=(*(signal_rig1->devices))->signal->buffer->number_of_samples;
						if (i<number_of_samples)
						{
							i=number_of_samples;
						}
						if ((number_of_samples>0)&&(signal_buffer=create_Signal_buffer(
							(*(signal_rig1->devices))->signal->buffer->value_type,
							number_of_signals,number_of_samples,
							(*(signal_rig1->devices))->signal->buffer->frequency)))
						{
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							switch (signal_buffer->value_type)
							{
								case FLOAT_VALUE:
								{
									float *value,*value1,*value2;

									value=(signal_buffer->signals).float_values;
									value1=((*(signal_rig1->devices))->signal->buffer->signals).
										float_values;
									value2=((*(signal_rig2->devices))->signal->buffer->signals).
										float_values;
									value2 += signal_offset*((*(signal_rig2->devices))->signal->
										buffer->number_of_signals);
									for (i=number_of_samples;i>0;i--)
									{
										for (j=(*(signal_rig1->devices))->signal->buffer->
											number_of_signals;j>0;j--)
										{
											*value= *value1;
											value++;
											value1++;
										}
										for (j=(*(signal_rig2->devices))->signal->buffer->
											number_of_signals;j>0;j--)
										{
											*value= *value2;
											value++;
											value2++;
										}
									}
								} break;
								case SHORT_INT_VALUE:
								{
									short int *value,*value1,*value2;

									value=(signal_buffer->signals).short_int_values;
									value1=((*(signal_rig1->devices))->signal->buffer->signals).
										short_int_values;
									value2=((*(signal_rig2->devices))->signal->buffer->signals).
										short_int_values;
									value2 += signal_offset*((*(signal_rig2->devices))->signal->
										buffer->number_of_samples);
									for (i=number_of_samples;i>0;i--)
									{
										for (j=(*(signal_rig1->devices))->signal->buffer->
											number_of_signals;j>0;j--)
										{
											*value= *value1;
											value++;
											value1++;
										}
										for (j=(*(signal_rig2->devices))->signal->buffer->
											number_of_signals;j>0;j--)
										{
											*value= *value2;
											value++;
											value2++;
										}
									}
								} break;
							}
							/* create the combined region list */
							combined_region_list=(struct Region_list_item *)NULL;
							region_list_item_address= &combined_region_list;
							number_of_regions=0;
							return_code=1;
							region_list_item=signal_rig1->region_list;
							while (return_code&&region_list_item)
							{
								if (*region_list_item_address=create_Region_list_item(
									create_Region(region_list_item->region->name,
									region_list_item->region->type,number_of_regions,0),
									(struct Region_list_item *)NULL))
								{
									switch (region_list_item->region->type)
									{
										case SOCK:
										{
											((*region_list_item_address)->region->properties).sock.
												focus=
												(region_list_item->region->properties).sock.focus;
										} break;
									}
									number_of_regions++;
									region_list_item=region_list_item->next;
									region_list_item_address=
										&((*region_list_item_address)->next);
								}
								else
								{
									printf("ERROR.  Could not combine region lists\n");
									return_code=0;
								}
							}
							region_list_item=signal_rig2->region_list;
							while (return_code&&region_list_item)
							{
								combined_region_list_item=combined_region_list;
								while (combined_region_list_item&&strcmp(
									combined_region_list_item->region->name,
									region_list_item->region->name))
								{
									combined_region_list_item=combined_region_list_item->next;
								}
								if (!combined_region_list_item)
								{
									if (*region_list_item_address=create_Region_list_item(
										create_Region(region_list_item->region->name,
										region_list_item->region->type,number_of_regions,0),
										(struct Region_list_item *)NULL))
									{
										switch (region_list_item->region->type)
										{
											case SOCK:
											{
												((*region_list_item_address)->region->properties).sock.
													focus=
													(region_list_item->region->properties).sock.focus;
											} break;
										}
										region_list_item=region_list_item->next;
										number_of_regions++;
										region_list_item_address=
											&((*region_list_item_address)->next);
									}
									else
									{
										printf("ERROR.  Could not combine region lists\n");
										return_code=0;
									}
								}
							}
							if (return_code)
							{
								/* create the combined device list */
								number_of_devices=(signal_rig1->number_of_devices)+
									(signal_rig2->number_of_devices);
								if (ALLOCATE(combined_devices,struct Device *,
									number_of_devices))
								{
									number_of_devices=0;
									i=signal_rig1->number_of_devices;
									device=signal_rig1->devices;
									while (return_code&&(i>0))
									{
										/* determine the region */
										region=(*device)->description->region;
										combined_region_list_item=combined_region_list;
										while (combined_region_list_item&&strcmp(
											combined_region_list_item->region->name,region->name))
										{
											combined_region_list_item=combined_region_list_item->next;
										}
										if (combined_region_list_item)
										{
											region=combined_region_list_item->region;
											if ((description=create_Device_description((*device)->
												description->name,(*device)->description->type,
												region))&&(channel=create_Channel(number_of_devices+1,
												(*device)->channel->offset,(*device)->channel->gain))&&
												(signal=create_Signal((*device)->signal->index,
												signal_buffer,(*device)->signal->status,0))&&
												(combined_devices[number_of_devices]=create_Device(
												number_of_devices,description,channel,signal)))
											{
												switch ((*device)->description->type)
												{
													case ELECTRODE:
													{
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.x=
															((*device)->description->properties).electrode.
															position.x;
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.y=
															((*device)->description->properties).electrode.
															position.y;
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.z=
															((*device)->description->properties).electrode.
															position.z;
													} break;
												}
												number_of_devices++;
												(region->number_of_devices)++;
												device++;
												i--;
											}
											else
											{
												printf("ERROR.  Creating device\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Unknown region\n");
											return_code=0;
										}
									}
									i=signal_rig2->number_of_devices;
									device=signal_rig2->devices;
									index_offset=(*(signal_rig1->devices))->signal->buffer->
										number_of_signals;
									while (return_code&&(i>0))
									{
										/* determine the region */
										region=(*device)->description->region;
										combined_region_list_item=combined_region_list;
										while (combined_region_list_item&&strcmp(
											combined_region_list_item->region->name,region->name))
										{
											combined_region_list_item=combined_region_list_item->next;
										}
										if (combined_region_list_item)
										{
											region=combined_region_list_item->region;
											if ((description=create_Device_description((*device)->
												description->name,(*device)->description->type,
												region))&&(channel=create_Channel(number_of_devices+1,
												(*device)->channel->offset,(*device)->channel->gain))&&
												(signal=create_Signal(index_offset+((*device)->signal->
												index),signal_buffer,(*device)->signal->status,0))&&
												(combined_devices[number_of_devices]=create_Device(
												number_of_devices,description,channel,signal)))
											{
												switch ((*device)->description->type)
												{
													case ELECTRODE:
													{
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.x=
															((*device)->description->properties).electrode.
															position.x;
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.y=
															((*device)->description->properties).electrode.
															position.y;
														((combined_devices[number_of_devices])->
															description->properties).electrode.position.z=
															((*device)->description->properties).electrode.
															position.z;
													} break;
												}
												number_of_devices++;
												(region->number_of_devices)++;
												device++;
												i--;
											}
											else
											{
												printf("ERROR.  Creating device\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Unknown region\n");
											return_code=0;
										}
									}
									if (return_code)
									{
										/* count the number of regions */
										number_of_regions=0;
										combined_region_list_item=combined_region_list;
										while (combined_region_list_item)
										{
											combined_region_list_item=combined_region_list_item->next;
											number_of_regions++;
										}
										if (char_ptr=strrchr(argv[4],'.'))
										{
											*char_ptr='\0';
										}
										if (combined_rig=create_Rig(argv[4],MONITORING_OFF,
											EXPERIMENT_OFF,number_of_devices,combined_devices,
											(struct Page_list_item *)NULL,number_of_regions,
											combined_region_list,(struct Region *)NULL))
										{
											if (char_ptr)
											{
												*char_ptr='.';
											}
											if ((signal_file=fopen(argv[4],"wb"))&&write_signal_file(
												signal_file,combined_rig))
											{
												printf("Combined signal file created\n");
											}
											else
											{
												printf("ERROR.  Writing combined signal file\n");
											}
										}
										else
										{
											printf("ERROR.  Could not create combined rig\n");
										}
									}
								}
								else
								{
									printf("ERROR.  Could not allocate combined device list\n");
								}
							}
						}
						else
						{
							printf("ERROR.  Could not create combined signal buffer\n");
							printf("  number_of_signals=%d\n",number_of_signals);
							printf("  number_of_samples=%d\n",number_of_samples);
						}
					}
					else
					{
						printf("ERROR.  Offset must be a non-negative integer.  %s\n",
							argv[3]);
					}
				}
				else
				{
					printf(
						"ERROR.  Signal files have different frequencies or value types\n");
				}
			}
			else
			{
				printf("ERROR.  Could not read second signal file.  %s\n",argv[2]);
			}
		}
		else
		{
			printf("ERROR.  Could not read first signal file.  %s\n",argv[1]);
		}
	}
	else
	{
		printf("usage: combine_signals signal_file1 signal_file2 offset combined_file\n");
		printf("  signal_file1 and signal_file2 are the names of the signal files to combine\n");
		printf("  offset is the number of samples to drop from the beginning of the second signal\n");
		printf("    file in order to align the files\n");
		printf("  combined_file is the name of the combined signal file\n");
	}

	return (0);
} /* main */
