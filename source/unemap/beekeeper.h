/*******************************************************************************
FILE : beekeeper.h

LAST MODIFIED : 29 July 1997

DESCRIPTION :
Functions for reading Beekeeper data files (EEG signals).  The format was
determined from BEEKEEPER_README and B2A_SOURCE that were provided by Peng.
==============================================================================*/
#if !defined (BEEKEEPER_H)
#define BEEKEEPER_H

/*
Global functions
----------------
*/
int read_beekeeper_eeg_file(char *file_name,void *rig_pointer);
/*******************************************************************************
LAST MODIFIED : 29 July 1997

DESCRIPTION :
Reads the signal data from a Beekeeper EEG file and creates a rig with a default
configuration (electrode information is not available).
==============================================================================*/
#endif /* !defined (BEEKEEPER_H) */
