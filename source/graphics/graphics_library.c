/*******************************************************************************
FILE : graphics_library.c

LAST MODIFIED : 5 December 2001

DESCRIPTION :
Functions for interfacing with the graphics library.
==============================================================================*/
#if defined (OPENGL_API)
#include <math.h>
#include <string.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif /* defined (MOTIF) */
#endif /* defined (OPENGL_API) */
#include "general/debug.h"
#include "graphics/graphics_library.h"
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
Global variables
----------------
*/
/*???DB.  Should be hidden in a function */
int graphics_library_open=0;

/*
Global functions
----------------
*/
int open_graphics_library(void)
/*******************************************************************************
LAST MODIFIED : 14 November 1996

DESCRIPTION :
Function to open the 3-D graphics library (needed for PHIGS).
==============================================================================*/
{
	int return_code;
#if defined (DECPHIGS_API)
	FILE *error_file;
#endif

	ENTER(open_graphics_library);
#if defined (DECPHIGS_API)
	error_file=fopen("sys$error:","w");
	popenphigs(error_file,0);
		/*???DB.  What is the 0 for ? */
	return_code=1;
#endif
#if defined (graPHIGS_API)
	popen_phigs(PDEF_ERR_FILE,PDEF_MEM_SIZE);
	return_code=1;
#endif
#if defined (GL_API)
	return_code=1;
#endif
#if defined (OPENGL_API)
	/* OpenGL initialization is performed at the same time we initialise the
		window manager */
	return_code = 1;
#endif
	graphics_library_open=1;
	LEAVE;

	return (return_code);
} /* open_graphics_library */

int close_graphics_library(void)
/*******************************************************************************
LAST MODIFIED : 2 May 1995

DESCRIPTION :
Function to close the 3-D graphics library (needed for PHIGS).
==============================================================================*/
{
	int return_code;

	ENTER(close_graphics_library);
#if defined (DECPHIGS_API)
	pclosephigs();
	return_code=1;
#endif
#if defined (graPHIGS_API)
	pclose_phigs();
	return_code=1;
#endif
#if defined (GL_API)
	gexit();
	return_code=1;
#endif
#if defined (OPENGL_API)
	return_code = 1;
#endif
	graphics_library_open=0;
	LEAVE;

	return (return_code);
} /* close_graphics_library */

int initialize_graphics_library(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 14 November 1995

DESCRIPTION :
Sets up the default light, material and light model for the graphics library.
==============================================================================*/
{
	int return_code;
	static int initialized=0;

	ENTER(initialize_graphics_library);
	return_code=1;
	if (!initialized)
	{
/*???DB.  Is this necessary ? */
#if defined (GL_API)
		mmode(MVIEWING);
#endif
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		wrapperInitText(user_interface);
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
/*
*  File scope variables
*/
static int fontOffset = 27000;
										/* ^ Is there any way to make sure this doesn't clash? */

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
	/* set the list base (i.e. the number that is added to each and every list
		call made from now on) */
	glListBase(fontOffset);
	/* call a vector of lists, consisting of unsigned bytes (chars in C).  (Each
		char in the string therefore invokes a list call that draws the character
		that it represents to screen, and updates the current Raster Position state
		variable in OpenGL to advance the "print cursor").  */
	glCallLists(strlen(theText),GL_UNSIGNED_BYTE,(GLubyte *)theText);
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
#if defined (MOTIF)
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
#endif /* defined (MOTIF) */

	ENTER(wrapperInitText);
	if (user_interface)
	{
#if defined (MOTIF)
		text_defaults.graphics_font=(XFontStruct *)NULL;
#endif /* defined (MOTIF) */
		text_defaults.offsetX = 0;
		text_defaults.offsetY = 0;
#if defined (MOTIF)
		XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
			&text_defaults,resources,XtNumber(resources),NULL);
		glXUseXFont(text_defaults.graphics_font->fid,0,256,fontOffset);
#endif /* defined (MOTIF) */
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
