/*******************************************************************************
FILE : cardiomapp.c

LAST MODIFIED : 23 August 1997

DESCRIPTION :
Functions for reading CardioMapp (ART) data files.  The format was given in
CARDIOMAPP_README (part of the CardioMapp developers support kit given to me by
Peng).
==============================================================================*/
#if !defined (CARDIOMAPP_H)
#define CARDIOMAPP_H

/*
Global functions
----------------
*/
int read_cardiomapp_file(char *file_name,void *rig_void);
/*******************************************************************************
LAST MODIFIED : 23 August 1997

DESCRIPTION :
Reads a .rdt signal file produced by ART's CardioMapp program.
==============================================================================*/
#endif /* !defined (CARDIOMAPP_H) */
