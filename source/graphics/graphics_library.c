/*******************************************************************************
FILE : graphics_library.c

LAST MODIFIED : 5 December 2001

DESCRIPTION :
Functions for interfacing with the graphics library.
==============================================================================*/
#if defined (OPENGL_API)
#if defined (UNIX)
#include <dlfcn.h>
#endif /* defined (UNIX) */
#include <math.h>
#include <string.h>
#include <stdio.h>
#if defined (MOTIF) /* switch (USER_INTERFACE) */
#include <X11/Xlib.h>
#include <GL/glu.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include <gtk/gtk.h>
#if ! defined (WIN32_SYSTEM)
/* SAB The WIN32 system currently uses GTK GL AREA for open GL. */
#include <gtk/gtkgl.h>
#endif /* ! defined (WIN32_SYSTEM) */
#endif /* switch (USER_INTERFACE) */
#endif /* defined (OPENGL_API) */
#include "general/debug.h"
#define GRAPHICS_LIBRARY_C
#include "graphics/graphics_library.h"
#if defined (MOTIF)
#include "three_d_drawing/ThreeDDraw.h"
#endif /* defined (MOTIF) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

static struct Text_defaults
{
#if defined (MOTIF)
  XFontStruct *graphics_font;
#endif /* defined (MOTIF) */
  int offsetX, offsetY;
} text_defaults;

/*
Global functions
----------------
*/

