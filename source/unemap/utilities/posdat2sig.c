/*******************************************************************************
FILE : posdat2sig.c

LAST MODIFIED : 27 November 2001

DESCRIPTION :
Combines a .pos file and a .dat file (eeg mapping with Blake Johnson) into a
.sig file (unemap).
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
	/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include "general/debug.h"
#include "general/geometry.h"
#include "unemap/rig.h"

int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
==============================================================================*/
{
	char ascii_value[81],character,*device_name,file_name[81];
	FILE *dat_file,*pos_file,*signal_file;
	float sampling_frequency=1/0.00375,*signal_value,x,y,z;
	int device_number,i,j,k,number_of_signals,number_of_samples,return_code=0,
		*time;
	struct Channel *channel;
	struct Device **device,**devices;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *region_list;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *signal_buffer;

	/* check arguments */
	printf("Position file name ? ");
	scanf("%s",file_name);
	if (pos_file=fopen(file_name,"rb"))
	{
		/* count the number of signals */
		number_of_signals=0;
		k=0;
		while (1==fread(&character,sizeof(char),1,pos_file))
		{
			if (0x0d==character)
			{
				number_of_signals++;
				k=0;
			}
			else
			{
				k++;
			}
		}
		if (0!=k)
		{
			number_of_signals++;
		}
		if (0<number_of_signals)
		{
			printf("%d positions\n",number_of_signals);
			rewind(pos_file);
			printf("Data file name ? ");
			scanf("%s",file_name);
			if (dat_file=fopen(file_name,"rb"))
			{
				/* count the number of samples */
				number_of_samples=0;
				k=0;
				while ((1==fread(&character,sizeof(char),1,dat_file))&&
					(0x0d!=character))
				{
					if ((0x65==character)||(0x2e==character)||(0x2d==character)||
						((0x30<=character)&&(0x39>=character)))
					{
						k++;
					}
					else
					{
						number_of_samples++;
						k=0;
					}
				}
				if (0!=k)
				{
					number_of_samples++;
				}
				if (0<number_of_samples)
				{
					printf("%d samples\n",number_of_samples);
					i=1;
					j=0;
					k=0;
					while (1==fread(&character,sizeof(char),1,dat_file))
					{
						if (!((0x65==character)||(0x2e==character)||(0x2d==character)||
							((0x30<=character)&&(0x39>=character))))
						{
							if (0x0d==character)
							{
								i++;
								if (0!=k)
								{
									j++;
								}
								if (j!=number_of_samples)
								{
									printf("Different number of samples (%d) for position %d\n",j,
										i);
								}
								j=0;
								k=0;
							}
							else
							{
								if (0!=k)
								{
									j++;
								}
								k=0;
							}
						}
						else
						{
							k++;
						}
					}
					if (0!=j)
					{
						i++;
					}
					if (i!=number_of_signals)
					{
						printf("Different number of positions (%d) in data file\n",i);
					}
					rewind(dat_file);
					/* get sampling frequency */
					sampling_frequency=0;
					printf("Sampling frequency ? ");
					scanf("%f",&sampling_frequency);
					if (0<sampling_frequency)
					{
						printf("Signal file name ? ");
						scanf("%s",file_name);
						if (signal_file=fopen(file_name,"wb"))
						{
							return_code=1;
							if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,
								number_of_signals,number_of_samples,sampling_frequency))
							{
								time=signal_buffer->times;
								for (i=0;i<number_of_samples;i++)
								{
									*time=i;
									time++;
								}
								i=0;
								j=0;
								k=0;
								signal_value=(signal_buffer->signals).float_values;
								while ((i<number_of_signals)&&
									(1==fread(&character,sizeof(char),1,dat_file)))
								{
									if ((0x65==character)||(0x2e==character)||(0x2d==character)||
										((0x30<=character)&&(0x39>=character)))
									{
										ascii_value[k]=character;
										k++;
									}
									else
									{
										if (0x0d==character)
										{
											while (j<number_of_samples)
											{
												*signal_value=0;
												signal_value += number_of_signals;
												j++;
											}
											i++;
											j=0;
											k=0;
											signal_value=(signal_buffer->signals).float_values+i;
										}
										else
										{
											if (j<number_of_samples)
											{
												ascii_value[k]='\0';
												if (1!=sscanf(ascii_value,"%f",signal_value))
												{
													*signal_value=0;
												}
												signal_value += number_of_signals;
												j++;
												k=0;
											}
										}
									}
								}
								ALLOCATE(devices,struct Device *,number_of_signals);
								ALLOCATE(device_name,char,
									2+(int)log10((double)number_of_signals));
								if (devices&&device_name)
								{
									/* create the region */
									if ((region=create_Region("region",SOCK,0,
										number_of_signals))&&(region_list=
										create_Region_list_item(region,
										(struct Region_list_item *)NULL)))
									{
										region->properties.sock.focus=1;
										/* create the devices */
										device=devices;
										device_number=0;
										i=0;
										k=0;
										x=0;
										y=0;
										z=0;
										while (return_code&&(device_number<number_of_signals)&&
											(1==fread(&character,sizeof(char),1,pos_file)))
										{
											if ((0x65==character)||(0x2e==character)||
												(0x2d==character)||
												((0x30<=character)&&(0x39>=character)))
											{
												ascii_value[k]=character;
												k++;
											}
											else
											{
												ascii_value[k]='\0';
												switch (i)
												{
													case 0:
													{
														sscanf(ascii_value,"%f",&x);
													} break;
													case 1:
													{
														sscanf(ascii_value,"%f",&y);
													} break;
													case 2:
													{
														sscanf(ascii_value,"%f",&z);
													} break;
												}
												i++;
												k=0;
												if (0x0d==character)
												{
													sprintf(device_name,"%d",device_number+1);
													if ((description=create_Device_description(
														device_name,ELECTRODE,region))&&(channel=
														create_Channel(device_number+1,(float)0,(float)1))&&
														(signal=create_Signal(device_number,signal_buffer,
														UNDECIDED,0))&&(*device=create_Device(device_number,
														description,channel,signal)))
													{
														/* prolate axis is x */
														description->properties.electrode.position.x=z;
														description->properties.electrode.position.y=x;
														description->properties.electrode.position.z=y;
														device++;
														i=0;
														k=0;
														x=0;
														y=0;
														z=0;
														device_number++;
													}
													else
													{
														printf(
															"ERROR.  Could not allocate memory for device");
														return_code=0;
													}
												}
											}
										}
										if (return_code&&(device_number<number_of_signals))
										{
											sprintf(device_name,"%d",device_number+1);
											if ((description=create_Device_description(
												device_name,ELECTRODE,region))&&(channel=
												create_Channel(device_number+1,(float)0,(float)1))&&
												(signal=create_Signal(device_number,signal_buffer,
												UNDECIDED,0))&&(*device=create_Device(device_number,
												description,channel,signal)))
											{
												/* prolate axis is x */
												description->properties.electrode.position.x=z;
												description->properties.electrode.position.y=x;
												description->properties.electrode.position.z=y;
											}
											else
											{
												printf(
													"ERROR.  Could not allocate memory for device");
												return_code=0;
											}
										}
										if (return_code)
										{
											/* create the rig */
											if (rig=create_Rig("posdat",MONITORING_OFF,EXPERIMENT_OFF,
												number_of_signals,devices,
												(struct Page_list_item *)NULL,1,region_list,
												(struct Region *)NULL))
											{
												if (write_signal_file(signal_file,rig))
												{
													printf("Created signal file: %s\n",file_name);
												}
												else
												{
													printf("ERROR.  Writing signal file");
													return_code=0;
												}
											}
											else
											{
												printf("ERROR.  Could not create rig");
												return_code=0;
											}
										}
									}
									else
									{
										printf("ERROR.  Could not allocate memory for region");
										return_code=0;
									}
								}
								else
								{
									printf("ERROR.  Could not allocate memory for device list");
									return_code=0;
								}
							}
							else
							{
								printf("ERROR.  Could not create combined signal buffer\n");
								printf("  number_of_signals=%d\n",number_of_signals);
								printf("  number_of_samples=%d\n",number_of_samples);
								return_code=0;
							}
							fclose(signal_file);
						}
						else
						{
							printf("Could not open signal file: %s\n",file_name);
							return_code=0;
						}
					}
					else
					{
						printf("Invalid sampling fequency\n");
						return_code=0;
					}
				}
				else
				{
					printf("No samples\n");
					return_code=0;
				}
				fclose(dat_file);
			}
			else
			{
				printf("Could not open data file: %s\n",file_name);
				return_code=0;
			}
		}
		else
		{
			printf("No positions\n");
			return_code=0;
		}
		fclose(pos_file);
	}
	else
	{
		printf("Could not open position file: %s\n",file_name);
		return_code=0;
	}

	/* zero is the return code for success with shell programs -- hence the not: */
	return (!return_code);
} /* main */
