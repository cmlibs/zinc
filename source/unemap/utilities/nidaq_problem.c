/*******************************************************************************
FILE : nidaq_problem.c

LAST MODIFIED : 23 May 2000

DESCRIPTION :
Illustrates a problem where the NIDAQ buffer seems to get out of synch -
DAQ_Check returns the wrong position - when the total number of samples (for
each channel, not rolling) reachs 2**32.
==============================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include "nidaq.h"
/* for RTSI */
#include "nidaqcns.h"

/*
Module constants
----------------
*/
#define PXI6071E_DEVICE_CODE 258
#define NUMBER_OF_CHANNELS_ON_NI_CARD ((i16)64)
#define SCAN_COUNTER ND_COUNTER_0

/*
Module variables
----------------
*/
FILE *history_file=(FILE *)NULL;
int module_sampling_on=0,scrolling_callback_NI_count=0,
	scrolling_callback_NI_lock=0;
i16 module_scrolling_refresh_period=0;
unsigned long number_of_samples,sample_buffer_size,starting_sample_number;
u32 total_number_of_samples=0;

/*
Module functions
----------------
*/
static void scrolling_callback_NI(HWND handle,UINT message,WPARAM wParam,
	LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Always called so that <starting_sample_number> and <number_of_samples> can be
kept up to date.
==============================================================================*/
{
	unsigned long end,end2,end3,sample_number;

	/* stops other functions from changing number_of_samples and
		starting_sample_number while this function is being called (asynchronous) */
	scrolling_callback_NI_lock=1;
	scrolling_callback_NI_count++;
	/* callback may come after the sampling has stopped, in which case
		<starting_sample_number> and <number_of_samples> will have already been
		updated */
	sample_number=(unsigned long)lParam;
	end=(starting_sample_number+number_of_samples)%sample_buffer_size;
	end2=end+module_scrolling_refresh_period;
	end3=end2%sample_buffer_size;
	if (module_sampling_on&&(((end<=sample_number)&&(sample_number<=end2))||
		((sample_number<=end3)&&(end3<end))))
	{
		/* keep <starting_sample_number> and <number_of_samples> up to date */
#if defined (OLD_CODE)
		if (history_file)
		{
			fprintf(history_file,
				"scrolling_callback_NI %lu %lu %lu %d %lu\n",
				sample_number,starting_sample_number,number_of_samples,
				scrolling_callback_NI_count,total_number_of_samples);
		}
#endif /* defined (OLD_CODE) */
		if (number_of_samples<sample_buffer_size)
		{
			if (sample_number<starting_sample_number+number_of_samples)
			{
				total_number_of_samples += ((sample_number+sample_buffer_size)-
					(starting_sample_number+number_of_samples))*
					NUMBER_OF_CHANNELS_ON_NI_CARD;
				number_of_samples += (sample_number+sample_buffer_size)-
					(starting_sample_number+number_of_samples);
			}
			else
			{
				total_number_of_samples += (sample_number-
					(starting_sample_number+number_of_samples))*
					NUMBER_OF_CHANNELS_ON_NI_CARD;
				number_of_samples += sample_number-
					(starting_sample_number+number_of_samples);
			}
			if (number_of_samples>=sample_buffer_size)
			{
				number_of_samples=sample_buffer_size;
				starting_sample_number=sample_number%sample_buffer_size;
#if defined (OLD_CODE)
				if (history_file)
				{
					fprintf(history_file,
						"1.  change starting_sample_number %lu %lu   %lu %lu\n",
						starting_sample_number,number_of_samples,sample_number,
						sample_buffer_size);
				}
#endif /* defined (OLD_CODE) */
			}
		}
		else
		{
			starting_sample_number=sample_number%sample_buffer_size;
#if defined (OLD_CODE)
			if (history_file)
			{
				fprintf(history_file,"2.  change starting_sample_number %lu\n",
					starting_sample_number);
			}
#endif /* defined (OLD_CODE) */
		}
	}
#if defined (OLD_CODE)
	/*???debug */
	else
	{
		if (history_file)
		{
			fprintf(history_file,
				"scrolling_callback_NI.  Skip %lu %lu %lu %d %lu %d\n",
				sample_number,starting_sample_number,number_of_samples,
				scrolling_callback_NI_count,total_number_of_samples,
				module_sampling_on);
		}
	}
#endif /* defined (OLD_CODE) */
	/* stops other functions from changing number_of_samples and
		starting_sample_number while this function is being called (asynchronous) */
	scrolling_callback_NI_lock=0;
} /* scrolling_callback_NI */

void main(void)
{
	char option;
	float average,maximum_sampling_frequency,sampling_frequency;
	HGLOBAL memory_object;
	int count,count2,end,end2,i,j,k,start;
	i16 card_code,channel_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],device_number,
		gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],*hardware_buffer,status,stopped,
		time_base;
	short int *sample;
	u16 sampling_interval;
	u32 hardware_buffer_size,last_total_number_of_samples,
		module_sampling_high_count,module_sampling_low_count,retrieved,
		sample_number;

	/* configure hardware */
	device_number=1;
	sample_buffer_size=(unsigned long)5000;
	sampling_frequency=(float)5000;
	if ((0==Init_DA_Brds(device_number,&card_code))&&
		(PXI6071E_DEVICE_CODE==card_code))
	{
		status=AI_Clear(device_number);
		memory_object=(HGLOBAL)NULL;
		hardware_buffer=(i16 *)NULL;
		time_base=0;
		hardware_buffer_size=0;
		/* configure analog input */
		if (0==AI_Configure(device_number,/* all channels */(i16)(-1),
			/* referenced single ended */(i16)1,/* ignored (input range) */(i16)0,
			/* bipolar */(i16)0,/* do not drive AISENSE to ground */(i16)0))
		{
			/* drive the A/D conversions as fast as possible */
			/* use the 20 MHz (50 ns) clock */
			time_base= -3;
			/* sample at 0.5 MS/s */
			sampling_interval=40;
			maximum_sampling_frequency=(float)1;
			for (i=(int)ceil(log10((double)NUMBER_OF_CHANNELS_ON_NI_CARD));i>0;i--)
			{
				maximum_sampling_frequency *= (float)10;
			}
			maximum_sampling_frequency=(float)1e9/((float)sampling_interval*
				(float)50*maximum_sampling_frequency);
			if ((0<sampling_frequency)&&
				(sampling_frequency<maximum_sampling_frequency))
			{
				sampling_frequency=maximum_sampling_frequency;
			}
			module_sampling_high_count=2;
			module_sampling_low_count=
				(u32)((double)20000000/(double)sampling_frequency);
			sampling_frequency=
				(float)20000000./(float)module_sampling_low_count;
			module_sampling_low_count -= module_sampling_high_count;
			if ((module_sampling_low_count>0)&&(module_sampling_high_count>0))
			{
				module_scrolling_refresh_period=(i16)(sample_buffer_size/2);
				status=Config_DAQ_Event_Message(device_number,
					/* add message */(i16)1,/* channel string */"AI0",
					/* send message every N scans */(i16)1,
					(i32)module_scrolling_refresh_period,(i32)0,(u32)0,(u32)0,(u32)0,
					(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
				/* have sample_buffer_size divisible by
					module_scrolling_refresh_period */
				hardware_buffer_size=(sample_buffer_size-1)/
					module_scrolling_refresh_period;
				hardware_buffer_size=(hardware_buffer_size+1)*
					module_scrolling_refresh_period;
				hardware_buffer_size *= (u32)NUMBER_OF_CHANNELS_ON_NI_CARD;
				for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
				{
					channel_vector[i]=i;
				}
				status=0;
				/* allocate buffer */
				if (memory_object=GlobalAlloc(GMEM_MOVEABLE,
					(DWORD)((hardware_buffer_size)*sizeof(i16))))
				{
					if (hardware_buffer=(i16 *)GlobalLock(memory_object))
					{
						status=DAQ_DB_Config(device_number,/* enable */(i16)1);
						if (0==status)
						{
							/* set the scan sequence and gain */
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								gain_vector[j]=1;
							}
							status=SCAN_Setup(device_number,NUMBER_OF_CHANNELS_ON_NI_CARD,
								channel_vector,gain_vector);
							if (0==status)
							{
								/* set the clock source on the RTSI bus */
								status=Select_Signal(device_number,ND_RTSI_CLOCK,
									ND_BOARD_CLOCK,ND_DONT_CARE);
								if (0!=status)
								{
									printf(
					"Select_Signal(0,ND_RTSI_CLOCK,ND_BOARD_CLOCK,ND_DONT_CARE) failed");
								}
								if (0==status)
								{
									status=Select_Signal(device_number,ND_IN_SCAN_START,
										ND_RTSI_0,ND_LOW_TO_HIGH);
									if (0==status)
									{
										/* acquisition won't actually start (controlled by
											conversion signal) */
										status=SCAN_Start(device_number,hardware_buffer,
											hardware_buffer_size,time_base,sampling_interval,(i16)0,
											(u16)0);
										if (0!=status)
										{
											printf("SCAN_Start failed.  %d",status);
										}
									}
									else
									{
										printf(
	"Select_Signal(*,ND_IN_SCAN_START,ND_PFI_7/ND_RTSI_0,ND_LOW_TO_HIGH) failed");
									}
								}
								if (0!=status)
								{
									GlobalUnlock(memory_object);
									GlobalFree(memory_object);
									DAQ_DB_Config(device_number,/* disable */(i16)0);
								}
							}
							else
							{
								printf("SCAN_Setup failed.  %d.  gain=%d",status,
									gain_vector[0]);
								GlobalUnlock(memory_object);
								GlobalFree(memory_object);
								DAQ_DB_Config(device_number,/* disable */(i16)0);
							}
						}
						else
						{
							printf("DAQ_DB_Config failed");
							GlobalUnlock(memory_object);
							GlobalFree(memory_object);
						}
					}
					else
					{
						printf("GlobalLock failed");
						GlobalFree(memory_object);
					}
				}
				else
				{
					printf("GlobalAlloc failed");
				}
				number_of_samples=0;
				starting_sample_number=0;
				if (history_file)
				{
					fprintf(history_file,"4.  change starting_sample_number %lu %lu\n",
						starting_sample_number,number_of_samples);
				}
#if defined (OLD_CODE)
				if (0==status)
				{
					/*???DB.  Needed, otherwise WFM_Op (waveform generation) fails */
					/*???DB.  Don't understand why.  See set_NI_gain */
					status=Config_DAQ_Event_Message(device_number,
						/* clear all messages */(i16)0,/* channel string */(char *)NULL,
						/* send message every N scans */(i16)0,
						(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,
						(HWND)NULL,(i16)0,(u32)0);
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				printf("Invalid sampling frequency.  %d %d",module_sampling_low_count,
					module_sampling_high_count);
			}
			sample_buffer_size=hardware_buffer_size/NUMBER_OF_CHANNELS_ON_NI_CARD;
			if ((0==status)&&(0<sample_buffer_size)&&(0<hardware_buffer_size))
			{
				/* set D/A low */
				AO_Write(device_number,0,(i16)0);
				/* test until error */
				/*???DB.  The history file id to try and determine why the saved file
					is getting out of synch with the on/off sequence */
				history_file=fopen("nidaq_history.txt","w");
				count=0;
				count2=0;
				do
				{
					count++;
					last_total_number_of_samples=total_number_of_samples;
					/* start sampling */
					/* set up the conversion signal */
					status=GPCTR_Set_Application(device_number,SCAN_COUNTER,
						ND_PULSE_TRAIN_GNR);
					if (0==status)
					{
						status=GPCTR_Change_Parameter(device_number,SCAN_COUNTER,
							ND_COUNT_1,module_sampling_low_count);
						if (0==status)
						{
							status=GPCTR_Change_Parameter(device_number,SCAN_COUNTER,
								ND_COUNT_2,module_sampling_high_count);
							if (0==status)
							{
								status=Select_Signal(device_number,ND_RTSI_0,ND_GPCTR0_OUTPUT,
									ND_DONT_CARE);
								if (0==status)
								{
									status=DAQ_Check(device_number,&stopped,&retrieved);
									if (0==status)
									{
										scrolling_callback_NI_count=0;
										starting_sample_number=((unsigned long)retrieved/
											(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD)%
											(hardware_buffer_size/
											(unsigned long)NUMBER_OF_CHANNELS_ON_NI_CARD);
										number_of_samples=0;
										module_sampling_on=1;
										if (history_file)
										{
											fprintf(history_file,
												"5.  change starting_sample_number %lu %lu %lu %lu\n",
												starting_sample_number,number_of_samples,retrieved,
												hardware_buffer_size);
										}
										/* start the data acquisition */
										status=GPCTR_Control(device_number,SCAN_COUNTER,
											ND_PROGRAM);
										if (0!=status)
										{
											module_sampling_on=0;
											printf("GPCTR_Control (ND_PROGRAM) failed");
										}
									}
									else
									{
										printf("DAQ_Check failed");
									}
								}
								else
								{
									printf("Select_Signal (ND_GPCTR0_SOURCE) failed");
								}
							}
							else
							{
								printf("GPCTR_Change_Parameter (high length) failed");
							}
						}
						else
						{
							printf("GPCTR_Change_Parameter (low length) failed");
						}
					}
					else
					{
						printf("GPCTR_Set_Application failed");
					}
					/* in milliseconds */
					Sleep((DWORD)100);
					/* set D/A high */
					AO_Write(device_number,0,(i16)256);
					/* in milliseconds */
					Sleep((DWORD)450);
					/* set D/A low */
					AO_Write(device_number,0,(i16)0);
					/* in milliseconds */
					Sleep((DWORD)100);
					/* stop sampling */
					/* stop continuous sampling */
					status=GPCTR_Control(device_number,SCAN_COUNTER,ND_RESET);
					if (0==status)
					{
						module_sampling_on=0;
						/* wait until scrolling_callback_NI is finished */
						/*???debug */
						if (scrolling_callback_NI_lock)
						{
							if (history_file)
							{
								fprintf(history_file,"Waiting for scrolling_callback_NI\n");
							}
							while (scrolling_callback_NI_lock)
							{
								printf("Waiting for scrolling_callback_NI\n");
							}
							/*???debug */
							printf("scrolling_callback_NI finished\n");
							if (history_file)
							{
								fprintf(history_file,"scrolling_callback_NI finished\n");
							}
						}
						status=DAQ_Check(device_number,&stopped,&sample_number);
						if (0==status)
						{
							sample_number /= NUMBER_OF_CHANNELS_ON_NI_CARD;
							if (history_file)
							{
								fprintf(history_file,
									"stop sampling.  %lu %lu %lu %lu %d %lu\n",
									starting_sample_number,number_of_samples,
									sample_buffer_size,sample_number,stopped,
									total_number_of_samples);
							}
							if (number_of_samples<sample_buffer_size)
							{
								if (sample_number<
									starting_sample_number+number_of_samples)
								{
									total_number_of_samples +=
										((sample_number+sample_buffer_size)-
										(starting_sample_number+
										number_of_samples))*
										NUMBER_OF_CHANNELS_ON_NI_CARD;
									number_of_samples +=
										(sample_number+sample_buffer_size)-
										(starting_sample_number+number_of_samples);
								}
								else
								{
									total_number_of_samples += (sample_number-
										(starting_sample_number+
										number_of_samples))*
										NUMBER_OF_CHANNELS_ON_NI_CARD;
									number_of_samples += sample_number-
										(starting_sample_number+number_of_samples);
								}
								if (number_of_samples>sample_buffer_size)
								{
									number_of_samples=sample_buffer_size;
									starting_sample_number=
										sample_number%sample_buffer_size;
									if (history_file)
									{
										fprintf(history_file,
											"7.  change starting_sample_number %lu %lu\n",
											starting_sample_number,number_of_samples);
									}
								}
							}
							else
							{
								starting_sample_number=sample_number%sample_buffer_size;
								if (history_file)
								{
									fprintf(history_file,
										"8.  change starting_sample_number %lu\n",
										starting_sample_number);
								}
							}
						}
						else
						{
							printf("DAQ_Check failed");
						}
					}
					else
					{
						printf("GPCTR_Control failed");
					}
					/* calculate average */
					average=0;
					start=starting_sample_number;
					if (starting_sample_number+number_of_samples>sample_buffer_size)
					{
						end=sample_buffer_size;
						end2=starting_sample_number+number_of_samples-
							sample_buffer_size;
					}
					else
					{
						end=starting_sample_number+number_of_samples;
						end2=0;
					}
					sample=hardware_buffer+(start*NUMBER_OF_CHANNELS_ON_NI_CARD);
					for (k=start;k<end;k++)
					{
						average += (float)(*sample);
						sample += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
					if (end2>0)
					{
						sample=hardware_buffer;
						for (k=0;k<end2;k++)
						{
							average += (float)(*sample);
							sample += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
					average /= number_of_samples;
					printf("%d.  %lu %lu %lu.  %f\n",count,starting_sample_number,
						number_of_samples,total_number_of_samples,average);
					if (history_file)
					{
						fprintf(history_file,"%d.  %lu %lu %lu.  %f\n",count,
							starting_sample_number,number_of_samples,total_number_of_samples,
							average);
					}
					if (0<count2)
					{
						count2++;
					}
					if (last_total_number_of_samples>total_number_of_samples)
					{
						count2++;
					}
				} while ((0==status)&&(number_of_samples<3300)&&(count2<=10));
/*				} while ((0==status)&&(number_of_samples<3300)&&
					(last_total_number_of_samples<total_number_of_samples));*/
				printf("number of power on/off cycles = %d.  %lu %lu\n",count,
					number_of_samples,total_number_of_samples);
				if (history_file)
				{
					fprintf(history_file,"number of power on/off cycles = %d.  %lu %lu\n",
						count,number_of_samples,total_number_of_samples);
					/* write the acquired samples */
					/* get the samples acquired */
					start=starting_sample_number;
					if (starting_sample_number+number_of_samples>sample_buffer_size)
					{
						end=sample_buffer_size;
						end2=starting_sample_number+number_of_samples-
							sample_buffer_size;
					}
					else
					{
						end=starting_sample_number+number_of_samples;
						end2=0;
					}
					sample=hardware_buffer+(start*NUMBER_OF_CHANNELS_ON_NI_CARD);
					count=1;
					for (k=start;k<end;k++)
					{
						fprintf(history_file,"%d  %d\n",count,*sample);
						sample += NUMBER_OF_CHANNELS_ON_NI_CARD;
						count++;
					}
					if (end2>0)
					{
						sample=hardware_buffer;
						for (k=0;k<end2;k++)
						{
							fprintf(history_file,"%d  %d\n",count,*sample);
							sample += NUMBER_OF_CHANNELS_ON_NI_CARD;
							count++;
						}
					}
					fclose(history_file);
					history_file=(FILE *)NULL;
				}
			}
			else
			{
				printf("No NI cards");
			}
			/* stop continuous sampling */
			status=GPCTR_Control(device_number,SCAN_COUNTER,ND_RESET);
			status=DAQ_Clear(device_number);
			GlobalUnlock(memory_object);
			GlobalFree(memory_object);
			/* turn off double buffering */
			status=DAQ_DB_Config(device_number,/* disable */(i16)0);
			number_of_samples=0;
			starting_sample_number=0;
		}
		else
		{
			printf("AI_Configure failed");
		}
	}
	else
	{
		printf("Init_DA_Brds failed");
	}
	scanf("%c",&option);
} /* main */