int initialize_graphics_library(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Sets up the default light, material and light model for the graphics library.
==============================================================================*/
{
#if defined(GLX_ARB_get_proc_address)
	Display *display;
#endif /* defined(GLX_ARB_get_proc_address) */
	int return_code;
	static int initialized=0;

	ENTER(initialize_graphics_library);

	return_code=1;
	if (!initialized)
	{
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		wrapperInitText(user_interface);

#if defined(GLX_ARB_get_proc_address)
		/* Try and load this function while we have the user_interface connection */
		display = User_interface_get_display(user_interface);
		if (255 == GLEXTENSIONFLAG(GLX_ARB_get_proc_address))
		{
			if (query_glx_extension("GLX_ARB_get_proc_address", display,
				DefaultScreen(display)))
			{
				GLEXTENSIONFLAG(GLX_ARB_get_proc_address) = 1;
			}
			else
			{
				GLEXTENSIONFLAG(GLX_ARB_get_proc_address) = 0;
			}
		}
#endif /* defined(GLX_ARB_get_proc_address) */
#endif
		initialized=1;
	}
	LEAVE;

	return (return_code);
} /* initialize_graphics_library */

int gtMatrix_is_identity(gtMatrix *matrix)
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix> is the 4x4 identity
==============================================================================*/
{
	int return_code;

	ENTER(gtMatrix_is_identity);
	if (matrix)
	{
		return_code = 
			((*matrix)[0][0] == 1.0) &&
			((*matrix)[0][1] == 0.0) &&
			((*matrix)[0][2] == 0.0) &&
			((*matrix)[0][3] == 0.0) &&
			((*matrix)[1][0] == 0.0) &&
			((*matrix)[1][1] == 1.0) &&
			((*matrix)[1][2] == 0.0) &&
			((*matrix)[1][3] == 0.0) &&
			((*matrix)[2][0] == 0.0) &&
			((*matrix)[2][1] == 0.0) &&
			((*matrix)[2][2] == 1.0) &&
			((*matrix)[2][3] == 0.0) &&
			((*matrix)[3][0] == 0.0) &&
			((*matrix)[3][1] == 0.0) &&
			((*matrix)[3][2] == 0.0) &&
			((*matrix)[3][3] == 1.0);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gtMatrix_is_identity.  Missing matrix");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gtMatrix_is_identity */

int gtMatrix_match(gtMatrix *matrix1, gtMatrix *matrix2)
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> are identical.
==============================================================================*/
{
	int return_code;

	ENTER(gtMatrix_match);
	if (matrix1 && matrix2)
	{
		return_code = 
			((*matrix1)[0][0] == (*matrix2)[0][0]) &&
			((*matrix1)[0][1] == (*matrix2)[0][1]) &&
			((*matrix1)[0][2] == (*matrix2)[0][2]) &&
			((*matrix1)[0][3] == (*matrix2)[0][3]) &&
			((*matrix1)[1][0] == (*matrix2)[1][0]) &&
			((*matrix1)[1][1] == (*matrix2)[1][1]) &&
			((*matrix1)[1][2] == (*matrix2)[1][2]) &&
			((*matrix1)[1][3] == (*matrix2)[1][3]) &&
			((*matrix1)[2][0] == (*matrix2)[2][0]) &&
			((*matrix1)[2][1] == (*matrix2)[2][1]) &&
			((*matrix1)[2][2] == (*matrix2)[2][2]) &&
			((*matrix1)[2][3] == (*matrix2)[2][3]) &&
			((*matrix1)[3][0] == (*matrix2)[3][0]) &&
			((*matrix1)[3][1] == (*matrix2)[3][1]) &&
			((*matrix1)[3][2] == (*matrix2)[3][2]) &&
			((*matrix1)[3][3] == (*matrix2)[3][3]);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gtMatrix_match.  Missing matrices");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gtMatrix_match */

int gtMatrix_to_euler(gtMatrix matrix, float *euler_angles)
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
Returns <euler_angles> in radians.
==============================================================================*/
{
	int return_code;
#define MATRIX_TO_EULER_TOLERANCE 1.0E-12

	ENTER(matrix_euler_float4);
	if ((fabs(matrix[0][0])>MATRIX_TO_EULER_TOLERANCE) &&
		(fabs(matrix[0][1])>MATRIX_TO_EULER_TOLERANCE))
	{
		euler_angles[0] = atan2(matrix[0][1],matrix[0][0]);
		euler_angles[2] = atan2(matrix[1][2],matrix[2][2]);
		euler_angles[1] = atan2(-matrix[0][2],matrix[0][0]/
			cos(euler_angles[0]));
	}
	else
	{
		if (fabs(matrix[0][0])>MATRIX_TO_EULER_TOLERANCE)
		{
			euler_angles[0] = atan2(matrix[0][1],matrix[0][0]);
			euler_angles[2] = atan2(matrix[1][2],matrix[2][2]);
			euler_angles[1] = atan2(-matrix[0][2],matrix[0][0]/
				cos(euler_angles[0]));
		}
		else
		{
			if (fabs(matrix[0][1])>MATRIX_TO_EULER_TOLERANCE)
			{
				euler_angles[0] = atan2(matrix[0][1],matrix[0][0]);
				euler_angles[2] = atan2(matrix[1][2],matrix[2][2]);
				euler_angles[1] = atan2(-matrix[0][2],matrix[0][1]/
					sin(euler_angles[0]));
			}
			else
			{
				euler_angles[1] = atan2(-matrix[0][2],0); /* get +/-1 */
				euler_angles[0] = 0;
				euler_angles[2] = atan2(-matrix[2][1],
					-matrix[2][0]*matrix[0][2]);
			}
		}
	}
	return_code = 1;

	LEAVE;
	return(return_code);
} /* matrix_euler */

int euler_to_gtMatrix(float *euler_angles, gtMatrix matrix)
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
<euler_angles> are in radians.
==============================================================================*/
{
	double cos_azimuth,cos_elevation,cos_roll,sin_azimuth,sin_elevation,
		sin_roll;
	int return_code;

	ENTER(euler_to_gtMatrix);

	cos_azimuth = cos(euler_angles[0]);
	sin_azimuth = sin(euler_angles[0]);
	cos_elevation = cos(euler_angles[1]);
	sin_elevation = sin(euler_angles[1]);
	cos_roll = cos(euler_angles[2]);
	sin_roll = sin(euler_angles[2]);
	matrix[0][0] = cos_azimuth*cos_elevation;
	matrix[0][1] = sin_azimuth*cos_elevation;
	matrix[0][2] = -sin_elevation;
	matrix[1][0] = cos_azimuth*sin_elevation*sin_roll-
		sin_azimuth*cos_roll;
	matrix[1][1] = sin_azimuth*sin_elevation*sin_roll+
		cos_azimuth*cos_roll;
	matrix[1][2] = cos_elevation*sin_roll;
	matrix[2][0] = cos_azimuth*sin_elevation*cos_roll+
		sin_azimuth*sin_roll;
	matrix[2][1] = sin_azimuth*sin_elevation*cos_roll-
		cos_azimuth*sin_roll;
	matrix[2][2] = cos_elevation*cos_roll;

	/* Populate the 4 x 4 */
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	matrix[3][2] = 0.0;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
	matrix[3][3] = 1.0;

	return_code = 1;

	LEAVE;
	return (return_code);
} /* euler_matrix */

int gtMatrix_match_with_tolerance(gtMatrix *matrix1, gtMatrix *matrix2,
	float tolerance)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> have no components different by
more than <tolerance> times the largest absolute value in either matrix.
==============================================================================*/
{
	float abs_value, difference, max_abs_value, max_difference;
	int i, j, return_code;

	ENTER(gtMatrix_match_with_tolerance);
	if (matrix1 && matrix2 && (0.0 <= tolerance) && (1.0 >= tolerance))
	{
		max_abs_value = 0.0;
		max_difference = 0.0;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				if ((abs_value = fabs((*matrix1)[i][j])) > max_abs_value)
				{
					max_abs_value = abs_value;
				}
				if ((abs_value = fabs((*matrix2)[i][j])) > max_abs_value)
				{
					max_abs_value = abs_value;
				}
				if ((difference = fabs((*matrix2)[i][j] - (*matrix1)[i][j])) >
					max_difference)
				{
					max_difference = difference;
				}
			}
		}
		return_code = (max_difference <= (tolerance*max_abs_value));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gtMatrix_match_with_tolerance.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gtMatrix_match_with_tolerance */

#if defined (OPENGL_API)
static int fontOffset = 0;

void wrapperReadMatrix(GLenum matrixName,gtMatrix *theMatrix)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	int col,i,row;
	static float vector[16];

	ENTER(wrapperReadMatrix);
	glGetFloatv(matrixName,vector);
	i=0;
	for (col=0;col<4;col++)
	{
		for (row=0;row<4;row++)
		{
			(*theMatrix)[col][row]=vector[i++];
		}
	}
	LEAVE;
} /* wrapperReadMatrix */

void wrapperLoadCurrentMatrix(gtMatrix *theMatrix)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	int col,i,row;
	static float vector[16];

	ENTER(wrapperLoadCurrentMatrix);
	i=0;
	for (col=0;col<4;col++)
	{
		for (row=0;row<4;row++)
		{
			vector[i++]=(*theMatrix)[col][row];
		}
	}
	glLoadMatrixf(vector);
	LEAVE;
} /* wrapperLoadCurrentMatrix */

void wrapperMultiplyCurrentMatrix(gtMatrix *theMatrix)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	int col,i,row;
	static float vector[16];

	ENTER(wrapperMultiplyCurrentMatrix);
	i=0;
	for (col=0;col<4;col++)
	{
		for (row=0;row<4;row++)
		{
			vector[i++]=(*theMatrix)[col][row];
		}
	}
	glMultMatrixf(vector);
	LEAVE;
} /* wrapperMultiplyCurrentMatrix */

void wrapperWindow(float window_left, float window_right, float window_bottom,
  float window_top, float window_near, float window_far)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	GLint currentMatrixMode;

	ENTER(wrapperWindow);
	glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(window_left,window_right,window_bottom,window_top,
	  window_near,window_far);
	glMatrixMode(currentMatrixMode);
	LEAVE;
} /* wrapperWindow */

void wrapperPolarview(float A,float B,float C,float D)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	ENTER(wrapperPolarview);
	glTranslatef(0.0,0.0,-A);
	glRotatef(-D/10.0,0.0,0.0,1.0);
	glRotatef(-C/10.0,1.0,0.0,0.0);
	glRotatef(-B/10.0,0.0,0.0,1.0);
	LEAVE;
} /* wrapperPolarview */

