/*******************************************************************************
FILE : time.c

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Defines the gettimeofday and relevant structure for UNIX and WIN32_SYSTEM
==============================================================================*/
#if defined (WIN32_SYSTEM)
#include <windows.h>
#include "general/time.h"

int gettimeofday(struct timeval *time, void *timezone)
{
  unsigned int gettime;
  
  gettime = timeGetTime();
  time->tv_sec = gettime / 1000;
  time->tv_usec = (gettime - (time->tv_sec*1000)) * 1000;
  return 0;
}
#else /* defined (WIN32_SYSTEM) */
/* Declare something so that this file will compile */
int dummy;
#endif /* defined (WIN32_SYSTEM) */
