/*******************************************************************************
FILE : test_unemap_hardware.c

LAST MODIFIED : 16 November 2001

DESCRIPTION :
For testing the unemap hardware software (client, server, standalone).
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "general/debug.h"
#include "unemap/unemap_hardware.h"

/* fileno is not ANSI */
extern int fileno(FILE *);

/*
Module variables
----------------
*/
int number_of_scrolling_callbacks=0;

#if defined (MOTIF)
	XtAppContext application_context;
#endif /* defined (MOTIF) */

float *calibrating_channel_gains=(float *)NULL,
	*calibrating_channel_offsets=(float *)NULL;
int calibrating=0,*calibrating_channel_numbers=(int *)NULL,
	calibrating_number_of_channels;

/*
Module functions
----------------
*/
static void scrolling_callback(int number_of_channels,int *channel_numbers,
	int number_of_values_per_channel,short *values,void *user_data)
{
	int i,j;

	USE_PARAMETER(user_data);
	number_of_scrolling_callbacks++;
	if (number_of_scrolling_callbacks<=5)
	{
		printf("scrolling_callback %d.  %d %p %d %p\n",
			number_of_scrolling_callbacks,number_of_channels,channel_numbers,
			number_of_values_per_channel,values);
		if ((0<number_of_channels)&&channel_numbers)
		{
			for (i=0;i<number_of_channels;i++)
			{
				printf("  %d",channel_numbers[i]);
				if ((0<number_of_values_per_channel)&&values)
				{
					printf(" ");
					for (j=0;j<number_of_values_per_channel;j++)
					{
						printf(" %d",values[i*number_of_values_per_channel+j]);
					}
				}
				printf("\n");
			}
		}
	}
	if (channel_numbers)
	{
		free(channel_numbers);
	}
	if (values)
	{
		free(values);
	}
} /* scrolling_callback */

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
	USE_PARAMETER(dummy_user_data);
	calibrating_number_of_channels=number_of_channels;
	calibrating_channel_numbers=(int *)channel_numbers;
	calibrating_channel_offsets=(float *)channel_offsets;
	calibrating_channel_gains=(float *)channel_gains;
	calibrating=0;
} /* calibration_finished */

int unemap_get_samples_acquired_background_finished=0;

static void acquired_data_callback(const int channel_number,
	const int number_of_samples,const short *samples,void *user_data)
{
	printf("unemap_get_samples_acquired_background finished.  %d %d %p %p\n",
		channel_number,number_of_samples,samples,user_data);
	unemap_get_samples_acquired_background_finished=1;
} /* acquired_data_callback */

static void print_menu(void)
{
	printf("0) Exit  ");
	printf("1) unemap_calibrate  ");
	printf("2) unemap_channel_valid_for_stimulator\n");
	printf("3) unemap_clear_scrolling_channels  ");
	printf("4) unemap_configure  ");
	printf("5) unemap_configured\n");
	printf("6) unemap_deconfigure  ");
	printf("7) unemap_get_antialiasing_filter_frequency\n");
	printf("8) unemap_get_channel_stimulating  ");
	printf("9) unemap_get_gain\n");
	printf("a) unemap_get_hardware_version  ");
	printf("b) unemap_get_isolate_record_mode\n");
	printf("c) unemap_get_maximum_number_of_samples  ");
	printf("d) unemap_get_number_of_channels\n");
	printf("e) unemap_get_number_of_samples_acquired  ");
	printf("f) unemap_get_number_of_stimulators\n");
	printf("g) unemap_get_power  ");
	printf("h) unemap_get_sample_range  ");
	printf("i) unemap_get_samples_acquired\n");
	printf("j) unemap_get_samples_acquired_background  ");
	printf("k) unemap_get_sampling\n");
	printf("l) unemap_get_sampling_frequency  ");
	printf("m) unemap_get_voltage_range\n");
	printf("n) unemap_load_current_stimulating  ");
	printf("o) unemap_load_voltage_stimulating\n");
	printf("p) unemap_set_antialiasing_filter_frequency  ");
	printf("q) unemap_set_channel_stimulating\n");
	printf("r) unemap_set_gain  ");
	printf("s) unemap_set_isolate_record_mode  ");
	printf("t) unemap_set_power\n");
	printf("u) unemap_set_powerup_antialiasing_filter_frequency\n");
	printf("v) unemap_set_scrolling_channel  ");
	printf("w) unemap_shutdown\n");
	printf("x) unemap_start_calibrating  ");
	printf("y) unemap_start_sampling\n");
	printf("z) unemap_start_scrolling  ");
	printf("A) unemap_start_stimulating\n");
	printf("B) unemap_stop_calibrating  ");
	printf("C) unemap_stop_sampling  ");
	printf("D) unemap_stop_scrolling\n");
	printf("E) unemap_stop_stimulating  ");
	printf("F) unemap_write_samples_acquired  ");
	printf("?\n");
} /* print_menu */