void wrapperPerspective(float A,float B,float C,float D)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	GLint currentMatrixMode;

	ENTER(wrapperPerspective);
	glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(0.1*A,B,C,D);
	glMatrixMode(currentMatrixMode);
	LEAVE;
} /* wrapperPerspective */

void wrapperOrtho(float a,float b,float c,float d,float e,float f)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	GLint currentMatrixMode;

	ENTER(wrapperOrtho);
	glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(a,b,c,d,e,f);
	glMatrixMode(currentMatrixMode);
	LEAVE;
} /* wrapperOrtho */

void wrapperOrtho2D(float a,float b,float c,float d)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	GLint currentMatrixMode;

	ENTER(wrapperOrtho2D);
	glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(a,b,c,d);
	glMatrixMode(currentMatrixMode);
	LEAVE;
} /* wrapperOrtho2D */

void wrapperPrintText(char *theText)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/
{
	ENTER(wrapperPrintText);
	/* push all the state for list operations onto the "attribute stack" */
	glPushAttrib(GL_LIST_BIT);
#if defined (MOTIF)
	glBitmap(0, 0, 0, 0, text_defaults.offsetX, text_defaults.offsetY, NULL);
#endif /* defined (MOTIF) */
	if (fontOffset)
	{
		/* set the list base (i.e. the number that is added to each and every list
			call made from now on) */
		glListBase(fontOffset);
		/* call a vector of lists, consisting of unsigned bytes (chars in C).  (Each
			char in the string therefore invokes a list call that draws the character
			that it represents to screen, and updates the current Raster Position state
			variable in OpenGL to advance the "print cursor").  */
		glCallLists(strlen(theText),GL_UNSIGNED_BYTE,(GLubyte *)theText);
	}
	else
	{
		display_message(WARNING_MESSAGE,"wrapperPrintText.  "
			"No graphics font has been initialised.");
	}
	/* restore the list state varibles back to the way they were. (This undoes the
		glListBase command, and put the list base back to its previous (and possibly
		non-zero) value. We could have read it in ourselves, but the book appears to
		believe that this is a cleaner implementation, and I tend to agree) */
	glPopAttrib();
	LEAVE;
} /* wrapperPrintText */

