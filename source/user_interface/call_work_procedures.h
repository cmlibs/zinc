/*******************************************************************************
FILE : call_work_procedures.h

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

Boolean XtCallWorkProc(XtAppContext app);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Exporting this private Xt function so that we can call it from user_interface.
==============================================================================*/

int XtTimeForTimeout(XtAppContext app, long *sec, long *usec);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Exporting this private Xt function so that we can call it from user_interface.
==============================================================================*/
