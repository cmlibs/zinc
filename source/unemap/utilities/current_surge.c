/*******************************************************************************
FILE : current_surge.c

LAST MODIFIED : 9 April 2000

DESCRIPTION :
Code for turning a unemap power distribution unit on and off until a current
surge occurs.  It will be monitoring several channels (VCC, VCC-, VCC+ and
others of interest).

???DB.  Put together from unemap_hardware.c and register.c
==============================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#if defined (WINDOWS)
#if defined (NI_DAQ)
#include "nidaq.h"
/* for RTSI */
#include "nidaqcns.h"
#endif /* defined (NI_DAQ) */
#endif /* defined (WINDOWS) */
#include "general/debug.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
???DB.  Used to be enum UNEMAP_hardware_version, but want to be able to do
	bitwise ors to get mixtures
???DB.  Replace UnEmap by UNEMAP when got rid of code switchs
==============================================================================*/
/* signal conditioning card used in second Oxford interim system */
#define UnEmap_1V2 (0x1)
/* signal conditioning card for Oxford final system */
#define UnEmap_2V1 (0x2)
/* current signal conditioning card */
#define UnEmap_2V2 (0x4)

/* see unemap_configure for a description of the arguments */
typedef void (Unemap_hardware_callback)(int,int *,int,short *,void *);

/* see unemap_calibrate for a description of the arguments */
typedef void (Calibration_end_callback)(const int,const int *,const float *, \
	const float *,void *);

/*
Module constants
----------------
*/
#if defined (NI_DAQ)
#define PCI6031E_DEVICE_CODE 220
#define PCI6033E_DEVICE_CODE 222
#define PXI6031E_DEVICE_CODE 259
#define PXI6071E_DEVICE_CODE 258
#define DIGITAL_IO_COUNTER ND_COUNTER_1
#define NUMBER_OF_CHANNELS_ON_NI_CARD ((i16)64)
#define SCAN_COUNTER ND_COUNTER_0
/*???DB.  Would like CALIBRATE_CHANNEL and STIMULATE_CHANNEL to be constants,
	but I think that they swapped between UnEmap_1V2 and UnEmap_2V1.  Also,
	they were both 1 for the last UnEmap_1V2 executable */
i16 CALIBRATE_CHANNEL=0,STIMULATE_CHANNEL=1;
#define PC_LED_SHIFT_REGISTER_UnEmap2vx 1
#define Stim_Source_SHIFT_REGISTER_UnEmap2vx 2
#define EXT_ISO_SHIFT_REGISTER_UnEmap2vx 3
#define D_CAL_SHIFT_REGISTER_UnEmap2vx 4
#define D_REC_SHIFT_REGISTER_UnEmap2vx 5
#define FCS_SHIFT_REGISTER_UnEmap2vx 6
#define FUD_SHIFT_REGISTER_UnEmap2vx 7
#define Master_SHIFT_REGISTER_UnEmap2v1 8
#define Batt24_SHIFT_REGISTER_UnEmap2vx 14
#define BattB_SHIFT_REGISTER_UnEmap2vx 15
#define BattA_SHIFT_REGISTER_UnEmap2vx 16
/* the first stimulate channel */
#define Stim_Base_SHIFT_REGISTER_UnEmap2vx 17
#define MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx (float)4.5
/*???DB.  Will eventually be a constant ? */
/*???DB.  Not happy with settling constants ?  Perhaps OK because dealing with
	448 channels rather than just 64 (register) */
#define CALIBRATE_FREQUENCY ((float)100)
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
float switching_time=(float)0.0003;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
float tol_settling;
int settling_step_max;
float initial_antialiasing_filter_frequency;
/*???debug */
	/*???DB.  Read in, but no longer used */
int number_of_valid_ni_channels=64;
#define NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL 4
#endif /* defined (NI_DAQ) */

/*
Module types
------------
*/
#if defined (NI_DAQ)
enum NI_card_type
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
???DB.  Might just want to have if its AD or AD_DA ? 
==============================================================================*/
{
	PCI6031E_AD_DA,
	PCI6033E_AD,
	PXI6031E_AD_DA,
	PXI6071E_AD_DA
}; /* enum NI_card_type */

struct NI_card
/*******************************************************************************
LAST MODIFIED : 10 September 1999

DESCRIPTION :
Information associated with each NI card/signal conditioning card pair.
==============================================================================*/
{
	enum NI_card_type type;
	/* to make the pinout for UnEmap_1V2 the same as for later versions */
	int channel_reorder[NUMBER_OF_CHANNELS_ON_NI_CARD];
	i16 device_number;
	HGLOBAL memory_object;
	i16 *hardware_buffer;
	i16 gain,input_mode,polarity,time_base;
	u16 sampling_interval;
	u32 hardware_buffer_size;
	int unemap_hardware_version;
	int anti_aliasing_filter_taps;
	struct
	{
		/* the current calibrate/record mode of the card */
		unsigned char isolate_mode;
		/* the stimulate channel settings.  Need to be saved while in calibrate
			mode */
		unsigned char stimulate_channels[8];
		/* the current settings of the NI output lines (that are not for control) */
		unsigned char gain_output_line_0,gain_output_line_1;
		/* the current settings of the 80 shift register digital outputs */
		unsigned char shift_register[10];
	} unemap_2vx;
}; /* struct NI_card */
#endif /* defined (NI_DAQ) */

/*
Module variables
----------------
*/
#if defined (NI_DAQ)
/*???DB.  Can't free da_buffer while analog output is in progress */
i16 *da_buffer=(i16 *)NULL;
i16 filter_increment_output_line;
i16 chip_select_line_UnEmap1vx,id_line_1_UnEmap1vx,id_line_2_UnEmap1vx,
	id_line_3_UnEmap1vx,led_line_UnEmap1vx,up_down_line_UnEmap1vx;
int relay_power_on_UnEmap2vx=0;
/* battery_A_line_UnEmap2v1/master_line_UnEmap2v2 is output for the first NI
	card and input for the others */
i16 battery_A_line_UnEmap2v1,battery_good_input_line_UnEmap2vx,
	gain_0_output_line_UnEmap2vx,gain_1_output_line_UnEmap2vx,
	master_line_UnEmap2v2,shift_register_clock_output_line_UnEmap2vx,
	shift_register_data_output_line_UnEmap2vx,
	shift_register_strobe_output_line_UnEmap2vx;
i16 BattA_setting_UnEmap2vx;
	/*???DB.  Get rid of BattA_setting_UnEmap2vx */
/*???debug */
char first_sample=1;
unsigned long max_sample_number,min_sample_number;
#endif /* defined (NI_DAQ) */

/* scrolling information set by unemap_configure and
	unemap_set_scrolling_channel */
Unemap_hardware_callback
	*module_scrolling_callback=(Unemap_hardware_callback *)NULL;
void *module_scrolling_callback_data=(void *)NULL;
#if defined (WINDOWS)
UINT module_scrolling_message=(UINT)0;
HWND module_scrolling_window=(HWND)NULL;
#endif /* defined (WINDOWS) */
int module_number_of_scrolling_channels=0,
	*module_scrolling_channel_numbers=(int *)NULL,module_scrolling_refresh_period;
int module_sampling_on=0,module_scrolling_on=1;

Calibration_end_callback *module_calibration_end_callback=
	(Calibration_end_callback *)NULL;
void *module_calibration_end_callback_data=NULL;
int *module_calibration_channels=(int *)NULL,
	module_calibration_number_of_channels=0;
float *module_calibration_gains=(float *)NULL,
	*module_calibration_offsets=(float *)NULL;

typedef int (Buffer_full_callback)(void *);
Buffer_full_callback *module_buffer_full_callback=(Buffer_full_callback *)NULL;
void *module_buffer_full_callback_data=NULL;

#if defined (NI_DAQ)
/* hardware information */
	/*???DB.  NI_CARDS->ni_cards, once made sure no other ni_cards */
float module_sampling_frequency=(float)0;
int module_configured=0,module_number_of_NI_CARDS=0,
	module_number_of_stimulators=0,*module_stimulator_NI_CARD_indices=(int *)NULL;
struct NI_card *module_NI_CARDS=(struct NI_card *)NULL;
unsigned long module_sample_buffer_size=0,module_starting_sample_number;
u32 module_sampling_high_count=0,module_sampling_low_count=0;
#endif /* defined (NI_DAQ) */
int module_slave=0;

int max_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;

/*
Module functions
----------------
*/
static FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type)
/*******************************************************************************
LAST MODIFIED : 11 January 1999

DESCRIPTION :
Opens a file in the UNEMAP_HARDWARE directory.
==============================================================================*/
{
	char *hardware_directory,*hardware_file_name;
	FILE *hardware_file;

	ENTER(fopen_UNEMAP_HARDWARE);
	hardware_file=(FILE *)NULL;
	if (file_name&&type)
	{
		hardware_file_name=(char *)NULL;
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			if (ALLOCATE(hardware_file_name,char,strlen(hardware_directory)+
				strlen(file_name)+2))
			{
				strcpy(hardware_file_name,hardware_directory);
#if defined (WIN32)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32) */
			}
		}
		else
		{
			if (ALLOCATE(hardware_file_name,char,strlen(file_name)+1))
			{
				hardware_file_name[0]='\0';
			}
		}
		if (hardware_file_name)
		{
			strcat(hardware_file_name,file_name);
			hardware_file=fopen(hardware_file_name,type);
			DEALLOCATE(hardware_file_name);
		}
	}
	LEAVE;

	return (hardware_file);
} /* fopen_UNEMAP_HARDWARE */

#if defined (NI_DAQ)
static int set_shift_register(struct NI_card *ni_card,int register_number,
	int high,int apply)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If <high> is not zero then shift <register_number> of the <ni_card> is set high,
