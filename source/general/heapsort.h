/*******************************************************************************
FILE : heapsort.h

LAST MODIFIED : 4 July 1992

DESCRIPTION :
Created because qsort didn't seem to be working properly in TurboC.
==============================================================================*/
#include <stddef.h>

void heapsort(void *base,size_t count,size_t size,
	int (*compare)(void *,void *));
/*******************************************************************************
LAST MODIFIED : 5 October 1991

DESCRIPTION :
An implementation of heapsort as described in "Numerical Recipes.  The Art of
Scientific Computing" by W.H. Press, B.P. Flannery, S.A. Teukolsky and W.T.
Vetterling.
==============================================================================*/
