/*******************************************************************************
FILE : acquisition.h

LAST MODIFIED : 26 March 1997

DESCRIPTION :
==============================================================================*/
#if !defined (ACQUISITION_H)
#define ACQUISITION_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

/*
Global constants
----------------
*/
#define MAXIMUM_NUMBER_OF_SAMPLES 16000
/*#define MAXIMUM_NUMBER_OF_SAMPLES 8000*/
/*#define MAXIMUM_NUMBER_OF_SAMPLES 4000*/

/*
Global functions
----------------
*/
#if defined (MOTIF)
void acquire_data(Widget widget,XtPointer client_data,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
***Serge***
see struct Signal_buffer in rig.h
When the user specified acquisition interval is in the buffer (determined by
looking at the current time) the UNIMA system will have to swap to writing to
the other buffer.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void start_stop_experiment(Widget widget,XtPointer acquisition_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 2 March 1993

DESCRIPTION :
***Serge***
-start
Allocate storage for the signal_buffer.
Initialize the UNIMA system.
Start the UNIMA system updating the time since the start of the experiment.
-stop
Stop the UNIMA system.
Deallocate the storage for the signal_buffer.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void start_stop_monitoring(Widget widget,XtPointer acquisition_window,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 1 March 1993

DESCRIPTION :
***Serge***
-start
see struct Signal_buffer in rig.h
Start the UNIMA system writing times and measurements to the buffer and
updating the first time and last time pointers.
-stop
see struct Signal_buffer in rig.h
Stop the UNIMA system writing times and measurements to the buffer and
updating the first time and last time pointers.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void calibrate_channels(Widget widget,XtPointer client_data,
	XtPointer call_data);
/******************************************************************************
LAST MODIFIED : 9 September 1992

DESCRIPTION :
=============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void set_calibrate_interface_module(Widget widget,XtPointer module,
	XtPointer call_data);
/******************************************************************************
LAST MODIFIED : 24 September 1992

DESCRIPTION :
=============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (ACQUISITION_H) */