void wrapperInitText(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 November 1998

DESCRIPTION :
???DB.  Check on list numbers
Graphics font name now read in from Cmgui XDefaults file.
==============================================================================*/
{
#if defined (MOTIF)  /* switch (USER_INTERFACE) */
#define XmNgraphicsFont "graphicsFont"
#define XmCGraphicsFont "GraphicsFont"
#define XmNgraphicsTextOffsetX "graphicsTextOffsetX"
#define XmCGraphicsTextOffsetX "GraphicsTextOffsetX"
#define XmNgraphicsTextOffsetY "graphicsTextOffsetY"
#define XmCGraphicsTextOffsetY "GraphicsTextOffsetY"
	static XtResource resources[]=
	{
		{
			XmNgraphicsFont,
			XmCGraphicsFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(struct Text_defaults,graphics_font),
			XmRString,
			"-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*"
		},
		{
			XmNgraphicsTextOffsetX,
			XmCGraphicsTextOffsetX,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Text_defaults,offsetX),
			XmRString,
			"0"
		},
		{
			XmNgraphicsTextOffsetY,
			XmCGraphicsTextOffsetY,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Text_defaults,offsetY),
			XmRString,
			"0"
		}
	};
#elif defined (GTK_USER_INTERFACE)  /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
	char font_string[] = "courier 12";
	static gint font_height;
	PangoFontDescription *font_desc;
	PangoFont *font;
	PangoFontMetrics *font_metrics;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* switch (USER_INTERFACE) */

	ENTER(wrapperInitText);
	if (user_interface)
	{
#if defined (MOTIF)
		text_defaults.graphics_font=(XFontStruct *)NULL;
#endif /* defined (MOTIF) */
		text_defaults.offsetX = 0;
		text_defaults.offsetY = 0;

		if (!fontOffset)
		{
			fontOffset = glGenLists (256);
#if defined (MOTIF)  /* switch (USER_INTERFACE) */
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&text_defaults,resources,XtNumber(resources),NULL);
			glXUseXFont(text_defaults.graphics_font->fid,0,256,fontOffset);
#elif defined (GTK_USER_INTERFACE)  /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2 && ! defined (WIN32_SYSTEM)
			/* SAB The WIN32 system currently uses GTK GL AREA for open GL. */
			/* Generate font display lists. */

			font_desc = pango_font_description_from_string (font_string);

			font = gdk_gl_font_use_pango_font (font_desc, 0, 128, fontOffset);
			if (font == NULL)
			{
				display_message(WARNING_MESSAGE,"wrapperInitText.  "
					"Text display is not implemented for Gtk prior to version 2.");
			}

			font_metrics = pango_font_get_metrics (font, NULL);

			font_height = pango_font_metrics_get_ascent (font_metrics) +
				pango_font_metrics_get_descent (font_metrics);
			font_height = PANGO_PIXELS (font_height);

			pango_font_description_free (font_desc);
			pango_font_metrics_unref (font_metrics);
#else /* GTK_MAJOR_VERSION >= 2 */
		/* Not implemented */
			display_message(WARNING_MESSAGE,"wrapperInitText.  "
				"Text display is not implemented for Gtk prior to version 2.");
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* switch (USER_INTERFACE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"wrapperInitText.  Missing user_interface");
	}
	LEAVE;
} /* wrapperInitText */
#endif

#if defined (OLD_CODE)
/*******************************************************************************
FILE : graphics_library.c

LAST MODIFIED : 13 February 1995

DESCRIPTION :
Functions for interfacing with the graphics library.

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

???DB.  It doesn't have an input callback because it based on a Composite
widget.  To get an input callback it will need to be changed to being based on
a XmDrawingArea widget ?

COMMENTS: The graPHIGS sections have not been developed along with the DECPHIGS
so far
==============================================================================*/
#if defined (GL_API) && defined (IBM)
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
/* include the private header files for the superclasses */
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
/* for the callback reasons */
#include <Xm/Xm.h>
#endif

/*
Module/Private types
--------------------
*/
#if defined (GL_API) && defined (IBM)
typedef struct
{
	/* pointer to extension record */
	caddr_t extension;
} GlibClassPart;

typedef struct _GlibClassRec
{
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	GlibClassPart glib_class;
} GlibClassRec;

typedef struct
{
	/* resources */
	XtCallbackList expose_callback;
	XtCallbackList gconfig_callback;
	XtCallbackList resize_callback;
	int init_color;
	/* private state */
	/* GL Window Id */
	long gid;
} GlibPart;

typedef struct _GlibRec
{
	CorePart core;
	CompositePart composite;
	GlibPart glib;
} GlibRec;
#endif

/*
Module/Private variables
------------------------
*/
#if defined (GL_API) && defined (IBM)
extern GlibClassRec glibClassRec;
int glwinid;
#endif

/*
Module/Private functions
------------------------
*/
#if defined (GL_API) && defined (IBM)
static void GlibDestroy(wid)
	GlibWidget wid;
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Destroy the widget data and close the GL window.
==============================================================================*/
{
	/* GL close command */
	winclose(wid->glib.gid);
} /* GlibDestroy */

static void GlibExpose(wid, event, region)
	GlibWidget wid;
	XEvent *event;
	Region region;
