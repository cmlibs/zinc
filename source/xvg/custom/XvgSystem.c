/***********************************************************************
*
*  Name:          XvgSystem.c
*
*  Author:        Paul Charette
*
*  Last Modified:
*                 9 December 1997: 
*
*  Purpose:       System utility routines.
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"
#include <sys/time.h>
#ifdef SGI
#include <signal.h>
#else
#include <sys/signal.h>
#endif

#ifndef SGI
extern int    gettimeofday(struct timeval *Tp, struct timezone *Tzp);
#endif

static struct timeval tv0, tv1, tv2;
#ifndef SGI
static struct timezone tz0, tz1, tz2;
#endif

static void ActionHandler(int Signal)
{
  /* message */
  if (VerboseFlag)
    printf("XVG ActionHandler() : CTRL-C trapped\n");
  
  /* terminate all child processes and remove semaphores */
  if (ChildProcess == B_FALSE) {
    StopChildren();
    RemoveSemaphores();
  }

  /* stop the process if required */
  if (Abort || !InteractiveFlag || ChildProcess)
    exit(EXIT_FAILURE);

  /* flag the entry to this handler */
  Abort = B_TRUE;
}

void XvgTrapSignal(void)
{
#ifndef SGI
  struct sigaction sa;
  int rc;
#endif
  
#ifndef SGI
  sa.sa_handler = ActionHandler;
#ifdef SGI
  sa.sa_flags = SA_SIGINFO;
#endif
#endif

  /* CTRL-C */
#ifdef SGI
	if (SIG_ERR==signal(SIGINT,ActionHandler))
	{
    printf("XvgTrapSignal() : Failed to install ctrl-C handler\n");
    exit(EXIT_FAILURE);
	}
#else
  if ((rc = sigaction(SIGINT, (struct sigaction *) (&sa),
		      (struct sigaction *) NULL)) != 0) {
/*
#ifdef SGI
    gerror_(cbuf, CBUF_SIZE);
#endif
    printf("XvgTrapSignal() : Failed to install ctrl-C handler (%s)\n", cbuf);
*/
    printf("XvgTrapSignal() : Failed to install ctrl-C handler\n");
    exit(EXIT_FAILURE);
  }
#endif

  /* KILL */
#ifdef SGI
/*	if (SIG_ERR==signal(SIGKILL,ActionHandler))
	{
    printf("XvgTrapSignal() : Failed to install SIGKILL handler\n");
    exit(EXIT_FAILURE);
	}*/
#else
/*
  if ((rc = sigaction(SIGKILL, (struct sigaction *) (&sa),
		      (struct sigaction *) NULL)) != 0) {
#ifdef SGI
    gerror_(cbuf, CBUF_SIZE);
#endif
    printf("XvgTrapSignal() : Failed to install SIGKILL handler (%s)\n", cbuf);
    exit(EXIT_FAILURE);
  }
*/
#endif
}

double XvgStartTimer(void)
{
#ifdef SGI
  gettimeofday(&tv0,NULL);
#else
  gettimeofday(&tv0, &tz0);
#endif
  return(((double) tv0.tv_sec) + ((double) tv0.tv_usec)/1000000.0);
}

double XvgStopTimer(void)
{
#ifdef SGI
  gettimeofday(&tv1,NULL);
#else
  gettimeofday(&tv1, &tz1);
#endif
  return(((double) tv1.tv_sec) + ((double) tv1.tv_usec)/1000000.0);
}

double XvgGetTime(void)
{
#ifdef SGI
  gettimeofday(&tv2,NULL);
#else
  gettimeofday(&tv2, &tz2);
#endif
  return(((double) tv2.tv_sec) + ((double) tv2.tv_usec)/1000000.0);
}

/* Elapsed time computation */
char *ElapsedTime(char *s)
{
  static char buf[512];
  int minutes, hours, seconds;

  hours = (tv1.tv_sec - tv0.tv_sec)/(60*60);
  minutes = (tv1.tv_sec - tv0.tv_sec - hours*60*60)/60;
  seconds = tv1.tv_sec - tv0.tv_sec - hours*60*60 - minutes*60;
  sprintf(buf, "%s > Time (hh:mm:ss) is %02d:%02d:%02d",
	    Msg, hours, minutes, seconds);
  sprintf(Msg, "");
  if ((VerboseFlag) && (InteractiveFlag))
    printf("%s%s\n", s, buf);
  return(buf);
}



