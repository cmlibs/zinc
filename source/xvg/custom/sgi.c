/***********************************************************************
*
*  Name:          Dummy declarations for SGI machines
*
*  Author:        Paul Charette
*
*  Last Modified: 12 Oct 1994
*              itrunc()
*              nearest()
*              second()
*              strcmp_2()
*
***********************************************************************/
#include <stdio.h>
#include <string.h>
#include "sgi.h"

/* duplicate declaration for strcmp because the SGI vesion doesn't like */
/* a NULL argument                                                      */
int XvgStrcmp(char *s1, char *s2)
{
  if ((s1 == NULL) && (s2 == NULL))
    return(0);
  
  if ((s1 == NULL) || (s2 == NULL))
    return(1);
  
  return(strcmp(s1, s2));
}

double second(void)
{
  return(XvgGetTime());
}

int itrunc(double v)
{
  return((int) trunc(v));
}
