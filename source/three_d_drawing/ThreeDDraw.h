/*******************************************************************************
FILE : ThreeDDraw.h

LAST MODIFIED : 25 July 1998

DESCRIPTION :
Public header file for the 3-D drawing widget.
==============================================================================*/
#if !defined (_ThreeDDraw_h)
#define _ThreeDDraw_h

#include "three_d_drawing/X3d.h"

typedef enum
/*******************************************************************************
LAST MODIFIED : 21 April 1994

DESCRIPTION :
???DB.  Analogous for PEXlib ?
==============================================================================*/
{
	X3dSINGLE_BUFFERING,
	X3dDOUBLE_BUFFERING
} X3dBufferingMode;

typedef enum
/*******************************************************************************
LAST MODIFIED : 21 April 1994

DESCRIPTION :
???DB.  Analogous for PEXlib ?
==============================================================================*/
{
	X3dCOLOUR_INDEX_MODE,
	X3dCOLOUR_RGB_MODE
} X3dBufferColourMode;

/*******************************************************************************
Resources :

Name               Class              RepType             Default Value
----               -----              -------             -------------
bufferColourMode   BufferColourMode   X3dBufferColourMode X3dCOLOUR_INDEX_MODE
bufferingMode      BufferingMode      X3dBufferingMode    X3dSINGLE_BUFFERING
exposeCallback     ExposeCallback     XtPointer           NULL
initializeCallback InitializeCallback XtPointer           NULL
inputCallback      InputCallback      XtPointer           NULL
resizeCallback     ResizeCallback     XtPointer           NULL
OpenGL
renderingContext   RenderingContext   GLXContext          NULL
PEX (IBM)
renderingContext   RenderingContext   PEXRenderer         NULL
==============================================================================*/
#define X3dNbufferColourMode "bufferColourMode"
#define X3dCBufferColourMode "BufferColourMode"
#define X3dRBufferColourMode "X3dBufferColourMode"
#define X3dNbufferingMode "bufferingMode"
#define X3dCBufferingMode "BufferingMode"
#define X3dRBufferingMode "X3dBufferingMode"
#define X3dNexposeCallback "exposeCallback"
#define X3dCExposeCallback "ExposeCallback"
#define X3dNinitializeCallback "initializeCallback"
#define X3dCInitializeCallback "InitializeCallback"
#define X3dNinputCallback "inputCallback"
#define X3dCInputCallback "InputCallback"
#define X3dNresizeCallback "resizeCallback"
#define X3dCResizeCallback "ResizeCallback"
#if defined (OPENGL_API) || defined (PEXLIB_API)
#define X3dNrenderingContext "renderingContext"
#define X3dCRenderingContext "RenderingContext"
#if defined (OPENGL_API)
#define X3dRRenderingContext "GLXContext"
#endif
#if defined (PEXLIB_API)
#define X3dRRenderingContext "PEXRenderer"
#endif
#endif

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

void X3dThreeDDrawingMakeCurrent(Widget widget);
/*******************************************************************************
LAST MODIFIED : 14 February 1995

DESCRIPTION :
Makes the widget the current one being drawn to.
==============================================================================*/

void X3dThreeDDrawingRemakeCurrent(void);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Change the context to the last ThreeDDrawing that was made current
==============================================================================*/

void X3dThreeDDrawingSwapBuffers(void);
/*******************************************************************************
LAST MODIFIED : 14 February 1995

DESCRIPTION :
Swaps the buffers current widget.
==============================================================================*/

void X3dThreeDDrawingCleanUp(Display *display);
/*******************************************************************************
LAST MODIFIED : 17 November 1997

DESCRIPTION :
Routine for cleaning up any dynamic module variables created with the
Three_D_Drawing, eg. OpenGL shareable_context.
==============================================================================*/

#if defined (OPENGL_API)
int query_gl_extension(char *extName);
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Search for extName in the GL extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from the insight book on OpenGL Extensions
==============================================================================*/

int query_glx_extension(char *extName, Display *display, int screen);
/*******************************************************************************
LAST MODIFIED : 14 September 1998

DESCRIPTION :
Search for extName in the GLX extensions string.
==============================================================================*/
#endif /* defined (OPENGL_API) */
#endif /* !defined (_ThreeDDraw_h) */
