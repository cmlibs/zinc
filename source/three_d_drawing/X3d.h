/*******************************************************************************
FILE : X3d.h

LAST MODIFIED : 12 February 1995

DESCRIPTION :
Public header file for the 3-D drawing widget library.  Modelled on Xm.h
==============================================================================*/
#if !defined (_X3d_h)
#define _X3d_h

enum
/*******************************************************************************
LAST MODIFIED : 11 February 1995

DESCRIPTION :
Callback reasons
==============================================================================*/
{
	X3dCR_EXPOSE,
	X3dCR_INITIALIZE,
	X3dCR_INPUT,
	X3dCR_RESIZE
};

/*
Callback structures
-------------------
*/
typedef struct
{
	int reason;
	XEvent *event;
} X3dAnyCallbackStruct;

typedef struct
{
	int reason;
	XEvent *event;
	Window window;
} X3dThreeDDrawCallbackStruct;
#endif
