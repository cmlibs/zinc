/*******************************************************************************
FILE : extract_signal.c

LAST MODIFIED : 2 November 1999

DESCRIPTION :
Extracts the signal for a specified device, as an ascii file, from a emap signal
file.

MAKE :
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
	char device_name[81],file_name[81];
	FILE *output_file,*signal_file;
	float frequency,gain,offset;
	int i,number_of_signals,output_format,return_code,*time;
	struct Device **device;
	struct Rig *rig=(struct Rig *)NULL;
	struct Signal_buffer *buffer;

	return_code=0;
	printf("Signal file name ? ");
	if ((1==scanf("%80s",file_name))&&(signal_file=fopen(file_name,"rb"))&&
		(read_signal_file(signal_file,(void *)&rig)))
	{
		printf("Device name ? ");
		if (1==scanf("%80s",device_name))
		{
			i=rig->number_of_devices;
			device=rig->devices;
			while ((i>0)&&(*device)&&((*device)->description)&&
				strcmp(device_name,(*device)->description->name))
			{
				device++;
				i--;
			}
			if ((i>0)&&(*device)&&((*device)->description)&&
				!strcmp(device_name,(*device)->description->name))
			{
				printf("Output format ?\n");
				printf("(1) Comma separated\n");
				printf("(2) Matlab\n");
				if (1==scanf("%d",&output_format))
				{
					if (output_format<1)
					{
						output_format=1;
					}
					else
					{
						if (output_format>2)
						{
							output_format=2;
						}
					}
					printf("Output file name ? ");
					if ((1==scanf("%80s",file_name))&&(output_file=fopen(file_name,"w")))
					{
						if (((*device)->signal)&&(buffer=(*device)->signal->buffer)&&
							((*device)->channel))
						{
							frequency=buffer->frequency;
							number_of_signals=buffer->number_of_signals;
							time=buffer->times;
							offset=(*device)->channel->offset;
							gain=(*device)->channel->gain;
							switch (buffer->value_type)
							{
								case FLOAT_VALUE:
								{
									float *value;

									value=((buffer->signals).float_values)+
										((*device)->signal->index);
									switch (output_format)
									{
										case 1:
										/* comma separated */
										{
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g,%g\n",
													((float)(*time))/frequency,gain*((*value)-offset));
												value += number_of_signals;
												time++;
											}
											return_code=1;
										} break;
										case 2:
										/* Matlab */
										{
											fprintf(output_file,"A=[\n");
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g\n",((float)(*time))/frequency);
												time++;
											}
											fprintf(output_file,"]\n");
											fprintf(output_file,"B=[\n");
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g\n",gain*((*value)-offset));
												value += number_of_signals;
											}
											fprintf(output_file,"]\n");
											return_code=1;
										} break;
									}
								} break;
								case SHORT_INT_VALUE:
								{
									short int *value;

									value=((buffer->signals).short_int_values)+
										((*device)->signal->index);
									switch (output_format)
									{
										case 1:
										/* comma separated */
										{
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g,%g\n",
													((float)(*time))/frequency,
													gain*((float)(*value)-offset));
												value += number_of_signals;
												time++;
											}
											return_code=1;
										} break;
										case 2:
										/* Matlab */
										{
											fprintf(output_file,"A=[\n");
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g\n",((float)(*time))/frequency);
												time++;
											}
											fprintf(output_file,"]\n");
											fprintf(output_file,"B=[\n");
											for (i=buffer->number_of_samples;i>0;i--)
											{
												fprintf(output_file,"%g\n",
													gain*((float)(*value)-offset));
												value += number_of_signals;
											}
											fprintf(output_file,"]\n");
											return_code=1;
										} break;
									}
								} break;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Device missing");
						}
						fclose(output_file);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Error reading output format");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Error opening output file");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Device missing");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Error reading device name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Invalid signal file");
	}

	return (return_code);
} /* main */
