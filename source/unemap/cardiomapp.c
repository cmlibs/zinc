/*******************************************************************************
FILE : cardiomapp.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Functions for reading CardioMapp (ART) data files.  The format was given in
CARDIOMAPP_README (part of the CardioMapp developers support kit given to me by
Peng).
???DB.  CardioMapp is the mapping system (up to 256 electrodes, .rdt files) and
	CardioLab is the EP lab system (up to 32 electrodes, .rdc files).  The file
	formats are very similar and Peng has also sent me the CardioLab developers
	kit.
???DB.  Need to remove CARDIOMAPP_README section for gcc because it parses it
	and complains about unterminated strings
==============================================================================*/

/******************************************************************************/
#if defined (CARDIOMAPP_README)
CARDIOMAPP SOFTWARE DEVELOPERS KIT


May 1, 1992


CardioMapp users may wish to analyze data collected with the 
CardioMapp system using other software.  This package is intended
to provide the information necessary to extract signal information
from the CardioMapp files.



DISCLAIMER:

        THIS INFORMATION IS NOT A PART OF THE CARDIOMAPP PRODUCT, AND IS
        NOT SUPPORTED BY ART OR PRUCKA ENGINEERING, INC. (PEI).
        PEI DOES NOT WARRANT THIS INFORMATION TO BE ACCURATE, NOR DOES
        PEI WARRANT ITS SUITABLILTY FOR ANY PURPOSE.  THE FILE FORMATS 
        OF CARDIOMAPP DATA MAY CHANGE FROM VERSION TO VERSION, AND THIS 
        DOCUMENT IS NOT AUTOMATICALLY UPDATED AND DISTRIBUTED AS A PART 
        OF THE CARDIOMAPP PACKAGE.

        THE RAW DATA FILE FORMAT IS THE PROPERTY OF PRUCKA ENGINEERING,
        INC.  YOU MAY MODIFY OR USE THIS INFORMATION FOR PERSONAL USE ONLY. 
        YOU MAY NOT DISTRIBUTE OR PUBLISH IT, OR ANY SOFTWARE DERIVED FROM
        IT, WITHOUT THE EXPRESS WRITTEN CONSENT OF PRUCKA ENGINEERING, INC.



PROGRAMS AND UTILITIES IN THE CARDIOMAPP SOFTWARE DEVELOPERS KIT

README          This file.

TIGA_HDR.EXE    Utility for viewing CardioMapp data file headers.

TIGAEDIT.EXE    Utility for modifying CardioMapp data file headers.

RDT_READ        Sample program for reading CardioMapp data file.

RDT_EXT         Sample program for extracting one channel of CardioMapp
                data to an ASCII file.

SHOW.H		Include file which contains file headers for all data files.




                     USING THE CARDIOMAPP UTILITIES



RDT_EXT.EXE:   EXTRACTING A SINGLE CHANNEL TO AN ASCII FILE: 

This is the most commonly used CardioMapp utility.  It extracts the 
samples of a single channel of data from a .RDT file, and writes 
the sample data out in ASCII format, to a specified text file.

Using the "RDT_EXT.EXE" utility, a single channel may be extracted as
ASCII data to a file.  Note that the channel number is indexed to begin
with "0".  The source file must be a .RDT file.

	RDT_EXT <chan_num> <file_name.RDT> <ascii_file_name> 

This utility is stored in a sub-directory called "RDT_EXT".  Along
with the executable, you will find C source code and a unix-style 
make file.  You may modify this source and re-compile it to meet 
your needs.




RDT_READ:    READING CARDIOMAPP DATA FROM A .RDT FILE

This sub-directory contains "C" source code for reading data from
a .RDT file.  Also included is a unix-style make file for compiling
this example.  You may also find the "RDT_EXT" subdirectory useful
for understanding the .RDT file usage.



TIGA_HDR Utility Program:

The utility program "tiga_hdr.exe" may be used to view the headers
for a number of types of CardioMapp files.  To look at the patient
inforamtion (Patient name, etc.) for the patient in P7.SDY, type:

	TIGA_HDR C:\CMAP.255\STUDIES\P0000007.SDY\PATIENT.INF

The full syntax for the "TIGA_HDR" program is

	TIGA_HDR file_spec

where the file_spec may include DOS wild-cards.




TIGAEDIT Utility Program:

The utility program "tigaedit.exe" may be used to modify the headers
for a number of types of CardioMapp files.  To edit the header for
the patient inforamtion (Patient name, etc.) for the patient in 
P7.SDY, type:

	TIGAEDIT C:\CMAP.255\STUDIES\P0000007.SDY\PATIENT.INF

The full syntax for the "TIGAEDIT" program is

	TIGAEDIT file_spec

where the file_spec may include dos wild-cards.







SHOW.H INCLUDE FILE

This file contains file headers for all types of files.  Commonly
used structures are:

	map_struct:	contains time and activation information
			for a map.

	rdt_parameters:	file header for a raw data file.






CARDIOLAB PATIENT DIRECTORY FILE STRUCTURE

All information for each CardioMapp patient is stored in a
sub-directory on the optical disk.  Example:


C:CMAP.255 
ÃÄÄÄTIGAFONT
ÃÄÄÄSTUDIES
³   ÃÄÄÄP0000000.SDY
³   ÃÄÄÄP0000001.SDY
³   ÃÄÄÄP0000002.SDY
³   ÀÄÄÄP0000003.SDY
ÀÄÄÄDEFAULT


This is an example of a directory tree from a hard disk which 
contains four patients.  All information for a single patient 
is stored in a single sub-directory.




CARDIOMAPP FILE STORAGE


All CardioMapp files are stored as binary files.  Each file has a 
header structure, which contains information about the file.


Raw data files are stored with the extension ".RDT".  Each file has
a header which is of the form of the structure "rdt_parameters". 
The data follows the structure, beginning with the first word after
the structure.  

        a)      File header. (struct rdt_parameters)

        b)      Raw data.  Two fields in the rdt_parameters structure are
                important when reading the raw data.  First, the
                "num_electrodes_used" field is used to determine the
                number of channels in the file.  It will be 64, 128, or
                256, depending on which system you have.  The second
                field of interest is the "acquisition_blocks" field,
                which tells how many data blocks are present.  

=======================================================================
NOTE:  NEW IN CARDIOMAPP 2.0:
	The A/D converters are 12 bits.  You should strip off the 
	most significant nibble before using the data.  

	example: Sample value = data value & 0xFFF  .  (See rdt_ext.c)

=======================================================================

NOTE:  	DATA VALUES ARE FROM 0x0 - 0xFFF  .  This means that 0 volts is
	represented by the value 0x800  .  You should subtract this value
	from each sample as part of the conversion to voltage.

	Value 0     = -10 V
	Value 0x800 = 0 V
	Value 0xFFF = 10 V

======================================================================
		

                Data is stored as 16 bit short integers, in the following
                form:

                Block 0:        Chan 0 (16 bits)
                                Chan 1 (16 bits)
                                Chan 2 (16 bits)
                                        ...
                                        ...
                                Chan 126 (16 bits)
                                Chan 127 (16 bits)

                Block 1:        Chan 0 (16 bits)
                                Chan 1 (16 bits)
                                Chan 2 (16 bits)
                                        ...
                                        ...
                                Chan 126 (16 bits)
                                Chan 127 (16 bits)

                ...
                ...
                ...     

                Block (acquisition_blocks - 1):                        
                                Chan 0 (16 bits)
                                Chan 1 (16 bits)
                                Chan 2 (16 bits)
                                        ...
                                        ...
                                Chan 126 (16 bits)
                                Chan 127 (16 bits)



ORDERING OF CHANNELS IN THE FILES

