/*******************************************************************************
FILE : call_work_procedures.c

LAST MODIFIED : 4 March 2002

DESCRIPTION :
SAB. This module provides some private X functions so that we can call timer events
and work procedures when necessary.  Unfortunately Xt does not provide the
necessary hooks in its public interface.  These functions and structure
definitions are reimplemented here, so that if a particular X implementation
did not implement these structures the same, this code would not work.
This small amount of cheating enables us to wrestle main loop control off X.
The code is basically copied from the X Window System, Version 11, Releae 6.4
from the Open Group X Project Team 30 January 1998
==============================================================================*/
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <X11/Intrinsic.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

typedef struct _WorkProcRec
{
	XtWorkProc proc;
	XtPointer closure;
	struct _WorkProcRec *next;
	XtAppContext app;
} WorkProcRec;

typedef struct _TimerEventRec
{
	struct timeval        te_timer_value;
	struct _TimerEventRec *te_next;
	XtTimerCallbackProc   te_proc;
	XtAppContext          app;
	XtPointer             te_closure;
} TimerEventRec;

typedef struct _ProcessContextRec *ProcessContext;

typedef struct internalCallbackRec *InternalCallbackList;

typedef struct _XtAppStruct 
{
	XtAppContext next;          /* link to next app in process context */
	ProcessContext process;     /* back pointer to our process context */
	InternalCallbackList destroy_callbacks;
	Display **list;
	TimerEventRec *timerQueue;
	WorkProcRec *workQueue;
	/* That's all folks cause that is all we need */
} XtAppStruct;

Boolean XtCallWorkProc(XtAppContext app)
{
	register WorkProcRec *w = app->workQueue;
	Boolean delete;

	if (w == NULL) return FALSE;

	app->workQueue = w->next;

	delete = (*(w->proc)) (w->closure);

	if (delete)
	{
		XtFree((XtPointer)w);
	}
	else
	{
		w->next = app->workQueue;
		app->workQueue = w;
	}
	return TRUE;
}

int XtTimeForTimeout(XtAppContext app, long *sec, long *usec)
{
	  /* Assumes we have just called XtAppPending to get the 
		  structures set up */
	int return_code;

	if(app->timerQueue != NULL)
	{
		if(app->timerQueue->te_proc != 0)
		{
			*sec = app->timerQueue->te_timer_value.tv_sec;
			*usec = app->timerQueue->te_timer_value.tv_usec;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}

	return (return_code);
}
