/*******************************************************************************
FILE : optical_signals.c

LAST MODIFIED : 26 June 2000

DESCRIPTION :
Read in a signal file and a list of pairs of electrodes and write out a signal
file containing the differences of the rms scaled pairs.
==============================================================================*/

/*#define MOVING_AVERAGE*/
#define SPECTRAL

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#if defined (SPECTRAL)
#include "unemap/spectral_methods.h"
#endif /* defined (SPECTRAL) */
#include "user_interface/user_interface.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
==============================================================================*/
{
	char *name,*numerator_name,*denominator_name;
	FILE *ratio_file,*signal_file;
	float *denominator_value,*denominator_values,*numerator_value,
		*numerator_values,scale,*value;
	fpos_t location;
	int arg_number,c,i,index,number_of_pairs,number_of_samples,number_of_signals,
		return_code,*time;
	struct Device **device,**numerator,**denominator;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;
#if defined (SPECTRAL)
	float denominator_50,denominator_100,numerator_50,numerator_100,temp_value;
	struct Device *imaginary_device,*real_device;
	struct Signal_buffer *frequency_buffer;
#else /* defined (SPECTRAL) */
	float denominator_rms,numerator_rms;
#if defined (MOVING_AVERAGE)
	float denominator_sum_after,denominator_sum_before,*denominator_value_after,
		*denominator_value_before,numerator_sum_after,numerator_sum_before,
		*numerator_value_after,*numerator_value_before,temp_value;
	int moving_average_width;
#else /* defined (MOVING_AVERAGE) */
	float denominator_mean,numerator_mean;
#endif /* defined (MOVING_AVERAGE) */
#endif /* defined (SPECTRAL) */

	/* check arguments */
	return_code=0;
#if defined (MOVING_AVERAGE)
	if (5==argc)
#else /* defined (MOVING_AVERAGE) */
	if (4==argc)
#endif /* defined (MOVING_AVERAGE) */
	{
		arg_number=0;
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		arg_number++;
		if ((signal_file=fopen(argv[arg_number],"rb"))&&
			read_signal_file(signal_file,&signal_rig))
		{
			fclose(signal_file);
			/* open the ratio pairs file */
			arg_number++;
			if (ratio_file=fopen(argv[arg_number],"r"))
			{
#if defined (MOVING_AVERAGE)
				/* get the moving average width */
				arg_number++;
				if (1==sscanf(argv[arg_number],"%d",&moving_average_width))
				{
					if (moving_average_width<0)
					{
						moving_average_width=0;
					}
#endif /* defined (MOVING_AVERAGE) */
					/* count the number of pairs/singles and check the device(s) exist */
					number_of_pairs=0;
					return_code=1;
					numerator_name=(char *)NULL;
					denominator_name=(char *)NULL;
					while (return_code&&!feof(ratio_file))
					{
						if (read_string(ratio_file,"s",&numerator_name))
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
								/* check if there is a denominator */
								fgetpos(ratio_file,&location);
								do
								{
									c=fgetc(ratio_file);
								} while (isspace(c)&&('\n'!=c)&&(EOF!=c));
								fsetpos(ratio_file,&location);
								if (('\n'!=c)&&(EOF!=c))
								{
									/* denominator */
									if (read_string(ratio_file,"s",&denominator_name))
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
											!strcmp(denominator_name,(*denominator)->description->
											name))
										{
											/* check if there is a scale */
											fgetpos(ratio_file,&location);
											do
											{
												c=fgetc(ratio_file);
											} while (isspace(c)&&('\n'!=c)&&(EOF!=c));
											fsetpos(ratio_file,&location);
											if (('\n'!=c)&&(EOF!=c))
											{
												/* read scale */
												if (1==fscanf(ratio_file,"%f",&scale))
												{
													number_of_pairs++;
												}
												else
												{
													printf("ERROR.  Reading scale\n");
													return_code=0;
												}
											}
											else
											{
												/* no scale */
												number_of_pairs++;
											}
										}
										else
										{
											printf("ERROR.  Unknown denominator.  %s\n",
												denominator_name);
											return_code=0;
										}
										DEALLOCATE(denominator_name);
									}
									else
									{
										printf("ERROR.  Reading denominator\n");
										return_code=0;
									}
								}
								else
								{
									/* single */
									number_of_pairs++;
								}
							}
							else
							{
								printf("ERROR.  Unknown numerator.  %s\n",numerator_name);
								return_code=0;
							}
							DEALLOCATE(numerator_name);
						}
						else
						{
							printf("ERROR.  Reading numerator\n");
							return_code=0;
						}
						fscanf(ratio_file," ");
					}
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
							if ((ratio_rig=create_standard_Rig("ratios",PATCH,MONITORING_OFF,
								EXPERIMENT_OFF,1,&number_of_pairs,1,0,(float)1))
#if defined (SPECTRAL)
								&&(frequency_buffer=create_Signal_buffer(FLOAT_VALUE,2,1,1))&&
								(real_device=create_Device(0,create_Device_description(
								(char *)NULL,AUXILIARY,(struct Region *)NULL),create_Channel(0,
								0,1),create_Signal(0,frequency_buffer,REJECTED,2)))&&
								(imaginary_device=create_Device(1,create_Device_description(
								(char *)NULL,AUXILIARY,(struct Region *)NULL),create_Channel(1,
								0,1),create_Signal(1,frequency_buffer,REJECTED,2)))
#endif /* defined (SPECTRAL) */
								)
							{
								/* fill in the devices and signals */
								rewind(ratio_file);
								number_of_pairs=0;
								numerator_name=(char *)NULL;
								denominator_name=(char *)NULL;
								device=ratio_rig->devices;
								index=0;
								numerator_name=(char *)NULL;
								denominator_name=(char *)NULL;
								while (return_code&&!feof(ratio_file))
								{
									read_string(ratio_file,"s",&numerator_name);
									i=signal_rig->number_of_devices;
									numerator=signal_rig->devices;
									while ((i>0)&&(*numerator)&&((*numerator)->description)&&
										strcmp(numerator_name,(*numerator)->description->name))
									{
										numerator++;
										i--;
									}
									/* check if there is a denominator */
									fgetpos(ratio_file,&location);
									do
									{
										c=fgetc(ratio_file);
									} while (isspace(c)&&('\n'!=c)&&(EOF!=c));
									fsetpos(ratio_file,&location);
									if (('\n'!=c)&&(EOF!=c))
									{
										/* denominator */
										read_string(ratio_file,"s",&denominator_name);
										i=signal_rig->number_of_devices;
										denominator=signal_rig->devices;
										while ((i>0)&&(*denominator)&&
											((*denominator)->description)&&strcmp(denominator_name,
											(*denominator)->description->name))
										{
											denominator++;
											i--;
										}
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
													/* check if there is a ratio */
													fgetpos(ratio_file,&location);
													do
													{
														c=fgetc(ratio_file);
													} while (isspace(c)&&('\n'!=c)&&(EOF!=c));
													fsetpos(ratio_file,&location);
													if (('\n'!=c)&&(EOF!=c))
													{
														/* read scale */
														fscanf(ratio_file," %f",&scale);
														/*???debug */
														printf("%s %s %g\n",numerator_name,denominator_name,
															scale);
													}
													else
													{
#if defined (SPECTRAL)
														if (fourier_transform(SQUARE_WINDOW,*numerator,
															(struct Device *)NULL,real_device,
															imaginary_device))
														{
															numerator_50=0;
															i=2*(int)floor(50*(frequency_buffer->frequency))-
																2;
															if (i<0)
															{
																i=0;
															}
															else
															{
																if (i>frequency_buffer->number_of_samples)
																{
																	i=(frequency_buffer->number_of_samples)-1;
																}
															}
															value=(frequency_buffer->signals).float_values+i;
															i=(frequency_buffer->number_of_samples)-i;
															if (i>5)
															{
																i=5;
															}
															while (i>0)
															{
																temp_value=(*value)*(*value);
																value++;
																temp_value += (*value)*(*value);
																value++;
																if (temp_value>numerator_50)
																{
																	numerator_50=temp_value;
																}
																i--;
															}
															numerator_50=(float)sqrt((double)numerator_50);
															numerator_100=0;
															i=2*(int)floor(100*(frequency_buffer->frequency))-
																2;
															if (i<0)
															{
																i=0;
															}
															else
															{
																if (i>frequency_buffer->number_of_samples)
																{
																	i=(frequency_buffer->number_of_samples)-1;
																}
															}
															value=(frequency_buffer->signals).float_values+i;
															i=(frequency_buffer->number_of_samples)-i;
															if (i>5)
															{
																i=5;
															}
															while (i>0)
															{
																temp_value=(*value)*(*value);
																value++;
																temp_value += (*value)*(*value);
																value++;
																if (temp_value>numerator_100)
																{
																	numerator_100=temp_value;
																}
																i--;
															}
															numerator_100=(float)sqrt((double)numerator_100);
															if (fourier_transform(SQUARE_WINDOW,*denominator,
																(struct Device *)NULL,real_device,
																imaginary_device))
															{
																denominator_50=0;
																i=2*(int)floor(50*
																	(frequency_buffer->frequency))-2;
																if (i<0)
																{
																	i=0;
																}
																else
																{
																	if (i>frequency_buffer->number_of_samples)
																	{
																		i=(frequency_buffer->number_of_samples)-1;
																	}
																}
																value=
																	(frequency_buffer->signals).float_values+i;
																i=(frequency_buffer->number_of_samples)-i;
																if (i>5)
																{
																	i=5;
																}
																while (i>0)
																{
																	temp_value=(*value)*(*value);
																	value++;
																	temp_value += (*value)*(*value);
																	value++;
																	if (temp_value>denominator_50)
																	{
																		denominator_50=temp_value;
																	}
																	i--;
																}
																denominator_50=
																	(float)sqrt((double)denominator_50);
																denominator_100=0;
																i=2*(int)floor(100*
																	(frequency_buffer->frequency))-2;
																if (i<0)
																{
																	i=0;
																}
																else
																{
																	if (i>frequency_buffer->number_of_samples)
																	{
																		i=(frequency_buffer->number_of_samples)-1;
																	}
																}
																value=
																	(frequency_buffer->signals).float_values+i;
																i=(frequency_buffer->number_of_samples)-i;
																if (i>5)
																{
																	i=5;
																}
																while (i>0)
																{
																	temp_value=(*value)*(*value);
																	value++;
																	temp_value += (*value)*(*value);
																	value++;
																	if (temp_value>denominator_100)
																	{
																		denominator_100=temp_value;
																	}
																	i--;
																}
																denominator_100=
																	(float)sqrt((double)denominator_100);
																if (0<denominator_50)
																{
																	numerator_50 /= denominator_50;
																}
																if (0<denominator_100)
																{
																	numerator_100 /= denominator_100;
																}
																scale=(numerator_50+numerator_100)/2;
																/*???debug */
																printf("%s, %s, %g\n",numerator_name,
																	denominator_name,scale);
															}
															else
															{
																printf(
											"ERROR.  Could not calculate Fourier transform for %s\n",
																	numerator_name);
																return_code=0;
															}
														}
														else
														{
															printf(
											"ERROR.  Could not calculate Fourier transform for %s\n",
																numerator_name);
															return_code=0;
														}
#else /* defined (SPECTRAL) */
#if defined (MOVING_AVERAGE)
														if (0<number_of_samples)
														{
															if (number_of_samples<2*moving_average_width+1)
															{
																moving_average_width=(number_of_samples-1)/2;
															}
														}
														else
														{
															moving_average_width=0;
														}
														numerator_value_after=numerator_values;
														denominator_value_after=denominator_values;
														numerator_sum_after=0;
														denominator_sum_after=0;
														for (i=moving_average_width;i>0;i--)
														{
															numerator_value_after++;
															denominator_value_after++;
															numerator_sum_after += *numerator_value_after;
															denominator_sum_after += *denominator_value_after;
														}
														numerator_sum_before=0;
														denominator_sum_before=0;
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														for (i=0;i<moving_average_width;i++)
														{
															numerator_value_after++;
															denominator_value_after++;
															numerator_sum_after += (*numerator_value_after)-
																(*numerator_value);
															denominator_sum_after +=
																(*denominator_value_after)-(*denominator_value);
															temp_value=(numerator_sum_before+
																(*numerator_value)+numerator_sum_after)/
																(float)(i+1+moving_average_width);
															temp_value -= *numerator_value;
															numerator_rms += temp_value*temp_value;
															temp_value=(denominator_sum_before+
																(*denominator_value)+denominator_sum_after)/
																(float)(i+1+moving_average_width);
															temp_value -= *denominator_value;
															denominator_rms += temp_value*temp_value;
															numerator_sum_before += *numerator_value;
															denominator_sum_before += *denominator_value;
															numerator_value++;
															denominator_value++;
														}
														numerator_value_before=numerator_values;
														denominator_value_before=denominator_values;
														for (i=number_of_samples-2*moving_average_width;i>0;
															i--)
														{
															numerator_value_after++;
															denominator_value_after++;
															numerator_sum_after += (*numerator_value_after)-
																(*numerator_value);
															denominator_sum_after +=
																(*denominator_value_after)-(*denominator_value);
															temp_value=(numerator_sum_before+
																(*numerator_value)+numerator_sum_after)/
																(float)(1+2*moving_average_width);
															temp_value -= *numerator_value;
															numerator_rms += temp_value*temp_value;
															temp_value=(denominator_sum_before+
																(*denominator_value)+denominator_sum_after)/
																(float)(1+2*moving_average_width);
															temp_value -= *denominator_value;
															denominator_rms += temp_value*temp_value;
															numerator_sum_before += (*numerator_value)-
																(*numerator_value_before);
															denominator_sum_before += (*denominator_value)-
																(*denominator_value_before);
															numerator_value++;
															denominator_value++;
															numerator_value_before++;
															denominator_value_before++;
														}
														for (i=moving_average_width;i>0;i--)
														{
															numerator_sum_after -= *numerator_value;
															denominator_sum_after -= *denominator_value;
															temp_value=(numerator_sum_before+
																(*numerator_value)+numerator_sum_after)/
																(float)(i+1+moving_average_width);
															temp_value -= *numerator_value;
															numerator_rms += temp_value*temp_value;
															temp_value=(denominator_sum_before+
																(*denominator_value)+denominator_sum_after)/
																(float)(i+1+moving_average_width);
															temp_value -= *denominator_value;
															denominator_rms += temp_value*temp_value;
															numerator_sum_before += (*numerator_value)-
																(*numerator_value_before);
															denominator_sum_before += (*denominator_value)-
																(*denominator_value_before);
															numerator_value++;
															denominator_value++;
															numerator_value_before++;
															denominator_value_before++;
														}
#else /* defined (MOVING_AVERAGE) */
														/* calculate mean */
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														numerator_mean=0;
														denominator_mean=0;
														for (i=number_of_samples;i>0;i--)
														{
															numerator_mean += *numerator_value;
															denominator_mean += *denominator_value;
															numerator_value++;
															denominator_value++;
														}
														numerator_mean /= (float)number_of_samples;
														denominator_mean /= (float)number_of_samples;
														/* calculate rms */
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														numerator_rms=0;
														denominator_rms=0;
														for (i=number_of_samples;i>0;i--)
														{
															numerator_rms +=
																(*numerator_value-numerator_mean)*
																(*numerator_value-numerator_mean);
															denominator_rms +=
																(*denominator_value-denominator_mean)*
																(*denominator_value-denominator_mean);
															numerator_value++;
															denominator_value++;
														}
#endif /* defined (MOVING_AVERAGE) */
														numerator_rms /= (float)number_of_samples;
														denominator_rms /= (float)number_of_samples;
														numerator_rms=(float)sqrt((double)numerator_rms);
														denominator_rms=
															(float)sqrt((double)denominator_rms);
														if (0==denominator_rms)
														{
															scale=numerator_rms;
														}
														else
														{
															scale=numerator_rms/denominator_rms;
														}
														/*???debug */
														printf("%s %g, %s %g, %g\n",numerator_name,
															numerator_rms,denominator_name,denominator_rms,
															scale);
#endif /* defined (SPECTRAL) */
													}
													/* calculate difference */
													numerator_value=numerator_values;
													denominator_value=denominator_values;
													value=(signal_buffer->signals).float_values+index;
													for (i=number_of_samples;i>0;i--)
													{
														*value=(*numerator_value)-
															scale*(*denominator_value);
														value += number_of_signals;
														numerator_value++;
														denominator_value++;
													}
													number_of_pairs++;
													device++;
												}
												else
												{
													printf(
							"ERROR.  Could not extract numerator and denominator values\n");
													return_code=0;
												}
												DEALLOCATE(numerator_values);
												DEALLOCATE(denominator_values);
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
										DEALLOCATE(denominator_name);
									}
									else
									{
										/* single */
										/* change name */
										if (REALLOCATE(name,(*device)->description->name,char,
											strlen(numerator_name)+1))
										{
											(*device)->description->name=name;
											strcpy(name,numerator_name);
											/* create the signal */
											if ((*device)->signal=create_Signal(index,signal_buffer,
												UNDECIDED,0))
											{
												/* fill in the values */
												numerator_values=(float *)NULL;
												if (extract_signal_information((struct FE_node *)NULL,
													(struct Draw_package *)NULL,*numerator,1,1,0,
													(int *)NULL,(int *)NULL,(float **)NULL,
													&numerator_values,(enum Event_signal_status **)NULL,
													(char **)NULL,(int *)NULL,(float *)NULL,
													(float *)NULL))
												{
													/* fill in values */
													numerator_value=numerator_values;
													value=(signal_buffer->signals).float_values+index;
													for (i=number_of_samples;i>0;i--)
													{
														*value= *numerator_value;
														value += number_of_signals;
														numerator_value++;
													}
													number_of_pairs++;
													device++;
												}
												else
												{
													printf(
							"ERROR.  Could not extract numerator values\n");
													return_code=0;
												}
												DEALLOCATE(numerator_values);
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
									fscanf(ratio_file," ");
									DEALLOCATE(numerator_name);
									index++;
								}
								if (return_code)
								{
									arg_number++;
									if (signal_file=fopen(argv[arg_number],"wb"))
									{
										if (write_signal_file(signal_file,ratio_rig))
										{
											printf("Created signal file: %s\n",argv[arg_number]);
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
											argv[arg_number]);
										return_code=0;
									}
								}
							}
							else
							{
#if defined (SPECTRAL)
								printf(
		"ERROR.  Could not create ratios rig or real device or imaginary device\n");
#else /* defined (SPECTRAL) */
								printf("ERROR.  Could not create ratios rig\n");
#endif /* defined (SPECTRAL) */
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
						printf("ERROR.  Invalid or empty ratio pairs file.  %s\n",
							argv[arg_number]);
						return_code=0;
					}
#if defined (MOVING_AVERAGE)
				}
				else
				{
					printf("ERROR.  Could not read moving average width.  %s\n",
						argv[arg_number]);
					return_code=0;
				}
#endif /* defined (MOVING_AVERAGE) */
				fclose(ratio_file);
			}
			else
			{
				printf("ERROR.  Could not open ratio pairs file.  %s\n",
					argv[arg_number]);
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
#if defined (MOVING_AVERAGE)
		printf("usage: optical_signals in_signal_file optical_pairs_file moving_average_width out_signal_file\n");
#else /* defined (MOVING_AVERAGE) */
		printf("usage: optical_signals in_signal_file optical_pairs_file out_signal_file\n");
#endif /* defined (MOVING_AVERAGE) */
		printf("  in_signal_file is the name of the signal file to be differenced\n");
		printf("  optical_pairs_file is a list of space separated pairs electrodes to be differenced.  One pair to a line\n");
#if defined (MOVING_AVERAGE)
		printf("  moving_average_width is the width used for the moving average which is used to estimate the noise\n");
#endif /* defined (MOVING_AVERAGE) */
		printf("  out_signal_file is the name for the signal file that contains the differences\n");
		return_code=0;
	}

	return (0);
} /* main */
