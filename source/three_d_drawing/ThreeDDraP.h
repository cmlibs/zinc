/*******************************************************************************
FILE : ThreeDDraP.h

LAST MODIFIED : 03 May 2004

DESCRIPTION :
Private header file for the 3-D drawing widget.
==============================================================================*/
#if !defined (_ThreeDDraP_h)
#define _ThreeDDraP_h

/* private header files for superclasses */
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
/* public header file for 3-D drawing widget */
#include "ThreeDDraw.h"
/* header for 3-D graphics application programming interface */

/*
Class types
-----------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* a structure can't be empty */
	int dummy;
} ThreeDDrawingClassPart;

typedef struct _ThreeDDrawingClassRec
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* superclass parts */
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	/* 3-D drawing widget part */
	ThreeDDrawingClassPart threeDDrawing_class;
} ThreeDDrawingClassRec;

/*
Class record
------------
*/
extern ThreeDDrawingClassRec threeDDrawingClassRec;

/*
Instance types
--------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 03 May 2004

DESCRIPTION :
==============================================================================*/
{
	/* specify if the buffer is present for the widget */
	Boolean present;
	Colormap colour_map;
	Visual *visual;
	XtCallbackList expose_callback;
	XtCallbackList initialize_callback;
} X3dOutputBuffer;

typedef struct
/*******************************************************************************
LAST MODIFIED : 27 April 1994

DESCRIPTION :
???DB.  At present, only implementing the normal buffer
==============================================================================*/
{
	/* resources */
	XtCallbackList input_callback;
	XtCallbackList resize_callback;
	/* private state */
	X3dOutputBuffer normal_buffer;
} ThreeDDrawingPart;

typedef struct _ThreeDDrawingRec
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* superclass parts */
	CorePart core;
	CompositePart composite;
	/* 3-D drawing widget part */
	ThreeDDrawingPart three_d_drawing;
} ThreeDDrawingRec;

#endif