A word of caution is in order when you are working with the
CardioMapp data.  A translation table is used to convert the 
electrode array/junction box combination to the hardware 
channel numbers.

      -  Each CardioMapp acquisition board contains 16 Amplifiers.

           -  A 255 channel mapping system contains 16 boards.
           -  A 127 channel mapping system contains 8 boards.
           -  A 63 channel mapping system contains 4 boards.

      Channels are interlaced:

           Surface lead channels are not consecutive.


      To determine the numbering scheme for the electrode array
      which you are using, do the following:

           - Run "CardioMapp" with the "-special" option.  
             Example:

                  cmap127 -special

           - Select the patient you wish to work on.

           - From the patient information screen, select 
             the "SETUP" icon.
 
	   - Note the Electrode Configuration configuration selected 
             for this patient.  You will need this information to
             select the correct Translation Table later in this 
             procedure.

	   - From the "Acquisition Configuration" screen, select 
             the "ADVANCED" icon.

           - From the "Advanced Configuration" screen, select the
             "Jacket" icon.  This is the first icon.

           - Select the Electrode Configuration that matches the 
             Electrode Configuration selected for this patient.

           - From the "Electrode Configuration" screen, note the 
             TRANSLATION TABLE corresponding to this Electrode 
             Configuration, and exit without saving.

           - From the "Advanced Configuration" screen, select the
             "Translation Table" icon.  This is the second icon.

           - Select the TRANSLATION TABLE previously identified as
             corresponding to the Electrode Configuration used when 
             acquiring the patient data you are working on.

           - The translation table for this patient will appear.


      How to read the translation table:


           - The translation table converts the software channel
             numbers to the corresponding hardware channel numbers.
             In the data file (the .RDT file), the channels are stored in the order
             of the hardware channel numbers, starting with hardware
             channel 0, then 1, up to the number of amplifier channels
             in the system.  (See the section titled CardioMapp
             File Storage).

             In the translation table, the number on the left (before 
             the colon) is the number of the CardioMapp software channel.
             The number on the right is the number of the data file 
             channel (beginning with 1) which contains the information 
             for that electrode.  

           - The channel numbers in the translation table begin
             with "1", while the channel numbers in the .RDT file
             begin with "0".  Once you have found the channel number
             you require in the translation table, according to the
             following directions, subtract "1" from it to get the
             .RDT file channel number.

           - The first 15 positions contain your surface leads and 
             reference channels.  

             For example, if you want the data for the first channel, 
             lead I, do the following:

                  Find the channel number in the translation table 
                  which is next to the position "1:".  For this 
                  example, assume that the number is 88 (i.e. 1:88).
                  (Yours may be different!!)
   
                  The data channel for this electrode is 87.  (88-1 = 87)

                  Run the RDT_EXT program, requesting channel 87, and
                  you will get the data for lead I.

           - All positions greater than or equal to 17 contain
             intracardiac data.  The "Electrode Configuration"
             contains information on which strips are connected
             to which positions.  

           - The "Electrode Configuration" is used to determine
             the exact location of any electrode.  To find any
             surface or intracardiac channel:

             1) View the electrode configuration which was used
                during acquisition.  (Do this by selecting the 
                "Jacket" icon from the Advanced Configuration
                screen).

             2) Once you have brought up the electrode configuration
                for this jacket, select the "Label" icon to view the 
                labels for this electrode configuration.  Find the 
                electrode number you wish to use.  If you are looking 
                for "A3", locate the number next to the text "A3". 
                This is the softare channel number of the electrode A3.

             3) Exit from the Electrode Configuration, and go back to 
                the "Translation Table".  To do this, exit from
                the Electrode Configuration without saving, then
                select the Translation Table icon.  

                In the translation table, locate the software channel
                number which you found in step 2.  This is the number 
                which you found next to "A3" in the label table.
                The number on the right side of the colon (:) is 
                the hardware channel number for that electrode.

                Subtract "1" from the hardware channel number, and
                you may then use it to access data from the .RDT
                file using the RDT_EXT.EXE program.
                
           

