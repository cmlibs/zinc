/*******************************************************************************
FILE : acquisition.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
???Move to acquisition_work_area.c ?
==============================================================================*/
#include <math.h>
#include <stddef.h>
#include <string.h>

#if !defined(VMS)
/*???DB.  For chdir and mkdir.  Not ansi */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined (POLLING)
#include <ulimit.h>
/* contains struct rlimit definition */
#include <sys/resource.h>
/* header file for plock syscall.  Used for locking the program in memory */
#include <sys/lock.h>
#endif /* defined (POLLING) */

#if defined (INTERRUPT)
#include <ulimit.h>
#include "myio.h"
/*??? temp */
#include <errno.h>
/* contains struct rlimit definition */
#include <sys/resource.h>
/* header file for shared memory functions */
/*#include <sys/shm.h>*/
/* header file for plock syscall.  Used for locking the program in memory */
#include <sys/lock.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/timers.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "general/debug.h"
#include "unemap/acquisition.h"
#include "unemap/acquisition_window.h"
#include "unemap/rig.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"

#if defined (UNIMA)
#if defined (INTERRUPT)
#include "UNIMA/uni.h"
#include "UNIMA/un.h"
#endif
#include "UNIMA/uim.h"
#include "UNIMA/unimax.h"
#include "UNIMA/uadapterboard.h"
/*???UNIMA*/
/*#include "UNIMA/unima_syscall.c"*/
#include "UNIMA/un05.h"

/* Number of samples to collect */
/* Number of IO cards in UNIMA crate */
	/*???DB.  Now set in Makefile */
/*#define NUM_IO_CARDS 16*/
/* Number of channels/UNIMA IO card */
#define MUX_CHANNELS 32
/* Mask to lower 16 bits of data    */
#define Mask 0x0000ffff
/* Time for UNIMA to initialize */
#define UNIMA_INITIALIZE_SEC 2
#define UNIMA_INITIALIZE_NSEC 0
/* Time for a conversion to complete 10usec+32bits@10MHz */
#define UNIMA_CONVERSION_SEC 0
#define UNIMA_CONVERSION_NSEC 14000
/* Unima pointers */
/* base address allocated by IBM System */
extern volatile unsigned int *UnimaBusPort;
/* addresses for the interface modules */
volatile unsigned int *calibrate_module,*interface_module_1,*interface_module_2,
	*interface_module_3,*interface_module_4,*interface_module_5,
	*interface_module_6,*interface_module_7,*interface_module_8,
	*interface_module_9,*interface_module_10,*interface_module_11,
	*interface_module_12,*interface_module_13,*interface_module_14,
	*interface_module_15,*interface_module_16,*interface_base;
/*???DB.  Serge was having some problems with some of the UNIMA interface
	modules at CSMC (due to problems with the back plane).  So he experimented
	with which EMAP card (PANEL) was connected to which interface module (CARD) */
/* PANEL                 01 02 03 04 05 06 07 08 09 10                       */
/* CARD:                 03 04 05 06 07 08 10 11 13 14 15 16 01 02 09 12 00  */
/*int module_order[18]={ 0, 7, 8, 9,10,11,12,14,15, 1, 3, 2, 4, 5, 6,13,16, 0};*/
int module_order[18]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16, 0};
#define Swap swapb
#define GETSWAP16(target,value) {\
	unsigned int z;   		 \
	z = (volatile unsigned int) value;	 \
	target = (short int)( ((z>>8)&0x0000ff00) | (z>>24) ); \
	}
int calibrate_module_number,channel_number;
float channel_offsets[MUX_CHANNELS*NUM_IO_CARDS],
	channel_gains[MUX_CHANNELS*NUM_IO_CARDS];
unsigned long real_time_clock_sec();
unsigned long real_time_clock_nsec();

/* So that the signal buffer is only created once no matter how many times the
	experiment is started or stop.  Because it is such a large block of memory,
	allocating and deallocating it can cause problems with plock */
struct Signal_buffer *acquisition_buffer=(struct Signal_buffer *)NULL;

#if defined (INTERRUPT)
#include "unemap/acquisition_interrupt.h"
#endif
#endif
#include "user_interface/user_interface.h"

/*???remove when move into acquisition_work_area.c ? */
#include "unemap/acquisition_work_area.h"

/*
Module variables
----------------
*/
#if defined (INTERRUPT)
Display *interrupt_display=NULL;
struct Acquisition_interrupt_data acquisition_interrupt_data;
/*???temporary variables for testing interrupting */
Widget acquisition_drawing_window;
int acquisition_drawing_height,acquisition_drawing_marker,
	acquisition_drawing_width;
/*???debug */
struct Rig *acquisition_rig;
#endif

/*
Module functions
----------------
*/
#if defined (INTERRUPT)
static void refresh_signal(/*int signal,int code,
	struct sigcontext *signal_context_pointer*/)
/*******************************************************************************
LAST MODIFIED : 23 July 1992

DESCRIPTION :
Called every 10th interrupt to refresh the display.
==============================================================================*/
{
	XDrawLine(interrupt_display,acquisition_drawing_window,
		graphics_contexts.accepted_colour,acquisition_drawing_marker,0,
		acquisition_drawing_marker,acquisition_drawing_height);
	XFlush(interrupt_display);
	if (++acquisition_drawing_marker>acquisition_drawing_width)
	{
		acquisition_drawing_marker=0;
	}
} /* refresh_signal */

static void tidyup_signal(/*int signal,int code,
	struct sigcontext *signal_context_pointer*/)
/*******************************************************************************
LAST MODIFIED : 13 May 1996

DESCRIPTION :
Called by the last interrupt to tidy up the process memory.
==============================================================================*/
{
/*???debug */
int i,index;
struct Device **device;
struct File_open_data *file_open_data;
struct Signal_buffer *buffer;

	/* unlock the process code, data and stack */
	plock(UNLOCK);
trcoff(0);
trcstop();
printf("experiment time = %lu\n",acquisition_interrupt_data.experiment_time);
/* set the indices for the channels */
device=acquisition_rig->devices;
buffer=(*device)->signal->buffer;
for (i=acquisition_rig->number_of_devices;i>0;i--)
{
	index=((*device)->channel->number-1)%MUX_CHANNELS-channel_number;
	if (index<0)
	{
		index += MUX_CHANNELS;
	}
	(*device)->signal->index=index*NUM_IO_CARDS+
	((*device)->channel->number-1)/MUX_CHANNELS;
	device++;
}
/* set the buffer start and end */
buffer->start=0;
buffer->end=acquisition_interrupt_data.experiment_time-1;
/* write to disk */
if (file_open_data=create_File_open_data(user_settings.signal_file_extension,
	REGULAR,acquisition_write_signal_file,(XtPointer)(&acquisition_rig),0))
{
	open_file_and_write((Widget)NULL,(XtPointer)file_open_data,(XtPointer)NULL);
}
else
{
	display_message(ERROR_MESSAGE,
		"tidyup_signal.  Could not create file open data");
}
/*???end debug */
} /* tidyup_signal */
#endif

