/*******************************************************************************
FILE : ThreeDDraw.h

LAST MODIFIED : 19 September 2002

DESCRIPTION :
Public header file for the 3-D drawing widget.
==============================================================================*/
#if !defined (_ThreeDDraw_h)
#define _ThreeDDraw_h

#include "three_d_drawing/X3d.h"

/*******************************************************************************
Resources :

Name               Class              RepType             Default Value
----               -----              -------             -------------
exposeCallback     ExposeCallback     XtPointer           NULL
initializeCallback InitializeCallback XtPointer           NULL
inputCallback      InputCallback      XtPointer           NULL
resizeCallback     ResizeCallback     XtPointer           NULL
==============================================================================*/

#define X3dNexposeCallback "exposeCallback"
#define X3dCExposeCallback "ExposeCallback"
#define X3dNinitializeCallback "initializeCallback"
#define X3dCInitializeCallback "InitializeCallback"
#define X3dNinputCallback "inputCallback"
#define X3dCInputCallback "InputCallback"
#define X3dNresizeCallback "resizeCallback"
#define X3dCResizeCallback "ResizeCallback"

/* class record constants */
extern WidgetClass threeDDrawingWidgetClass;
typedef struct _ThreeDDrawingClassRec *ThreeDDrawingWidgetClass;
typedef struct _ThreeDDrawingRec *ThreeDDrawingWidget;

/*
Global macros
-------------
*/
#if !defined IsThreeDDrawing
#define IsThreeDDrawing(w)  (XtIsSubclass(w,threeDDrawingWidgetClass))
#endif

/*
Global functions
----------------
*/
int X3dThreeDisInitialised(Widget widget);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns true if the X3dThreeD <widget> is initialised correctly.  This enables 
us to fail nicely if the Initialise routine was unable to complete properly, 
i.e. it couldn't create a valid rendering context.
==============================================================================*/

#endif /* !defined (_ThreeDDraw_h) */