/*******************************************************************************
LAST MODIFIED : 24 March 1993

DESCRIPTION :
Process all exposure events (GL REDRAW events).  Callback to application and let
them handle the redraw.
==============================================================================*/
{
	GlibCallbackStruct callback;

	callback.reason=XmCR_EXPOSE;
	callback.event=event;
	callback.region=region;
	XtWidgetCallCallbacks(wid->glib.expose_callback,&callback);
} /* GlibExpose */

static void GlibResize(wid,event)
	GlibWidget wid;
	XEvent *event;
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Process all resize events (GL REDRAW/PEICECHANGE events).  Callback to
application and let them handle the resize.
==============================================================================*/
{
	GlibCallbackStruct callback;

	callback.reason=XmCR_RESIZE;
	callback.event=event;
	callback.region=(Region)NULL;
	XtWidgetCallCallbacks(wid->glib.resize_callback,&callback);
} /* GlibResize */

static void GlibInitialize(request, new)
	GlibWidget request, new;
/*******************************************************************************
LAST MODIFIED : 24 March 1993

DESCRIPTION :
Initialize widget data structures.
==============================================================================*/
{
	/* Create GL widget */
	new->core.x = request->core.x;
	new->core.y = request->core.y;
	new->core.width = request->core.width;
	new->core.height = request->core.height;
	new->core.parent = request->core.parent;
} /* GlibInitialize */

static void GlibRealize(wid, vmask, attr)
	GlibWidget wid;
	Mask *vmask;
	XSetWindowAttributes *attr;
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Actually create the GL window and place it at the proper location within the
widget.
???DB.  What should be done about the callback ?
==============================================================================*/
{
	/* temporary window id of parent */
	Window tmpid;

	/* Does a window exist already. If yes do nothing */
	if (wid->core.window == None)
	{
		/* Open an unmapped GL window with noborder of the proper size */
		/* do not map window */
		noport();
		/* do not have a border */
		noborder();
		/* size */
		prefsize(wid->core.width,wid->core.height);
		/* open GL window */
		glwinid =  wid->glib.gid = winopen("");
		/* Get the XWindow ID of the GL window */
		wid->core.window = getXwid(wid->glib.gid);
		/* Get the XWindow ID of the parent window */
		tmpid = XtWindow(wid->core.parent);
		/* If there is a parent continue */
		if (tmpid != None)
		{
			/* Reparent the GL window to be a child of the specified parent window.
				This alleviates the need for visual matching to occur. */
			XReparentWindow(XtDisplay(wid),wid->core.window,tmpid,wid->core.x,
				wid->core.y);
		}
	}
	/* Map the widget */
	XtMapWidget(wid);
	/* set window to be current render window */
	winset(wid->glib.gid);
	/* Call the gconfig callbacks */
	XtWidgetCallCallbacks(wid->glib.gconfig_callback,NULL);
	return;
} /* GlibRealize */
#endif

#if defined (GL_API) && defined (IBM)
/*
Glib resources
--------------
*/
static XtResource resources[] =
{
	{
		XglNexposeCallback,
		XtCCallback, XtRCallback, sizeof (caddr_t),
		XtOffset (GlibWidget, glib.expose_callback),
		XtRCallback, (caddr_t) NULL
	},
	{
		XglNgconfigCallback,
		XtCCallback, XtRCallback, sizeof (caddr_t),
		XtOffset (GlibWidget, glib.gconfig_callback),
		XtRCallback, (caddr_t) NULL
	},
	{
		XglNresizeCallback,
		XtCCallback, XtRCallback, sizeof (caddr_t),
		XtOffset (GlibWidget, glib.resize_callback),
		XtRCallback, (caddr_t) NULL
	},
};

/* Glib class record */
externaldef(glibclassrec) GlibClassRec glibClassRec =
{
	/* Core Class Rec */
	{
		/* superclass */ (WidgetClass) &compositeClassRec,
		/* class_name */ "Glib",
		/* size */ sizeof(GlibRec),
		/* Class Initializer */ NULL,
		/* class_part_initialize */ NULL,
		/* Class init'ed ? */ FALSE,
		/* initialize */ GlibInitialize,
		/* initialize_notify */ NULL,
		/* realize */ GlibRealize,
		/* actions */ NULL,
		/* num_actions */ 0,
		/* resources */ resources,
		/* resource_count */ XtNumber(resources),
		/* xrm_class */ NULLQUARK,
		/* compress_motion */ FALSE,
		/* compress_exposure */ TRUE,
		/* compress_enterleave */ TRUE,
		/* visible_interest */ FALSE,
		/* destroy */ GlibDestroy,
		/* resize */ GlibResize,
		/* expose */ GlibExpose,
		/* set_values */ NULL,
		/* set_values_hook */ NULL,
		/* set_values_almost */ XtInheritSetValuesAlmost,
		/* get_values_hook */ NULL,
		/* accept_focus */ NULL,
		/* intrinsics version */ XtVersion,
		/* callback offsets */ NULL,
		/* tm_table */ NULL,
		/* query_geometry */ NULL,
		/* display_accelerator */ NULL,
		/* extension */ NULL
	},
	/* Composite Class Rec */
	{
		/* geometry_manager */ XtInheritGeometryManager,
		/* change_managed */ XtInheritChangeManaged,
		/* insert_child */ XtInheritInsertChild,
		/* delete_child */ XtInheritDeleteChild,
		/* extension */ NULL
	},
	/* Glib Class Rec */
	{
		/* extension */ NULL
	}
};