other wise the register is set low.  If <apply> is non-zero then this change is
downloaded to the physical device otherwise the change is cached - this allows
several changes to be applied at once.
==============================================================================*/
{
	int bit_number,byte_number,return_code;
	i16 status;
	unsigned char bit_mask;
	u32 counter,counter_start;

	ENTER(set_shift_register);
	return_code=0;
	/* check arguments */
	if (ni_card&&
		((UnEmap_2V1==ni_card->unemap_hardware_version)||
		(UnEmap_2V2==ni_card->unemap_hardware_version))&&
		(0<register_number)&&(register_number<=80))
	{
		byte_number=(register_number-1)/8;
		bit_mask=(unsigned char)0x1;
		bit_mask <<= (register_number-1)%8;
		if (high)
		{
			((ni_card->unemap_2vx).shift_register)[byte_number] |= bit_mask;
		}
		else
		{
			((ni_card->unemap_2vx).shift_register)[byte_number] &= ~bit_mask;
		}
		if (apply)
		{
			/* use timer to make sure that don't go too quickly */
			/* make sure that RCLK and Rstrobe are low */
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_clock_output_line_UnEmap2vx,(i16)0);
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)0);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
			/* clock in the data */
			for (byte_number=9;byte_number>=0;byte_number--)
			{
				bit_mask=(unsigned char)0x80;
				for (bit_number=7;bit_number>=0;bit_number--)
				{
					/* set the data line */
					if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							shift_register_data_output_line_UnEmap2vx,(i16)1);
					}
					else
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							shift_register_data_output_line_UnEmap2vx,(i16)0);
					}
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set the clock line high */
					DIG_Out_Line(ni_card->device_number,(i16)0,
						shift_register_clock_output_line_UnEmap2vx,(i16)1);
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set the clock line low */
					DIG_Out_Line(ni_card->device_number,(i16)0,
						shift_register_clock_output_line_UnEmap2vx,(i16)0);
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					bit_mask >>= 1;
				}
			}
			/* apply strobe */
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)1);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)0);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
		}
		return_code=1;
	}
	else
	{
		if (ni_card)
		{
			display_message(ERROR_MESSAGE,
				"set_shift_register.  Invalid argument(s).  %p %d %d (%d or %d)",
				ni_card,register_number,ni_card->unemap_hardware_version,UnEmap_2V1,
				UnEmap_2V2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_shift_register.  Invalid argument(s).  %p %d",ni_card,
				register_number);
		}
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_shift_register */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
#define READ_WORD( file , word ) \
fscanf(file," %80[^=]",word); \
i=strlen(word); \
while ((i>0)&&(isspace(word[i-1]))) \
{ \
	i--; \
} \
word[i]='\0';

static int search_for_NI_cards(void)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
==============================================================================*/
{
	char word[81];
	int unemap_hardware_version;
	FILE *digital_io_lines;
	float gain;
	int i,j,return_code,*stimulator_card_indices;
	i16 card_code,card_number,GA0_setting,GA1_setting,input_mode,polarity,status;
	struct NI_card *ni_card;
	unsigned char shift_registers[10];
	unsigned short shift_register;
	u16 sampling_interval;

	ENTER(search_for_NI_cards);
	return_code=1;
	if (!module_NI_CARDS)
	{
		if (digital_io_lines=fopen_UNEMAP_HARDWARE("digital.txt","r"))
		{
			module_NI_CARDS=(struct NI_card *)NULL;
			module_number_of_NI_CARDS=0;
			module_number_of_stimulators=0;
			module_stimulator_NI_CARD_indices=(int *)NULL;
			card_number=1;
			word[0]='\0';
			/* determine the hardware version */
			return_code=0;
			if (1==fscanf(digital_io_lines," cs = %hd ",
				&chip_select_line_UnEmap1vx))
			{
				unemap_hardware_version=UnEmap_1V2;
				READ_WORD(digital_io_lines,word);
				return_code=1;
			}
			else
			{
				if (1==fscanf(digital_io_lines," BattGood = %hd ",
					&battery_good_input_line_UnEmap2vx))
				{
					unemap_hardware_version=UnEmap_2V1;
					READ_WORD(digital_io_lines,word);
					return_code=1;
				}
				else
				{
					if (1==fscanf(digital_io_lines," version = %10s ",word))
					{
						if (!strcmp("UnEmap_1vx",word))
						{
							unemap_hardware_version=UnEmap_1V2;
							chip_select_line_UnEmap1vx=0;
							READ_WORD(digital_io_lines,word);
							if (!strcmp("cs",word))
							{
								fscanf(digital_io_lines," = %hd ",&chip_select_line_UnEmap1vx);
								READ_WORD(digital_io_lines,word);
							}
						}
						else
						{
							if (!strcmp(word,"UnEmap_2v1"))
							{
								unemap_hardware_version=UnEmap_2V1;
								return_code=1;
							}
							else
							{
								if (!strcmp(word,"UnEmap_2v2"))
								{
									unemap_hardware_version=UnEmap_2V2;
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Unknown version, %s, from digital.txt",word);
									return_code=0;
								}
							}
							if (return_code)
							{
								battery_good_input_line_UnEmap2vx=0;
								READ_WORD(digital_io_lines,word);
								if (!strcmp("BattGood",word))
								{
									fscanf(digital_io_lines," = %hd ",
										&battery_good_input_line_UnEmap2vx);
									READ_WORD(digital_io_lines,word);
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading version from digital.txt");
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				initial_antialiasing_filter_frequency=(float)200;
				input_mode=1;
				polarity=0;
				gain=(float)1;
				sampling_interval=0;
				switch (unemap_hardware_version)
				{
					case UnEmap_1V2:
					{
						/* NB.  chip_select_line_UnEmap1vx default is set above (before it
							is read in */
						filter_increment_output_line=4;
						up_down_line_UnEmap1vx=5;
						led_line_UnEmap1vx=2;
						id_line_1_UnEmap1vx=6;
						id_line_2_UnEmap1vx=3;
						id_line_3_UnEmap1vx=7;
#if defined (OLD_CODE)
						/* for UnEmap_1V1 (no working systems) */
						filter_increment_output_line=5;
						up_down_line_UnEmap1vx=4;
						led_line_UnEmap1vx=2;
						id_line_1_UnEmap1vx=6;
						id_line_2_UnEmap1vx=3;
						id_line_3_UnEmap1vx=7;
#endif /* defined (OLD_CODE) */
						if (0==strcmp("u/d",word))
						{
							fscanf(digital_io_lines," = %hd ",&up_down_line_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("inc",word))
						{
							fscanf(digital_io_lines," = %hd ",&filter_increment_output_line);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("led",word))
						{
							fscanf(digital_io_lines," = %hd ",&led_line_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id1",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_1_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id2",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_2_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id3",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_3_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("filter",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&initial_antialiasing_filter_frequency);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("gain",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("input_mode",word))
						{
							fscanf(digital_io_lines," = %hd ",&input_mode);
							if (input_mode< -1)
							{
								input_mode= -1;
							}
							else
							{
								if (input_mode>2)
								{
									input_mode=2;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("polarity",word))
						{
							fscanf(digital_io_lines," = %hd ",&polarity);
							if (polarity<0)
							{
								polarity=0;
							}
							else
							{
								if (polarity>1)
								{
									polarity=1;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						/*???debug */
						{
							FILE *unemap_debug;

							if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
							{
								fprintf(unemap_debug,"version=UnEmap_1vx\n");
								fprintf(unemap_debug,"chip_select_line=%d\n",
									chip_select_line_UnEmap1vx);
								fprintf(unemap_debug,"up_down_line=%d\n",
									up_down_line_UnEmap1vx);
								fprintf(unemap_debug,"increment_line=%d\n",
									filter_increment_output_line);
								fprintf(unemap_debug,"led_line=%d\n",led_line_UnEmap1vx);
								fprintf(unemap_debug,"id_line_1=%d\n",id_line_1_UnEmap1vx);
								fprintf(unemap_debug,"id_line_2=%d\n",id_line_2_UnEmap1vx);
								fprintf(unemap_debug,"id_line_3=%d\n",id_line_3_UnEmap1vx);
								fprintf(unemap_debug,"antialiasing_filter_frequency=%g\n",
									initial_antialiasing_filter_frequency);
								fprintf(unemap_debug,"gain=%g\n",gain);
								fprintf(unemap_debug,"input_mode=");
								switch (input_mode)
								{
									case -1:
									{
										fprintf(unemap_debug,"none\n");
									} break;
									case 0:
									{
										fprintf(unemap_debug,"differential\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"referenced single-ended\n");
									} break;
									case 2:
									{
										fprintf(unemap_debug,"non-referenced single-ended\n");
									} break;
								}
								fprintf(unemap_debug,"polarity=");
								switch (polarity)
								{
									case 0:
									{
										fprintf(unemap_debug,"bipolar\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"unipolar\n");
									} break;
								}
								fclose(unemap_debug);
							}
						}
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					{
						filter_increment_output_line=5;
						switch (unemap_hardware_version)
						{
							case UnEmap_2V1:
							{
								battery_A_line_UnEmap2v1=2;
							} break;
							case UnEmap_2V2:
							{
								master_line_UnEmap2v2=2;
							} break;
						}
						gain_0_output_line_UnEmap2vx=7;
						gain_1_output_line_UnEmap2vx=3;
						shift_register_clock_output_line_UnEmap2vx=6;
						shift_register_data_output_line_UnEmap2vx=4;
						shift_register_strobe_output_line_UnEmap2vx=1;
						BattA_setting_UnEmap2vx=0;
						GA0_setting=0;
						GA1_setting=0;
						shift_registers[0]=(unsigned char)0x79;
						shift_registers[1]=(unsigned char)0x0;
						for (i=2;i<10;i++)
						{
							shift_registers[i]=(unsigned char)0xff;
						}
						tol_settling=(float)1.;
						settling_step_max=20;
						if (0==strcmp("Rstrobe",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_strobe_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA1",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain_1_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("Rdata",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_data_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("FINC",word))
						{
							fscanf(digital_io_lines," = %hd ",&filter_increment_output_line);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("RCLK",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_clock_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA0",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain_0_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("filter",word))
						{
							fscanf(digital_io_lines," = %f ",
								&initial_antialiasing_filter_frequency);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("gain",word))
						{
							fscanf(digital_io_lines," = %f ",&gain);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("input_mode",word))
						{
							fscanf(digital_io_lines," = %hd ",&input_mode);
							if (input_mode< -1)
							{
								input_mode= -1;
							}
							else
							{
								if (input_mode>2)
								{
									input_mode=2;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("polarity",word))
						{
							fscanf(digital_io_lines," = %hd ",&polarity);
							if (polarity<0)
							{
								polarity=0;
							}
							else
							{
								if (polarity>1)
								{
									polarity=1;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA0 setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&GA0_setting);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA1 setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&GA1_setting);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("shift registers",word))
						{
							fscanf(digital_io_lines," = ");
							j=0;
							while ((j<10)&&
								(1==fscanf(digital_io_lines,"%2hx",&shift_register)))
							{
								shift_registers[j]=(unsigned char)shift_register;
								j++;
							}
							if (j<10)
							{
								display_message(ERROR_MESSAGE,
									"search_for_NI_cards.  Could not read shift registers");
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("settling magnitude",word))
						{
							fscanf(digital_io_lines," = %f ",&tol_settling);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("relay power on",word))
						{
							fscanf(digital_io_lines," = %d ",&relay_power_on_UnEmap2vx);
							if (relay_power_on_UnEmap2vx)
							{
								shift_registers[0] &= (unsigned char)0xe7;
							}
							else
							{
								shift_registers[0] |= (unsigned char)0x18;
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("number of valid ni channels",word))
						{
							fscanf(digital_io_lines," = %d ",&number_of_valid_ni_channels);
							if (number_of_valid_ni_channels<1)
							{
								number_of_valid_ni_channels=1;
							}
							else
							{
								if (number_of_valid_ni_channels>NUMBER_OF_CHANNELS_ON_NI_CARD)
								{
									number_of_valid_ni_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						switch (unemap_hardware_version)
						{
							case UnEmap_2V1:
							{
								if (0==strcmp("BattA",word))
								{
									fscanf(digital_io_lines," = %hd ",&battery_A_line_UnEmap2v1);
									READ_WORD(digital_io_lines,word);
								}
							} break;
							case UnEmap_2V2:
							{
								if (0==strcmp("Master",word))
								{
									fscanf(digital_io_lines," = %hd ",&master_line_UnEmap2v2);
									READ_WORD(digital_io_lines,word);
								}
							} break;
						}
						if (0==strcmp("BattA setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&BattA_setting_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("sampling interval",word))
						{
							fscanf(digital_io_lines," = %hd ",&sampling_interval);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("max settling steps",word))
						{
							fscanf(digital_io_lines," = %d ",&settling_step_max);
							READ_WORD(digital_io_lines,word);
						}
						/*???debug */
						{
							FILE *unemap_debug;

							if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
							{
								switch (unemap_hardware_version)
								{
									case UnEmap_2V1:
									{
										fprintf(unemap_debug,"version=UnEmap_2v1\n");
									} break;
									case UnEmap_2V2:
									{
										fprintf(unemap_debug,"version=UnEmap_2v2\n");
									} break;
								}
								fprintf(unemap_debug,"battery_good_input_line=%d\n",
									battery_good_input_line_UnEmap2vx);
								fprintf(unemap_debug,"shift_register_strobe_output_line=%d\n",
									shift_register_strobe_output_line_UnEmap2vx);
								fprintf(unemap_debug,"gain_1_output_line=%d\n",
									gain_1_output_line_UnEmap2vx);
								fprintf(unemap_debug,"shift_register_data_output_line=%d\n",
									shift_register_data_output_line_UnEmap2vx);
								fprintf(unemap_debug,"filter_increment_output_line=%d\n",
									filter_increment_output_line);
								fprintf(unemap_debug,"shift_register_clock_output_line=%d\n",
									shift_register_clock_output_line_UnEmap2vx);
								fprintf(unemap_debug,"gain_0_output_line=%d\n",
									gain_0_output_line_UnEmap2vx);
								fprintf(unemap_debug,"antialiasing_filter_frequency=%g\n",
									initial_antialiasing_filter_frequency);
								fprintf(unemap_debug,"gain=%g\n",gain);
								fprintf(unemap_debug,"input_mode=");
								switch (input_mode)
								{
									case -1:
									{
										fprintf(unemap_debug,"none\n");
									} break;
									case 0:
									{
										fprintf(unemap_debug,"differential\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"referenced single-ended\n");
									} break;
									case 2:
									{
										fprintf(unemap_debug,"non-referenced single-ended\n");
									} break;
								}
								fprintf(unemap_debug,"polarity=");
								switch (polarity)
								{
									case 0:
									{
										fprintf(unemap_debug,"bipolar\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"unipolar\n");
									} break;
								}
								fprintf(unemap_debug,"GA0 setting=");
								if (GA0_setting)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"GA1 setting=");
								if (GA1_setting)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"shift registers=");
								for (j=0;j<10;j++)
								{
									fprintf(unemap_debug,"%02x",
										(unsigned short)shift_registers[j]);
								}
								fprintf(unemap_debug,"\n");
								fprintf(unemap_debug,"settling_magnitude=%g\n",tol_settling);
								fprintf(unemap_debug,"relay_power_on=%d\n",
									relay_power_on_UnEmap2vx);
								fprintf(unemap_debug,"number_of_valid_ni_channels=%d\n",
									number_of_valid_ni_channels);
								switch (unemap_hardware_version)
								{
									case UnEmap_2V1:
									{
										fprintf(unemap_debug,"battery_A_line=%d\n",
											battery_A_line_UnEmap2v1);
									} break;
									case UnEmap_2V2:
									{
										fprintf(unemap_debug,"master_line=%d\n",
											master_line_UnEmap2v2);
									} break;
								}
								fprintf(unemap_debug,"BattA setting=");
								if (BattA_setting_UnEmap2vx)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"sampling interval=%d\n",
									sampling_interval);
								fprintf(unemap_debug,"max settling steps=%d\n",
									settling_step_max);
								fclose(unemap_debug);
							}
						}
					} break;
					default:
					{
						return_code=0;
					} break;
				}
				if (return_code)
				{
					/* gain should be 1,2,5,10,20,50 or 100 */
					if (gain<2)
					{
						gain=(float)1;
					}
					else
					{
						if (gain<4)
						{
							gain=(float)2;
						}
						else
						{
							if (gain<8)
							{
								gain=(float)5;
							}
							else
							{
								if (gain<15)
								{
									gain=(float)10;
								}
								else
								{
									if (gain<35)
									{
										gain=(float)20;
									}
									else
									{
										if (gain<75)
										{
											gain=(float)50;
										}
										else
										{
											gain=(float)100;
										}
									}
								}
							}
						}
					}
					while (return_code&&(0==Init_DA_Brds(card_number,&card_code)))
					{
						if ((PCI6031E_DEVICE_CODE==card_code)||
							(PCI6033E_DEVICE_CODE==card_code)||
							(PXI6031E_DEVICE_CODE==card_code)||
							(PXI6071E_DEVICE_CODE==card_code))
						{
							/* make sure that not mixing 12-bit and 16-bit */
							if ((0==module_number_of_NI_CARDS)||
								((PXI6071E_DEVICE_CODE==card_code)&&
								(PXI6071E_AD_DA==(module_NI_CARDS[0]).type))||
								((PXI6071E_DEVICE_CODE!=card_code)&&
								(PXI6071E_AD_DA!=(module_NI_CARDS[0]).type)))
							{
								if (REALLOCATE(ni_card,module_NI_CARDS,struct NI_card,
									module_number_of_NI_CARDS+1))
								{
									module_NI_CARDS=ni_card;
									ni_card += module_number_of_NI_CARDS;
									ni_card->device_number=card_number;
									ni_card->unemap_hardware_version=unemap_hardware_version;
									switch (card_code)
									{
										case PCI6031E_DEVICE_CODE:
										{
											ni_card->type=PCI6031E_AD_DA;
										} break;
										case PCI6033E_DEVICE_CODE:
										{
											ni_card->type=PCI6033E_AD;
										} break;
										case PXI6031E_DEVICE_CODE:
										{
											ni_card->type=PXI6031E_AD_DA;
										} break;
										case PXI6071E_DEVICE_CODE:
										{
											ni_card->type=PXI6071E_AD_DA;
										} break;
									}
									if ((PCI6031E_AD_DA==ni_card->type)||
										(PXI6031E_AD_DA==ni_card->type)||
										(PXI6071E_AD_DA==ni_card->type))
									{
										if (REALLOCATE(stimulator_card_indices,
											module_stimulator_NI_CARD_indices,int,
											module_number_of_stimulators+1))
										{
											module_stimulator_NI_CARD_indices=stimulator_card_indices;
											module_stimulator_NI_CARD_indices[
												module_number_of_stimulators]=module_number_of_NI_CARDS;
											module_number_of_stimulators++;
										}
									}
									ni_card->memory_object=(HGLOBAL)NULL;
									ni_card->hardware_buffer=(i16 *)NULL;
									ni_card->time_base=0;
									ni_card->sampling_interval=sampling_interval;
									ni_card->hardware_buffer_size=0;
									ni_card->input_mode=0;
									ni_card->polarity=0;
									ni_card->gain=(i16)gain;
									/*???DB.  To get -10 to 10 */
									ni_card->gain= -1;
									/* initially the filter setting is undefined */
									ni_card->anti_aliasing_filter_taps= -1;
									ni_card->input_mode=input_mode;
									ni_card->polarity=polarity;
									if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
										(UnEmap_2V2==ni_card->unemap_hardware_version))
									{
										(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x0;
										(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x0;
										(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x0;
										for (i=9;i>=0;i--)
										{
											((ni_card->unemap_2vx).shift_register)[i]=
												(unsigned char)0x0;
										}
										for (j=0;j<10;j++)
										{
											(ni_card->unemap_2vx).shift_register[j]=
												shift_registers[j];
										}
										for (j=0;j<8;j++)
										{
											(ni_card->unemap_2vx).stimulate_channels[j]=
												shift_registers[j+2];
										}
									}
									/* to make the pinout for UnEmap_1V2 the same as for later
										versions */
									if (UnEmap_1V2==ni_card->unemap_hardware_version)
									{
										for (i=0;i<16;i++)
										{
											(ni_card->channel_reorder)[i]=55-i;
										}
										for (i=16;i<32;i++)
										{
											(ni_card->channel_reorder)[i]=31-(i/2)+(8*(i%2));
										}
										for (i=32;i<40;i++)
										{
											(ni_card->channel_reorder)[i]=95-i;
										}
										for (i=40;i<48;i++)
										{
											(ni_card->channel_reorder)[i]=79-i;
										}
										for (i=48;i<64;i++)
										{
											(ni_card->channel_reorder)[i]=39-(i/2)-(8*(i%2));
										}
									}
									else
									{
										for (i=NUMBER_OF_CHANNELS_ON_NI_CARD-1;i>=0;i--)
										{
											(ni_card->channel_reorder)[i]=i;
										}
									}
									if (0<=input_mode)
									{
										/* configure the card */
										status=AI_Clear(ni_card->device_number);
										/* analog input */
										status=AI_Configure(ni_card->device_number,
											/* all channels */(i16)(-1),input_mode,
											/* ignored (input range) */(i16)0,polarity,
											/* do not drive AISENSE to ground */(i16)0);
									}
									if (0==status)
									{
										/* digital I/O */
										/* start counter used for digital I/O timing */
										GPCTR_Control(ni_card->device_number,DIGITAL_IO_COUNTER,
											ND_RESET);
										GPCTR_Set_Application(ni_card->device_number,
											DIGITAL_IO_COUNTER,ND_SIMPLE_EVENT_CNT);
										GPCTR_Change_Parameter(ni_card->device_number,
											DIGITAL_IO_COUNTER,ND_SOURCE,ND_INTERNAL_20_MHZ);
										GPCTR_Control(ni_card->device_number,DIGITAL_IO_COUNTER,
											ND_PROGRAM);
										/* configure digital I/O lines */
										switch (ni_card->unemap_hardware_version)
										{
											case UnEmap_1V2:
											{
												status=DIG_Line_Config(ni_card->device_number,0,
													chip_select_line_UnEmap1vx,(i16)1);
												if (0==status)
												{
													status=DIG_Line_Config(ni_card->device_number,0,
														up_down_line_UnEmap1vx,(i16)1);
													if (0==status)
													{
														status=DIG_Line_Config(ni_card->device_number,0,
															filter_increment_output_line,(i16)1);
														if (0==status)
														{
															status=DIG_Line_Config(ni_card->device_number,0,
																led_line_UnEmap1vx,(i16)1);
															if (0==status)
															{
																status=DIG_Line_Config(ni_card->device_number,0,
																	id_line_1_UnEmap1vx,(i16)0);
																if (0==status)
																{
																	status=DIG_Line_Config(ni_card->device_number,
																		0,id_line_2_UnEmap1vx,(i16)0);
																	if (0==status)
																	{
																		status=DIG_Line_Config(
																			ni_card->device_number,0,
																			id_line_3_UnEmap1vx,(i16)0);
																		if (0==status)
																		{
																			/* set cs high to disable programming */
																			status=DIG_Out_Line(
																				ni_card->device_number,0,
																				chip_select_line_UnEmap1vx,(i16)1);
																			if (0==status)
																			{
																				/* set the led on */
																				status=DIG_Out_Line(
																					ni_card->device_number,0,
																					led_line_UnEmap1vx,(i16)1);
																				if (0==status)
																				{
																					i16 state;

																					/* read the analog front-end card
																						id */
																					status=DIG_In_Line(
																						ni_card->device_number,0,
																						id_line_1_UnEmap1vx,&state);
																					if (0==status)
																					{
																						status=DIG_In_Line(
																							ni_card->device_number,0,
																							id_line_2_UnEmap1vx,&state);
																						if (0==status)
																						{
																							status=DIG_In_Line(
																								ni_card->device_number,0,
																								id_line_3_UnEmap1vx,&state);
																							if (0==status)
																							{
																							}
																							else
																							{
																								display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_3_UnEmap1vx");
																							}
																						}
																						else
																						{
																							display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_2_UnEmap1vx");
																						}
																					}
																					else
																					{
																						display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_1_UnEmap1vx");
																					}
																				}
																				else
																				{
																					display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_Out_Line failed for led_line_UnEmap1vx");
																				}
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
		"search_for_NI_cards.  DIG_Out_Line failed for chip_select_line_UnEmap1vx");
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_3_UnEmap1vx");
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_2_UnEmap1vx");
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_1_UnEmap1vx");
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for led_line_UnEmap1vx");
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for filter_increment_output_line");
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
		"search_for_NI_cards.  DIG_Line_Config failed for up_down_line_UnEmap1vx");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for chip_select_line_UnEmap1vx");
												}
											} break;
											case UnEmap_2V1:
											case UnEmap_2V2:
											{
												status=DIG_Line_Config(ni_card->device_number,0,
													battery_good_input_line_UnEmap2vx,(i16)0);
												if (0==status)
												{
													status=DIG_Line_Config(ni_card->device_number,0,
														filter_increment_output_line,(i16)1);
													if (0==status)
													{
														status=DIG_Line_Config(ni_card->device_number,0,
															gain_0_output_line_UnEmap2vx,(i16)1);
														if (0==status)
														{
															status=DIG_Line_Config(ni_card->device_number,0,
																gain_1_output_line_UnEmap2vx,(i16)1);
															if (0==status)
															{
																status=DIG_Line_Config(ni_card->device_number,0,
																	shift_register_clock_output_line_UnEmap2vx,
																	(i16)1);
																if (0==status)
																{
																	status=DIG_Line_Config(ni_card->device_number,
																		0,
																		shift_register_data_output_line_UnEmap2vx,
																		(i16)1);
																	if (0==status)
																	{
																		status=DIG_Line_Config(
																			ni_card->device_number,0,
																			shift_register_strobe_output_line_UnEmap2vx,
																			(i16)1);
																		if (0==status)
																		{
																			if (0==module_number_of_NI_CARDS)
																			{
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							battery_A_line_UnEmap2v1,(i16)1);
																						if (0==status)
																						{
																							if (BattA_setting_UnEmap2vx)
																							{
																								DIG_Out_Line(
																									ni_card->device_number,
																									(i16)0,
																									battery_A_line_UnEmap2v1,
																									(i16)1);
																							}
																							else
																							{
																								DIG_Out_Line(
																									ni_card->device_number,
																									(i16)0,
																									battery_A_line_UnEmap2v1,
																									(i16)0);
																							}
																						}
																						/* set master high */
																						set_shift_register(ni_card,
																							Master_SHIFT_REGISTER_UnEmap2v1,1,
																							0);
																					} break;
																					case UnEmap_2V2:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							master_line_UnEmap2v2,(i16)1);
																						if (0==status)
																						{
																							/* set master low (only set high
																								when everything is
																								configured) */
																							DIG_Out_Line(
																								ni_card->device_number,
																								(i16)0,master_line_UnEmap2v2,
																								(i16)0);
																							if (BattA_setting_UnEmap2vx)
																							{
																								set_shift_register(ni_card,
																									BattA_SHIFT_REGISTER_UnEmap2vx,
																									1,0);
																							}
																							else
																							{
																								set_shift_register(ni_card,
																									BattA_SHIFT_REGISTER_UnEmap2vx,
																									0,0);
																							}
																						}
																					} break;
																				}
																			}
																			else
																			{
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							battery_A_line_UnEmap2v1,(i16)0);
																						/* set master low */
																						set_shift_register(ni_card,
																							Master_SHIFT_REGISTER_UnEmap2v1,0,
																							0);
																					} break;
																					case UnEmap_2V2:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							master_line_UnEmap2v2,(i16)0);
																					} break;
																				}
																			}
																			if (0==status)
																			{
																				/* set cs high to disable filter
																					programming */
																				set_shift_register(ni_card,
																					FCS_SHIFT_REGISTER_UnEmap2vx,1,0);
																				/* set the led */
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						if (BattA_setting_UnEmap2vx)
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								1,1);
																						}
																						else
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								0,1);
																						}
																					} break;
																					case UnEmap_2V2:
																					{
																						if (BattA_setting_UnEmap2vx)
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								0,1);
																						}
																						else
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								1,1);
																						}
																					} break;
																				}
																				/* set the gain on the signal
																					conditioning card */
																				if (GA0_setting)
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_0=
																						(unsigned char)0x1;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,
																						gain_0_output_line_UnEmap2vx,
																						(i16)1);
																				}
																				else
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_0=
																						(unsigned char)0x0;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_0_output_line_UnEmap2vx,
																						(i16)0);
																				}
																				if (GA1_setting)
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_1=
																						(unsigned char)0x1;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_1_output_line_UnEmap2vx,
																						(i16)1);
																				}
																				else
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_1=
																						(unsigned char)0x0;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_1_output_line_UnEmap2vx,
																						(i16)0);
																				}
																				/* make sure that the card is in isolate
																					mode */
																				(ni_card->unemap_2vx).isolate_mode=
																					(unsigned char)0x0;
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
	"search_for_NI_cards.  DIG_Line_Config failed for battery_A_line_UnEmap2v1");
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_strobe_output_line_UnEmap2vx");
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_data_output_line_UnEmap2vx");
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_clock_output_line_UnEmap2vx");
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for gain_1_output_line_UnEmap2vx");
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for gain_0_output_line_UnEmap2vx");
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for filter_increment_output_line");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for battery_good_input_line_UnEmap2vx");
												}
											} break;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"search_for_NI_cards.  AI_Configure failed");
									}
									/*???DB.  Moved from unemap_configure end */
									module_number_of_NI_CARDS++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"search_for_NI_cards.  Could not allocate ni_cards");
									module_number_of_NI_CARDS=0;
									DEALLOCATE(module_NI_CARDS);
									return_code=0;
								}
							}
							else
							{
								if (0==PXI6071E_DEVICE_CODE)
								{
									display_message(WARNING_MESSAGE,
	"Cannot mix 12-bit and 16-bit cards.  12-bit card, device number %d, ignored",
										card_number);
								}
								else
								{
									display_message(WARNING_MESSAGE,
	"Cannot mix 12-bit and 16-bit cards.  16-bit card, device number %d, ignored",
										card_number);
								}
							}
						}
						card_number++;
					}
					if (return_code)
					{
						if (UnEmap_2V2==module_NI_CARDS->unemap_hardware_version)
						{
							/* set master high */
							DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
								master_line_UnEmap2v2,(i16)1);
						}
					}
				}
			}
			fclose(digital_io_lines);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open digital.txt\n");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* search_for_NI_cards */
#endif /* defined (NI_DAQ) */

#if defined (WINDOWS)
#if defined (NI_DAQ)
static void scrolling_callback_NI(HWND handle,UINT message,WPARAM wParam,
	LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 10 September 1999

DESCRIPTION :
Always called so that <module_starting_sample_number> and
<module_sample_buffer_size> can be kept up to date.
==============================================================================*/
{
	int averaging_length,channel_number,*channel_numbers,*channel_numbers_2,i,j,k,
		number_of_bytes;
	i16 *hardware_buffer;
	long sum;
	SHORT *value,*value_array,*value_array_2;
	static int first_error=1;
	struct NI_card *ni_card;
	unsigned char *byte_array;
	unsigned long offset,number_of_samples,sample_number;

	ENTER(scrolling_callback_NI);
	/* keep <module_starting_sample_number> and <module_sample_buffer_size> up to
		date */
	sample_number=(unsigned long)lParam;
	number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
		NUMBER_OF_CHANNELS_ON_NI_CARD;
	/* NIDAQ returns the number of the next sample */
	if (module_sample_buffer_size<number_of_samples)
	{
		if (sample_number<module_starting_sample_number+module_sample_buffer_size)
		{
			module_sample_buffer_size += (sample_number+number_of_samples)-
				(module_starting_sample_number+module_sample_buffer_size);
		}
		else
		{
			module_sample_buffer_size += sample_number-
				(module_starting_sample_number+module_sample_buffer_size);
		}
		if (module_sample_buffer_size>=number_of_samples)
		{
			module_sample_buffer_size=number_of_samples;
			module_starting_sample_number=sample_number%number_of_samples;
			if (module_buffer_full_callback)
			{
				(*module_buffer_full_callback)(module_buffer_full_callback_data);
			}
		}
	}
	else
	{
		module_starting_sample_number=sample_number%number_of_samples;
	}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	static int count=0;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"scrolling_callback_NI %d %ld %d %lu\n",message,
			sample_number,count,module_sample_buffer_size);
		count++;
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	sample_number += number_of_samples-1;
	sample_number %= number_of_samples;
	if (module_scrolling_on&&module_sampling_on&&
		(0<module_number_of_scrolling_channels)&&
		(module_scrolling_window||module_scrolling_callback))
	{
		if (module_scrolling_window)
		{
			number_of_bytes=(module_number_of_scrolling_channels+2)*sizeof(int)+
				module_number_of_scrolling_channels*
				NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL*sizeof(SHORT)+
				sizeof(module_scrolling_callback_data);
			if (ALLOCATE(byte_array,unsigned char,number_of_bytes))
			{
				*((int *)byte_array)=module_number_of_scrolling_channels;
				channel_numbers=(int *)(byte_array+sizeof(int));
				*((int *)(byte_array+(module_number_of_scrolling_channels+1)*
					sizeof(int)))=NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL;
				value_array=(SHORT *)(byte_array+
					(module_number_of_scrolling_channels+2)*sizeof(int));
				*((void **)(byte_array+((module_number_of_scrolling_channels+2)*
					sizeof(int)+module_number_of_scrolling_channels*
					NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL*sizeof(SHORT))))=
					module_scrolling_callback_data;
			}
			else
			{
				if (first_error)
				{
					display_message(ERROR_MESSAGE,
						"scrolling_callback_NI.  Could not allocate byte_array");
					first_error=0;
				}
			}
		}
		else
		{
			ALLOCATE(channel_numbers,int,module_number_of_scrolling_channels);
			ALLOCATE(value_array,SHORT,module_number_of_scrolling_channels*
				NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL);
			if (!(channel_numbers&&value_array))
			{
				DEALLOCATE(channel_numbers);
				DEALLOCATE(value_array);
				if (first_error)
				{
					display_message(ERROR_MESSAGE,
			"scrolling_callback_NI.  Could not allocate channel_numbers/value_array");
					first_error=0;
				}
			}
		}
		/* calculate the values */
		if (value=value_array)
		{
			for (k=0;k<module_number_of_scrolling_channels;k++)
			{
				channel_number=module_scrolling_channel_numbers[k];
				channel_numbers[k]=channel_number;
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				hardware_buffer=ni_card->hardware_buffer;
				offset=NUMBER_OF_CHANNELS_ON_NI_CARD*sample_number+
					(ni_card->channel_reorder)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD];
				averaging_length=module_scrolling_refresh_period/
					NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL;
				for (j=NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL-1;j>=0;j--)
				{
					sum=0;
					for (i=averaging_length;i>0;i--)
					{
						sum += (long)(hardware_buffer[offset]);
						if (offset<NUMBER_OF_CHANNELS_ON_NI_CARD)
						{
							offset += NUMBER_OF_CHANNELS_ON_NI_CARD*(number_of_samples-1);
						}
						else
						{
							offset -= NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
					value[j]=(SHORT)(sum/averaging_length);
				}
				value += NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL;
			}
			if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
				(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
			{
				/* Unemap_2V1 and UnEmap_2V2 invert */
				value=value_array;
				for (k=module_number_of_scrolling_channels*
					NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL;k>0;k--)
				{
					*value= -(*value);
					value++;
				}
			}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		for (i=0;i<module_number_of_scrolling_channels;i++)
		{
			fprintf(unemap_debug,"  %d ",module_scrolling_channel_numbers[i]);
			for (j=0;j<NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL;j++)
			{
				fprintf(unemap_debug," %d",
					value_array[i*NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL+j]);
			}
			fprintf(unemap_debug,"\n");
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
			if (module_scrolling_window)
			{
				if (module_scrolling_callback)
				{
					ALLOCATE(channel_numbers_2,int,module_number_of_scrolling_channels);
					ALLOCATE(value_array_2,SHORT,module_number_of_scrolling_channels*
						NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL);
					if (channel_numbers_2&&value_array_2)
					{
						memcpy((char *)channel_numbers_2,(char *)channel_numbers,
							module_number_of_scrolling_channels*sizeof(int));
						memcpy((char *)value_array_2,(char *)value_array,
							module_number_of_scrolling_channels*
							NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL*sizeof(SHORT));
						(*module_scrolling_callback)(module_number_of_scrolling_channels,
							channel_numbers_2,(int)NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL,
							value_array_2,module_scrolling_callback_data);
					}
					else
					{
						DEALLOCATE(channel_numbers_2);
						DEALLOCATE(value_array_2);
						if (first_error)
						{
							display_message(ERROR_MESSAGE,
	"scrolling_callback_NI.  Could not allocate channel_numbers_2/value_array_2");
							first_error=0;
						}
					}
				}
				PostMessage(module_scrolling_window,module_scrolling_message,
					(WPARAM)byte_array,(ULONG)number_of_bytes);
			}
			else
			{
				(*module_scrolling_callback)(module_number_of_scrolling_channels,
					channel_numbers,(int)NUMBER_OF_SCROLLING_VALUES_PER_CHANNEL,
					value_array,module_scrolling_callback_data);
			}
		}
	}
	LEAVE;
} /* scrolling_callback_NI */
#endif /* defined (NI_DAQ) */
#endif /* defined (WINDOWS) */

static int unemap_set_isolate_record_mode(int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i,j;
	i16 status;
	struct NI_card *ni_card;
	unsigned char stimulate_off;
	u32 counter,counter_save,counter_time;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_isolate_record_mode);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD))
		{
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			return_code=1;
			if (isolate)
			{
				ni_card=module_NI_CARDS;
				for (j=0;j<module_number_of_NI_CARDS;j++)
				{
					if ((-1==card_number)||(j==card_number))
					{
						if (((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))&&
							!((ni_card->unemap_2vx).isolate_mode))
						{
							/* turn off stimulation channels */
							if (relay_power_on_UnEmap2vx)
							{
								stimulate_off=(unsigned char)0x0;
							}
							else
							{
								stimulate_off=(unsigned char)0xff;
							}
							for (i=0;i<8;i++)
							{
								((ni_card->unemap_2vx).stimulate_channels)[i]=
									((ni_card->unemap_2vx).shift_register)[i+2];
								((ni_card->unemap_2vx).shift_register)[i+2]=stimulate_off;
							}
							/* isolate external circuits */
								/*???DB.  In terms of relay_power_on_UnEmap2vx ? */
							set_shift_register(ni_card,EXT_ISO_SHIFT_REGISTER_UnEmap2vx,0,0);
							/* set calibrate mode */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,0);
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							/* use timer to make sure that don't go too quickly */
							/* set D_CAL high for 5 ms */
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								relay_power_on_UnEmap2vx,1);
							status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
								ND_COUNT,&counter_save);
							counter_time=0;
							do
							{
								status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
									ND_COUNT,&counter);
								if (counter<counter_save)
								{
									counter_save=0;
								}
								counter_time += counter-counter_save;
								counter_save=counter;
							} while ((0==status)&&(counter_time<(u32)100000));
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x1;
						}
					}
					ni_card++;
				}
			}
			else
			{
				ni_card=module_NI_CARDS;
				for (j=0;j<module_number_of_NI_CARDS;j++)
				{
					if ((-1==card_number)||(j==card_number))
					{
						if (((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))&&
							((ni_card->unemap_2vx).isolate_mode))
						{
							/* turn on stimulation channels */
							for (i=0;i<8;i++)
							{
								((ni_card->unemap_2vx).shift_register)[i+2]=
									((ni_card->unemap_2vx).stimulate_channels)[i];
							}
							/* connect external circuits */
								/*???DB.  In terms of relay_power_on_UnEmap2vx ? */
							set_shift_register(ni_card,EXT_ISO_SHIFT_REGISTER_UnEmap2vx,1,0);
							/* set record mode */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,0);
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							/* use timer to make sure that don't go too quickly */
							/* set D_REC high for 5 ms */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								relay_power_on_UnEmap2vx,1);
							status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
								ND_COUNT,&counter_save);
							counter_time=0;
							do
							{
								status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
									ND_COUNT,&counter);
								if (counter<counter_save)
								{
									counter_save=0;
								}
								counter_time += counter-counter_save;
								counter_save=counter;
							} while ((0==status)&&(counter_time<(u32)100000));
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x0;
						}
					}
					ni_card++;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_isolate_record_mode.  Invalid channel_number.  %d",
				channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_isolate_record_mode */

static int unemap_deconfigure(void)
/*******************************************************************************
LAST MODIFIED : 19 July 1999

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i;
	i16 status;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_deconfigure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_deconfigure %lu\n",
		module_sample_buffer_size);
#endif /* defined (DEBUG) */
	return_code=1;
#if defined (NI_DAQ)
	if (module_configured)
	{
		if ((0<module_number_of_NI_CARDS)&&module_NI_CARDS)
		{
			/* stop continuous sampling */
			status=GPCTR_Control((module_NI_CARDS[0]).device_number,SCAN_COUNTER,
				ND_RESET);
			/* make sure that in calibrate mode */
			unemap_set_isolate_record_mode(0,1);
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				status=DAQ_Clear((module_NI_CARDS[i]).device_number);
				GlobalUnlock((module_NI_CARDS[i]).memory_object);
				GlobalFree((module_NI_CARDS[i]).memory_object);
				/* turn off double buffering */
				status=DAQ_DB_Config((module_NI_CARDS[i]).device_number,
					/* disable */(i16)0);
				if ((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type))
				{
					/* stop waveform generation */
					WFM_Group_Control((module_NI_CARDS[i]).device_number,/*group*/1,
						/*Clear*/0);
					/* set the output voltage to 0 */
					AO_Write((module_NI_CARDS[i]).device_number,CALIBRATE_CHANNEL,
						(i16)0);
					AO_Write((module_NI_CARDS[i]).device_number,STIMULATE_CHANNEL,
						(i16)0);
				}
			}
			module_sample_buffer_size=0;
			module_starting_sample_number=0;
			module_scrolling_refresh_period=0;
#if defined (WINDOWS)
			module_scrolling_window=(HWND)NULL;
			module_scrolling_message=(UINT)0;
#endif /* defined (WINDOWS) */
			module_scrolling_callback=(Unemap_hardware_callback *)NULL;
			module_scrolling_callback_data=(void *)NULL;
			module_number_of_scrolling_channels=0;
			DEALLOCATE(module_scrolling_channel_numbers);
			DEALLOCATE(module_calibration_channels);
			DEALLOCATE(module_calibration_offsets);
			DEALLOCATE(module_calibration_gains);
		}
		module_configured=0;
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_deconfigure %d %lu\n",
		return_code,module_sample_buffer_size);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_deconfigure */

static int unemap_get_antialiasing_filter_frequency(int channel_number,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 13 September 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int tap;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((1<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD)&&frequency)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			tap=ni_card->anti_aliasing_filter_taps;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
#if defined (OLD_CODE)
/*???DB.  Changed to new clock circuit */
				{
					*frequency=(float)1.07e6/((float)(99-tap)*(float)101);
					return_code=1;
				} break;
#endif /* defined (OLD_CODE) */
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					*frequency=(float)tap*(float)50.0;
					return_code=1;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_get_antialiasing_filter_frequency.  Invalid argument(s).  %d %p",
				channel_number,frequency);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"unemap_get_antialiasing_filter_frequency.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_antialiasing_filter_frequency */

int unemap_set_antialiasing_filter_frequency(int channel_number,
	float frequency)
/*******************************************************************************
LAST MODIFIED : 18 October 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the anti-aliasing filter to the specified <frequency> for all channels.
UnEmap_1V2
3dB cutoff frequency = 1.07/(100*R*C)
C = 10 nF
R = (99-tap) * 101 ohm
UnEmap_2V1 or UnEmap_2V2
3dB cutoff frequency = 50*tap
<frequency> is a request and the actual value used can be found with
unemap_get_antialiasing_filter.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i,j,tap;
	i16 status;
	u32 counter,counter_start;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	/* check configuration */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if ((-1==card_number)||(i==card_number))
				{
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
#if defined (OLD_CODE)
/*???DB.  Changed to new clock circuit */
						{
							tap=99-(int)floor((float)1.07e6/(frequency*(float)101)+0.5);
						} break;
#endif /* defined (OLD_CODE) */
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							tap=(int)floor(frequency/(float)50.0+0.5);
						} break;
					}
					if (tap<1)
					{
						tap=1;
					}
					else
					{
						if (tap>99)
						{
							tap=99;
						}
					}
					if (tap!=(module_NI_CARDS[i]).anti_aliasing_filter_taps)
					{
						(module_NI_CARDS[i]).anti_aliasing_filter_taps=tap;
						/* set chip select low to program */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									chip_select_line_UnEmap1vx,(i16)0);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FCS_SHIFT_REGISTER_UnEmap2vx,0,1);
							} break;
						}
						/* use timer to make sure that don't go too quickly */
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						/* tap to the bottom */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									up_down_line_UnEmap1vx,(i16)0);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FUD_SHIFT_REGISTER_UnEmap2vx,0,1);
							} break;
						}
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
							filter_increment_output_line,(i16)0);
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)40));
						for (j=100;j>0;j--)
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)1);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)0);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
						}
						/* tap to the required value */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									up_down_line_UnEmap1vx,(i16)1);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FUD_SHIFT_REGISTER_UnEmap2vx,1,1);
							} break;
						}
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						for (j=tap;j>0;j--)
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)1);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)0);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
						}
						/* set chip select high to stop programming */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									chip_select_line_UnEmap1vx,(i16)1);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FCS_SHIFT_REGISTER_UnEmap2vx,1,1);
							} break;
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_set_antialiasing_filter_frequency.  Invalid channel_number.  %d",
				channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"unemap_set_antialiasing_filter_frequency.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_antialiasing_filter_frequency */