#define MAX_NUM_ELECTRODES 256
#define INPUT_CHANNEL_MAX 16
#define DMA_TRANSFER_WORDS 8192
#define RDT_DESCRIPTION_LEN 40
#define RDT_VERSION_LEN 40
#define RDT_VERSION_STRING "RDT V1.0"
#define RDT_COMMENT_LEN 40
#define RDT_DATE_LEN 40
#define RDT_LOOK_UP_LEN 80
#define RDT_EXTENSION "RDT"

struct rdt_parameters
{
	long 		id_stamp;
	long 		modify_stamp;
	long 		check_sum;

	char 		version[ RDT_VERSION_LEN ];         		/* Data Version String 	*/

	char 		description[ RDT_DESCRIPTION_LEN ]; 		/* Description of data 	*/
	char 		comment[ RDT_COMMENT_LEN ];				 			/* User input comment  	*/
	char 		look_up_description[ RDT_LOOK_UP_LEN ]; /* Look up Desc.   		 	*/
	long 		acquisition_time;											 	/* Time of acquisition 	*/
	char 		acquisition_date[ RDT_DATE_LEN ];	 			/* Date of acquisition 	*/
	short 	acquisition_blocks;								 			/* Number of blocks			*/
	double 	acquisition_length;									 		/* Time in seconds of a	*/

	/**************************/
	/* Information from setup	*/
	/**************************/
	short 		channel_selected;
	short 		first_display_electrode;
	short 		gain_shift[ MAX_NUM_ELECTRODES ];
	short 		offset[ INPUT_CHANNEL_MAX ];
	short 		offset_total[ INPUT_CHANNEL_MAX ];
	short 		offset_gain_adjusted[ MAX_NUM_ELECTRODES ];

	long			eci_id_stamp;
	long			eci_modify_stamp;
	long			eci_check_sum;

	short			channels_to_display;
	short			overlap_signal_display;
	short			non_overlap_num_channels;
	short			first_overlap_electrode;

	short			samples_per_second;
	short			num_electrodes_used;

	short			mode_16at16k;
	short			unused;

	long			fat[15];
};

/******************/
/* DMA parameters	*/
/******************/
#define DMA_TRANSFER_WORDS 					8192
#define DMA_BUFFERS_MAX 						4
#define DMA_SAVE_WORDS 							512
#define DMA_SAVE_BLOCKS 						16
#define DMA_SECONDS_PER_PAGE 				0.06274

#define SAMPLES_PER_SECOND 					1024

/****************************/
/* Real samples per second.	*/
/****************************/
#define DEFAULT_SAMPLES_PER_SECOND 	979		

#define SKIP_POINTS 								2
#define A_TO_D_MAX_VOLTS						10.0L

struct dma_buffer_parameters
{
	/********************/
	/* Buffer addresses	*/
	/********************/
	unsigned short *buffer;

	long			dma_physical_address;
	long			dma_current_physical_address;

	/**********************/
	/* Buffer information	*/
	/**********************/
	short first_page;			/* First logical page of buffer */
	short last_page;			/* Last logical page of buffer  */
	short start_page;			/* First logical page of full buffer */
	short end_page;				/* Last logical page of full buffer  */
	short total_number_of_pages;/* Number of logical pages      */
	short filled_number_of_pages;  /* Number of logical pages filled */
	short filled;         /* YES if buffer contains data. */
	short save;						/* Save to disk.                */

	long 	rdt_id_stamp;
	long 	rdt_modify_stamp;
	long 	rdt_check_sum;

	short saved_to_file;  /* Has the data been saved to file */

	long	fat[16];
};






/***************************************/
/* Electrode Configuration Information */
/***************************************/

