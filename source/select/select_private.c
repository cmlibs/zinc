/*******************************************************************************
FILE : select_private.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Variables shared by, but private to select_*.c files.
==============================================================================*/
/* have define to avoid uid variable being declared */
#define NO_SELECT_UIDH 1
#include "select/select_private.h"
#undef NO_SELECT_UIDH

/*
Global variables
----------------
*/
#if defined (MOTIF)
int select_hierarchy_open=0;
MrmHierarchy select_hierarchy;
#endif /* defined (MOTIF) */
