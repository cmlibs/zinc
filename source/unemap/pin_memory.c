/*******************************************************************************
FILE : pin_memory.c

LAST MODIFIED : 30 March 1993

DESCRIPTION :
AIX kernel extension to allow a user program to pin and unpin memory.  Based on
the kernel extension in section A.9.3 (p.219) of
'IBM AIX Version 3.1 RISC System/6000 as a Real-Time System', Document Number
  GG24-3633-0, March 1991, International Technical Support Center Austin, Texas.

COMPILATION :
cc -c pin_memory.c -D_AIX -D_KERNEL -o pin_memory.o
ld pin_memory.o -o pin_memory -e configure_pin_memory -lsys -lcsys \
	-bI:/lib/kernex.exp -bE:./pin_memory.exp
Explanation of compiler flags (from p.59)
-e indicates that configure_pin_memory is the entry point of the object file.
  This entry point may be called by the sysconfig() subroutine for kernel object
  initialization or termination.
-bE: indicates that the pin_memory.exp is the export file.
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

#include "pin_memory.h"

/*
Module functions
----------------
*/
int configure_pin_memory(int command,struct uio *uio)
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
		if (pincode(configure_pin_memory)!=0)
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
		unpincode(configure_pin_memory);
		return_code=0;
	}

	return (return_code);
} /* configure_pin_memory */

/*
Global functions
----------------
*/
int pin_memory(caddr_t start_address,int number_of_bytes)
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function pins the specified <number_of_bytes> from the <start_address>
into physical memory.  It returns 0 if successful.
==============================================================================*/
{
	return (pinu(start_address,number_of_bytes,UIO_USERSPACE));
} /* pin_memory */

int unpin_memory(caddr_t start_address,int number_of_bytes)
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function unpins the specified <number_of_bytes> from the <start_address>
from physical memory.  It returns 0 if successful.
==============================================================================*/
{
	return (unpinu(start_address,number_of_bytes,UIO_USERSPACE));
} /* unpin_memory */

int pin_function(int(*function)())
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function pins the specified <function> into physical memory.  It returns 0
if successful.
==============================================================================*/
{
	return (pincode(function));
} /* pin_function */

int unpin_function(int(*function)())
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function unpins the specified <function> from physical memory.  It returns
0 if successful.
==============================================================================*/
{
	return (unpincode(function));
} /* unpin_function */
