/*******************************************************************************
FILE : acquisition_interrupt.c

LAST MODIFIED : 1 November 1995

DESCRIPTION :
AIX kernel extension to allow data acquisition from the UNIMA system.  Based on
the interval timer kernel extension in section A.2.2 (p.109) of
'IBM AIX Version 3.1 RISC System/6000 as a Real-Time System', Document Number
  GG24-3633-0, March 1991, International Technical Support Center Austin, Texas.

COMPILATION :
cc acquisition_interrupt.c -e configure_acquisition -o acquisition_interrupt \
  -lsys -lcsys -bI:/lib/kernex.exp -bE:./acquisition_interrupt.exp
Explanation of compiler flags (from p.59)
-e indicates that acquisition_interrupt is the entry point of the object file.
  This entry point may be called by the sysconfig() subroutine for kernel object
  initialization or termination.
-bE: indicates that the acquisition_interrupt.exp is the export file.
-bI: specifies the import files.
???Can I put this in acquisition.c ?
==============================================================================*/
/* header file for error numbers */
#include <sys/errno.h>
/* header file for AIX types */
#include <sys/types.h>
/* header file for sysconfig */
#include <sys/device.h>
/* header file for signal syscalls */
#include <sys/signal.h>
/* header file for timer syscalls */
#include <sys/timer.h>
/* header file for memory pinning functions */
	/*??? have to include pin.h to be able to us pinu */
#include <sys/pin.h>
#include <sys/uio.h>
/* header file for cross memory operations */
#include <sys/xmem.h>
/* the data that needs to be passed to the interrupt handler */
#include "acquisition_interrupt.h"
/*???I'm not sure about making UNIMA syscalls from the interrupt handler */
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include "UNIMA/uim.h"
#include "UNIMA/un05.h"

/*???debug */
	/*???to use trace macros (trchook) need -bI:/lib/syscalls.exp */
#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#define HK_0 0x00000000
#define HK_1 0x00100000
#define HK_2 0x00200000
#define HK_3 0x00300000

/*??? have to declare errno otherwise get an Exec format error (error number 8)
	when trying to load the extension */
	/*???in /lib/syscalls.exp */
/*int errno;*/

/*
Module variables
----------------
*/
/* signal display is updated every REFRESH_PERIOD interrupts */
#define REFRESH_PERIOD 10
struct Acquisition_interrupt_data interrupt_data;
struct xmem cross_memory_descriptor;
/* UNIMA micro-channel bus id */
#define UNIMA_BUS_ID 0x820c0020
/* Number of IO cards in UNIMA crate */
	/*???DB.  Now set in Makefile */
/*#define NUM_IO_CARDS 16*/
/* Number of channels/UNIMA IO card */
#define MUX_CHANNELS 32
/* Time in nanoseconds for a UNIMA to initialize */
#define UNIMA_INITIALIZE_NSEC 1500000000
/* Time in nanoseconds for a conversion to complete 10usec+32bits@10MHz */
#define UNIMA_CONVERSION_NSEC 14000
/* Unima pointers */
/* base address allocated by AIX */
	/*???volatile ? */
unsigned int *unima_bus_location,*unima_bus_port;
unsigned int *unima_base_io_registers;
/* addresses for the interface modules */
volatile unsigned int *interface_module_1,*interface_module_2,
	*interface_module_3,*interface_module_4,*interface_module_5,
	*interface_module_6,*interface_module_7,*interface_module_8,
	*interface_module_9,*interface_module_10,*interface_module_11,
	*interface_module_12,*interface_module_13,*interface_module_14,
	*interface_module_15,*interface_module_16;
#define GETSWAP16(target,value) {\
	unsigned int z;\
	z=(volatile unsigned int)value;\
	target=(short int)(((z>>8)&0x0000ff00)|(z>>24));\
	}
unsigned long real_time_clock_sec();
unsigned long real_time_clock_nsec();
/* kernal storage for conversions */
/*short int buffer[NUM_IO_CARDS*MUX_CHANNELS];*/
short int buffer[REFRESH_PERIOD*NUM_IO_CARDS*MUX_CHANNELS];
short int *signal_ptr;
int *time_ptr,time_buffer[REFRESH_PERIOD];
short int *current_signal;
int *current_time;

/* Simplified UNIMA functions */
void UimError()
{
}
int swapb(int value)
{
        char *pt, *pf;
        int i;

        pf = (char *)&value;
        pt = (char *) (&i);
        pt[0] = pf[3];
        pt[1] = pf[2];
        pt[2] = pf[1];
        pt[3] = pf[0];
        return(i);
}

