/*******************************************************************************
FILE : img2sig.c

LAST MODIFIED : 14 October 1998

DESCRIPTION :
Converts a plt file (stdin) to a cnfg file (stdout)
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
	/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include "general/debug.h"
#include "unemap/rig.h"

int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 7 October 1998

DESCRIPTION :
==============================================================================*/
{
	char *device_name;
	FILE *img_file,*signal_file;
	float sampling_frequency=1/0.00375;
	int column_number,columns=96,device_number,i,number_of_signals,
		number_of_samples,return_code=0,row_number,rows=96,*time;
	long int img_file_size;
	struct Channel *channel;
	struct Device **device,**devices;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *region_list;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *signal_buffer;

	/* check arguments */
	if ((3==argc)||(6==argc))
	{
		if (img_file=fopen(argv[1],"rb"))
		{
			if (signal_file=fopen(argv[2],"wb"))
			{
				return_code=1;
				if (6==argc)
				{
					if ((1==sscanf(argv[3],"%d",&rows))&&(0<rows))
					{
						if ((1==sscanf(argv[4],"%d",&columns))&&(0<columns))
						{
							if ((1==sscanf(argv[5],"%f",&sampling_frequency))&&
								(0<sampling_frequency))
							{
								/* OK */
							}
							else
							{
								printf("Invalid sampling_frequency: %s\n",argv[5]);
								return_code=0;
							}
						}
						else
						{
							printf("Invalid #columns: %s\n",argv[4]);
							return_code=0;
						}
					}
					else
					{
						printf("Invalid #rows: %s\n",argv[3]);
						return_code=0;
					}
				}
				if (return_code)
				{
					number_of_signals=rows*columns;
					/* determine the number of samples */
					number_of_samples=0;
					if (0==fseek(img_file,0,SEEK_END))
					{
						img_file_size=ftell(img_file);
						if ((0<img_file_size)&&
							(0==img_file_size%(long int)(2*number_of_signals)))
						{
							number_of_samples=
								(int)(img_file_size/(long int)(2*number_of_signals));
#if defined (DEBUG)
							/*???debug */
							printf("number_of_samples=%d\n",number_of_samples);
							/*???debug */
							printf("number_of_signals=%d\n",number_of_signals);
#endif /* defined (DEBUG) */
							if ((0<number_of_samples)&&(signal_buffer=create_Signal_buffer(
								SHORT_INT_VALUE,number_of_signals,number_of_samples,
								sampling_frequency)))
							{
								time=signal_buffer->times;
								for (i=0;i<number_of_samples;i++)
								{
									*time=i;
									time++;
								}
								rewind(img_file);
								if (number_of_signals*number_of_samples==
									fread((signal_buffer->signals).short_int_values,2,
									number_of_signals*number_of_samples,img_file))
								{
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									/* swap bytes */
#if defined (DEBUG)
									/*???debug */
									printf("swapping bytes\n");
#endif /* defined (DEBUG) */
									{
										char byte,*value;

										value=(char *)((signal_buffer->signals).short_int_values);
										for (i=number_of_signals*number_of_samples;i>0;i--)
										{
											byte= *value;
											*value=value[1];
											value++;
											*value=byte;
											value++;
										}
									}
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									/* less fluorescence means higher voltage, so negative gain */
									{
										short *value;

										value=(signal_buffer->signals).short_int_values;
										for (i=number_of_signals*number_of_samples;i>0;i--)
										{
											*value= -(*value);
											value++;
										}
									}
									ALLOCATE(devices,struct Device *,number_of_signals);
#if defined (OLD_CODE)
									ALLOCATE(device_name,char,3+(int)log10((double)rows)+
										(int)log10((double)columns));
#endif /* defined (OLD_CODE) */
									ALLOCATE(device_name,char,
										2+(int)log10((double)number_of_signals));
									if (devices&&device_name)
									{
										/* create the region */
										if ((region=create_Region("region",PATCH,0,
											number_of_signals))&&(region_list=
											create_Region_list_item(region,
											(struct Region_list_item *)NULL)))
										{
											/* create the devices */
											device=devices;
											device_number=0;
											row_number=0;
											while ((row_number<rows)&&return_code)
											{
												row_number++;
												column_number=0;
												while ((column_number<columns)&&return_code)
												{
													column_number++;
#if defined (OLD_CODE)
													sprintf(device_name,"%d %d",row_number,column_number);
#endif /* defined (OLD_CODE) */
													sprintf(device_name,"%d",device_number+1);
													if ((description=create_Device_description(
														device_name,ELECTRODE,region))&&(channel=
														create_Channel(device_number+1,(float)0,(float)1))&&
														(signal=create_Signal(device_number,signal_buffer,
														UNDECIDED,0))&&(*device=create_Device(device_number,
														description,channel,signal)))
													{
														description->properties.electrode.position.x=
															(float)column_number;
														description->properties.electrode.position.y=
															(float)-row_number;
														device_number++;
														device++;
													}
													else
													{
														printf(
															"ERROR.  Could not allocate memory for device\n");
														return_code=0;
													}
												}
											}
											if (return_code)
											{
												/* create the rig */
												if (rig=create_Rig("img",MONITORING_OFF,EXPERIMENT_OFF,
													number_of_signals,devices,
													(struct Page_list_item *)NULL,1,region_list,
													(struct Region *)NULL))
												{
													if (write_signal_file(signal_file,rig))
													{
														printf("Created signal file: %s\n",argv[2]);
													}
													else
													{
														printf("ERROR.  Writing signal file\n");
														return_code=0;
													}
												}
												else
												{
													printf("ERROR.  Could not create rig\n");
													return_code=0;
												}
											}
										}
										else
										{
											printf("ERROR.  Could not allocate memory for region\n");
											return_code=0;
										}
									}
									else
									{
										printf(
											"ERROR.  Could not allocate memory for device list\n");
										return_code=0;
									}
								}
								else
								{
									printf("ERROR.  Error reading img file\n");
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
						}
						else
						{
							printf("Invalid file size %ld\n",img_file_size);
							return_code=0;
						}
					}
					else
					{
						printf("Error in fseek\n");
						return_code=0;
					}
				}
				fclose(signal_file);
			}
			else
			{
				printf("Could not open signal file: %s\n",argv[2]);
				return_code=0;
			}
			fclose(img_file);
		}
		else
		{
			printf("Could not open img file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf(
	"usage: img2sig img_file signal_file <#rows #columns sampling_frequency>\n");
		printf("  img_file is the name of the IMG file (provided)\n");
		printf("  signal_file is the name of the signal file (created)\n");
		printf(
	"  #rows, #columns and sampling_frequency are optional.  The defaults are\n");
		printf("    96, 96, 1/0.00375 Hz\n");
		return_code=0;
	}

	return (return_code);
} /* main */