externaldef(glibwidgetclass) WidgetClass glibWidgetClass =
	(WidgetClass) (&glibClassRec);
#endif

/*
Global/Public functions
-----------------------
*/
#if defined (GL_API) && defined (IBM)
int GlWinsetWidget(Widget widget)
/*******************************************************************************
LAST MODIFIED : 24 March 1993

DESCRIPTION :
Set the window to which rendering will appear.
==============================================================================*/
{
	GlibWidget glib_widget;
	int return_code;
	XWindowAttributes window_attributes;

	if ((glib_widget=(GlibWidget)widget)&&(XtWindow(glib_widget)))
	{
		/* GL winset on stored GL id */
		winset(glib_widget->glib.gid);
		/* Motif is a window manager that does not assign seperate colormaps to
			child windows */
		/* get the colormap for the GL window */
		XGetWindowAttributes(XtDisplay(glib_widget),XtWindow(glib_widget),
			&window_attributes);
		/* Install the colormap for the GL window */
		XInstallColormap(XtDisplay(glib_widget),window_attributes.colormap);
		/* Flush stream to server */
		XFlush(XtDisplay(glib_widget));
		return_code=1;
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* GlWinsetWidget */

Widget GlXCreateMDraw(Widget parent,char *name,ArgList arglist,
	Cardinal argcount)
/*******************************************************************************
LAST MODIFIED : 24 March 1993

DESCRIPTION :
A Motif-style create routine.
==============================================================================*/
{
	return (XtCreateWidget(name,glibWidgetClass,parent,arglist,argcount));
} /* GlXCreateMDraw */
#endif
#endif

#if defined (OPENGL_API)
int query_gl_extension(char *extName)
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Search for extName in the GL extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from the insight book on OpenGL Extensions
==============================================================================*/
{
	char *end, *p;
#if defined (DEBUG)
	char *glu, *vendor, *version;
#endif /* defined (DEBUG) */
	int extNameLen, n, return_code;

	/* check arguments */
	if (extName)
	{
		extNameLen=strlen(extName);

		p=(char *)glGetString(GL_EXTENSIONS);
#if defined (DEBUG)
		/* For debugging */
		vendor=(char *)glGetString(GL_VENDOR);
		version=(char *)glGetString(GL_VERSION);
		glu=(char *)gluGetString(GLU_EXTENSIONS);
		printf("Vendor %s\n", vendor);
		printf("Version %s\n", version);
		printf("OpenGL %s\n", p);
		printf("GLU %s\n", glu);
#endif /* defined (DEBUG) */
		if (NULL==p)
		{
			return_code=0;
		}
		else
		{
			end=p+strlen(p);
			return_code = 0;
			while (p<end)
			{
				n=strcspn(p," ");
				if ((extNameLen==n)&&(strncmp(extName,p,n)==0)) 
				{
					return_code=1;
				}
				p += (n+1);
			}
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* query_extension */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int query_gl_version(int major_version, int minor_version)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Returns true if the OpenGL version is at least <major_version>.<minor_version>
==============================================================================*/
{
	char *version;
	static int major = 0, minor = 0;
	int return_code;

	return_code = 0;

	if (!major)
	{
		version=(char *)glGetString(GL_VERSION);

		if (!(2 == sscanf(version, "%d.%d", &major, &minor)))
		{
			major = -1;
			minor = -1;
		}
	}

	if (major > major_version)
	{
		return_code = 1;
	}
	else if (major == major_version)
	{
		if (minor >= minor_version)
		{
			return_code = 1;
		}
	}

	return (return_code);
} /* query_gl_version */
#endif /* defined (OPENGL_API) */

int Graphics_library_read_pixels(unsigned char *frame_data,
	int width, int height, enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Read pixels from the current graphics context into <frame_data> of size <width>
and <height> according to the storage type.  'MakeCurrent' the desired source 
before calling this routine.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_library_read_pixels);
	if (frame_data && width && height)
	{
#if defined (OPENGL_API)
		/* Make sure we get it from the front for a double buffer,
			has no effect on a single buffer, keep the old read
			buffer so we can set it back after reading */
		glReadBuffer(GL_FRONT);
		switch(storage)
		{
			case TEXTURE_LUMINANCE:
			{
				glReadPixels(0, 0, width, height, GL_LUMINANCE,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
			case TEXTURE_LUMINANCE_ALPHA:
			{
				glReadPixels(0, 0, width, height, GL_LUMINANCE_ALPHA,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
			case TEXTURE_RGB:
			{
				glReadPixels(0, 0, width, height, GL_RGB,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
			case TEXTURE_RGBA:
			{
				glReadPixels(0, 0, width, height, GL_RGBA,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
#if defined (GL_ABGR_EXT)
			case TEXTURE_ABGR:
			{
				glReadPixels(0, 0, width, height, GL_ABGR_EXT,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
#endif /* defined (GL_ABGR_EXT) */
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_library_read_pixels.  Unsupported or unknown storage type");
				return_code=0;
			} break;
		}
#else /* defined (OPENGL_API) */
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_library_read_pixels.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_library_read_pixels */

static void *Graphics_library_get_function_ptr(char *function_name)
/*******************************************************************************
LAST MODIFIED : 20 February 2004

DESCRIPTION :
Finds and loads gl function symbols at runtime.
==============================================================================*/
{
	void *function_ptr, *symbol_table;

	ENTER(Graphics_library_get_function_ptr);
	if (function_name)
	{
#if defined (OPENGL_API)
#if defined (WIN32_SYSTEM)
		function_ptr = (void *) wglGetProcAddress(function_name);
#else /* defined (WIN32_SYSTEM) */
#if defined(GLX_ARB_get_proc_address)
		/* We don't try and load this here because testing for a GLX extension
			requires the Display, it is done in the initialize_graphics_library above */
		if (1 == GLEXTENSIONFLAG(GLX_ARB_get_proc_address))
		{
			function_ptr = (void *) glXGetProcAddressARB((const GLubyte *) function_name);
		}
		else
		{
#endif /* defined(GLX_ARB_get_proc_address) */
#if defined (UNIX)
			symbol_table = dlopen((char *)0, RTLD_LAZY);
			function_ptr = dlsym(symbol_table, function_name);
			dlclose(symbol_table);
#else /* defined (UNIX) */
			function_ptr = NULL;			
#endif /* defined (UNIX) */
#if defined(GLX_ARB_get_proc_address)
		}
#endif /* defined(GLX_ARB_get_proc_address) */
#endif /* defined (WIN32_SYSTEM) */
#else /* defined (OPENGL_API) */
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_library_get_function_ptr.  Missing function name.");
		function_ptr = NULL;
	}

	LEAVE;

	return (function_ptr);
} /* Graphics_library_get_function_ptr */

int Graphics_library_load_extension(char *extension_name)
/*******************************************************************************
LAST MODIFIED : 20 February 2004

DESCRIPTION :
Attempts to load the particular extensions.  Returns true if all
the extensions succeed, false if not.
==============================================================================*/
{
	int return_code;
#if defined (GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES)
#define GRAPHICS_LIBRARY_ASSIGN_HANDLE(function,type) function ## _handle = (type)
#else /* defined (GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES) */
#define GRAPHICS_LIBRARY_ASSIGN_HANDLE(function,type)
#endif /* defined (GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES) */

	ENTER(Graphics_library_load_extension);
	if (extension_name)
	{
#if defined (OPENGL_API)
		if (NULL == extension_name)
		{
			return_code = 0;
		}
#if defined GL_VERSION_1_2
		else if (!strcmp(extension_name, "GL_VERSION_1_2"))
		{
			if (255 != GLEXTENSIONFLAG(GL_VERSION_1_2))
			{
				return_code = GLEXTENSIONFLAG(GL_VERSION_1_2);
			}
			else
			{
				if (query_gl_version(1, 2))
				{
					if (GRAPHICS_LIBRARY_ASSIGN_HANDLE(glTexImage3D, PFNGLTEXIMAGE3DPROC)
						Graphics_library_get_function_ptr("glTexImage3D"))
					{
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
				GLEXTENSIONFLAG(GL_VERSION_1_2) = return_code;
			}			
		}
#endif /* GL_VERSION_1_2 */
#if defined GL_VERSION_1_3
		else if (!strcmp(extension_name, "GL_VERSION_1_3"))
		{
			if (255 != GLEXTENSIONFLAG(GL_VERSION_1_3))
			{
				return_code = GLEXTENSIONFLAG(GL_VERSION_1_3);
			}
			else
			{
				if (query_gl_version(1, 3))
				{
					if (GRAPHICS_LIBRARY_ASSIGN_HANDLE(glActiveTexture, PFNGLACTIVETEXTUREPROC)
						Graphics_library_get_function_ptr("glActiveTexture"))
					{
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
				GLEXTENSIONFLAG(GL_VERSION_1_3) = return_code;
			}
		}
#endif /* GL_VERSION_1_3 */
#if defined GL_VERSION_1_4
		else if (!strcmp(extension_name, "GL_VERSION_1_4"))
		{
			if (255 != GLEXTENSIONFLAG(GL_VERSION_1_4))
			{
				return_code = GLEXTENSIONFLAG(GL_VERSION_1_4);
			}
			else
			{
				if (query_gl_version(1, 4))
				{
					if (GRAPHICS_LIBRARY_ASSIGN_HANDLE(glBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC)
						Graphics_library_get_function_ptr("glBlendFuncSeparate"))
					{
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
				GLEXTENSIONFLAG(GL_VERSION_1_4) = return_code;
			}
		}
#endif /* GL_VERSION_1_4 */
#if defined GL_EXT_texture3D
		else if (!strcmp(extension_name, "GL_EXT_texture3D"))
		{
			if (255 != GLEXTENSIONFLAG(GL_EXT_texture3D))
			{
				return_code = GLEXTENSIONFLAG(GL_EXT_texture3D);
			}
			else
			{
				if (query_gl_extension(extension_name))
				{
					/* We are using the non EXT version of these functions as this simplifies
						the code where they are used, and the SGI implementation while currently
						only OpenGL 1.1 supplies both glTexImage3D and glTexImage3DEXT */
					if (GRAPHICS_LIBRARY_ASSIGN_HANDLE(glTexImage3D, PFNGLTEXIMAGE3DPROC)
						Graphics_library_get_function_ptr("glTexImage3D"))
					{
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
				GLEXTENSIONFLAG(GL_EXT_texture3D) = return_code;
			}
		}
#endif /* GL_EXT_texture3D */
#if defined GL_ARB_vertex_program
		else if (!strcmp(extension_name, "GL_ARB_vertex_program"))
		{
			if (255 != GLEXTENSIONFLAG(GL_ARB_vertex_program))
			{
				return_code = GLEXTENSIONFLAG(GL_ARB_vertex_program);
			}
			else
			{
				if (query_gl_extension(extension_name))
				{
					if ((GRAPHICS_LIBRARY_ASSIGN_HANDLE(glGenProgramsARB, PFNGLGENPROGRAMSARBPROC)
						Graphics_library_get_function_ptr("glGenProgramsARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glBindProgramARB, PFNGLBINDPROGRAMARBPROC)
						Graphics_library_get_function_ptr("glBindProgramARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glProgramStringARB, PFNGLPROGRAMSTRINGARBPROC)
						Graphics_library_get_function_ptr("glProgramStringARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glDeleteProgramsARB, PFNGLDELETEPROGRAMSARBPROC)
						Graphics_library_get_function_ptr("glDeleteProgramsARB")))
					{
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
				GLEXTENSIONFLAG(GL_ARB_vertex_program) = return_code;
			}
		}
#endif /* GL_ARB_vertex_program */
#if defined GL_ARB_fragment_program
		else if (!strcmp(extension_name, "GL_ARB_fragment_program"))
		{
			if (255 != GLEXTENSIONFLAG(GL_ARB_fragment_program))
			{
				return_code = GLEXTENSIONFLAG(GL_ARB_fragment_program);
			}
			else
			{
				if (query_gl_extension(extension_name))
				{
					if ((GRAPHICS_LIBRARY_ASSIGN_HANDLE(glGenProgramsARB, PFNGLGENPROGRAMSARBPROC)
						Graphics_library_get_function_ptr("glGenProgramsARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glBindProgramARB, PFNGLBINDPROGRAMARBPROC)
						Graphics_library_get_function_ptr("glBindProgramARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glProgramStringARB, PFNGLPROGRAMSTRINGARBPROC)
						Graphics_library_get_function_ptr("glProgramStringARB")) &&
						(GRAPHICS_LIBRARY_ASSIGN_HANDLE(glDeleteProgramsARB, PFNGLDELETEPROGRAMSARBPROC)
						Graphics_library_get_function_ptr("glDeleteProgramsARB")))
					{
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
				GLEXTENSIONFLAG(GL_ARB_fragment_program) = return_code;
			}
		}
#endif /* GL_ARB_fragment_program */
		else
		{
			display_message(ERROR_MESSAGE,  "Graphics_library_load_extension.  "
				"Extension %s was not compiled in or is unknown.", extension_name);
			return_code = 0;
		}
#else /* defined (OPENGL_API) */
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_library_load_extension.  Missing extension name.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* Graphics_library_load_extension */

int Graphics_library_load_extensions(char *extensions)
/*******************************************************************************
LAST MODIFIED : 20 February 2004

DESCRIPTION :
Attempts to load the space separated list of extensions.  Returns true if all
the extensions succeed, false if not.
==============================================================================*/
{
	char *extension, *next_extension;
	int return_code;

	ENTER(Graphics_library_load_extensions);
	if (extensions)
	{
		return_code = 1;
		extension = extensions;
		while (return_code && (next_extension = strchr(extension, ' ')))
		{
			*next_extension = 0;
			return_code = Graphics_library_load_extension(extension);
			extension = next_extension + 1;
		}
		if (*extension)
		{
			return_code = Graphics_library_load_extension(extension);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_library_load_extensions.  Missing extension name list.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* Graphics_library_load_extensions */