// This structure contains the contents of the *.ECI file, which 
// Describes the jacket strips and labels.  It references the 
// translation table (ETT) it uses by means of an id stamp (ett_id_stamp).
// The ETT id stamp is contained in the first 4 bytes of the .ETT file.


#define ECI_VERSION_LEN 40
#define ECI_VERSION_STRING "ECI V1.0"
#define ECI_DESCRIPTION_LEN 40
#define ECI_DEFAULT_FILE_NAME "F0.ECI"
#define ECI_MAX_NUMBER_OF_ROWS 256
#define ECI_EXTENSION "ECI"
#define ECI_LABEL_LENGTH				5
#define ECI_LABEL_NUMBERS			  0
#define ECI_LABEL_BLANK					1

struct eci_parameters
{
	long 	id_stamp;
	long 	modify_stamp;
	long 	check_sum;

	char 	version[ ECI_VERSION_LEN ];
	char 	description[ ECI_DESCRIPTION_LEN ];

	long	ett_id_stamp;
	long	ett_modify_stamp;
	long	ett_check_sum;

	short 	number_of_rows;
	short 	elements_per_row[ ECI_MAX_NUMBER_OF_ROWS ];
	short 	first_element_of_row[ ECI_MAX_NUMBER_OF_ROWS ];
	short 	total_number_of_electrodes;
	short 	index[ MAX_NUM_ELECTRODES + MAX_NUM_OUTLINES * NUM_EDGE_ELECTRODES ];
	char	label[ MAX_NUM_ELECTRODES ][ ECI_LABEL_LENGTH ];

	/* The following data must equal 32 shorts */
	/* Added 8/2/90 */
	char	electrode_array_pn[14];
	char	junction_box_pn[14];		/* From ETT table */
	char  sys_max_num_elec_string[6]; 	/* Translated From ETT table */

	short	fat1[1];
	long	fat[7];
};


/*******************************************/
/* Electrode Translation Table Information */
/*******************************************/

#define ETT_VERSION_LEN 40
#define ETT_VERSION_STRING "ETT V1.0"
#define ETT_DESCRIPTION_LEN 40
#define ETT_DEFAULT_FILE_NAME "F0.ETT"
#define ETT_EXTENSION "ETT"

struct ett_parameters
{	
	long 	id_stamp;
	long 	modify_stamp;
	long 	check_sum;

	char 	version[ ETT_VERSION_LEN ];
	char 	description[ ETT_DESCRIPTION_LEN ];
	unsigned short 	translate[ MAX_NUM_ELECTRODES ];

	short	sys_max_num_electrodes;

	/* The following data must equal 32 shorts */
	/* Added 8/2/90 */
	char	junction_box_pn[14];

	short	fat1[1];
	long	fat[12];
};


#endif /* defined (CARDIOMAPP_README) */
/*============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/cardiomapp.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define DMA_TRANSFER_WORDS 8192
#if defined (OLD_CODE)
#define MAX_NUM_ELECTRODES 256
#define INPUT_CHANNEL_MAX 16
#define RDT_DESCRIPTION_LEN 40
#define RDT_VERSION_LEN 40
#define RDT_VERSION_STRING "RDT V1.0"
#define RDT_COMMENT_LEN 40
#define RDT_DATE_LEN 40
#define RDT_LOOK_UP_LEN 80
#define RDT_EXTENSION "RDT"

/*
Module types
------------
*/
struct RDT_parameters
{
	long 		id_stamp;
	long 		modify_stamp;
	long 		check_sum;

	char 		version[ RDT_VERSION_LEN ];         		/* Data Version String 	*/

	char 		description[ RDT_DESCRIPTION_LEN ]; 		/* Description of data 	*/
	char 		comment[ RDT_COMMENT_LEN ];				 			/* User input comment  	*/
	char 		look_up_description[ RDT_LOOK_UP_LEN ]; /* Look up Desc.   		 	*/
	long 		acquisition_time;											 	/* Time of acquisition 	*/
	char 		acquisition_date[ RDT_DATE_LEN ];	 			/* Date of acquisition 	*/
	short 	acquisition_blocks;								 			/* Number of blocks			*/
	double 	acquisition_length;									 		/* Time in seconds of a	*/