#if defined (UNIMA)
static void cancel_calibrate(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
{
	ENTER(cancel_calibrate);
	USE_PARAMETER(widget);
	USE_PARAMETER(client_data);
	USE_PARAMETER(call_data);
	/* reset the Unima system */
	U00SysReset();
	/* reset the Unima adapter */
	UAReset();
	UASetAdapter(DISABLE);
	LEAVE;
} /* cancel_calibrate */
#endif /* defined (UNIMA) */

#if defined (UNIMA)
static void calculate_gains(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
???Written for UNIMA auto start mode.
==============================================================================*/
{
	struct Acquisition_window *acquisition;
#if defined (POLLING)
	int number_of_samples,sampling_period_nsec;
	char negative_gain;
	float *gain,*offset;
	FILE *output_file;
	struct Rig **rig_address;
	struct rlimit resource_limits;
	int channel,old_priority,sample;
	unsigned long convert_nsec,convert_sec,next_nsec,next_sec,temp_sec;
	short int prevent_removal_by_optimizer,temp_short;
	int board_sums[MUX_CHANNELS],*sum;
#endif /* defined (POLLING) */

	ENTER(calculate_gains);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((acquisition=(struct Acquisition_window *)acquisition_window)&&((
#if defined (POLLING)
		number_of_samples=
#endif /* defined (POLLING) */
		(acquisition->end_time)-(acquisition->start_time))>0))
	{
#if defined (POLLING)
		sampling_period_nsec=1000000000/(acquisition->sampling_frequency_hz);
		/* shrink the stack and lock the process code, data and stack */
		resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
		if ((0==setrlimit(RLIMIT_STACK,&resource_limits))&&(0==plock(PROCLOCK)))
		{
			/* turn the interrupts off */
			old_priority=disable_interrupts(1);
			/* zero the sums */
			sum=board_sums;
			for (channel=MUX_CHANNELS;channel>0;channel--)
			{
				*sum=0;
				sum++;
			}
			/* get the start time */
			next_sec=real_time_clock_sec();
			next_nsec=real_time_clock_nsec();
			/* throw away the first reading (auto start) */
			/* the value returned by UNIMA is not important, but the conversion
				must be carried out.  To prevent the optimizer from removing the
				conversion statement the value returned must be used */
			prevent_removal_by_optimizer= *calibrate_module>>16;
			convert_sec=next_sec;
			convert_nsec=next_nsec;
			if (channel_number>=MUX_CHANNELS)
			{
				channel_number=0;
			}
			/* acquire the data */
			for (sample=0;sample<number_of_samples;sample++)
			{
				/* for each channel */
				for (channel=MUX_CHANNELS;channel>0;channel--)
				{
					/* wait for the conversion to complete */
					convert_sec += UNIMA_CONVERSION_SEC;
					convert_nsec += UNIMA_CONVERSION_NSEC;
					if (convert_nsec>=1000000000)
					{
						convert_sec++;
						convert_nsec -= 1000000000;
					}
					while (((temp_sec=real_time_clock_sec())<convert_sec)||
						((temp_sec==convert_sec)&&(real_time_clock_nsec()<convert_nsec)));
					/* find the conversion time */
					convert_sec=real_time_clock_sec();
					convert_nsec=real_time_clock_nsec();
					/* Read the last conversion back and trigger the next conversion */
					GETSWAP16(temp_short,*calibrate_module);
					board_sums[channel_number] += temp_short;
					channel_number++;
					if (channel_number>=MUX_CHANNELS)
					{
						channel_number=0;
					}
				}
				/* wait for next sampling time */
				next_nsec += sampling_period_nsec;
				if (next_nsec>=1000000000)
				{
					next_sec++;
					next_nsec -= 1000000000;
				}
				while (((temp_sec=real_time_clock_sec())<next_sec)||
					((temp_sec==next_sec)&&(real_time_clock_nsec()<next_nsec)));
			}
			enable_interrupts(old_priority);
			/* unlock the process code, data and stack */
			plock(UNLOCK);
			/* calculate the gains */
			sum=board_sums;
			gain=channel_gains+(calibrate_module_number-1)*MUX_CHANNELS;
			offset=channel_offsets+(calibrate_module_number-1)*MUX_CHANNELS;
			negative_gain=0;
			channel=MUX_CHANNELS;
			while ((channel>0)&&(!negative_gain))
			{
				*gain=(float)(*sum)/(float)number_of_samples;
				if ((*gain)==(*offset))
				{
					*gain=1;
				}
				else
				{
					/*???enter the reference voltage ? */
					*gain=300./((*gain)-(*offset));
				}
				if (*gain<0)
				{
					negative_gain=1;
				}
				gain++;
				offset++;
				sum++;
				channel--;
			}
			if (!negative_gain)
			{
				/* write the calibration file */
				if (output_file=fopen("calibrate.dat","w"))
				{
					fprintf(output_file,"channel  offset  gain\n");
					gain=channel_gains;
					offset=channel_offsets;
					for (channel=1;channel<=MUX_CHANNELS*NUM_IO_CARDS;channel++)
					{
						fprintf(output_file,"%d %g %g\n",channel,*offset,*gain);
						gain++;
						offset++;
					}
					fclose(output_file);
					/* update the acquisition rig */
					if (rig_address=acquisition->rig_address)
					{
						read_calibration_file("calibrate.dat",*rig_address);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_gains.  Could not write calibration file");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
"Negative gain.  Repeat calibration with calibration voltage polarity reversed"
					);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_gains.  Could not lock the process in memory");
		}
#endif /* defined (POLLING) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_gains.  Invalid acquisition_window");
	}
#if defined (POLLING)
	/* reset the Unima system */
	U00SysReset();
	/* reset the Unima adapter */
	UAReset();
	UASetAdapter(DISABLE);
#endif /* defined (POLLING) */
	LEAVE;
} /* calculate_gains */
#endif /* defined (UNIMA) */

#if defined (UNIMA)
static void calculate_offsets(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
???Written for UNIMA auto start mode.
==============================================================================*/
{
	struct Acquisition_window *acquisition;
#if defined (POLLING)
	int number_of_samples,sampling_period_nsec;
	float *offset;
	struct rlimit resource_limits;
	int channel,old_priority,sample;
	unsigned long convert_nsec,convert_sec,next_nsec,next_sec,temp_sec;
	short int temp_short;
	short int prevent_removal_by_optimizer;
	int board_sums[MUX_CHANNELS],*sum;
#endif /* defined (POLLING) */

	ENTER(calculate_offsets);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((acquisition=(struct Acquisition_window *)acquisition_window)&&((
#if defined (POLLING)
		number_of_samples=
#endif /* defined (POLLING) */
		(acquisition->end_time)-(acquisition->start_time))>0))
	{
#if defined (POLLING)
		sampling_period_nsec=1000000000/(acquisition->sampling_frequency_hz);
		/* shrink the stack and lock the process code, data and stack */
		resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
		if ((0==setrlimit(RLIMIT_STACK,&resource_limits))&&(0==plock(PROCLOCK)))
		{
			/* turn the interrupts off */
			old_priority=disable_interrupts(1);
			/* zero the sums */
			sum=board_sums;
			for (channel=MUX_CHANNELS;channel>0;channel--)
			{
				*sum=0;
				sum++;
			}
			sum=board_sums;
			/* get the start time */
			next_sec=real_time_clock_sec();
			next_nsec=real_time_clock_nsec();
			/* throw away the first reading (auto start) */
			prevent_removal_by_optimizer= *calibrate_module>>16;
			channel_number++;
			if (channel_number>=MUX_CHANNELS)
			{
				channel_number=0;
			}
			convert_sec=next_sec;
			convert_nsec=next_nsec;
			/* acquire the data */
			for (sample=0;sample<number_of_samples;sample++)
			{
				/* for each channel */
				for (channel=MUX_CHANNELS;channel>0;channel--)
				{
					/* wait for the conversion to complete */
					convert_sec += UNIMA_CONVERSION_SEC;
					convert_nsec += UNIMA_CONVERSION_NSEC;
					if (convert_nsec>=1000000000)
					{
						convert_sec++;
						convert_nsec -= 1000000000;
					}
					while (((temp_sec=real_time_clock_sec())<convert_sec)||
						((temp_sec==convert_sec)&&(real_time_clock_nsec()<convert_nsec)));
					/* find the conversion time */
					convert_sec=real_time_clock_sec();
					convert_nsec=real_time_clock_nsec();
					/* Read the last conversion back and trigger the next conversion */
					GETSWAP16(temp_short,*calibrate_module);
					board_sums[channel_number] += temp_short;
					channel_number++;
					if (channel_number>=MUX_CHANNELS)
					{
						channel_number=0;
					}
				}
				/* wait for next sampling time */
				next_nsec += sampling_period_nsec;
				if (next_nsec>=1000000000)
				{
					next_sec++;
					next_nsec -= 1000000000;
				}
				while (((temp_sec=real_time_clock_sec())<next_sec)||
					((temp_sec==next_sec)&&(real_time_clock_nsec()<next_nsec)));
			}
			enable_interrupts(old_priority);
			/* unlock the process code, data and stack */
			plock(UNLOCK);
			/* calculate the offsets */
			sum=board_sums;
			offset=channel_offsets+(calibrate_module_number-1)*MUX_CHANNELS;
			for (channel=MUX_CHANNELS;channel>0;channel--)
			{
				*offset=(float)(*sum)/(float)number_of_samples;
				sum++;
				offset++;
			}
			/*??? input the calibration voltage */
			print_question((XtPointer)calculate_gains,acquisition_window,
				(XtPointer)cancel_calibrate,(XtPointer)NULL,
				1,"Are you ready to calibrate gains ?");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_offsets.  Could not lock the process in memory");
		}
#endif /* defined (POLLING) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_offsets.  Invalid acquisition_window");
	}
	LEAVE;
} /* calculate_offsets */
#endif /* defined (UNIMA) */

static int start_experiment(char *directory_name,void *acquisition_window)
/******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
***Serge***
Allocate storage for the signal_buffer.
Initialize the UNIMA system.
Start the UNIMA system updating the time since the start of the experiment.
=============================================================================*/
{
	struct Acquisition_window *acquisition;
	int return_code;
	struct Rig *rig;
#if defined (INTERRUPT)
	struct sigaction acquisition_signal;
/*	int acquisition_interrupt_data_key;*/
	struct rlimit resource_limits;
	struct mapr_area unima_map_area;
/*??? temp */
XWindowAttributes attributes;
#endif
#if defined (UNIMA)
	float frequency;
	int index,number_of_devices,sampling_period_nsec;
	unsigned long convert_nsec,convert_sec;
	unsigned long start_nsec,start_sec,temp_sec;
	short int prevent_removal_by_optimizer;
	struct Signal_buffer *buffer;
	struct Device **device;
#endif

	ENTER(start_experiment);
	if ((acquisition=(struct Acquisition_window *)acquisition_window)&&
		(acquisition->rig_address)&&(rig= *(acquisition->rig_address)))
	{
#if defined (VMS)
		return_code=1;
#else
		if (directory_name)
		{
			mkdir(directory_name,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			if (chdir(directory_name))
			{
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			return_code=0;
		}
#endif
		if (return_code)
		{
#if defined (INTERRUPT)
/*???debug */
acquisition_rig=rig;
#endif
			if (EXPERIMENT_OFF==rig->experiment)
			{
#if defined (UNIMA)
				sampling_period_nsec=1000000000/(acquisition->sampling_frequency_hz);
				frequency=1000000000./(float)sampling_period_nsec;
				/*???from IOSample.c (received 23/7/92).  To start with I'll hard code
					the IOBuffer into the kernel (see acquisition_interrupt.c) */
				Unima(1,0);
				start_sec=real_time_clock_sec();
				start_nsec=real_time_clock_nsec();
				/* wait for the UNIMA initialization to complete */
				start_sec += UNIMA_INITIALIZE_SEC;
				start_nsec += UNIMA_INITIALIZE_NSEC;
				if (start_nsec>=1000000000)
				{
					start_sec++;
					start_nsec -= 1000000000;
				}
				while (((temp_sec=real_time_clock_sec())<start_sec)||
					((temp_sec==start_sec)&&(real_time_clock_nsec()<start_nsec)));
#if !defined (AUTO_START_AD)
				UUserConfigRegister[0]=UUserConfigRegister[1];
#endif
#if defined (INTERRUPT)
				if (UnimaFile&&(0==ioctl(UnimaFile,UNI_IOC_QMAPR,&unima_map_area)))
				{
#endif
					/* the number of the pair of pins on each EMAP card where the next
						conversion will be */
					channel_number=STARTING_CHANNEL_NUMBER;
					/* create the buffer */
					if ((device=rig->devices)&&
						((number_of_devices=rig->number_of_devices)>0))
					{
						if (!acquisition_buffer)
						{
							acquisition_buffer=create_Signal_buffer(SHORT_INT_VALUE,
								NUM_IO_CARDS*MUX_CHANNELS,MAXIMUM_NUMBER_OF_SAMPLES,frequency);
						}
						if (buffer=acquisition_buffer)
						{
							index=0;
							/*???channel acquisition order for card ? */
							while ((index<number_of_devices)&&
								((*device)->signal=create_Signal(
								(((*device)->channel->number-1)%MUX_CHANNELS)*NUM_IO_CARDS+
								((*device)->channel->number-1)/MUX_CHANNELS,buffer,UNDECIDED,
								0)))
							{
								index++;
								device++;
							}
							if (index==number_of_devices)
							{
								/* initialize the pointers to the interface modules */
								interface_base=
									(unsigned int *)(UnimaBusPort+UN05_CHANNEL0);
								interface_module_1=interface_base+(module_order[1]*8);
#if (NUM_IO_CARDS>1)
								interface_module_2=interface_base+(module_order[2]*8);
#if (NUM_IO_CARDS>2)
								interface_module_3=interface_base+(module_order[3]*8);
#if (NUM_IO_CARDS>3)
								interface_module_4=interface_base+(module_order[4]*8);
#if (NUM_IO_CARDS>4)
								interface_module_5=interface_base+(module_order[5]*8);
#if (NUM_IO_CARDS>5)
								interface_module_6=interface_base+(module_order[6]*8);
#if (NUM_IO_CARDS>6)
								interface_module_7=interface_base+(module_order[7]*8);
#if (NUM_IO_CARDS>7)
								interface_module_8=interface_base+(module_order[8]*8);
#if (NUM_IO_CARDS>8)
								interface_module_9=interface_base+(module_order[9]*8);
#if (NUM_IO_CARDS>9)
								interface_module_10=interface_base+(module_order[10]*8);
#if (NUM_IO_CARDS>10)
								interface_module_11=interface_base+(module_order[11]*8);
#if (NUM_IO_CARDS>11)
								interface_module_12=interface_base+(module_order[12]*8);
#if (NUM_IO_CARDS>12)
								interface_module_13=interface_base+(module_order[13]*8);
#if (NUM_IO_CARDS>13)
								interface_module_14=interface_base+(module_order[14]*8);
#if (NUM_IO_CARDS>14)
								interface_module_15=interface_base+(module_order[15]*8);
#if (NUM_IO_CARDS>15)
								interface_module_16=interface_base+(module_order[16]*8);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
								/* get rid of garbage */
								convert_sec=real_time_clock_sec();
								convert_nsec=real_time_clock_nsec();
#if defined (AUTO_START_AD)
								/* the values returned by UNIMA are not important, but the
									conversions must be carried out.  To prevent the optimizer
									from removing the conversion statements the values returned
									must be used */
								prevent_removal_by_optimizer=0;
								prevent_removal_by_optimizer += *interface_module_1>>16;
#if (NUM_IO_CARDS>1)
								prevent_removal_by_optimizer +=	*interface_module_2>>16;
#if (NUM_IO_CARDS>2)
								prevent_removal_by_optimizer +=	*interface_module_3>>16;
#if (NUM_IO_CARDS>3)
								prevent_removal_by_optimizer +=	*interface_module_4>>16;
#if (NUM_IO_CARDS>4)
								prevent_removal_by_optimizer +=	*interface_module_5>>16;
#if (NUM_IO_CARDS>5)
								prevent_removal_by_optimizer +=	*interface_module_6>>16;
#if (NUM_IO_CARDS>6)
								prevent_removal_by_optimizer +=	*interface_module_7>>16;
#if (NUM_IO_CARDS>7)
								prevent_removal_by_optimizer +=	*interface_module_8>>16;
#if (NUM_IO_CARDS>8)
								prevent_removal_by_optimizer +=	*interface_module_9>>16;
#if (NUM_IO_CARDS>9)
								prevent_removal_by_optimizer +=	*interface_module_10>>16;
#if (NUM_IO_CARDS>10)
								prevent_removal_by_optimizer +=	*interface_module_11>>16;
#if (NUM_IO_CARDS>11)
								prevent_removal_by_optimizer +=	*interface_module_12>>16;
#if (NUM_IO_CARDS>12)
								prevent_removal_by_optimizer +=	*interface_module_13>>16;
#if (NUM_IO_CARDS>13)
								prevent_removal_by_optimizer +=	*interface_module_14>>16;
#if (NUM_IO_CARDS>14)
								prevent_removal_by_optimizer +=	*interface_module_15>>16;
#if (NUM_IO_CARDS>15)
								prevent_removal_by_optimizer +=	*interface_module_16>>16;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#else
								/* send a multicast conversion command */
								USetControl(0,A,1);
								USetControl(0,A,0);
#endif
								/* wait for the conversion to complete */
								convert_sec += UNIMA_CONVERSION_SEC;
								convert_nsec += UNIMA_CONVERSION_NSEC;
								if (convert_nsec>=1000000000)
								{
									convert_sec++;
									convert_nsec -= 1000000000;
								}
								while (((temp_sec=real_time_clock_sec())<convert_sec)||
									((temp_sec==convert_sec)&&
									(real_time_clock_nsec()<convert_nsec)));
								convert_sec=real_time_clock_sec();
								convert_nsec=real_time_clock_nsec();
#if defined (AUTO_START_AD)
								prevent_removal_by_optimizer += *interface_module_1>>16;
#if (NUM_IO_CARDS>1)
								prevent_removal_by_optimizer +=	*interface_module_2>>16;
#if (NUM_IO_CARDS>2)
								prevent_removal_by_optimizer +=	*interface_module_3>>16;
#if (NUM_IO_CARDS>3)
								prevent_removal_by_optimizer +=	*interface_module_4>>16;
#if (NUM_IO_CARDS>4)
								prevent_removal_by_optimizer +=	*interface_module_5>>16;
#if (NUM_IO_CARDS>5)
								prevent_removal_by_optimizer +=	*interface_module_6>>16;
#if (NUM_IO_CARDS>6)
								prevent_removal_by_optimizer +=	*interface_module_7>>16;
#if (NUM_IO_CARDS>7)
								prevent_removal_by_optimizer +=	*interface_module_8>>16;
#if (NUM_IO_CARDS>8)
								prevent_removal_by_optimizer +=	*interface_module_9>>16;
#if (NUM_IO_CARDS>9)
								prevent_removal_by_optimizer +=	*interface_module_10>>16;
#if (NUM_IO_CARDS>10)
								prevent_removal_by_optimizer +=	*interface_module_11>>16;
#if (NUM_IO_CARDS>11)
								prevent_removal_by_optimizer +=	*interface_module_12>>16;
#if (NUM_IO_CARDS>12)
								prevent_removal_by_optimizer +=	*interface_module_13>>16;
#if (NUM_IO_CARDS>13)
								prevent_removal_by_optimizer +=	*interface_module_14>>16;
#if (NUM_IO_CARDS>14)
								prevent_removal_by_optimizer +=	*interface_module_15>>16;
#if (NUM_IO_CARDS>15)
								prevent_removal_by_optimizer +=	*interface_module_16>>16;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#else
								/* send a multicast conversion command */
								USetControl(0,A,1);
								USetControl(0,A,0);
#endif
								/* wait for the conversion to complete */
								convert_sec += UNIMA_CONVERSION_SEC;
								convert_nsec += UNIMA_CONVERSION_NSEC;
								if (convert_nsec>=1000000000)
								{
									convert_sec++;
									convert_nsec -= 1000000000;
								}
								while (((temp_sec=real_time_clock_sec())<convert_sec)||
									((temp_sec==convert_sec)&&
									(real_time_clock_nsec()<convert_nsec)));
#if defined (INTERRUPT)
								/* set the "refresh" function */
								acquisition_signal.sa_handler=refresh_signal;
								acquisition_signal.sa_mask.losigs=0;
								acquisition_signal.sa_mask.hisigs=0;
								acquisition_signal.sa_flags=0;
								if (-1!=sigaction(SIGUSR1,&acquisition_signal,
									(struct sigaction *)NULL))
								{
									/* set the "tidyup" function */
									acquisition_signal.sa_handler=tidyup_signal;
									if (-1!=sigaction(SIGUSR2,&acquisition_signal,
										(struct sigaction *)NULL))
									{
										/* shrink the stack and lock the process code, data and
											stack */
										resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
										if ((0==setrlimit(RLIMIT_STACK,&resource_limits))&&
											(0==plock(PROCLOCK)))
										{
											/* initialize the acquisition interrupt data */
											/* determine the interrupt interval required. */
											acquisition_interrupt_data.interval_nsec=
												sampling_period_nsec;
printf("interrupt interval = %d\n",acquisition_interrupt_data.interval_nsec);
											acquisition_interrupt_data.process_id=getpid();
											acquisition_interrupt_data.experiment_time=0;
											acquisition_interrupt_data.refresh_count=0;
											acquisition_interrupt_data.on=0;
											acquisition_interrupt_data.signals=
												buffer->signals.short_int_values;
											acquisition_interrupt_data.times=buffer->times;
											acquisition_interrupt_data.sample_number=
											acquisition_interrupt_data.number_of_samples=
												(unsigned short)(buffer->number_of_samples);
											acquisition_interrupt_data.unima_io_offset=
												unima_map_area.IOoffset;
											for (i=0;i<17;i++)
											{
												acquisition_interrupt_data.unima_interface_registers[i]=
													UUserConfigRegister[i];
											}
/*??? temporary for testing */
printf("interrupt display %p\n",interrupt_display);
acquisition_drawing_window=XtWindow(acquisition->window->drawing_area);
XGetWindowAttributes(display,acquisition_drawing_window,&attributes);
acquisition_drawing_height=attributes.height;
acquisition_drawing_width=attributes.width;
acquisition_drawing_marker=0;
/*???debug */
trcstart("-af");
trcon(0);
											if (0==start_acquisition_interrupt(
												&acquisition_interrupt_data))
											{
printf("on=%d\n",(int)(acquisition_interrupt_data.on));
#endif
#endif
												rig->experiment=EXPERIMENT_ON;
												/* set the experiment toggle on */
												XtVaSetValues(acquisition->experiment_toggle,XmNset,
													True,NULL);
												/* ghost the mapping file button */
												if ((acquisition->mapping_window_address)&&
													(*(acquisition->mapping_window_address)))
												{
													XtSetSensitive(
														(*(acquisition->mapping_window_address))->
														file_button,False);
												}
												/*??? have a function to keep consistent ? */
												/* ghost the calibrate button */
												XtSetSensitive(acquisition->calibrate_button,False);
												/* unghost the monitoring_toggle_button */
												XtSetSensitive(acquisition->monitoring_toggle,True);
												/* set the monitoring toggle off */
												XtVaSetValues(acquisition->monitoring_toggle,XmNset,
													False,NULL);
												start_stop_monitoring((Widget)NULL,acquisition_window,
													(XtPointer)NULL);
												/* unghost the acquire_button */
													/*???DB.  temp */
												XtSetSensitive(acquisition->acquire_button,True);
#if defined (UNIMA)
#if defined (INTERRUPT)
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"start_experiment.  Could not start interrupting");
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
										"start_experiment.  Could not lock the process in memory");
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"start_experiment.  Could not set the tidyup signal");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"start_experiment.  Could not set the refresh signal");
								}
#endif
							}
							else
							{
								while (index>0)
								{
									device--;
									destroy_Signal(&((*device)->signal));
									index--;
								}
								display_message(ERROR_MESSAGE,
									"start_experiment.  Could not allocate memory for signals");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"start_experiment.  Could not allocate memory for buffer");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"start_experiment.  Invalid rig");
					}
#if defined (INTERRUPT)
				}
#endif
#endif
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"start_experiment.  Invalid experiment directory");
		}
	}
	else
	{
		if (acquisition)
		{
			if (acquisition->rig_address)
			{
				display_message(ERROR_MESSAGE,
					"start_experiment.  acquisition rig missing");
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"start_experiment.  acquisition rig address missing");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"start_experiment.  acquisition window missing");
		}
	}
	LEAVE;

	return (return_code);
} /* start_experiment */

int acquisition_write_signal_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 1 December 1993

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct Rig *rig;

	ENTER(acquisition_write_signal_file);
	/* check that the rig exists */
	if (rig= *((struct Rig **)rig_pointer))
	{
		/* open the output file */
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
				"acquisition_write_signal_file.  Invalid file: %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"acquisition_write_signal_file.  Missing rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* acquisition_write_signal_file */

/*
Global functions
----------------
*/
void acquire_data(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
***Serge***
see struct Signal_buffer in rig.h
When the user specified acquisition interval is in the buffer (determined by
looking at the current time) the UNIMA system will have to swap to writing to
the other buffer.
Assuming that the UNIMA auto start mode is being used.
=============================================================================*/
{
	struct Acquisition_window *acquisition;
	struct Signal_buffer *buffer;
	struct Rig *rig;
	struct Device **device;
#if defined (POLLING)
	int i,index,number_of_samples,sampling_period_nsec,*times;
	int channel,old_priority,sample;
	/* times */
	unsigned long convert_nsec,convert_sec,next_nsec,next_sec,temp_sec;
	short int prevent_removal_by_optimizer;
	struct rlimit resource_limits;
	short int k,*signals;
#endif /* defined (POLLING) */

	ENTER(acquire_data);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((acquisition=(struct Acquisition_window *)acquisition_window)&&
		(rig= *(acquisition->rig_address))&&(device=rig->devices)&&
		((*device)->signal)&&(buffer=(*device)->signal->buffer)&&
		(
#if defined (POLLING)
		times=
#endif /* defined (POLLING) */
		buffer->times)&&(SHORT_INT_VALUE==buffer->value_type)&&
		(
#if defined (POLLING)
		signals=
#endif /* defined (POLLING) */
		buffer->signals.short_int_values)&&((
#if defined (POLLING)
		number_of_samples=
#endif /* defined (POLLING) */
		(acquisition->end_time)-(acquisition->start_time))>0))
	{
#if defined (POLLING)
		sampling_period_nsec=1000000000/(acquisition->sampling_frequency_hz);
		/* shrink the stack and lock the process code, data and stack */
		resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
/*???The problem of not being able to lock the process into memory is because
	I'm are trying to lock everything (stack, all data and all code) into real
	memory.  I should just lock what I need into memory.  I should be able to
	lock the acquisition buffer (signals and times) and the function acquire
	using the memory pinning kernel extension in the red book.  I can use the
	same extension to pin the stack once I know what the base address is. */
		if ((0==setrlimit(RLIMIT_STACK,&resource_limits))&&(0==plock(PROCLOCK)))
/*		if ((0==plock(TXTLOCK))&&
			(0==pin_memory(times,number_of_samples*sizeof(*times)))&&
			(0==pin_memory(signals,
			number_of_samples*NUM_IO_CARDS*MUX_CHANNELS*sizeof(*times))))*/
		{
			/* turn the interrupts off */
			old_priority=disable_interrupts(1);
			/* get the start time */
			next_sec=real_time_clock_sec();
			next_nsec=real_time_clock_nsec();
#if defined (AUTO_START_AD)
			/* throw away the first reading (auto start) */
			convert_sec=next_sec;
			convert_nsec=next_nsec;
			/* the values returned by UNIMA are not important, but the conversions
				must be carried out.  To prevent the optimizer from removing the
				conversion statements the values returned must be used */
			prevent_removal_by_optimizer=0;
			prevent_removal_by_optimizer += *interface_module_1>>16;
#if (NUM_IO_CARDS>1)
			prevent_removal_by_optimizer += *interface_module_2>>16;
#if (NUM_IO_CARDS>2)
			prevent_removal_by_optimizer += *interface_module_3>>16;
#if (NUM_IO_CARDS>3)
			prevent_removal_by_optimizer += *interface_module_4>>16;
#if (NUM_IO_CARDS>4)
			prevent_removal_by_optimizer += *interface_module_5>>16;
#if (NUM_IO_CARDS>5)
			prevent_removal_by_optimizer += *interface_module_6>>16;
#if (NUM_IO_CARDS>6)
			prevent_removal_by_optimizer += *interface_module_7>>16;
#if (NUM_IO_CARDS>7)
			prevent_removal_by_optimizer += *interface_module_8>>16;
#if (NUM_IO_CARDS>8)
			prevent_removal_by_optimizer += *interface_module_9>>16;
#if (NUM_IO_CARDS>9)
			prevent_removal_by_optimizer += *interface_module_10>>16;
#if (NUM_IO_CARDS>10)
			prevent_removal_by_optimizer += *interface_module_11>>16;
#if (NUM_IO_CARDS>11)
			prevent_removal_by_optimizer += *interface_module_12>>16;
#if (NUM_IO_CARDS>12)
			prevent_removal_by_optimizer += *interface_module_13>>16;
#if (NUM_IO_CARDS>13)
			prevent_removal_by_optimizer += *interface_module_14>>16;
#if (NUM_IO_CARDS>14)
			prevent_removal_by_optimizer += *interface_module_15>>16;
#if (NUM_IO_CARDS>15)
			prevent_removal_by_optimizer += *interface_module_16>>16;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#else
			/* send a multicast conversion command */
			USetControl(0,A,1);
			USetControl(0,A,0);
			/* find the conversion time */
			convert_sec=real_time_clock_sec();
			convert_nsec=real_time_clock_nsec();
#endif
			channel_number++;
			if (channel_number>=MUX_CHANNELS)
			{
				channel_number=0;
			}
			/* acquire the data */
			for (sample=0;sample<number_of_samples;sample++)
			{
				*(times++)=sample;
				/* for each channel */
				for (channel=MUX_CHANNELS;channel>0;channel--)
				{
					/* wait for the conversion to complete */
					convert_sec += UNIMA_CONVERSION_SEC;
					convert_nsec += UNIMA_CONVERSION_NSEC;
					if (convert_nsec>=1000000000)
					{
						convert_sec++;
						convert_nsec -= 1000000000;
					}
					while (((temp_sec=real_time_clock_sec())<convert_sec)||
						((temp_sec==convert_sec)&&(real_time_clock_nsec()<convert_nsec)));
#if defined (AUTO_START_AD)
					/* find the conversion time */
					convert_sec=real_time_clock_sec();
					convert_nsec=real_time_clock_nsec();
					/* Read the last conversion back and trigger the next conversion */
#endif
					GETSWAP16(*(signals++),*interface_module_1);
#if (NUM_IO_CARDS>1)
					GETSWAP16(*(signals++),*interface_module_2);
#if (NUM_IO_CARDS>2)
					GETSWAP16(*(signals++),*interface_module_3);
#if (NUM_IO_CARDS>3)
					GETSWAP16(*(signals++),*interface_module_4);
#if (NUM_IO_CARDS>4)
					GETSWAP16(*(signals++),*interface_module_5);
#if (NUM_IO_CARDS>5)
					GETSWAP16(*(signals++),*interface_module_6);
#if (NUM_IO_CARDS>6)
					GETSWAP16(*(signals++),*interface_module_7);
#if (NUM_IO_CARDS>7)
					GETSWAP16(*(signals++),*interface_module_8);
#if (NUM_IO_CARDS>8)
					GETSWAP16(*(signals++),*interface_module_9);
#if (NUM_IO_CARDS>9)
					GETSWAP16(*(signals++),*interface_module_10);
#if (NUM_IO_CARDS>10)
					GETSWAP16(*(signals++),*interface_module_11);
#if (NUM_IO_CARDS>11)
					GETSWAP16(*(signals++),*interface_module_12);
#if (NUM_IO_CARDS>12)
					GETSWAP16(*(signals++),*interface_module_13);
#if (NUM_IO_CARDS>13)
					GETSWAP16(*(signals++),*interface_module_14);
#if (NUM_IO_CARDS>14)
					GETSWAP16(*(signals++),*interface_module_15);
#if (NUM_IO_CARDS>15)
					GETSWAP16(*(signals++),*interface_module_16);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#if !defined (AUTO_START_AD)
					/* send a multicast conversion command */
					USetControl(0,A,1);
					USetControl(0,A,0);
					/* find the conversion time */
					convert_sec=real_time_clock_sec();
					convert_nsec=real_time_clock_nsec();
#endif
					channel_number++;
					if (channel_number>=MUX_CHANNELS)
					{
						channel_number=0;
					}
				}
				/* wait for next sampling time */
				next_nsec += sampling_period_nsec;
				if (next_nsec>=1000000000)
				{
					next_sec++;
					next_nsec -= 1000000000;
				}
				while (((temp_sec=real_time_clock_sec())<next_sec)||
					((temp_sec==next_sec)&&(real_time_clock_nsec()<next_nsec)));
			}
			/* turn the interrupts on */
			enable_interrupts(old_priority);
			/* unlock the process code, data and stack */
			plock(UNLOCK);
			/* set the indices for the channels */
			for (i=rig->number_of_devices;i>0;i--)
			{
				index=((*device)->channel->number-1)%MUX_CHANNELS-channel_number;
				if (index<0)
				{
					index += MUX_CHANNELS;
				}
				(*device)->signal->index=index*NUM_IO_CARDS+
					((*device)->channel->number-1)/MUX_CHANNELS;
				device++;
			}
			/* set the buffer start and end */
			buffer->start=0;
			buffer->end=number_of_samples-1;
			/* write to disk */
			if (!(acquisition->acquire_file_open_data))
			{
				acquisition->acquire_file_open_data=create_File_open_data(
					acquisition->signal_file_extension_write,REGULAR,
					acquisition_write_signal_file,
					(XtPointer)(acquisition->rig_address),0,
					acquisition->user_interface);
			}
			if (acquisition->acquire_file_open_data)
			{
				open_file_and_write(acquisition->acquire_button,
					(XtPointer)(acquisition->acquire_file_open_data),(XtPointer)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"acquire_data.  Could not create file open data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"acquire_data.  Could not lock the process in memory");
		}
#endif /* defined (POLLING) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"acquire_data.  Buffer missing");
	}
	LEAVE;
} /* acquire_data */

void start_stop_experiment(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/******************************************************************************
LAST MODIFIED : 24 November 1996

DESCRIPTION :
-stop
Stop the UNIMA system.
Deallocate the storage for the signal buffer.
=============================================================================*/
{
	Boolean status;
	struct Acquisition_window *acquisition;
	struct Rig *rig;
#if defined (UNIMA)
	int index,number_of_devices;
	struct Device **device;
	struct Signal_buffer *buffer;
#endif

	ENTER(start_stop_experiment);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		if (acquisition->rig_address)
		{
			if (rig= *(acquisition->rig_address))
			{
				XtVaGetValues(acquisition->experiment_toggle,XmNset,&status,NULL);
				if (True==status)
				{
					if (EXPERIMENT_OFF==rig->experiment)
					{
						/* set the experiment toggle off */
						XtVaSetValues(acquisition->experiment_toggle,XmNset,False,NULL);
						/* attempt to open the experiment directory */
						if (!(acquisition->experiment_file_open_data))
						{
							acquisition->experiment_file_open_data=create_File_open_data(
								(char *)NULL,DIRECTORY,start_experiment,(void *)acquisition,0,
								acquisition->user_interface);
						}
						if (acquisition->experiment_file_open_data)
						{
							open_file_and_write(widget,
								(XtPointer)(acquisition->experiment_file_open_data),
								(XtPointer)NULL);
						}
					}
				}
				else
				{
					if (EXPERIMENT_ON==rig->experiment)
					{
						/* set the monitoring toggle off */
						XtVaSetValues(acquisition->monitoring_toggle,XmNset,False,NULL);
						/* stop monitoring */
						start_stop_monitoring((Widget)NULL,acquisition_window,
							(XtPointer)NULL);
						/*???DB.  temp */
						XtSetSensitive(acquisition->acquire_button,False);
						/* ghost the monitoring_toggle_button */
						XtSetSensitive(acquisition->monitoring_toggle,False);
						/* unghost the calibrate button */
						XtSetSensitive(acquisition->calibrate_button,True);
						/*??? stop the experiment */
#if defined (UNIMA)
#if defined (INTERRUPT)
						/* stop the interrupting */
						stop_acquisition_interrupt(&acquisition_interrupt_data);
#endif
/*???temporary #if */
#if !defined (INTERRUPT)
						if ((device=rig->devices)&&
							((number_of_devices=rig->number_of_devices)>0))
						{
							buffer= (*device)->signal->buffer;
							/* destroy signals */
							index=0;
							while (index<number_of_devices)
							{
								index++;
								destroy_Signal(&((*device)->signal));
								device++;
							}
						}
#endif
						/* reset the Unima system */
						U00SysReset();
						/* reset the Unima adapter */
						UAReset();
						UASetAdapter(DISABLE);
#endif
						rig->experiment=EXPERIMENT_OFF;
						/* unghost the mapping file button */
						if ((acquisition->mapping_window_address)&&
							(*(acquisition->mapping_window_address)))
						{
							XtSetSensitive((*(acquisition->mapping_window_address))->
								file_button,True);
						}
						/*??? have a function to keep consistent ? */
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"start_stop_experiment.  acquisition rig missing");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"start_stop_experiment.  acquisition rig address missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"start_stop_experiment.  acquisition window missing");
	}
	LEAVE;
} /* start_stop_experiment */

void start_stop_monitoring(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
***Serge***
-start
see struct Signal_buffer in rig.h
Start the UNIMA system writing times and measurements to the buffer and
updating the first time and last time pointers.
-stop
see struct Signal_buffer in rig.h
Stop the UNIMA system writing times and measurements to the buffer and
updating the first time and last time pointers.
=============================================================================*/
{
	Boolean status;
	struct Acquisition_window *acquisition;
	struct Rig *rig;
#if defined (POLLING)
	int i,index,number_of_samples,sampling_period_nsec,*times;
	int acquire,acquire_time,buffer_full,buffer_size,channel,old_priority,sample;
	/* times */
	unsigned long convert_nsec,convert_sec,next_nsec,next_sec,temp_sec;
	short int prevent_removal_by_optimizer;
	short int k,*signal,*signals;
	struct rlimit resource_limits;
	unsigned switch_state;
	struct Device **device;
	struct Signal_buffer *buffer;
#endif /* defined (POLLING) */

	ENTER(start_stop_monitoring);
#if !defined (POLLING)
	USE_PARAMETER(widget);
#endif /* !defined (POLLING) */
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		if (acquisition->rig_address)
		{
			if (rig= *(acquisition->rig_address))
			{
				XtVaGetValues(acquisition->monitoring_toggle,XmNset,&status,NULL);
				if (True==status)
				{
					if (MONITORING_OFF==rig->monitoring)
					{
						/*??? start monitoring */
						rig->monitoring=MONITORING_ON;
						/* unghost the acquisition_page_button */
						if ((acquisition->mapping_window_address)&&
							(*(acquisition->mapping_window_address)))
						{
							XtSetSensitive(
								(*(acquisition->mapping_window_address))->page_button,True);
						}
						/* unghost the acquire_button */
						XtSetSensitive(acquisition->acquire_button,True);
#if defined (POLLING)
						sampling_period_nsec=
							1000000000/(acquisition->sampling_frequency_hz);
						if ((device=rig->devices)&&((*device)->signal)&&
							(buffer=(*device)->signal->buffer)&&(times=buffer->times)&&
							(SHORT_INT_VALUE==buffer->value_type)&&
							(signals=buffer->signals.short_int_values)&&((number_of_samples=
							(acquisition->end_time)-(acquisition->start_time))>0)&&
							((acquire_time=(acquisition->acquire_time)-
							(acquisition->start_time))>=0))
						{
							/* finish updating the display */
							XSync(display,False);
							/* shrink the stack and lock the process code, data and stack */
							resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
					/*???The problem of not being able to lock the process into memory is
						because I'm are trying to lock everything (stack, all data and all
						code) into real memory.  I should just lock what I need into memory.
						I should be able to lock the acquisition buffer (signals and times)
						and the function acquire using the memory pinning kernel extension
						in the red book.  I can use the same extension to pin the stack once
						I know what the base address is. */
							if ((0==setrlimit(RLIMIT_STACK,&resource_limits))&&
								(0==plock(PROCLOCK)))
					/*		if ((0==plock(TXTLOCK))&&
								(0==pin_memory(times,number_of_samples*sizeof(*times)))&&
								(0==pin_memory(signals,
								number_of_samples*NUM_IO_CARDS*MUX_CHANNELS*sizeof(*times))))*/
							{
								/* turn the interrupts off */
								old_priority=disable_interrupts(1);
								/* get the start time */
								next_sec=real_time_clock_sec();
								next_nsec=real_time_clock_nsec();
#if defined (AUTO_START_AD)
								/* throw away the first reading (auto start) */
								convert_sec=next_sec;
								convert_nsec=next_nsec;
								/* the values returned by UNIMA are not important, but the
									conversions must be carried out.  To prevent the optimizer
									from removing the conversion statements the values returned
									must be used */
								prevent_removal_by_optimizer=0;
								prevent_removal_by_optimizer += *interface_module_1>>16;
#if (NUM_IO_CARDS>1)
								prevent_removal_by_optimizer += *interface_module_2>>16;
#if (NUM_IO_CARDS>2)
								prevent_removal_by_optimizer += *interface_module_3>>16;
#if (NUM_IO_CARDS>3)
								prevent_removal_by_optimizer += *interface_module_4>>16;
#if (NUM_IO_CARDS>4)
								prevent_removal_by_optimizer += *interface_module_5>>16;
#if (NUM_IO_CARDS>5)
								prevent_removal_by_optimizer += *interface_module_6>>16;
#if (NUM_IO_CARDS>6)
								prevent_removal_by_optimizer += *interface_module_7>>16;
#if (NUM_IO_CARDS>7)
								prevent_removal_by_optimizer += *interface_module_8>>16;
#if (NUM_IO_CARDS>8)
								prevent_removal_by_optimizer += *interface_module_9>>16;
#if (NUM_IO_CARDS>9)
								prevent_removal_by_optimizer += *interface_module_10>>16;
#if (NUM_IO_CARDS>10)
								prevent_removal_by_optimizer += *interface_module_11>>16;
#if (NUM_IO_CARDS>11)
								prevent_removal_by_optimizer += *interface_module_12>>16;
#if (NUM_IO_CARDS>12)
								prevent_removal_by_optimizer += *interface_module_13>>16;
#if (NUM_IO_CARDS>13)
								prevent_removal_by_optimizer += *interface_module_14>>16;
#if (NUM_IO_CARDS>14)
								prevent_removal_by_optimizer += *interface_module_15>>16;
#if (NUM_IO_CARDS>15)
								prevent_removal_by_optimizer += *interface_module_16>>16;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#else
								/* send a multicast conversion command */
								USetControl(0,A,1);
								USetControl(0,A,0);
								/* find the conversion time */
								convert_sec=real_time_clock_sec();
								convert_nsec=real_time_clock_nsec();
#endif
								channel_number++;
								if (channel_number>=MUX_CHANNELS)
								{
									channel_number=0;
								}
								/* monitor the channels */
								acquire=0;
								signal=signals;
								buffer_size=
									(buffer->number_of_samples)*MUX_CHANNELS*NUM_IO_CARDS;
								buffer_full=0;
								while (!acquire||(sample<number_of_samples))
								{
									/* for each channel */
									for (channel=MUX_CHANNELS;channel>0;channel--)
									{
										/* wait for the conversion to complete */
										convert_sec += UNIMA_CONVERSION_SEC;
										convert_nsec += UNIMA_CONVERSION_NSEC;
										if (convert_nsec>=1000000000)
										{
											convert_sec++;
											convert_nsec -= 1000000000;
										}
										while (((temp_sec=real_time_clock_sec())<convert_sec)||
											((temp_sec==convert_sec)&&
											(real_time_clock_nsec()<convert_nsec)));
#if defined (AUTO_START_AD)
										/* find the conversion time */
										convert_sec=real_time_clock_sec();
										convert_nsec=real_time_clock_nsec();
										/* Read the last conversion back and trigger the next
											conversion */
#endif
										GETSWAP16(*(signal++),*interface_module_1);
#if (NUM_IO_CARDS>1)
										GETSWAP16(*(signal++),*interface_module_2);
#if (NUM_IO_CARDS>2)
										GETSWAP16(*(signal++),*interface_module_3);
#if (NUM_IO_CARDS>3)
										GETSWAP16(*(signal++),*interface_module_4);
#if (NUM_IO_CARDS>4)
										GETSWAP16(*(signal++),*interface_module_5);
#if (NUM_IO_CARDS>5)
										GETSWAP16(*(signal++),*interface_module_6);
#if (NUM_IO_CARDS>6)
										GETSWAP16(*(signal++),*interface_module_7);
#if (NUM_IO_CARDS>7)
										GETSWAP16(*(signal++),*interface_module_8);
#if (NUM_IO_CARDS>8)
										GETSWAP16(*(signal++),*interface_module_9);
#if (NUM_IO_CARDS>9)
										GETSWAP16(*(signal++),*interface_module_10);
#if (NUM_IO_CARDS>10)
										GETSWAP16(*(signal++),*interface_module_11);
#if (NUM_IO_CARDS>11)
										GETSWAP16(*(signal++),*interface_module_12);
#if (NUM_IO_CARDS>12)
										GETSWAP16(*(signal++),*interface_module_13);
#if (NUM_IO_CARDS>13)
										GETSWAP16(*(signal++),*interface_module_14);
#if (NUM_IO_CARDS>14)
										GETSWAP16(*(signal++),*interface_module_15);
#if (NUM_IO_CARDS>15)
										GETSWAP16(*(signal++),*interface_module_16);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#if !defined (AUTO_START_AD)
										/* send a multicast conversion command */
										USetControl(0,A,1);
										USetControl(0,A,0);
										/* find the conversion time */
										convert_sec=real_time_clock_sec();
										convert_nsec=real_time_clock_nsec();
#endif
										channel_number++;
										if (channel_number>=MUX_CHANNELS)
										{
											channel_number=0;
										}
									}
									if (acquire)
									{
										sample++;
									}
									else
									{
										/* check if the switch is closed */
										switch_state=(unsigned int)(*(UnimaBusPort+23));
										/*???DB.  switch_state was coming back with a b in the
											second byte from the right - Sylvain said that it is
											something to do with a short, but I shouldn't worry about
											it.  So */
										switch_state=switch_state|0xf0ffffff;
										if ((0xfdffffff==switch_state)||(0xfeffffff==switch_state))
										{
											acquire=1;
											acquire_time=
												acquisition->acquire_time-acquisition->start_time+1;
											if (buffer_full)
											{
												sample=acquire_time;
											}
											else
											{
												if ((sample=(signal-signals)/
													(MUX_CHANNELS*NUM_IO_CARDS))>acquire_time)
												{
													sample=acquire_time;
												}
											}
										}
									}
									/* wait for next sampling time */
									next_nsec += sampling_period_nsec;
									if (next_nsec>=1000000000)
									{
										next_sec++;
										next_nsec -= 1000000000;
									}
									/* make buffer circular */
									if (signal-signals>=buffer_size)
									{
										signal=signals;
										buffer_full=1;
									}
									while (((temp_sec=real_time_clock_sec())<next_sec)||
										((temp_sec==next_sec)&&(real_time_clock_nsec()<next_nsec)));
								}
								/* turn the interrupts on */
								enable_interrupts(old_priority);
								/* unlock the process code, data and stack */
								plock(UNLOCK);
								/* set the indices for the channels */
								for (i=rig->number_of_devices;i>0;i--)
								{
									index=
										((*device)->channel->number-1)%MUX_CHANNELS-channel_number;
									if (index<0)
									{
										index += MUX_CHANNELS;
									}
									(*device)->signal->index=index*NUM_IO_CARDS+
										((*device)->channel->number-1)/MUX_CHANNELS;
									device++;
								}
								/* set the buffer start and end */
								buffer->end=(signal-signals)/(MUX_CHANNELS*NUM_IO_CARDS);
								if (0==buffer->end)
								{
									buffer->end=buffer->number_of_samples-1;
								}
								else
								{
									buffer->end--;
								}
								if ((buffer->start=buffer->end-number_of_samples+1)<0)
								{
									buffer->start += buffer->number_of_samples;
								}
								/* set the times */
								sample=0;
								times += buffer->start;
								if (buffer->start<=buffer->end)
								{
									for (i=number_of_samples;i>0;i--)
									{
										*(times++)=sample;
										sample++;
									}
								}
								else
								{
									for (i=buffer->number_of_samples-buffer->start;i>0;i--)
									{
										*(times++)=sample;
										sample++;
									}
									times=buffer->times;
									for (i=buffer->end;i>=0;i--)
									{
										*(times++)=sample;
										sample++;
									}
								}
								/* write to disk */
								if (!(acquisition->monitoring_file_open_data))
								{
									acquisition->monitoring_file_open_data=create_File_open_data(
										acquisition->signal_file_extension_write,REGULAR,
										acquisition_write_signal_file,
										(XtPointer)(acquisition->rig_address),0,
										acquisition->user_interface);
								}
								if (acquisition->monitoring_file_open_data)
								{
									open_file_and_write(acquisition->monitoring_toggle,
										(XtPointer)(acquisition->monitoring_file_open_data),
										(XtPointer)NULL);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"start_stop_monitoring.  Could not create file open data");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"start_stop_monitoring.  Could not lock the process in memory");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"start_stop_monitoring.  Buffer missing");
						}
						/* turn off monitoring */
						XtVaSetValues(acquisition->monitoring_toggle,XmNset,False,NULL);
						start_stop_monitoring(widget,acquisition_window,(XtPointer)NULL);
#endif /* defined (POLLING) */
					}
				}
				else
				{
					if (MONITORING_ON==rig->monitoring)
					{
						/* ghost the acquire_button */
							/*???DB.  temp comment out */
/*						XtSetSensitive(acquisition->acquire_button,False);*/
						/*??? stop the page displays */
						/* ghost the acquisition_page_button */
						if ((acquisition->mapping_window_address)&&
							(*(acquisition->mapping_window_address)))
						{
							XtSetSensitive(
								(*(acquisition->mapping_window_address))->page_button,False);
						}
						/*??? stop monitoring */
						rig->monitoring=MONITORING_OFF;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"start_stop_monitoring.  acquisition rig missing");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"start_stop_monitoring.  acquisition rig address missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"start_stop_monitoring.  acquisition window missing");
	}
	LEAVE;
} /* start_stop_monitoring */

void calibrate_channels(Widget widget,XtPointer acquisition_window,
	XtPointer call_data)
/******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
=============================================================================*/
{
#if defined (UNIMA)
	char *calibration_file_name;
	FILE *calibration_file;
	float channel_gain,channel_offset,*gain,*offset;
	int number,i;
	struct Acquisition_window *acquisition;
#endif /* defined (UNIMA) */

	ENTER(calibrate_channels);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (
#if defined (UNIMA)
		acquisition=
#endif /* defined (UNIMA) */
		(struct Acquisition_window *)acquisition_window)
	{
#if defined (UNIMA)
		/* initialize the offsets and gains */
		offset=channel_offsets;
		gain=channel_gains;
		for (i=MUX_CHANNELS*NUM_IO_CARDS;i>0;i--)
		{
			*offset=0;
			*gain=1;
			offset++;
			gain++;
		}
		if (0<strlen(acquisition->calibration_directory))
		{
			if (ALLOCATE(calibration_file_name,char,
				strlen(acquisition->calibration_directory)+15))
			{
				strcpy(calibration_file_name,acquisition->calibration_directory);
				strcat(calibration_file_name,"/calibrate.dat");
			}
		}
		else
		{
			if (ALLOCATE(calibration_file_name,char,14))
			{
				strcpy(calibration_file_name,"calibrate.dat");
			}
		}
		/* read in any current offsets and gains */
		if (calibration_file_name)
		{
			if (calibration_file=fopen(calibration_file_name,"r"))
			{
				/* skip the heading */
				if (EOF!=fscanf(calibration_file,"%*[^\n]\n"))
				{
					/* read the channel calibrations */
					while (3==fscanf(calibration_file,"%d %f %f \n",&number,
						&channel_offset,&channel_gain))
					{
						if ((number>0)&&(number<=MUX_CHANNELS*NUM_IO_CARDS))
						{
							channel_offsets[number-1]=channel_offset;
							channel_gains[number-1]=channel_gain;
						}
					}
					if (!feof(calibration_file))
					{
						display_message(ERROR_MESSAGE,
							"calibrate_channels.  Error reading file");
					}
				}
				fclose(calibration_file);
			}
			DEALLOCATE(calibration_file_name);
		}
		print_question((XtPointer)calculate_offsets,acquisition_window,
			(XtPointer)cancel_calibrate,(XtPointer)NULL,
			1,"Are you ready to calibrate offsets ?");
#endif /* defined (UNIMA) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calibrate_channels.  acquisition window missing");
	}
	LEAVE;
} /* calibrate_channels */

void set_calibrate_interface_module(Widget widget,XtPointer module,
	XtPointer call_data)
/******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
=============================================================================*/
{
#if defined (UNIMA)
	int interface_module;
	short int prevent_removal_by_optimizer;
	unsigned long convert_nsec,convert_sec,start_nsec,start_sec,temp_sec;
#endif

	ENTER(set_calibrate_interface_module);
	USE_PARAMETER(widget);
#if !defined (UNIMA)
	USE_PARAMETER(module);
#endif /* !defined (UNIMA) */
	USE_PARAMETER(call_data);
#if defined (UNIMA)
	if ((0<(interface_module= *((int *)module)))&&
		(interface_module<=NUM_IO_CARDS))
	{
		calibrate_module_number=interface_module;
		/* start Unima */
		Unima(1,0);
		/* wait for the UNIMA initialization to complete */
		start_sec += UNIMA_INITIALIZE_SEC;
		start_nsec += UNIMA_INITIALIZE_NSEC;
		if (start_nsec>=1000000000)
		{
			start_sec++;
			start_nsec -= 1000000000;
		}
		while (((temp_sec=real_time_clock_sec())<start_sec)||
			((temp_sec==start_sec)&&(real_time_clock_nsec()<start_nsec)));
		/* the number of the pair of pins on each EMAP card where the next
			conversion will be */
		channel_number=STARTING_CHANNEL_NUMBER;
		/*???should rewrite Unima (in unima_syscall.c) so that has a return code */
/*		if (!Unima(1,0))
		{*/
			calibrate_module=(unsigned int *)(UnimaBusPort+
				8*calibrate_module_number+UN05_CHANNEL0);
			/* get rig of garbage */
			/* the values returned by UNIMA are not important, but the conversions
				must be carried out.  To prevent the optimizer from removing the
				conversion statements the values returned must be used */
			prevent_removal_by_optimizer=0;
			/* find the conversion time */
			convert_sec=real_time_clock_sec();
			convert_nsec=real_time_clock_nsec();
			prevent_removal_by_optimizer += *calibrate_module>>16;
			/* wait for the conversion to complete */
			convert_sec += UNIMA_CONVERSION_SEC;
			convert_nsec += UNIMA_CONVERSION_NSEC;
			if (convert_nsec>=1000000000)
			{
				convert_sec++;
				convert_nsec -= 1000000000;
			}
			while (((temp_sec=real_time_clock_sec())<convert_sec)||
				((temp_sec==convert_sec)&&(real_time_clock_nsec()<convert_nsec)));
			/* find the conversion time */
			convert_sec=real_time_clock_sec();
			convert_nsec=real_time_clock_nsec();
			prevent_removal_by_optimizer += *calibrate_module>>16;
			/* wait for the conversion to complete */
			convert_sec += UNIMA_CONVERSION_SEC;
			convert_nsec += UNIMA_CONVERSION_NSEC;
			if (convert_nsec>=1000000000)
			{
				convert_sec++;
				convert_nsec -= 1000000000;
			}
			while (((temp_sec=real_time_clock_sec())<convert_sec)||
				((temp_sec==convert_sec)&&(real_time_clock_nsec()<convert_nsec)));
/*		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_calibrate_interface_module.  Could not start UNIMA");
		}*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_calibrate_interface_module.  Invalid interface module");
	}
#endif
	LEAVE;
} /* set_calibrate_interface_module */
