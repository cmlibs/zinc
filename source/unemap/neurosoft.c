/*******************************************************************************
FILE : neurosoft.c

LAST MODIFIED : 4 August 1997

DESCRIPTION :
Functions for reading output from Neurosoft's SCAN package.  SCAN has four ASCII
formats (shared example has 32 electrodes and 1550 times) -
1 BESA (Brain Electric Source Analysis) .RAW format.  Example p7besa.raw
	file size=297664=(32 electrodes)*((1550 times)*(6 bytes)+(LF byte)+(CR byte))
	value=<space><sign>%.%%
2 ROWS=POINTS format.  Example p7points.dat
	NAME(elect#1) NAME(elect#2) ... NAME(NELECTS)
	X(elect#1,time#1) X(elect#2,time#1) ... X(NELECTS,time#1)
	X(elect#1,time#2) X(elect#2,time#2) ... X(NELECTS,time#2)
	.
	.
	.
	X(elect#1,NTIMES) X(elect#2,NTIMES) ... X(NELECTS,NTIMES)
	file size=400318=(32 electrodes)*(13 characters for name)+(LF byte)+(CR byte)+
		(1550 times)*((32 electrodes)*(8 characters for value)+(LF byte)+(CR byte))
3 ROWS=ELECT format.  Example p7elect.dat
	NAME(elect#1) X(elect#1,time#1) X(elect#1,time#2) ... X(elect#1,NTIMES)
	NAME(elect#2) X(elect#2,time#1) X(elect#2,time#2) ... X(elect#2,NTIMES)
	.
	.
	.
	NAME(NELECTS) X(NELECTS,time#1) X(NELECTS,time#2) ... X(NELECTS,NTIMES)
	file size=397280=(32 electrodes)*((13 characters for name)+
		(1550 times)*(8 characters for value)+(LF byte)+(CR byte))
	name='10 characters'<space>
	value=<space><sign>%.%%%% (in microvolts)
4 GROUP VAR format.  No example/not interested

With the example is  config.pos .  This has locations, but they don't seem to
correspond to the picture I have of the electrode layout.

???DB.  Will go ahead with ROWS=POINTS format, but want the following
1 Sampling frequency (guess 200 Hz)
2 Layout (will create a neurosoft.cnfg from diagram)
3 Raw data (A/D results and gains)
==============================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/neurosoft.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define SAMPLING_FREQUENCY 200

/*
Global functions
----------------
*/
int read_neurosoft_row_points_file(char *file_name,void *rig_void)
/*******************************************************************************
LAST MODIFIED : 4 August 1997

DESCRIPTION :
Reads a ROWS=POINTS format signal file produced by Neurosoft's SCAN program.
Assumes the SAMPLING_FREQUENCY and the configuration (have created
neurosoft.cnfg) has been read.
==============================================================================*/
{
	char device_name[11];
	FILE *neurosoft_file;
	float sample_value,*signal_value;
	int i,index,number_of_devices,number_of_samples,number_of_signals,return_code,
		*time;
	struct Device **device,**new_device,**new_devices;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;

	ENTER(read_neurosoft_row_points_file);
	if (file_name&&(rig=(struct Rig *)rig_void))
	{
		if (neurosoft_file=fopen(file_name,"rb"))
		{
			/* count the number of signals */
			number_of_signals=0;
			while (1==fscanf(neurosoft_file," \' %[^\']\'",device_name))
			{
				number_of_signals++;
			}
			/* count the number of samples */
			number_of_samples=0;
			while (1==fscanf(neurosoft_file,"%f",&sample_value))
			{
				number_of_samples++;
			}
			if ((0<number_of_signals)&&(0==number_of_samples%number_of_signals)&&
				(0<(number_of_samples /= number_of_signals)))
			{
				/* create the signal buffer */
				if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,number_of_signals,
					number_of_samples,SAMPLING_FREQUENCY))
				{
					rewind(neurosoft_file);
					/* link the devices to the signals */
					index=0;
					return_code=1;
					number_of_devices=0;
					while (1==fscanf(neurosoft_file," \' %[^\']\'",device_name))
					{
						i=rig->number_of_devices;
						device=rig->devices;
						return_code=1;
						while ((i>0)&&strcmp(device_name,(*device)->description->name))
						{
							device++;
							i--;
						}
						if (i>0)
						{
							number_of_devices++;
							if (ELECTRODE==(*device)->description->type)
							{
								if (!((*device)->signal=create_Signal(index,signal_buffer,
									UNDECIDED,0)))
								{
									return_code=0;
								}
							}
							else
							{
								if (!((*device)->signal=create_Signal(index,signal_buffer,
									REJECTED,0)))
								{
									return_code=0;
								}
							}
						}
						index++;
					}
					if (return_code&&(0<number_of_devices))
					{
						if (number_of_devices<rig->number_of_devices)
						{
							if (ALLOCATE(new_devices,struct Device *,number_of_devices))
							{
								device=rig->devices;
								new_device=new_devices;
								for (i=rig->number_of_devices;i>0;i--)
								{
									if ((*device)->signal)
									{
										*new_device= *device;
										new_device++;
									}
									else
									{
										destroy_Device(device);
									}
									device++;
								}
								DEALLOCATE(rig->devices);
								rig->devices=new_devices;
							}
							else
							{
								destroy_Signal_buffer(&signal_buffer);
								display_message(ERROR_MESSAGE,
				"read_neurosoft_row_points_file.  Insufficient memory for new_devices");
								return_code=0;
							}
						}
						if (return_code)
						{
							/* assign the times */
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* read the signal values (in microvolts) */
							signal_value=(signal_buffer->signals).float_values;
							while (1==fscanf(neurosoft_file,"%f",signal_value))
							{
								signal_value++;
							}
						}
					}
					else
					{
						destroy_Signal_buffer(&signal_buffer);
						if (return_code)
						{
							display_message(ERROR_MESSAGE,
					"No signals are in both the configuration file and the signal file");
							return_code=0;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_neurosoft_row_points_file.  Error creating signals");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_neurosoft_row_points_file.  Could not create signal buffer");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_neurosoft_row_points_file.  No data in file: %s",file_name);
				return_code=0;
			}
			fclose(neurosoft_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_neurosoft_row_points_file.  Could not open file: %s",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_neurosoft_row_points_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_neurosoft_row_points_file */
