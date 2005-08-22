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
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
