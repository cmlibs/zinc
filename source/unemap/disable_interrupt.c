/*******************************************************************************
FILE : disable_interrupt.c

LAST MODIFIED : 30 March 1993

DESCRIPTION :
AIX kernel extension that provides system calls disable_interrupts and
enable_interrupts for disabling and re-enabling interrupts from a user program.
An adaptation of the kernel extension in section A.5.1 (p.152) of
'IBM AIX Version 3.1 RISC System/6000 as a Real-Time System', Document Number
  GG24-3633-0, March 1991, International Technical Support Center Austin, Texas.

COMPILATION :
cc disable_interrupt.c -c -D_AIX -D_KERNEL -DTRACE -o disable_interrupt.o
ld disable_interrupt.o -e configure_disable -o disable_interrupt -lsys -lcsys \
-bI:/lib/kernex.exp -bE:./disable_interrupt.exp
Explanation of compiler flags (from p.59)
-e indicates that acquisition_interrupt is the entry point of the object file.
  This entry point may be called by the sysconfig() subroutine for kernel object
  initialization or termination.
-bE: indicates that the acquisition_interrupt.exp is the export file.
-bI: specifies the import files.
==============================================================================*/
/* header file for error numbers */
#include <sys/errno.h>
/* header file for AIX types */
#include <sys/types.h>
/* defines CFG_INIT */
#include <sys/device.h>
/* user I/O services header file */
#include <sys/uio.h>

/*
Module functions
----------------
*/
int configure_disable(int command,struct uio *uio)
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function is called when the kernel extension is loaded or unloaded.
==============================================================================*/
{
	int return_code;

	if (command==CFG_INIT)
	/* configure the kernel extension */
	{
		/* pin the kernel extension into memory */
		if (pincode(configure_disable)!=0)
		{
			return_code=ENOMEM;
		}
		else
		{
			return_code=0;
		}
	}
	else
	/* unconfigure the kernel extension */
	{
		/* unpin the kernel extension from memory */
		unpincode(configure_disable);
		return_code=0;
	}

	return (return_code);
} /* configure_acquisition */

/*
Global functions
----------------
*/
int disable_interrupts(priority)
int priority;
/*???Can this be prototyped ? */
/*******************************************************************************
LAST MODIFIED : 30 July 1992

DESCRIPTION :
Set the interrupt priority to a more favoured value to disable interrupts.
This function returns the previous interrupt setting.
NB In the Red Book they had problems setting the interrupt priority to 0 and
recommend using 1 (disable all interrupts except the "early power-off warning").
==============================================================================*/
{
	return (i_disable(priority));
} /* disable_interrupts */

void enable_interrupts(old_priority)
int old_priority;
/*???Can this be prototyped ? */
/*******************************************************************************
LAST MODIFIED : 30 July 1992

DESCRIPTION :
Restores the interrupt priority to a less favoured value to re-enable
interrupts.
==============================================================================*/
{
	i_enable(old_priority);
} /* enable_interrupts */
