/*******************************************************************************
FILE : text2sig.c

LAST MODIFIED : 14 July 2002

DESCRIPTION :
Reads in a text signal file.  Finds the end of the header as the last line
starting with a non-number
Two modes
1 Configuration file supplied
	Reads in the configuration file and matchs the device names to the column
	names (first is time) on the last line of the header.  Writes out a signal
	file.
2 Configuration file not supplied
	Counts the number of signals (columns) and creates a configuration with this
	number of electrodes.  Writes out a signal file.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "unemap/rig.h"
#include "user_interface/message.h"

int main(int argc,char *argv[])
{
	char device_name[100],end_of_line;
	FILE *input_file,*signal_file;
	float time_float,*value;
	float end_time,frequency,start_time;
	int arg_number,header_end,i,j,number_of_devices,number_of_samples,
		number_of_signals,return_code,*time;
	struct Device **device;
	struct Rig *signal_rig;
	struct Signal_buffer *buffer;

	/* check arguments */
	return_code=0;
	if ((3==argc)||(4==argc))
	{
		arg_number=0;
		/* open the signal text file */
		arg_number++;
		if (input_file=fopen(argv[arg_number],"r"))
		{
			/* read the configuration file */
			if (4==argc)
			{
				arg_number++;
				return_code=read_configuration_file(argv[arg_number],&signal_rig);
			}
			else
			{
				return_code=1;
				signal_rig=(struct Rig *)NULL;
			}
			if (return_code)
			{
				/* find the end of the header and count the number of samples */
				header_end=0;
				number_of_samples=0;
				while (EOF!=(return_code=fscanf(input_file,"%f",&end_time)))
				{
					if (1==return_code)
					{
						if (0==number_of_samples)
						{
							start_time=end_time;
						}
						number_of_samples++;
					}
					else
					{
						header_end += number_of_samples+1;
						number_of_samples=0;
					}
					fscanf(input_file,"%*[^\n] ");
				}
				if ((1<number_of_samples)&&(start_time<end_time))
				{
					frequency=(float)(number_of_samples-1)/(end_time-start_time);
				}
				else
				{
					frequency=1;
				}
				return_code=1;
				/*???debug */
				printf("number_of_samples=%d\n",number_of_samples);
				printf("header_end=%d\n",header_end);
				printf("frequency=%g\n",frequency);
				/* count the number of signals */
				rewind(input_file);
				for (i=header_end;i>0;i--)
				{
					fscanf(input_file,"%*[^\n] ");
				}
				number_of_signals=0;
				/* skip "time" */
				fscanf(input_file," %[^ \t\n]%c",device_name,&end_of_line);
				while ('\n'!=end_of_line)
				{
					number_of_signals++;
					fscanf(input_file," %[^ \t\n]%c",device_name,&end_of_line);
				}
				/*???debug */
				printf("number_of_signals=%d\n",number_of_signals);
				if (3==argc)
				{
					/* create a rig with the required number of electrodes */
					signal_rig=create_standard_Rig("text",PATCH,MONITORING_OFF,
						EXPERIMENT_OFF,1,&number_of_signals,1,0,(float)1);
				}
				if (signal_rig)
				{
					number_of_devices=signal_rig->number_of_devices;
					/* create the signal buffer */
					if (buffer=create_Signal_buffer(FLOAT_VALUE,number_of_signals,
						number_of_samples,frequency))
					{
						/* assign the signals devices */
						rewind(input_file);
						return_code=1;
						if (4==argc)
						{
							for (i=header_end-1;i>0;i--)
							{
								fscanf(input_file,"%*[^\n] ");
							}
							/* skip "time" */
							fscanf(input_file," %[^ \t\n]%c",device_name,&end_of_line);
							i=0;
							while (return_code&&(i<number_of_signals))
							{
								fscanf(input_file," %[^ \t\n]",device_name);
								device=signal_rig->devices;
								j=number_of_devices;
								while ((j>0)&&strcmp(device_name,(*device)->description->name))
								{
									device++;
									j--;
								}
								if (j>0)
								{
									if (!((*device)->signal=create_Signal(i,buffer,UNDECIDED,i)))
									{
										printf("ERROR.  Could not create signal for %s\n",
											device_name);
										return_code=0;
									}
								}
								else
								{
									printf("WARNING.  Unknown device.  %s\n",device_name);
								}
								i++;
							}
						}
						else
						{
							for (i=header_end;i>0;i--)
							{
								fscanf(input_file,"%*[^\n] ");
							}
							device=signal_rig->devices;
							i=0;
							while ((i<number_of_devices)&&
								((*device)->signal=create_Signal(i,buffer,UNDECIDED,i)))
							{
								device++;
								i++;
							}
							if (i<number_of_devices)
							{
								return_code=0;
							}
						}
						if (return_code)
						{
							/* read the signal values */
							value=(buffer->signals).float_values;
							time=buffer->times;
							for (i=number_of_samples;i>0;i--)
							{
								fscanf(input_file,"%f",&time_float);
								*time=(int)(time_float*frequency+0.5);
								time++;
								for (j=number_of_signals;j>0;j--)
								{
									fscanf(input_file,"%f",value);
									value++;
								}
							}
							arg_number++;
							if (signal_file=fopen(argv[arg_number],"wb"))
							{
								write_signal_file(signal_file,signal_rig);
								fclose(signal_file);
							}
							else
							{
								printf("ERROR.  Could not open signal file.  %s\n",
									argv[arg_number]);
								return_code=0;
							}
						}
#if defined (OLD_CODE)
					/* read the signal file */
					signal_rig=(struct Rig *)NULL;
					arg_number++;
					if ((signal_file=fopen(argv[arg_number],"rb"))&&
						read_signal_file(signal_file,&signal_rig))
					{
						fclose(signal_file);
						device=signal_rig->devices;
						if (device&&(*device)&&((*device)->signal)&&
							(buffer=(*device)->signal->buffer)&&((*device)->channel))
						{
							frequency=buffer->frequency;
							time=buffer->times;
							number_of_devices=signal_rig->number_of_devices;
							number_of_signals=buffer->number_of_signals;
							/* write heading */
							fprintf(input_file,"time");
							for (i=number_of_devices;i>0;i--)
							{
								fprintf(input_file,"\t%s",(*device)->description->name);
								device++;
							}
							fprintf(input_file,"\n");
							/* write times and signal values */
							switch (buffer->value_type)
							{
								case FLOAT_VALUE:
								{
									float *value;

									value=(buffer->signals).float_values;
									for (i=buffer->number_of_samples;i>0;i--)
									{
										fprintf(input_file,"%g",((float)(*time))/frequency);
										device=signal_rig->devices;
										for (j=number_of_devices;j>0;j--)
										{
											offset=(*device)->channel->offset;
											gain=(*device)->channel->gain;
											fprintf(input_file,"\t%g",
												gain*(value[(*device)->signal->index]-offset));
											device++;
										}
										fprintf(input_file,"\n");
										value += number_of_signals;
										time++;
									}
								} break;
								case SHORT_INT_VALUE:
								{
									short int *value;

									value=(buffer->signals).short_int_values;
									for (i=buffer->number_of_samples;i>0;i--)
									{
										fprintf(input_file,"%g",((float)(*time))/frequency);
										device=signal_rig->devices;
										for (j=number_of_devices;j>0;j--)
										{
											offset=(*device)->channel->offset;
											gain=(*device)->channel->gain;
											fprintf(input_file,"\t%g",
												gain*((float)value[(*device)->signal->index]-offset));
											device++;
										}
										fprintf(input_file,"\n");
										value += number_of_signals;
										time++;
									}
								} break;
							}
						}
						else
						{
							printf("ERROR.  No devices");
							return_code=0;
						}
						fclose(input_file);
					}
					else
					{
						printf("ERROR.  Could not read signal file.  %s\n",
							argv[arg_number]);
						return_code=0;
					}
#endif /* defined (OLD_CODE) */
					}
					else
					{
						printf("ERROR.  Could not create signal buffer.  %d %d %g\n",
							number_of_signals,number_of_samples,frequency);
						return_code=0;
					}
				}
				else
				{
					printf("ERROR.  Creating default rig\n");
					return_code=0;
				}
			}
			else
			{
				printf("ERROR.  Could not read configuration file.  %s\n",
					argv[arg_number]);
				return_code=0;
			}
		}
		else
		{
			printf("ERROR.  Could not open text signal file.  %s\n",argv[arg_number]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: text2sig in_text_file in_cnfg_file out_signal_file\n");
		printf("  in_text_file is the name for the text signal file that is input\n");
		printf("  in_cnfg_file is the name for the configuration file that is input (optional)\n");
		printf("  out_signal_file is the name of the signal file is created\n");
		return_code=0;
	}

	return (return_code);
} /* main */