void USetControl(unsigned int module, unsigned int control,
                 unsigned int level)
{
        unsigned int value=0;
        volatile unsigned int *p;

        if (module > NMODULE) UimError(0);
        else if (level > 1) UimError(21);
        else {
        switch(control) {
          case A : UUserConfigRegister[module].ctla = level;
                   break;
          case B : UUserConfigRegister[module].ctlb = level;
                   break;
          case C : UUserConfigRegister[module].ctlc = level;
                   break;
          default : UimError(22);
        }
        p = unima_bus_port+8*module+UN_USERCONFIG;
        value = *((volatile int *)(&UUserConfigRegister[module]));
        *p = swapb(value);
        }
}

void acquisition_interrupt_handler(timer_request_block)
struct trb *timer_request_block;
/*???Can this be prototyped ? */
/*******************************************************************************
LAST MODIFIED : 11 February 1993

DESCRIPTION :
The interrupt handler which will be called to interface to the UNIMA system
and will be called as the second level interrupt handler - see pages 68,70,72 of
'IBM AIX Version 3.1 RISC System/6000 as a Real-Time System', Document Number
  GG24-3633-0, March 1991, International Technical Support Center Austin, Texas.
==============================================================================*/
{
	/* counter */
	int channel;
/*	short int *signal;*/
	/* times */
	unsigned long convert_nsec,convert_sec;

	if (interrupt_data.on)
	{
		timer_request_block->flags=T_INCINTERVAL;
		timer_request_block->timeout.it_value.tv_sec=0;
		timer_request_block->timeout.it_value.tv_nsec=
			interrupt_data.interval_nsec;
		timer_request_block->timeout.it_interval.tv_sec=0;
		timer_request_block->timeout.it_interval.tv_nsec=
			interrupt_data.interval_nsec;
			/*???Does this make it keep submitting ?*/
		timer_request_block->func=acquisition_interrupt_handler;
		timer_request_block->ipri=INT_TIMLVL;
		tstart(timer_request_block);
		/* increment the experiment time */
		(interrupt_data.experiment_time)++;
		/* check for refresh and send signal if required */
		if (++(interrupt_data.refresh_count)>=REFRESH_PERIOD)
		{
/*???debug */
TRCHKL0T(HKWD_USER1+HK_2);
			interrupt_data.refresh_count=0;
			/* save the signals */
/*			xmemout(current_signal,(char *)buffer,REFRESH_PERIOD*NUM_IO_CARDS*
				MUX_CHANNELS*sizeof(short int),&cross_memory_descriptor);
			signal_ptr=buffer;
			current_signal += REFRESH_PERIOD*NUM_IO_CARDS*MUX_CHANNELS;*/
			/* save the times */
/*			xmemout(current_time,(char *)time_buffer,REFRESH_PERIOD*sizeof(int),
				&cross_memory_descriptor);
			time_ptr=time_buffer;
			current_time += REFRESH_PERIOD;*/
			/* send refresh signal back to the calling process */
			pidsig(interrupt_data.process_id,SIGUSR1);
		}
		/* map to the micro-channel bus */
		unima_bus_location=(unsigned int *)BUSIO_ATT(UNIMA_BUS_ID,0);
		unima_bus_port=(unsigned int *)((ulong)unima_bus_location+
			(ulong)interrupt_data.unima_io_offset);
		unima_base_io_registers=(unsigned int *)(unima_bus_port+8*1+UN05_CHANNEL0);
/*		interface_module_1=(unsigned int *)(unima_bus_port+8*1+UN05_CHANNEL0);
		interface_module_2=interface_module_1+8;
		interface_module_3=interface_module_2+8;
		interface_module_4=interface_module_3+8;
		interface_module_5=interface_module_4+8;
		interface_module_6=interface_module_5+8;
		interface_module_7=interface_module_6+8;
		interface_module_8=interface_module_7+8;
		interface_module_9=interface_module_8+8;
		interface_module_10=interface_module_9+8;
		interface_module_11=interface_module_10+8;
		interface_module_12=interface_module_11+8;
		interface_module_13=interface_module_12+8;
		interface_module_14=interface_module_13+8;
		interface_module_15=interface_module_14+8;
		interface_module_16=interface_module_15+8;*/
		/*???triggering autostart ? */
		/* do A/D conversions */
		/* for each channel */
		/*??? some work to do on making sure get readings at right times, because
			actually reading back last conversion and triggering next */
		signal_ptr=buffer;
/*???debug */
TRCHKL0T(HKWD_USER1+HK_0);
/*		for (channel=MUX_CHANNELS;channel>0;channel--)*/
		for (channel=MUX_CHANNELS/2;channel>0;channel--)
		{
			/* send a multi-cast command to start an A/D conversion */
			USetControl(1,A,1);
			USetControl(1,A,0);
			/* wait for the conversion to complete */
			convert_sec=real_time_clock_sec();
			convert_nsec=real_time_clock_nsec();
			while ((real_time_clock_sec()-convert_sec)*1000000000+
				real_time_clock_nsec()-convert_nsec<10000);
			/* send a multi-cast command to start another A/D conversion */
			USetControl(1,A,1);
			USetControl(1,A,0);
			/* wait for the conversion to complete */
			convert_sec=real_time_clock_sec();
			convert_nsec=real_time_clock_nsec();
			while ((real_time_clock_sec()-convert_sec)*1000000000+
				real_time_clock_nsec()-convert_nsec<10000);
			/* read back the results */
			read_32_channels(unima_base_io_registers,signal_ptr);
			signal_ptr += 2*NUM_IO_CARDS;
			/* read the last conversion back and trigger the next conversion */
/*			GETSWAP16(*(signal_ptr++),*(interface_module_1));
			GETSWAP16(*(signal_ptr++),*(interface_module_2));
			GETSWAP16(*(signal_ptr++),*(interface_module_3));
			GETSWAP16(*(signal_ptr++),*(interface_module_4));
			GETSWAP16(*(signal_ptr++),*(interface_module_5));
			GETSWAP16(*(signal_ptr++),*(interface_module_6));
			GETSWAP16(*(signal_ptr++),*(interface_module_7));
			GETSWAP16(*(signal_ptr++),*(interface_module_8));
			GETSWAP16(*(signal_ptr++),*(interface_module_9));
			GETSWAP16(*(signal_ptr++),*(interface_module_10));
			GETSWAP16(*(signal_ptr++),*(interface_module_11));
			GETSWAP16(*(signal_ptr++),*(interface_module_12));
			GETSWAP16(*(signal_ptr++),*(interface_module_13));
			GETSWAP16(*(signal_ptr++),*(interface_module_14));
			GETSWAP16(*(signal_ptr++),*(interface_module_15));
			GETSWAP16(*(signal_ptr++),*(interface_module_16));*/
/*			*(signal_ptr++)= *(interface_module_1);
			*(signal_ptr++)= *(interface_module_2);
			*(signal_ptr++)= *(interface_module_3);
			*(signal_ptr++)= *(interface_module_4);
			*(signal_ptr++)= *(interface_module_5);
			*(signal_ptr++)= *(interface_module_6);
			*(signal_ptr++)= *(interface_module_7);
			*(signal_ptr++)= *(interface_module_8);
			*(signal_ptr++)= *(interface_module_9);
			*(signal_ptr++)= *(interface_module_10);
			*(signal_ptr++)= *(interface_module_11);
			*(signal_ptr++)= *(interface_module_12);
			*(signal_ptr++)= *(interface_module_13);
			*(signal_ptr++)= *(interface_module_14);
			*(signal_ptr++)= *(interface_module_15);
			*(signal_ptr++)= *(interface_module_16);*/
/*			while ((real_time_clock_sec()-convert_sec)*1000000000+
				real_time_clock_nsec()-convert_nsec<UNIMA_CONVERSION_NSEC);*/
		}
/*???debug */
TRCHKL0T(HKWD_USER1+HK_1);
		/* save the signals */
		xmemout(buffer,(char *)current_signal,NUM_IO_CARDS*MUX_CHANNELS*
			sizeof(short int),&cross_memory_descriptor);
		/* save the time */
			/*??? should the actual conversion time be saved ? */
		xmemout(&(interrupt_data.experiment_time),(char *)current_time,
			sizeof(interrupt_data.experiment_time),&cross_memory_descriptor);
/*		*(time_ptr++)=interrupt_data.experiment_time;*/
		/*???writing experiment time back ? */
		/* buffer cycling */
		interrupt_data.sample_number--;
		if (interrupt_data.sample_number>0)
		{
			current_signal += NUM_IO_CARDS*MUX_CHANNELS;
			current_time++;
		}
		else
		{
			current_signal=interrupt_data.signals;
			current_time=interrupt_data.times;
			interrupt_data.sample_number=interrupt_data.number_of_samples;
		}
	}
	else
	{
/*???debug */
TRCHKL0T(HKWD_USER1+HK_3);
		/* stop the timer */
		tstop(timer_request_block);
		/* free the timer request block */
		tfree(timer_request_block);
		/* copy the data back to the user */
		xmemout((char *)&interrupt_data,(char *)(timer_request_block->t_union.addr),
			sizeof(interrupt_data),&cross_memory_descriptor);
		/* detach from user space */
		xmdetach(&cross_memory_descriptor);
		/* send tidy up signal back to the calling process */
		pidsig(interrupt_data.process_id,SIGUSR2);
	}
} /* acquisition_interrupt_handler */