static void process_keyboard(
#if defined (WINDOWS)
	void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer dummy_client_data,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
{
	char option;
	int return_code;

	USE_PARAMETER(dummy_client_data);
	USE_PARAMETER(source);
	USE_PARAMETER(id);
	scanf("%c",&option);
	if (isalnum(option))
	{
		if ('0'!=option)
		{
			switch (option)
			{
				case '1':
				{
					int i;

					calibrating=1;
					return_code=unemap_calibrate(calibration_finished,(void *)NULL);
					printf("return_code=%d\n",return_code);
					if (return_code)
					{
						printf("calibrating ");
						fflush(stdout);
						/* waiting for calibration to finish */
#if defined (MOTIF)
						if (application_context)
						{
							while (calibrating)
							{
								printf(".");
								fflush(stdout);
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
						if ((0<calibrating_number_of_channels)&&
							calibrating_channel_numbers&&calibrating_channel_offsets&&
							calibrating_channel_gains)
						{
							printf(" succeeded\n");
							printf("channel  offset  gain\n");
							for (i=1;i<calibrating_number_of_channels;i++)
							{
								printf("%d %g %g\n",calibrating_channel_numbers[i],
									calibrating_channel_offsets[i],
									calibrating_channel_gains[i]);
							}
						}
						else
						{
							printf(" failed\n");
						}
					}
					else
					{
						printf("Failed to start\n");
					}
				} break;
				case '2':
				{
					int channel_number,stimulator_number;

					printf("stimulator_number ? ");
					scanf(" %d",&stimulator_number);
					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_channel_valid_for_stimulator(stimulator_number,
						channel_number);
					printf("return_code=%d\n",return_code);
				} break;
				case '3':
				{
					return_code=unemap_clear_scrolling_channels();
					printf("return_code=%d\n",return_code);
				} break;
				case '4':
				{
					float sampling_frequency;
					int number_of_samples,synchronization_card;

					sampling_frequency=(float)1000;
					number_of_samples=1000;
					printf("sampling_frequency ? ");
					scanf(" %f",&sampling_frequency);
					printf("number_of_samples ? ");
					scanf(" %d",&number_of_samples);
					printf("synchronization_card ? ");
					scanf(" %d",&synchronization_card);
					return_code=unemap_configure(sampling_frequency,number_of_samples,
#if defined (WINDOWS)
						(HWND)NULL,0,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
						application_context,
#endif /* defined (MOTIF) */
						scrolling_callback,(void *)NULL,(float)5,synchronization_card);
					printf("return_code=%d\n",return_code);
				} break;
				case '5':
				{
					return_code=unemap_configured();
					printf("return_code=%d\n",return_code);
				} break;
				case '6':
				{
					return_code=unemap_deconfigure();
					printf("return_code=%d\n",return_code);
				} break;
				case '7':
				{
					float frequency;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_get_antialiasing_filter_frequency(channel_number,
						&frequency);
					printf("return_code=%d\n",return_code);
					printf("frequency=%g\n",frequency);
				} break;
				case '8':
				{
					int channel_number,stimulating;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_get_channel_stimulating(channel_number,
						&stimulating);
					printf("return_code=%d\n",return_code);
					printf("stimulating=%d\n",stimulating);
				} break;
				case '9':
				{
					float pre_filter_gain,post_filter_gain;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_get_gain(channel_number,&pre_filter_gain,
						&post_filter_gain);
					printf("return_code=%d\n",return_code);
					printf("pre_filter_gain=%g\n",pre_filter_gain);
					printf("post_filter_gain=%g\n",post_filter_gain);
				} break;
				case 'a':
				{
					int hardware_version;

					return_code=unemap_get_hardware_version(&hardware_version);
					printf("return_code=%d\n",return_code);
					printf(
					"hardware_version=%d (%d=UnEmap_1V2, %d=UnEmap_2V1, %d=UnEmap_2V2)\n",
						hardware_version,UnEmap_1V2,UnEmap_2V1,UnEmap_2V2);
				} break;
				case 'b':
				{
					int channel_number,isolate;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_get_isolate_record_mode(channel_number,&isolate);
					printf("return_code=%d\n",return_code);
					printf("isolate=%d\n",isolate);
				} break;
				case 'c':
				{
					unsigned long number_of_samples;

					return_code=unemap_get_maximum_number_of_samples(
						&number_of_samples);
					printf("return_code=%d\n",return_code);
					printf("number_of_samples=%lu\n",number_of_samples);
				} break;
				case 'd':
				{
					int number_of_channels;

					return_code=unemap_get_number_of_channels(&number_of_channels);
					printf("return_code=%d\n",return_code);
					printf("number_of_channels=%d\n",number_of_channels);
				} break;
				case 'e':
				{
					unsigned long number_of_samples;

					return_code=unemap_get_number_of_samples_acquired(
						&number_of_samples);
					printf("return_code=%d\n",return_code);
					printf("number_of_samples=%lu\n",number_of_samples);
				} break;
				case 'f':
				{
					int number_of_stimulators;

					return_code=unemap_get_number_of_stimulators(&number_of_stimulators);
					printf("return_code=%d\n",return_code);
					printf("number_of_stimulators=%d\n",number_of_stimulators);
				} break;
				case 'g':
				{
					int on;

					return_code=unemap_get_power(&on);
					printf("return_code=%d\n",return_code);
					printf("on=%d\n",on);
				} break;
				case 'h':
				{
					long maximum_sample_value,minimum_sample_value;

					return_code=unemap_get_sample_range(&minimum_sample_value,
						&maximum_sample_value);
					printf("return_code=%d\n",return_code);
					printf("minimum_sample_value=%ld\n",minimum_sample_value);
					printf("maximum_sample_value=%ld\n",maximum_sample_value);
				} break;
				case 'i':
				{
					int channel_number,number_of_channels;
					short *samples;
					unsigned long number_of_samples;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					unemap_get_number_of_samples_acquired(&number_of_samples);
					if (0==channel_number)
					{
						unemap_get_number_of_channels(&number_of_channels);
						number_of_samples *= number_of_channels;
					}
					if (samples=(short *)malloc(sizeof(short)*number_of_samples))
					{
						return_code=unemap_get_samples_acquired(channel_number,samples);
						printf("return_code=%d\n",return_code);
						free(samples);
					}
					else
					{
						printf("Could not allocate samples %lu\n",number_of_samples);
					}
				} break;
				case 'j':
				{
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					unemap_get_samples_acquired_background_finished=0;
					return_code=unemap_get_samples_acquired_background(channel_number,
						acquired_data_callback,(void *)NULL);
					printf("return_code=%d\n",return_code);
					if (return_code)
					{
						printf("retrieving ");
						fflush(stdout);
						/* waiting for retrieval to finish */
#if defined (MOTIF)
						if (application_context)
						{
							while (!unemap_get_samples_acquired_background_finished)
							{
								printf(".");
								fflush(stdout);
								XtAppProcessEvent(application_context,XtIMAlternateInput);
							}
						}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
						while (!unemap_get_samples_acquired_background_finished)
						{
							sleep((unsigned)1);
							printf(".");
							fflush(stdout);
						}
#endif /* defined (WINDOWS) */
						printf("\n");
					}
				} break;
				case 'k':
				{
					return_code=unemap_get_sampling();
					printf("return_code=%d\n",return_code);
				} break;
				case 'l':
				{
					float frequency;

					return_code=unemap_get_sampling_frequency(&frequency);
					printf("return_code=%d\n",return_code);
					printf("frequency=%g\n",frequency);
				} break;
				case 'm':
				{
					float maximum_voltage,minimum_voltage;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_get_voltage_range(channel_number,&minimum_voltage,
						&maximum_voltage);
					printf("return_code=%d\n",return_code);
					printf("minimum_voltage=%g\n",minimum_voltage);
					printf("maximum_voltage=%g\n",maximum_voltage);
				} break;
				case 'n':
				{
					char file_name[121];
					float *values,values_per_second;
					int channel_number,constant_voltage,number_of_values;
					unsigned int number_of_cycles;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("Waveform file ? ");
					scanf(" %s",file_name);
					printf("number_of_cycles ? ");
					scanf(" %u",&number_of_cycles);
					if (unemap_read_waveform_file((FILE *)NULL,file_name,
						&number_of_values,&values_per_second,&values,&constant_voltage))
					{
						if (!constant_voltage)
						{
							return_code=unemap_load_current_stimulating(1,&channel_number,
								number_of_values,values_per_second,values,number_of_cycles,
								(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
							printf("return_code=%d\n",return_code);
						}
						else
						{
							printf("Not constant current\n");
						}
						free(values);
					}
					else
					{
						printf("Error reading %s\n",file_name);
					}
				} break;
				case 'o':
				{
					char file_name[121];
					float *values,values_per_second;
					int channel_number,constant_voltage,number_of_values;
					unsigned int number_of_cycles;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("Waveform file ? ");
					scanf(" %s",file_name);
					printf("number_of_cycles ? ");
					scanf(" %u",&number_of_cycles);
					if (unemap_read_waveform_file((FILE *)NULL,file_name,
						&number_of_values,&values_per_second,&values,&constant_voltage))
					{
						if (constant_voltage)
						{
							return_code=unemap_load_voltage_stimulating(1,&channel_number,
								number_of_values,values_per_second,values,number_of_cycles,
								(Unemap_stimulation_end_callback *)NULL,(void *)NULL);
							printf("return_code=%d\n",return_code);
						}
						else
						{
							printf("Not constant_voltage\n");
						}
						free(values);
					}
					else
					{
						printf("Error reading %s\n",file_name);
					}
				} break;
				case 'p':
				{
					float frequency;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("frequency ? ");
					scanf(" %f",&frequency);
					return_code=unemap_set_antialiasing_filter_frequency(channel_number,
						frequency);
					printf("return_code=%d\n",return_code);
				} break;
				case 'q':
				{
					int channel_number,stimulating;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("stimulating ? ");
					scanf(" %d",&stimulating);
					return_code=unemap_set_channel_stimulating(channel_number,
						stimulating);
					printf("return_code=%d\n",return_code);
				} break;
				case 'r':
				{
					float pre_filter_gain,post_filter_gain;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("pre_filter_gain ? ");
					scanf(" %f",&pre_filter_gain);
					printf("post_filter_gain ? ");
					scanf(" %f",&post_filter_gain);
					return_code=unemap_set_gain(channel_number,pre_filter_gain,
						post_filter_gain);
					printf("return_code=%d\n",return_code);
				} break;
				case 's':
				{
					int channel_number,isolate;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("isolate ? ");
					scanf(" %d",&isolate);
					return_code=unemap_set_isolate_record_mode(channel_number,isolate);
					printf("return_code=%d\n",return_code);
				} break;
				case 't':
				{
					int on;

					printf("on ? ");
					scanf(" %d",&on);
					return_code=unemap_set_power(on);
					printf("return_code=%d\n",return_code);
				} break;
				case 'u':
				{
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_set_powerup_antialiasing_filter_frequency(
						channel_number);
					printf("return_code=%d\n",return_code);
				} break;
				case 'v':
				{
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_set_scrolling_channel(channel_number);
					printf("return_code=%d\n",return_code);
				} break;
				case 'w':
				{
					return_code=unemap_shutdown();
					printf("return_code=%d\n",return_code);
				} break;
				case 'x':
				{
					char file_name[121];
					float *values,values_per_second;
					int channel_number,constant_voltage,number_of_values;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("Waveform file ? ");
					scanf(" %s",file_name);
					if (unemap_read_waveform_file((FILE *)NULL,file_name,
						&number_of_values,&values_per_second,&values,&constant_voltage))
					{
						if (constant_voltage)
						{
							return_code=unemap_start_calibrating(channel_number,
								number_of_values,values_per_second,values);
							printf("return_code=%d\n",return_code);
						}
						else
						{
							printf("Not constant_voltage\n");
						}
						free(values);
					}
					else
					{
						printf("Error reading %s\n",file_name);
					}
				} break;
#if defined (OLD_CODE)
				case 'u':
				{
					char file_name[121];
					float *values,values_per_second;
					int channel_number,constant_voltage,number_of_values;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("Waveform file ? ");
					scanf(" %s",file_name);
					if (unemap_read_waveform_file((FILE *)NULL,file_name,
						&number_of_values,&values_per_second,&values,&constant_voltage))
					{
						if (!constant_voltage)
						{
							return_code=unemap_start_current_stimulating(channel_number,
								number_of_values,values_per_second,values);
							printf("return_code=%d\n",return_code);
						}
						else
						{
							printf("Not constant current\n");
						}
						free(values);
					}
					else
					{
						printf("Error reading %s\n",file_name);
					}
				} break;
#endif /* defined (OLD_CODE) */
				case 'y':
				{
					return_code=unemap_start_sampling();
					printf("return_code=%d\n",return_code);
				} break;
				case 'z':
				{
					return_code=unemap_start_scrolling();
					printf("return_code=%d\n",return_code);
				} break;
				case 'A':
				{
					return_code=unemap_start_stimulating();
					printf("return_code=%d\n",return_code);
				} break;
#if defined (OLD_CODE)
				case 'x':
				{
					char file_name[121];
					float *values,values_per_second;
					int channel_number,constant_voltage,number_of_values;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("Waveform file ? ");
					scanf(" %s",file_name);
					if (unemap_read_waveform_file((FILE *)NULL,file_name,
						&number_of_values,&values_per_second,&values,&constant_voltage))
					{
						if (constant_voltage)
						{
							return_code=unemap_start_voltage_stimulating(channel_number,
								number_of_values,values_per_second,values);
							printf("return_code=%d\n",return_code);
						}
						else
						{
							printf("Not constant_voltage\n");
						}
						free(values);
					}
					else
					{
						printf("Error reading %s\n",file_name);
					}
				} break;
#endif /* defined (OLD_CODE) */
				case 'B':
				{
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_stop_calibrating(channel_number);
					printf("return_code=%d\n",return_code);
				} break;
				case 'C':
				{
					return_code=unemap_stop_sampling();
					printf("return_code=%d\n",return_code);
				} break;
				case 'D':
				{
					return_code=unemap_stop_scrolling();
					printf("return_code=%d\n",return_code);
					number_of_scrolling_callbacks=0;
				} break;
				case 'E':
				{
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					return_code=unemap_stop_stimulating(channel_number);
					printf("return_code=%d\n",return_code);
				} break;
				case 'F':
				{
					char file_name[81];
					FILE *out_file;
					int channel_number;

					printf("channel_number ? ");
					scanf(" %d",&channel_number);
					printf("file name ? ");
					scanf("%s",file_name);
					if (out_file=fopen(file_name,"wb"))
					{
						return_code=unemap_write_samples_acquired(channel_number,out_file);
						printf("return_code=%d\n",return_code);
						fclose(out_file);
					}
					else
					{
						printf("Could not open %s\n",file_name);
					}
				} break;
				default:
				{
					printf(">>>Unknown option\n");
				} break;
			}
			print_menu();
		}
		else
		{
			exit(0);
		}
	}
} /* process_keyboard */

/*
Global functions
----------------
*/
void main(void)
{
#if defined (MOTIF)
	if (application_context=XtCreateApplicationContext())
	{
		if (XtAppAddInput(application_context,fileno(stdin),
			(XtPointer)XtInputReadMask,process_keyboard,(XtPointer)NULL))
		{
			print_menu();
			XtAppMainLoop(application_context);
		}
	}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	print_menu();
	while (1)
	{
		process_keyboard();
	}
#endif /* defined (WINDOWS) */
} /* main */
