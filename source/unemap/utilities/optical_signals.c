/*******************************************************************************
FILE : optical_signals.c

LAST MODIFIED : 7 June 2000

DESCRIPTION :
Read in a signal file and a list of pairs of electrodes and write out a signal
file containing the differences of the rms scaled pairs.
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

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
==============================================================================*/
{
	char *name,*numerator_name,*denominator_name;
	FILE *ratio_file,*signal_file;
	float denominator_mean,denominator_rms,*denominator_value,*denominator_values,
		numerator_mean,numerator_rms,*numerator_value,*numerator_values,scale,
		*value;
	int i,index,number_of_pairs,number_of_samples,number_of_signals,return_code,
		*time;
	struct Device **device,**numerator,**denominator;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;

	/* check arguments */
	return_code=0;
	if (4==argc)
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
													numerator_rms /= (float)number_of_samples;
													denominator_rms /= (float)number_of_samples;
													numerator_rms=(float)sqrt((double)numerator_rms);
													denominator_rms=(float)sqrt((double)denominator_rms);
													if (0==denominator_rms)
													{
														scale=numerator_rms;
													}
													else
													{
														scale=numerator_rms/denominator_rms;
													}
													/*???debug */
													printf("%s %g, %s %g\n",numerator_name,
														numerator_rms,denominator_name,denominator_rms);
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
								if (signal_file=fopen(argv[3],"wb"))
								{
									if (write_signal_file(signal_file,ratio_rig))
									{
										printf("Created signal file: %s\n",argv[3]);
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
									printf("ERROR.  Could not open new signal file %s\n",argv[3]);
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
		printf("usage: optical_signals in_signal_file optical_pairs_file out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be differenced\n");
		printf("  optical_pairs_file is a list of space separated pairs electrodes to be differenced.  One pair to a line\n");
		printf("  out_signal_file is the name for the signal file that contains the differences\n");
		return_code=0;
	}

	return (0);
} /* main */
