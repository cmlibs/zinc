/*******************************************************************************
FILE : pin_memory.h

LAST MODIFIED : 30 March 1993

DESCRIPTION :
AIX kernel extension to allow a user program to pin and unpin memory.
==============================================================================*/
#if !defined (PIN_MEMORY_H)
#define PIN_MEMORY_H

#include <sys/types.h>

/*
Global functions
----------------
*/
int pin_memory(caddr_t start_address,int number_of_bytes);
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function pins the specified <number_of_bytes> from the <start_address>
into physical memory.  It returns 0 if successful.
==============================================================================*/

int unpin_memory(caddr_t start_address,int number_of_bytes);
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function unpins the specified <number_of_bytes> from the <start_address>
from physical memory.  It returns 0 if successful.
==============================================================================*/

int pin_function(int(*function)());
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function pins the specified <function> into physical memory.  It returns 0
if successful.
==============================================================================*/

int unpin_function(int(*function)());
/*******************************************************************************
LAST MODIFIED : 30 March 1993

DESCRIPTION :
This function unpins the specified <function> from physical memory.  It returns
0 if successful.
==============================================================================*/
#endif
