/*******************************************************************************
FILE : ratio_signals.c

LAST MODIFIED : 25 May 2000

DESCRIPTION :
Read in a signal file and a list of pairs of electrodes and write out a signal
file containing the ratios of the pairs.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "user_interface/user_interface.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
==============================================================================*/
{
	char *name,*numerator_name,*denominator_name;
	FILE *ratio_file,*signal_file;
	float *denominator_value,*denominator_values,*numerator_value,
		*numerator_values,signal_offset,*value;
	int i,number_of_pairs,number_of_samples,number_of_signals,return_code,*time;
	struct Device **device,**numerator,**denominator;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;

	/* check arguments */
	return_code=0;
	if (5==argc)
	{
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		if ((signal_file=fopen(argv[1],"rb"))&&read_signal_file(signal_file,
			&signal_rig))
		{
			fclose(signal_file);
			/* open the ratio pairs file */
			if (ratio_file=fopen(argv[2],"r"))
			{
				/* get the signal offset */
				if (1==sscanf(argv[3],"%f",&signal_offset))
				{
					/* count the number of pairs and check the devices exist */
					number_of_pairs=0;
					return_code=1;
					numerator_name=(char *)NULL;
					denominator_name=(char *)NULL;
					while (return_code&&read_string(ratio_file,"s",&numerator_name)&&
						read_string(ratio_file,"s",&denominator_name)&&!feof(ratio_file))
					{
						i=signal_rig->number_of_devices;
						numerator=signal_rig->devices;
						while ((i>0)&&(*numerator)&&((*numerator)->description)&&
							strcmp(numerator_name,(*numerator)->description->name))
						{
							numerator++;
							i--;
						}
						if ((i>0)&&(*numerator)&&((*numerator)->description)&&
							!strcmp(numerator_name,(*numerator)->description->name))
						{
							i=signal_rig->number_of_devices;
							denominator=signal_rig->devices;
							while ((i>0)&&(*denominator)&&((*denominator)->description)&&
								strcmp(denominator_name,(*denominator)->description->name))
							{
								denominator++;
								i--;
							}
							if ((i>0)&&(*denominator)&&((*denominator)->description)&&
								!strcmp(denominator_name,(*denominator)->description->name))
							{
								number_of_pairs++;
							}
							else
							{
								printf("ERROR.  Unknown denominator.  %s\n",denominator_name);
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Unknown numerator.  %s\n",numerator_name);
							return_code=0;
						}
						DEALLOCATE(numerator_name);
						DEALLOCATE(denominator_name);
					}
					DEALLOCATE(numerator_name);
					DEALLOCATE(denominator_name);
					if (return_code&&(0<number_of_pairs))
					{
						/* create ratios signal buffer */
						number_of_signals=number_of_pairs;
						number_of_samples=
							(*(signal_rig->devices))->signal->buffer->number_of_samples;
						if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,number_of_pairs,
							number_of_samples,
							(*(signal_rig->devices))->signal->buffer->frequency))
						{
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* create ratio rig */
							if (ratio_rig=create_standard_Rig("ratios",PATCH,MONITORING_OFF,
								EXPERIMENT_OFF,1,&number_of_pairs,1,0,(float)1))
							{
								/* fill in the devices and signals */
								rewind(ratio_file);
								number_of_pairs=0;
								numerator_name=(char *)NULL;
								denominator_name=(char *)NULL;
								device=ratio_rig->devices;
								while (return_code&&
									read_string(ratio_file,"s",&numerator_name)&&
									read_string(ratio_file,"s",&denominator_name)&&
									!feof(ratio_file))
								{
									i=signal_rig->number_of_devices;
									numerator=signal_rig->devices;
									while ((i>0)&&(*numerator)&&((*numerator)->description)&&
										strcmp(numerator_name,(*numerator)->description->name))
									{
										numerator++;
										i--;
									}
									if ((i>0)&&(*numerator)&&((*numerator)->description)&&
										!strcmp(numerator_name,(*numerator)->description->name))
									{
										i=signal_rig->number_of_devices;
										denominator=signal_rig->devices;
										while ((i>0)&&(*denominator)&&
											((*denominator)->description)&&strcmp(denominator_name,
											(*denominator)->description->name))
										{
											denominator++;
											i--;
										}
										if ((i>0)&&(*denominator)&&((*denominator)->description)&&
											!strcmp(denominator_name,
											(*denominator)->description->name))
										{
											/* change name */
											if (REALLOCATE(name,(*device)->description->name,char,
												strlen(numerator_name)+strlen(denominator_name)+2))
											{
												(*device)->description->name=name;
												strcpy(name,numerator_name);
												strcat(name,"/");
												strcat(name,denominator_name);
												/* create the signal */
												if ((*device)->signal=create_Signal(
													(*device)->channel->number,signal_buffer,UNDECIDED,0))
												{
													/* fill in the values */
													numerator_values=(float *)NULL;
													denominator_values=(float *)NULL;
													if (extract_signal_information((struct FE_node *)NULL,
														(struct Draw_package *)NULL,*numerator,1,1,0,
														(int *)NULL,(int *)NULL,(float **)NULL,
														&numerator_values,(enum Event_signal_status **)NULL,
														(char **)NULL,(int *)NULL,(float *)NULL,
														(float *)NULL)&&
														extract_signal_information((struct FE_node *)NULL,
														(struct Draw_package *)NULL,*denominator,1,1,0,
														(int *)NULL,(int *)NULL,(float **)NULL,
														&denominator_values,
														(enum Event_signal_status **)NULL,(char **)NULL,
														(int *)NULL,(float *)NULL,(float *)NULL))
													{
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														value=(signal_buffer->signals).float_values+
															((*device)->channel->number);
														for (i=number_of_samples;i>0;i--)
														{
															if (0!=(*denominator_value)+signal_offset)
															{
																*value=((*numerator_value)+signal_offset)/
																	((*denominator_value)+signal_offset);
															}
															else
															{
																*value=(float)0;
															}
															value += number_of_signals;
															numerator_value++;
															denominator_value++;
														}
														DEALLOCATE(numerator_values);
														DEALLOCATE(denominator_values);
														number_of_pairs++;
														device++;
													}
													else
													{
														printf(
								"ERROR.  Could not extract numerator and denominator values\n");
														return_code=0;
													}
												}
												else
												{
													printf("ERROR.  Could not create signal\n");
													return_code=0;
												}
											}
											else
											{
												printf("ERROR.  Could not reallocate device name\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Unknown denominator.  %s\n",
												denominator_name);
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Unknown numerator.  %s\n",numerator_name);
										return_code=0;
									}
									DEALLOCATE(numerator_name);
									DEALLOCATE(denominator_name);
								}
								DEALLOCATE(numerator_name);
								DEALLOCATE(denominator_name);
								if (return_code)
								{
									if (signal_file=fopen(argv[4],"wb"))
									{
										if (write_signal_file(signal_file,ratio_rig))
										{
											printf("Created signal file: %s\n",argv[4]);
										}
										else
										{
											printf("ERROR.  Writing signal file\n");
											return_code=0;
										}
										fclose(signal_file);
									}
									else
									{
										printf("ERROR.  Could not open new signal file %s\n",
											argv[4]);
										return_code=0;
									}
								}
							}
							else
							{
								printf("ERROR.  Could not create ratios rig\n");
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Could not create ratios signal buffer\n");
							printf("  number_of_signals=%d\n",number_of_pairs);
							printf("  number_of_samples=%d\n",number_of_samples);
							return_code=0;
						}
					}
					else
					{
						printf("ERROR.  Invalid or empty ratio pairs file.  %s\n",argv[2]);
						return_code=0;
					}
					fclose(ratio_file);
				}
				else
				{
					printf("ERROR.  Could not read signal offset\n");
					return_code=0;
				}
			}
			else
			{
				printf("ERROR.  Could not open ratio pairs file.  %s\n",argv[2]);
				return_code=0;
			}
		}
		else
		{
			printf("ERROR.  Could not read signal file.  %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: ratio_signals in_signal_file ratio_pairs_file signal_offset out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be ratioed\n");
		printf("  ratio_pairs_file is a list of space separated pairs electrodes to be ratioed.  One pair to a line.  Numerator first in each pair\n");
		printf("  signal_offset is an offset which is added to every signal before the ratios are calculated\n");
		printf("  out_signal_file is the name for the signal file that contains the ratios\n");
		return_code=0;
	}

	return (0);
} /* main */