int configure_acquisition(int command,struct uio *uio)
/*******************************************************************************
LAST MODIFIED : 17 July 1992

DESCRIPTION :
This function is called when the kernel extension is loaded or unloaded.
==============================================================================*/
{
	int return_code;

	if (command==CFG_INIT)
	/* configure the kernel extension */
	{
		/* pin the kernel extension into memory */
		if (pincode(configure_acquisition)!=0)
		{
			return_code=errno;
		}
		else
		{
			/* pin the interrupt handler into memory */
			if (pincode(acquisition_interrupt_handler)!=0)
			{
				return_code=errno;
			}
			else
			{
				/* pin the interrupt data into memory */
				if ((pin(&interrupt_data,sizeof(interrupt_data))==0)&&
/*					(pin(buffer,NUM_IO_CARDS*MUX_CHANNELS*sizeof(short int))==0)&&*/
					(pin(buffer,REFRESH_PERIOD*NUM_IO_CARDS*MUX_CHANNELS*sizeof(short int))==0)&&
					(pin(time_buffer,REFRESH_PERIOD*sizeof(int))==0)&&
					(pin(&unima_bus_port,sizeof(unsigned int *))==0)&&
					(pin(&unima_bus_location,sizeof(unsigned int *))==0)&&
					(pin(&current_time,sizeof(int *))==0)&&
					(pin(&time_ptr,sizeof(int *))==0)&&
					(pin(&signal_ptr,sizeof(short int *))==0)&&
					(pin(&current_signal,sizeof(short int *))==0)&&
					(pin(&interface_module_1,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_2,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_3,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_4,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_5,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_6,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_7,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_8,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_9,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_10,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_11,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_12,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_13,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_14,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_15,sizeof(unsigned int *))==0)&&
					(pin(&interface_module_16,sizeof(unsigned int *))==0)&&
					(pin(&cross_memory_descriptor,sizeof(cross_memory_descriptor))==0))
				{
					return_code=0;
				}
				else
				{
					return_code=errno;
				}
			}
		}
	}
	else
	/* unconfigure the kernel extension */
	{
		/* unpin the interrupt data into memory */
		unpin(&interrupt_data,sizeof(interrupt_data));
/*		unpin(buffer,NUM_IO_CARDS*MUX_CHANNELS*sizeof(short int));*/
		unpin(buffer,REFRESH_PERIOD*NUM_IO_CARDS*MUX_CHANNELS*sizeof(short int));
		unpin(time_buffer,REFRESH_PERIOD*sizeof(int));
		unpin(&unima_bus_port,sizeof(unsigned int *));
		unpin(&unima_bus_location,sizeof(unsigned int *));
		unpin(&time_ptr,sizeof(int *));
		unpin(&current_time,sizeof(int *));
		unpin(&signal_ptr,sizeof(short int *));
		unpin(&current_signal,sizeof(short int *));
		unpin(&interface_module_1,sizeof(unsigned int *));
		unpin(&interface_module_2,sizeof(unsigned int *));
		unpin(&interface_module_3,sizeof(unsigned int *));
		unpin(&interface_module_4,sizeof(unsigned int *));
		unpin(&interface_module_5,sizeof(unsigned int *));
		unpin(&interface_module_6,sizeof(unsigned int *));
		unpin(&interface_module_7,sizeof(unsigned int *));
		unpin(&interface_module_8,sizeof(unsigned int *));
		unpin(&interface_module_9,sizeof(unsigned int *));
		unpin(&interface_module_10,sizeof(unsigned int *));
		unpin(&interface_module_11,sizeof(unsigned int *));
		unpin(&interface_module_12,sizeof(unsigned int *));
		unpin(&interface_module_13,sizeof(unsigned int *));
		unpin(&interface_module_14,sizeof(unsigned int *));
		unpin(&interface_module_15,sizeof(unsigned int *));
		unpin(&interface_module_16,sizeof(unsigned int *));
		unpin(&cross_memory_descriptor,sizeof(cross_memory_descriptor));
		/* unpin the kernel extension from memory */
		unpincode(configure_acquisition);
		/* unpin the interrupt handler from memory */
		unpincode(acquisition_interrupt_handler);
		return_code=0;
	}

	return (return_code);
} /* configure_acquisition */

