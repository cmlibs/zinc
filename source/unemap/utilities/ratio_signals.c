/*******************************************************************************
FILE : ratio_signals.c

LAST MODIFIED : 9 May 2002

DESCRIPTION :
Read in a signal file and a list of pairs of electrodes and write out a signal
file containing the ratios of the pairs.

CODE SWITCHS :
TESTING - try and reproduce Darren's ratio executable - not general code.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "user_interface/user_interface.h"

#define TESTING

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 9 May 2002

DESCRIPTION :
==============================================================================*/
{
#if defined (TESTING)
	char *name;
	FILE *offsets_file,*signal_file;
	float background_offset,*background_offsets,*denominator_value,
		*denominator_values,laser_average,laser_rms,moving_average,*numerator_value,
		*numerator_values,*saved_value,*saved_values,scale,signal_rms,temp_float,
		*value;
	int i,index,number_of_offsets,number_of_samples,number_of_signals,
		number_in_moving_average,return_code,*time;
	struct Device **denominator,**device,**numerator,**ratio_device;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;
#else /* defined (TESTING) */
	char *name,*numerator_name,*denominator_name;
	FILE *ratio_file,*signal_file;
	float *denominator_value,*denominator_values,*numerator_value,
		*numerator_values,signal_offset,*value;
	int i,index,number_of_pairs,number_of_samples,number_of_signals,return_code,
		*time;
	struct Device **device,**numerator,**denominator;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;
#endif /* defined (TESTING) */

	/* check arguments */
	return_code=0;
#if defined (TESTING)
	if (5==argc)
	{
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		if ((signal_file=fopen(argv[1],"rb"))&&read_signal_file(signal_file,
			&signal_rig))
		{
			fclose(signal_file);
			/* open the background offsets file */
			if (offsets_file=fopen(argv[2],"r"))
			{
				/* read in the offsets */
				number_of_offsets=0;
				return_code=1;
				background_offsets=(float *)NULL;
				while (return_code&&(1==fscanf(offsets_file,"%f",&background_offset)))
				{
					if (REALLOCATE(value,background_offsets,float,number_of_offsets+1))
					{
						background_offsets=value;
						background_offsets[number_of_offsets]=background_offset;
						number_of_offsets++;
					}
					else
					{
						printf("ERROR.  Could not reallocate background_offsets\n");
						return_code=0;
					}
				}
				if (return_code&&(16==signal_rig->number_of_devices)&&
					(12==number_of_offsets))
				{
					if ((1==sscanf(argv[3],"%d",&number_in_moving_average))&&
						(0<number_in_moving_average)&&ALLOCATE(saved_values,float,
						number_in_moving_average+1))
					{
						number_in_moving_average /= 2;
						number_in_moving_average *= 2;
						number_in_moving_average++;
						/* create ratios signal buffer */
						number_of_signals=
							(signal_rig->number_of_devices)+number_of_offsets+6;
						number_of_samples=
							(*(signal_rig->devices))->signal->buffer->number_of_samples;
						if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,
							number_of_signals,number_of_samples,
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
								EXPERIMENT_OFF,1,&number_of_signals,1,0,(float)1))
							{
								/* fill in the devices and signals */
								ratio_device=ratio_rig->devices;
								index=0;
								/* put in existing signals */
								device=signal_rig->devices;
								while (return_code&&(index<(signal_rig->number_of_devices)))
								{
									/* change name */
									if (REALLOCATE(name,(*ratio_device)->description->name,char,
										strlen((*device)->description->name)+1))
									{
										(*ratio_device)->description->name=name;
										strcpy(name,(*device)->description->name);
										/* create the signal */
										if ((*ratio_device)->signal=create_Signal(index,
											signal_buffer,UNDECIDED,0))
										{
											/* fill in the values */
											numerator_values=(float *)NULL;
											if (extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*device,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&numerator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL))
											{
												numerator_value=numerator_values;
												value=(signal_buffer->signals).float_values+index;
												for (i=number_of_samples;i>0;i--)
												{
													*value= *numerator_value;
													value += number_of_signals;
													numerator_value++;
												}
												DEALLOCATE(numerator_values);
												device++;
												ratio_device++;
												index++;
											}
											else
											{
												printf("ERROR.  Could not extract signal values\n");
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
								/* add background subtracted signals */
								device=signal_rig->devices;
								if (return_code&&
									extract_signal_information((struct FE_node *)NULL,
									(struct Signal_drawing_package *)NULL,device[3],1,
									1,0,(int *)NULL,(int *)NULL,(float **)NULL,
									&denominator_values,(enum Event_signal_status **)NULL,
									(char **)NULL,(int *)NULL,(float *)NULL,
									(float *)NULL))
								{
									saved_value=saved_values;
									temp_float=denominator_values[0];
									moving_average=(float)(number_in_moving_average/2)*temp_float;
									for (i=number_in_moving_average/2;i>0;i--)
									{
										*saved_value=temp_float;
										saved_value++;
									}
									denominator_value=denominator_values;
									for (i=number_in_moving_average/2+1;i>0;i--)
									{
										*saved_value= *denominator_value;
										moving_average += *denominator_value;
										denominator_value++;
										saved_value++;
									}
									saved_value=saved_values;
									laser_average=0;
									denominator_value=denominator_values;
									laser_rms=0;
									for (i=number_of_samples-(number_in_moving_average/2)-1;i>0;
										i--)
									{
										temp_float=(*denominator_value)-moving_average/
											(float)number_in_moving_average;
										laser_rms += temp_float*temp_float;
										laser_average += *denominator_value;
										moving_average -= *saved_value;
										*saved_value=denominator_value[number_in_moving_average/2];
										moving_average += *saved_value;
										saved_value++;
										if (number_in_moving_average<=saved_value-saved_values)
										{
											saved_value=saved_values;
										}
										denominator_value++;
									}
									for (i=(number_in_moving_average/2)+1;i>0;i--)
									{
										temp_float=(*denominator_value)-moving_average/
											(float)number_in_moving_average;
										laser_rms += temp_float*temp_float;
										laser_average += *denominator_value;
										moving_average -= *saved_value;
										*saved_value=denominator_values[number_of_samples-1];
										moving_average += *saved_value;
										saved_value++;
										if (number_in_moving_average<=saved_value-saved_values)
										{
											saved_value=saved_values;
										}
										denominator_value++;
									}
									laser_average /= (float)number_of_samples;
									laser_rms=(float)sqrt((double)laser_rms/
										(double)number_in_moving_average);
									denominator_value=denominator_values;
									for (i=number_of_samples;i>0;i--)
									{
										*denominator_value -= laser_average;
										denominator_value++;
									}
									device += 4;
									while (return_code&&(index<(signal_rig->number_of_devices)+
										number_of_offsets))
									{
										/* change name */
										if (REALLOCATE(name,(*ratio_device)->description->name,char,
											strlen((*device)->description->name)+12))
										{
											(*ratio_device)->description->name=name;
											strcpy(name,(*device)->description->name);
											strcat(name,"_b");
											/* create the signal */
											if ((*ratio_device)->signal=create_Signal(index,
												signal_buffer,UNDECIDED,0))
											{
												/* fill in the values */
												numerator_values=(float *)NULL;
												if (extract_signal_information((struct FE_node *)NULL,
													(struct Signal_drawing_package *)NULL,*device,1,
													1,0,(int *)NULL,(int *)NULL,(float **)NULL,
													&numerator_values,(enum Event_signal_status **)NULL,
													(char **)NULL,(int *)NULL,(float *)NULL,
													(float *)NULL))
												{
													saved_value=saved_values;
													temp_float=numerator_values[0];
													moving_average=
														(float)(number_in_moving_average/2)*temp_float;
													for (i=number_in_moving_average/2;i>0;i--)
													{
														*saved_value=temp_float;
														saved_value++;
													}
													numerator_value=numerator_values;
													for (i=number_in_moving_average/2+1;i>0;i--)
													{
														*saved_value= *numerator_value;
														moving_average += *numerator_value;
														numerator_value++;
														saved_value++;
													}
													saved_value=saved_values;
													numerator_value=numerator_values;
													signal_rms=0;
													for (
														i=number_of_samples-(number_in_moving_average/2)-1;
														i>0;i--)
													{
														temp_float=(*numerator_value)-moving_average/
															(float)number_in_moving_average;
														signal_rms += temp_float*temp_float;
														moving_average -= *saved_value;
														*saved_value=
															numerator_value[number_in_moving_average/2];
														moving_average += *saved_value;
														saved_value++;
														if (number_in_moving_average<=
															saved_value-saved_values)
														{
															saved_value=saved_values;
														}
														numerator_value++;
													}
													for (i=(number_in_moving_average/2)+1;i>0;i--)
													{
														temp_float=(*numerator_value)-moving_average/
															(float)number_in_moving_average;
														signal_rms += temp_float*temp_float;
														moving_average -= *saved_value;
														*saved_value=numerator_values[number_of_samples-1];
														moving_average += *saved_value;
														saved_value++;
														if (number_in_moving_average<=
															saved_value-saved_values)
														{
															saved_value=saved_values;
														}
														numerator_value++;
													}
													signal_rms=(float)sqrt((double)signal_rms/
														(double)number_in_moving_average);
													numerator_value=numerator_values;
													denominator_value=denominator_values;
													value=(signal_buffer->signals).float_values+index;
													scale=signal_rms/laser_rms;
													background_offset=
														background_offsets[index-number_of_offsets];
													for (i=number_of_samples;i>0;i--)
													{
														*value=(*numerator_value)-background_offset-
															(*denominator_value)*scale;
														value += number_of_signals;
														numerator_value++;
														denominator_value++;
													}
													DEALLOCATE(numerator_values);
													device++;
													ratio_device++;
													index++;
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
									DEALLOCATE(denominator_values);
								}
								else
								{
									if (return_code)
									{
										printf("ERROR.  Could not extract laser signal values\n");
										return_code=0;
									}
								}
								/* add ratio signals */
								numerator=(ratio_rig->devices)+(signal_rig->number_of_devices);
								denominator=numerator+1;
								while (return_code&&(index<number_of_signals))
								{
									/* change name */
									if (REALLOCATE(name,(*ratio_device)->description->name,char,
										strlen((*numerator)->description->name)+
										strlen((*denominator)->description->name)+2))
									{
										(*ratio_device)->description->name=name;
										strcpy(name,(*numerator)->description->name);
										strcat(name,"/");
										strcat(name,(*denominator)->description->name);
										/* create the signal */
										if ((*ratio_device)->signal=create_Signal(index,signal_buffer,
											UNDECIDED,0))
										{
											/* fill in the values */
											numerator_values=(float *)NULL;
											denominator_values=(float *)NULL;
											if (extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*numerator,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&numerator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL)&&
												extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*denominator,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&denominator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL))
											{
												numerator_value=numerator_values;
												denominator_value=denominator_values;
												value=(signal_buffer->signals).float_values+index;
												for (i=number_of_samples;i>0;i--)
												{
													if (0!=(*denominator_value))
													{
														*value=(*numerator_value)/(*denominator_value);
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
												ratio_device++;
												numerator += 2;
												denominator += 2;
												index++;
											}
											else
											{
												printf("ERROR.  Could not extract signal values\n");
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
							printf("  number_of_signals=%d\n",number_of_offsets);
							printf("  number_of_samples=%d\n",number_of_samples);
							return_code=0;
						}
						DEALLOCATE(saved_values);
					}
					else
					{
						printf("ERROR.  Invalid number_in_moving_average or "
							"could not allocate saved_values\n");
						printf("  number_in_moving_average=%s\n",argv[3]);
						return_code=0;
					}
				}
				else
				{
					if (return_code)
					{
						printf("ERROR.  Invalid background offsets or rig.  %d %d\n",
							number_of_offsets,signal_rig->number_of_devices);
						return_code=0;
					}
				}
				fclose(offsets_file);
			}
			else
			{
				printf("ERROR.  Could not open background offsets file.  %s\n",argv[2]);
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
		printf("usage: ratio_signals in_signal_file background_offsets_file number_in_moving_average out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be ratioed\n");
		printf("  background_offsets_file is a list of space separated offsets, one for each electrode\n");
		printf("  number_in_moving_average is the number of values to be used for the moving averages\n");
		printf("  out_signal_file is the name for the signal file that contains the ratios\n");
		return_code=0;
	}
#else /* defined (TESTING) */
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
								index=0;
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
												if ((*device)->signal=create_Signal(index,signal_buffer,
													UNDECIDED,0))
												{
													/* fill in the values */
													numerator_values=(float *)NULL;
													denominator_values=(float *)NULL;
													if (extract_signal_information((struct FE_node *)NULL,
														(struct Signal_drawing_package *)NULL,*numerator,1,
														1,0,(int *)NULL,(int *)NULL,(float **)NULL,
														&numerator_values,(enum Event_signal_status **)NULL,
														(char **)NULL,(int *)NULL,(float *)NULL,
														(float *)NULL)&&
														extract_signal_information((struct FE_node *)NULL,
														(struct Signal_drawing_package *)NULL,*denominator,
														1,1,0,(int *)NULL,(int *)NULL,(float **)NULL,
														&denominator_values,
														(enum Event_signal_status **)NULL,(char **)NULL,
														(int *)NULL,(float *)NULL,(float *)NULL))
													{
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														value=(signal_buffer->signals).float_values+index;
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
									index++;
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
#endif /* defined (TESTING) */

	return (return_code);
} /* main */
