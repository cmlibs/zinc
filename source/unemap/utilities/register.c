/*******************************************************************************
FILE : register.c

LAST MODIFIED : 27 July 2000

DESCRIPTION :
For setting and checking registers on second version of the signal conditioning
card.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#if defined (MOTIF)
#include <unistd.h>
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <conio.h>
#endif /* defined (WINDOWS) */
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include "general/debug.h"
#include "unemap/rig.h"
#include "unemap/unemap_hardware.h"
#include "user_interface/message.h"

/*
Compilation flags
-----------------
*/
/*#define CALIBRATE_SQUARE_WAVE 1*/

/* fileno is not ANSI */
extern int fileno(FILE *);

int unemap_get_card_state(int channel_number,int *battA_state,
	int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling,
	int *sampling_interval,int *settling_step_max);
/*******************************************************************************
LAST MODIFIED : 30 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/

int unemap_write_card_state(int channel_number,FILE *out_file);
/*******************************************************************************
LAST MODIFIED : 20 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Writes the current state of the signal conditioning card containing the
<channel_number> to the specified <out_file>.

Intended for diagnostic use only.
==============================================================================*/

int unemap_toggle_shift_register(int channel_number,int register_number);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/

/*
Macros
------
*/
#if defined (OLD_CODE)
#define ALLOCATE( result , type , number ) \
( result = ( type *) allocate( ( number ) * sizeof( type ) , __FILE__ , \
	__LINE__ ))

#define DEALLOCATE( ptr ) \
{ deallocate((char *) ptr , __FILE__ , __LINE__ ); ( ptr )=NULL;}

#define ENTER( function_name )

#define LEAVE

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) reallocate( (char *)( initial ) , \
	( number ) * sizeof( type ) , __FILE__ , __LINE__ ))
#endif /* defined (OLD_CODE) */

/*
Module constants
----------------
*/
/*#define FLOAT_FORMAT "%10g"*/
#define FLOAT_FORMAT "%10.3f"
#define MAXIMUM_NUMBER_OF_NI_CARDS 7
#define NUMBER_OF_CHANNELS_ON_NI_CARD 64
#if defined (OLD_CODE)
#define CALIBRATE_AMPLITUDE_FACTOR ((float)0.75)
#endif /* defined (OLD_CODE) */
#define CALIBRATE_AMPLITUDE_FACTOR ((float)0.9)
#define CALIBRATE_WAVEFORM_FREQUENCY ((float)100)

/*
Module types
------------
*/
#if defined (MOTIF)
struct Process_keyboard_data
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
For passing to the process_keyboard callback.
==============================================================================*/
{
	float sampling_frequency;
	int channel_number;
	short int *samples;
	unsigned sampling_delay;
	unsigned long number_of_channels,number_of_samples;
}; /* struct Process_keyboard_data */
#endif /* defined (MOTIF) */

/*
Module variables
----------------
*/
int max_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;

const float *calibrating_channel_gains=(float *)NULL,
	*calibrating_channel_offsets=(float *)NULL;
const int *calibrating_channel_numbers=(int *)NULL;
int calibrating=0,calibrating_number_of_channels;

#if defined (MOTIF)
XtAppContext application_context;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
char pause_for_error_option='n';

static void pause_for_error(void)
{
	ENTER(pause_for_error);
	if (('y'==pause_for_error_option)||('Y'==pause_for_error_option))
	{
#if defined (WINDOWS)
		printf("Press any key to continue\n");
		while (!kbhit());
		getch();
#endif /* defined (WINDOWS) */
	}
	LEAVE;
} /* pause_for_error */

