/*******************************************************************************
FILE : text2sig.c

LAST MODIFIED : 24 September 2003

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
	int arg_number,header_end,i,j,line_number,number_of_bad_sample_lines,
		number_of_columns,number_of_devices,number_of_good_sample_lines,
		number_of_sample_lines,number_of_signals,return_code,*time,
		time_column_absent;
	struct Device **device;
	struct Region_list_item *region_list_item,*region_list_item_previous;
	struct Rig *signal_rig;
	struct Signal_buffer *buffer;

	/* check arguments */
	return_code=0;
	if ((3==argc)||(4==argc)||(5==argc))
	{
		arg_number=1;
		time_column_absent=0;
		if (4<=argc)
		{
			if (1==sscanf(argv[arg_number],"%f",&frequency))
			{
				time_column_absent=1;
				arg_number++;
			}
		}
		/* open the signal text file */
		if (input_file=fopen(argv[arg_number],"r"))
		{
			arg_number++;
			/* read the configuration file */
			if (4==argc-time_column_absent)
			{
				return_code=read_configuration_file(argv[arg_number],&signal_rig);
				arg_number++;
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
				number_of_good_sample_lines=0;
				number_of_bad_sample_lines=0;
				while ((EOF!=fscanf(input_file,"%*[ \t]"))&&
					(EOF!=(return_code=fscanf(input_file,"%[^ \t\n\r]",device_name))))
				{
					if (1==return_code)
					{
						if ((1==return_code)&&(1==sscanf(device_name,"%f",&end_time)))
						{
							if (0==number_of_good_sample_lines)
							{
								start_time=end_time;
							}
							number_of_good_sample_lines++;
						}
						else
						{
							header_end +=
								number_of_good_sample_lines+number_of_bad_sample_lines+1;
							number_of_good_sample_lines=0;
							number_of_bad_sample_lines=0;
						}
					}
					else
					{
						number_of_bad_sample_lines++;
					}
					fscanf(input_file,"%*[^\n]");
					fscanf(input_file,"%*c");
				}
#if defined (OLD_CODE)
				while (EOF!=(return_code=fscanf(input_file,"%f",&end_time)))
				{
					if (1==return_code)
					{
						if (0==number_of_good_sample_lines)
						{
							start_time=end_time;
						}
						number_of_good_sample_lines++;
					}
					else
					{
						header_end += number_of_good_sample_lines+1;
						number_of_good_sample_lines=0;
					}
					fscanf(input_file,"%*[^\n] ");
				}
#endif /* defined (OLD_CODE) */
				if (!time_column_absent)
				{
					if ((1<number_of_good_sample_lines)&&(start_time<end_time))
					{
						frequency=(float)(number_of_good_sample_lines-1)/
							(end_time-start_time);
					}
					else
					{
						frequency=1;
					}
				}
				return_code=1;
				number_of_sample_lines=number_of_good_sample_lines+
					number_of_bad_sample_lines;
				/*???debug */
				printf("number_of_sample_lines=%d\n",number_of_sample_lines);
				printf("header_end=%d\n",header_end);
				printf("frequency=%g\n",frequency);
				/* count the number of signals */
				rewind(input_file);
				for (i=header_end;i>0;i--)
				{
					fscanf(input_file,"%*[^\n] ");
				}
				number_of_signals=0;
				if (time_column_absent)
				{
					end_of_line=' ';
				}
				else
				{
					/* skip "time" */
					fscanf(input_file," %[^ \t\n\r]%c",device_name,&end_of_line);
					if ('\r'==end_of_line)
					{
						end_of_line=(char)fgetc(input_file);
					}
					if ('\n'!=end_of_line)
					{
						fscanf(input_file,"%*[ \t]");
						end_of_line=(char)fgetc(input_file);
						if ('\r'==end_of_line)
						{
							end_of_line=(char)fgetc(input_file);
						}
						if ('\n'!=end_of_line)
						{
							ungetc((int)end_of_line,input_file);
						}
					}
				}
				while ('\n'!=end_of_line)
				{
					number_of_signals++;
					fscanf(input_file," %[^ \t\n\r]%c",device_name,&end_of_line);
					if ('\r'==end_of_line)
					{
						end_of_line=(char)fgetc(input_file);
					}
					if ('\n'!=end_of_line)
					{
						fscanf(input_file,"%*[ \t]");
						end_of_line=(char)fgetc(input_file);
						if ('\r'==end_of_line)
						{
							end_of_line=(char)fgetc(input_file);
						}
						if ('\n'!=end_of_line)
						{
							ungetc((int)end_of_line,input_file);
						}
					}
				}
				/*???debug */
				printf("number_of_signals=%d\n",number_of_signals);
				if (3==argc-time_column_absent)
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
						number_of_good_sample_lines,frequency))
					{
						/* assign the signals devices */
						rewind(input_file);
						return_code=1;
						if (4==argc-time_column_absent)
						{
							for (i=header_end-1;i>0;i--)
							{
								fscanf(input_file,"%*[^\n] ");
							}
							if (0<header_end)
							{
								if (!time_column_absent)
								{
									/* skip "time" column */
									fscanf(input_file," %[^ \t\n]%c",device_name,&end_of_line);
								}
							}
							i=0;
							while (return_code&&(i<number_of_signals))
							{
								device=signal_rig->devices;
								j=number_of_devices;
								if (0<header_end)
								{
									fscanf(input_file," %[^ \t\n]",device_name);
									while ((j>0)&&strcmp(device_name,
										(*device)->description->name))
									{
										device++;
										j--;
									}
								}
								else
								{
									while ((j>0)&&(!(*device)||!((*device)->channel)||
										(i+1!=(*device)->channel->number)))
									{
										device++;
										j--;
									}
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
									if (0<header_end)
									{
										printf("WARNING.  Unknown device.  %s\n",device_name);
									}
									else
									{
										printf("WARNING.  Unknown channel.  %d\n",i+1);
									}
								}
								i++;
							}
							/* get rid of devices without signals */
							region_list_item=signal_rig->region_list;
							while (region_list_item)
							{
								if (region_list_item->region)
								{
									region_list_item->region->number_of_devices=0;
								}
								region_list_item=region_list_item->next;
							}
							j=0;
							device=signal_rig->devices;
							for (i=0;i<number_of_devices;i++)
							{
								if ((device[i])->signal)
								{
									if (i!=j)
									{
										device[j]=device[i];
									}
									if ((device[i]->description)&&
										(device[i]->description->region))
									{
										(device[i]->description->region->number_of_devices)++;
									}
									j++;
								}
							}
							signal_rig->number_of_devices=j;
							region_list_item=signal_rig->region_list;
							region_list_item_previous=NULL;
							signal_rig->number_of_regions=0;
							while (region_list_item)
							{
								if ((region_list_item->region)&&
									(0<region_list_item->region->number_of_devices))
								{
									(signal_rig->number_of_regions)++;
									region_list_item_previous=region_list_item;
								}
								else
								{
									if (region_list_item_previous)
									{
										region_list_item_previous->next=region_list_item->next;
									}
									else
									{
										signal_rig->region_list=region_list_item->next;
									}
								}
								region_list_item=region_list_item->next;
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
							line_number=header_end+1;
							number_of_bad_sample_lines=0;
							number_of_good_sample_lines=0;
							value=(buffer->signals).float_values;
							time=buffer->times;
							for (i=number_of_sample_lines;i>0;i--)
							{
								number_of_columns=0;
								/* time */
								if (time_column_absent||
									((2==fscanf(input_file," %[^ \t\n\r]%c",device_name,
									&end_of_line))&&(1==sscanf(device_name,"%f",&time_float))))
								{
									if (time_column_absent)
									{
										*time=number_of_good_sample_lines;
										fscanf(input_file,"%*[ \t]");
										end_of_line=(char)fgetc(input_file);
										if ('\r'==end_of_line)
										{
											end_of_line=(char)fgetc(input_file);
										}
										if ('\n'!=end_of_line)
										{
											ungetc((int)end_of_line,input_file);
										}
									}
									else
									{
										*time=(int)(time_float*frequency+0.5);
									}
									time++;
									number_of_columns++;
									if ('\n'!=end_of_line)
									{
										fscanf(input_file,"%*[ \t]");
										end_of_line=(char)fgetc(input_file);
										if ('\r'==end_of_line)
										{
											end_of_line=(char)fgetc(input_file);
										}
										if ('\n'!=end_of_line)
										{
											ungetc((int)end_of_line,input_file);
										}
									}
									while (('\n'!=end_of_line)&&(2==fscanf(input_file,
										" %[^ \t\n\r]%c",device_name,&end_of_line))&&
										(1==sscanf(device_name,"%f",value)))
									{
										value++;
										number_of_columns++;
										if ('\r'==end_of_line)
										{
											end_of_line=(char)fgetc(input_file);
										}
										if ('\n'!=end_of_line)
										{
											fscanf(input_file,"%*[ \t]");
											end_of_line=(char)fgetc(input_file);
											if ('\r'==end_of_line)
											{
												end_of_line=(char)fgetc(input_file);
											}
											if ('\n'!=end_of_line)
											{
												ungetc((int)end_of_line,input_file);
											}
										}
									}
								}
								if (number_of_columns==number_of_signals+1)
								{
									number_of_good_sample_lines++;
								}
								else
								{
									number_of_bad_sample_lines++;
									printf("Error in line %d\n",line_number);
									if (0<number_of_columns)
									{
										time--;
										value -= number_of_columns-1;
									}
								}
								line_number++;
							}
							buffer->number_of_samples=number_of_good_sample_lines;
							buffer->end=number_of_good_sample_lines-1;
							/*???debug */
							printf("number_of_samples=%d\n",buffer->number_of_samples);
							if (signal_file=fopen(argv[arg_number],"wb"))
							{
								arg_number++;
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
					}
					else
					{
						printf("ERROR.  Could not create signal buffer.  %d %d %g\n",
							number_of_signals,number_of_good_sample_lines,frequency);
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
		printf("usage: text2sig sampling_frequency in_text_file in_cnfg_file out_signal_file\n");
		printf("  sampling_frequency is the value in Hz (optional)\n");
		printf("  in_text_file is the name for the text signal file that is input\n");
		printf("    If the sampling_frequency is present then all the columns in\n");
		printf("    are assumed to be signals.  Otherwise, the first columns is\n");
		printf("    assumed to be time\n");
		printf("  in_cnfg_file is the name for the configuration file that is input (optional)\n");
		printf("  out_signal_file is the name of the signal file is created\n");
		return_code=0;
	}

	return (return_code);
} /* main */
