/*******************************************************************************
FILE : select_private.c

LAST MODIFIED : 14 May 1997

DESCRIPTION :
Variables shared by, but private to select_*.c files.
==============================================================================*/
#include "select/select_private.h"

/*
Global variables
----------------
*/
#if defined (MOTIF)
int select_hierarchy_open=0;
MrmHierarchy select_hierarchy;
#endif /* defined (MOTIF) */