static int unemap_set_power(int on)
/*******************************************************************************
LAST MODIFIED : 26 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float frequency;
	int channel_number,i;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_power);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
			(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
		{
			return_code=1;
			if (on)
			{
				if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
				{
					DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
						battery_A_line_UnEmap2v1,(i16)1);
				}
				else
				{
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,1,
						1);
				}
				channel_number=1;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					/* set power light on (and set other shift registers) */
					if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,1,1);
					}
					else
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,0,1);
					}
					if (module_configured)
					{
						/* force a reset of the filter frequency */
						unemap_get_antialiasing_filter_frequency(channel_number,&frequency);
						module_NI_CARDS[i].anti_aliasing_filter_taps= -1;
						unemap_set_antialiasing_filter_frequency(channel_number,frequency);
					}
					channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
				}
				BattA_setting_UnEmap2vx=1;
			}
			else
			{
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					/* set power light off */
					if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,0,1);
					}
					else
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,1,1);
					}
				}
				if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
				{
					DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
						battery_A_line_UnEmap2v1,(i16)0);
				}
				else
				{
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,0,
						1);
				}
				BattA_setting_UnEmap2vx=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Invalid configuration.  %d %p %d",module_configured,
			module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_power */

static int unemap_get_maximum_number_of_samples(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer, in number of samples per channel, is assigned to
<*number_of_samples>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_maximum_number_of_samples);
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
				NUMBER_OF_CHANNELS_ON_NI_CARD;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Missing number_of_samples");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_maximum_number_of_samples */

static int unemap_get_sampling_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The sampling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_sampling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*frequency=module_sampling_frequency;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sampling_frequency.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sampling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sampling_frequency */

