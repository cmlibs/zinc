/*******************************************************************************
FILE : neurosoft.h

LAST MODIFIED : 23 August 1997

DESCRIPTION :
Functions for reading output from Neurosoft's SCAN package.
==============================================================================*/
#if !defined (NEUROSOFT_H)
#define NEUROSOFT_H

/*
Global functions
----------------
*/
int read_neurosoft_row_points_file(char *file_name,void *rig_void);
/*******************************************************************************
LAST MODIFIED : 4 August 1997

DESCRIPTION :
Reads a ROWS=POINTS format signal file produced by Neurosoft's SCAN program.
Assumes the SAMPLING_FREQUENCY and the configuration (have created
neurosoft.cnfg) has been read.
==============================================================================*/
#endif /* !defined (NEUROSOFT_H) */