	/**************************/
	/* Information from setup	*/
	/**************************/
	short 		channel_selected;
	short 		first_display_electrode;
	short 		gain_shift[ MAX_NUM_ELECTRODES ];
	short 		offset[ INPUT_CHANNEL_MAX ];
	short 		offset_total[ INPUT_CHANNEL_MAX ];
	short 		offset_gain_adjusted[ MAX_NUM_ELECTRODES ];

	long			eci_id_stamp;
	long			eci_modify_stamp;
	long			eci_check_sum;

	short			channels_to_display;
	short			overlap_signal_display;
	short			non_overlap_num_channels;
	short			first_overlap_electrode;

	short			samples_per_second;
	short			num_electrodes_used;

	short			mode_16at16k;
	short			unused;

	long			fat[15];
}; /* struct RDT_parameters */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
int read_cardiomapp_file(char *file_name,void *rig_void)
/*******************************************************************************
LAST MODIFIED : 23 August 1997

DESCRIPTION :
Reads a .rdt signal file produced by ART's CardioMapp program.
???DB.  Don't use struct RDT_parameters to avoid alignment problems
???DB.  Could read the header field by field (BINARY_FILE_READ)
==============================================================================*/
{
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
	char *swap,temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
	char header_buffer[1446];
	float sampling_frequency;
	FILE *cardiomapp_file;
	int device_offset,i,index,number_of_devices,number_of_samples,
		number_of_signals,return_code,*time;
	short int *value;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;

	ENTER(read_cardiomapp_file);
	if (file_name&&(rig=(struct Rig *)rig_void))
	{
		if (cardiomapp_file=fopen(file_name,"rb"))
		{
			/* read the file header */
			if (1446==fread(header_buffer,sizeof(char),1446,cardiomapp_file))
			{
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
				/* convert from big to little endian */
				swap=header_buffer+1380;
				temp_char=swap[0];
				swap[0]=swap[1];
				swap[1]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
				number_of_signals=(int)(*((short *)(header_buffer+1380)));
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
				/* convert from big to little endian */
				swap=header_buffer+256;
				temp_char=swap[0];
				swap[0]=swap[1];
				swap[1]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
				number_of_samples=(int)(*((short *)(header_buffer+256)))*
					(int)DMA_TRANSFER_WORDS;
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
				/* convert from big to little endian */
				swap=header_buffer+1378;
				temp_char=swap[0];
				swap[0]=swap[1];
				swap[1]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
				sampling_frequency=(float)(*((short *)(header_buffer+1378)));
				if ((0<number_of_signals)&&(0==number_of_samples%number_of_signals)&&
					(0<(number_of_samples /= number_of_signals))&&
					(0<sampling_frequency))
				{
					/* create the signal buffer */
					if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,
						number_of_signals,number_of_samples,sampling_frequency))
					{
						/* read the signals */
							/*???DB.  BINARY_FIELD_READ converts from little to big endian on
								the PC */
						if (number_of_signals*number_of_samples==fread(
							(char *)signal_buffer->signals.short_int_values,
							sizeof(short int),number_of_samples*number_of_signals,
							cardiomapp_file))
						{
							/* A/D converters are 12 bit */
							value=signal_buffer->signals.short_int_values;
							for (i=number_of_signals*number_of_samples;i>0;i--)
							{
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
								/* convert from big to little endian */
								swap=(char *)value;
								temp_char=swap[0];
								swap[0]=swap[1];
								swap[1]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
								*value &= 0x0fff;
								*value -= 2048;
								value++;
							}
							/* assign the times */
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* link the devices to the signals */
							return_code=1;
							i=rig->number_of_devices;
							device=rig->devices;
							number_of_devices=0;
							while (return_code&&(i>0))
							{
								if ((*device)&&((*device)->description))
								{
									if (((*device)->channel)&&
										(0<=(index=((*device)->channel->number)-1))&&
										(index<number_of_signals))
									{
										if (ELECTRODE==(*device)->description->type)
										{
											if (!((*device)->signal=create_Signal(index,
												signal_buffer,UNDECIDED,0)))
											{
												return_code=0;
											}
										}
										else
										{
											if (!((*device)->signal=create_Signal(index,
												signal_buffer,REJECTED,0)))
											{
												return_code=0;
											}
										}
										if (return_code)
										{
											number_of_devices++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_cardiomapp_file.  Could not create signal");
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Invalid channel for device %s",
											(*device)->description->name);
										if ((*device)->description->region)
										{
											((*device)->description->region->number_of_devices)--;
										}
										destroy_Device(device);
									}
								}
								else
								{
									if (*device)
									{
										destroy_Device(device);
									}
								}
								device++;
								i--;
							}
							if (return_code&&(0<number_of_devices))
							{
								if (number_of_devices!=rig->number_of_devices)
								{
									/* strip out destroyed devices */
									device_offset=0;
									device=rig->devices;
									for (i=rig->number_of_devices;i>0;i--)
									{
										if (device[device_offset])
										{
											if (0<device_offset)
											{
												*device=device[device_offset];
											}
											device++;
										}
										else
										{
											device_offset++;
										}
									}
									rig->number_of_devices=number_of_devices;
								}
							}
							else
							{
								destroy_Signal_buffer(&signal_buffer);
								if (return_code)
								{
									display_message(ERROR_MESSAGE,"No valid channels");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_cardiomapp_file.  Error reading signal values");
							destroy_Signal_buffer(&signal_buffer);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_cardiomapp_file.  Could not create signal buffer");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_cardiomapp_file.  No data in file: %s",file_name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_cardiomapp_file.  Could not read file header");
				return_code=0;
			}
			fclose(cardiomapp_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_cardiomapp_file.  Could not open file: %s",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_cardiomapp_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_cardiomapp_file */

#if defined (OLD_CODE)
int main()
{
	char buffer[sizeof(struct Rdt_parameters)],*swap,temp_char;
	FILE *cardiomapp_file;
	struct Rdt_parameters rdt_parameters;

	if (cardiomapp_file=fopen("information/f7.rdt","rb"))
	{
		if (1==fread((char *)&rdt_parameters,sizeof(struct Rdt_parameters),1,
			cardiomapp_file))
		{
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
			/* convert from big to little endian */
			swap=(char *)&(rdt_parameters.id_stamp);
			temp_char=swap[0];
			swap[0]=swap[3];
			swap[3]=temp_char;
			temp_char=swap[1];
			swap[1]=swap[2];
			swap[2]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
			printf("%d\n",rdt_parameters.id_stamp);
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
			/* convert from big to little endian */
			swap=(char *)&(rdt_parameters.modify_stamp);
			temp_char=swap[0];
			swap[0]=swap[3];
			swap[3]=temp_char;
			temp_char=swap[1];
			swap[1]=swap[2];
			swap[2]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
			printf("%d\n",rdt_parameters.modify_stamp);
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
			/* convert from big to little endian */
			swap=(char *)&(rdt_parameters.check_sum);
			temp_char=swap[0];
			swap[0]=swap[3];
			swap[3]=temp_char;
			temp_char=swap[1];
			swap[1]=swap[2];
			swap[2]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
			printf("%d\n",rdt_parameters.check_sum);
			printf("%40s\n",rdt_parameters.version);
			printf("%40s\n",rdt_parameters.description);
			printf("%40s\n",rdt_parameters.comment);
		}
		else
		{
			printf("Could not read header\n");
		}
	}
	else
	{
		printf("Could not open information/f7.rdt\n");
	}
} /* main */
#endif /* defined (OLD_CODE) */
