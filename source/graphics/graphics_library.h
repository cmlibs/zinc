/*******************************************************************************
FILE : graphics_library.h

LAST MODIFIED : 12 September 2002

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
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#if defined (WIN32_SYSTEM)
/* SAB On Win32 I think you have to load all OpenGL 1.2, 1.3 etc functions
	as extensions and keep pointer references to them.  I haven't done this
	yet so we will undefine the version symbols */
#undef GL_VERSION_1_2
#undef GL_VERSION_1_3
#undef GL_VERSION_1_4
#endif /* defined (WIN32_SYSTEM) */
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

typedef float gtMatrix[4][4];

enum Texture_storage_type
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_LUMINANCE,
	TEXTURE_LUMINANCE_ALPHA,
	TEXTURE_RGB,
	TEXTURE_RGBA,
	TEXTURE_ABGR,
	/* The last two types are special and are not user-selectable */
	TEXTURE_DMBUFFER,
	TEXTURE_PBUFFER
}; /* enum Texture_storage_type */

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

int gtMatrix_is_identity(gtMatrix *matrix);
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix> is the 4x4 identity
==============================================================================*/

int gtMatrix_match(gtMatrix *matrix1, gtMatrix *matrix2);
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> are identical.
==============================================================================*/

int gtMatrix_match_with_tolerance(gtMatrix *matrix1, gtMatrix *matrix2,
	float tolerance);
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> have no components different by
more than <tolerance> times the largest absolute value in either matrix.
==============================================================================*/

int gtMatrix_to_euler(gtMatrix matrix, float *euler_angles);
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
Returns <euler_angles> in radians.
==============================================================================*/

int euler_to_gtMatrix(float *euler_angles, gtMatrix matrix);
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
<euler_angles> are in radians.
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
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int query_gl_version(int major_version, int minor_version);
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Returns true if the OpenGL version is at least <major_version>.<minor_version>
==============================================================================*/
#endif /* defined (OPENGL_API) */

int Graphics_library_read_pixels(unsigned char *frame_data,
	int width, int height, enum Texture_storage_type storage);
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Read pixels from the current graphics context into <frame_data> of size <width>
and <height> according to the storage type.  'MakeCurrent' the desired source 
before calling this routine.
==============================================================================*/
#endif