static int unemap_get_number_of_channels(int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_channels);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_channels)
	{
		return_code=1;
		if (search_for_NI_cards()&&module_NI_CARDS)
		{
			*number_of_channels=(int)(module_number_of_NI_CARDS*
				NUMBER_OF_CHANNELS_ON_NI_CARD);
		}
		else
		{
			*number_of_channels=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_channels.  Missing number_of_channels");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_channels */

int unemap_stop_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_scrolling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		return_code=1;
		module_scrolling_on=0;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_stop_scrolling */

static int unemap_configure(float sampling_frequency_slave,
	int number_of_samples_in_buffer,
#if defined (WINDOWS)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtAppContext application_context,
#endif /* defined (MOTIF) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_refresh_frequency)
/*******************************************************************************
LAST MODIFIED : 3 April 2000

DESCRIPTION :
Configures the hardware for sampling at the specified
<sampling_frequency_slave> and with the specified <number_of_samples_in_buffer>.
<sampling_frequency_slave> and <number_of_samples_in_buffer> are requests and
the actual values used should be determined using unemap_get_sampling_frequency
and unemap_get_maximum_number_of_samples.

Every <sampling_frequency_slave>/<scrolling_refresh_frequency> samples (one
	sample is a measument on every channel)
- if <scrolling_callback> is not NULL, <scrolling_callback> is called with
	- first argument is the number of scrolling channels
	- second argument is an array of channel numbers (in the order they were added
		using unemap_set_scrolling_channel)
	- third argument is the number of values for each channel
	- fourth argument is an array of channel values (all the values for the first
		channel, then all the values for the second channel and so on)
	- fifth argument is the user supplied callback data
	- it is the responsibilty of the callback to free the channel numbers and
		values arrays
- for WINDOWS, if <scrolling_window> is not NULL, a <scrolling_message> for
	<scrolling_window> is added to the message queue with
	- WPARAM is an array of unsigned char amalgamating the five arguments above
		- first sizeof(int) bytes is the number of scrolling channels
		- next (number of scrolling channels)*sizeof(int) bytes are the scrolling
			channel numbers
		- next sizeof(int) bytes is the number of values for each channel
		- next (number of scrolling channels)*(number of values)*sizeof(short) bytes
			are the channel values
		- last sizeof(void *) bytes are the user supplied callback data
	- LPARAM is a ULONG specifying the number of bytes ing the WPARAM array
		over the <scrolling_refresh_period>) in the array
	- it is the responsibility of the window message handler to free the WPARAM
		array.

Initially there are no scrolling channels.

For this local hardware version, the sign of <sampling_frequency_slave>
determines whether the hardware is configured as slave (<0) or master (>0)
==============================================================================*/
{
	float sampling_frequency;
	int return_code;
#if defined (NI_DAQ)
	int i,j;
	i16 channel_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],
		gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],status,
		time_base;
	u16 sampling_interval;
	u32 hardware_buffer_size;
#endif /* defined (NI_DAQ) */
#if defined (WINDOWS)
	/* for getting the total physical memory */
	MEMORYSTATUS memory_status;
#endif /* defined (WINDOWS) */

	ENTER(unemap_configure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_configure %lu\n",
		module_sample_buffer_size);
#endif /* defined (DEBUG) */
	return_code=0;
	if (sampling_frequency_slave<0)
	{
		sampling_frequency= -sampling_frequency_slave;
		module_slave=1;
	}
	else
	{
		sampling_frequency=sampling_frequency_slave;
		module_slave=0;
	}
	/* check arguments */
	if ((0<sampling_frequency)&&(0<number_of_samples_in_buffer))
	{
#if defined (NI_DAQ)
		if (0==module_configured)
		{
			if (search_for_NI_cards()&&module_NI_CARDS)
			{
				/* drive the A/D conversions as fast as possible */
				/* use the 20 MHz (50 ns) clock */
				time_base= -3;
				sampling_interval=module_NI_CARDS->sampling_interval;
				if (sampling_interval<=0)
				{
					if (PXI6071E_AD_DA==module_NI_CARDS->type)
					{
						/* 1.25 MS/s */
						/* do a A/D conversion every 800 nano-seconds = 16 * 50 ns */
						sampling_interval=16;
						/* if scanning can't go at 1.25 MS/s, limit to 0.5 MS/s */
						sampling_interval=40;
					}
					else
					{
						/* 100 kS/s */
						/* do a A/D conversion every 10 micro-seconds = 200 * 50 ns */
						sampling_interval=200;
						/*???DB.  What is safe when scanning ? */
					}
				}
				module_sampling_frequency=(float)1;
				for (i=(int)ceil(log10((double)NUMBER_OF_CHANNELS_ON_NI_CARD));i>0;i--)
				{
					module_sampling_frequency *= (float)10;
				}
				module_sampling_frequency=(float)1e9/((float)sampling_interval*
					(float)50*module_sampling_frequency);
				if ((0<sampling_frequency)&&
					(sampling_frequency<module_sampling_frequency))
				{
					module_sampling_frequency=sampling_frequency;
				}
				module_sampling_high_count=2;
				module_sampling_low_count=
					(u32)((double)20000000/(double)module_sampling_frequency);
				module_sampling_frequency=
					(float)20000000./(float)module_sampling_low_count;
				module_sampling_low_count -= module_sampling_high_count;
				if ((module_sampling_low_count>0)&&(module_sampling_high_count>0))
				{
					if ((0<scrolling_refresh_frequency)&&(scrolling_callback
#if defined (WINDOWS)
						||scrolling_window
#endif /* defined (WINDOWS) */
						))
					{
						module_scrolling_refresh_period=(int)(module_sampling_frequency/
							scrolling_refresh_frequency);
						/* have scrolling period divisible by 4 */
						module_scrolling_refresh_period=
							(module_scrolling_refresh_period-1)/4;
						module_scrolling_refresh_period=
							(module_scrolling_refresh_period+1)*4;
#if defined (WINDOWS)
						module_scrolling_window=scrolling_window;
						module_scrolling_message=scrolling_message;
#endif /* defined (WINDOWS) */
						module_scrolling_callback=scrolling_callback;
						module_scrolling_callback_data=scrolling_callback_data;
					}
					else
					{
						/* have a callback to keep <module_starting_sample_number> and
							<module_sample_buffer_size> up to date */
						module_scrolling_refresh_period=number_of_samples_in_buffer/2;
#if defined (WINDOWS)
						module_scrolling_window=(HWND)NULL;
						module_scrolling_message=(UINT)0;
#endif /* defined (WINDOWS) */
						module_scrolling_callback=(Unemap_hardware_callback *)NULL;
						module_scrolling_callback_data=(void *)NULL;
					}
					module_scrolling_channel_numbers=(int *)NULL;
					module_number_of_scrolling_channels=0;
					status=Config_DAQ_Event_Message(module_NI_CARDS[0].device_number,
						/* add message */(i16)1,/* channel string */"AI0",
						/* send message every N scans */(i16)1,
						(i32)module_scrolling_refresh_period,(i32)0,(u32)0,(u32)0,(u32)0,
						(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"module_sampling_frequency=%g, sampling_interval=%u\n",
						module_sampling_frequency,sampling_interval);
#endif /* defined (DEBUG) */
					if (0!=status)
					{
						display_message(ERROR_MESSAGE,
							"unemap_configure.  Config_DAQ_Event_Message 1 failed.  %d %d",
							status,module_scrolling_refresh_period);
					}
#if defined (WINDOWS)
					/* have number_of_samples_in_buffer divisible by
						module_scrolling_refresh_period */
					hardware_buffer_size=(number_of_samples_in_buffer-1)/
						module_scrolling_refresh_period;
					hardware_buffer_size=(hardware_buffer_size+1)*
						module_scrolling_refresh_period;
					hardware_buffer_size *= (u32)NUMBER_OF_CHANNELS_ON_NI_CARD;
					/* limit the total locked memory to a third of physical memory */
					memory_status.dwLength=sizeof(MEMORYSTATUS);
					GlobalMemoryStatus(&memory_status);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"total physical memory=%d, available physical memory=%d\n",
						memory_status.dwTotalPhys,memory_status.dwAvailPhys);
#endif /* defined (DEBUG) */
					if ((DWORD)(3*module_number_of_NI_CARDS*hardware_buffer_size)*
						sizeof(i16)>memory_status.dwTotalPhys)
					{
						hardware_buffer_size=(u32)(memory_status.dwTotalPhys/
							(3*module_number_of_NI_CARDS*sizeof(i16)));
						hardware_buffer_size=(hardware_buffer_size-1)/
							(module_scrolling_refresh_period*NUMBER_OF_CHANNELS_ON_NI_CARD);
						hardware_buffer_size=(hardware_buffer_size+1)*
							(module_scrolling_refresh_period*NUMBER_OF_CHANNELS_ON_NI_CARD);
					}
					for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
					{
						channel_vector[i]=i;
					}
#endif /* defined (WINDOWS) */
					/* configure cards */
					i=0;
					status=0;
					while ((0==status)&&(i<module_number_of_NI_CARDS))
					{
						(module_NI_CARDS[i]).time_base=time_base;
						(module_NI_CARDS[i]).sampling_interval=sampling_interval;
						(module_NI_CARDS[i]).hardware_buffer_size=hardware_buffer_size;
						if (0==status)
						{
							/* allocate buffer */
							if ((module_NI_CARDS[i]).memory_object=GlobalAlloc(GMEM_MOVEABLE,
								(DWORD)(((module_NI_CARDS[i]).hardware_buffer_size)*
								sizeof(i16))))
							{
								if ((module_NI_CARDS[i]).hardware_buffer=
									(i16 *)GlobalLock((module_NI_CARDS[i]).memory_object))
								{
									/* working from "Building Blocks" section, p.3-25 (pdf 65) in
										the "NI-DAQ User Manual" */
									/* configuration block.  Start with default settings except
										for changing to double buffering mode (DAQ_DB_Config).
										Could also use AI_Configure, AI_Max_Config, DAQ_Config,
										DAQ_StopTrigger_Config */
									status=DAQ_DB_Config((module_NI_CARDS[i]).device_number,
										/* enable */(i16)1);
									if (0==status)
									{
										/* set the scan sequence and gain */
										for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
										{
											gain_vector[j]=(module_NI_CARDS[i]).gain;
										}
										status=SCAN_Setup((module_NI_CARDS[i]).device_number,
											NUMBER_OF_CHANNELS_ON_NI_CARD,channel_vector,gain_vector);
										if (0==status)
										{
											/* first card is the "master */
											if (0==i)
											{
												/* set the clock source on the RTSI bus */
												status=Select_Signal((module_NI_CARDS[i]).device_number,
													ND_RTSI_CLOCK,ND_BOARD_CLOCK,ND_DONT_CARE);
												if (0!=status)
												{
													display_message(ERROR_MESSAGE,
"unemap_configure.  Select_Signal(0,ND_RTSI_CLOCK,ND_BOARD_CLOCK,ND_DONT_CARE) failed");
												}
											}
											else
											{
												/* use the clock source on the RTSI bus */
												status=Select_Signal((module_NI_CARDS[i]).device_number,
													ND_BOARD_CLOCK,ND_RTSI_CLOCK,ND_DONT_CARE);
												if (0!=status)
												{
													display_message(ERROR_MESSAGE,
"unemap_configure.  Select_Signal(>0,ND_BOARD_CLOCK,ND_RTSI_CLOCK,ND_DONT_CARE) failed");
												}
											}
											if (0==status)
											{
												/* set to output the A/D conversion signal (for use
													on other crates */
												if (!module_slave)
												{
													status=Select_Signal(
														(module_NI_CARDS[i]).device_number,ND_PFI_7,
														ND_IN_SCAN_IN_PROG,ND_LOW_TO_HIGH);
												}
												if (0==status)
												{
													/* configure RTSI */
													/* set to input the A/D conversion signal */
													if (module_slave&&(0==i))
													{
														status=Select_Signal(
															(module_NI_CARDS[i]).device_number,
															ND_IN_SCAN_START,ND_PFI_7,ND_LOW_TO_HIGH);
													}
													else
													{
														status=Select_Signal(
															(module_NI_CARDS[i]).device_number,
															ND_IN_SCAN_START,ND_RTSI_0,ND_LOW_TO_HIGH);
													}
													if (0==status)
													{
														/* acquisition won't actually start (controlled by
															conversion signal) */
														status=SCAN_Start(
															(module_NI_CARDS[i]).device_number,
															(i16 *)((module_NI_CARDS[i]).hardware_buffer),
															(u32)((module_NI_CARDS[i]).hardware_buffer_size),
															(module_NI_CARDS[i]).time_base,
															(module_NI_CARDS[i]).sampling_interval,(i16)0,
															(u16)0);
														if (0!=status)
														{
															display_message(ERROR_MESSAGE,
																"unemap_configure.  SCAN_Start failed.  %d",
																status);
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
"unemap_configure.  Select_Signal(*,ND_IN_SCAN_START,ND_PFI_7/ND_RTSI_0,ND_LOW_TO_HIGH) failed");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"unemap_configure.  Select_Signal(*,ND_PFI_7,ND_IN_SCAN_START,ND_LOW_TO_HIGH) failed");
												}
											}
											if (0==status)
											{
												i++;
											}
											else
											{
												GlobalUnlock((module_NI_CARDS[i]).memory_object);
												GlobalFree((module_NI_CARDS[i]).memory_object);
												DAQ_DB_Config((module_NI_CARDS[i]).device_number,
													/* disable */(i16)0);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"unemap_configure.  SCAN_Setup failed.  %d.  gain=%d",
												status,gain_vector[0]);
											GlobalUnlock((module_NI_CARDS[i]).memory_object);
											GlobalFree((module_NI_CARDS[i]).memory_object);
											DAQ_DB_Config((module_NI_CARDS[i]).device_number,
												/* disable */(i16)0);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"unemap_configure.  DAQ_DB_Config failed");
										GlobalUnlock((module_NI_CARDS[i]).memory_object);
										GlobalFree((module_NI_CARDS[i]).memory_object);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"unemap_configure.  GlobalLock failed");
									GlobalFree((module_NI_CARDS[i]).memory_object);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"unemap_configure.  GlobalAlloc failed");
							}
						}
					}
					module_sample_buffer_size=0;
					module_starting_sample_number=0;
					if (0==status)
					{
						return_code=1;
						/*???DB.  Needed, otherwise WFM_Op (waveform generation) fails */
						/*???DB.  Don't understand why.  See set_NI_gain */
						status=Config_DAQ_Event_Message(module_NI_CARDS[0].device_number,
							/* clear all messages */(i16)0,/* channel string */(char *)NULL,
							/* send message every N scans */(i16)0,
							(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,
							(HWND)NULL,(i16)0,(u32)0);
						module_configured=1;
						unemap_set_isolate_record_mode(0,1);
						unemap_set_antialiasing_filter_frequency(0,
							initial_antialiasing_filter_frequency);
					}
					else
					{
						while (i>0)
						{
							i--;
							DAQ_Clear((module_NI_CARDS[i]).device_number);
							GlobalUnlock((module_NI_CARDS[i]).memory_object);
							GlobalFree((module_NI_CARDS[i]).memory_object);
							DAQ_DB_Config((module_NI_CARDS[i]).device_number,
							/* disable */(i16)0);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_configure.  Invalid sampling frequency.  %d %d",
						module_sampling_low_count,module_sampling_high_count);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_configure.  Missing module_NI_CARDS");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_configure.  Hardware is already configured");
			return_code=0;
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_configure.  Invalid argument(s).  %g %d",sampling_frequency,
			number_of_samples_in_buffer);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_configure %d %lu\n",
		return_code,module_sample_buffer_size);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_configure */

int unemap_get_number_of_samples_acquired(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The number of samples acquired per channel since <unemap_start_sampling> is
assigned to <*number_of_samples>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_samples_acquired);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_samples)
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*number_of_samples=module_sample_buffer_size;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"unemap_get_number_of_samples_acquired.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_samples_acquired.  Missing number_of_samples");
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_get_number_of_samples_acquired %d %lu\n",return_code,
		*number_of_samples);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_samples_acquired */

int unemap_get_samples_acquired(int channel_number,short int *samples)
/*******************************************************************************
LAST MODIFIED : 2 April 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.

???DB.  When trying to read back 64 channels sampled at 1kHz for 20s, this
function hung.  The hardware buffer would be 2.56MB for each card.  So for all
7 cards the hardware buffers would total 17.92MB.  <samples> would also be
17.92MB.  The NI crate had 32MB.  So I think that paging was slowing it down
(drive light was on continuously).  Should have a function that checks on the
available physical memory and limits the buffer based on this.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int end,end2,i,j,k,start,step;
	short int *sample,*source;
	struct NI_card *ni_card;
	unsigned long maximum_number_of_samples;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
#if defined (NI_DAQ)
	/* check arguments */
	if ((sample=samples)&&(0<=channel_number)&&
		(channel_number<=module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			maximum_number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
				NUMBER_OF_CHANNELS_ON_NI_CARD;
			if (0==channel_number)
			{
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
					end2=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
					end2=0;
				}
				step=module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD;
				ni_card=module_NI_CARDS;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
						(UnEmap_2V2==ni_card->unemap_hardware_version))
					{
						/* UnEmap_2V1 and UnEmap_2V2 invert */
						for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
						{
							source=(ni_card->hardware_buffer)+
								(start*NUMBER_OF_CHANNELS_ON_NI_CARD+
								(ni_card->channel_reorder)[j]);
							sample=samples+(i*NUMBER_OF_CHANNELS_ON_NI_CARD+j);
							for (k=start;k<end;k++)
							{
								if (*source<SHRT_MAX)
								{
									*sample= -(*source);
								}
								else
								{
									*sample=SHRT_MIN;
								}
								sample += step;
								source += NUMBER_OF_CHANNELS_ON_NI_CARD;
							}
							if (end2>0)
							{
								source=(ni_card->hardware_buffer)+
									((ni_card->channel_reorder)[j]);
								for (k=0;k<end2;k++)
								{
									if (*source<SHRT_MAX)
									{
										*sample= -(*source);
									}
									else
									{
										*sample=SHRT_MIN;
									}
									sample += step;
									source += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
							}
						}
					}
					else
					{
						for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
						{
							source=(ni_card->hardware_buffer)+
								(start*NUMBER_OF_CHANNELS_ON_NI_CARD+
								(ni_card->channel_reorder)[j]);
							sample=samples+(i*NUMBER_OF_CHANNELS_ON_NI_CARD+j);
							for (k=start;k<end;k++)
							{
								*sample= *source;
								sample += step;
								source += NUMBER_OF_CHANNELS_ON_NI_CARD;
							}
							if (end2>0)
							{
								source=(ni_card->hardware_buffer)+
									((ni_card->channel_reorder)[j]);
								for (k=0;k<end2;k++)
								{
									*sample= *source;
									sample += step;
									source += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
							}
						}
					}
					ni_card++;
				}
#if defined (OLD_CODE)
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
				}
				for (i=start;i<end;i++)
				{
					for (j=0;j<module_number_of_NI_CARDS;j++)
					{
						source=module_NI_CARDS[j].hardware_buffer+
							(i*NUMBER_OF_CHANNELS_ON_NI_CARD);
						if ((UnEmap_2V1==module_NI_CARDS[j].unemap_hardware_version)||
							(UnEmap_2V2==module_NI_CARDS[j].unemap_hardware_version))
						{
							/* UnEmap_2V1 and UnEmap_2V2 invert */
							for (k=NUMBER_OF_CHANNELS_ON_NI_CARD;k>0;k--)
							{
								if (*source<SHRT_MAX)
								{
									*sample= -(*source);
								}
								else
								{
									*sample=SHRT_MIN;
								}
								sample++;
								source++;
							}
						}
						else
						{
							for (k=NUMBER_OF_CHANNELS_ON_NI_CARD;k>0;k--)
							{
								*sample= *source;
								sample++;
								source++;
							}
						}
					}
				}
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					start=0;
					end=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
					for (i=start;i<end;i++)
					{
						for (j=0;j<module_number_of_NI_CARDS;j++)
						{
							source=module_NI_CARDS[j].hardware_buffer+
								(i*NUMBER_OF_CHANNELS_ON_NI_CARD);
							if ((UnEmap_2V1==module_NI_CARDS[j].unemap_hardware_version)||
								(UnEmap_2V2==module_NI_CARDS[j].unemap_hardware_version))
							{
								/* UnEmap_2V1 and UnEmap_2V2 invert */
								for (k=NUMBER_OF_CHANNELS_ON_NI_CARD;k>0;k--)
								{
									if (*source<SHRT_MAX)
									{
										*sample= -(*source);
									}
									else
									{
										*sample=SHRT_MIN;
									}
									sample++;
									source++;
								}
							}
							else
							{
								for (k=NUMBER_OF_CHANNELS_ON_NI_CARD;k>0;k--)
								{
									*sample= *source;
									sample++;
									source++;
								}
							}
						}
					}
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
					end2=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
					end2=0;
				}
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				source=(ni_card->hardware_buffer)+(start*NUMBER_OF_CHANNELS_ON_NI_CARD+
					(ni_card->channel_reorder)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD]);
				if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
					(UnEmap_2V2==ni_card->unemap_hardware_version))
				{
					/* UnEmap_2V1 and UnEmap_2V2 invert */
					for (k=start;k<end;k++)
					{
						if (*source<SHRT_MAX)
						{
							*sample= -(*source);
						}
						else
						{
							*sample=SHRT_MIN;
						}
						sample++;
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
					if (end2>0)
					{
						source=(ni_card->hardware_buffer)+((ni_card->channel_reorder)[
							(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]);
						for (k=0;k<end2;k++)
						{
							if (*source<SHRT_MAX)
							{
								*sample= -(*source);
							}
							else
							{
								*sample=SHRT_MIN;
							}
							sample++;
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
				}
				else
				{
					for (k=start;k<end;k++)
					{
						*sample= *source;
						sample++;
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
					if (end2>0)
					{
						source=(ni_card->hardware_buffer)+((ni_card->channel_reorder)[
							(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]);
						for (k=0;k<end2;k++)
						{
							*sample= *source;
							sample++;
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
				}
#if defined (OLD_CODE)
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
				}
				source=((module_NI_CARDS[(channel_number-1)/
					NUMBER_OF_CHANNELS_ON_NI_CARD]).hardware_buffer)+
					(start*NUMBER_OF_CHANNELS_ON_NI_CARD+
					(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD);
				if ((UnEmap_2V1==module_NI_CARDS[(channel_number-1)/
					NUMBER_OF_CHANNELS_ON_NI_CARD].unemap_hardware_version)||
					(UnEmap_2V2==module_NI_CARDS[(channel_number-1)/
					NUMBER_OF_CHANNELS_ON_NI_CARD].unemap_hardware_version))
				{
					/* UnEmap_2V1 and UnEmap_2V2 invert */
					for (i=start;i<end;i++)
					{
						if (*source<SHRT_MAX)
						{
							*sample= -(*source);
						}
						else
						{
							*sample=SHRT_MIN;
						}
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						sample++;
					}
				}
				else
				{
					for (i=start;i<end;i++)
					{
						*sample= *source;
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						sample++;
					}
				}
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					start=0;
					end=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
					source=((module_NI_CARDS[(channel_number-1)/
						NUMBER_OF_CHANNELS_ON_NI_CARD]).hardware_buffer)+
						((channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD);
					if ((UnEmap_2V1==module_NI_CARDS[(channel_number-1)/
						NUMBER_OF_CHANNELS_ON_NI_CARD].unemap_hardware_version)||
						(UnEmap_2V2==module_NI_CARDS[(channel_number-1)/
						NUMBER_OF_CHANNELS_ON_NI_CARD].unemap_hardware_version))
					{
						/* UnEmap_2V1 and UnEmap_2V2 invert */
						for (i=start;i<end;i++)
						{
							if (*source<SHRT_MAX)
							{
								*sample= -(*source);
							}
							else
							{
								*sample=SHRT_MIN;
							}
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
							sample++;
						}
					}
					else
					{
						for (i=start;i<end;i++)
						{
							*sample= *source;
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
							sample++;
						}
					}
				}
#endif /* defined (OLD_CODE) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_samples_acquired.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired.  Invalid argument(s).  %p %d",samples,
			channel_number);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_get_samples_acquired %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired */

int unemap_get_card_state(int channel_number,int *battA_state,
	int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling_address,
	int *sampling_interval,int *settling_step_max_address)
/*******************************************************************************
LAST MODIFIED : 30 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i;
	i16 state,status;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_card_state);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&battA_state&&battGood_state&&
		filter_frequency&&filter_taps&&shift_registers&&GA0_state&&GA1_state&&
		NI_gain&&input_mode&&polarity&&tol_settling_address&&sampling_interval&&
		settling_step_max_address)
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			*battA_state=BattA_setting_UnEmap2vx;
			status=DIG_In_Line(ni_card->device_number,0,
				battery_good_input_line_UnEmap2vx,&state);
			if (state)
			{
				*battGood_state=1;
			}
			else
			{
				*battGood_state=0;
			}
			for (i=0;i<10;i++)
			{
				shift_registers[i]=((ni_card->unemap_2vx).shift_register)[i];
			}
			if ((ni_card->unemap_2vx).gain_output_line_0)
			{
				*GA0_state=1;
			}
			else
			{
				*GA0_state=0;
			}
			if ((ni_card->unemap_2vx).gain_output_line_1)
			{
				*GA1_state=1;
			}
			else
			{
				*GA1_state=0;
			}
			*NI_gain=(int)(ni_card->gain);
			unemap_get_antialiasing_filter_frequency(channel_number,filter_frequency);
			*filter_taps=ni_card->anti_aliasing_filter_taps;
			*input_mode=ni_card->input_mode;
			*polarity=ni_card->polarity;
			*tol_settling_address=tol_settling;
			*sampling_interval=ni_card->sampling_interval;
			*settling_step_max_address=settling_step_max;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_card_state.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_get_card_state.  Invalid argument(s).  %d %p %p %p %p %p %p %p %p %p %p %p %p %p",
			channel_number,battA_state,battGood_state,filter_frequency,filter_taps,
			shift_registers,GA0_state,GA1_state,NI_gain,input_mode,polarity,
			tol_settling,sampling_interval,settling_step_max);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_card_state */

int unemap_start_sampling(void)
/*******************************************************************************
LAST MODIFIED : 30 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts the sampling.
???DB.  Check if already going
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float frequency;
	int channel_number,i;
	i16 status,stopped;
	u32 retrieved;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_sampling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS)&&
		(0<module_sampling_low_count)&&(0<module_sampling_high_count))
	{
		if (UnEmap_1V2==module_NI_CARDS->unemap_hardware_version)
		{
			/* force a reset of the filter frequency (because power may have been
				turned on and off manually */
			channel_number=1;
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				unemap_get_antialiasing_filter_frequency(channel_number,&frequency);
				module_NI_CARDS[i].anti_aliasing_filter_taps= -1;
				unemap_set_antialiasing_filter_frequency(channel_number,frequency);
				channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
		}
		if (!module_slave)
		{
			/* stop any current sampling */
				/*???DB.  Check instead using GPCTR_Watch ? */
			status=GPCTR_Control((module_NI_CARDS[0]).device_number,SCAN_COUNTER,
				ND_RESET);
			/* set up the conversion signal */
				/*???DB.  Can most of this be moved into unemap_configure ? */
			if (0==status)
			{
				status=GPCTR_Set_Application((module_NI_CARDS[0]).device_number,
					SCAN_COUNTER,ND_PULSE_TRAIN_GNR);
				if (0==status)
				{
					status=GPCTR_Change_Parameter((module_NI_CARDS[0]).device_number,
						SCAN_COUNTER,ND_COUNT_1,module_sampling_low_count);
					if (0==status)
					{
						status=GPCTR_Change_Parameter((module_NI_CARDS[0]).device_number,
							SCAN_COUNTER,ND_COUNT_2,module_sampling_high_count);
						if (0==status)
						{
							status=Select_Signal((module_NI_CARDS[0]).device_number,ND_RTSI_0,
								ND_GPCTR0_OUTPUT,ND_DONT_CARE);
							if (0==status)
							{
								status=DAQ_Check((module_NI_CARDS[0]).device_number,&stopped,
									&retrieved);
								if (0==status)
								{
									module_starting_sample_number=((unsigned long)retrieved/
										(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD)%
										((module_NI_CARDS[0]).hardware_buffer_size/
										(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD);
									module_sample_buffer_size=0;
									/* start the data acquisition */
									status=GPCTR_Control((module_NI_CARDS[0]).device_number,
										SCAN_COUNTER,ND_PROGRAM);
									if (0==status)
									{
										return_code=1;
										module_sampling_on=1;
									}
									else
									{
										display_message(ERROR_MESSAGE,
									"unemap_start_sampling.  GPCTR_Control (ND_PROGRAM) failed");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"unemap_start_sampling.  DAQ_Check failed");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"unemap_start_sampling.  Select_Signal (ND_GPCTR0_SOURCE) failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"unemap_start_sampling.  GPCTR_Change_Parameter (high length) failed");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  GPCTR_Change_Parameter (low length) failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_start_sampling.  GPCTR_Set_Application failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  GPCTR_Control (ND_RESET) failed");
			}
		}
		else
		{
			status=DAQ_Check((module_NI_CARDS[0]).device_number,&stopped,
				&retrieved);
			if (0==status)
			{
				module_starting_sample_number=((unsigned long)retrieved/
					(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD)%
					((module_NI_CARDS[0]).hardware_buffer_size/
					(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD);
				module_sample_buffer_size=0;
				return_code=1;
				module_sampling_on=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  DAQ_Check failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_sampling.  Invalid configuration.  %d %p %d %d %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS,
			module_sampling_low_count,module_sampling_high_count);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_start_sampling */

int unemap_stop_sampling(void)
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops the sampling.  Use <unemap_get_number_of_samples_acquired> to find out how
many samples were acquired.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	i16 status,stopped;
	u32 number_of_samples,sample_number;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_stop_sampling);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_stop_sampling %lu\n",
		module_sample_buffer_size);
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* stop continuous sampling */
		status=GPCTR_Control((module_NI_CARDS[0]).device_number,SCAN_COUNTER,
			ND_RESET);
		if (0==status)
		{
			status=DAQ_Check((module_NI_CARDS[0]).device_number,&stopped,
				&sample_number);
			if (0==status)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"%lu %lu %lu\n",sample_number,
					(module_NI_CARDS[0]).hardware_buffer_size,
					module_starting_sample_number);
#endif /* defined (DEBUG) */
				sample_number /= NUMBER_OF_CHANNELS_ON_NI_CARD;
				number_of_samples=((module_NI_CARDS[0]).hardware_buffer_size)/
					NUMBER_OF_CHANNELS_ON_NI_CARD;
				if (module_sample_buffer_size<number_of_samples)
				{
					if (sample_number<
						module_starting_sample_number+module_sample_buffer_size)
					{
						module_sample_buffer_size += (sample_number+number_of_samples)-
							(module_starting_sample_number+module_sample_buffer_size);
					}
					else
					{
						module_sample_buffer_size += sample_number-
							(module_starting_sample_number+module_sample_buffer_size);
					}
					if (module_sample_buffer_size>number_of_samples)
					{
						module_sample_buffer_size=number_of_samples;
						module_starting_sample_number=sample_number%number_of_samples;
					}
				}
				else
				{
					module_starting_sample_number=sample_number%number_of_samples;
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_stop_sampling.  DAQ_Check failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_sampling.  GPCTR_Control failed");
		}
		module_sampling_on=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_sampling.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_stop_sampling %d %lu\n",
		return_code,module_sample_buffer_size);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_sampling */

int register_write_signal_file(char *file_name,int channel_number)
{
	FILE *output_file;
	float sampling_frequency;
	int electrodes_in_row[8]={8,8,8,8,8,8,8,8},i,index,j,return_code;
	short int *destination,*samples,*source;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;
	unsigned long number_of_channels,number_of_samples;

	ENTER(register_write_signal_file);
	return_code=0;
	/* check arguments */
	if (file_name&&unemap_get_number_of_channels(&number_of_channels)&&
		(0<channel_number)&&((unsigned long)channel_number<number_of_channels))
	{
		if (unemap_get_number_of_samples_acquired(&number_of_samples)&&
			(0<number_of_samples)&&ALLOCATE(samples,short int,number_of_samples*
			number_of_channels))
		{
			if (unemap_get_samples_acquired(0,samples)&&
				unemap_get_sampling_frequency(&sampling_frequency))
			{
				/* create a rig for saving signals */
				if (rig=create_standard_Rig("default",PATCH,MONITORING_OFF,
					EXPERIMENT_OFF,8,electrodes_in_row,1,0,(float)1))
				{
					if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,
						max_channels,(int)number_of_samples,sampling_frequency))
					{
						/* set the times */
						for (index=0;index<(int)number_of_samples;index++)
						{
							(signal_buffer->times)[index]=index;
						}
						device=rig->devices;
						i=rig->number_of_devices;
						while (rig&&(i>0))
						{
							index=((*device)->channel->number)-1;
							if (!((*device)->signal=create_Signal(index,signal_buffer,
								UNDECIDED,0)))
							{
								display_message(ERROR_MESSAGE,"Could not create signal_buffer");
								destroy_Rig(&rig);
							}
							device++;
							i--;
						}
						if (rig)
						{
							source=samples+(((channel_number-1)/
								NUMBER_OF_CHANNELS_ON_NI_CARD)*NUMBER_OF_CHANNELS_ON_NI_CARD);
							destination=(signal_buffer->signals).short_int_values;
							signal_buffer->start=0;
							signal_buffer->end=(int)number_of_samples-1;
							for (index=(int)number_of_samples;index>0;index--)
							{
								for (j=max_channels;j>0;j--)
								{
									*destination= *source;
									destination++;
									source++;
								}
								source += (number_of_channels-max_channels);
							}
							if (output_file=fopen(file_name,"wb"))
							{
								return_code=write_signal_file(output_file,rig);
								fclose(output_file);
								if (!return_code)
								{
									remove(file_name);
								}
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"register_write_signal_file.  Invalid file: %s",file_name);
							}
						}
						destroy_Signal_buffer(&signal_buffer);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"register_write_signal_file.  Could not create signal_buffer");
					}
					destroy_Rig(&rig);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"register_write_signal_file.  Could not create rig");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"register_write_signal_file.  Could not retrieve samples");
			}
			DEALLOCATE(samples);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"register_write_signal_file.  Insufficient memory for samples");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"register_write_signal_file.  Invalid argument(s).  %p %d\n",file_name,
			channel_number);
	}
	LEAVE;

	return (return_code);
} /* register_write_signal_file */

static void print_menu(int channel_number,unsigned long number_of_channels)
{
	float filter_frequency,tol_settling;
	int battA_state,battGood_state,filter_taps,GA0_state,GA1_state,i,input_mode,
		NI_gain,polarity,sampling_interval,settling_step_max;
	unsigned char bit_mask,byte_number,shift_registers[10];

	printf("card number %d of %lu\n",
		(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD+1,
		number_of_channels/NUMBER_OF_CHANNELS_ON_NI_CARD);
	if (unemap_get_card_state(channel_number,&battA_state,&battGood_state,
		&filter_frequency,&filter_taps,shift_registers,&GA0_state,&GA1_state,
		&NI_gain,&input_mode,&polarity,&tol_settling,&sampling_interval,
		&settling_step_max))
	{
		printf("BattA : %d, ",battA_state);
		printf("BattGood : %d\n",battGood_state);
		printf("GA0 : %d, ",GA0_state);
		printf("GA1 : %d, ",GA1_state);
		printf("NI gain : %d, ",NI_gain);
		printf("filter frequency : %g (%d taps)\n",filter_frequency,filter_taps);
		bit_mask=(unsigned char)0x1;
		byte_number=0;
		for (i=1;i<=80;i++)
		{
			printf("%2d ",i);
			if (shift_registers[byte_number]&bit_mask)
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
			if (0==i%10)
			{
				printf("\n");
			}
			else
			{
				printf("  * ");
			}
			if (0==i%8)
			{
				byte_number++;
				bit_mask=(unsigned char)0x1;
			}
			else
			{
				bit_mask <<= 1;
			}
		}
	}
	printf("(0) Exit                    \n");
	printf("(1) Single test             \n");
	printf("(2) Set calibrate mode      \n");
	printf("(3) Set record mode         \n");
	printf("(4) Test until error        \n");
	printf("?\n");
} /* print_menu */

static void process_keyboard(
#if defined (WINDOWS)
	int channel_number,unsigned long number_of_channels,
	unsigned long number_of_samples,unsigned sampling_delay,short int *samples,
	float sampling_frequency
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer process_keyboard_data_void,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
{
	char option;
	float mean_VCC,mean_VCCm,mean_VCCp,ratio_VCCm_VCC,ratio_VCC_VCCp;
	int count,i,offset,return_code;
	short int *sample;
#if defined (MOTIF)
	float sampling_frequency;
	int channel_number;
	short int *samples;
	struct Process_keyboard_data *process_keyboard_data;
	unsigned sampling_delay;
	unsigned long number_of_channels,number_of_samples;
#endif /* defined (MOTIF) */

#if defined (MOTIF)
	USE_PARAMETER(source);
	USE_PARAMETER(id);
	process_keyboard_data=
		(struct Process_keyboard_data *)process_keyboard_data_void;
	sampling_frequency=process_keyboard_data->sampling_frequency;
	channel_number=process_keyboard_data->channel_number;
	samples=process_keyboard_data->samples;
	sampling_delay=process_keyboard_data->sampling_delay;
	number_of_channels=process_keyboard_data->number_of_channels;
	number_of_samples=process_keyboard_data->number_of_samples;
#endif /* defined (MOTIF) */
	scanf("%c",&option);
	if (isalnum(option))
	{
		if ('0'!=option)
		{
			switch (option)
			{
				case '1':
				{
					/* single test */
					unemap_start_sampling();
					unemap_set_power(1);
#if defined (WINDOWS)
					/* in milliseconds */
					Sleep((DWORD)100);
#endif /* defined (WINDOWS) */
					unemap_set_power(0);
#if defined (WINDOWS)
					/* in milliseconds */
					Sleep((DWORD)100);
#endif /* defined (WINDOWS) */
					unemap_stop_sampling();
					register_write_signal_file("surge.sig",channel_number);
					if (unemap_get_samples_acquired(0,samples))
					{
						/* assume that channel 1 is VCC, channel 2 is VCC- and channel 3 is
							VCC+ */
						mean_VCC=0;
						mean_VCCp=0;
						mean_VCCm=0;
						sample=samples;
						for (i=number_of_samples;i>0;i--)
						{
							mean_VCC += sample[0];
							mean_VCCm += sample[1];
							mean_VCCp += sample[2];
							sample += number_of_channels;
						}
						mean_VCC /= (float)number_of_samples;
						mean_VCCm /= (float)number_of_samples;
						mean_VCCp /= (float)number_of_samples;
						printf("mean VCC = %g\n",mean_VCC);
						printf("mean VCC- = %g\n",mean_VCCm);
						printf("mean VCC+ = %g\n",mean_VCCp);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"main.  measure noise.  Could not retrieve samples");
					}
				} break;
				case '2':
				{
					/* set isolate mode */
					unemap_set_isolate_record_mode(channel_number,1);
				} break;
				case '3':
				{
					/* set record mode */
					unemap_set_isolate_record_mode(channel_number,0);
				} break;
				case '4':
				{
					/* test until error */
					return_code=1;
					count=0;
					do
					{
						count++;
						unemap_start_sampling();
						unemap_set_power(1);
#if defined (WINDOWS)
						/* in milliseconds */
						Sleep((DWORD)100);
#endif /* defined (WINDOWS) */
						unemap_set_power(0);
#if defined (WINDOWS)
						/* in milliseconds */
						Sleep((DWORD)100);
#endif /* defined (WINDOWS) */
						unemap_stop_sampling();
						if (unemap_get_samples_acquired(0,samples))
						{
							/* assume that channel 1 is VCC, channel 2 is VCC- and channel 3
								is VCC+ */
							mean_VCC=0;
							mean_VCCp=0;
							mean_VCCm=0;
							sample=samples;
							/* only average "steady state" */
/*							for (i=number_of_samples;i>0;i--) */
							offset=(int)(0.1*sampling_frequency);
							sample += offset*number_of_channels;
							for (i=number_of_samples-2*offset;i>0;i--)
							{
								mean_VCC += sample[0];
								mean_VCCm += sample[1];
								mean_VCCp += sample[2];
								sample += number_of_channels;
							}
							mean_VCC /= (float)number_of_samples;
							mean_VCCm /= (float)number_of_samples;
							mean_VCCp /= (float)number_of_samples;
							if (0!=mean_VCC)
							{
								ratio_VCCm_VCC=mean_VCCm/mean_VCC;
							}
							else
							{
								ratio_VCCm_VCC=0;
							}
							if (0!=mean_VCCp)
							{
								ratio_VCC_VCCp=mean_VCC/mean_VCCp;
							}
							else
							{
								ratio_VCC_VCCp=0;
							}
						}
						else
						{
							return_code=0;
							display_message(ERROR_MESSAGE,
								"main.  test until error.  Could not retrieve samples");
						}
						/*???debug */
						printf("%d.  %g %g\n",count,ratio_VCCm_VCC,ratio_VCC_VCCp);
						printf("  mean VCC = %g\n",mean_VCC);
						printf("  mean VCC- = %g\n",mean_VCCm);
						printf("  mean VCC+ = %g\n",mean_VCCp);
						/* give time for capacitors to discharge */
#if defined (WINDOWS)
						/* in milliseconds */
						Sleep((DWORD)2000);
#endif /* defined (WINDOWS) */
/*					} while (return_code&&(ratio_VCCm_VCC< -1)&&(1<ratio_VCC_VCCp));*/
					} while (return_code&&(ratio_VCCm_VCC< -0.9)&&(0.9<ratio_VCC_VCCp));
					printf("number of power on/off cycles = %d\n",count);
					printf("mean VCC = %g\n",mean_VCC);
					printf("mean VCC- = %g\n",mean_VCCm);
					printf("mean VCC+ = %g\n",mean_VCCp);
					register_write_signal_file("surge.sig",channel_number);
				} break;
				default:
				{
					printf(">>>Unknown option\n");
				} break;
			}
			print_menu(channel_number,number_of_channels);
		}
		else
		{
			unemap_set_isolate_record_mode(0,1);
			unemap_set_power(0);
			unemap_deconfigure();
			exit(0);
		}
	}
#if defined (MOTIF)
	process_keyboard_data->sampling_frequency=sampling_frequency;
	process_keyboard_data->channel_number=channel_number;
	process_keyboard_data->samples=samples;
	process_keyboard_data->sampling_delay=sampling_delay;
	process_keyboard_data->number_of_channels=number_of_channels;
	process_keyboard_data->number_of_samples=number_of_samples;
#endif /* defined (MOTIF) */
} /* process_keyboard */

void main(void)
{
	float sampling_frequency;
	int channel_number;
	short int *samples;
	unsigned long number_of_channels,number_of_samples,sampling_delay;
#if defined (MOTIF)
	struct Process_keyboard_data process_keyboard_data;
#endif /* defined (MOTIF) */

	/* initialize */
	number_of_samples=(unsigned long)5000;
	sampling_frequency=(float)5000;
#if defined (MOTIF)
	if (application_context=XtCreateApplicationContext())
	{
#endif /* defined (MOTIF) */
		/* in case unemap hardware already running */
		unemap_set_isolate_record_mode(0,1);
		unemap_set_power(0);
		unemap_deconfigure();
		if (unemap_configure(sampling_frequency,(int)number_of_samples,
#if defined (WINDOWS)
			(HWND)NULL,0,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			application_context,
#endif /* defined (MOTIF) */
			(Unemap_hardware_callback *)NULL,(void *)NULL,(float)0)&&
			unemap_get_sampling_frequency(&sampling_frequency)&&
			unemap_get_maximum_number_of_samples(&number_of_samples)&&
			unemap_get_number_of_channels(&number_of_channels))
		{
			unemap_stop_scrolling();
			if (ALLOCATE(samples,short int,number_of_samples*number_of_channels))
			{
				sampling_delay=1+number_of_samples/(unsigned)sampling_frequency;
				channel_number=1;
#if defined (MOTIF)
				process_keyboard_data.sampling_frequency=sampling_frequency;
				process_keyboard_data.channel_number=channel_number;
				process_keyboard_data.samples=samples;
				process_keyboard_data.sampling_delay=sampling_delay;
				process_keyboard_data.number_of_channels=number_of_channels;
				process_keyboard_data.number_of_samples=number_of_samples;
				if (XtAppAddInput(application_context,fileno(stdin),
					(XtPointer)XtInputReadMask,process_keyboard,
					(XtPointer)&process_keyboard_data))
				{
#endif /* defined (MOTIF) */
					print_menu(channel_number,number_of_channels);
#if defined (MOTIF)
					XtAppMainLoop(application_context);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					while (1)
					{
						process_keyboard(channel_number,number_of_channels,
							number_of_samples,sampling_delay,samples,sampling_frequency);
					}
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				}
#endif /* defined (MOTIF) */
				DEALLOCATE(samples);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"main.  Insufficient memory for samples");
			}
			/* make sure that the cards are in isolate mode and powered off */
			unemap_set_isolate_record_mode(0,1);
			unemap_set_power(0);
		}
		else
		{
			display_message(ERROR_MESSAGE,"No NI cards");
		}
#if defined (MOTIF)
	}
#endif /* defined (MOTIF) */
} /* main */

#if defined (OLD_CODE)
#if defined (NI_DAQ)
static int set_NI_gain(struct NI_card *ni_card,int gain)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Convenience function for setting the <ni_card> <gain>.  Doesn't change the gain
field of <ni_card>.
==============================================================================*/
{
	int return_code,sampling_on;
	i16 channel_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],
		gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],i,j,status;
	struct NI_card *ni_card_temp;

	ENTER(set_NI_gain);
	return_code=0;
	if (ni_card)
	{
		if (sampling_on=module_sampling_on)
		{
			unemap_stop_sampling();
		}
		/* would like to be able to restart the rolling buffer for just <ni_card>,
			but because the rolling buffer can only be restarted at the beginning,
			all the cards have to be restarted, otherwise they'd be out of synch */
		module_sample_buffer_size=0;
		module_starting_sample_number=0;
		ni_card_temp=module_NI_CARDS;
		status=0;
		j=module_number_of_NI_CARDS;
		for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
		{
			channel_vector[i]=i;
		}
		while ((0==status)&&(j>0))
		{
			if (ni_card==ni_card_temp)
			{
				for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
				{
					gain_vector[i]=(i16)gain;
				}
			}
			else
			{
				for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
				{
					gain_vector[i]=(i16)(ni_card_temp->gain);
				}
			}
			status=DAQ_Clear(ni_card_temp->device_number);
			if (0==status)
			{
				if (module_NI_CARDS==ni_card_temp)
				{
					status=Config_DAQ_Event_Message(ni_card_temp->device_number,
						/* add message */(i16)1,/* channel string */"AI0",
						/* send message every N scans */(i16)1,
						(i32)module_scrolling_refresh_period,(i32)0,(u32)0,(u32)0,
						(u32)0,(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
					if (0!=status)
					{
						display_message(ERROR_MESSAGE,
							"set_NI_gain.  Config_DAQ_Event_Message 1 failed.  %d %d",status,
							module_scrolling_refresh_period);
					}
				}
				status=DAQ_DB_Config(ni_card_temp->device_number,/* enable */(i16)1);
				if (0==status)
				{
					status=SCAN_Setup(ni_card_temp->device_number,
						NUMBER_OF_CHANNELS_ON_NI_CARD,channel_vector,gain_vector);
					if (0==status)
					{
						status=SCAN_Start(ni_card_temp->device_number,
							(i16 *)(ni_card_temp->hardware_buffer),
							(u32)(ni_card_temp->hardware_buffer_size),ni_card_temp->time_base,
							ni_card_temp->sampling_interval,(i16)0,(u16)0);
						if (0==status)
						{
							if (module_NI_CARDS==ni_card_temp)
							{
								/*???DB.  I don't understand why this is needed, but if it is
									not present, then get lots of extraneous callbacks (lParam is
									wrong) */
								status=Config_DAQ_Event_Message(
									module_NI_CARDS[0].device_number,
									/* clear all messages */(i16)0,
									/* channel string */(char *)NULL,
									/* send message every N scans */(i16)0,
									(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,(HWND)NULL,(i16)0,(u32)0);
								if (0!=status)
								{
									display_message(ERROR_MESSAGE,
										"set_NI_gain.  Config_DAQ_Event_Message 2 failed.  %d",
										status);
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_NI_gain.  SCAN_Start failed.  %d",status);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_NI_gain.  SCAN_Setup failed.  %d",status);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_NI_gain.  DAQ_DB_Config failed.  %d",status);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_NI_gain.  DAQ_Clear failed.  %d",
					status);
			}
			j--;
			ni_card_temp++;
		}
		if (0==status)
		{
			return_code=1;
		}
		if (sampling_on)
		{
			unemap_start_sampling();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_NI_gain.  Invalid argument.  %p",
			ni_card);
	}
	LEAVE;

	return (return_code);
} /* set_NI_gain */
#endif /* defined (NI_DAQ) */

static int compare_float(const float *float_1,const float *float_2)
{
	int return_code;

	ENTER(compare_float);
	if (*float_1< *float_2)
	{
		return_code= -1;
	}
	else
	{
		if (*float_1> *float_2)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_float */

#if defined (NI_DAQ)
static int calibration_callback(void *dummy)
/*******************************************************************************
LAST MODIFIED : 17 August 1999

DESCRIPTION :
Output a square wave which alternates between +/- a known voltage.  Wait for the
DC removal to settle.  Acquire.  Average for each channel.  Assume that the
measurements below the average are for minus the known voltage and that the
measurements above the average are for plus the known voltage.  Calculate
offsets and gains.  Over-write calibrate.dat

The D/A on the NI cards doesn't have enough resolution (+/-10V and 12 or 16-bit)
to cope with the full gain (100*11*8=8800), so the NI gain and the signal
conditioning gain are done separately and then combined.

stage 0 is setup
stage 1 is calculating the signal conditioning fixed gain
stage 2 is calculating the signal conditioning variable gain
stage 3 is calculating the NI gain
stage 4 is calculating the offset
==============================================================================*/
{
	float *calibration_gains,*calibration_offsets,*filter_frequency,*gain,
		*offset,*previous_offset,temp;
	f64 *calibrate_amplitude;
	int *calibration_channels,card_number,channel_number,i,j,k,
		number_of_settled_channels,return_code;
	i16 da_channel,status;
	long int maximum_sample_value,minimum_sample_value;
	short int *hardware_buffer;
	static float *filter_frequencies,*gains=(float *)NULL,*offsets=(float *)NULL,
		*previous_offsets=(float *)NULL,saturated;
	static f64 *calibrate_amplitudes,*calibrate_voltages;
	static int calibrate_stage=0,*channel_failed,number_of_channels,
		number_of_waveform_points,settling_step,stage_flag,stage_mask;
	static i16 *calibrate_voltage_DAs;
	static unsigned number_of_samples;
	struct NI_card *ni_card;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
	float *sorted_signal_entry;
	static float *sorted_signal=(float *)NULL;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
	float temp_2;
	double two_pi;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */

	ENTER(calibration_callback);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"enter calibration_callback %d\n",calibrate_stage);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	return_code=0;
	status=0;
	if (0==calibrate_stage)
	{
		/* set up */
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			stage_flag=0x1;
			stage_mask=0x1;
			number_of_channels=
				module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD;
			number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
				NUMBER_OF_CHANNELS_ON_NI_CARD;
			unemap_get_sample_range(&minimum_sample_value,&maximum_sample_value);
			saturated=(float)(-minimum_sample_value);
			ALLOCATE(calibrate_amplitudes,f64,module_number_of_NI_CARDS);
#if defined (CALIBRATE_SIGNAL_SQUARE)
			number_of_waveform_points=2;
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
			number_of_waveform_points=500;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
			ALLOCATE(calibrate_voltages,f64,number_of_waveform_points);
			ALLOCATE(calibrate_voltage_DAs,i16,number_of_waveform_points);
			ALLOCATE(filter_frequencies,float,module_number_of_NI_CARDS);
			ALLOCATE(offsets,float,number_of_channels);
			ALLOCATE(previous_offsets,float,number_of_channels);
			ALLOCATE(gains,float,number_of_channels);
			ALLOCATE(channel_failed,int,number_of_channels);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
			ALLOCATE(sorted_signal,float,number_of_samples);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			if (calibrate_voltages&&calibrate_voltage_DAs&&filter_frequencies&&
				offsets&&previous_offsets&&gains&&channel_failed&&calibrate_amplitudes
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				&&sorted_signal
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
				)
			{
				for (i=0;i<number_of_channels;i++)
				{
					offsets[i]=(float)0;
					channel_failed[i]=0xf;
				}
				return_code=unemap_stop_sampling();
				card_number=0;
				ni_card=module_NI_CARDS;
				calibrate_amplitude=calibrate_amplitudes;
				filter_frequency=filter_frequencies;
				while (return_code&&(card_number<module_number_of_NI_CARDS))
				{
					/* if the card can be calibrated (has D/A) */
					if ((PCI6031E_AD_DA==ni_card->type)||
						(PXI6031E_AD_DA==ni_card->type)||(PXI6071E_AD_DA==ni_card->type))
					{
						channel_number=1+card_number*NUMBER_OF_CHANNELS_ON_NI_CARD;
						/* put in isolate mode */
						if (unemap_set_isolate_record_mode(channel_number,1))
						{
							/* stop waveform generation */
							WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
							/* change NI gain to 1 */
							return_code=set_NI_gain(ni_card,(int)1);
							if (return_code)
							{
								/* configure the DA to bipolar +/-10 */
								status=AO_Configure(ni_card->device_number,CALIBRATE_CHANNEL,
									/*bipolar*/(i16)0,/*internal voltage reference*/(i16)0,
									/*reference voltage*/(f64)10,
									/*update DAC when written to*/(i16)0);
								if (0==status)
								{
									/* determine the calibration voltage */
									/* start with full-range voltage */
									if (PXI6071E_AD_DA==ni_card->type)
									{
										*calibrate_amplitude=(f64)5;
									}
									else
									{
										*calibrate_amplitude=(f64)10;
									}
									/* use half-range */
									*calibrate_amplitude /= (f64)2;
									switch (ni_card->unemap_hardware_version)
									{
										case UnEmap_1V2:
										{
											/* fixed gain of 10 */
											*calibrate_amplitude /= (f64)10;
										} break;
										case UnEmap_2V1:
										case UnEmap_2V2:
										{
											/* fixed gain of 11 */
											*calibrate_amplitude /= (f64)11;
										} break;
									}
#if defined (CALIBRATE_SIGNAL_SQUARE)
									calibrate_voltages[0]= *calibrate_amplitude;
									calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
									/* calculate cosine wave */
									two_pi=(double)8*atan((double)1);
									for (i=0;i<number_of_waveform_points;i++)
									{
										calibrate_voltages[i]=
											(*calibrate_amplitude)*(f64)cos(two_pi*
											(double)i/(double)number_of_waveform_points);
									}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
									/* convert to a DA setting */
									status=WFM_Scale(ni_card->device_number,CALIBRATE_CHANNEL,
										number_of_waveform_points,/*gain*/(f64)1,calibrate_voltages,
										calibrate_voltage_DAs);
									if (0==status)
									{
										/* allow for DA resolution */
										if (PXI6071E_AD_DA==ni_card->type)
										{
											*calibrate_amplitude=
												(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)4096;
										}
										else
										{
											*calibrate_amplitude=
												(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)65536;
										}
										/* start the square waveform generation */
										da_channel=CALIBRATE_CHANNEL;
										status=WFM_Op(ni_card->device_number,
											/*number of DA channels*/1,&da_channel,
											calibrate_voltage_DAs,number_of_waveform_points,
											/*continous*/(u32)0,
											/*points/s*/(f64)((float)number_of_waveform_points*
											CALIBRATE_FREQUENCY));
										if (0==status)
										{
											if ((UnEmap_1V2==ni_card->unemap_hardware_version)||
												(UnEmap_2V2==ni_card->unemap_hardware_version))
											{
												/* set the programmable gain */
												DIG_Out_Line(ni_card->device_number,(i16)0,
													gain_0_output_line_UnEmap2vx,(i16)0);
												DIG_Out_Line(ni_card->device_number,(i16)0,
													gain_1_output_line_UnEmap2vx,(i16)0);
											}
											/* set low pass filter as high as possible */
											unemap_get_antialiasing_filter_frequency(channel_number,
												filter_frequency);
											unemap_set_antialiasing_filter_frequency(channel_number,
												(float)10000);
											card_number++;
											ni_card++;
											calibrate_amplitude++;
											filter_frequency++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"calibration_callback.  WFM_Op failed");
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"calibration_callback.  WFM_Scale failed");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  AO_Configure failed");
									return_code=0;
								}
								if (0==return_code)
								{
									set_NI_gain(ni_card,(int)(ni_card->gain));
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calibration_callback.  set_NI_gain failed 1");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"calibration_callback.  unemap_set_isolate_record_mode failed");
							return_code=0;
						}
					}
				}
				if (return_code)
				{
					module_buffer_full_callback=calibration_callback;
					module_buffer_full_callback_data=(void *)NULL;
					settling_step=0;
					calibrate_stage=1;
					if (return_code=unemap_start_sampling())
					{
						display_message(INFORMATION_MESSAGE,"Calibration.  Phase 1 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 1\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
					}
					else
					{
						module_buffer_full_callback=(Buffer_full_callback *)NULL;
						module_buffer_full_callback_data=(void *)NULL;
					}
				}
				if (!return_code)
				{
					while (card_number>0)
					{
						card_number--;
						ni_card--;
						calibrate_amplitude--;
						filter_frequency--;
						channel_number=1+card_number*NUMBER_OF_CHANNELS_ON_NI_CARD;
						unemap_set_antialiasing_filter_frequency(channel_number,
							*filter_frequency);
						/* stop waveform generation */
						WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
						/* set the output voltage to 0 */
						AO_Write(ni_card->device_number,CALIBRATE_CHANNEL,(i16)0);
						set_NI_gain(ni_card,(int)(ni_card->gain));
						if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))
						{
							/* programmable gain */
							if ((ni_card->unemap_2vx).gain_output_line_0)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)1);
							}
							if ((ni_card->unemap_2vx).gain_output_line_1)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)1);
							}
						}
					}
				}
			}
			else
			{
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				display_message(ERROR_MESSAGE,
		"calibration_callback.  Could not allocate storage.  %p %p %p %p %p %p %p",
					calibrate_voltages,calibrate_voltage_DAs,filter_frequencies,offsets,
					previous_offsets,gains,sorted_signal);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
				display_message(ERROR_MESSAGE,
				"calibration_callback.  Could not allocate storage.  %p %p %p %p %p %p",
					calibrate_voltages,calibrate_voltage_DAs,filter_frequencies,offsets,
					previous_offsets,gains);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			}
			if (0==return_code)
			{
				DEALLOCATE(calibrate_amplitudes);
				DEALLOCATE(calibrate_voltages);
				DEALLOCATE(calibrate_voltage_DAs);
				DEALLOCATE(filter_frequencies);
				DEALLOCATE(offsets);
				DEALLOCATE(previous_offsets);
				DEALLOCATE(gains);
				DEALLOCATE(channel_failed);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calibration_callback.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
		if ((0==return_code)&&module_calibration_end_callback)
		{
			(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
				(float *)NULL,module_calibration_end_callback_data);
		}
	}
	else
	{
		/* calibrate stage 1, 2, 3 or 4 */
		unemap_stop_sampling();
		/* average */
		offset=offsets;
		previous_offset=previous_offsets;
		for (i=0;i<number_of_channels;i++)
		{
			*previous_offset= *offset;
			*offset=(float)0;
			previous_offset++;
			offset++;
		}
		channel_number=0;
		ni_card=module_NI_CARDS;
		for (i=0;i<module_number_of_NI_CARDS;i++)
		{
			for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
			{
				hardware_buffer=(ni_card->hardware_buffer)+j;
				temp=(float)0;
				for (k=number_of_samples;k>0;k--)
				{
					temp += (float)(*hardware_buffer);
					hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
				}
				offsets[i*NUMBER_OF_CHANNELS_ON_NI_CARD+(ni_card->channel_reorder)[j]]=
					temp;
			}
		}
#if defined (OLD_CODE)
		channel_number=0;
		ni_card=module_NI_CARDS;
		for (i=module_number_of_NI_CARDS;i>0;i--)
		{
			hardware_buffer=ni_card->hardware_buffer;
			for (j=number_of_samples;j>0;j--)
			{
				offset=offsets+channel_number;
				for (k=NUMBER_OF_CHANNELS_ON_NI_CARD;k>0;k--)
				{
					*offset += (float)(*hardware_buffer);
					offset++;
					hardware_buffer++;
				}
			}
			channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
			ni_card++;
		}
#endif /* defined (OLD_CODE) */
		offset=offsets;
		for (i=0;i<number_of_channels;i++)
		{
			*offset /= (float)number_of_samples;
			offset++;
		}
		if (0<settling_step)
		{
			number_of_settled_channels=0;
			for (i=0;i<number_of_channels;i++)
			{
				if (channel_failed[i]&stage_flag)
				{
					if (((float)fabs((double)(previous_offsets[i]))<saturated)&&
						((float)fabs((double)(offsets[i]))<saturated)&&
						((float)fabs((double)(offsets[i]-previous_offsets[i]))<
						tol_settling))
					{
						channel_failed[i] &= !stage_flag;
						number_of_settled_channels++;
					}
				}
				if (!(channel_failed[i]&stage_mask))
				{
					number_of_settled_channels++;
				}
			}
/*???debug */
{
	FILE *unemap_debug;
	float maximum,temp;
	int ii,jj;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		maximum=(float)-1;
		jj= -1;
		for (ii=0;ii<number_of_channels;ii++)
		{
			temp=(float)fabs((double)(offsets[ii]-previous_offsets[ii]));
			if (temp>maximum)
			{
				maximum=temp;
				jj=ii;
			}
		}
		fprintf(unemap_debug,"settling %d %d %g %g %d %g\n",settling_step,i,
			saturated,tol_settling,jj,maximum);
		fclose(unemap_debug);
	}
}
#if defined (DEBUG)
#endif /* defined (DEBUG) */
			if ((number_of_settled_channels>=number_of_channels)||
				((number_of_settled_channels>0)&&(settling_step>=settling_step_max)))
			{
				/* settled */
				switch (calibrate_stage)
				{
					case 1:
					{
						/* signal conditioning fixed gain */
						gain=gains;
						offset=offsets;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						for (i=module_number_of_NI_CARDS;i>0;i--)
						{
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								hardware_buffer=(ni_card->hardware_buffer)+j;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
								sorted_signal_entry=sorted_signal;
								for (k=number_of_samples;k>0;k--)
								{
									temp=(float)(*hardware_buffer)-(*offset);
									if (temp<(float)0)
									{
										temp= -temp;
									}
									*sorted_signal_entry=temp;
									sorted_signal_entry++;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								qsort(sorted_signal,number_of_samples,sizeof(float),
									compare_float);
								temp=(float)0;
								for (k=(int)(switching_time*200*(float)number_of_samples);
									k<(int)number_of_samples;k++)
								{
									temp += sorted_signal[k];
								}
								temp /= (1-200*switching_time)*(float)number_of_samples;
#else /* defined (SWITCHING_TIME) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp_2=(float)(*hardware_buffer)-(*offset);
									temp += temp_2*temp_2;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
								temp *= 2;
								temp=(float)sqrt((double)temp);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								*gain=(float)(*calibrate_amplitude)/temp;
								offset++;
								gain++;
							}
							calibrate_amplitude++;
							ni_card++;
						}
						/* set up for next stage */
						card_number=0;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						return_code=1;
						while (return_code&&(card_number<module_number_of_NI_CARDS))
						{
							/* stop waveform generation */
							WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
							/* determine the calibration voltage */
							/* start with full-range voltage */
							if (PXI6071E_AD_DA==ni_card->type)
							{
								*calibrate_amplitude=(f64)5;
							}
							else
							{
								*calibrate_amplitude=(f64)10;
							}
							/* use half-range */
							*calibrate_amplitude /= (f64)2;
							switch (ni_card->unemap_hardware_version)
							{
								case UnEmap_1V2:
								{
									/* fixed gain of 10 */
									*calibrate_amplitude /= (f64)10;
								} break;
								case UnEmap_2V1:
								case UnEmap_2V2:
								{
									/* fixed gain of 11 */
									*calibrate_amplitude /= (f64)11;
									/* programmable gain */
									if ((ni_card->unemap_2vx).gain_output_line_0)
									{
										DIG_Out_Line(ni_card->device_number,(i16)0,
											gain_0_output_line_UnEmap2vx,(i16)1);
										*calibrate_amplitude /= (f64)2;
									}
									if ((ni_card->unemap_2vx).gain_output_line_1)
									{
										DIG_Out_Line(ni_card->device_number,(i16)0,
											gain_1_output_line_UnEmap2vx,(i16)1);
										*calibrate_amplitude /= (f64)4;
									}
								} break;
							}
#if defined (CALIBRATE_SIGNAL_SQUARE)
							calibrate_voltages[0]= *calibrate_amplitude;
							calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							/* calculate cosine wave */
							two_pi=(double)8*atan((double)1);
							for (i=0;i<number_of_waveform_points;i++)
							{
								calibrate_voltages[i]=
									(*calibrate_amplitude)*(f64)cos(two_pi*
									(double)i/(double)number_of_waveform_points);
							}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
							/* convert to a DA setting */
							status=WFM_Scale(ni_card->device_number,CALIBRATE_CHANNEL,
								number_of_waveform_points,/*gain*/(f64)1,calibrate_voltages,
								calibrate_voltage_DAs);
							if (0==status)
							{
								/* allow for DA resolution */
								if (PXI6071E_AD_DA==ni_card->type)
								{
									*calibrate_amplitude=
										(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)4096;
								}
								else
								{
									*calibrate_amplitude=
										(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)65536;
								}
								/* start the square waveform generation */
								da_channel=CALIBRATE_CHANNEL;
								status=WFM_Op(ni_card->device_number,
									/*number of DA channels*/1,&da_channel,
									calibrate_voltage_DAs,number_of_waveform_points,
									/*continous*/(u32)0,
									/*points/s*/(f64)((float)number_of_waveform_points*
									CALIBRATE_FREQUENCY));
								if (0==status)
								{
									card_number++;
									ni_card++;
									calibrate_amplitude++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  WFM_Op failed");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calibration_callback.  WFM_Scale failed");
								return_code=0;
							}
						}
						if (return_code)
						{
							settling_step=0;
							calibrate_stage=2;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 2 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 2\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 2:
					{
						/* signal conditioning variable gain */
						gain=gains;
						offset=offsets;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						for (i=module_number_of_NI_CARDS;i>0;i--)
						{
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								hardware_buffer=(ni_card->hardware_buffer)+j;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
								sorted_signal_entry=sorted_signal;
								for (k=number_of_samples;k>0;k--)
								{
									temp=(float)(*hardware_buffer)-(*offset);
									if (temp<(float)0)
									{
										temp= -temp;
									}
									*sorted_signal_entry=temp;
									sorted_signal_entry++;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								qsort(sorted_signal,number_of_samples,sizeof(float),
									compare_float);
								temp=(float)0;
								for (k=(int)(switching_time*200*(float)number_of_samples);
									k<(int)number_of_samples;k++)
								{
									temp += sorted_signal[k];
								}
								temp /= (1-200*switching_time)*(float)number_of_samples;
#else /* defined (SWITCHING_TIME) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp_2=(float)(*hardware_buffer)-(*offset);
									temp += temp_2*temp_2;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
								temp *= 2;
								temp=(float)sqrt((double)temp);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* *gain is already the fixed gain */
								*gain=((float)(*calibrate_amplitude)/temp)/(*gain);
								offset++;
								gain++;
							}
							calibrate_amplitude++;
							ni_card++;
						}
						/* set up for next stage */
						card_number=0;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						return_code=1;
						while (return_code&&(card_number<module_number_of_NI_CARDS))
						{
							/* stop waveform generation */
							WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
							/* set the gain */
							DIG_Out_Line(ni_card->device_number,(i16)0,
								gain_0_output_line_UnEmap2vx,(i16)0);
							DIG_Out_Line(ni_card->device_number,(i16)0,
								gain_1_output_line_UnEmap2vx,(i16)0);
							return_code=set_NI_gain(ni_card,(int)(ni_card->gain));
							if (return_code)
							{
								/* determine the calibration voltage */
								/* start with full-range voltage */
								if (PXI6071E_AD_DA==ni_card->type)
								{
									*calibrate_amplitude=(f64)5;
								}
								else
								{
									*calibrate_amplitude=(f64)10;
								}
								/* use half-range */
								*calibrate_amplitude /= (f64)2;
								switch (ni_card->unemap_hardware_version)
								{
									case UnEmap_1V2:
									{
										/* fixed gain of 10 */
										*calibrate_amplitude /= (f64)10;
									} break;
									case UnEmap_2V1:
									case UnEmap_2V2:
									{
										/* fixed gain of 11 */
										*calibrate_amplitude /= (f64)11;
									} break;
								}
								/* NI gain of 11 */
								*calibrate_amplitude /= (f64)(ni_card->gain);
#if defined (CALIBRATE_SIGNAL_SQUARE)
								calibrate_voltages[0]= *calibrate_amplitude;
								calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* calculate cosine wave */
								two_pi=(double)8*atan((double)1);
								for (i=0;i<number_of_waveform_points;i++)
								{
									calibrate_voltages[i]=
										(*calibrate_amplitude)*(f64)cos(two_pi*
										(double)i/(double)number_of_waveform_points);
								}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* convert to a DA setting */
								status=WFM_Scale(ni_card->device_number,CALIBRATE_CHANNEL,
									number_of_waveform_points,/*gain*/(f64)1,calibrate_voltages,
									calibrate_voltage_DAs);
								if (0==status)
								{
									/* allow for DA resolution */
									if (PXI6071E_AD_DA==ni_card->type)
									{
										*calibrate_amplitude=
											(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)4096;
									}
									else
									{
										*calibrate_amplitude=
											(f64)calibrate_voltage_DAs[0]*(f64)20/(f64)65536;
									}
									/* start the square waveform generation */
									da_channel=CALIBRATE_CHANNEL;
									status=WFM_Op(ni_card->device_number,
										/*number of DA channels*/1,&da_channel,
										calibrate_voltage_DAs,number_of_waveform_points,
										/*continous*/(u32)0,
										/*points/s*/(f64)((float)number_of_waveform_points*
										CALIBRATE_FREQUENCY));
									if (0==status)
									{
										card_number++;
										ni_card++;
										calibrate_amplitude++;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"calibration_callback.  WFM_Op failed");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  WFM_Scale failed");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calibration_callback.  set_NI_gain failed 2");
							}
						}
						if (return_code)
						{
							settling_step=0;
							calibrate_stage=3;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 3 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 3\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 3:
					{
						/* NI gain */
						gain=gains;
						offset=offsets;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						for (i=module_number_of_NI_CARDS;i>0;i--)
						{
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								hardware_buffer=(ni_card->hardware_buffer)+j;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
								sorted_signal_entry=sorted_signal;
								for (k=number_of_samples;k>0;k--)
								{
									temp=(float)(*hardware_buffer)-(*offset);
									if (temp<(float)0)
									{
										temp= -temp;
									}
									*sorted_signal_entry=temp;
									sorted_signal_entry++;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								qsort(sorted_signal,number_of_samples,sizeof(float),
									compare_float);
								temp=(float)0;
								for (k=(int)(switching_time*200*(float)number_of_samples);
									k<(int)number_of_samples;k++)
								{
									temp += sorted_signal[k];
								}
								temp /= (1-200*switching_time)*(float)number_of_samples;
#else /* defined (SWITCHING_TIME) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								temp=(float)0;
								for (k=number_of_samples;k>0;k--)
								{
									temp_2=(float)(*hardware_buffer)-(*offset);
									temp += temp_2*temp_2;
									hardware_buffer += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								temp /= number_of_samples;
								temp *= 2;
								temp=(float)sqrt((double)temp);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* *gain is already the programmable gain */
								*gain *= ((float)(*calibrate_amplitude)/temp);
								offset++;
								gain++;
							}
							calibrate_amplitude++;
							ni_card++;
						}
						/* set up for next stage */
						card_number=0;
						ni_card=module_NI_CARDS;
						return_code=1;
						while (return_code&&(card_number<module_number_of_NI_CARDS))
						{
							/* stop waveform generation */
							WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
							/* set the gain */
							if ((ni_card->unemap_2vx).gain_output_line_0)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)1);
							}
							if ((ni_card->unemap_2vx).gain_output_line_1)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)1);
							}
							/* set the output voltage to 0 */
							status=AO_Write(ni_card->device_number,CALIBRATE_CHANNEL,(i16)0);
							if (0==status)
							{
								card_number++;
								ni_card++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calibration_callback.  AO_Write failed");
								return_code=0;
							}
						}
						if (return_code)
						{
							settling_step=0;
							calibrate_stage=4;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 4 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 4\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 4:
					{
						/* offsets */
						calibrate_stage=0;
						module_buffer_full_callback=(Buffer_full_callback *)NULL;
						module_buffer_full_callback_data=(void *)NULL;
						ni_card=module_NI_CARDS;
						filter_frequency=filter_frequencies;
						channel_number=1;
						for (i=module_number_of_NI_CARDS;i>0;i--)
						{
							unemap_set_antialiasing_filter_frequency(channel_number,
								*filter_frequency);
							/* stop waveform generation */
							WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
							/* set the output voltage to 0 */
							AO_Write(ni_card->device_number,CALIBRATE_CHANNEL,(i16)0);
							ni_card++;
							filter_frequency++;
							channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
						return_code=1;
						if (module_calibration_end_callback)
						{
							if (REALLOCATE(calibration_channels,module_calibration_channels,
								int,number_of_settled_channels)&&REALLOCATE(calibration_offsets,
								module_calibration_offsets,float,number_of_settled_channels)&&
								REALLOCATE(calibration_gains,module_calibration_gains,float,
								number_of_settled_channels))
							{
								module_calibration_channels=calibration_channels;
								module_calibration_offsets=calibration_offsets;
								module_calibration_gains=calibration_gains;
								ni_card=module_NI_CARDS;
								channel_number=0;
								k=0;
								for (i=module_number_of_NI_CARDS;i>0;i--)
								{
									if ((PCI6031E_AD_DA==ni_card->type)||
										(PXI6031E_AD_DA==ni_card->type)||
										(PXI6071E_AD_DA==ni_card->type))
									{
										for (j=NUMBER_OF_CHANNELS_ON_NI_CARD;j>0;j--)
										{
											if (!(channel_failed[channel_number]&stage_mask))
											{
												module_calibration_offsets[k]=offsets[channel_number];
												module_calibration_gains[k]=gains[channel_number];
												module_calibration_channels[k]=channel_number+1;
												k++;
											}
											channel_number++;
										}
									}
									else
									{
										channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
									}
									ni_card++;
								}
								module_calibration_number_of_channels=k;
								(*module_calibration_end_callback)(
									module_calibration_number_of_channels,
									module_calibration_channels,module_calibration_offsets,
									module_calibration_gains,
									module_calibration_end_callback_data);
							}
							else
							{
								display_message(ERROR_MESSAGE,
"calibration_callback.  Could not reallocate calibration information.  %p %p %d",
									calibration_channels,calibration_offsets,calibration_gains);
								if (calibration_channels)
								{
									module_calibration_channels=calibration_channels;
									if (calibration_offsets)
									{
										module_calibration_offsets=calibration_offsets;
									}
								}
								(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
									(float *)NULL,module_calibration_end_callback_data);
							}
						}
						DEALLOCATE(calibrate_amplitudes);
						DEALLOCATE(calibrate_voltages);
						DEALLOCATE(calibrate_voltage_DAs);
						DEALLOCATE(filter_frequencies);
						DEALLOCATE(offsets);
						DEALLOCATE(previous_offsets);
						DEALLOCATE(gains);
						DEALLOCATE(channel_failed);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
						DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
					} break;
				}
				stage_flag <<= 1;
				stage_mask <<= 1;
				stage_mask |= 0x1;
			}
			else
			{
				/* not settled */
				if (settling_step<settling_step_max)
				{
					/* try again */
					settling_step++;
					return_code=unemap_start_sampling();
				}
				else
				{
					/* give up */
					display_message(ERROR_MESSAGE,
						"calibration_callback.  Failed to settle for stage %d\n",
						calibrate_stage);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"Failed to settle for stage %d\n",calibrate_stage);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
					return_code=0;
				}
			}
		}
		else
		{
			settling_step=1;
			return_code=unemap_start_sampling();
		}
		if (!return_code)
		{
			calibrate_stage=0;
			module_buffer_full_callback=(Buffer_full_callback *)NULL;
			module_buffer_full_callback_data=(void *)NULL;
			card_number=0;
			ni_card=module_NI_CARDS;
			filter_frequency=filter_frequencies;
			while (card_number<module_number_of_NI_CARDS)
			{
				/* stop waveform generation */
				WFM_Group_Control(ni_card->device_number,/*group*/1,/*Clear*/0);
				/* set the output voltage to 0 */
				AO_Write(ni_card->device_number,CALIBRATE_CHANNEL,(i16)0);
				channel_number=1+card_number*NUMBER_OF_CHANNELS_ON_NI_CARD;
				unemap_set_antialiasing_filter_frequency(channel_number,
					*filter_frequency);
				set_NI_gain(ni_card,(int)(ni_card->gain));
				if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
					(UnEmap_2V2==ni_card->unemap_hardware_version))
				{
					/* programmable gain */
					if ((ni_card->unemap_2vx).gain_output_line_0)
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							gain_0_output_line_UnEmap2vx,(i16)1);
					}
					if ((ni_card->unemap_2vx).gain_output_line_1)
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							gain_1_output_line_UnEmap2vx,(i16)1);
					}
				}
				card_number++;
				ni_card++;
				filter_frequency++;
			}
			DEALLOCATE(calibrate_amplitudes);
			DEALLOCATE(calibrate_voltages);
			DEALLOCATE(calibrate_voltage_DAs);
			DEALLOCATE(filter_frequencies);
			DEALLOCATE(offsets);
			DEALLOCATE(previous_offsets);
			DEALLOCATE(gains);
			DEALLOCATE(channel_failed);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
			DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			if (module_calibration_end_callback)
			{
				(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
					(float *)NULL,module_calibration_end_callback_data);
			}
		}
	}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"leave calibration_callback %d %d\n",calibrate_stage,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* calibration_callback */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static int start_NI_DA(i16 da_channel,int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 7 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the NI card with channels ((<channel_number>-1)
div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0
then the function applies to all NI cards.  Otherwise, the function fails.

Starts the <da_channel> for the specified NI card(s).  If <number_of_voltages>
is 0 then the signal is the zero signal.  If <number_of_voltages> is 1 then the
signal is constant at <*voltages>.  If <number_of_voltages> is >1 then the
signal is that in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int return_code;
	f64 da_frequency,*da_voltage_buffer;
	int card_number,first_DA_card,i,j;
	i16 status,stopped,*temp_da_buffer;
	u32 iterations,number_of_da_points,points;

	ENTER(start_NI_DA);
	return_code=0;
	/* check arguments */
	if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&((0==number_of_voltages)||
		((1==number_of_voltages)&&voltages)||((1<number_of_voltages)&&voltages&&
		(0<voltages_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			number_of_da_points=(u32)number_of_voltages;
			da_frequency=(f64)voltages_per_second;
			if (0<number_of_voltages)
			{
				if (ALLOCATE(da_voltage_buffer,f64,number_of_voltages)&&
					REALLOCATE(temp_da_buffer,da_buffer,i16,number_of_voltages))
				{
					da_buffer=temp_da_buffer;
					for (i=0;i<number_of_voltages;i++)
					{
						da_voltage_buffer[i]=(f64)voltages[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"start_NI_DA.  Could not allocate buffers %p %p",da_voltage_buffer,
						temp_da_buffer);
					DEALLOCATE(da_voltage_buffer);
					return_code=0;
				}
			}
			else
			{
				da_voltage_buffer=(f64 *)NULL;
			}
			if (return_code)
			{
				i=0;
				first_DA_card=1;
				while (i<module_number_of_NI_CARDS)
				{
					if (((-1==card_number)||(i==card_number))&&
						((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
					{
						/* check if waveform generation is already underway */
							/*???DB.  Just to try out stopping.  Might use for stimulation */
						status=WFM_Check((module_NI_CARDS[i]).device_number,da_channel,
							&stopped,&iterations,&points);
						if ((0!=status)||stopped)
						{
							/* stop waveform generation */
								/*???DB.  Not sure if it is possible to have different signals
									for AO0 and AO1 or if we need it */
							WFM_Group_Control((module_NI_CARDS[i]).device_number,/*group*/1,
								/*Clear*/0);
							if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
								(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
							{
								/* make sure that constant voltage */
								set_shift_register(module_NI_CARDS+i,
									Stim_Source_SHIFT_REGISTER_UnEmap2vx,0,1);
							}
							/* set the output voltage to 0 */
							AO_Write((module_NI_CARDS[i]).device_number,CALIBRATE_CHANNEL,
								(i16)0);
							/* configure the DA to bipolar +/-10 */
							status=AO_Configure((module_NI_CARDS[i]).device_number,da_channel,
								/*bipolar*/(i16)0,/*internal voltage reference*/(i16)0,
								/*reference voltage*/(f64)10,
								/*update DAC when written to*/(i16)0);
							if (0==status)
							{
								if (0==number_of_voltages)
								{
									/* set the output voltage to 0 */
									AO_Write((module_NI_CARDS[i]).device_number,da_channel,
										(i16)0);
								}
								else
								{
									if (first_DA_card)
									{
										status=WFM_Scale((module_NI_CARDS[i]).device_number,
											da_channel,number_of_da_points,/*gain*/(f64)1,
											da_voltage_buffer,da_buffer);
										/* set the desired voltages to the actual voltages used */
										if (PXI6071E_AD_DA==(module_NI_CARDS[i]).type)
										{
											for (j=0;j<number_of_voltages;j++)
											{
												voltages[j]=(float)da_buffer[j]*(float)20/(float)4096;
											}
										}
										else
										{
											for (j=0;j<number_of_voltages;j++)
											{
												voltages[j]=(float)da_buffer[j]*(float)20/(float)65536;
											}
										}
									}
									if (0==status)
									{
										if (1==number_of_voltages)
										{
											AO_Write((module_NI_CARDS[i]).device_number,da_channel,
												da_buffer[0]);
										}
										else
										{
											status=WFM_Op((module_NI_CARDS[i]).device_number,
												/*number of DA channels*/1,&da_channel,da_buffer,
												number_of_da_points,/*continous*/(u32)0,
												/*frequency*/da_frequency);
											if (0!=status)
											{
												display_message(ERROR_MESSAGE,
													"start_NI_DA.  WFM_Op failed");
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"start_NI_DA.  WFM_Scale failed");
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"start_NI_DA.  AO_Configure failed");
							}
							first_DA_card=0;
						}
					}
					i++;
				}
				DEALLOCATE(da_voltage_buffer);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"start_NI_DA.  Invalid configuration.  %d %p %d",module_configured,
				module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"start_NI_DA.  Invalid arguments %d %d %d %g %p",da_channel,
			channel_number,number_of_voltages,voltages_per_second,voltages);
	}
	LEAVE;

	return (return_code);
} /* start_NI_DA */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
int stop_NI_DA(i16 da_channel,int channel_number)
/*******************************************************************************
LAST MODIFIED : 7 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the NI card with channels ((<channel_number>-1)
div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0
then the function applies to all NI cards.  Otherwise, the function fails.

Stops the <da_channel> for the specified NI card(s).
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
	i16 status,stopped;
	u32 iterations,points;
#endif /* defined (NI_DAQ) */

	ENTER(stop_NI_DA);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			i=0;
			while (i<module_number_of_NI_CARDS)
			{
				if (((-1==card_number)||(i==card_number))&&
					((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
				{
					/* check if waveform generation is already underway */
					status=WFM_Check((module_NI_CARDS[i]).device_number,da_channel,
						&stopped,&iterations,&points);
					if ((0==status)&&!stopped)
					{
						/* stop waveform generation */
						WFM_Group_Control((module_NI_CARDS[i]).device_number,
							/*group*/1,/*Clear*/0);
						if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
							(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
						{
							/* make sure that constant voltage */
							set_shift_register(module_NI_CARDS+i,
								Stim_Source_SHIFT_REGISTER_UnEmap2vx,0,1);
						}
						/* set the output voltage to 0 */
						AO_Write((module_NI_CARDS[i]).device_number,da_channel,(i16)0);
					}
				}
				i++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"stop_NI_DA.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"stop_NI_DA.  Invalid argument %d",channel_number);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* stop_NI_DA */
#endif /* defined (NI_DAQ) */

/*
Global functions
----------------
*/
int unemap_configured(void)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Returns a non-zero if unemap is configured and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_configured);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured)
	{
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_configured */

int unemap_get_hardware_version(int *hardware_version)
/*******************************************************************************
LAST MODIFIED : 13 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_hardware_version);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if (hardware_version)
		{
			*hardware_version=module_NI_CARDS->unemap_hardware_version;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_hardware_version.  Missing hardware_version");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_hardware_version.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_hardware_version */

int unemap_shutdown(void)
/*******************************************************************************
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer.
???DB.  Not really anything to do with unemap hardware ?
==============================================================================*/
{
	int return_code;
#if defined (WINDOWS)
	HANDLE token;
	TOKEN_PRIVILEGES token_privileges;
#endif /* defined (WINDOWS) */

	ENTER(unemap_shutdown);
	return_code=0;
#if defined (WINDOWS)
	/* get a token for this process */
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&token))
	{
		/* get LUID for the shutdown privilege */
		LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,
			&(token_privileges.Privileges[0].Luid));
		/* one privilege to set */
		token_privileges.PrivilegeCount=1;
		token_privileges.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		/* get the shut down privilege for this process */
		AdjustTokenPrivileges(token,FALSE,&token_privileges,0,
			(PTOKEN_PRIVILEGES)NULL,0);
		/* cannot test the return value of AdjustTokenPrivileges */
		if (ERROR_SUCCESS==GetLastError())
		{
			/* shut down the system and force all applications to close */
			if (ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE,0))
			{
				return_code=1;
			}
		}
	}
#endif /* defined (WINDOWS) */
	LEAVE;

	return (return_code);
} /* unemap_shutdown */

int unemap_set_scrolling_channel(int channel_number)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int *temp;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_scrolling_channel);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((1<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD))
		{
			if (REALLOCATE(temp,module_scrolling_channel_numbers,int,
				module_number_of_scrolling_channels+1))
			{
				module_scrolling_channel_numbers=temp;
				module_scrolling_channel_numbers[module_number_of_scrolling_channels]=
					channel_number;
				module_number_of_scrolling_channels++;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"unemap_set_scrolling_channel.  Could not reallocate module_scrolling_channel_numbers");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"unemap_set_scrolling_channel.  Invalid channel_number %d (should be 1<=channel_number<=%d)",
				channel_number,module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD);
		}
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_scrolling_channel */

int unemap_clear_scrolling_channels(void)
/*******************************************************************************
LAST MODIFIED : 19 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code;

	ENTER(unemap_clear_scrolling_channels);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		DEALLOCATE(module_scrolling_channel_numbers);
		module_number_of_scrolling_channels=0;
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_clear_scrolling_channels */

int unemap_start_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
#if defined (NI_DAQ)
	i16 status,stopped;
	unsigned long ending_sample_number,number_of_samples;
	u32 retrieved;
#endif /* defined (NI_DAQ) */
#endif /* defined (OLD_CODE) */

	ENTER(unemap_start_scrolling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		return_code=1;
		module_scrolling_on=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_start_scrolling */

int unemap_calibrate(Calibration_end_callback *calibration_end_callback,
	void *calibration_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

When the calibration is completed <calibration_end_callback> is called with -
the number of channels calibrated, the channel numbers for the calibrated
channels, the offsets for the calibrated channels, the gains for the
calibrated channels and the <calibration_end_callback_data>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_calibrate);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured)
	{
		module_calibration_end_callback=calibration_end_callback;
		module_calibration_end_callback_data=calibration_end_callback_data;
		return_code=calibration_callback((void *)NULL);
	}
	else
	{
		if (calibration_end_callback)
		{
			(*calibration_end_callback)(0,(int *)NULL,(float *)NULL,(float *)NULL,
				calibration_end_callback_data);
		}
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_calibrate */

int unemap_get_isolate_record_mode(int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_isolate_record_mode);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((1<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD)&&isolate)
		{
			ni_card=module_NI_CARDS+(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
				(UnEmap_2V2==ni_card->unemap_hardware_version))
			{
				return_code=1;
				if (module_NI_CARDS[(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD].
					unemap_2vx.isolate_mode)
				{
					*isolate=1;
				}
				else
				{
					*isolate=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_isolate_record_mode.  Invalid argument(s).  %d %p",
				channel_number,isolate);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_isolate_record_mode.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_isolate_record_mode */

int unemap_set_powerup_antialiasing_filter_frequency(int channel_number)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Makes the current anti-aliasing filter frequency the power up value for the
group.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
	i16 status;
	u32 counter,counter_start;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_powerup_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	/* check configuration */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if ((-1==card_number)||(i==card_number))
				{
					/* set chip select low to program */
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								chip_select_line_UnEmap1vx,(i16)0);
						} break;
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							set_shift_register(module_NI_CARDS+i,FCS_SHIFT_REGISTER_UnEmap2vx,
								0,1);
						} break;
					}
					/* use timer to make sure that don't go too quickly */
					status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
						DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)2));
					/* set increment high so that setting is saved in nvRAM */
					DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
						filter_increment_output_line,(i16)1);
					status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
						DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set chip select high to stop programming */
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								chip_select_line_UnEmap1vx,(i16)1);
						} break;
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							set_shift_register(module_NI_CARDS+i,FCS_SHIFT_REGISTER_UnEmap2vx,
								1,1);
						} break;
					}
					/* set increment low */
					DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
						filter_increment_output_line,(i16)0);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"unemap_set_powerup_antialiasing_filter_frequency.  Invalid channel_number.  %d",
				channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_set_powerup_antialiasing_filter_frequency.  Invalid configuration.  %p %d",
			module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_powerup_antialiasing_filter_frequency */

int unemap_get_sample_range(long int *minimum_sample_value,
	long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The minimum possible sample value is assigned to <*minimum_sample_value> and the
maximum possible sample value is assigned to <*maximum_sample_value>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_sample_range);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (minimum_sample_value&&maximum_sample_value)
	{
		if (search_for_NI_cards()&&module_NI_CARDS)
		{
			if (PXI6071E_AD_DA==module_NI_CARDS->type)
			{
				*maximum_sample_value=(long int)2048;
				*minimum_sample_value=(long int)-2047;
			}
			else
			{
				*maximum_sample_value=(long int)32768;
				*minimum_sample_value=(long int)-32767;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_sample_range.  No hardware");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sample_range.  Invalid argument(s).  %p %p",
			minimum_sample_value,maximum_sample_value);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_sample_range */

int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float pre_filter_gain,post_filter_gain,temp;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_voltage_range);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((1<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD)&&minimum_voltage&&maximum_voltage)
		{
			if (unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain))
			{
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				if (PXI6071E_AD_DA==ni_card->type)
				{
					temp=(float)5;
				}
				else
				{
					temp=(float)10;
				}
				temp /= pre_filter_gain*post_filter_gain;
				*minimum_voltage= -temp;
				*maximum_voltage=temp;
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_voltage_range.  Invalid argument(s).  %d %p %p",
				channel_number,minimum_voltage,maximum_voltage);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_voltage_range.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_voltage_range */

int unemap_write_samples_acquired(int channel_number,FILE *file)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are written to <file>.  If
<channel_number> is 0 then the <samples> for all channels are written to <file>.
Otherwise the function fails.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int end,end2,i,j,k,start,step;
	short int sample,samples[NUMBER_OF_CHANNELS_ON_NI_CARD],*source;
	struct NI_card *ni_card;
	unsigned long maximum_number_of_samples;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_write_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_write_samples_acquired\n");
#endif /* defined (DEBUG) */
#if defined (NI_DAQ)
	/* check arguments */
	if (file&&(0<=channel_number)&&
		(channel_number<=module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			maximum_number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
				NUMBER_OF_CHANNELS_ON_NI_CARD;
			if (0==channel_number)
			{
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
					end2=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
					end2=0;
				}
				for (k=start;k<end;k++)
				{
					ni_card=module_NI_CARDS;
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						source=(ni_card->hardware_buffer)+(k*NUMBER_OF_CHANNELS_ON_NI_CARD);
						if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))
						{
							/* UnEmap_2V1 and UnEmap_2V2 invert */
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								sample=source[(ni_card->channel_reorder)[j]];
								if (sample<SHRT_MAX)
								{
									sample= -sample;
								}
								else
								{
									sample=SHRT_MIN;
								}
								samples[j]=sample;
							}
						}
						else
						{
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								samples[j]=source[(ni_card->channel_reorder)[j]];
							}
						}
						fwrite((char *)samples,sizeof(short int),
							NUMBER_OF_CHANNELS_ON_NI_CARD,file);
					}
				}
				if (end2>0)
				{
					for (k=0;k<end2;k++)
					{
						ni_card=module_NI_CARDS;
						for (i=0;i<module_number_of_NI_CARDS;i++)
						{
							source=(ni_card->hardware_buffer)+
								(k*NUMBER_OF_CHANNELS_ON_NI_CARD);
							if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
								(UnEmap_2V2==ni_card->unemap_hardware_version))
							{
								/* UnEmap_2V1 and UnEmap_2V2 invert */
								for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
								{
									sample=source[(ni_card->channel_reorder)[j]];
									if (sample<SHRT_MAX)
									{
										sample= -sample;
									}
									else
									{
										sample=SHRT_MIN;
									}
									samples[j]=sample;
								}
							}
							else
							{
								for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
								{
									samples[j]=source[(ni_card->channel_reorder)[j]];
								}
							}
							fwrite((char *)samples,sizeof(short int),
								NUMBER_OF_CHANNELS_ON_NI_CARD,file);
						}
					}
				}
			}
			else
			{
				start=module_starting_sample_number;
				if (module_starting_sample_number+module_sample_buffer_size>
					maximum_number_of_samples)
				{
					end=maximum_number_of_samples;
					end2=module_starting_sample_number+module_sample_buffer_size-
						maximum_number_of_samples;
				}
				else
				{
					end=module_starting_sample_number+module_sample_buffer_size;
					end2=0;
				}
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				source=(ni_card->hardware_buffer)+(start*NUMBER_OF_CHANNELS_ON_NI_CARD+
					(ni_card->channel_reorder)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD]);
				i=0;
				if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
					(UnEmap_2V2==ni_card->unemap_hardware_version))
				{
					/* UnEmap_2V1 and UnEmap_2V2 invert */
					for (k=start;k<end;k++)
					{
						sample= *source;
						if (sample<SHRT_MAX)
						{
							sample= -sample;
						}
						else
						{
							sample=SHRT_MIN;
						}
						samples[i]=sample;
						i++;
						if (i>=NUMBER_OF_CHANNELS_ON_NI_CARD)
						{
							fwrite((char *)samples,sizeof(short int),
								NUMBER_OF_CHANNELS_ON_NI_CARD,file);
							i=0;
						}
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
					if (end2>0)
					{
						source=(ni_card->hardware_buffer)+((ni_card->channel_reorder)[
							(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]);
						for (k=0;k<end2;k++)
						{
							sample= *source;
							if (sample<SHRT_MAX)
							{
								sample= -sample;
							}
							else
							{
								sample=SHRT_MIN;
							}
							samples[i]=sample;
							i++;
							if (i>=NUMBER_OF_CHANNELS_ON_NI_CARD)
							{
								fwrite((char *)samples,sizeof(short int),
									NUMBER_OF_CHANNELS_ON_NI_CARD,file);
								i=0;
							}
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
				}
				else
				{
					for (k=start;k<end;k++)
					{
						samples[i]= *source;
						i++;
						if (i>=NUMBER_OF_CHANNELS_ON_NI_CARD)
						{
							fwrite((char *)samples,sizeof(short int),
								NUMBER_OF_CHANNELS_ON_NI_CARD,file);
							i=0;
						}
						source += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
					if (end2>0)
					{
						source=(ni_card->hardware_buffer)+((ni_card->channel_reorder)[
							(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]);
						for (k=0;k<end2;k++)
						{
							samples[i]= *source;
							i++;
							if (i>=NUMBER_OF_CHANNELS_ON_NI_CARD)
							{
								fwrite((char *)samples,sizeof(short int),
									NUMBER_OF_CHANNELS_ON_NI_CARD,file);
								i=0;
							}
							source += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
				}
				if (i>0)
				{
					fwrite((char *)samples,sizeof(short int),i,file);
					i=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_write_samples_acquired.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_write_samples_acquired.  Invalid argument(s).  %p %d",file,
			channel_number);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_write_samples_acquired %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_write_samples_acquired */

int unemap_get_samples_acquired_background(int channel_number,
	Acquired_data_callback *callback,void *user_data);
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The function gets the samples specified by the <channel_number> and calls the
<callback> with the <channel_number>, the number of samples, the samples and the
<user_data>.

When the function returns, it is safe to call any of the other functions
(including unemap_start_sampling), but the <callback> may not have finished or
even been called yet.  This function allows data to be transferred in the
background in a client/server arrangement.
==============================================================================*/

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 9 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the gain before the band pass filter and the gain after the band pass
filter to the specified values.

For UnEmap_1V2, there is no gain before the filter (<pre_filter_gain> ignored).
For UnEmap_2V1 and UnEmap_2V2, the gain before the filter can be 1, 2, 4 or 8.

For UnEmap_1V2, the post filter gain can be 10, 20, 50, 100, 200, 500 or 1000
(fixed gain of 10)
For UnEmap_2V1 and UnEmap_2V2, the post filter gain can be 11, 22, 55, 110, 220,
550 or 1100 (fixed gain of 11).
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
	i16 gain;
	struct NI_card *ni_card;
	i16 GA0_setting,GA1_setting;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_gain);
	return_code=0;
#if defined (NI_DAQ)
	/* check configuration */
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			ni_card=module_NI_CARDS;
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if ((-1==card_number)||(i==card_number))
				{
					switch (ni_card->unemap_hardware_version)
					{
						case UnEmap_1V2:
						{
							if (post_filter_gain<15)
							{
								gain=1;
							}
							else
							{
								if (post_filter_gain<35)
								{
									gain=2;
								}
								else
								{
									if (post_filter_gain<75)
									{
										gain=5;
									}
									else
									{
										if (post_filter_gain<150)
										{
											gain=10;
										}
										else
										{
											if (post_filter_gain<350)
											{
												gain=20;
											}
											else
											{
												if (post_filter_gain<750)
												{
													gain=50;
												}
												else
												{
													gain=100;
												}
											}
										}
									}
								}
							}
							if (gain!=ni_card->gain)
							{
								set_NI_gain(ni_card,gain);
								ni_card->gain=gain;
							}
						} break;
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							if (pre_filter_gain<1.5)
							{
								GA0_setting=0;
								GA1_setting=0;
							}
							else
							{
								if (pre_filter_gain<3)
								{
									GA0_setting=1;
									GA1_setting=0;
								}
								else
								{
									if (pre_filter_gain<6)
									{
										GA0_setting=0;
										GA1_setting=1;
									}
									else
									{
										GA0_setting=1;
										GA1_setting=1;
									}
								}
							}
							if (post_filter_gain<16)
							{
								gain=1;
							}
							else
							{
								if (post_filter_gain<38)
								{
									gain=2;
								}
								else
								{
									if (post_filter_gain<82)
									{
										gain=5;
									}
									else
									{
										if (post_filter_gain<165)
										{
											gain=10;
										}
										else
										{
											if (post_filter_gain<385)
											{
												gain=20;
											}
											else
											{
												if (post_filter_gain<825)
												{
													gain=50;
												}
												else
												{
													gain=100;
												}
											}
										}
									}
								}
							}
							if (GA0_setting)
							{
								(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x1;
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)1);
							}
							else
							{
								(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x0;
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)0);
							}
							if (GA1_setting)
							{
								(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x1;
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)1);
							}
							else
							{
								(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x0;
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)0);
							}
							if (gain!=ni_card->gain)
							{
								set_NI_gain(ni_card,gain);
								ni_card->gain=gain;
							}
						} break;
					}
				}
				ni_card++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_gain.  Invalid channel_number.  %d",channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_gain.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_gain */

int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 5 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float temp;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_gain);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((1<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
			NUMBER_OF_CHANNELS_ON_NI_CARD)&&pre_filter_gain&&post_filter_gain)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			temp=(float)1;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
				{
					/* fixed gain of 10 */
					temp=(float)10*(float)(ni_card->gain);
				} break;
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					/* fixed gain of 11 */
					temp=(float)11*(float)(ni_card->gain);
				} break;
			}
			*post_filter_gain=temp;
			temp=(float)1;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					if ((ni_card->unemap_2vx).gain_output_line_0)
					{
						temp *= 2;
					}
					if ((ni_card->unemap_2vx).gain_output_line_1)
					{
						temp *= 4;
					}
				} break;
			}
			*pre_filter_gain=temp;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_gain.  Missing gain");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_gain */

