/*******************************************************************************
FILE : text2sig.c

LAST MODIFIED : 15 November 2000

DESCRIPTION :
Reads in a text signal file (produced by sig2text) and a configuration file and
writes out a signal file.
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
	float frequency;
	int arg_number,i,j,number_of_devices,number_of_samples,number_of_signals,
		return_code,*time;
	struct Device **device;
	struct Rig *signal_rig;
	struct Signal_buffer *buffer;

	/* check arguments */
	return_code=0;
	if (4==argc)
	{
		arg_number=0;
		/* open the signal text file */
		arg_number++;
		if (input_file=fopen(argv[arg_number],"r"))
		{
			/* read the configuration file */
			arg_number++;
			if (read_configuration_file(argv[arg_number],&signal_rig))
			{
				number_of_devices=signal_rig->number_of_devices;
				/* count the number of signals */
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
				/* count the number of samples */
				number_of_samples=0;
				frequency=1;
				fscanf(input_file," ");
				while (EOF!=fscanf(input_file,"%f",&frequency))
				{
					number_of_samples++;
					fscanf(input_file,"%*[^\n] ");
				}
				if ((0<number_of_samples)&&(0<frequency))
				{
					frequency=(float)number_of_samples/frequency;
				}
				else
				{
					frequency=1;
				}
				/*???debug */
				printf("frequency=%g\n",frequency);
				/*???debug */
				printf("number_of_samples=%d\n",number_of_samples);
				rewind(input_file);
				/* create the signal buffer */
				if (buffer=create_Signal_buffer(FLOAT_VALUE,number_of_signals,
					number_of_samples,frequency))
				{
					/* assign the signals devices */
					/* skip "time" */
					fscanf(input_file," %[^ \t\n]%c",device_name,&end_of_line);
					i=0;
					return_code=1;
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
								printf("ERROR.  Could not create signal for %s\n",device_name);
								return_code=0;
							}
						}
						else
						{
							printf("WARNING.  Unknown device.  %s\n",device_name);
						}
						i++;
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
					printf("ERROR.  Could not read signal file.  %s\n",argv[arg_number]);
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
		printf("  in_cnfg_file is the name for the configuration file that is input\n");
		printf("  out_signal_file is the name of the signal file is created\n");
		return_code=0;
	}

	return (return_code);
} /* main */
