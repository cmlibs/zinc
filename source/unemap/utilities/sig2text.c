/*******************************************************************************
FILE : sig2text.c

LAST MODIFIED : 26 June 2000

DESCRIPTION :
Writes out a signal file as text with devices across and time down.  The columns
are tab separated.
cc extract_signal.c -o extract_signal -lm
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
	FILE *output_file,*signal_file;
	float frequency,gain,offset;
	int arg_number,i,j,number_of_devices,number_of_signals,return_code,*time;
	struct Device **device;
	struct Rig *signal_rig;
	struct Signal_buffer *buffer;

	/* check arguments */
	return_code=0;
	if (3==argc)
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
			if (output_file=fopen(argv[arg_number],"w"))
			{
				device=signal_rig->devices;
				if (device&&(*device)&&((*device)->signal)&&
					(buffer=(*device)->signal->buffer)&&((*device)->channel))
				{
					frequency=buffer->frequency;
					time=buffer->times;
					number_of_devices=signal_rig->number_of_devices;
					number_of_signals=buffer->number_of_signals;
					/* write heading */
					fprintf(output_file,"time");
					for (i=number_of_devices;i>0;i--)
					{
						fprintf(output_file,"\t%s",(*device)->description->name);
						device++;
					}
					fprintf(output_file,"\n");
					/* write times and signal values */
					switch (buffer->value_type)
					{
						case FLOAT_VALUE:
						{
							float *value;

							value=(buffer->signals).float_values;
							for (i=buffer->number_of_samples;i>0;i--)
							{
								fprintf(output_file,"%g",((float)(*time))/frequency);
								device=signal_rig->devices;
								for (j=number_of_devices;j>0;j--)
								{
									offset=(*device)->channel->offset;
									gain=(*device)->channel->gain;
									fprintf(output_file,"\t%g",
										gain*(value[(*device)->signal->index]-offset));
									device++;
								}
								fprintf(output_file,"\n");
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
								fprintf(output_file,"%g",((float)(*time))/frequency);
								device=signal_rig->devices;
								for (j=number_of_devices;j>0;j--)
								{
									offset=(*device)->channel->offset;
									gain=(*device)->channel->gain;
									fprintf(output_file,"\t%g",
										gain*((float)value[(*device)->signal->index]-offset));
									device++;
								}
								fprintf(output_file,"\n");
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
				fclose(output_file);
			}
			else
			{
				printf("ERROR.  Could not open output file.  %s\n",argv[arg_number]);
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
		printf("usage: sig2text in_signal_file out_text_file\n");
		printf("  in_signal_file is the name of the signal file to be converted\n");
		printf("  out_text_file is the name for the text file that is output\n");
		return_code=0;
	}

	return (return_code);
} /* main */