int unemap_start_voltage_stimulating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 7 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_voltages> is 0 then the stimulating signal is the zero signal.  If
<number_of_voltages> is 1 then the stimulating signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the stimulating signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_voltage_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&((0==number_of_voltages)||
		((1==number_of_voltages)&&voltages)||((1<number_of_voltages)&&voltages&&
		(0<voltages_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			/* make sure that constant voltage */
			i=0;
			while (i<module_number_of_NI_CARDS)
			{
				if (((-1==card_number)||(i==card_number))&&
					((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
				{
					if ((UnEmap_2V1==module_NI_CARDS[i].unemap_hardware_version)||
						(UnEmap_2V2==module_NI_CARDS[i].unemap_hardware_version))
					{
						set_shift_register(module_NI_CARDS+i,
							Stim_Source_SHIFT_REGISTER_UnEmap2vx,0,1);
					}
				}
				i++;
			}
			return_code=start_NI_DA(STIMULATE_CHANNEL,channel_number,
				number_of_voltages,voltages_per_second,voltages);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_voltage_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_voltage_stimulating.  Invalid arguments %d %d %g %p",
			channel_number,number_of_voltages,voltages_per_second,voltages);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_voltages;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_start_voltage_stimulating %d %d %g %p %d\n",
			channel_number,number_of_voltages,voltages_per_second,voltages,
			return_code);
		maximum_voltages=10;
		if (number_of_voltages<maximum_voltages)
		{
			maximum_voltages=number_of_voltages;
		}
		for (i=0;i<maximum_voltages;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,voltages[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_voltage_stimulating */

int unemap_start_current_stimulating(int channel_number,int number_of_currents,
	float currents_per_second,float *currents)
/*******************************************************************************
LAST MODIFIED : 7 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_currents> is 0 then the stimulating signal is the zero signal.  If
<number_of_currents> is 1 then the stimulating signal is constant at the
<*currents>.  If <number_of_currents> is >1 then the stimulating signal is that
in <currents> at the specified number of <currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float current;
	int card_number,i;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_current_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&((0==number_of_currents)||
		((1==number_of_currents)&&currents)||((1<number_of_currents)&&currents&&
		(0<currents_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			if (0==channel_number)
			{
				card_number= -1;
			}
			else
			{
				card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
			/* change the currents into voltages */
			for (i=0;i<number_of_currents;i++)
			{
				current=currents[i];
				if ((float)1<current)
				{
					current=(float)1;
				}
				else
				{
					if ((float)-1>current)
					{
						current=(float)-1;
					}
				}
				currents[i]=current*MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx;
			}
			/* make sure that constant current */
			i=0;
			while (i<module_number_of_NI_CARDS)
			{
				if (((-1==card_number)||(i==card_number))&&
					((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
				{
					if ((UnEmap_2V1==module_NI_CARDS[i].unemap_hardware_version)||
						(UnEmap_2V2==module_NI_CARDS[i].unemap_hardware_version))
					{
						set_shift_register(module_NI_CARDS+i,
							Stim_Source_SHIFT_REGISTER_UnEmap2vx,1,1);
					}
				}
				i++;
			}
			return_code=start_NI_DA(STIMULATE_CHANNEL,channel_number,
				number_of_currents,currents_per_second,currents);
			/* set the desired currents to the actual currents used */
			for (i=0;i<number_of_currents;i++)
			{
				currents[i] /= MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_current_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_current_stimulating.  Invalid arguments %d %d %g %p",
			channel_number,number_of_currents,currents_per_second,currents);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_currents;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_start_current_stimulating %d %d %g %p %d\n",
			channel_number,number_of_currents,currents_per_second,currents,
			return_code);
		maximum_currents=10;
		if (number_of_currents<maximum_currents)
		{
			maximum_currents=number_of_currents;
		}
		for (i=0;i<maximum_currents;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,currents[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_current_stimulating */

int unemap_stop_stimulating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_stimulating);
#if defined (NI_DAQ)
	return_code=stop_NI_DA(STIMULATE_CHANNEL,channel_number);
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_stop_stimulating %d %d\n",channel_number,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_stimulating */

int unemap_set_channel_stimulating(int channel_number,int stimulating)
/*******************************************************************************
LAST MODIFIED : 10 September 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Zero <stimulating> means off.  Non-zero <stimulating> means on.  If
<channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i,j,k;
	struct NI_card *ni_card;
	unsigned char stimulate_mode;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_channel_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	if ((0<=channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		if (module_configured&&(ni_card=module_NI_CARDS)&&
			(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			if (0==channel_number)
			{
				for (i=module_number_of_NI_CARDS;i>0;i--)
				{
					if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
						(UnEmap_2V2==ni_card->unemap_hardware_version))
					{
						if ((ni_card->unemap_2vx).isolate_mode)
						{
							if ((stimulating&&relay_power_on_UnEmap2vx)||
								(!stimulating&&!relay_power_on_UnEmap2vx))
							{
								stimulate_mode=(unsigned char)0xff;
							}
							else
							{
								stimulate_mode=(unsigned char)0x0;
							}
							for (j=0;j<8;j++)
							{
								((ni_card->unemap_2vx).stimulate_channels)[j]=stimulate_mode;
							}
						}
						else
						{
							k=Stim_Base_SHIFT_REGISTER_UnEmap2vx;
							for (j=NUMBER_OF_CHANNELS_ON_NI_CARD-1;j>0;j--)
							{
								set_shift_register(ni_card,k,!stimulating,0);
								k++;
							}
							set_shift_register(ni_card,k,!stimulating,1);
						}
					}
					ni_card++;
				}
			}
			else
			{
				ni_card += (channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
				k=(ni_card->channel_reorder)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD];
				if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
					(UnEmap_2V2==ni_card->unemap_hardware_version))
				{
					if ((ni_card->unemap_2vx).isolate_mode)
					{
						i=k/8;
						stimulate_mode=(unsigned char)0x1;
						stimulate_mode <<= k%8;
						if ((stimulating&&relay_power_on_UnEmap2vx)||
							(!stimulating&&!relay_power_on_UnEmap2vx))
						{
							((ni_card->unemap_2vx).stimulate_channels)[i] |= stimulate_mode;
						}
						else
						{
							((ni_card->unemap_2vx).stimulate_channels)[i] &= ~stimulate_mode;
						}
					}
					else
					{
						k += Stim_Base_SHIFT_REGISTER_UnEmap2vx;
						set_shift_register(ni_card,k,!stimulating,1);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_channel_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_channel_stimulating.  Invalid channel_number");
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_set_channel_stimulating %d %d %d\n",
			channel_number,stimulating,return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_set_channel_stimulating */

int unemap_get_channel_stimulating(int channel_number,int *stimulating)
/*******************************************************************************
LAST MODIFIED : 10 September 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <stimulating> is set to 1 if <channel_number> is stimulating
and 0 otherwise.  Otherwise the function fails.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int byte_number,k;
	struct NI_card *ni_card;
	unsigned char bit_mask;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_channel_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	if ((0<channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&stimulating)
	{
		if (module_configured&&(ni_card=module_NI_CARDS)&&
			(0<module_number_of_NI_CARDS))
		{
			return_code=1;
			ni_card += (channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			k=Stim_Base_SHIFT_REGISTER_UnEmap2vx+(ni_card->channel_reorder)[
				(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD];
			byte_number=(k-1)/8;
			bit_mask=(unsigned char)0x1;
			bit_mask <<= (k-1)%8;
			if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
			{
				*stimulating=0;
			}
			else
			{
				*stimulating=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_channel_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_channel_stimulating.  Invalid argument(s).  %d %p",
			channel_number,stimulating);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_channel_stimulating */

int unemap_start_calibrating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts generating the calibration signal for the specified group.  If
<number_of_voltages> is 0 then the calibration signal is the zero signal.  If
<number_of_voltages> is 1 then the calibration signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the calibration signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_start_calibrating);
#if defined (NI_DAQ)
	return_code=start_NI_DA(CALIBRATE_CHANNEL,channel_number,
		number_of_voltages,voltages_per_second,voltages);
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_voltages;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_start_calibrating %d %d %g %p %d\n",
			channel_number,number_of_voltages,voltages_per_second,voltages,
			return_code);
		maximum_voltages=10;
		if (number_of_voltages<maximum_voltages)
		{
			maximum_voltages=number_of_voltages;
		}
		for (i=0;i<maximum_voltages;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,voltages[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_calibrating */

int unemap_stop_calibrating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_calibrating);
#if defined (NI_DAQ)
	return_code=stop_NI_DA(CALIBRATE_CHANNEL,channel_number);
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_stop_calibrating %d %d\n",channel_number,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_calibrating */

int unemap_get_power(int *on)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on then <*on> is set to 1, otherwise <*on> is set to 0.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_power);
	return_code=0;
#if defined (NI_DAQ)
	if (on)
	{
		if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
				(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
			{
				return_code=1;
				if (BattA_setting_UnEmap2vx)
				{
					*on=1;
				}
				else
				{
					*on=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_power.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_power.  Missing on");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_power */

int unemap_read_waveform_file(char *waveform_file_name,int *number_of_values,
	float *values_per_second,float **values,int *constant_voltage)
/*******************************************************************************
LAST MODIFIED : 20 May 1999

DESCRIPTION :
The function does not need the hardware to be configured.

A waveform suitable for stimulation is read from a file.  If
<*constant_voltage>, then the <*values> are voltages otherwise the <*values> are
proportions of the maximum current.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	FILE *waveform_file;
	float *value;
	int i;
#endif /* defined (NI_DAQ) */

	ENTER(read_waveform_file);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (waveform_file_name&&number_of_values&&values&&values_per_second&&
		constant_voltage)
	{
		if (waveform_file=fopen(waveform_file_name,"r"))
		{
			fscanf(waveform_file,"Number of ");
			if (1==fscanf(waveform_file,"voltages = %d ",
				number_of_values))
			{
				*constant_voltage=1;
				return_code=1;
			}
			else
			{
				if (1==fscanf(waveform_file,"currents = %d ",
					number_of_values))
				{
					*constant_voltage=0;
					return_code=1;
				}
			}
			if (return_code)
			{
				return_code=0;
				if (0< *number_of_values)
				{
					if (1==fscanf(waveform_file,"Frequency (pts/s) = %f ",
						values_per_second))
					{
						if (0< *values_per_second)
						{
							if (ALLOCATE(*values,float,*number_of_values))
							{
								if (*constant_voltage)
								{
									fscanf(waveform_file," Voltages ");
								}
								else
								{
									fscanf(waveform_file," Currents ");
								}
								i= *number_of_values;
								return_code=1;
								value= *values;
								while (return_code&&(i>0))
								{
									if (1==fscanf(waveform_file," %f ",value))
									{
										i--;
										value++;
									}
									else
									{
										return_code=0;
									}
								}
								if (0==return_code)
								{
									display_message(ERROR_MESSAGE,"Error reading value from %s",
										waveform_file_name);
									DEALLOCATE(*values);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"read_waveform_file.  Could not allocate memory for values");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid frequency (%g) from %s",
								*values_per_second,waveform_file_name);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Error reading frequency from %s",
							waveform_file_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Invalid number of values (%d) from %s",*number_of_values,
						waveform_file_name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Error reading number of values from %s",waveform_file_name);
			}
			fclose(waveform_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open waveform file %s",
				waveform_file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_waveform_file.  Invalid argument(s)");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* read_waveform_file */

int unemap_get_number_of_stimulators(int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_stimulators);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_stimulators)
	{
		return_code=1;
		if (search_for_NI_cards()&&module_NI_CARDS)
		{
			*number_of_stimulators=module_number_of_stimulators;
		}
		else
		{
			*number_of_stimulators=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_stimulators.  Missing number_of_stimulators");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_stimulators */

int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_channel_valid_for_stimulator);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<stimulator_number)&&
		(stimulator_number<=module_number_of_stimulators)&&(1<=channel_number)&&
		(channel_number<=module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)&&
		(module_stimulator_NI_CARD_indices[stimulator_number-1]==
		(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_channel_valid_for_stimulator */

/*
Diagnostic functions
--------------------
*/
int unemap_write_card_state(int channel_number,FILE *out_file)
/*******************************************************************************
LAST MODIFIED : 20 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Writes the current state of the signal conditioning card containing the
<channel_number> to the specified <out_file>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float filter_frequency,pre_filter_gain,post_filter_gain;
	int i;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_write_card_state);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&out_file)
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
				{
					fprintf(out_file,"cs = %d\n",chip_select_line_UnEmap1vx);
					fprintf(out_file,"u/d = %d\n",up_down_line_UnEmap1vx);
					fprintf(out_file,"inc = %d\n",filter_increment_output_line);
					fprintf(out_file,"led = %d\n",led_line_UnEmap1vx);
					fprintf(out_file,"id1 = %d\n",id_line_1_UnEmap1vx);
					fprintf(out_file,"id2 = %d\n",id_line_2_UnEmap1vx);
					fprintf(out_file,"id3 = %d\n",id_line_3_UnEmap1vx);
					unemap_get_antialiasing_filter_frequency(channel_number,
						&filter_frequency);
					fprintf(out_file,"filter = %g\n",filter_frequency);
					unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
					fprintf(out_file,"gain = %g\n",post_filter_gain/10.);
					fprintf(out_file,"input_mode = %d\n",ni_card->input_mode);
					fprintf(out_file,"polarity = %d\n",ni_card->polarity);
				} break;
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					fprintf(out_file,"BattGood = %d\n",battery_good_input_line_UnEmap2vx);
					fprintf(out_file,"Rstrobe = %d\n",
						shift_register_strobe_output_line_UnEmap2vx);
					fprintf(out_file,"GA1 = %d\n",gain_1_output_line_UnEmap2vx);
					fprintf(out_file,"Rdata = %d\n",
						shift_register_data_output_line_UnEmap2vx);
					fprintf(out_file,"FINC = %d\n",filter_increment_output_line);
					fprintf(out_file,"RCLK = %d\n",
						shift_register_clock_output_line_UnEmap2vx);
					fprintf(out_file,"GA0 = %d\n",gain_0_output_line_UnEmap2vx);
					unemap_get_antialiasing_filter_frequency(channel_number,
						&filter_frequency);
					fprintf(out_file,"filter = %g\n",filter_frequency);
					unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
					fprintf(out_file,"gain = %g\n",post_filter_gain/11.);
					fprintf(out_file,"input_mode = %d\n",ni_card->input_mode);
					fprintf(out_file,"polarity = %d\n",ni_card->polarity);
					fprintf(out_file,"GA0 setting = ");
					if ((ni_card->unemap_2vx).gain_output_line_0)
					{
						fprintf(out_file,"1\n");
					}
					else
					{
						fprintf(out_file,"0\n");
					}
					fprintf(out_file,"GA1 setting = ");
					if ((ni_card->unemap_2vx).gain_output_line_1)
					{
						fprintf(out_file,"1\n");
					}
					else
					{
						fprintf(out_file,"0\n");
					}
					fprintf(out_file,"shift registers = ");
					for (i=0;i<10;i++)
					{
						fprintf(out_file,"%02x",
							(unsigned short)((ni_card->unemap_2vx).shift_register)[i]);
					}
					fprintf(out_file,"\n");
					fprintf(out_file,"settling magnitude = %g\n",tol_settling);
					fprintf(out_file,"relay power on = %d\n",relay_power_on_UnEmap2vx);
					fprintf(out_file,"number of valid ni channels = %d\n",
						number_of_valid_ni_channels);
					if (UnEmap_2V1==ni_card->unemap_hardware_version)
					{
						fprintf(out_file,"BattA = %d\n",battery_A_line_UnEmap2v1);
					}
					else
					{
						fprintf(out_file,"Master = %d\n",master_line_UnEmap2v2);
					}
					fprintf(out_file,"BattA setting = %d\n",BattA_setting_UnEmap2vx);
					fprintf(out_file,"sampling interval = %d\n",
						ni_card->sampling_interval);
					fprintf(out_file,"max settling steps = %d\n",settling_step_max);
				} break;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_card_state.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_write_card_state.  Invalid argument(s).  %d %p",
			channel_number,out_file);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_write_card_state */

int unemap_toggle_shift_register(int channel_number,int register_number)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int byte_number;
	struct NI_card *ni_card;
	unsigned char bit_mask;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_toggle_shift_register);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0<channel_number)&&(channel_number<=module_number_of_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)&&(0<register_number)&&(register_number<=80))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
				(UnEmap_2V2==ni_card->unemap_hardware_version))
			{
				bit_mask=0x1;
				bit_mask <<= (register_number-1)%8;
				byte_number=(register_number-1)/8;
				if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
				{
					set_shift_register(ni_card,register_number,0,1);
				}
				else
				{
					set_shift_register(ni_card,register_number,1,1);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_toggle_shift_register.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_toggle_shift_register.  Invalid argument(s).  %d %d",
			channel_number,register_number);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_toggle_shift_register */
#endif /* defined (OLD_CODE) */
