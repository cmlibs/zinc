/*******************************************************************************
FILE : graphics_library.h

LAST MODIFIED : 14 November 1996

DESCRIPTION :
Functions and structures for interfacing with the graphics library.
==============================================================================*/
#if !defined (GRAPHICS_LIBRARY_H)
#define GRAPHICS_LIBRARY_H

#if defined (GL_API)
#if defined (MOTIF)
/* needed because X and GL use some of the same names and if X isn't included
	before GL there are errors */
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF) */
#include <gl/gl.h>
#endif
#if defined (OPENGL_API)
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#if defined (DECPHIGS_API)
#include <phigs.h>
#endif
#if defined (graPHIGS_API)
#include "phigs.h"
#endif
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
/*???DB.  I don't think that this should be here */
typedef float gtMatrix[4][4];

/*
Global variables
----------------
*/
extern int graphics_library_open;

/*
Global functions
----------------
*/
int open_graphics_library(void);
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Function to open the PHIGS graphics library.
==============================================================================*/

int close_graphics_library(void);
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Function to close the PHIGS graphics library.
==============================================================================*/

int initialize_graphics_library(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 November 1996

DESCRIPTION :
Sets up the default light, material and light model for the graphics library.
==============================================================================*/

#if defined (OPENGL_API)
void wrapperReadMatrix(GLenum matrixName,gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperLoadCurrentMatrix(gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperMultiplyCurrentMatrix(gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperWindow(float left,float right,float bottom,float top,float near,
	float far);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPolarview(float A,float B,float C,float D);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPerspective(float A,float B,float C,float D);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperOrtho(float a,float b,float c,float d,float e,float f);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperOrtho2D(float a,float b,float c,float d);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPrintText(char *theText);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperInitText(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 November 1996

DESCRIPTION :
==============================================================================*/
#endif

#if defined (OLD_CODE)
/*******************************************************************************
FILE : graphics_library.h

LAST MODIFIED : 4 December 1994

DESCRIPTION :
Functions and structures for interfacing with the graphics library.

Includes a prototype GL widget for AIX.  Taken from the files
	R2/GL/examples/utilities/inc/Glib.h
	R2/GL/examples/utilities/inc/GlibP.h
	R2/GL/examples/utilities/gutil/Glib.c
With resources:
	Name              Class        RepType   Default Value
	----              -----        -------   -------------
	exposeCallback    Callback     Pointer   NULL
	resizeCallback    Callback     Pointer   NULL
	gconfigCallback   Callback     Pointer   NULL
	initClearColor    Colorindex   int       0
==============================================================================*/
#include <stddef.h>

#if defined (GL_API) || defined (OPENGL_API)
/* include things needed to support GL or OpenGL */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#if defined (IBM) || defined (OPENGL_API)
#include <X11/Xutil.h>
#else
#include <X11/Xirisw/GlxMDraw.h>
#endif
#endif

#if defined (GL_API)
#include <gl/gl.h>
#endif

#if defined (OPENGL_API)
#include <GL/gl.h>
#endif

#if defined (DECPHIGS_API)
#include <Xm/DrawingA.h>
#include <phigs.h>
#endif

#if defined (graPHIGS_API)
#include <afmnc.h>
#include "phigs.h"
#endif

#if defined (GL_API) && defined (IBM)
#define XglNexposeCallback "exposeCallback"
#define XglNgconfigCallback "gconfigCallback"
#define XglNresizeCallback "resizeCallback"
#define XglNinitClearColor "initClearColor"
/* define the callback structure */
typedef struct
{
	int reason;
	XEvent *event;
	Region region;
} GlibCallbackStruct;
/* declare specific GlibWidget class and instance datatypes */
typedef struct _GlibClassRec*  GlibWidgetClass;
typedef struct _GlibRec*  GlibWidget;
#endif

/*
Global/Public variables
-----------------------
*/
#if defined (GL_API) && defined (IBM)
/* declare the class constant */
extern WidgetClass glibWidgetClass;
#endif

#if defined (GL_API) && defined (IBM)
int GlWinsetWidget(Widget widget);
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Set the window to which rendering will appear.
==============================================================================*/

Widget GlXCreateMDraw(Widget parent,char *name,ArgList arglist,
	Cardinal argcount);
/*******************************************************************************
LAST MODIFIED : 24 March 1993

DESCRIPTION :
Set the window to which rendering will appear.
==============================================================================*/
#endif
#endif
#endif
