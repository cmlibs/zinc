/*******************************************************************************
FILE : time.h

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Defines the gettimeofday and relevant structure for UNIX and WIN32_SYSTEM
==============================================================================*/
#if !defined (GENERAL_TIME_H) /* Distinguish general/time.h and time/time.h */
#define GENERAL_TIME_H

#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
#include <sys/time.h>
#include <sys/times.h>
#elif defined (WIN32_SYSTEM) /* switch (OPERATING_SYSTEM) */
#include <windows.h>
typedef long time_t;
int gettimeofday(struct timeval *time, void *timezone);
typedef long clock_t;
struct tms 
{
	/* The times function in cmgui is just used to get a timestamp at
		the moment, if more than this is required it will need to be 
		implemented in the c function as well as added to this structure */
	int dummy;
};
clock_t times(struct tms *buffer);
#endif /* switch (OPERATING_SYSTEM) */

#endif /* !defined (GENERAL_TIME_H) */
