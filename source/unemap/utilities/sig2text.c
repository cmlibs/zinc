/*******************************************************************************
FILE : sig2text.c

LAST MODIFIED : 13 May 2004

DESCRIPTION :
Writes out a signal file as text with devices across and time down.  The columns
are tab separated.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "general/debug.h"
#include "unemap/rig.h"
#include "user_interface/message.h"

enum Output_signals
{
	ALL_SIGNALS,
	NOT_REJECTED_SIGNALS
}; /* enum Output_signals */

int main(int argc,char *argv[])
{
	char *cnfg_file_name,*output_file_name,*signal_file_name,*temp_string;
	enum Output_signals output_signals;
	FILE *output_file,*signal_file;
	float frequency,gain,offset;
	int arg_number,i,j,length,number_of_devices,number_of_signals,return_code,
		*time;
	struct Device **device;
	struct Rig *signal_rig;
	struct Signal_buffer *buffer;

	return_code=0;
	output_signals=ALL_SIGNALS;
	signal_file_name=(char *)NULL;
	output_file_name=(char *)NULL;
	cnfg_file_name=(char *)NULL;
	/* check arguments */
	if (3<=argc)
	{
		return_code=1;
		arg_number=1;
		if ('-'==argv[arg_number][0])
		{
			length=strlen(argv[arg_number]);
			if (ALLOCATE(temp_string,char,length+1))
			{
				for (i=0;i<length;i++)
				{
					if (isupper(argv[arg_number][i]))
					{
						temp_string[i]=tolower(argv[arg_number][i]);
					}
					else
					{
						temp_string[i]=argv[arg_number][i];
					}
				}
				if (!strncmp(temp_string,"-out=",5))
				{
					if (!strcmp(temp_string+5,"all"))
					{
						output_signals=ALL_SIGNALS;
					}
					else if (!strcmp(temp_string+5,"not_reject"))
					{
						output_signals=NOT_REJECTED_SIGNALS;
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
				DEALLOCATE(temp_string);
			}
			else
			{
				return_code=0;
			}
			arg_number++;
		}
		if (return_code&&(2<=argc-arg_number))
		{
			signal_file_name=argv[arg_number];
			arg_number++;
			output_file_name=argv[arg_number];
			arg_number++;
			cnfg_file_name=(char *)NULL;
			if (1<=argc-arg_number)
			{
				if (1==argc-arg_number)
				{
					cnfg_file_name=argv[arg_number];
					arg_number++;
				}
				else
				{
					return_code=0;
				}
			}
		}
	}
	if (return_code)
	{
		return_code=0;
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		if ((signal_file=fopen(signal_file_name,"rb"))&&
			read_signal_file(signal_file,&signal_rig))
		{
			fclose(signal_file);
			/* open the signal text file */
			if (output_file=fopen(output_file_name,"w"))
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
						if ((ALL_SIGNALS==output_signals)||
							((NOT_REJECTED_SIGNALS==output_signals)&&
							((ACCEPTED==(*device)->signal->status)||
							(UNDECIDED==(*device)->signal->status))))
						{
							fprintf(output_file,"\t%s",(*device)->description->name);
						}
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
									if ((ALL_SIGNALS==output_signals)||
										((NOT_REJECTED_SIGNALS==output_signals)&&
										((ACCEPTED==(*device)->signal->status)||
										(UNDECIDED==(*device)->signal->status))))
									{
										offset=(*device)->channel->offset;
										gain=(*device)->channel->gain;
										fprintf(output_file,"\t%g",
											gain*(value[(*device)->signal->index]-offset));
									}
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
									if ((ALL_SIGNALS==output_signals)||
										((NOT_REJECTED_SIGNALS==output_signals)&&
										((ACCEPTED==(*device)->signal->status)||
										(UNDECIDED==(*device)->signal->status))))
									{
										offset=(*device)->channel->offset;
										gain=(*device)->channel->gain;
										fprintf(output_file,"\t%g",
											gain*((float)value[(*device)->signal->index]-offset));
									}
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
				if (cnfg_file_name)
				{
					/* open the signal text file */
					if (output_file=fopen(cnfg_file_name,"w"))
					{
						write_configuration(signal_rig,output_file,TEXT);
						fclose(output_file);
					}
					else
					{
						printf("ERROR.  Could not open configuration file.  %s\n",
							cnfg_file_name);
						return_code=0;
					}
				}
			}
			else
			{
				printf("ERROR.  Could not open text signal file.  %s\n",
					output_file_name);
				return_code=0;
			}
		}
		else
		{
			printf("ERROR.  Could not read signal file.  %s\n",signal_file_name);
			return_code=0;
		}
	}
	else
	{
		printf("usage: sig2text <-out=all|not_reject> in_signal_file out_text_file <out_cnfg_file>\n");
		printf("  -out= is an option for specifying the signals to be written.  The default is all.  not_reject means that rejected signals will not be written");
		printf("  in_signal_file is the name of the signal file to be converted\n");
		printf("  out_text_file is the name for the text file that is output\n");
		printf("  out_cnfg_file is the name for the configuration file that is output.  This is optional\n");
		return_code=0;
	}
  if (0==return_code)
  {
    return_code= -1;
  }
  else
  {
    return_code=0;
  }

	return (return_code);
} /* main */