int draw_histogram(FILE *file,int channel_number)
{
	int *histogram,i,j,max_histogram,number_of_channels,return_code;
	short int *buffer,max,min,*samples;
	unsigned long number_of_samples;

	ENTER(draw_histogram);
	return_code=0;
	/* check arguments */
	if (file&&unemap_get_number_of_channels(&number_of_channels)&&
		(0<channel_number)&&(channel_number<number_of_channels))
	{
		if (unemap_get_number_of_samples_acquired(&number_of_samples)&&
			(0<number_of_samples)&&ALLOCATE(samples,short int,number_of_samples))
		{
			if (unemap_get_samples_acquired(channel_number,samples))
			{
				buffer=samples;
				min= *buffer;
				max=min;
				for (i=(int)number_of_samples-1;i>0;i--)
				{
					buffer++;
					if (*buffer<min)
					{
						min= *buffer;
					}
					else
					{
						if (*buffer>max)
						{
							max= *buffer;
						}
					}
				}
				if (ALLOCATE(histogram,int,max-min+1))
				{
					for (i=0;i<max-min+1;i++)
					{
						histogram[i]=0;
					}
					buffer=samples;
					for (i=(int)number_of_samples;i>0;i--)
					{
						histogram[(*buffer)-min]++;
						buffer++;
					}
					max_histogram=histogram[0];
					for (i=1;i<max-min+1;i++)
					{
						if (histogram[i]>max_histogram)
						{
							max_histogram=histogram[i];
						}
					}
					for (i=0;i<max-min+1;i++)
					{
						fprintf(file,"%4d",min+i);
						for (j=(histogram[i]*72)/max_histogram;j>0;j--)
						{
							fprintf(file,"*");
						}
						for (j=72-(histogram[i]*72)/max_histogram;j>0;j--)
						{
							fprintf(file," ");
						}
						fprintf(file,"%4d\n",histogram[i]);
					}
					DEALLOCATE(histogram);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"draw_histogram.  Insufficient memory for histogram");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_histogram.  Could not retrieve samples");
			}
			DEALLOCATE(samples);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"draw_histogram.  Insufficient memory for samples");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_histogram.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* draw_histogram */

int register_write_signal_file(char *file_name,int channel_number)
{
	FILE *output_file;
	float sampling_frequency;
	int electrodes_in_row[8]={8,8,8,8,8,8,8,8},i,index,j,number_of_channels,
		return_code;
	short int *destination,*samples,*source;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;
	unsigned long number_of_samples;

	ENTER(register_write_signal_file);
	return_code=0;
	/* check arguments */
	if (file_name&&unemap_get_number_of_channels(&number_of_channels)&&
		(0<channel_number)&&(channel_number<number_of_channels))
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

static int compare_float(const void *float_1_void,const void *float_2_void)
{
	float float_1,float_2;
	int return_code;

	ENTER(compare_float);
	float_1= *((float *)float_1_void);
	float_2= *((float *)float_2_void);
	if (float_1<float_2)
	{
		return_code= -1;
	}
	else
	{
		if (float_1>float_2)
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

#if defined (WINDOWS)
static void sleep(unsigned seconds)
{
	Sleep((DWORD)seconds*(DWORD)1000);
}
#endif /* defined (WINDOWS) */

static void calibration_finished(const int number_of_channels,
	const int *channel_numbers,const float *channel_offsets,
	const float *channel_gains,void *dummy_user_data)
{
	ENTER(calibration_finished);
	USE_PARAMETER(dummy_user_data);
	calibrating_number_of_channels=number_of_channels;
	calibrating_channel_numbers=channel_numbers;
	calibrating_channel_offsets=channel_offsets;
	calibrating_channel_gains=channel_gains;
	calibrating=0;
	LEAVE;
} /* calibration_finished */

static int allow_to_settle(int test_channel,int *test_cards,int *channel_check,
	int phase_flag,FILE *report,int *number_of_settled_channels,
	int sampling_delay,short int *samples,float *mean,
	unsigned long number_of_samples,unsigned long number_of_channels,
	float tol_settling,int max_settling)
{
	float maximum,
		previous_mean[MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD],
		saturated,temp;
	int channel_number,i,j,k,l,return_code;
	long int maximum_sample_value,minimum_sample_value;
	short int *sample;

	return_code=1;
	/* wait for the high-pass (DC removal) to settle */
	printf("Settling\n");
	fprintf(report,"Settling\n");
	*number_of_settled_channels=0;
	channel_number=0;
	for (j=0;j<MAXIMUM_NUMBER_OF_NI_CARDS;j++)
	{
		if (test_cards[j])
		{
			if (0==test_channel)
			{
				for (i=0;i<max_channels;i++)
				{
					channel_check[channel_number] |= phase_flag;
					channel_number++;
				}
				*number_of_settled_channels +=
					NUMBER_OF_CHANNELS_ON_NI_CARD-max_channels;
				channel_number +=
					NUMBER_OF_CHANNELS_ON_NI_CARD-max_channels;
			}
			else
			{
				channel_check[channel_number+test_channel-1] |= phase_flag;
				*number_of_settled_channels += NUMBER_OF_CHANNELS_ON_NI_CARD-1;
				channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
		}
		else
		{
			*number_of_settled_channels += NUMBER_OF_CHANNELS_ON_NI_CARD;
			channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
		}
	}
	k=0;
	unemap_get_sample_range(&minimum_sample_value,&maximum_sample_value);
	saturated=(float)(-minimum_sample_value);
	do
	{
		unemap_start_sampling();
		sleep(4+sampling_delay);
		/*???debug */
		unemap_stop_sampling();
		unemap_get_samples_acquired(0,samples);
		channel_number=0;
		for (l=0;l<MAXIMUM_NUMBER_OF_NI_CARDS;l++)
		{
			if (test_cards[l])
			{
				for (i=0;i<max_channels;i++)
				{
					previous_mean[channel_number]=mean[channel_number];
					mean[channel_number]=(float)0;
					sample=samples+(l*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
					for (j=(int)number_of_samples;j>0;j--)
					{
						mean[channel_number] += (float)(*sample);
						sample += number_of_channels;
					}
					mean[channel_number] /= (float)number_of_samples;
					channel_number++;
				}
				channel_number +=
					NUMBER_OF_CHANNELS_ON_NI_CARD-max_channels;
			}
			else
			{
				channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
			}
		}
		if (k>0)
		{
			maximum=(float)0;
			channel_number=0;
			for (l=0;l<MAXIMUM_NUMBER_OF_NI_CARDS;l++)
			{
				if (test_cards[l])
				{
					for (i=0;i<max_channels;i++)
					{
						if (channel_check[channel_number]&phase_flag)
						{
							/* channel hasn't yet converged */
							temp=(float)fabs((double)(mean[channel_number]-
								previous_mean[channel_number]));
							if (((float)fabs((double)(previous_mean[channel_number]))<
								saturated)&&((float)fabs((double)(mean[channel_number]))<
								saturated)&&(temp<tol_settling))
							{
								channel_check[channel_number] &= ~phase_flag;
								(*number_of_settled_channels)++;
							}
							if (temp>maximum)
							{
								maximum=temp;
							}
						}
						channel_number++;
					}
					channel_number +=
						NUMBER_OF_CHANNELS_ON_NI_CARD-max_channels;
				}
				else
				{
					channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
				}
			}
			printf(" %g\n",maximum);
			fprintf(report," %g\n",maximum);
		}
		k++;
	} while ((*number_of_settled_channels<
		MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)&&
		(k<max_settling));
	if (*number_of_settled_channels<MAXIMUM_NUMBER_OF_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD)
	{
		printf("Failed to settle for:");
		for (i=0;i<MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
		{
			if (channel_check[i]&phase_flag)
			{
				printf(" %d",i+1);
			}
		}
		printf("\n");
		pause_for_error();
	}

	return (return_code);
} /* allow_to_settle */

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
	printf("(1) Change card number      ");
	printf("(2) Toggle GA0              \n");
	printf("(3) Toggle GA1              ");
	printf("(4) Toggle shift register   \n");
	printf("(5) Set filter frequency    ");
	printf("(6) Set calibrate mode      \n");
	printf("(7) Set record mode         ");
	printf("(8) Save                    \n");
	printf("(9) Measure noise           ");
	printf("(a) Auto test               \n");
	printf("(b) Toggle BattA            ");
	printf("(c) Stimulate               \n");
	printf("(d) Calibrate               ");
	printf("(e) Calibrate DA            \n");
	printf("(f) Set power up filter f   ");
	printf("(g) Set NI gain             \n");
	printf("(h) Shutdown SCU PC         \n");
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
	char *file_name,*hardware_directory,option,phase_option;
	double two_pi;
	FILE *report,*settings;
	float a,b,calibrate_amplitude_1,calibrate_amplitude_2,
		calibrate_voltage_1[5000],calibrate_voltage_2[5000],db_per_decade,
		db_reduction,decay_constant,filter_frequency,gain,maximum_voltage,
		mean[MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD],mean_x,
		mean_y,minimum_voltage,post_filter_gain,pre_filter_gain,r,rms,
		rms_save[MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD],
		signal_rms,sorted_mean[MAXIMUM_NUMBER_OF_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD],sum_x,sum_xx,sum_xy,sum_y,sum_yy,
		switching_time,temp,tol_calibrate_gain,tol_correlation_coefficient,tol_db1,
		tol_db2,tol_decay_constant,tol_gain,tol_isolation,tol_offset,
		tol_offset_spread,tol_rms,tol_settling,tol_signal_form,tol_signal_value,x,y;
	int battA_state,battGood_state,card_used[MAXIMUM_NUMBER_OF_NI_CARDS],
		channel_check[MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD],
		channel_number_1,channel_number_2,channel_number_3,count,filter_taps,first,
		GA0_state,GA1_state,i,input_mode,j,k,l,max_settling,NI_gain,
		number_of_settled_channels,number_of_test_cards,number_of_waveform_points,
		phase_finished,phase_flag,phase0_flag,polarity,sampling_interval,
		settling_step_max,shift_register_number,
		temp_channel_check[MAXIMUM_NUMBER_OF_NI_CARDS*
		NUMBER_OF_CHANNELS_ON_NI_CARD],temp_c_number,test_channel,tested_card,
		tested_cards[MAXIMUM_NUMBER_OF_NI_CARDS],tester_card_1,tester_card_2,
		tester_card_3,total_checks;
	long int maximum_sample_value,minimum_sample_value;
	short int *sample;
	static int calibrate_DA_on[MAXIMUM_NUMBER_OF_NI_CARDS],first_call=1,
		stimulate_DA_on[MAXIMUM_NUMBER_OF_NI_CARDS];
	unsigned char shift_registers[10];
#if defined (MOTIF)
	float sampling_frequency;
	int channel_number;
	short int *samples;
	struct Process_keyboard_data *process_keyboard_data;
	unsigned sampling_delay;
	unsigned long number_of_channels,number_of_samples;
#endif /* defined (MOTIF) */
#if defined (CALIBRATE_SQUARE_WAVE)
	float *sorted_signal;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */

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
	if (first_call)
	{
		first_call=0;
		unemap_stop_stimulating(0);
		for (i=0;i<MAXIMUM_NUMBER_OF_NI_CARDS;i++)
		{
			stimulate_DA_on[i]=0;
		}
		unemap_stop_calibrating(0);
		for (i=0;i<MAXIMUM_NUMBER_OF_NI_CARDS;i++)
		{
			calibrate_DA_on[i]=0;
		}
	}
	scanf("%c",&option);
	if (isalnum(option))
	{
		if ('0'!=option)
		{
			switch (option)
			{
				case '1':
				{
					printf("Card number ? ");
					scanf("%d",&channel_number);
					if (channel_number<1)
					{
						channel_number=1;
					}
					else
					{
						channel_number=(channel_number-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
						if (channel_number>(int)number_of_channels)
						{
							channel_number=(int)number_of_channels;
						}
					}
				} break;
				case '2':
				{
					if (unemap_get_gain(channel_number,&pre_filter_gain,
						&post_filter_gain))
					{
						if (1==pre_filter_gain)
						{
							pre_filter_gain=(float)2;
						}
						else
						{
							if (2==pre_filter_gain)
							{
								pre_filter_gain=(float)1;
							}
							else
							{
								if (4==pre_filter_gain)
								{
									pre_filter_gain=(float)8;
								}
								else
								{
									if (8==pre_filter_gain)
									{
										pre_filter_gain=(float)4;
									}
									else
									{
										printf("Unknown pre_filter_gain %g\n",pre_filter_gain);
									}
								}
							}
						}
						unemap_set_gain(channel_number,pre_filter_gain,post_filter_gain);
					}
					else
					{
						printf("Could not get gain\n");
					}
				} break;
				case '3':
				{
					if (unemap_get_gain(channel_number,&pre_filter_gain,
						&post_filter_gain))
					{
						if (1==pre_filter_gain)
						{
							pre_filter_gain=(float)4;
						}
						else
						{
							if (2==pre_filter_gain)
							{
								pre_filter_gain=(float)8;
							}
							else
							{
								if (4==pre_filter_gain)
								{
									pre_filter_gain=(float)1;
								}
								else
								{
									if (8==pre_filter_gain)
									{
										pre_filter_gain=(float)2;
									}
									else
									{
										printf("Unknown pre_filter_gain %g\n",pre_filter_gain);
									}
								}
							}
						}
						unemap_set_gain(channel_number,pre_filter_gain,post_filter_gain);
					}
					else
					{
						printf("Could not get gain\n");
					}
				} break;
				case '4':
				{
					printf("Shift register number ? ");
					scanf("%d",&shift_register_number);
					if ((0<shift_register_number)&&(shift_register_number<=80))
					{
						unemap_toggle_shift_register(channel_number,shift_register_number);
					}
					else
					{
						printf(">>>Invalid shift register\n");
					}
				} break;
				case '5':
				{
					/* set filter frequency */
					printf("Frequency (Hz) ? ");
					scanf("%f",&filter_frequency);
					unemap_set_antialiasing_filter_frequency(channel_number,
						filter_frequency);
				} break;
				case '6':
				{
					/* set isolate mode */
					unemap_set_isolate_record_mode(channel_number,1);
				} break;
				case '7':
				{
					/* set record mode */
					unemap_set_isolate_record_mode(channel_number,0);
				} break;
				case '8':
				{
					FILE *digital_io_lines;

					if (hardware_directory=getenv("UNEMAP_HARDWARE"))
					{
						if (ALLOCATE(file_name,char,strlen(hardware_directory)+
							strlen("digital.txt")+2))
						{
							strcpy(file_name,hardware_directory);
							if ('/'!=file_name[strlen(file_name)-1])
							{
								strcat(file_name,"/");
							}
						}
						else
						{
							printf(">>>Could not allocate file_name\n");
						}
					}
					else
					{
						printf(">>>Using current directory for digital.txt\n");
						if (ALLOCATE(file_name,char,strlen("digital.txt")+1))
						{
							file_name[0]='\0';
						}
						else
						{
							printf(">>>Could not allocate file_name\n");
						}
					}
					if (file_name)
					{
						strcat(file_name,"digital.txt");
						if (digital_io_lines=fopen(file_name,"w"))
						{
							unemap_write_card_state(channel_number,digital_io_lines);
							fclose(digital_io_lines);
						}
						else
						{
							printf(">>>Could not open %s\n",file_name);
						}
						DEALLOCATE(file_name);
					}
				} break;
				case '9':
				{
					/* measure noise */
					pause_for_error_option='y';
					unemap_start_sampling();
					sleep(sampling_delay);
					unemap_stop_sampling();
					register_write_signal_file("noise.sig",channel_number);
					if (unemap_get_samples_acquired(0,samples))
					{
						printf("means:\n");
						for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
						{
							mean[i]=(float)0;
							sample=samples+(channel_number-1+i);
							for (j=(int)number_of_samples;j>0;j--)
							{
								mean[i] += (float)(*sample);
								sample += number_of_channels;
							}
							mean[i] /= (float)number_of_samples;
							printf("%2d " FLOAT_FORMAT,i+1,mean[i]);
							if (0==(i+1)%4)
							{
								printf("\n");
							}
							else
							{
								printf(" *  ");
							}
						}
						pause_for_error();
						printf("rms:\n");
						for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
						{
							rms=(float)0;
							sample=samples+(channel_number-1+i);
							for (j=(int)number_of_samples;j>0;j--)
							{
								temp=(float)(*sample)-mean[i];
								rms += temp*temp;
								sample += number_of_channels;
							}
							rms /= (float)number_of_samples;
							rms=(float)sqrt((double)rms);
							printf("%2d " FLOAT_FORMAT,i+1,rms);
							if (0==(i+1)%4)
							{
								printf("\n");
							}
							else
							{
								printf(" *  ");
							}
						}
						pause_for_error();
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"main.  measure noise.  Could not retrieve samples");
					}
				} break;
				case 'a':
				{
					/* auto-test */
					unemap_set_power(1);
					unemap_stop_stimulating(0);
					unemap_set_channel_stimulating(0,0);
					unemap_set_gain(0,(float)1,(float)1);
					unemap_set_antialiasing_filter_frequency(0,(float)10000);
					unemap_set_isolate_record_mode(0,0);
#if defined (CALIBRATE_SQUARE_WAVE)
					number_of_waveform_points=2;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
					number_of_waveform_points=500;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
					for (i=0;i<MAXIMUM_NUMBER_OF_NI_CARDS;i++)
					{
						tested_cards[i]=0;
					}
					pause_for_error_option='y';
					number_of_test_cards=
						(int)number_of_channels/NUMBER_OF_CHANNELS_ON_NI_CARD;
					if (number_of_test_cards>MAXIMUM_NUMBER_OF_NI_CARDS)
					{
						number_of_test_cards=MAXIMUM_NUMBER_OF_NI_CARDS;
					}
					if (number_of_test_cards>=4)
					{
						for (i=0;i<number_of_test_cards;i++)
						{
							card_used[i]=0;
						}
						printf("Card to be tested (1-%d) or all (0) or first n (-n) ? ",
							number_of_test_cards);
						scanf("%d",&tested_card);
						if (tested_card<0)
						{
							tested_card= -tested_card;
							if (tested_card<number_of_test_cards)
							{
								number_of_test_cards=tested_card;
								if (number_of_test_cards<4)
								{
									number_of_test_cards=4;
								}
							}
							tested_card=0;
						}
						else
						{
							if (tested_card>number_of_test_cards)
							{
								tested_card=number_of_test_cards;
							}
						}
						if (0!=tested_card)
						{
							tested_cards[tested_card-1]=1;
							card_used[tested_card-1]=1;
							printf("Tester card 1 (1-%d) ? ",number_of_test_cards);
							scanf("%d",&tester_card_1);
							if (tester_card_1<1)
							{
								tester_card_1=1;
							}
							else
							{
								if (tester_card_1>number_of_test_cards)
								{
									tester_card_1=number_of_test_cards;
								}
							}
							if (card_used[tester_card_1-1])
							{
								i=0;
								while (card_used[i])
								{
									i++;
								}
								printf("Card %d already in use.  Using %d for tester card 1\n",
									tester_card_1,i+1);
								tester_card_1=i+1;
							}
							card_used[tester_card_1-1]=1;
							printf("Tester card 2 (1-%d) ? ",number_of_test_cards);
							scanf("%d",&tester_card_2);
							if (tester_card_2<1)
							{
								tester_card_2=1;
							}
							else
							{
								if (tester_card_2>number_of_test_cards)
								{
									tester_card_2=number_of_test_cards;
								}
							}
							if (card_used[tester_card_2-1])
							{
								i=0;
								while (card_used[i])
								{
									i++;
								}
								printf("Card %d already in use.  Using %d for tester card 2\n",
									tester_card_2,i+1);
								tester_card_2=i+1;
							}
							card_used[tester_card_2-1]=1;
							printf("Tester card 3 (1-%d) ? ",number_of_test_cards);
							scanf("%d",&tester_card_3);
							if (tester_card_3<1)
							{
								tester_card_3=1;
							}
							else
							{
								if (tester_card_3>number_of_test_cards)
								{
									tester_card_3=number_of_test_cards;
								}
							}
							if (card_used[tester_card_3-1])
							{
								i=0;
								while (card_used[i])
								{
									i++;
								}
								printf("Card %d already in use.  Using %d for tester card 3\n",
									tester_card_3,i+1);
								tester_card_3=i+1;
							}
							card_used[tester_card_3-1]=1;
						}
						if (report=fopen("report.txt","w"))
						{
							fprintf(report,"Card to be tested = %d\n",tested_card);
							if (0!=tested_card)
							{
								fprintf(report,"Tester card 1 = %d\n",tester_card_1);
								fprintf(report,"Tester card 2 = %d\n",tester_card_2);
								fprintf(report,"Tester card 3 = %d\n",tester_card_3);
							}
							fprintf(report,"\n");
							fprintf(report,"number_of_samples = %lu\n",number_of_samples);
							fprintf(report,"sampling_frequency = %g\n",
								sampling_frequency);
							fprintf(report,"\n");
							if (0!=tested_card)
							{
								fprintf(report,"digital.txt\n");
								unemap_write_card_state(
									(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,report);
							}
							max_settling=20;
							tol_settling=(float)1;
							tol_offset=(float)20;
							tol_offset_spread=(float)15;
							tol_rms=(float)1.4;
							/* there is a small signal for isolation (coming through WCT?)
								whose size is dependent on the size of the test signal and on
								the phase difference between the signals on the even and odd
								channels.  Needs to be bigger now because the test signal is
								bigger and the even and odd signals are in phase */
							tol_isolation=(float)2.1*CALIBRATE_AMPLITUDE_FACTOR;
							tol_signal_value=(float)0.02; /* proportion */
							tol_signal_form=(float)10;
							switching_time=(float)0.0003; /* seconds */
							/* 330k resistor, 10u capacitor gives a decay constant of
								1/RC=0.3030, but don't get this */
							decay_constant=(float)-0.27;
							tol_decay_constant=(float)0.04;
							tol_correlation_coefficient=(float)0.01;
							db_per_decade=(float)100;
							tol_db1=(float)0.2;
							tol_db2=(float)0.15; /* proportion */
#if defined (OLD_CODE)
							tol_gain=(float)0.005; /* proportion */
#endif /* defined (OLD_CODE) */
							tol_gain=(float)0.01; /* proportion */
							tol_calibrate_gain=(float)0.05; /* proportion */
							if (hardware_directory=getenv("UNEMAP_HARDWARE"))
							{
								if (ALLOCATE(file_name,char,strlen(hardware_directory)+
									strlen("settings.txt")+2))
								{
									strcpy(file_name,hardware_directory);
									if ('/'!=file_name[strlen(file_name)-1])
									{
										strcat(file_name,"/");
									}
								}
								else
								{
									printf(">>>Could not allocate file_name\n");
								}
							}
							else
							{
								printf("Using current directory for settings.txt\n");
								if (ALLOCATE(file_name,char,strlen("settings.txt")+1))
								{
									file_name[0]='\0';
								}
								else
								{
									printf(">>>Could not allocate file_name\n");
								}
							}
							if (file_name)
							{
								strcat(file_name,"settings.txt");
								if (settings=fopen(file_name,"r"))
								{
									fscanf(settings," max_settling = %d ",&max_settling);
									fscanf(settings," tol_settling = %f ",&tol_settling);
									fscanf(settings," tol_offset = %f ",&tol_offset);
									fscanf(settings," tol_offset_spread = %f ",
										&tol_offset_spread);
									fscanf(settings," tol_rms = %f ",&tol_rms);
									fscanf(settings," tol_signal_value = %f ",
										&tol_signal_value);
									fscanf(settings," tol_signal_form = %f ",&tol_signal_form);
									fscanf(settings," switching_time = %f ",&switching_time);
									fscanf(settings," decay_constant = %f ",&decay_constant);
									fscanf(settings," tol_decay_constant = %f ",
										&tol_decay_constant);
									fscanf(settings," tol_correlation_coefficient = %f ",
										&tol_correlation_coefficient);
									fscanf(settings," db_per_decade = %f ",&db_per_decade);
									fscanf(settings," tol_db1 = %f ",&tol_db1);
									fscanf(settings," tol_db2 = %f ",&tol_db2);
									fscanf(settings," tol_gain = %f ",&tol_gain);
									fscanf(settings," tol_calibrate_gain = %f ",
										&tol_calibrate_gain);
									fscanf(settings," tol_isolation = %f ",&tol_isolation);
									fclose(settings);
									fprintf(report,"From %s\n",file_name);
								}
								else
								{
									fprintf(report,"No %s\n",file_name);
								}
								DEALLOCATE(file_name);
							}
							fprintf(report,"max_settling=%d\n",max_settling);
							fprintf(report,"tol_settling=%g\n",tol_settling);
							fprintf(report,"tol_offset=%g\n",tol_offset);
							fprintf(report,"tol_offset_spread=%g\n",tol_offset_spread);
							fprintf(report,"tol_rms=%g\n",tol_rms);
							fprintf(report,"tol_signal_value=%g\n",tol_signal_value);
							fprintf(report,"tol_signal_form=%g\n",tol_signal_form);
							fprintf(report,"switching_time=%g\n",switching_time);
							fprintf(report,"decay_constant=%g\n",decay_constant);
							fprintf(report,"tol_decay_constant=%g\n",tol_decay_constant);
							fprintf(report,"tol_correlation_coefficient=%g\n",
								tol_correlation_coefficient);
							fprintf(report,"db_per_decade=%g\n",db_per_decade);
							fprintf(report,"tol_db1=%g\n",tol_db1);
							fprintf(report,"tol_db2=%g\n",tol_db2);
							fprintf(report,"tol_gain=%g\n",tol_gain);
							fprintf(report,"tol_calibrate_gain=%g\n",tol_calibrate_gain);
							fprintf(report,"tol_isolation=%g\n",tol_isolation);
							fprintf(report,"\n");
							total_checks=0;
							unemap_set_power(1);
							unemap_set_isolate_record_mode(0,0);
							printf("Test channel (1-64) or all (0) or 1-32 (-1) ? ");
							scanf("%d",&test_channel);
							if ((1<=test_channel)&&
								(test_channel<=NUMBER_OF_CHANNELS_ON_NI_CARD))
							{
								fprintf(report,"Testing channel %d\n",test_channel);
								max_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;
							}
							else
							{
								if (-1==test_channel)
								{
									fprintf(report,"Testing channels 1-32 (motherboard)\n");
									test_channel=0;
									max_channels=NUMBER_OF_CHANNELS_ON_NI_CARD/2;
								}
								else
								{
									fprintf(report,"Testing channels 1-64\n");
									test_channel=0;
									max_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
							}
							printf("Pause for errors ? (y/n) ");
							do
							{
								scanf("%c",&pause_for_error_option);
							} while (!isalnum(pause_for_error_option));
							for (i=number_of_test_cards*NUMBER_OF_CHANNELS_ON_NI_CARD-1;
								i>=0;i--)
							{
								channel_check[i]=0x0;
								temp_channel_check[i]=0x0;
							}
							phase0_flag=0x1;
							phase_flag=phase0_flag<<1;
							printf("Phase 1 : Testing noise and offset ? (y/n/a/q) ");
							do
							{
								scanf("%c",&phase_option);
							} while (!isalnum(phase_option));
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 1 : Testing noise and offset\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_2)
												{
													tester_card_1=i+1;
												}
												else
												{
													tester_card_2=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
"Even channels of TESTER_1 (%d) to stimulate (output 0).  Odd channels to record\n",
										tester_card_1);
									fprintf(report,
"Odd channels of TESTER_2 (%d) to stimulate (output 0).  Even channels to record\n",
										tester_card_2);
									/* ground all inputs of TEST (card 1) :
										- set all channels of TEST to record
										- set even channels of GROUND (card 2) to stimulate and
											output zero.  Set odd channels to record
										- set odd channels of SIGNAL (card 3) to stimulate and
											output zero.  Set even channels to record */
									for (i=1;i<=max_channels;i++)
									{
										for (j=0;j<number_of_test_cards;j++)
										{
											if (tested_cards[j])
											{
												unemap_set_channel_stimulating(
													j*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
									}
									channel_number_1=
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									channel_number_2=
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									if (unemap_load_voltage_stimulating(1,&channel_number_1,
										(int)0,(float)0,(float *)NULL)&&
										unemap_load_voltage_stimulating(1,&channel_number_2,(int)0,
										(float)0,(float *)NULL)&&unemap_start_stimulating())
#if defined (OLD_CODE)
									if (unemap_start_voltage_stimulating(
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,(int)0,
										(float)0,(float *)NULL)&&unemap_start_voltage_stimulating(
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,(int)0,
										(float)0,(float *)NULL))
#endif /* defined (OLD_CODE) */
									{
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											printf("Failed to settle for:");
											for (i=0;i<number_of_test_cards*
												NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
											{
												if (temp_channel_check[i]&phase_flag)
												{
													printf(" %d",i+1);
													channel_check[i] |= phase_flag;
												}
											}
											printf("\n");
											pause_for_error();
										}
										if (0==test_channel)
										{
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													first=0;
													number_of_settled_channels=0;
													for (i=0;i<max_channels;i++)
													{
														if (!(temp_channel_check[temp_c_number+i]&
															phase_flag))
														{
															sorted_mean[number_of_settled_channels]=
																mean[temp_c_number+i];
															number_of_settled_channels++;
														}
													}
													if (0<number_of_settled_channels)
													{
														fprintf(report,"Test card %d\n",k+1);
														/* check offsets */
														fprintf(report,"Offsets:\n");
														qsort(sorted_mean,number_of_settled_channels,
															sizeof(float),compare_float);
														for (i=0;i<max_channels;i++)
														{
															fprintf(report,"%2d",temp_c_number+i+1);
															if (temp_channel_check[temp_c_number+i]&
																phase_flag)
															{
																channel_check[temp_c_number+i] |= phase_flag;
																fprintf(report," <<< CHECK settling >>>");
															}
															else
															{
																fprintf(report," " FLOAT_FORMAT,
																	mean[temp_c_number+i]);
																if (!((float)fabs((double)(mean[temp_c_number+
																	i]-sorted_mean[number_of_settled_channels/
																	2]))<tol_offset_spread))
																{
																	channel_check[temp_c_number+i] |=
																		phase_flag;
																	fprintf(report,
																		" <<< CHECK offset spread too large >>>");
																	total_checks++;
																	if (0==first)
																	{
																		printf("Offset spread too large for :\n");
																	}
																	first++;
																	printf("%2d " FLOAT_FORMAT,temp_c_number+i+1,
																		mean[temp_c_number+i]);
																	if (0==first%4)
																	{
																		printf("\n");
																	}
																	else
																	{
																		printf(" *  ");
																	}
																}
															}
															fprintf(report,"\n");
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														fprintf(report,"Offset median %g",
															sorted_mean[number_of_settled_channels/2]);
														if (!((float)fabs((double)(sorted_mean[
															number_of_settled_channels/2]))<tol_offset))
														{
															fprintf(report," <<< CHECK too large >>>");
															total_checks++;
															printf("Offset median %g is too large\n",
																sorted_mean[number_of_settled_channels/2]);
															first++;
														}
														fprintf(report,"\n");
														if (0==first)
														{
															printf("Offsets OK\n");
															fprintf(report,"Offsets OK\n");
														}
														else
														{
															pause_for_error();
														}
														/* check rms */
														fprintf(report,"RMS:\n");
														first=0;
														for (i=0;i<max_channels;i++)
														{
															fprintf(report,"%2d",temp_c_number+i+1);
															if (temp_channel_check[temp_c_number+i]&
																phase_flag)
															{
																channel_check[temp_c_number+i] |= phase_flag;
																fprintf(report," <<< CHECK settling >>>");
															}
															else
															{
																rms=(float)0;
																sample=samples+
																	(k*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
																for (j=(int)number_of_samples;j>0;j--)
																{
																	temp=(float)(*sample)-mean[temp_c_number+i];
																	rms += temp*temp;
																	sample += number_of_channels;
																}
																rms /= (float)number_of_samples;
																rms=(float)sqrt((double)rms);
																fprintf(report," " FLOAT_FORMAT,rms);
																if (!((float)fabs((double)rms)<tol_rms))
																{
																	channel_check[temp_c_number+i] |=
																		phase_flag;
																	fprintf(report," <<< CHECK too large >>>");
																	total_checks++;
																	if (0==first)
																	{
																		printf("RMS too large for :\n");
																	}
																	first++;
																	printf("%2d " FLOAT_FORMAT,temp_c_number+i+1,
																		rms);
																	if (0==first%4)
																	{
																		printf("\n");
																	}
																	else
																	{
																		printf(" *  ");
																	}
																}
															}
															fprintf(report,"\n");
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														if (0==first)
														{
															printf("Noise OK\n");
															fprintf(report,"Noise OK\n");
														}
														else
														{
															pause_for_error();
														}
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										else
										{
											i=test_channel-1;
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													if (temp_channel_check[temp_c_number+i]&phase_flag)
													{
														channel_check[temp_c_number+i] |= phase_flag;
														fprintf(report,"<<< CHECK settling >>>\n");
													}
													else
													{
														/* check offset */
														fprintf(report,"Offset: %g",mean[temp_c_number+i]);
														if (0==tested_card)
														{
															printf("Channel: %d\n",temp_c_number+i+1);
														}
														printf("Offset: %g",mean[temp_c_number+i]);
														if ((float)fabs((double)(mean[temp_c_number+i]))<
															tol_offset)
														{
															fprintf(report," OK");
															printf(" OK");
														}
														else
														{
															channel_check[temp_c_number+i] |= phase_flag;
															fprintf(report," <<< CHECK too large >>>");
															printf(" too large");
															total_checks++;
															pause_for_error();
														}
														fprintf(report,"\n");
														printf("\n");
														/* check rms */
														rms=(float)0;
														sample=samples+
															(k*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
														for (j=(int)number_of_samples;j>0;j--)
														{
															temp=(float)(*sample)-mean[temp_c_number+i];
															rms += temp*temp;
															sample += number_of_channels;
														}
														rms /= (float)number_of_samples;
														rms=(float)sqrt((double)rms);
														fprintf(report,"RMS: " FLOAT_FORMAT,rms);
														printf("RMS: " FLOAT_FORMAT,rms);
														if ((float)fabs((double)rms)<tol_rms)
														{
															fprintf(report," OK");
															printf(" OK");
														}
														else
														{
															channel_check[i] |= phase_flag;
															fprintf(report," <<< CHECK too large >>>");
															printf(" too large");
															total_checks++;
															pause_for_error();
														}
														fprintf(report,"\n");
														printf("\n");
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										if (tested_card)
										{
											register_write_signal_file("phase1.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
										/* turn off stimulation */
										unemap_stop_stimulating(0);
										unemap_set_channel_stimulating(0,0);
									}
									else
									{
										printf("Phase 1.  Could not start stimulation\n");
										fprintf(report,
											"Phase 1.  Could not start stimulation <<< CHECK >>>\n");
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													channel_check[temp_c_number+i] |= phase_flag;
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										total_checks++;
									}
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 2 : Testing cross-talk and fidelity");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,
									"Phase 2 : Testing cross-talk and fidelity\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										tester_card_3=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_3)
												{
													if (tester_card_2)
													{
														tester_card_1=i+1;
													}
													else
													{
														tester_card_2=i+1;
													}
												}
												else
												{
													tester_card_3=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
												if (tester_card_3<=2)
												{
													tester_card_3=3;
												}
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2)&&
													(i+1!=tester_card_3))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
"Even channels of TESTER_1 (%d) to stimulate (output 0).  Odd channels to record\n",
										tester_card_1);
									fprintf(report,"All channels of TESTER_2 (%d) to record\n",
										tester_card_2);
									fprintf(report,
"Odd channels of TESTER_3 (%d) to stimulate (output 0).  Even channels to record\n",
										tester_card_3);
									fprintf(report,
										"Set TEST_CHANNEL of TESTER_1/TESTER_3 to record\n");
									fprintf(report,
			"Set TEST_CHANNEL of TESTER_2 to stimulate and output the test signal\n");
									/* put a 100 Hz square wave into TEST_CHANNEL of TEST (card
										1) and ground the others:
										- set all channels of TEST to record
										- set TEST_CHANNEL of GROUND (card 2 [and 4]) to record,
											set the other channels to stimulate and output zero
										- set TEST_CHANNEL of SIGNAL (card 3) to stimulate and
											output the test signal, set the other channels to
											record */
									for (i=1;i<=max_channels;i++)
									{
										for (j=0;j<number_of_test_cards;j++)
										{
											if (tested_cards[j])
											{
												unemap_set_channel_stimulating(
													j*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
										unemap_set_channel_stimulating(
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
									}
									/* set low pass filter as high as possible */
									unemap_set_antialiasing_filter_frequency(0,(float)10000);
									unemap_get_sample_range(&minimum_sample_value,
										&maximum_sample_value);
									unemap_get_voltage_range(1,&minimum_voltage,&maximum_voltage);
									gain=(maximum_voltage-minimum_voltage)/
										(float)(maximum_sample_value-minimum_sample_value);
									for (l=1;l<=max_channels;l++)
									{
										if ((0==test_channel)||(test_channel==l))
										{
											fprintf(report,"Testing channel ");
											j=0;
											for (i=0;i<number_of_test_cards;i++)
											{
												if (tested_cards[i])
												{
													if (j)
													{
														fprintf(report,",");
													}
													fprintf(report,"%d",
														i*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
													j=1;
												}
											}
											fprintf(report,"\n");
											/* use half of maximum */
											calibrate_amplitude_1=
												CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
#if defined (CALIBRATE_SQUARE_WAVE)
											calibrate_voltage_1[0]=calibrate_amplitude_1;
											calibrate_voltage_1[1]= -calibrate_amplitude_1;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
											/* calculate cosine wave */
											two_pi=(double)8*atan((double)1);
											for (i=0;i<number_of_waveform_points;i++)
											{
												calibrate_voltage_1[i]=
													calibrate_amplitude_1*(float)cos(two_pi*
													(double)i/(double)number_of_waveform_points);
											}
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
											if (0==l%2)
											{
												unemap_set_channel_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
											}
											else
											{
												unemap_set_channel_stimulating(
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
											}
											channel_number_1=
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
											channel_number_2=
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
											channel_number_3=
												(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
											if (unemap_load_voltage_stimulating(1,&channel_number_1,
												(int)0,(float)0,(float *)NULL)&&
												unemap_load_voltage_stimulating(1,&channel_number_3,
												(int)0,(float)0,(float *)NULL)&&
												unemap_load_voltage_stimulating(1,&channel_number_2,
												number_of_waveform_points,
												/*points/s*/(float)number_of_waveform_points*
												CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_1)&&
												unemap_start_stimulating())
#if defined (OLD_CODE)
											if (unemap_start_voltage_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,
												(float)0,(float *)NULL)&&
												unemap_start_voltage_stimulating(
												(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,
												(float)0,(float *)NULL)&&
												unemap_start_voltage_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,
												number_of_waveform_points,
												/*points/s*/(float)number_of_waveform_points*
												CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_1))
#endif /* defined (OLD_CODE) */
											{
												calibrate_amplitude_1=calibrate_voltage_1[0];
												/* wait for the high-pass (DC removal) to settle */
												allow_to_settle(0,tested_cards,temp_channel_check,
													phase_flag,report,&number_of_settled_channels,
													sampling_delay,samples,mean,number_of_samples,
													number_of_channels,tol_settling,max_settling);
												if (number_of_settled_channels<
													MAXIMUM_NUMBER_OF_NI_CARDS*
													NUMBER_OF_CHANNELS_ON_NI_CARD)
												{
													temp_c_number=0;
													for (j=0;j<number_of_test_cards;j++)
													{
														if (tested_cards[j])
														{
															for (i=0;i<max_channels;i++)
															{
																if (temp_channel_check[temp_c_number+i]&
																	phase_flag)
																{
																	channel_check[temp_c_number+i] |= phase0_flag;
																}
															}
														}
														temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
													}
												}
												temp_c_number=0;
												for (k=0;k<number_of_test_cards;k++)
												{
													if (tested_cards[k])
													{
														first=0;
														number_of_settled_channels=0;
														for (i=0;i<max_channels;i++)
														{
															if (!(temp_channel_check[temp_c_number+i]&
																phase_flag))
															{
																sorted_mean[number_of_settled_channels]=
																	mean[temp_c_number+i];
																number_of_settled_channels++;
															}
														}
														if (0<number_of_settled_channels)
														{
															fprintf(report,"Test card %d\n",k+1);
															/* check offsets */
															fprintf(report,"Cross-talk offsets:\n");
															qsort(sorted_mean,number_of_settled_channels,
																sizeof(float),compare_float);
															for (i=0;i<max_channels;i++)
															{
																if (i!=l-1)
																{
																	fprintf(report,"%2d",temp_c_number+i+1);
																	if (temp_channel_check[temp_c_number+i]&
																		phase_flag)
																	{
																		fprintf(report," <<< CHECK settling >>>");
																	}
																	else
																	{
																		fprintf(report," " FLOAT_FORMAT,
																			mean[temp_c_number+i]);
																		if (!((float)fabs((double)(mean[
																			temp_c_number+i]-sorted_mean[
																			number_of_settled_channels/2]))<
																			tol_offset_spread))
																		{
																			channel_check[temp_c_number+i] |=
																				phase0_flag;
																			fprintf(report,
																		" <<< CHECK offset spread too large >>>");
																			total_checks++;
																			if (0==first)
																			{
																				printf(
												"Testing channel %d.  Offset spread too large for :\n",
																					temp_c_number+l);
																			}
																			first++;
																			printf("%2d " FLOAT_FORMAT,
																				temp_c_number+i+1,
																				mean[temp_c_number+i]);
																			if (0==first%4)
																			{
																				printf("\n");
																			}
																			else
																			{
																				printf(" *  ");
																			}
																		}
																	}
																	fprintf(report,"\n");
																}
															}
															if (0!=first%4)
															{
																printf("\n");
															}
															fprintf(report,"Offset median %g",
																sorted_mean[number_of_settled_channels/2]);
															if (!((float)fabs((double)(sorted_mean[
																number_of_settled_channels/2]))<tol_offset))
															{
																channel_check[temp_c_number+l-1] |=
																	phase_flag;
																fprintf(report," <<< CHECK >>>");
																total_checks++;
																printf("Offset median %g is too large\n",
																	sorted_mean[number_of_settled_channels/2]);
																first++;
															}
															fprintf(report,"\n");
															if (0==first)
															{
																printf("Cross-talk offsets OK for channel %d\n",
																	temp_c_number+l);
																fprintf(report,
																	"Cross-talk offsets OK for channel %d\n",
																	temp_c_number+l);
															}
															else
															{
																pause_for_error();
															}
															/* check rms */
															fprintf(report,"Cross-talk RMS:\n");
															first=0;
															for (i=0;i<max_channels;i++)
															{
																if (i!=l-1)
																{
																	fprintf(report,"%2d",temp_c_number+i+1);
																	if (temp_channel_check[temp_c_number+i]&
																		phase_flag)
																	{
																		fprintf(report," <<< CHECK settling >>>");
																	}
																	else
																	{
																		rms=(float)0;
																		sample=samples+
																			(k*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
																		for (j=(int)number_of_samples;j>0;j--)
																		{
																			temp=(float)(*sample)-
																				mean[temp_c_number+i];
																			rms += temp*temp;
																			sample += number_of_channels;
																		}
																		rms /= (float)number_of_samples;
																		rms=(float)sqrt((double)rms);
																		fprintf(report," " FLOAT_FORMAT,rms);
																		if (!((float)fabs((double)rms)<tol_rms))
																		{
																			channel_check[temp_c_number+i] |=
																				phase0_flag;
																			fprintf(report,
																				" <<< CHECK too large >>>");
																			total_checks++;
																			if (0==first)
																			{
																				printf(
																	"Testing channel %d.  RMS too large for :\n",
																					temp_c_number+l);
																			}
																			first++;
																			printf("%2d " FLOAT_FORMAT,
																				temp_c_number+i+1,rms);
																			if (0==first%4)
																			{
																				printf("\n");
																			}
																			else
																			{
																				printf(" *  ");
																			}
																		}
																	}
																	fprintf(report,"\n");
																}
															}
															if (0!=first%4)
															{
																printf("\n");
															}
															if (0==first)
															{
																printf("Cross-talk noise OK for channel %d\n",
																	temp_c_number+l);
																fprintf(report,
																	"Cross-talk noise OK for channel %d\n",
																	temp_c_number+l);
															}
															else
															{
																pause_for_error();
															}
															if ((0!=test_channel)&&(0!=tested_card))
															{
																register_write_signal_file("phase2.sig",
																	k*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
															}
															/* check fidelity */
															fprintf(report,"Fidelity:");
															if (temp_channel_check[temp_c_number+l-1]&
																phase_flag)
															{
																channel_check[temp_c_number+l-1] |=
																	phase_flag;
																fprintf(report," <<< CHECK settling >>>\n");
															}
															else
															{
																fprintf(report,"\n");
#if defined (CALIBRATE_SQUARE_WAVE)
																if (ALLOCATE(sorted_signal,float,
																	number_of_samples))
																{
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																	sample=samples+
																		(k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1);
#if defined (CALIBRATE_SQUARE_WAVE)
																	for (j=0;j<(int)number_of_samples;j++)
																	{
																		temp=(float)(*sample)-
																			mean[temp_c_number+l-1];
																		if (temp<(float)0)
																		{
																			temp= -temp;
																		}
																		sorted_signal[j]=temp;
																		sample += number_of_channels;
																	}
																	qsort(sorted_signal,number_of_samples,
																		sizeof(float),compare_float);
																	first=0;
																	rms=(float)0;
																	for (j=(int)(switching_time*200*
																		(float)number_of_samples);
																		j<(int)number_of_samples;j++)
																	{
																		temp=sorted_signal[j];
																		rms += temp*temp;
																	}
																	rms /= (1-200*switching_time)*
																		(float)number_of_samples;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
																	/* calculate the component at the test
																		frequency */
																	two_pi=(double)8*atan((double)1);
																	a=(float)0;
																	b=(float)0;
																	for (j=0;j<(int)number_of_samples;j++)
																	{
																		temp=(float)(*sample)-
																			mean[temp_c_number+l-1];
																		a += temp*(float)sin(two_pi*
																			(double)CALIBRATE_WAVEFORM_FREQUENCY*
																			(double)j/(double)number_of_samples);
																		b += temp*(float)cos(two_pi*
																			(double)CALIBRATE_WAVEFORM_FREQUENCY*
																			(double)j/(double)number_of_samples);
																		sample += number_of_channels;
																	}
																	a *= (float)2/(float)number_of_samples;
																	b *= (float)2/(float)number_of_samples;
																	rms=a*a+b*b;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																	rms=(float)sqrt((double)rms);
																	fprintf(report,
																		"Measured amplitude %g.  Input %g",
																		(float)rms*gain,
																		(float)calibrate_amplitude_1);
																	if (!((float)fabs((double)(
																		calibrate_amplitude_1-gain*rms))<
																		tol_signal_value*calibrate_amplitude_1))
																	{
																		channel_check[temp_c_number+l-1] |=
																			phase_flag;
																		fprintf(report," <<< CHECK >>>");
																		total_checks++;
																		printf(
					"Incorrect value for channel %d.  Measured amplitude %g.  Input %g\n",
																			temp_c_number+l,(float)rms*gain,
																			(float)calibrate_amplitude_1);
																		first++;
																	}
																	fprintf(report,"\n");
																	fprintf(report,"mean=%g, rms=%g\n",
																		mean[temp_c_number+l-1],rms);
																	signal_rms=(float)0;
#if defined (CALIBRATE_SQUARE_WAVE)
																	for (j=(int)(switching_time*200*
																		(float)number_of_samples);
																		j<(int)number_of_samples;j++)
																	{
																		temp=sorted_signal[j]-rms;
																		signal_rms += temp*temp;
																	}
																	signal_rms /= (1-200*switching_time)*
																		(float)number_of_samples;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
																	sample=samples+
																		(k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1);
																	for (j=0;j<(int)number_of_samples;j++)
																	{
																		temp=(float)(*sample)-
																			mean[temp_c_number+l-1]-
																			(a*(float)sin(two_pi*
																			(double)CALIBRATE_WAVEFORM_FREQUENCY*
																			(double)j/(double)number_of_samples)+
																			b*(float)cos(two_pi*
																			(double)CALIBRATE_WAVEFORM_FREQUENCY*
																			(double)j/(double)number_of_samples));
																		signal_rms += temp*temp;
																		sample += number_of_channels;
																	}
																	signal_rms /= (float)number_of_samples;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																	signal_rms=(float)sqrt((double)signal_rms);
																	fprintf(report,
															"RMS of difference between measured and input %g",
																		signal_rms);
																	if (!(signal_rms<tol_signal_form))
																	{
																		channel_check[temp_c_number+l-1] |=
																			phase_flag;
																		fprintf(report," <<< CHECK >>>");
																		total_checks++;
																		printf(
														"Incorrect signal for channel %d.  RMS error %g\n",
																			temp_c_number+l,signal_rms);
																		first++;
																	}
																	fprintf(report,"\n");
																	if (0==first)
																	{
																		printf("Fidelity OK for channel %d\n",
																			temp_c_number+l);
																		fprintf(report,
																			"Fidelity OK for channel %d\n",
																			temp_c_number+l);
																	}
																	else
																	{
																		pause_for_error();
																	}
#if defined (CALIBRATE_SQUARE_WAVE)
																	DEALLOCATE(sorted_signal);
																}
																else
																{
																	display_message(ERROR_MESSAGE,
																	"Phase 2.  Could not allocate sorted_signal");
																}
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
															}
														}
													}
													temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
												}
												unemap_stop_stimulating(0);
											}
											else
											{
												printf(
									"Phase 2.  Could not start stimulation for channel %d\n",
													temp_c_number+l);
												fprintf(report,
				"Phase 2.  Could not start stimulation for channel %d <<< CHECK >>>\n",
													temp_c_number+l);
												channel_check[temp_c_number+l-1] |= phase_flag;
												total_checks++;
											}
											if (0==l%2)
											{
												unemap_set_channel_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
											}
											else
											{
												unemap_set_channel_stimulating(
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
											}
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
										}
									}
									/* turn off stimulation */
									unemap_stop_stimulating(0);
									unemap_set_channel_stimulating(0,0);
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 3 : Testing stimulation");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 3 : Testing stimulation\n");
								/* only one card can be tested at a time */
								for (k=0;k<number_of_test_cards;k++)
								{
									if ((0==tested_card)||(k+1==tested_card))
									{
										for (i=0;i<number_of_test_cards;i++)
										{
											tested_cards[i]=0;
										}
										tested_cards[k]=1;
										if (0==tested_card)
										{
											tester_card_1=(k+1)%number_of_test_cards+1;
											tester_card_2=(k+2)%number_of_test_cards+1;
											tester_card_3=(k+3)%number_of_test_cards+1;
										}
										fprintf(report,"All channels of TESTED (%d) to record\n",
											k+1);
										fprintf(report,
	"Even channels of TESTER_1 (%d) to stimulate (output 0).  Odd channels to record\n",
											tester_card_1);
										fprintf(report,"All channels of TESTER_2 (%d) to record\n",
											tester_card_2);
										fprintf(report,
	"Odd channels of TESTER_3 (%d) to stimulate (output 0).  Even channels to record\n",
											tester_card_3);
										fprintf(report,"Set TEST_CHANNEL of TESTED to stimulate\n");
										fprintf(report,
											"Set TEST_CHANNEL of TESTER_1/TESTER_3 to record\n");
										/* put a 100 Hz square wave out of TEST_CHANNEL of TEST
											(card 1) and ground the others:
											- set TEST_CHANNEL of TEST to stimulate and output the
												test signal, set the other channels to record
											- set TEST_CHANNEL of GROUND (card 2 [and 4]) to record,
												set the other channels to stimulate and output zero
											- set all channels of SIGNAL (card 3) to record */
										for (i=1;i<=max_channels;i++)
										{
											unemap_set_channel_stimulating(
												k*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											if (0==i%2)
											{
												unemap_set_channel_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
												unemap_set_channel_stimulating(
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
											else
											{
												unemap_set_channel_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
												unemap_set_channel_stimulating(
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											}
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
										/* set low pass filter as high as possible */
										unemap_set_antialiasing_filter_frequency(0,(float)10000);
										unemap_get_sample_range(&minimum_sample_value,
											&maximum_sample_value);
										unemap_get_voltage_range(
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,
											&minimum_voltage,&maximum_voltage);
										gain=(maximum_voltage-minimum_voltage)/
											(float)(maximum_sample_value-minimum_sample_value);
										for (l=1;l<=max_channels;l++)
										{
											if ((0==test_channel)||(test_channel==l))
											{
												fprintf(report,"Testing channel %d\n",
													k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
												unemap_set_channel_stimulating(
													k*NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
												if (0==l%2)
												{
													unemap_set_channel_stimulating((tester_card_1-1)*
														NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
												}
												else
												{
													unemap_set_channel_stimulating((tester_card_3-1)*
														NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
												}
												/* use half of maximum */
												calibrate_amplitude_1=
													CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
#if defined (CALIBRATE_SQUARE_WAVE)
												calibrate_voltage_1[0]=calibrate_amplitude_1;
												calibrate_voltage_1[1]= -calibrate_amplitude_1;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
												/* calculate cosine wave */
												two_pi=(double)8*atan((double)1);
												for (i=0;i<number_of_waveform_points;i++)
												{
													calibrate_voltage_1[i]=
														calibrate_amplitude_1*(float)cos(two_pi*
														(double)i/(double)number_of_waveform_points);
												}
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
												channel_number_2=k*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
												channel_number_1=
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
												channel_number_3=
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
												if (unemap_load_voltage_stimulating(1,&channel_number_2,
													number_of_waveform_points,
													/*points/s*/(float)number_of_waveform_points*
													CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_1)&&
													unemap_load_voltage_stimulating(1,&channel_number_1,
													(int)0,(float)0,(float *)NULL)&&
													unemap_load_voltage_stimulating(1,&channel_number_3,
													(int)0,(float)0,(float *)NULL)&&
													unemap_start_stimulating())
#if defined (NOT_DEBUG)
#endif /* defined (NOT_DEBUG) */
#if defined (DEBUG)
												calibrate_voltage_2[0]=0;
												calibrate_voltage_2[1]=0;
												for (i=0;i<number_of_waveform_points;i++)
												{
													calibrate_voltage_2[i]=0;
												}
												if (
													unemap_load_voltage_stimulating(1,&channel_number_2,
													number_of_waveform_points,
													/*points/s*/(float)number_of_waveform_points*
													CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_1)&&
													unemap_load_voltage_stimulating(1,&channel_number_1,
													2/*number_of_waveform_points/50*/,
													(float)number_of_waveform_points*
													CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_2)&&
													unemap_load_voltage_stimulating(1,&channel_number_3,
													2/*number_of_waveform_points/50*/,
													(float)number_of_waveform_points*
													CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_2)
													&&unemap_start_stimulating()
													)
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
												if (unemap_start_voltage_stimulating(
													k*NUMBER_OF_CHANNELS_ON_NI_CARD+1,
													number_of_waveform_points,
													/*points/s*/(float)number_of_waveform_points*
													CALIBRATE_WAVEFORM_FREQUENCY,calibrate_voltage_1)&&
													unemap_start_voltage_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,
													(float)0,(float *)NULL)&&
													unemap_start_voltage_stimulating(
													(tester_card_3-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,
													(float)0,(float *)NULL))
#endif /* defined (OLD_CODE) */
												{
													calibrate_amplitude_1=calibrate_voltage_1[0];
													/* wait for the high-pass (DC removal) to settle */
													tested_cards[k]=0;
													tested_cards[tester_card_2-1]=1;
													allow_to_settle(0,tested_cards,temp_channel_check,
														phase_flag,report,&number_of_settled_channels,
														sampling_delay,samples,mean,number_of_samples,
														number_of_channels,tol_settling,max_settling);
													tested_cards[k]=1;
													tested_cards[tester_card_2-1]=0;
													first=0;
													number_of_settled_channels=0;
													temp_c_number=(tester_card_2-1)*
														NUMBER_OF_CHANNELS_ON_NI_CARD;
													for (i=0;i<max_channels;i++)
													{
														if (!(temp_channel_check[i]&phase_flag))
														{
															sorted_mean[number_of_settled_channels]=
																mean[temp_c_number+i];
															number_of_settled_channels++;
														}
													}
													if (0<number_of_settled_channels)
													{
														/* check offsets */
														fprintf(report,"Cross-talk offsets:\n");
														qsort(sorted_mean,number_of_settled_channels,
															sizeof(float),compare_float);
														for (i=0;i<max_channels;i++)
														{
															if (i!=l-1)
															{
																fprintf(report,"%2d",
																	k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1);
																if (temp_channel_check[temp_c_number+i]&
																	phase_flag)
																{
																	fprintf(report," <<< CHECK settling >>>");
																	channel_check[
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
																		phase0_flag;
																}
																else
																{
																	fprintf(report," " FLOAT_FORMAT,
																		mean[temp_c_number+i]);
																	if (!((float)fabs((double)(mean[
																		temp_c_number+i]-sorted_mean[
																		number_of_settled_channels/2]))<
																		tol_offset_spread))
																	{
																		channel_check[
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1] |=
																			phase_flag;
																		fprintf(report,
																		" <<< CHECK offset spread too large >>>");
																		total_checks++;
																		if (0==first)
																		{
																			printf(
												"Testing channel %d.  Offset spread too large for :\n",
																				k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
																		}
																		first++;
																		printf("%2d " FLOAT_FORMAT,
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1,
																			mean[temp_c_number+i]);
																		if (0==first%4)
																		{
																			printf("\n");
																		}
																		else
																		{
																			printf(" *  ");
																		}
																	}
																}
																fprintf(report,"\n");
															}
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														fprintf(report,"Offset median %g",
															sorted_mean[number_of_settled_channels/2]);
														if (!((float)fabs((double)(sorted_mean[
															number_of_settled_channels/2]))<tol_offset))
														{
															channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-
																1] |= phase_flag;
															fprintf(report," <<< CHECK too large >>>");
															total_checks++;
															printf("Offset median %g is too large\n",
																sorted_mean[number_of_settled_channels/2]);
															first++;
														}
														fprintf(report,"\n");
														if (0==first)
														{
															printf("Cross-talk offsets OK for channel %d\n",
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
															fprintf(report,
																"Cross-talk offsets OK for channel %d\n",
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
														}
														else
														{
															pause_for_error();
														}
														/* check rms */
														fprintf(report,"Cross-talk RMS:\n");
														first=0;
														for (i=0;i<max_channels;i++)
														{
															if (i!=l-1)
															{
																fprintf(report,"%2d",
																	k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1);
																if (temp_channel_check[i]&phase_flag)
																{
																	fprintf(report," <<< CHECK settling >>>");
																}
																else
																{
																	rms=(float)0;
																	sample=samples+((tester_card_2-1)*
																		NUMBER_OF_CHANNELS_ON_NI_CARD+i);
																	for (j=(int)number_of_samples;j>0;j--)
																	{
																		temp=(float)(*sample)-
																			mean[temp_c_number+i];
																		rms += temp*temp;
																		sample += number_of_channels;
																	}
																	rms /= (float)number_of_samples;
																	rms=(float)sqrt((double)rms);
																	fprintf(report," " FLOAT_FORMAT,rms);
																	if (!((float)fabs((double)rms)<tol_rms))
																	{
																		channel_check[
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
																			phase0_flag;
																		fprintf(report," <<< CHECK too large >>>");
																		total_checks++;
																		if (0==first)
																		{
																			printf(
																	"Testing channel %d.  RMS too large for :\n",
																				k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
																		}
																		first++;
																		printf("%2d " FLOAT_FORMAT,
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1,rms);
																		if (0==first%4)
																		{
																			printf("\n");
																		}
																		else
																		{
																			printf(" *  ");
																		}
																	}
																}
																fprintf(report,"\n");
															}
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														if (0==first)
														{
															printf("Cross-talk noise OK for channel %d\n",
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
															fprintf(report,
																"Cross-talk noise OK for channel %d\n",
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
														}
														else
														{
															pause_for_error();
														}
														if ((0!=test_channel)&&(0!=tested_card))
														{
															register_write_signal_file("phase3.sig",
																(tester_card_2-1)*
																NUMBER_OF_CHANNELS_ON_NI_CARD+1);
														}
														/* check fidelity */
														fprintf(report,"Fidelity:");
														if (temp_channel_check[
															k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1]&phase_flag)
														{
															channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-
																1] |= phase_flag;
															fprintf(report," <<< CHECK settling >>>\n");
														}
														else
														{
															fprintf(report,"\n");
#if defined (CALIBRATE_SQUARE_WAVE)
															if (ALLOCATE(sorted_signal,float,
																number_of_samples))
															{
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																sample=samples+((tester_card_2-1)*
																	NUMBER_OF_CHANNELS_ON_NI_CARD+l-1);
#if defined (CALIBRATE_SQUARE_WAVE)
																for (j=0;j<(int)number_of_samples;j++)
																{
																	temp=(float)(*sample)-
																		mean[temp_c_number+l-1];
																	if (temp<(float)0)
																	{
																		temp= -temp;
																	}
																	sorted_signal[j]=temp;
																	sample += number_of_channels;
																}
																qsort(sorted_signal,number_of_samples,
																	sizeof(float),compare_float);
																first=0;
																rms=(float)0;
																for (j=(int)(switching_time*200*
																	(float)number_of_samples);
																	j<(int)number_of_samples;j++)
																{
																	temp=sorted_signal[j];
																	rms += temp*temp;
																}
																rms /= (1-200*switching_time)*
																	(float)number_of_samples;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
																/* calculate the component at the test
																	frequency */
																two_pi=(double)8*atan((double)1);
																a=(float)0;
																b=(float)0;
																for (j=0;j<(int)number_of_samples;j++)
																{
																	temp=(float)(*sample)-
																		mean[temp_c_number+l-1];
																	a += temp*(float)sin(two_pi*
																		(double)CALIBRATE_WAVEFORM_FREQUENCY*
																		(double)j/(double)number_of_samples);
																	b += temp*(float)cos(two_pi*
																		(double)CALIBRATE_WAVEFORM_FREQUENCY*
																		(double)j/(double)number_of_samples);
																	sample += number_of_channels;
																}
																a *= (float)2/(float)number_of_samples;
																b *= (float)2/(float)number_of_samples;
																rms=a*a+b*b;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																rms=(float)sqrt((double)rms);
																fprintf(report,
																	"Measured amplitude %g.  Input %g",
																	(float)rms*gain,(float)calibrate_amplitude_1);
																if (!((float)fabs((double)(
																	calibrate_amplitude_1-gain*rms))<
																	tol_signal_value*calibrate_amplitude_1))
																{
																	channel_check[
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1] |=
																		phase_flag;
																	fprintf(report," <<< CHECK >>>");
																	total_checks++;
																	printf(
					"Incorrect value for channel %d.  Measured amplitude %g.  Input %g\n",
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l,
																		(float)rms*gain,
																		(float)calibrate_amplitude_1);
																	first++;
																}
																fprintf(report,"\n");
																fprintf(report,"mean=%g, rms=%g\n",
																	mean[k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1],
																	rms);
																signal_rms=(float)0;
#if defined (CALIBRATE_SQUARE_WAVE)
																for (j=(int)(switching_time*200*
																	(float)number_of_samples);
																	j<(int)number_of_samples;j++)
																{
																	temp=sorted_signal[j]-rms;
																	signal_rms += temp*temp;
																}
																signal_rms /= (1-200*switching_time)*
																	(float)number_of_samples;
#else /* defined (CALIBRATE_SQUARE_WAVE) */
																sample=samples+((tester_card_2-1)*
																	NUMBER_OF_CHANNELS_ON_NI_CARD+l-1);
																for (j=0;j<(int)number_of_samples;j++)
																{
																	temp=(float)(*sample)-
																		mean[temp_c_number+l-1]-
																		(a*(float)sin(two_pi*
																		(double)CALIBRATE_WAVEFORM_FREQUENCY*
																		(double)j/(double)number_of_samples)+
																		b*(float)cos(two_pi*
																		(double)CALIBRATE_WAVEFORM_FREQUENCY*
																		(double)j/(double)number_of_samples));
																	signal_rms += temp*temp;
																	sample += number_of_channels;
																}
																signal_rms /= (float)number_of_samples;
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
																signal_rms=(float)sqrt((double)signal_rms);
																fprintf(report,
															"RMS of difference between measured and input %g",
																	signal_rms);
																if (!(signal_rms<tol_signal_form))
																{
																	channel_check[
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1] |=
																		phase_flag;
																	fprintf(report," <<< CHECK >>>");
																	total_checks++;
																	printf(
														"Incorrect signal for channel %d.  RMS error %g\n",
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l,
																		signal_rms);
																	first++;
																}
																fprintf(report,"\n");
																if (0==first)
																{
																	printf("Fidelity OK for channel %d\n",
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
																	fprintf(report,"Fidelity OK for channel %d\n",
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
																}
																else
																{
																	pause_for_error();
																}
#if defined (CALIBRATE_SQUARE_WAVE)
																DEALLOCATE(sorted_signal);
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"Phase 3.  Could not allocate sorted_signal");
															}
#endif /* defined (CALIBRATE_SQUARE_WAVE) */
														}
													}
													unemap_stop_stimulating(0);
												}
												else
												{
													printf(
											"Phase 3.  Could not start stimulation for channel %d\n",
														k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
													fprintf(report,
				"Phase 3.  Could not start stimulation for channel %d <<< CHECK >>>\n",
														k*NUMBER_OF_CHANNELS_ON_NI_CARD+l);
													channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+l-1] |=
														phase_flag;
													total_checks++;
												}
												if (0==l%2)
												{
													unemap_set_channel_stimulating((tester_card_1-1)*
														NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
												}
												else
												{
													unemap_set_channel_stimulating((tester_card_3-1)*
														NUMBER_OF_CHANNELS_ON_NI_CARD+l,1);
												}
												unemap_set_channel_stimulating(
													k*NUMBER_OF_CHANNELS_ON_NI_CARD+l,0);
											}
										}
										/* turn off stimulation */
										unemap_stop_stimulating(0);
										unemap_set_channel_stimulating(0,0);
									}
								}
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 4 : Testing DC removal");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 4 : Testing DC removal\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_2)
												{
													tester_card_1=i+1;
												}
												else
												{
													tester_card_2=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
			"Even channels of TESTER_1 (%d) to stimulate.  Odd channels to record\n",
										tester_card_1);
									fprintf(report,
			"Odd channels of TESTER_2 (%d) to stimulate.  Even channels to record\n",
										tester_card_2);
									/* set all channels of TEST (card 1) to 0, let settle, set
										all channels to a known voltage and calculate the decay
										constants:
										- set all channels of TEST to record
										- set even channels of GROUND (card 2) to record and odd
											channels to stimulate
										- set odd channels of SIGNAL (card 3) to record and even
											channels to stimulate */
									for (i=1;i<=max_channels;i++)
									{
										for (j=0;j<number_of_test_cards;j++)
										{
											if (tested_cards[j])
											{
												unemap_set_channel_stimulating(
													j*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
									}
									channel_number_1=
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									channel_number_2=
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									if (unemap_load_voltage_stimulating(1,&channel_number_1,
										(int)0,(float)0,(float *)NULL)&&
										unemap_load_voltage_stimulating(1,&channel_number_2,
										(int)0,(float)0,(float *)NULL)&&unemap_start_stimulating())
#if defined (OLD_CODE)
									if (unemap_start_voltage_stimulating((tester_card_1-1)*
										NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,(float)0,(float *)NULL)&&
										unemap_start_voltage_stimulating((tester_card_2-1)*
										NUMBER_OF_CHANNELS_ON_NI_CARD+1,0,(float)0,(float *)NULL))
#endif /* defined (OLD_CODE) */
									{
										unemap_get_sample_range(&minimum_sample_value,
											&maximum_sample_value);
										k=0;
										while ((k<number_of_test_cards-1)&&!tested_cards[k])
										{
											k++;
										}
										unemap_get_voltage_range(k*NUMBER_OF_CHANNELS_ON_NI_CARD+1,
											&minimum_voltage,&maximum_voltage);
										gain=(maximum_voltage-minimum_voltage)/
											(float)(maximum_sample_value-minimum_sample_value);
										/* use half of maximum */
										calibrate_amplitude_1=
											CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
										calibrate_amplitude_2=
											CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											printf("Failed to settle for:");
											for (i=0;i<number_of_test_cards*
												NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
											{
												if (temp_channel_check[i]&phase_flag)
												{
													printf(" %d",i+1);
													channel_check[i] |= phase_flag;
												}
											}
											printf("\n");
											pause_for_error();
										}
										unemap_stop_stimulating(0);
										channel_number_1=
											(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
										channel_number_2=
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
										if (unemap_load_voltage_stimulating(1,&channel_number_1,
											(int)1,(float)0,&calibrate_amplitude_1)&&
											unemap_load_voltage_stimulating(1,&channel_number_2,
											(int)1,(float)0,&calibrate_amplitude_2)&&
											unemap_start_stimulating())
#if defined (OLD_CODE)
										if (unemap_start_voltage_stimulating(
											(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1,
											(float)0,&calibrate_amplitude_1)&&
											unemap_start_voltage_stimulating(
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1,
											(float)0,&calibrate_amplitude_2))
#endif /* defined (OLD_CODE) */
										{
											unemap_start_sampling();
											sleep(sampling_delay);
											unemap_stop_sampling();
											unemap_get_samples_acquired(0,samples);
											if (0!=tested_card)
											{
												register_write_signal_file("phase4.sig",
													(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
											}
											/* assume that v[i]=A*exp(k*t[i]) so that
												ln(v[i])=k*t[i]+ln(A)
												and can use linear regression */
#if defined (OLD_CODE)
											printf("Results of fitting v=A*exp(k*t)\n");
											fprintf(report,"Results of fitting v=A*exp(k*t)\n");
											printf(
						"r is the correlation coefficient and should be close to -1\n");
											printf("  (V decreases as t increases)\n");
											fprintf(report,
						"r is the correlation coefficient and should be close to -1\n");
											fprintf(report,"  (V decreases as t increases)\n");
#endif /* defined (OLD_CODE) */
											first=0;
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if ((0==test_channel)||(i+1==test_channel))
														{
															fprintf(report,"%2d",
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1);
															if (temp_channel_check[
																k*NUMBER_OF_CHANNELS_ON_NI_CARD+i]&phase_flag)
															{
																channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+
																	i] |= phase_flag;
																fprintf(report," <<< CHECK settling >>>");
																printf("%2d <<< CHECK settling >>>\n",
																	k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1);
															}
															else
															{
																sum_x=(float)0;
																sum_y=(float)0;
																count=0;
																sample=samples+
																	(k*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
																for (j=0;j<(int)number_of_samples;j++)
																{
																	x=(float)j/sampling_frequency;
																	y=mean[k*NUMBER_OF_CHANNELS_ON_NI_CARD+i]-
																		(float)(*sample);
																	if (y<0)
																	{
																		y= -y;
																	}
																	if (y>(float)0)
																	{
																		count++;
																		y=(float)log((double)y);
																		sum_x += x;
																		sum_y += y;
																	}
																	sample += number_of_channels;
																}
																if (count>0)
																{
																	if (count<(int)number_of_samples)
																	{
																		printf(
																		"%2d.  %d out of %lu valid <<< CHECK >>>\n",
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1,count,
																			number_of_samples);
																		fprintf(report,
																			".  %d out of %lu valid <<< CHECK >>>",
																			count,number_of_samples);
																		first++;
																		total_checks++;
																		channel_check[
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
																			phase_flag;
																	}
																	mean_x=sum_x/(float)count;
																	mean_y=sum_y/(float)count;
																	sum_xx=(float)0;
																	sum_yy=(float)0;
																	sum_xy=(float)0;
																	b=(float)0;
																	sample=samples+
																		(k*NUMBER_OF_CHANNELS_ON_NI_CARD+i);
																	for (j=0;j<(int)number_of_samples;j++)
																	{
																		x=(float)j/sampling_frequency;
																		y=mean[k*NUMBER_OF_CHANNELS_ON_NI_CARD+i]-
																			(float)(*sample);
																		if (y<0)
																		{
																			y= -y;
																		}
																		if (y>(float)0)
																		{
																			y=(float)log((double)y);
																			x -= mean_x;
																			b += x*y;
																			y -= mean_y;
																			sum_xx += x*x;
																			sum_yy += y*y;
																			sum_xy += x*y;
																		}
																		sample += number_of_channels;
																	}
																	b /= sum_xx;
																	a=(mean_y-b*mean_x);
																	a=(float)exp((double)a);
																	r=sum_xy/(float)sqrt((double)(sum_xx*sum_yy));
																	fprintf(report,".  A=%g V, k=%g Hz, r=%g",a,b,
																		r);
																	if (!((fabs(b-decay_constant)<
																		tol_decay_constant)&&(fabs(r+1)<
																		tol_correlation_coefficient)))
																	{
																		printf(
																"%2d.  A=%g V, k=%g Hz, r=%g.  <<< CHECK >>>\n",
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i+1,a,b,
																			r);
																		fprintf(report," <<< CHECK >>>");
																		channel_check[
																			k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
																			phase_flag;
																		first++;
																		total_checks++;
																	}
																}
																else
																{
																	printf(".  No valid data\n");
																	fprintf(report,
																		".  No valid data <<< CHECK >>>");
																	channel_check[
																		k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
																		phase_flag;
																	first++;
																	total_checks++;
																}
															}
															fprintf(report,"\n");
														}
													}
													if (0==first)
													{
														printf("DC removal OK\n");
														fprintf(report,"DC removal OK\n");
													}
													pause_for_error();
												}
											}
										}
										else
										{
											printf("Phase 4.  Could not start stimulation 2\n");
											fprintf(report,
								"Phase 4.  Could not start stimulation 2 <<< CHECK >>>\n");
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
															phase_flag;
													}
												}
											}
											total_checks++;
										}
									}
									else
									{
										printf("Phase 4.  Could not start stimulation 1\n");
										fprintf(report,
										"Phase 4.  Could not start stimulation 1 <<< CHECK >>>\n");
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													channel_check[k*NUMBER_OF_CHANNELS_ON_NI_CARD+i] |=
														phase_flag;
												}
											}
										}
										total_checks++;
									}
									/* turn off stimulation */
									unemap_stop_stimulating(0);
									unemap_set_channel_stimulating(0,0);
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 5 : Testing anti-aliasing filter");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 5 : Testing anti-aliasing filter\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_2)
												{
													tester_card_1=i+1;
												}
												else
												{
													tester_card_2=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
			"Even channels of TESTER_1 (%d) to stimulate.  Odd channels to record\n",
										tester_card_1);
									fprintf(report,
			"Odd channels of TESTER_2 (%d) to stimulate.  Even channels to record\n",
										tester_card_2);
									/* set the anti-aliasing filter on TEST (card 1) to 10 kHz,
										input a 200 Hz sine wave, record, set the filter to
										200 Hz, input a 200 Hz sine wave, record, compute the dB
										reduction (should be 3 dB point), set the filter to
										100 Hz, input a 250 Hz sine wave, record, input a 500 Hz
										sine wave and compute the dB reduction (should be
										log(500/250)*100dB - 5th order Bessel is 100 dB per
										decade) :
										- set all channels of TEST to record
										- set even channels of GROUND (card 2) to record and odd
											channels to stimulate
										- set odd channels of SIGNAL (card 3) to record and even
											channels to stimulate */
									for (i=1;i<=max_channels;i++)
									{
										for (j=0;j<number_of_test_cards;j++)
										{
											if (tested_cards[j])
											{
												unemap_set_channel_stimulating(
													j*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
									}
									/* set low pass filter to 10 kHz */
									unemap_set_antialiasing_filter_frequency(0,(float)10000);
									unemap_get_sample_range(&minimum_sample_value,
										&maximum_sample_value);
									k=0;
									while ((k<number_of_test_cards-1)&&!tested_cards[k])
									{
										k++;
									}
									unemap_get_voltage_range(k*NUMBER_OF_CHANNELS_ON_NI_CARD+1,
										&minimum_voltage,&maximum_voltage);
									gain=(maximum_voltage-minimum_voltage)/
										(float)(maximum_sample_value-minimum_sample_value);
									/* use half of maximum */
									calibrate_amplitude_1=
										CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
									/* calculate sine wave */
									two_pi=(double)8*atan((double)1);
									for (i=0;i<500;i++)
									{
										calibrate_voltage_1[i]=calibrate_amplitude_1*
											(float)sin(two_pi*(double)i/(double)500);
										calibrate_voltage_2[i]=calibrate_voltage_1[i];
									}
									channel_number_1=
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									channel_number_2=
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									if (unemap_load_voltage_stimulating(1,&channel_number_1,
										(int)500,/*points/s gives 200 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_load_voltage_stimulating(1,&channel_number_2,
										(int)500,/*points/s gives 200 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2)&&
										unemap_start_stimulating())
#if defined (OLD_CODE)
									if (unemap_start_voltage_stimulating(
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,500,
										/*points/s gives 200 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_start_voltage_stimulating(
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,500,
										/*points/s gives 200 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2))
#endif /* defined (OLD_CODE) */
									{
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											printf("Failed to settle for:");
											for (i=0;i<number_of_test_cards*
												NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
											{
												if (temp_channel_check[i]&phase_flag)
												{
													printf(" %d",i+1);
													channel_check[i] |= phase_flag;
												}
											}
											printf("\n");
											pause_for_error();
										}
										first=0;
										if (0!=tested_card)
										{
											register_write_signal_file("phase5_1.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
										/* calculate rms */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													rms=(float)0;
													sample=samples+(temp_c_number+i);
													for (j=(int)number_of_samples;j>0;j--)
													{
														temp=(float)(*sample)-mean[temp_c_number+i];
														rms += temp*temp;
														sample += number_of_channels;
													}
													rms /= (float)number_of_samples;
													rms_save[temp_c_number+i]=(float)sqrt((double)rms);
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										/* set low pass filter to 200 Hz */
										unemap_set_antialiasing_filter_frequency(0,(float)200);
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											number_of_settled_channels=0;
											printf("Failed to settle for:");
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															printf(" %d",temp_c_number+i+1);
															channel_check[temp_c_number+i] |= phase_flag;
														}
														else
														{
															if (!(channel_check[temp_c_number+i]&phase_flag))
															{
																number_of_settled_channels++;
															}
														}
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
											printf("\n");
											pause_for_error();
										}
										first=0;
										if (0<number_of_settled_channels)
										{
											if (0!=tested_card)
											{
												register_write_signal_file("phase5_2.sig",
													(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
											}
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if ((0==test_channel)||(i+1==test_channel))
														{
															fprintf(report,"channel %d",temp_c_number+i+1);
															if (channel_check[temp_c_number+i]&phase_flag)
															{
																fprintf(report," <<< CHECK settling >>>\n");
															}
															else
															{
																fprintf(report,"\n");
																rms=(float)0;
																sample=samples+(temp_c_number+i);
																for (j=(int)number_of_samples;j>0;j--)
																{
																	temp=(float)(*sample)-mean[temp_c_number+i];
																	rms += temp*temp;
																	sample += number_of_channels;
																}
																rms /= (float)number_of_samples;
																rms=(float)sqrt((double)rms);
																db_reduction=(float)20*(float)log10((double)(
																	rms_save[temp_c_number+i]/rms));
																fprintf(report,
																	"  200 Hz signal, 10000 Hz filter, rms=%g\n",
																	rms_save[temp_c_number+i]);
																fprintf(report,
																	"  200 Hz signal, 200 Hz filter, rms=%g\n",
																	rms);
																fprintf(report,"  dB reduction=%g",
																	db_reduction);
																if (!(fabs(db_reduction-3)<tol_db1))
																{
																	printf("channel %d\n",temp_c_number+i+1);
																	printf(
																	"  200 Hz signal, 10000 Hz filter, rms=%g\n",
																		rms_save[temp_c_number+i]);
																	printf(
																		"  200 Hz signal, 200 Hz filter, rms=%g\n",
																		rms);
																	printf("  dB reduction=%g <<< CHECK >>>\n",
																		db_reduction);
																	fprintf(report," <<< CHECK >>>");
																	channel_check[temp_c_number+i] |= phase_flag;
																	total_checks++;
																	first++;
																}
																fprintf(report,"\n");
															}
														}
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
											pause_for_error();
										}
										unemap_stop_stimulating(0);
										/* set low pass filter to 100 Hz */
										unemap_set_antialiasing_filter_frequency(0,(float)100);
										/* calculate sine wave */
										for (i=0;i<400;i++)
										{
											calibrate_voltage_1[i]=calibrate_amplitude_1*
												(float)sin(two_pi*(double)i/(double)400);
											calibrate_voltage_2[i]=calibrate_voltage_1[i];
										}
										channel_number_1=
											(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
										channel_number_2=
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
										if (unemap_load_voltage_stimulating(1,&channel_number_1,
											(int)400,/*points/s gives 250 Hz waveform frequency*/
											(float)100000.,calibrate_voltage_1)&&
											unemap_load_voltage_stimulating(1,&channel_number_2,
											(int)400,/*points/s gives 250 Hz waveform frequency*/
											(float)100000.,calibrate_voltage_2)&&
											unemap_start_stimulating())
#if defined (OLD_CODE)
										if (unemap_start_voltage_stimulating(
											(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,400,
											/*points/s gives 250 Hz waveform frequency*/
											(float)100000.,calibrate_voltage_1)&&
											unemap_start_voltage_stimulating(
											(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,400,
											/*points/s gives 250 Hz waveform frequency*/
											(float)100000.,calibrate_voltage_2))
#endif /* defined (OLD_CODE) */
										{
											/* wait for the high-pass (DC removal) to settle */
											allow_to_settle(test_channel,tested_cards,
												temp_channel_check,phase_flag,report,
												&number_of_settled_channels,sampling_delay,samples,mean,
												number_of_samples,number_of_channels,tol_settling,
												max_settling);
											if (number_of_settled_channels<
												MAXIMUM_NUMBER_OF_NI_CARDS*
												NUMBER_OF_CHANNELS_ON_NI_CARD)
											{
												number_of_settled_channels=0;
												printf("Failed to settle for:");
												temp_c_number=0;
												for (k=0;k<number_of_test_cards;k++)
												{
													if (tested_cards[k])
													{
														for (i=0;i<max_channels;i++)
														{
															if (temp_channel_check[temp_c_number+i]&
																phase_flag)
															{
																printf(" %d",temp_c_number+i+1);
																channel_check[temp_c_number+i] |= phase_flag;
															}
															else
															{
																if (!(channel_check[temp_c_number+i]&
																	phase_flag))
																{
																	number_of_settled_channels++;
																}
															}
														}
													}
													temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
												}
												printf("\n");
												pause_for_error();
											}
											if (0<number_of_settled_channels)
											{
												if (0!=tested_card)
												{
													register_write_signal_file("phase5_3.sig",
														(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
												}
												temp_c_number=0;
												for (k=0;k<number_of_test_cards;k++)
												{
													if (tested_cards[k])
													{
														for (i=0;i<max_channels;i++)
														{
															if ((0==test_channel)||(i+1==test_channel))
															{
																if (!(channel_check[temp_c_number+i]&
																	phase_flag))
																{
																	rms=(float)0;
																	sample=samples+(temp_c_number+i);
																	for (j=(int)number_of_samples;j>0;j--)
																	{
																		temp=
																			(float)(*sample)-mean[temp_c_number+i];
																		rms += temp*temp;
																		sample += number_of_channels;
																	}
																	rms /= (float)number_of_samples;
																	rms_save[temp_c_number+i]=
																		(float)sqrt((double)rms);
																}
															}
														}
													}
													temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
												}
												pause_for_error();
												unemap_stop_stimulating(0);
												/* calculate sine wave */
												for (i=0;i<250;i++)
												{
													calibrate_voltage_1[i]=calibrate_amplitude_1*
														(float)sin(two_pi*(double)i/(double)250);
													calibrate_voltage_2[i]=calibrate_voltage_1[i];
												}
												channel_number_1=
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
												channel_number_2=
													(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
												if (unemap_load_voltage_stimulating(1,&channel_number_1,
													(int)250,/*points/s gives 400 Hz waveform frequency*/
													(float)100000.,calibrate_voltage_1)&&
													unemap_load_voltage_stimulating(1,&channel_number_2,
													(int)250,/*points/s gives 400 Hz waveform frequency*/
													(float)100000.,calibrate_voltage_2)&&
													unemap_start_stimulating())
#if defined (OLD_CODE)
												if (unemap_start_voltage_stimulating(
													(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,250,
													/*points/s gives 400 Hz waveform frequency*/
													(float)100000.,calibrate_voltage_1)&&
													unemap_start_voltage_stimulating(
													(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,250,
													/*points/s gives 400 Hz waveform frequency*/
													(float)100000.,calibrate_voltage_2))
#endif /* defined (OLD_CODE) */
												{
													/* wait for the high-pass (DC removal) to
														settle */
													allow_to_settle(test_channel,tested_cards,
														temp_channel_check,phase_flag,report,
														&number_of_settled_channels,sampling_delay,samples,
														mean,number_of_samples,number_of_channels,
														tol_settling,max_settling);
													if (number_of_settled_channels<
														MAXIMUM_NUMBER_OF_NI_CARDS*
														NUMBER_OF_CHANNELS_ON_NI_CARD)
													{
														number_of_settled_channels=0;
														printf("Failed to settle for:");
														temp_c_number=0;
														for (k=0;k<number_of_test_cards;k++)
														{
															if (tested_cards[k])
															{
																for (i=0;i<max_channels;i++)
																{
																	if (temp_channel_check[temp_c_number+i]&
																		phase_flag)
																	{
																		printf(" %d",temp_c_number+i+1);
																		channel_check[temp_c_number+i] |=
																			phase_flag;
																	}
																	else
																	{
																		if (!(channel_check[temp_c_number+i]&
																			phase_flag))
																		{
																			number_of_settled_channels++;
																		}
																	}
																}
															}
															temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
														}
														printf("\n");
														pause_for_error();
													}
													if (0<number_of_settled_channels)
													{
														if (0!=tested_card)
														{
															register_write_signal_file("phase5_4.sig",
																(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+
																1);
														}
														temp_c_number=0;
														for (k=0;k<number_of_test_cards;k++)
														{
															if (tested_cards[k])
															{
																for (i=0;i<max_channels;i++)
																{
																	if ((0==test_channel)||(i+1==test_channel))
																	{
																		fprintf(report,"channel %d",
																			temp_c_number+i+1);
																		if (channel_check[temp_c_number+i]&
																			phase_flag)
																		{
																			fprintf(report,
																				" <<< CHECK settling >>>\n");
																		}
																		else
																		{
																			fprintf(report,"\n");
																			rms=(float)0;
																			sample=samples+(temp_c_number+i);
																			for (j=(int)number_of_samples;j>0;j--)
																			{
																				temp=(float)(*sample)-
																					mean[temp_c_number+i];
																				rms += temp*temp;
																				sample += number_of_channels;
																			}
																			rms /= (float)number_of_samples;
																			rms=(float)sqrt((double)rms);
																			db_reduction=(float)20*
																				(float)log10((double)(rms_save[
																				temp_c_number+i]/rms));
																			fprintf(report,
																		"  250 Hz signal, 100 Hz filter, rms=%g\n",
																				rms_save[temp_c_number+i]);
																			fprintf(report,
																		"  400 Hz signal, 100 Hz filter, rms=%g\n",
																				rms);
																			fprintf(report,"  dB reduction=%g",
																				db_reduction);
#if defined (OLD_CODE)
																			if (!(fabs(db_reduction-db_per_decade*
																				log10(400./250.))<tol_db2))
#endif /* defined (OLD_CODE) */
																			if (!(fabs(db_reduction-
																				db_per_decade*log10(400./250.))<tol_db2*
																				db_per_decade*log10(400./250.)))
																			{
																				printf("channel %d\n",
																					temp_c_number+i+1);
																				printf(
																		"  250 Hz signal, 100 Hz filter, rms=%g\n",
																					rms_save[temp_c_number+i]);
																				printf(
																		"  400 Hz signal, 100 Hz filter, rms=%g\n",
																					rms);
																				printf(
																					"  dB reduction=%g <<< CHECK >>>\n",
																					db_reduction);
																				fprintf(report," <<< CHECK >>>");
																				channel_check[temp_c_number+i] |=
																					phase_flag;
																				total_checks++;
																				first++;
																			}
																			fprintf(report,"\n");
																		}
																	}
																}
																pause_for_error();
															}
															temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
														}
													}
												}
												else
												{
													printf(
														"Phase 5.  Could not start stimulation 3\n");
													fprintf(report,
										"Phase 5.  Could not start stimulation 3 <<< CHECK >>>\n");
													temp_c_number=0;
													for (k=0;k<number_of_test_cards;k++)
													{
														if (tested_cards[k])
														{
															for (i=0;i<max_channels;i++)
															{
																channel_check[temp_c_number+i] |= phase_flag;
															}
														}
														temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
													}
													total_checks++;
													first++;
												}
											}
										}
										else
										{
											printf("Phase 5.  Could not start stimulation 2\n");
											fprintf(report,
										"Phase 5.  Could not start stimulation 2 <<< CHECK >>>\n");
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														channel_check[temp_c_number+i] |= phase_flag;
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
											total_checks++;
											first++;
										}
										if (0==first)
										{
											printf("Anti-aliasing filter OK\n");
											fprintf(report,"Anti-aliasing filter OK\n");
										}
									}
									else
									{
										printf("Phase 5.  Could not start stimulation 1\n");
										fprintf(report,
										"Phase 5.  Could not start stimulation 1 <<< CHECK >>>\n");
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													channel_check[temp_c_number+i] |= phase_flag;
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										total_checks++;
									}
									/* turn off stimulation */
									unemap_stop_stimulating(0);
									unemap_set_channel_stimulating(0,0);
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 6 : Testing programmable gain");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 6 : Testing programmable gain\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_2)
												{
													tester_card_1=i+1;
												}
												else
												{
													tester_card_2=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
			"Even channels of TESTER_1 (%d) to stimulate.  Odd channels to record\n",
										tester_card_1);
									fprintf(report,
			"Odd channels of TESTER_2 (%d) to stimulate.  Even channels to record\n",
										tester_card_2);
									/* set the programmable gain on TEST (card 1) to 1, input a
										100 Hz sine wave, record, set the gain to 2, record,
										compute the gain, set the gain to 4, record, compute the
										gain, set the gain to 8, record and compute the gain:
										- set all channels of TEST to record
										- set even channels of GROUND (card 2) to record and odd
											channels to stimulate
										- set odd channels of SIGNAL (card 3) to record and even
											channels to stimulate */
									for (i=1;i<=max_channels;i++)
									{
										for (j=0;j<number_of_test_cards;j++)
										{
											if (tested_cards[j])
											{
												unemap_set_channel_stimulating(
													j*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
									}
									/* set low pass filter to 10 kHz */
									unemap_set_antialiasing_filter_frequency(0,(float)10000);
									unemap_get_sample_range(&minimum_sample_value,
										&maximum_sample_value);
									j=0;
									temp_c_number=1;
									for (k=0;k<number_of_test_cards;k++)
									{
										if (tested_cards[k])
										{
											if (0==j)
											{
												unemap_get_gain(temp_c_number,&pre_filter_gain,
													&post_filter_gain);
											}
											/* set the programmable gain to 1 */
											unemap_set_gain(temp_c_number,(float)1,post_filter_gain);
											if (0==j)
											{
												unemap_get_voltage_range(temp_c_number,
													&minimum_voltage,&maximum_voltage);
												j=1;
											}
										}
										temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
									}
									gain=(maximum_voltage-minimum_voltage)/
										(float)(maximum_sample_value-minimum_sample_value);
									/* use half of maximum */
									calibrate_amplitude_1=
										CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
									/* allow for maximum programmable gain */
									calibrate_amplitude_1 /= (float)8;
									/* calculate sine wave */
									two_pi=(double)8*atan((double)1);
									for (i=0;i<1000;i++)
									{
										calibrate_voltage_1[i]=calibrate_amplitude_1*
											(float)sin(two_pi*(double)i/(double)1000);
										calibrate_voltage_2[i]=calibrate_voltage_1[i];
									}
									for (i=0;i<number_of_test_cards*NUMBER_OF_CHANNELS_ON_NI_CARD;
										i++)
									{
										rms_save[i]=(float)-1;
									}
									channel_number_1=
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									channel_number_2=
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									if (unemap_load_voltage_stimulating(1,&channel_number_1,
										(int)1000,/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_load_voltage_stimulating(1,&channel_number_2,
										(int)1000,/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2)&&
										unemap_start_stimulating())
#if defined (OLD_CODE)
									if (unemap_start_voltage_stimulating(
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1000,
										/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_start_voltage_stimulating(
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1000,
										/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2))
#endif /* defined (OLD_CODE) */
									{
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											printf("Failed to settle for:");
											for (i=0;i<number_of_test_cards*
												NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
											{
												if (temp_channel_check[i]&phase_flag)
												{
													printf(" %d",i+1);
													channel_check[temp_c_number+i] |= phase_flag;
												}
											}
											printf("\n");
											pause_for_error();
										}
										/* calculate rms */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													if ((0==test_channel)||(i+1==test_channel))
													{
														fprintf(report,"channel %d",temp_c_number+i+1);
														if (temp_channel_check[temp_c_number+i]&
															phase_flag)
														{
															fprintf(report," <<< CHECK settling >>>");
														}
														else
														{
															rms=(float)0;
															sample=samples+(temp_c_number+i);
															for (j=(int)number_of_samples;j>0;j--)
															{
																temp=(float)(*sample)-mean[temp_c_number+i];
																rms += temp*temp;
																sample += number_of_channels;
															}
															rms /= (float)number_of_samples;
															rms_save[temp_c_number+i]=
																(float)sqrt((double)rms);
															fprintf(report,".  rms=%g",
																rms_save[temp_c_number+i]);
														}
														fprintf(report,"\n");
													}
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										if (0!=tested_card)
										{
											register_write_signal_file("phase6_1.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
										/* set the programmable gain to 2 */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												unemap_set_gain(temp_c_number+1,(float)2,
													post_filter_gain);
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										printf("Programmable gain is 2\n");
										fprintf(report,"Programmable gain is 2\n");
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											number_of_settled_channels=0;
											printf("Failed to settle for:");
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															printf(" %d",temp_c_number+i+1);
															channel_check[temp_c_number+i] |= phase_flag;
														}
														else
														{
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																number_of_settled_channels++;
															}
														}
													}
													printf("\n");
													pause_for_error();
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										/* calculate rms */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													if ((0==test_channel)||(i+1==test_channel))
													{
														fprintf(report,"channel %d",temp_c_number+i+1);
														if (temp_channel_check[temp_c_number+i]&
															phase_flag)
														{
															fprintf(report," <<< CHECK settling >>>");
														}
														else
														{
															rms=(float)0;
															sample=samples+(temp_c_number+i);
															for (j=(int)number_of_samples;j>0;j--)
															{
																temp=(float)(*sample)-mean[temp_c_number+i];
																rms += temp*temp;
																sample += number_of_channels;
															}
															rms /= (float)number_of_samples;
															rms=(float)sqrt((double)rms);
															fprintf(report,".  rms=%g",rms);
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																rms /= rms_save[temp_c_number+i];
																fprintf(report,".  gain=%g",rms);
																if (!(fabs(rms-2)<tol_gain*2))
																{
																	channel_check[temp_c_number+i] |= phase_flag;
																	fprintf(report," <<< CHECK >>>");
																	printf("channel %d.  gain=%g <<< CHECK >>>\n",
																		temp_c_number+i+1,rms);
																	total_checks++;
																}
															}
															else
															{
																fprintf(report,
																	" <<< CHECK gain=1 settling >>>");
															}
														}
														fprintf(report,"\n");
													}
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										if (0!=tested_card)
										{
											register_write_signal_file("phase6_2.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
										/* set the programmable gain to 4 */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												unemap_set_gain(temp_c_number+1,(float)4,
													post_filter_gain);
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										printf("Programmable gain is 4\n");
										fprintf(report,"Programmable gain is 4\n");
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											number_of_settled_channels=0;
											printf("Failed to settle for:");
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															printf(" %d",temp_c_number+i+1);
															channel_check[temp_c_number+i] |= phase_flag;
														}
														else
														{
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																number_of_settled_channels++;
															}
														}
													}
													printf("\n");
													pause_for_error();
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										/* calculate rms */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													if ((0==test_channel)||(i+1==test_channel))
													{
														fprintf(report,"channel %d",temp_c_number+i+1);
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															fprintf(report," <<< CHECK settling >>>");
														}
														else
														{
															rms=(float)0;
															sample=samples+(temp_c_number+i);
															for (j=(int)number_of_samples;j>0;j--)
															{
																temp=(float)(*sample)-mean[temp_c_number+i];
																rms += temp*temp;
																sample += number_of_channels;
															}
															rms /= (float)number_of_samples;
															rms=(float)sqrt((double)rms);
															fprintf(report,".  rms=%g",rms);
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																rms /= rms_save[temp_c_number+i];
																fprintf(report,".  gain=%g",rms);
																if (!(fabs(rms-4)<tol_gain*4))
																{
																	channel_check[temp_c_number+i] |= phase_flag;
																	fprintf(report," <<< CHECK >>>");
																	printf("channel %d.  gain=%g <<< CHECK >>>\n",
																		temp_c_number+i+1,rms);
																	total_checks++;
																}
															}
															else
															{
																fprintf(report,
																	" <<< CHECK gain=1 settling >>>");
															}
														}
														fprintf(report,"\n");
													}
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										if (0!=tested_card)
										{
											register_write_signal_file("phase6_4.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
										/* set the programmable gain to 8 */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												unemap_set_gain(temp_c_number+1,(float)8,
													post_filter_gain);
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										printf("Programmable gain is 8\n");
										fprintf(report,"Programmable gain is 8\n");
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											number_of_settled_channels=0;
											printf("Failed to settle for:");
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													for (i=0;i<max_channels;i++)
													{
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															printf(" %d",temp_c_number+i+1);
															channel_check[temp_c_number+i] |= phase_flag;
														}
														else
														{
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																number_of_settled_channels++;
															}
														}
													}
													printf("\n");
													pause_for_error();
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										/* calculate rms */
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													if ((0==test_channel)||(i+1==test_channel))
													{
														fprintf(report,"channel %d",temp_c_number+i+1);
														if (temp_channel_check[temp_c_number+i]&phase_flag)
														{
															fprintf(report," <<< CHECK settling >>>");
														}
														else
														{
															rms=(float)0;
															sample=samples+(temp_c_number+i);
															for (j=(int)number_of_samples;j>0;j--)
															{
																temp=(float)(*sample)-mean[temp_c_number+i];
																rms += temp*temp;
																sample += number_of_channels;
															}
															rms /= (float)number_of_samples;
															rms=(float)sqrt((double)rms);
															fprintf(report,".  rms=%g",rms);
															if (rms_save[temp_c_number+i]>=(float)0)
															{
																rms /= rms_save[temp_c_number+i];
																fprintf(report,".  gain=%g",rms);
																if (!(fabs(rms-8)<tol_gain*8))
																{
																	channel_check[temp_c_number+i] |= phase_flag;
																	fprintf(report," <<< CHECK >>>");
																	printf("channel %d.  gain=%g <<< CHECK >>>\n",
																		temp_c_number+i+1,rms);
																	total_checks++;
																}
															}
															else
															{
																fprintf(report,
																	" <<< CHECK gain=1 settling >>>");
															}
														}
														fprintf(report,"\n");
													}
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										if (0!=tested_card)
										{
											register_write_signal_file("phase6_8.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
									}
									else
									{
										printf("Phase 6.  Could not start stimulation\n");
										fprintf(report,
											"Phase 6.  Could not start stimulation <<< CHECK >>>\n");
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													channel_check[temp_c_number+i] |= phase_flag;
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										total_checks++;
									}
									temp_c_number=0;
									for (k=0;k<number_of_test_cards;k++)
									{
										if (tested_cards[k])
										{
											unemap_set_gain(temp_c_number+1,pre_filter_gain,
												post_filter_gain);
										}
										temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
									}
									/* turn off stimulation */
									unemap_stop_stimulating(0);
									unemap_set_channel_stimulating(0,0);
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 7 : Testing isolation");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 7 : Testing isolation\n");
								for (i=0;i<number_of_test_cards;i++)
								{
									card_used[i]=0;
								}
								phase_finished=0;
								do
								{
									for (i=0;i<number_of_test_cards;i++)
									{
										tested_cards[i]=0;
									}
									if (0==tested_card)
									{
										i=number_of_test_cards-1;
										tester_card_1=0;
										tester_card_2=0;
										while ((i>=0)&&!tester_card_1)
										{
											if (card_used[i])
											{
												if (tester_card_2)
												{
													tester_card_1=i+1;
												}
												else
												{
													tester_card_2=i+1;
												}
											}
											i--;
										}
										if (!tester_card_1)
										{
											tester_card_1=1;
											if (tester_card_2<=1)
											{
												tester_card_2=2;
											}
										}
										j=0;
										for (i=0;i<number_of_test_cards;i++)
										{
											if (card_used[i])
											{
												j++;
											}
											else
											{
												if ((i+1!=tester_card_1)&&(i+1!=tester_card_2))
												{
													tested_cards[i]=1;
													card_used[i]=1;
													j++;
												}
											}
										}
										if (j>=number_of_test_cards)
										{
											phase_finished=1;
										}
									}
									else
									{
										tested_cards[tested_card-1]=1;
										phase_finished=1;
									}
									fprintf(report,"All channels of TESTED (");
									j=0;
									for (i=0;i<number_of_test_cards;i++)
									{
										if (tested_cards[i])
										{
											if (j)
											{
												fprintf(report,",");
											}
											fprintf(report,"%d",i+1);
											j=1;
										}
									}
									fprintf(report,") to record\n");
									fprintf(report,
			"Even channels of TESTER_1 (%d) to stimulate.  Odd channels to record\n",
										tester_card_1);
									fprintf(report,
			"Odd channels of TESTER_2 (%d) to stimulate.  Even channels to record\n",
										tester_card_2);
									/* put TEST (card 1) in isolate mode, set the calibration
										signal for TEST to 0, input a 100 Hz sine wave and record:
										- put TEST in isolate mode, set all channels of TEST to
											record
										- set even channels of GROUND (card 2) to record and odd
											channels to stimulate
										- set odd channels of SIGNAL (card 3) to record and even
											channels to stimulate */
									for (k=0;k<number_of_test_cards;k++)
									{
										if (tested_cards[k])
										{
											unemap_set_isolate_record_mode(
												k*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1);
										}
									}
									for (i=1;i<=max_channels;i++)
									{
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												unemap_set_channel_stimulating(
													k*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											}
										}
										if (0==i%2)
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
										}
										else
										{
											unemap_set_channel_stimulating(
												(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,1);
											unemap_set_channel_stimulating(
												(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+i,0);
										}
									}
									/* set low pass filter to 10 kHz */
									unemap_set_antialiasing_filter_frequency(0,(float)10000);
									unemap_get_sample_range(&minimum_sample_value,
										&maximum_sample_value);
									unemap_get_voltage_range(1,&minimum_voltage,&maximum_voltage);
									gain=(maximum_voltage-minimum_voltage)/
										(float)(maximum_sample_value-minimum_sample_value);
									/* use half of maximum */
									calibrate_amplitude_1=
										CALIBRATE_AMPLITUDE_FACTOR*maximum_voltage;
									/* calculate sine wave */
									two_pi=(double)8*atan((double)1);
									for (i=0;i<1000;i++)
									{
										calibrate_voltage_1[i]=calibrate_amplitude_1*
											(float)sin(two_pi*(double)i/(double)1000);
										calibrate_voltage_2[i]=calibrate_voltage_1[i];
									}
									channel_number_1=
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									channel_number_2=
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
									if (unemap_load_voltage_stimulating(1,&channel_number_1,
										(int)1000,/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_load_voltage_stimulating(1,&channel_number_2,
										(int)1000,/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2)&&
										unemap_start_stimulating())
#if defined (OLD_CODE)
									if (unemap_start_voltage_stimulating(
										(tester_card_1-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1000,
										/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_1)&&
										unemap_start_voltage_stimulating(
										(tester_card_2-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1,1000,
										/*points/s gives 100 Hz waveform frequency*/
										(float)100000.,calibrate_voltage_2))
#endif /* defined (OLD_CODE) */
									{
										/* wait for the high-pass (DC removal) to settle */
										allow_to_settle(test_channel,tested_cards,
											temp_channel_check,phase_flag,report,
											&number_of_settled_channels,sampling_delay,samples,mean,
											number_of_samples,number_of_channels,tol_settling,
											max_settling);
										if (number_of_settled_channels<
											MAXIMUM_NUMBER_OF_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD)
										{
											printf("Failed to settle for:");
											for (i=0;i<number_of_test_cards*
												NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
											{
												if (temp_channel_check[i]&phase_flag)
												{
													printf(" %d",i+1);
													channel_check[i] |= phase_flag;
												}
											}
											printf("\n");
											pause_for_error();
										}
										if (0==test_channel)
										{
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													first=0;
													number_of_settled_channels=0;
													for (i=0;i<max_channels;i++)
													{
														if (!(temp_channel_check[temp_c_number+i]&
															phase_flag))
														{
															sorted_mean[number_of_settled_channels]=
																mean[temp_c_number+i];
															number_of_settled_channels++;
														}
													}
													if (0<number_of_settled_channels)
													{
														/* check offsets */
														fprintf(report,"Offsets:\n");
														qsort(sorted_mean,number_of_settled_channels,
															sizeof(float),compare_float);
														for (i=0;i<max_channels;i++)
														{
															fprintf(report,"%2d",temp_c_number+i+1);
															if (temp_channel_check[temp_c_number+i]&
																phase_flag)
															{
																channel_check[temp_c_number+i] |= phase_flag;
																fprintf(report," <<< CHECK settling >>>");
															}
															else
															{
																fprintf(report," " FLOAT_FORMAT,
																	mean[temp_c_number+i]);
																if (!((float)fabs((double)(
																	mean[temp_c_number+i]-sorted_mean[
																	number_of_settled_channels/2]))<
																	tol_offset_spread))
																{
																	channel_check[temp_c_number+i] |= phase_flag;
																	fprintf(report," <<< CHECK too large >>>");
																	total_checks++;
																	if (0==first)
																	{
																		printf("Offset spread too large for :\n");
																	}
																	first++;
																	printf("%2d " FLOAT_FORMAT,temp_c_number+i+1,
																		mean[temp_c_number+i]);
																	if (0==first%4)
																	{
																		printf("\n");
																	}
																	else
																	{
																		printf(" *  ");
																	}
																}
															}
															fprintf(report,"\n");
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														fprintf(report,"Offset median %g",
															sorted_mean[number_of_settled_channels/2]);
														if (!((float)fabs((double)(sorted_mean[
															number_of_settled_channels/2]))<tol_offset))
														{
															fprintf(report," <<< CHECK >>>");
															total_checks++;
															printf("Offset median %g is too large\n",
																sorted_mean[number_of_settled_channels/2]);
															first++;
														}
														fprintf(report,"\n");
														if (0==first)
														{
															printf("Offsets OK\n");
															fprintf(report,"Offsets OK\n");
														}
														else
														{
															pause_for_error();
														}
														/* check rms */
														fprintf(report,"RMS:\n");
														first=0;
														for (i=0;i<max_channels;i++)
														{
															rms=(float)0;
															sample=samples+(temp_c_number+i);
															for (j=(int)number_of_samples;j>0;j--)
															{
																temp=(float)(*sample)-mean[temp_c_number+i];
																rms += temp*temp;
																sample += number_of_channels;
															}
															rms /= (float)number_of_samples;
															rms=(float)sqrt((double)rms);
															fprintf(report,"%2d " FLOAT_FORMAT,
																temp_c_number+i+1,rms);
															if (!(fabs(rms)<tol_isolation))
															{
																channel_check[temp_c_number+i] |= phase_flag;
																fprintf(report," <<< CHECK >>>");
																total_checks++;
																if (0==first)
																{
																	printf("RMS too large for :\n");
																}
																first++;
																printf("%2d " FLOAT_FORMAT,temp_c_number+i+1,
																	rms);
																if (0==first%4)
																{
																	printf("\n");
																}
																else
																{
																	printf(" *  ");
																}
															}
															fprintf(report,"\n");
														}
														if (0!=first%4)
														{
															printf("\n");
														}
														if (0==first)
														{
															printf("Noise OK\n");
															fprintf(report,"Noise OK\n");
														}
														else
														{
															pause_for_error();
														}
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										else
										{
											i=test_channel-1;
											temp_c_number=0;
											for (k=0;k<number_of_test_cards;k++)
											{
												if (tested_cards[k])
												{
													if (temp_channel_check[temp_c_number+i]&phase_flag)
													{
														channel_check[temp_c_number+i] |= phase_flag;
														fprintf(report,"<<< CHECK to settle >>>\n");
													}
													else
													{
														/* check offset */
														fprintf(report,"Offset: %g",mean[temp_c_number+i]);
														printf("Offset: %g",mean[temp_c_number+i]);
														if ((float)fabs((double)(mean[temp_c_number+i]))<
															tol_offset)
														{
															fprintf(report," OK");
															printf(" OK");
														}
														else
														{
															channel_check[temp_c_number+i] |= phase_flag;
															fprintf(report," <<< CHECK too large >>>");
															printf(" too large");
															total_checks++;
															pause_for_error();
														}
														fprintf(report,"\n");
														printf("\n");
														/* check rms */
														rms=(float)0;
														sample=samples+(temp_c_number+i);
														for (j=(int)number_of_samples;j>0;j--)
														{
															temp=(float)(*sample)-mean[temp_c_number+i];
															rms += temp*temp;
															sample += number_of_channels;
														}
														rms /= (float)number_of_samples;
														rms=(float)sqrt((double)rms);
														fprintf(report,"RMS: " FLOAT_FORMAT,rms);
														printf("RMS: " FLOAT_FORMAT,rms);
														if ((float)fabs((double)rms)<tol_isolation)
														{
															fprintf(report," OK");
															printf(" OK");
														}
														else
														{
															channel_check[temp_c_number+i] |= phase_flag;
															fprintf(report," <<< CHECK too large >>>");
															printf(" too large");
															total_checks++;
															pause_for_error();
														}
														fprintf(report,"\n");
														printf("\n");
													}
												}
												temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
											}
										}
										if (0!=tested_card)
										{
											register_write_signal_file("phase7.sig",
												(tested_card-1)*NUMBER_OF_CHANNELS_ON_NI_CARD+1);
										}
									}
									else
									{
										printf("Phase 7.  Could not start stimulation 1\n");
										fprintf(report,
											"Phase 7.  Could not start stimulation <<< CHECK >>>\n");
										temp_c_number=0;
										for (k=0;k<number_of_test_cards;k++)
										{
											if (tested_cards[k])
											{
												for (i=0;i<max_channels;i++)
												{
													channel_check[temp_c_number+i] |= phase_flag;
												}
											}
											temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
										}
										total_checks++;
									}
									/* turn off stimulation */
									unemap_stop_stimulating(0);
									unemap_set_channel_stimulating(0,0);
									temp_c_number=0;
									for (k=0;k<number_of_test_cards;k++)
									{
										if (tested_cards[k])
										{
											unemap_set_isolate_record_mode(temp_c_number+1,0);
										}
										temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
									}
								} while (!phase_finished);
							}
							phase_flag <<= 1;
							if (!(('q'==phase_option)||('Q'==phase_option)))
							{
								printf("Phase 8 : Testing calibration");
								if (!(('a'==phase_option)||('A'==phase_option)))
								{
									printf(" ? (y/n/a/q) ");
									do
									{
										scanf("%c",&phase_option);
									} while (!isalnum(phase_option));
								}
								else
								{
									printf("\n");
								}
							}
							if (('y'==phase_option)||('Y'==phase_option)||
								('a'==phase_option)||('A'==phase_option))
							{
								fprintf(report,"Phase 8 : Testing calibration\n");
								calibrating=1;
								unemap_calibrate(calibration_finished,(void *)report);
								/* waiting for calibration to finish */
#if defined (MOTIF)
								if (application_context)
								{
									while (calibrating)
									{
/*										printf(".");
										fflush(stdout);*/
										XtAppProcessEvent(application_context,XtIMAlternateInput);
									}
								}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
								while (calibrating)
								{
									sleep((unsigned)1);
									printf(".");
									fflush(stdout);
								}
#endif /* defined (WINDOWS) */
								temp_c_number=0;
								first=0;
								for (k=0;k<number_of_test_cards;k++)
								{
									if ((0==tested_card)||(k+1==tested_card))
									{
										for (i=0;i<max_channels;i++)
										{
											if ((0==test_channel)||(i+1==test_channel))
											{
												channel_check[temp_c_number+i] |= phase_flag;
												first++;
											}
										}
									}
									temp_c_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
								if ((0<calibrating_number_of_channels)&&
									calibrating_channel_numbers&&calibrating_channel_offsets&&
									calibrating_channel_gains)
								{
									unemap_get_sample_range(&minimum_sample_value,
										&maximum_sample_value);
									fprintf(report,"channel  offset  gain\n");
									for (j=0;j<calibrating_number_of_channels;j++)
									{
										temp_c_number=calibrating_channel_numbers[j];
										if (((0==test_channel)||((temp_c_number-1)%
											NUMBER_OF_CHANNELS_ON_NI_CARD+1==test_channel))&&
											((0==tested_card)||((temp_c_number-1)/
											NUMBER_OF_CHANNELS_ON_NI_CARD+1==tested_card)))
										{
											fprintf(report,"%d %g %g",temp_c_number,
												calibrating_channel_offsets[j],
												calibrating_channel_gains[j]);
											unemap_get_voltage_range(temp_c_number,&minimum_voltage,
												&maximum_voltage);
											gain=(maximum_voltage-minimum_voltage)/
												(float)(maximum_sample_value-minimum_sample_value);
											if ((float)fabs((double)(gain-
												calibrating_channel_gains[j]))<
												tol_calibrate_gain*gain)
											{
												channel_check[temp_c_number-1] &= ~phase_flag;
												first--;
											}
											else
											{
												fprintf(report," <<< CHECK target gain %g >>>",gain);
												printf("%d %g %g <<< CHECK target gain %g >>>\n",
													temp_c_number,calibrating_channel_offsets[j],
													calibrating_channel_gains[j],gain);
												channel_check[temp_c_number-1] |= phase_flag;
												total_checks++;
											}
											fprintf(report,"\n");
										}
									}
								}
								if (0==first)
								{
									printf("Calibration OK\n");
									fprintf(report,"Calibration OK\n");
								}
								else
								{
									printf("<<< CHECK Calibration >>>\n");
									fprintf(report,"<<< CHECK Calibration >>>\n");
									pause_for_error();
								}
							}
							phase_flag <<= 1;
							/* turn off stimulation */
							unemap_stop_stimulating(0);
							unemap_set_isolate_record_mode(0,0);
							if (0==total_checks)
							{
								printf("Passed test(s)\n");
								fprintf(report,"Passed test(s)\n");
							}
							else
							{
								printf("%d check(s) required.  See report.txt\n",
									total_checks);
							}
							i=number_of_test_cards*NUMBER_OF_CHANNELS_ON_NI_CARD-1;
							while ((i>=0)&&(!channel_check[i]))
							{
								i--;
							}
							if (i>=0)
							{
								printf("Check channels:\n");
								fprintf(report,"Check channels:\n");
								for (i=0;i<number_of_test_cards*NUMBER_OF_CHANNELS_ON_NI_CARD;
									i++)
								{
									if (channel_check[i])
									{
										printf("%2d.  Phase(s)",i+1);
										fprintf(report,"%2d.  Phase(s)",i+1);
										phase_flag=0x1;
										for (j=0;j<=8;j++)
										{
											if (channel_check[i]&phase_flag)
											{
												printf(" %d",j);
												fprintf(report," %d",j);
											}
											phase_flag <<= 1;
										}
										printf("\n");
										fprintf(report,"\n");
									}
								}
							}
							fclose(report);
						}
						else
						{
							printf("Could not open report.txt\n");
							pause_for_error();
						}
					}
					else
					{
						printf("Need at least 4 cards for auto-testing\n");
						pause_for_error();
					}
					unemap_stop_stimulating(0);
					unemap_set_channel_stimulating(0,0);
					unemap_set_isolate_record_mode(0,1);
					unemap_set_power(0);
				} break;
				case 'b':
				{
					if (unemap_get_power(&i))
					{
						if (i)
						{
							unemap_set_isolate_record_mode(0,1);
							unemap_set_power(0);
						}
						else
						{
							unemap_set_power(1);
							unemap_set_isolate_record_mode(0,1);
						}
					}
				} break;
				case 'c':
				{
					float *values,values_per_second;
					int constant_voltage,number_of_values;

					/* stimulate */
					if (!stimulate_DA_on[(channel_number-1)%
						NUMBER_OF_CHANNELS_ON_NI_CARD])
					{
						printf(
							"Stimulate channel (1-64) or all (0) or none (otherwise) ? ");
						scanf("%d",&test_channel);
						if ((1<=test_channel)&&(test_channel<=64))
						{
							for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
							{
								unemap_set_channel_stimulating(channel_number+i,0);
							}
							unemap_set_channel_stimulating(channel_number+test_channel-1,
								1);
						}
						else
						{
							if (0==test_channel)
							{
								for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
								{
									unemap_set_channel_stimulating(channel_number+i,1);
								}
							}
						}
						if (unemap_read_waveform_file("stimulate.wfm",&number_of_values,
							&values_per_second,&values,&constant_voltage))
						{
							if (constant_voltage)
							{
								printf("Voltage stimulation\n");
								if (unemap_load_voltage_stimulating(1,&channel_number,
									number_of_values,values_per_second,values)&&
									unemap_start_stimulating())
#if defined (OLD_CODE)
								if (unemap_start_voltage_stimulating(channel_number,
									number_of_values,values_per_second,values))
#endif /* defined (OLD_CODE) */
								{
									stimulate_DA_on[(channel_number-1)%
										NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
								}
								else
								{
									printf("Could not start voltage stimulation\n");
								}
							}
							else
							{
								printf("Current stimulation\n");
								if (unemap_load_current_stimulating(1,&channel_number,
									number_of_values,values_per_second,values)&&
									unemap_start_stimulating())
#if defined (OLD_CODE)
								if (unemap_start_current_stimulating(channel_number,
									number_of_values,values_per_second,values))
#endif /* defined (OLD_CODE) */
								{
									stimulate_DA_on[(channel_number-1)%
										NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
								}
								else
								{
									printf("Could not start current stimulation\n");
								}
							}
							DEALLOCATE(values);
						}
						else
						{
							printf("Could not read stimulate.wfm\n");
						}
					}
					else
					{
						/* turn off stimulation */
						unemap_stop_stimulating(channel_number);
						for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
						{
							unemap_set_channel_stimulating(channel_number+i,0);
						}
						stimulate_DA_on[(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]=0;
					}
				} break;
				case 'd':
				{
					calibrating=1;
					unemap_calibrate(calibration_finished,(void *)NULL);
					/* waiting for calibration to finish */
					while (calibrating)
					{
						sleep((unsigned)1);
					}
					register_write_signal_file("calibrate.sig",1);
					if ((0<calibrating_number_of_channels)&&
						calibrating_channel_numbers&&calibrating_channel_offsets&&
						calibrating_channel_gains&&
						(channel_number+NUMBER_OF_CHANNELS_ON_NI_CARD-1<=
						calibrating_number_of_channels))
					{
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"channel  offset  gain\n");
		for (i=0;i<calibrating_number_of_channels;i++)
		{
			fprintf(debug_nidaq,"%d %g %g\n",calibrating_channel_numbers[i],
				calibrating_channel_offsets[i],calibrating_channel_gains[i]);
		}
	}
}
						printf("channel  offset  gain\n");
						for (i=channel_number-1;
							i<channel_number+NUMBER_OF_CHANNELS_ON_NI_CARD-1;i++)
						{
							printf("%d %g %g\n",calibrating_channel_numbers[i],
								calibrating_channel_offsets[i],
								calibrating_channel_gains[i]);
						}
					}
				} break;
				case 'e':
				{
					float *values,values_per_second;
					int constant_voltage,number_of_values;

					/* calibrate */
					if (!calibrate_DA_on[(channel_number-1)%
						NUMBER_OF_CHANNELS_ON_NI_CARD])
					{
						if (unemap_read_waveform_file("calibrate.wfm",&number_of_values,
							&values_per_second,&values,&constant_voltage))
						{
							if (constant_voltage)
							{
								if (unemap_start_calibrating(channel_number,number_of_values,
									values_per_second,values))
								{
									calibrate_DA_on[(channel_number-1)%
										NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
								}
								else
								{
									printf("Could not start calibration\n");
								}
							}
							else
							{
								printf("Can't do current calibration\n");
							}
							DEALLOCATE(values);
						}
						else
						{
							printf("Could not read calibrate.wfm\n");
						}
					}
					else
					{
						/* turn off calibration */
						unemap_stop_calibrating(channel_number);
						calibrate_DA_on[(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]=0;
					}
				} break;
				case 'f':
				{
					/* set power up filter frequency */
					unemap_set_powerup_antialiasing_filter_frequency(channel_number);
				} break;
				case 'g':
				{
					/* set NI gain */
					unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
					printf("Gain ? ");
					scanf("%f",&gain);
					unemap_get_card_state(channel_number,&battA_state,&battGood_state,
						&filter_frequency,&filter_taps,shift_registers,&GA0_state,
						&GA1_state,&NI_gain,&input_mode,&polarity,&tol_settling,
						&sampling_interval,&settling_step_max);
					post_filter_gain *= gain/(float)NI_gain;
					unemap_set_gain(channel_number,pre_filter_gain,post_filter_gain);
				} break;
				case 'h':
				{
					/* shutdown SCU PC */
					unemap_set_isolate_record_mode(0,1);
					unemap_set_power(0);
					unemap_deconfigure();
					unemap_shutdown();
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

/*
Global functions
----------------
*/
void main(void)
{
	float sampling_frequency;
	int channel_number,number_of_channels;
	long maximum_sample_value,minimum_sample_value;
	short int *samples;
	unsigned long number_of_samples,sampling_delay;
#if defined (MOTIF)
	struct Process_keyboard_data process_keyboard_data;
#endif /* defined (MOTIF) */

	/* initialize */
	pause_for_error_option='y';
	number_of_samples=(unsigned long)5000;
	sampling_frequency=(float)5000;
	/* to distinguish between 12 and 16 bit cards */
	if (unemap_get_sample_range(&minimum_sample_value,&maximum_sample_value)&&
		(4096<maximum_sample_value))
	{
		number_of_samples=(unsigned long)1000;
		sampling_frequency=(float)1000;
	}
#if defined (OLD_CODE)
/*???DB.  Accuracy seems to be dependent on this */
	/*???DB.  Temporary */
	{
		FILE *number_of_samples_file;

		if (number_of_samples_file=fopen("samples.txt","r"))
		{
			if (1==fscanf(number_of_samples_file," number_of_samples = %d ",
				&number_of_samples))
			{
				/* make a multiple of 1000 */
				if (number_of_samples<=0)
				{
					number_of_samples=1;
				}
				number_of_samples=1000*(((number_of_samples-1)/1000)+1);
				if (1==fscanf(number_of_samples_file," sampling_frequency = %f ",
					&sampling_frequency))
				{
				}
			}
			fclose(number_of_samples_file);
		}
	}
#endif /* defined (OLD_CODE) */
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
			(Unemap_hardware_callback *)NULL,(void *)NULL,(float)0,1)&&
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
			pause_for_error();
		}
#if defined (MOTIF)
	}
#endif /* defined (MOTIF) */
} /* main */
