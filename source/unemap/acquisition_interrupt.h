/*******************************************************************************
FILE : acquisition_interrupt.h

LAST MODIFIED : 17 February 1993

DESCRIPTION :
==============================================================================*/

#include "UNIMA/uim.h"

struct Acquisition_interrupt_data
/*******************************************************************************
LAST MODIFIED : 17 February 1993

DESCRIPTION :
The structure which will be passed to the interrupt handler in the timer
request block.
==============================================================================*/
{
	char on;
	int experiment_time,*times;
	pid_t process_id;
	short int *signals;
	/* interrupt interval */
	unsigned long interval_nsec;
	unsigned short number_of_samples,refresh_count,sample_number;
	/* offset for UNIMA I/O registers */
	caddr_t unima_io_offset;
	/* contents of the UNIMA master controller register */
	UUserConfig unima_interface_registers[17];
}; /* Acquisition_interrupt_data */
