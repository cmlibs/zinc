/*******************************************************************************
FILE : bard.h

LAST MODIFIED : 9 April 1994

DESCRIPTION :
Function prototypes for reading files from the Bard system.
==============================================================================*/
#if !defined (BARD_H)
#define BARD_H

/*
Global functions
----------------
*/
int read_bard_electrode_file(char *file_name,void *rig_pointer);
/*******************************************************************************
LAST MODIFIED : 28 December 1992

DESCRIPTION :
Reads in a rig configuration from a Bard electrode file.
==============================================================================*/

int read_bard_signal_file(char *file_name,void *rig);
/*******************************************************************************
LAST MODIFIED : 9 April 1994

DESCRIPTION :
Reads in a Bard signal file (window.dat) and creates a buffer for it as part of
the <rig>.  See this files header for information on window.dat
==============================================================================*/
#endif