int start_acquisition_interrupt(
	struct Acquisition_interrupt_data *acquisition_interrupt_data)
/*???Is this the correct return type ? */
/*******************************************************************************
LAST MODIFIED : 11 February 1993

DESCRIPTION :
This function is called to set the interrupt and start the interval timer.
==============================================================================*/
{
	int return_code;
	struct trb *timer_request_block;
int i;

	/* lock the acquistion interrupt data in real memory */
	if ((0==pinu((caddr_t)acquisition_interrupt_data,
		sizeof(struct Acquisition_interrupt_data),UIO_USERSPACE))&&
		(0==pinu((caddr_t)(acquisition_interrupt_data->signals),
		(acquisition_interrupt_data->number_of_samples)*NUM_IO_CARDS*MUX_CHANNELS*
		sizeof(short int),UIO_USERSPACE))&&
		(0==pinu((caddr_t)(acquisition_interrupt_data->times),
		(acquisition_interrupt_data->number_of_samples)*sizeof(unsigned long),
		UIO_USERSPACE)))
	{
		cross_memory_descriptor.aspace_id=XMEM_INVAL;
		if ((XMEM_SUCC==xmattach(acquisition_interrupt_data,
			sizeof(struct Acquisition_interrupt_data),&cross_memory_descriptor,
			USER_ADSPACE))&&(XMEM_SUCC==xmemin((char *)acquisition_interrupt_data,
			(char *)&interrupt_data,sizeof(struct Acquisition_interrupt_data),
			&cross_memory_descriptor)))
		{
			for (i=0;i<17;i++)
			{
				UUserConfigRegister[i]=interrupt_data.unima_interface_registers[i];
			}
			current_signal=interrupt_data.signals;
			current_time=interrupt_data.times;
			signal_ptr=buffer;
			time_ptr=time_buffer;
			if (timer_request_block=talloc())
			{
				/* initialize the timer request block */
				timer_request_block->flags=T_INCINTERVAL;
					/*???Can one of the flags make it keep submitting ?*/
				timer_request_block->timeout.it_value.tv_sec=0;
				timer_request_block->timeout.it_value.tv_nsec=
					interrupt_data.interval_nsec;
				timer_request_block->timeout.it_interval.tv_sec=0;
				timer_request_block->timeout.it_interval.tv_nsec=
					interrupt_data.interval_nsec;
					/*???Does this make it keep submitting ?*/
				timer_request_block->func=acquisition_interrupt_handler;
				timer_request_block->ipri=INT_TIMLVL;
					/*???May need to increase this */
				timer_request_block->t_union.addr=(caddr_t)acquisition_interrupt_data;
				interrupt_data.on=1;
				xmemout(&(interrupt_data.on),(char *)&(acquisition_interrupt_data->on),
					sizeof(interrupt_data.on),&cross_memory_descriptor);
				tstart(timer_request_block);
				return_code=0;
			}
			else
			{
				return_code=errno;
			}
		}
		else
		{
			return_code= -1;
		}
	}
	else
	{
		return_code=errno;
	}

	return (return_code);
} /* start_acquisition_interrupt */

int stop_acquisition_interrupt(acquisition_interrupt_data)
/*???Is this the correct return type ? */
struct Acquisition_interrupt_data *acquisition_interrupt_data;
/*???Can this be prototyped ? */
/*******************************************************************************
LAST MODIFIED : 18 July 1992

DESCRIPTION :
This function is called to clear the interrupt and stop the interval timer.
==============================================================================*/
{
	int old_interrupt_priority,return_code;

	/* lock out interrupts.  See note on page 77 of red book as to why 1 is used
		instead of 0 */
	old_interrupt_priority=i_disable(1);
	/* change the on flag to off */
	interrupt_data.on=0;
/*???unpin memory ? */
	/* unlock interrupts */
	i_enable(old_interrupt_priority);
	return_code=0;

	return (return_code);
} /* stop_acquisition_interrupt */
