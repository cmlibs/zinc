/*******************************************************************************
FILE : difference_signals.c

LAST MODIFIED : 9 July 1998

DESCRIPTION :
Allow the user to difference two signal files.  Does not use X-windows.  Usage:
	difference_signals signal_file1 signal_file2 offset differenced_file
signal_file1 and signal_file2 are the names of the signal files to difference
offset is the number of samples to drop from the beginning of the second signal
	file in order to align the files
differenced_file is the name of the differenced signal file
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
LAST MODIFIED : 9 July 1998

DESCRIPTION :
==============================================================================*/
{
	char *char_ptr;
	FILE *signal_file;
	int i,j,number_of_devices,number_of_regions,number_of_samples,
		number_of_signals,return_code,signal_offset,*time;
	struct Channel *channel;
	struct Device *device,**device_correspondence,**device1,**device2,
		**differenced_devices;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *differenced_region_list,
		*differenced_region_list_item,**region_list_item_address;
	struct Rig *differenced_rig,*signal_rig1,*signal_rig2;
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
						/* create the differenced signal buffer */
						/* determine the number of shared devices and the channel number
							correspondence */
						number_of_devices=signal_rig1->number_of_devices;
						if ((0<number_of_devices)&&ALLOCATE(device_correspondence,
							struct Device *,number_of_devices))
						{
							device1=signal_rig1->devices;
							number_of_signals=0;
							for (i=0;i<number_of_devices;i++)
							{
								device_correspondence[i]=(struct Device *)NULL;
								if (*device1)
								{
									device2=signal_rig2->devices;
									j=signal_rig2->number_of_devices;
									while ((j>0)&&!(device_correspondence[i]))
									{
										if (*device2)
										{
											if (0==strcmp((*device1)->description->name,
												(*device2)->description->name))
											{
												device_correspondence[i]= *device2;
												number_of_signals++;
											}
										}
										device2++;
										j--;
									}
								}
								device1++;
							}
							number_of_samples=
								((*(signal_rig2->devices))->signal->buffer->number_of_samples)-
								signal_offset;
							i=(*(signal_rig1->devices))->signal->buffer->number_of_samples;
							if (i<number_of_samples)
							{
								i=number_of_samples;
							}
							if ((number_of_samples>0)&&(number_of_signals>0)&&
								(signal_buffer=create_Signal_buffer(
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
										for (i=0;i<number_of_samples;i++)
										{
											device1=signal_rig1->devices;
											for (j=0;j<number_of_devices;j++)
											{
												if (device=device_correspondence[j])
												{
													signal=(*device1)->signal;
													value1=(signal->buffer->signals).float_values;
													value1 += i*(signal->buffer->number_of_signals)+
														(signal->index);
													signal=device->signal;
													value2=(signal->buffer->signals).float_values;
													value2 += (i+signal_offset)*(signal->buffer->
														number_of_signals)+(signal->index);
													*value=(*value1)-(*value2);
													value++;
												}
												device1++;
											}
										}
									} break;
									case SHORT_INT_VALUE:
									{
										short int *value,*value1,*value2;

										value=(signal_buffer->signals).short_int_values;
										for (i=0;i<number_of_samples;i++)
										{
											device1=signal_rig1->devices;
											for (j=0;j<number_of_devices;j++)
											{
												if (device=device_correspondence[j])
												{
													signal=(*device1)->signal;
													value1=(signal->buffer->signals).short_int_values;
													value1 += i*(signal->buffer->number_of_signals)+
														(signal->index);
													signal=device->signal;
													value2=(signal->buffer->signals).short_int_values;
													value2 += (i+signal_offset)*(signal->buffer->
														number_of_signals)+(signal->index);
													*value=(*value1)-(*value2);
													value++;
												}
												device1++;
											}
										}
									} break;
								}
								/* create the differenced region list */
								differenced_region_list=(struct Region_list_item *)NULL;
								region_list_item_address= &differenced_region_list;
								number_of_regions=0;
								return_code=1;
								i=0;
								device1=signal_rig1->devices;
								while (return_code&&(i<number_of_devices))
								{
									if (device_correspondence[i])
									{
										/* check if "new" region */
										device2=signal_rig1->devices;
										j=0;
										region=(*device1)->description->region;
										while ((j<i)&&(!device_correspondence[j]||
											(region!=(*device2)->description->region)))
										{
											j++;
											device2++;
										}
										if (j<i)
										{
											if (*region_list_item_address=create_Region_list_item(
												create_Region(region->name,region->type,
												number_of_regions,0),(struct Region_list_item *)NULL))
											{
												switch (region->type)
												{
													case SOCK:
													{
														((*region_list_item_address)->region->properties).
															sock.focus=(region->properties).sock.focus;
													} break;
												}
												number_of_regions++;
												region_list_item_address=
													&((*region_list_item_address)->next);
											}
											else
											{
												printf(
													"ERROR.  Could not create differenced region list\n");
												return_code=0;
											}
										}
									}
									i++;
									device1++;
								}
								if (return_code)
								{
									/* create the differenced device list */
									if (ALLOCATE(differenced_devices,struct Device *,
										number_of_signals))
									{
										i=0;
										j=0;
										device1=signal_rig1->devices;
										while (return_code&&(i<number_of_devices))
										{
											if (device_correspondence[i])
											{
												/* determine the region */
												region=(*device1)->description->region;
												differenced_region_list_item=differenced_region_list;
												while (differenced_region_list_item&&strcmp(
													differenced_region_list_item->region->name,
													region->name))
												{
													differenced_region_list_item=
														differenced_region_list_item->next;
												}
												if (differenced_region_list_item)
												{
													region=differenced_region_list_item->region;
													if ((description=create_Device_description(
														(*device1)->description->name,
														(*device1)->description->type,region))&&
														(channel=create_Channel(j+1,
														(*device1)->channel->offset,
														(*device1)->channel->gain))&&
														(signal=create_Signal(j,signal_buffer,
														(*device1)->signal->status,0))&&
														(differenced_devices[j]=create_Device(j,description,
														channel,signal)))
													{
														switch ((*device1)->description->type)
														{
															case ELECTRODE:
															{
																((differenced_devices[j])->description->
																	properties).electrode.position.x=
																	((*device1)->description->properties).
																	electrode.position.x;
																((differenced_devices[j])->description->
																	properties).electrode.position.y=
																	((*device1)->description->properties).
																	electrode.position.y;
																((differenced_devices[j])->description->
																	properties).electrode.position.z=
																	((*device1)->description->properties).
																	electrode.position.z;
															} break;
														}
														(region->number_of_devices)++;
														j++;
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
											device1++;
											i++;
										}
										if (return_code)
										{
											/* count the number of regions */
											number_of_regions=0;
											differenced_region_list_item=differenced_region_list;
											while (differenced_region_list_item)
											{
												differenced_region_list_item=
													differenced_region_list_item->next;
												number_of_regions++;
											}
											if (char_ptr=strrchr(argv[4],'.'))
											{
												*char_ptr='\0';
											}
											if (differenced_rig=create_Rig(argv[4],MONITORING_OFF,
												EXPERIMENT_OFF,number_of_devices,differenced_devices,
												(struct Page_list_item *)NULL,number_of_regions,
												differenced_region_list,(struct Region *)NULL))
											{
												if (char_ptr)
												{
													*char_ptr='.';
												}
												if ((signal_file=fopen(argv[4],"wb"))&&
													write_signal_file(signal_file,differenced_rig))
												{
													printf("Combined signal file created\n");
												}
												else
												{
													printf("ERROR.  Writing differenced signal file\n");
												}
											}
											else
											{
												printf("ERROR.  Could not create differenced rig\n");
											}
										}
									}
									else
									{
										printf(
											"ERROR.  Could not allocate differenced device list\n");
									}
								}
							}
							else
							{
								printf("ERROR.  Could not create differenced signal buffer\n");
								printf("  number_of_signals=%d\n",number_of_signals);
								printf("  number_of_samples=%d\n",number_of_samples);
							}
							DEALLOCATE(device_correspondence);
						}
						else
						{
							printf(
		"ERROR.  Could not allocate device correspondence.  number_of_devices=%d\n",
								number_of_devices);
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
		printf("usage: difference_signals signal_file1 signal_file2 offset differenced_file\n");
		printf("  signal_file1 and signal_file2 are the names of the signal files to difference\n");
		printf("  offset is the number of samples to drop from the beginning of the second signal\n");
		printf("    file in order to align the files\n");
		printf("  differenced_file is the name of the differenced signal file\n");
	}

	return (0);
} /* main */
