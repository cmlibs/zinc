/*******************************************************************************
FILE : ThreeDDraP.h

LAST MODIFIED : 15 February 1995

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
#if defined (OPENGL_API)
#include <GL/gl.h>
#include <GL/glx.h>
#endif
#if defined (GL_API)
#include <gl/gl.h>
#if defined (SGI)
#include <gl/glws.h>
#endif
#endif
#if defined (PEXLIB_API)
#if defined (IBM)
#include <X11/PEX5/PEXlib.h>
#endif
#if defined (VAX)
#include <PEXlib.h>
#endif
#endif

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
LAST MODIFIED : 3 January 1995

DESCRIPTION :
GL has a number of output buffers - normal, overlay, underlay and popup.
???DB.  Analogous for PEXlib ?
???DB.  See GLXgetconfig and GlxDrawP.h for things that I've skipped ?
==============================================================================*/
{
	/* specify if the buffer is present for the widget */
	Boolean present;
	X3dBufferingMode buffering_mode;
	X3dBufferColourMode colour_mode;
	Colormap colour_map;
	XVisualInfo *visual_information;
	XtCallbackList expose_callback;
	XtCallbackList initialize_callback;
#if defined (OPENGL_API)
	/*???DB.  May be one for all buffers ? */
	GLXContext rendering_context;
#endif
#if defined (GL_API)
#if defined (IBM)
	long int window;
#endif
#if defined (SGI)
	GLXconfig *glx_config;
#endif
#endif
#if defined (PEXLIB_API)
#if defined (IBM)
	PEXPipelineContext pipeline_context;
	PEXRenderer renderer;
#endif
#if defined (VAX)
	pexPipelineContext pipeline_context;
	pexRenderer renderer;
#endif
#endif
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

#if defined (OPENGL_API)
void X3dThreeDDrawingAddReadContext(GLXDrawable read_drawable);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Makes the current ThreeDDrawing the current GL destination and the 
supplied drawable the current GL source.
==============================================================================*/

GLXContext ThreeDDrawing_get_shareable_context(void);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Exports the shareable context for the digital media extensions.
==============================================================================*/
#endif /* defined (OPENGL_API) */
#endif
