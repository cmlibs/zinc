/*******************************************************************************
FILE : bard.c

LAST MODIFIED : 26 December 1996

DESCRIPTION :
Functions for reading files from the Bard system.

Information on Bard64 Cardiac Mapping System

1 WINDOW.DAT file structure
WINDOW.DAT is a fixed length file containing 8 seconds of 64 channel data
sampled at 1 kHz.  The file consists of 8000 frames of 65 words (2 bytes) each,
each frame represents one millisecond of simultaneously sampled data.

Each frame begins with a frame sync word which includes embedded status
information about one of the 8 connectors on the BARD64 amplifier.  The frame
marker is followed by 64 channel information words, one for each of the 64
channels which were simultaneously acquired.  The order of the channels within
a frame is given below.

1.1 Frame sync word
The frames are in sets of 8.  The difference between the frames with in a set is
in the information being sent in the sync word.  The order of the sync words is
connector A
connector B
connector C
connector D
connector E
connector F
connector G
unipolar/bipolar
The bits in a frame sync word are
 1.  1
 2.  1
 3.  PARITY  set so that after the frame sync word is AND'd with 0xAFFF the
			 number of 1's is odd
 ???4.  LAT ECG SAT  non-latched ECG saturation information sent in CC0..CC7
 5.  CONN ZERO  1 for the connector A frame sync word and 0 otherwise
 ???6.  NLAT ECG SAT  latched ECG saturation signal information set in CC0..CC7
 7.  REF STATUS  0 for grounded, 1 for not grounded
 8.  0
 9.  CC7
10.  CC6
11.  CC5
12.  CC4
13.  CC3
14.  CC2
15.  CC1
16.  CC0
For the unipolar/bipolar frame sync word, CC7=1 and CC6..CC0 indicate if front
panel connectors G..A are set for bipolar (0) or unipolar (1).

1.2 Channel information word
Each channel information word contains a 12 bit A/D sample value in binary
offset format, one parity bit, one status bit and two unused bits.  The raw A/D
values may be converted to integer values by
integer_value = (sample_value & 0x0FFF) - 2048
The bits in a channel information word are
 1.  0
 2.  0
 3.  PARITY  set so that after the frame sync word is AND'd with 0xAFFF the
			 number of 1's is odd
 ???4.  LAT ECG SAT  non-latched ECG saturation information sent in CC0..CC7
 5.  AD11
 6.  AD10
 7.  AD9
 8.  AD8
 9.  AD7
10.  AD6
11.  AD5
12.  AD4
13.  AD3
14.  AD2
15.  AD1
16.  AD0

1.3 Channel order within frame
A8 A7 A6 A5 A4 A3 A2 A1
E1 E2 E3 E4 E5 E6 E7 E8
D4 C4 D3 C3 D2 C2 D1 C1
C5 D5 C6 D6 C7 D7 C8 D8
High_level V aVF aVL aVR III II I
B1 B2 B3 B4 B5 B6 B7 B8
F1 F2 F3 F4 F5 F6 F7 F8
G8 G7 G6 G5 G4 G3 G2 G1
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/bard.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
int read_bard_electrode_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
Reads in a rig configuration from a Bard electrode file.
==============================================================================*/
{
	char device_name[81];
	FILE *electrode_file;
	float x,y;
	int channel_number,device_number,number_of_devices,number_of_electrodes,
		return_code;
	static char *auxiliary_name[]=
	{
		"I",
		"II",
		"III",
		"aVR",
		"aVL",
		"aVF",
		"V",
		"High Level"
	};
	struct Channel *channel;
	struct Device **device;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *region_item;
	struct Rig *rig;

	ENTER(read_bard_electrode_file);
	if (file_name&&rig_pointer)
	{
		/* read the rig configuration from the electrode file */
		if (electrode_file=fopen(file_name,"rt"))
		{
			/* read the number of electrodes */
			if ((1==fscanf(electrode_file,"%d ",&number_of_electrodes))&&
				(number_of_electrodes>0))
			{
				/* for Bard there are 8 auxiliary inputs */
				number_of_devices=number_of_electrodes+8;
				/* allocate storage for the rig */
				if ((ALLOCATE(device,struct Device *,number_of_devices))&&
					(region=create_Region("Bard",PATCH,0,number_of_devices
#if defined (UNEMAP_USE_NODES)
						/*??JW do we need to pass this down from above?*/
						,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_NODES) */
					))&&
					(region_item=create_Region_list_item(region,
					(struct Region_list_item *)NULL))&&(rig=create_Rig("Bard",
					MONITORING_OFF,EXPERIMENT_OFF,number_of_devices,device,
					(struct Page_list_item *)NULL,1,region_item,region
#if defined (UNEMAP_USE_NODES)
						/*??JW do we need to pass this down from above?*/
						,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_NODES) */
					)))
				{
					/* read in the electrode information */
					return_code=1;
					device_number=0;
					while ((device_number<number_of_electrodes)&&return_code)
					{
						if (4==fscanf(electrode_file,"%d %80s %*d %f %f ",&channel_number,
							device_name,&x,&y))
						{
							/* assign memory for device */
							if ((description=create_Device_description(device_name,
								ELECTRODE,region))&&(channel=create_Channel(channel_number,0,
								1))&&(*device=create_Device(device_number,description,channel,
								(struct Signal *)NULL)))
							{
								description->properties.electrode.position.x=x;
								description->properties.electrode.position.y=y;
								device++;
								device_number++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_bard_electrode_file.  Insufficient memory");
								destroy_Channel(&channel);
								destroy_Device_description(&description);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"read_bard_electrode_file.  Error reading electrode information");
							return_code=0;
						}
					}
					if (return_code)
					{
						/* set up the 8 auxiliary inputs */
						channel_number=56;
						while ((device_number<number_of_devices)&&return_code)
						{
							/* assign memory for device */
							if ((description=create_Device_description(
								auxiliary_name[channel_number-56],AUXILIARY,region))&&
								(channel=create_Channel(channel_number,0,1))&&
								(*device=create_Device(device_number,description,channel,
								(struct Signal *)NULL)))
							{
								device++;
								device_number++;
								channel_number++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_bard_electrode_file.  Insufficient memory");
								destroy_Channel(&channel);
								destroy_Device_description(&description);
								return_code=0;
							}
						}
						if (!return_code)
						{
							rig->number_of_devices=device_number;
							destroy_Rig(&rig);
						}
					}
					else
					{
						rig->number_of_devices=device_number;
						destroy_Rig(&rig);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_bard_electrode_file.  Insufficient memory");
					rig=(struct Rig *)NULL;
					destroy_Region_list(&region_item);
					destroy_Region(&region);
					DEALLOCATE(device);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_bard_electrode_file.  Error reading number of electrodes");
				rig=(struct Rig *)NULL;
			}
			fclose(electrode_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_bard_electrode_file.  Could not open file: %s",file_name);
			rig=(struct Rig *)NULL;
		}
		if (rig)
		{
			*((struct Rig **)rig_pointer)=rig;
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_bard_electrode_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_bard_electrode_file */

int read_bard_signal_file(char *file_name,void *rig)
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
Reads in a Bard signal file (window.dat) and creates a buffer for it as part of
the <rig>.  See this files header for information on window.dat
==============================================================================*/
{
	FILE *signal_file;
	float frequency;
	int channel,frame,i,number_of_devices,number_of_samples,number_of_signals,
		return_code,*time;
	short unsigned *sample;
	static int channel_to_frame[]=
	{
		8,7,6,5,4,3,2,1,
		41,42,43,44,45,46,47,48,
		24,22,20,18,25,27,29,31,
		23,21,19,17,26,28,30,32,
		9,10,11,12,13,14,15,16,
		49,50,51,52,53,54,55,56,
		64,63,62,61,60,59,58,57,
		40,39,38,37,36,35,34,33
	};
	struct Device **device;
	struct Signal_buffer *buffer;
	struct Rig *bard_rig;

	ENTER(read_bard_signal_file);
	if (file_name&&(bard_rig=(struct Rig *)rig))
	{
		/* read in the bard signal file */
		if (signal_file=fopen(file_name,"rb"))
		{
			number_of_signals=65;
			number_of_samples=8000;
			frequency=1000;
			if (buffer=create_Signal_buffer(SHORT_INT_VALUE,number_of_signals,
				number_of_samples,frequency))
			{
				time=buffer->times;
				for (i=0;i<number_of_samples;i++)
				{
					*time=i;
					time++;
				}
				/* read in the signals */
				if (BINARY_FILE_READ((char *)(buffer->signals.short_int_values),2,
					number_of_samples*number_of_signals,signal_file)==
					number_of_samples*number_of_signals)
				{
					/* convert sample values to integers */
					sample=(short unsigned *)((buffer->signals).short_int_values);
					for (frame=8000;frame>0;frame--)
					{
						/* skip frame sync word */
						sample++;
						for (channel=64;channel>0;channel--)
						{
							/* convert (includes byte swap) */
							*sample=((((*sample)<<8)&0x0F00)|(((*sample)>>8)&0x00FF))-2048;
							sample++;
						}
					}
					/* link the devices to the signals */
					number_of_devices=bard_rig->number_of_devices;
					device=bard_rig->devices;
					return_code=1;
					while ((number_of_devices>0)&&return_code)
					{
						if (ELECTRODE==(*device)->description->type)
						{
							if (!((*device)->signal=create_Signal(
								channel_to_frame[(*device)->channel->number],buffer,UNDECIDED,
								0)))
							{
								return_code=0;
							}
						}
						else
						{
							if (!((*device)->signal=create_Signal(
								channel_to_frame[(*device)->channel->number],buffer,REJECTED,
								0)))
							{
								return_code=0;
							}
						}
						device++;
						number_of_devices--;
					}
					if (!return_code)
					{
						destroy_Signal_buffer(&buffer);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_bard_signal_file.  Error reading signals");
					destroy_Signal_buffer(&buffer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_bard_signal_file.  Could not create signal buffer");
			}
			fclose(signal_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_bard_signal_file.  Could not open file: %s",file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_bard_signal_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_bard_signal_file */
