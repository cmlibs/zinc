/*******************************************************************************
FILE : compare.h

LAST MODIFIED : 15 March 1999

DESCRIPTION :
Prototypes of functions for comparing data types (analogous to strcmp).
Functions used for managers and lists.
==============================================================================*/
#if !defined (COMPARE_H)

/*
Global functions
----------------
*/
int compare_int(int int_1,int int_2);
/*******************************************************************************
LAST MODIFIED : 27 April 1995

DESCRIPTION :
Returns -1 if int_1 < int_2, 0 if int_1 = int_2 and 1 if int_1 > int_2.
==============================================================================*/

int compare_pointer(void *pointer_1,void *pointer_2);
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Returns -1 if pointer_1 < pointer_2, 0 if pointer_1 = pointer_2 
and 1 if pointer_1 > pointer_2.
==============================================================================*/
#endif
