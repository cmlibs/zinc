/*******************************************************************************
FILE : ThreeDDraw.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Code for the 3-D drawing widget (including methods and actions).
11 April 1994

Useful examples/files
---------------------
RS6000
/usr/lpp/X11/bin/README.PEX
/usr/lpp/X11/Xamples/pex
/usr/lpp/X11/Xamples/pexlib
/usr/lpp/X11/Xamples/pexlib/doc/PEXlib.PS.tar.Z
/usr/lpp/GL/utilities
/usr/lpp/GL/README
/usr/lpp/graPHIGS/samples/widgets/lib
SGI
No PEXlib
VAX
No GL

Extensions to GL for X
----------------------
SGI
/usr/include/gl/gl.h
/usr/include/gl/glws.h
GLXgetconfig - takes the display, screen and configuration information and
	returns the data needed to create and render GL into an X window.
GLXlink - informs the GL that you intend to render GL into an X window.
GLXunlink - informs the GL that rendering into the X window is finished (frees
	system resources)
GLXwinset - specifies the X window that subsequent GL rendering calls will occur
	in.

IBM
getXdpy - returns the X window connection of the GL session.
getXwid - returns the X window identifier of a GL window.
winX - converts an X window into a GL window.

26 November 1994

OpenGL extension to X
---------------------
Determine whether the GLX extension is defined on the X server:
	Bool glXQueryExtension(Display *dpy,int *errorBase,int *eventBase);
	Bool glXQueryVersion(Display *dpy,int *major,int *minor);
Obtain the desired visual:
	XVisualInfo *glXChooseVisual(Display *dpy,int screen,int *attribList);
	int glXGetConfig(Display *dpy,XVisualInfo *vis,int attrib,int *value);
Manage or query an OpenGL rendering context:
	GLXContext glXCreateContext(Display *dpy,XVisualInfo *vis,
		GLXContext sharelist,Bool direct);
	void glXDestroyContext(Display *dpy,GLXContext ctx);
	void glXCopyContext(Display *dpy,GLXContext src,GLXContext dst,GLuint mask);
	Bool glXIsDirect(Display *dpy,GLXContext ctx);
	Bool glXMakeCurrent(Display *dpy,GLXDrawable draw,GLXContext ctx);
	GLXContext glXGetCurrentContext(void);
	GLXDrawable glXGetCurrentDrawable(void);
Perform off-screen rendering:
	GLXPixmap glXCreateGLXPixmap(Display *dpy,XVisualInfo *vis,Pixmap pixmap);
	void glXDestroyGLXPixmap(Display *dpy,GLXPixmap pix);
Synchronize execution:
	void glXWaitGL(void)
	void glXWaitX(void)
Exchange front and back buffers:
	void glXSwapBuffers(Display *dpy,GLXDrawable drawable);
Use an X font:
	void glXUseXFont(Font font,int first,int count,int listBase);

???MS.  25 July 1996.  Made graphics glXcontexts sharable for openGL to work
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>
#include "three_d_drawing/ThreeDDraP.h"
#if defined (PEXLIB_API)
#if defined (VAX)
#include <PEXlibprotos.h>
#endif
#endif
#include "general/debug.h"

/*
Module variables
----------------
*/
#if defined (PEXLIB_API)
static int PEXlib_initialized=0;
#endif
static Widget current_ThreeDDrawing=(Widget)NULL;
#if defined (OPENGL_API)
/* To share OpenGL display lists between GLXContexts, need to specify with */
/* each new one a context to share with. The shareable_context will be the */
/* first context created, and is only destroyed in X3dThreeDDrawingCleanUp */
static GLXContext shareable_context=(GLXContext)NULL;
static int glx_major_version = 0;
static int glx_minor_version = 0;
#endif

/*
Resources
---------
*/
static XtResource resources[]=
/*******************************************************************************
LAST MODIFIED : 27 April 1994

DESCRIPTION :
Resources for the 3-D drawing widget.
???DB.  Configure callback ?  Probably not because graphics library dependent.
	Settings ?
???DB.  Some of these may eventually disappear, eg expose will be done inside ?
==============================================================================*/
{
	{
		X3dNbufferColourMode,
		X3dCBufferColourMode,
		X3dRBufferColourMode,
		sizeof(X3dBufferColourMode),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.colour_mode),
		XtRString,
		"X3dCOLOUR_INDEX_MODE"
	},
	{
		X3dNbufferingMode,
		X3dCBufferingMode,
		X3dRBufferingMode,
		sizeof(X3dBufferingMode),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.buffering_mode),
		XtRString,
		"X3dSINGLE_BUFFERING"
	},
	{
		X3dNvisualId,
		X3dCVisualId,
		XtRInt,
		sizeof(int),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.visual_id),
		XtRInt,
		0
	},
	{
		X3dNexposeCallback,
		X3dCExposeCallback,
		XtRCallback,
		sizeof(XtPointer),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.expose_callback),
		XtRCallback,
		(XtPointer)NULL
	},
	{
		X3dNinitializeCallback,
		X3dCInitializeCallback,
		XtRCallback,
		sizeof(XtPointer),
		XtOffsetOf(ThreeDDrawingRec,
			three_d_drawing.normal_buffer.initialize_callback),
		XtRCallback,
		(XtPointer)NULL
	},
	{
		X3dNinputCallback,
		X3dCInputCallback,
		XtRCallback,
		sizeof(XtPointer),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.input_callback),
		XtRCallback,
		(XtPointer)NULL
	},
	{
		X3dNresizeCallback,
		X3dCResizeCallback,
		XtRCallback,
		sizeof(XtPointer),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.resize_callback),
		XtRCallback,
		(XtPointer)NULL
	},
#if defined (OPENGL_API) || defined (PEXLIB_API)
	{
		X3dNrenderingContext,
		X3dCRenderingContext,
		X3dRRenderingContext,
#if defined (OPENGL_API)
		sizeof(GLXContext),
		XtOffsetOf(ThreeDDrawingRec,
			three_d_drawing.normal_buffer.rendering_context),
#endif
#if defined (PEXLIB_API)
		sizeof(PEXRenderer),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.renderer),
#endif
		XtRImmediate,
		NULL
	},
#endif
}; /* resources */

/*
Methods
-------
*/
static void ThreeDDrawingRealize(Widget widget,XtValueMask *value_mask,
	XSetWindowAttributes *attributes)
/*******************************************************************************
LAST MODIFIED : 18 April 1998

DESCRIPTION :
The realize method.  Create the 3-D drawing window and place it within the
widget.
==============================================================================*/
{
	Dimension height,width;
	Display *display;
	int i,number_of_colour_map_windows;
	ThreeDDrawingWidget drawing_widget;
	Position x,y;
	unsigned int number_of_children;
	Window *children,*colour_map_windows,*new_colour_map_windows,parent_window,
		root_window,top_level_window,window;
	XVisualInfo *visual_information;
	X3dThreeDDrawCallbackStruct call_data;
#if defined (GL_API)
#if defined (SGI)
	GLXconfig *configuration_entry;
#endif
#endif
#if defined (PEXLIB_API)
	PEXRendererAttributes renderer_attributes;
#endif

	window = (Window)NULL;
	if (drawing_widget=(ThreeDDrawingWidget)widget)
	{
		/* if the widget doesn't already have a window */
		if (None==(drawing_widget->core).window)
		{
			display=XtDisplay(drawing_widget);
#if defined (OPENGL_API) || defined (GL_API) || defined (PEXLIB_API)
			if (
#if defined (OPENGL_API)
				((drawing_widget->three_d_drawing).normal_buffer.rendering_context)&&
#endif
#if defined (GL_API) && (SGI)
				(configuration_entry=
				(drawing_widget->three_d_drawing).normal_buffer.glx_config)&&
#endif
#if defined (PEXLIB_API)
				(PEXlib_initialized)&&
#endif
				(visual_information=
				(drawing_widget->three_d_drawing).normal_buffer.visual_information))
			{
				/* create the colour map */
				if ((X3dCOLOUR_INDEX_MODE==(drawing_widget->three_d_drawing).
					normal_buffer.colour_mode)&&(PseudoColor==visual_information->class))
				{
					attributes->colormap=XCreateColormap(display,
						RootWindow(display,visual_information->screen),
						visual_information->visual,AllocAll);
				}
				else
				{
					attributes->colormap=XCreateColormap(display,
						RootWindow(display,visual_information->screen),
						visual_information->visual,AllocNone);
				}
				*value_mask |= CWColormap;
				if (None!=attributes->colormap)
				{
					XtVaGetValues(widget,
						XtNx,&x,
						XtNy,&y,
						XtNwidth,&width,
						XtNheight,&height,
						NULL);
					/* best performance if no backing store */
					attributes->backing_store=NotUseful;
					*value_mask |= CWBackingStore;
					/* create the window */
					if (window=XCreateWindow(display,XtWindow(XtParent(drawing_widget)),
						x,y,width,height,0,visual_information->depth,
						(unsigned int)InputOutput,visual_information->visual,*value_mask,
						attributes))
					{
						(drawing_widget->core).window=window,
						(drawing_widget->core).colormap=attributes->colormap;
						(drawing_widget->three_d_drawing).normal_buffer.colour_map=
							attributes->colormap;
							/*???DB.  Should colour_map be in the buffer ? */
						/* find the top level window (child of parent) */
						parent_window=window;
						do
						{
							top_level_window=parent_window;
							XQueryTree(display,top_level_window,&root_window,&parent_window,
								&children,&number_of_children);
							XFree(children);
						} while (parent_window!=root_window);
						/* add the window to the list of windows that need special colour
							maps (used by the window manager) */
						if (XGetWMColormapWindows(display,top_level_window,
							&colour_map_windows,&number_of_colour_map_windows)&&
							(new_colour_map_windows=(Window *)malloc(sizeof(Window)*
							(number_of_colour_map_windows+1))))
						{
							/* a list of colour map windows already exists */
							for (i=0;i<number_of_colour_map_windows;i++)
							{
								new_colour_map_windows[i]=colour_map_windows[i];
							}
							new_colour_map_windows[i]=window;
							if (!XSetWMColormapWindows(display,top_level_window,
								new_colour_map_windows,number_of_colour_map_windows+1))
							{
			printf("ThreeDDrawingRealize.  Could not set colour map windows list\n");
							}
							XFree(colour_map_windows);
							free(new_colour_map_windows);
						}
						else
						{
							/* a list of colour map windows dose not exist */
							if (!XSetWMColormapWindows(display,top_level_window,&window,1))
							{
			printf("ThreeDDrawingRealize.  Could not set colour map windows list\n");
							}
						}
#if defined (OPENGL_API)
#if defined (OLD_CODE)
/*???DB.  glXWaitX() causes Mesa to crash.  XSync does the same thing, but is
	slower because it involves a round trip to the server (not important here) */
						glXWaitX();
#endif /* defined (OLD_CODE) */
						XSync(display,False);
						glXMakeCurrent(display,XtWindow(widget),
							((ThreeDDrawingWidget)widget)->three_d_drawing.normal_buffer.
							rendering_context);
#endif
#if defined (GL_API)
#if defined (SGI)
						/* set the window in the GL configuration */
						while (configuration_entry->buffer)
						{
							if ((GLX_NORMAL==configuration_entry->buffer)&&
								(GLX_WINDOW==configuration_entry->mode))
							{
								configuration_entry->arg=XtWindow(widget);
							}
							configuration_entry++;
						}
						/* link GL to the X window */
						if (GLWS_NOERROR==GLXlink(XtDisplay(widget),
							(drawing_widget->three_d_drawing).normal_buffer.glx_config))
						{
							/* set GL drawing to the window */
							GLXwinset(XtDisplay(widget),XtWindow(widget));
						}
						else
						{
							printf("ThreeDDrawingRealize.  Could not link GL to X\n");
						}
#endif
#if defined (IBM)
						/* convert the X window to a GL window */
/*            XSync(display,True);*/
						/*???DB.  Not sure that the GL window id needs to be remembered */
						(drawing_widget->three_d_drawing).normal_buffer.window=
							winX(display,XtWindow(widget));
							/*???DB.  winX is an IBM X extension to GL */
						if (X3dCOLOUR_RGB_MODE==
							(drawing_widget->three_d_drawing).normal_buffer.colour_mode)
						{
							/* convert the window to RGB mode */
							RGBmode();
						}
						if (X3dDOUBLE_BUFFERING==
							(drawing_widget->three_d_drawing).normal_buffer.buffering_mode)
						{
							doublebuffer();
						}
						gconfig();
/*            XSync(display,False);*/
#endif
#endif
#if defined (PEXLIB_API)
						(((ThreeDDrawingWidget)widget)->three_d_drawing).normal_buffer.
							renderer=PEXCreateRenderer(XtDisplay(widget),XtWindow(widget),0,
							&renderer_attributes);
#endif
					}
					else
					{
						printf("ThreeDDrawingRealize.  Could not create window\n");
						XFreeColormap(display,attributes->colormap);
					}
				}
				else
				{
					printf("ThreeDDrawingRealize.  Could not create colour map\n");
				}
			}
			else
			{
				printf(
		"ThreeDDrawingRealize.  Missing visual information and/or graphics info\n");
			}
#endif
#if defined (OLD_CODE)
#if defined (GL_API)
#if defined (IBM)
			if (visual_information=
				(drawing_widget->three_d_drawing).normal_buffer.visual_information)
			{
				/* create an X window for displaying GL */
				XtCreateWindow(widget,(unsigned int)InputOutput,
					visual_information->visual,*value_mask,attributes);
				/* convert the X window to a GL window */
/*        XSync(display,True);*/
				/*???DB.  Not sure that the GL window id needs to be remembered */
				(drawing_widget->three_d_drawing).normal_buffer.window=
					winX(display,(drawing_widget->core).window);
					/*???DB.  winX is an IBM X extension to GL */
				if (X3dCOLOUR_RGB_MODE==
					(drawing_widget->three_d_drawing).normal_buffer.colour_mode)
				{
					/* convert the window to RGB mode */
					RGBmode();
				}
				if (X3dDOUBLE_BUFFERING==
					(drawing_widget->three_d_drawing).normal_buffer.buffering_mode)
				{
					doublebuffer();
				}
				gconfig();
/*        XSync(display,False);*/
			}
			else
			{
				printf("ThreeDDrawingRealize.  Missing visual information\n");
				/* use the core realize instead */
				(*(coreClassRec.core_class.realize))(widget,value_mask,attributes);
			}
#endif
#if defined (SGI)
			if ((visual_information=
				(drawing_widget->three_d_drawing).normal_buffer.visual_information)&&
				(configuration_entry=
				(drawing_widget->three_d_drawing).normal_buffer.glx_config))
			{
				/* create an X window for displaying GL */
				XtCreateWindow(widget,(unsigned int)InputOutput,
					visual_information->visual,*value_mask,attributes);
				/* set the window in the GL configuration */
				while (configuration_entry->buffer)
				{
					if ((GLX_NORMAL==configuration_entry->buffer)&&
						(GLX_WINDOW==configuration_entry->mode))
					{
						configuration_entry->arg=XtWindow(widget);
					}
					configuration_entry++;
				}
				/* link GL to the X window */
				if (GLWS_NOERROR==GLXlink(XtDisplay(widget),
					(drawing_widget->three_d_drawing).normal_buffer.glx_config))
				{
					/* set GL drawing to the window */
					GLXwinset(XtDisplay(widget),XtWindow(widget));
				}
				else
				{
					printf("ThreeDDrawingRealize.  Could not link GL to X\n");
				}
			}
			else
			{
				printf(
"ThreeDDrawingRealize.  Missing visual information and/or Gl configuration\n");
				/* use the core realize instead */
				(*(coreClassRec.core_class.realize))(widget,value_mask,attributes);
			}
#endif
#if defined (CODE_FRAGMENTS)
			/* open an unmapped GL window with no border and the requested size */
			/* do not map window */
			noport();
			/* do not have a border */
			noborder();
			/* specify size */
			prefsize((drawing_widget->core).width,(drawing_widget->core).height);
			/* open GL window */
			(drawing_widget->three_d_drawing).window=winopen("");
			/* get the X window id of the GL window */
				/*???DB.  I don't know where this function comes from and suspect that
					it may be IBM specific.  gltoXwindowId ? */
			(drawing_widget->core).window=
				getXwid((drawing_widget->three_d_drawing).window);
			if (None!=(parent_window=XtWindow((drawing_widget->core).parent)))
			{
				/* reparent the GL window to be a child of the parent window.  This
					alleviates the need for visual matching */
					/*???DB.  What does this mean */
				XReparentWindow(XtDisplay(drawing_widget),(drawing_widget->core).window,
					parent_window,(drawing_widget->core).x,(drawing_widget->core).y);
			}
			/* map the widget */
			XtMapWidget(drawing_widget);
			/* set window to be the current render window */
			winset((drawing_widget->three_d_drawing).window);
#endif
#endif
#if defined (PEXLIB_API)
			if ((PEXlib_initialized)&&(visual_information=
				(drawing_widget->three_d_drawing).normal_buffer.visual_information))
			{
				/* create an X window for displaying PEX */
				XtCreateWindow(widget,(unsigned int)InputOutput,
					visual_information->visual,*value_mask,attributes);
			}
			else
			{
				printf("ThreeDDrawingRealize.  Missing PEXlib or visual information\n");
				/* use the core realize instead */
				(*(coreClassRec.core_class.realize))(widget,value_mask,attributes);
			}
#endif
#endif
#if defined (OPENGL_API)
			/* Stop this from crashing if the visual wasn't found. */
			if (window)
			{
#endif /* defined (OPENGL_API) */
				/* set up the callback data */
				call_data.reason=X3dCR_INITIALIZE;
				call_data.event=(XEvent *)NULL;
				call_data.window=XtWindow(widget);
				/*???DB.  May need something different for multiple buffers or if want
				  to pass GL window id */
				XtCallCallbackList(widget,(((ThreeDDrawingWidget)widget)->
					three_d_drawing).normal_buffer.initialize_callback,
					(XtPointer)&call_data);
#if defined (OPENGL_API)
			}
#endif /* defined (OPENGL_API) */
		}
	}
	else
	{
		printf("ThreeDDrawingRealize.  Missing widget\n");
	}
} /* ThreeDDrawingRealize */

#define SET_TO(var,type) \
if (to->addr) \
{ \
	if (to->size<sizeof(type)) \
	{ \
		to->size=sizeof(type); \
		success=False; \
	} \
	else \
	{ \
		*((type *)(to->addr))=var; \
		success=True; \
	} \
} \
else \
{ \
	to->addr=(caddr_t)&var; \
	success=True; \
}

XrmQuark X3dDOUBLE_BUFFERING_quark,X3dSINGLE_BUFFERING_quark;

static Boolean X3dCvtStringToBufferingMode(Display *display,XrmValue *args,
	Cardinal *num_args,XrmValue *from,XrmValue *to,XtPointer *converter_data)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Converts a string to a buffering mode.
???DB.  Use XtWarningMessage ?
==============================================================================*/
{
	Boolean success;
	XrmQuark string_quark;
	static X3dBufferingMode buffering_mode;

	USE_PARAMETER(args);
	USE_PARAMETER(converter_data);
	if (display&&num_args&&(0== *num_args)&&from&&to)
	{
		if (string_quark=XrmStringToQuark((char *)(from->addr)))
		{
			if (X3dSINGLE_BUFFERING_quark==string_quark)
			{
				buffering_mode=X3dSINGLE_BUFFERING;
				SET_TO(buffering_mode,X3dBufferingMode);
			}
			else
			{
				if (X3dDOUBLE_BUFFERING_quark==string_quark)
				{
					buffering_mode=X3dDOUBLE_BUFFERING;
					SET_TO(buffering_mode,X3dBufferingMode);
				}
				else
				{
					success=False;
				}
			}
		}
		else
		{
			printf(
				"X3dCvtStringToBufferColourMode.  Could not convert string to quark\n");
			success=False;
		}
	}
	else
	{
		printf("X3dCvtStringToBufferColourMode.  Invalid arguments\n");
		success=False;
	}

	return (success);
} /* X3dCvtStringToBufferColourMode */

XrmQuark X3dCOLOUR_INDEX_MODE_quark,X3dCOLOUR_RGB_MODE_quark;

static Boolean X3dCvtStringToBufferColourMode(Display *display,XrmValuePtr args,
	Cardinal *num_args,XrmValuePtr from,XrmValuePtr to,XtPointer *converter_data)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Converts a string to a buffer colour mode.
???DB.  Use XtWarningMessage ?
==============================================================================*/
{
	Boolean success;
	XrmQuark string_quark;
	static X3dBufferColourMode buffer_colour_mode;

	USE_PARAMETER(args);
	USE_PARAMETER(converter_data);
	if (display&&num_args&&(0== *num_args)&&from&&to)
	{
		if (string_quark=XrmStringToQuark((char *)(from->addr)))
		{
			if (X3dCOLOUR_INDEX_MODE_quark==string_quark)
			{
				buffer_colour_mode=X3dCOLOUR_INDEX_MODE;
				SET_TO(buffer_colour_mode,X3dBufferColourMode);
			}
			else
			{
				if (X3dCOLOUR_RGB_MODE_quark==string_quark)
				{
					buffer_colour_mode=X3dCOLOUR_RGB_MODE;
					SET_TO(buffer_colour_mode,X3dBufferColourMode);
				}
				else
				{
					success=False;
				}
			}
		}
		else
		{
			printf(
				"X3dCvtStringToBufferColourMode.  Could not convert string to quark\n");
			success=False;
		}
	}
	else
	{
		printf("X3dCvtStringToBufferColourMode.  Invalid arguments\n");
		success=False;
	}

	return (success);
} /* X3dCvtStringToBufferColourMode */

static void ThreeDDrawingClassInitialize(void)
/*******************************************************************************
LAST MODIFIED : 29 April 1994

DESCRIPTION :
The class initialize method.
==============================================================================*/
{
	/* register type converters */
	X3dSINGLE_BUFFERING_quark=XrmStringToQuark("X3dSINGLE_BUFFERING");
	X3dDOUBLE_BUFFERING_quark=XrmStringToQuark("X3dDOUBLE_BUFFERING");
	XtSetTypeConverter(XtRString,X3dRBufferingMode,X3dCvtStringToBufferingMode,
		(XtConvertArgList)NULL,0,XtCacheNone,NULL);
	X3dCOLOUR_INDEX_MODE_quark=XrmStringToQuark("X3dCOLOUR_INDEX_MODE");
	X3dCOLOUR_RGB_MODE_quark=XrmStringToQuark("X3dCOLOUR_RGB_MODE");
	XtSetTypeConverter(XtRString,X3dRBufferColourMode,
		X3dCvtStringToBufferColourMode,(XtConvertArgList)NULL,0,XtCacheNone,NULL);
} /* ThreeDDrawingClassInitialize */

static void ThreeDDrawingInitialize(Widget request_widget,Widget new_widget,
	ArgList args,Cardinal *num_args)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
The initialize method.
???DB.  GlibInitialize copies some core values, but this doesn't need to be done
because the initialize method is downward chained.
???DB.  Need to copy values for "new" values ?
==============================================================================*/
{
	Display *display;
	int screen_number;
	ThreeDDrawingWidget request,new;
#if defined (OPENGL_API)
	Bool direct_rendering;
	int best_buffer_size,best_depth_size,error_base,event_base,i,
		major_version_number,minor_version_number,number_of_valid_visual_infos,
		number_of_visual_infos,value;
	XVisualInfo *best_visual_info,**valid_visual_info_list,*visual_info,
		*visual_info_list,visual_info_template;
#if defined (OLD_CODE)
	int configuration[3];
#endif
#endif
#if defined (GL_API)
	int number_of_matching_visual_structures;
	XVisualInfo visual_info_template;
#if defined (IBM)
	int i;
	XVisualInfo *best_visual_info,*visual_info,*visual_info_list;
#endif
#if defined (SGI)
	int i;
	GLXconfig *new_configuration,request_configuration[3];
#endif
#endif
#if defined (PEXLIB_API)
	int error_base,event_base,major_version_number,
		number_of_matching_visual_structures;
	XVisualInfo visual_info_template;
#if defined (IBM)
	PEXExtensionInfo *extension_info;
#endif
#if defined (VAX)
	pxlInfo *extension_info;
#endif
#endif

	USE_PARAMETER(args);
	USE_PARAMETER(num_args);
	if ((request=(ThreeDDrawingWidget)request_widget)&&
		(new=(ThreeDDrawingWidget)new_widget))
	{
		display=XtDisplay(new_widget);
		screen_number=XScreenNumberOfScreen(XtScreen(new_widget));
		(new->three_d_drawing).normal_buffer.colour_mode=
			(request->three_d_drawing).normal_buffer.colour_mode;
		(new->three_d_drawing).normal_buffer.buffering_mode=
			(request->three_d_drawing).normal_buffer.buffering_mode;
		/* get visual information */
#if defined (OPENGL_API)
		/* check the existence of the GLX server extension */
		if (True==glXQueryExtension(display,&error_base,&event_base))
		{
			/* find  version numbers for the GLX extension */
			if (True==glXQueryVersion(display,&major_version_number,
				&minor_version_number))
			{
				if (!glx_major_version)
				{
					glx_major_version = major_version_number;
					glx_minor_version = minor_version_number;
					/* only want to print this once */
					printf("GLX version = %d.%d\n",
						major_version_number,minor_version_number);
				}
			}
			visual_info_template.screen=screen_number;
			if (visual_info_list=XGetVisualInfo(display,VisualScreenMask,
				&visual_info_template,&number_of_visual_infos))
			{
				if (valid_visual_info_list=(XVisualInfo **)
					malloc(number_of_visual_infos*sizeof(XVisualInfo *)))
				{
					/* choose the visuals with appropriate buffering */
					number_of_valid_visual_infos=0;
					visual_info=visual_info_list;
					for (i=number_of_visual_infos;i>0;i--)
					{
						glXGetConfig(display,visual_info,GLX_USE_GL,&value);
						/* only use read only visuals */
						if (value
#if defined (USING_MESA)
							&&((PseudoColor==visual_info->class)||
							(TrueColor==visual_info->class))
#endif /* defined (USING_MESA) */
							)
						{
							glXGetConfig(display,visual_info,GLX_DOUBLEBUFFER,&value);
							if ((value&&(X3dDOUBLE_BUFFERING==
								(request->three_d_drawing).normal_buffer.buffering_mode))||
								(!value&&(X3dSINGLE_BUFFERING==
									(request->three_d_drawing).normal_buffer.buffering_mode)))
							{
								valid_visual_info_list[number_of_valid_visual_infos]=
									visual_info;
								number_of_valid_visual_infos++;
							}
						}
						visual_info++;
					}
					if (0<number_of_valid_visual_infos)
					{
						/* choose the visual with the best colour mode */
						best_visual_info=(XVisualInfo *)NULL;
						best_buffer_size=0;
						/* if the widget has been told a required visual use that */
						if ((request->three_d_drawing).normal_buffer.visual_id)
						{
							for (i=0;i<number_of_valid_visual_infos;i++)
							{
								if (valid_visual_info_list[i]->visualid == 
 									(request->three_d_drawing).normal_buffer.visual_id)
								{
									best_visual_info=valid_visual_info_list[i];
									glXGetConfig(display, best_visual_info,
										GLX_BUFFER_SIZE, &best_buffer_size);
									glXGetConfig(display, best_visual_info,
										GLX_DEPTH_SIZE, &best_depth_size);
								}
							}							
						}
						else
						{	
							for (i=0;i<number_of_valid_visual_infos;i++)
							{
#if defined (DEBUG)
								{
									printf("Selecting visual, id %d\n",
										valid_visual_info_list[i]->visualid);
									glXGetConfig(display,valid_visual_info_list[i],GLX_RGBA,
										&value);
									printf("RGB %d\n", value);
									glXGetConfig(display,valid_visual_info_list[i],
										GLX_BUFFER_SIZE,&value);
									printf("buffer %d\n", value);
									glXGetConfig(display,valid_visual_info_list[i],
										GLX_DEPTH_SIZE, &value);
									printf("depth %d\n\n", value);
								}
#endif /* defined (DEBUG) */

								glXGetConfig(display,valid_visual_info_list[i],GLX_RGBA,&value);
								if ((value&&(X3dCOLOUR_RGB_MODE==
										  (request->three_d_drawing).normal_buffer.colour_mode))||
									(!value&&(X3dCOLOUR_INDEX_MODE==
										(request->three_d_drawing).normal_buffer.colour_mode)))
								{
									if (best_visual_info)
									{
										glXGetConfig(display,valid_visual_info_list[i],
											GLX_BUFFER_SIZE,&value);
										if (value>best_buffer_size)
										{
											best_visual_info=valid_visual_info_list[i];
											best_buffer_size=value;
											glXGetConfig(display,best_visual_info,GLX_DEPTH_SIZE,
												&best_depth_size);
										}
										else
										{
											if (value==best_buffer_size)
											{
												glXGetConfig(display,valid_visual_info_list[i],
													GLX_DEPTH_SIZE, &value);
												if (value>best_depth_size)
												{
													best_visual_info=valid_visual_info_list[i];
													best_depth_size=value;
												}
											}
										}
									}
									else
									{
										best_visual_info=valid_visual_info_list[i];
										glXGetConfig(display, best_visual_info,
											GLX_BUFFER_SIZE, &best_buffer_size);
										glXGetConfig(display, best_visual_info,
											GLX_DEPTH_SIZE, &best_depth_size);
									}
								}
							}
						}
						if (best_visual_info && ((new->three_d_drawing).normal_buffer.visual_information=
							XGetVisualInfo(display,VisualAllMask,best_visual_info,
							  &number_of_visual_infos)))
						{
							/*???debug */
							/* only want to print this once, unless using different visuals */
							if (!shareable_context)
							{
								printf("openGL visual id = %lu\n",(new->three_d_drawing)
									.normal_buffer.visual_information->visualid);
							}
							/* create the OpenGL rendering context */
							direct_rendering=GL_TRUE;
							if ((new->three_d_drawing).normal_buffer.rendering_context=
								glXCreateContext(display,(new->three_d_drawing).normal_buffer.
								visual_information,shareable_context,direct_rendering))
							{
								if (!shareable_context)
								{
									shareable_context=
										(new->three_d_drawing).normal_buffer.rendering_context;
								}
							}
							else
							{
								printf(
							"ThreeDDrawingInitialize.  Could not create rendering context\n");
							}
						}
						else
						{
							printf(
						"ThreeDDrawingInitialize.  No visuals with required colour mode\n");
						}
					}
					else
					{
						printf(
				"ThreeDDrawingInitialize.  No visuals with required buffering mode\n");
					}
					free(valid_visual_info_list);
				}
				else
				{
					printf(
			"ThreeDDrawingInitialize.  Insufficient memory for valid visual list\n");
				}
				XFree(visual_info_list);
			}
			else
			{
				printf("ThreeDDrawingInitialize.  Could not find visual\n");
			}
#if defined (OLD_CODE)
			/* construct the desired OpenGL configuration */
			i=0;
			if (X3dCOLOUR_RGB_MODE==
				(request->three_d_drawing).normal_buffer.colour_mode)
			{
				configuration[i]=GLX_RGBA;
				i++;
			}
			if (X3dDOUBLE_BUFFERING==
				(request->three_d_drawing).normal_buffer.buffering_mode)
			{
				configuration[i]=GLX_DOUBLEBUFFER;
				i++;
			}
			configuration[i]=None;
			/* retrieve the visual information */
			if ((new->three_d_drawing).normal_buffer.visual_information=
				glXChooseVisual(display,screen_number,configuration))
			{
/*???debug */
printf("openGL visual id = %d\n",
	(new->three_d_drawing).normal_buffer.visual_information->visualid);
				/* create the OpenGL rendering context */
				share_context=(GLXContext)NULL;
				direct_rendering=GL_TRUE;
				if (!((new->three_d_drawing).normal_buffer.rendering_context=
					glXCreateContext(display,(new->three_d_drawing).normal_buffer.
					visual_information,share_context,direct_rendering)))
				{
					printf(
						"ThreeDDrawingInitialize.  Could not create rendering context\n");
				}
			}
			else
			{
				printf("ThreeDDrawingInitialize.  Could not find visual\n");
			}
#endif
		}
		else
		{
			printf("ThreeDDrawingInitialize.  Missing GLX server extension\n");
		}
#endif
#if defined (GL_API)
#if defined (IBM)
		visual_info_template.screen=screen_number;
		visual_info_template.class=PseudoColor;
			/*???DB.  PseudoColor ? */
		if (visual_info_list=XGetVisualInfo(display,
			VisualScreenMask|VisualClassMask,&visual_info_template,
			&number_of_matching_visual_structures))
		{
			/* find the largest visual */
			visual_info=visual_info_list;
			best_visual_info=visual_info;
/*???debug */
printf("visual=%d, depth=%d, colourmap size=%d\n",visual_info->visualid,
	visual_info->depth,visual_info->colormap_size);
			for (i=number_of_matching_visual_structures-1;i>0;i--)
			{
				visual_info++;
/*???debug */
printf("visual=%d, depth=%d, colourmap size=%d\n",visual_info->visualid,
	visual_info->depth,visual_info->colormap_size);
/*        if (visual_info->colormap_size>best_visual_info->colormap_size)
				{
					best_visual_info=visual_info;
				}*/
				/*???DB.  I don't know how to get the colour map if I don't use the
					first ("default") one */
			}
			(new->three_d_drawing).normal_buffer.visual_information=best_visual_info;
			(new->core).depth=best_visual_info->depth;
			/*???DB.  Is this right ? */
			(new->core).colormap=(Colormap)NULL;
		}
		else
		{
			printf(
	"ThreeDDrawingInitialize.  Could not find a visual for %d bit PseudoColor\n",
				DefaultDepth(display,screen_number));
		}
#if defined (OLD_CODE)
/*???DB.  Need to do better */
		visual_info_template.screen=screen_number;
		visual_info_template.depth=DefaultDepth(display,screen_number);
		visual_info_template.class=PseudoColor;
			/*???DB.  PseudoColor ? */
		if ((new->three_d_drawing).normal_buffer.visual_information=
			XGetVisualInfo(display,VisualScreenMask|VisualDepthMask|VisualClassMask,
			&visual_info_template,&number_of_matching_visual_structures))
		{
			(new->core).depth=
				((new->three_d_drawing).normal_buffer.visual_information)->depth;
			/*???DB.  Is this right ? */
			(new->core).colormap=(Colormap)NULL;
		}
		else
		{
			printf(
	"ThreeDDrawingInitialize.  Could not find a visual for %d bit PseudoColor\n",
				DefaultDepth(display,screen_number));
		}
#endif
#endif
#if defined (SGI)
		i=0;
		if (X3dCOLOUR_RGB_MODE==
			(request->three_d_drawing).normal_buffer.colour_mode)
		{
			request_configuration[i].buffer=GLX_NORMAL;
			request_configuration[i].mode=GLX_RGB;
			request_configuration[i].arg=True;
			i++;
		}
		if (X3dDOUBLE_BUFFERING==
			(request->three_d_drawing).normal_buffer.buffering_mode)
		{
			request_configuration[i].buffer=GLX_NORMAL;
			request_configuration[i].mode=GLX_DOUBLE;
			request_configuration[i].arg=True;
			i++;
		}
		request_configuration[i].buffer=0;
		request_configuration[i].mode=0;
		request_configuration[i].arg=0;
		if (new_configuration=GLXgetconfig(display,screen_number,
			request_configuration))
		{
			(new->three_d_drawing).normal_buffer.glx_config=new_configuration;
			while (new_configuration->buffer)
			{
				switch (new_configuration->buffer)
				{
					case GLX_NORMAL:
					{
						switch (new_configuration->mode)
						{
							case GLX_DOUBLE:
							{
								if (True==new_configuration->arg)
								{
									(request->three_d_drawing).normal_buffer.buffering_mode=
										X3dDOUBLE_BUFFERING;
								}
								else
								{
									(request->three_d_drawing).normal_buffer.buffering_mode=
										X3dSINGLE_BUFFERING;
								}
							} break;
							case GLX_RGB:
							{
								if (True==new_configuration->arg)
								{
									(request->three_d_drawing).normal_buffer.colour_mode=
										X3dCOLOUR_RGB_MODE;
								}
								else
								{
									(request->three_d_drawing).normal_buffer.colour_mode=
										X3dCOLOUR_INDEX_MODE;
								}
							} break;
							case GLX_ACSIZE:
							case GLX_BUFSIZE:
							case GLX_MSSAMPLE:
							case GLX_MSSSIZE:
							case GLX_MSZSIZE:
							case GLX_RGBSIZE:
							case GLX_STENSIZE:
							case GLX_STEREOBUF:
							case GLX_ZSIZE:
							{
								/*???DB.  Do nothing at present */
							} break;
							case GLX_VISUAL:
							{
								visual_info_template.visualid=new_configuration->arg;
								visual_info_template.screen=screen_number;
								if ((new->three_d_drawing).normal_buffer.visual_information=
									XGetVisualInfo(display,VisualScreenMask|VisualIDMask,
									&visual_info_template,&number_of_matching_visual_structures))
								{
/*???debug */
printf("GL visual id = %d\n",
	(new->three_d_drawing).normal_buffer.visual_information->visualid);
									(new->core).depth=((new->three_d_drawing).normal_buffer.
										visual_information)->depth;
								}
								else
								{
									printf(
								"ThreeDDrawingInitialize.  Could not get visual information\n");
								}
							} break;
							case GLX_COLORMAP:
							{
								/*???DB.  Looked after by GL.  Should NOT use XStoreColors on
									it.  Can use for creating windows.  Can use a different
									colour map (have to create one). */
								(new->three_d_drawing).normal_buffer.colour_map=
									new_configuration->arg;
								(new->core).colormap=new_configuration->arg;
							} break;
							case GLX_WINDOW:
							{
								if (GLX_NONE!=new_configuration->arg)
								{
									printf(
										"ThreeDDrawingInitialize.  Invalid window id returned\n");
								}
							} break;
							default:
							{
								printf("ThreeDDrawingInitialize.  Invalid mode %d\n",
									new_configuration->mode);
							} break;
						}
					} break;
					case GLX_POPUP:
					case GLX_OVERLAY:
					case GLX_UNDERLAY:
					{
						/*???DB.  Do nothing at present */
					} break;
					default:
					{
						printf("ThreeDDrawingInitialize.  Invalid buffer\n");
					} break;
				}
				new_configuration++;
			}
		}
		else
		{
			printf(
				"ThreeDDrawingInitialize.  Requested configuration not available\n");
		}
#endif
#endif
#if defined (PEXLIB_API)
		if (!PEXlib_initialized)
		{
			/* check the existence of the PEX server extension */
			if (True==XQueryExtension(display,PEX_NAME_STRING,&major_version_number,
				&event_base,&error_base))
			{
				/* initialize PEXlib */
#if defined (IBM)
				if (True==PEXInitialize(display))
#endif
#if defined (VAX)
				if (True==PEXInitialize(display,extension_info))
#endif
				{
					PEXlib_initialized=1;
#if defined (IBM)
					if (extension_info=PEXGetExtensionInfo(display))
					{
						printf("PEXlib\n");
						printf(
							"  major version = %d, minor version = %d, release = %d\n",
							extension_info->major_version,extension_info->minor_version,
							extension_info->release);
						printf("  vendor name = %s\n",extension_info->vendor_name);
						if (PEXCompleteImplementation==extension_info->subset_info)
						{
							printf("  complete implementation\n");
						}
						else
						{
							if (PEXImmediateMode&(extension_info->subset_info))
							{
								printf("  immediate mode");
							}
							if (PEXStructureMode&(extension_info->subset_info))
							{
								printf("  structure mode");
							}
							if (PEXWorkstationOnly&(extension_info->subset_info))
							{
								printf("  workstation only");
							}
							printf("\n");
						}
					}
#endif
#if defined (VAX)
					if (extension_info)
					{
						printf("PEXlib\n");
						printf(
							"  major version = %d, minor version = %d, release = %d\n",
							extension_info->majorVersion,extension_info->minorVersion,
							extension_info->release);
						printf("  vendor name = %s\n",extension_info->vendorName);
						if (0==extension_info->subsetInfo)
						{
							printf("  complete implementation\n");
						}
						else
						{
							if (0x1&(extension_info->subsetInfo))
							{
								printf("  immediate mode");
							}
							if (0x2&(extension_info->subsetInfo))
							{
								printf("  workstation only");
							}
							printf("\n");
							PEXFree(extension_info);
						}
					}
#endif
				}
				else
				{
					printf("ThreeDDrawingInitialize.  Could not initialize PEXlib\n");
				}
			}
			else
			{
				printf("ThreeDDrawingInitialize.  Missing PEX server extension\n");
			}
		}
		if (PEXlib_initialized)
		{
			/*???DB.  Same as IBM GL */
/*???DB.  Need to do better */
			visual_info_template.screen=screen_number;
			visual_info_template.depth=DefaultDepth(display,screen_number);
			visual_info_template.class=PseudoColor;
				/*???DB.  PseudoColor ? */
			if ((new->three_d_drawing).normal_buffer.visual_information=
				XGetVisualInfo(display,VisualScreenMask|VisualDepthMask|VisualClassMask,
				&visual_info_template,&number_of_matching_visual_structures))
			{
				(new->core).depth=
					((new->three_d_drawing).normal_buffer.visual_information)->depth;
				/*???DB.  Is this right ? */
				(new->core).colormap=(Colormap)NULL;
			}
			else
			{
				printf(
	"ThreeDDrawingInitialize.  Could not find a visual for %d bit PseudoColor\n",
					DefaultDepth(display,screen_number));
			}
		}
		else
		{
			(new->three_d_drawing).normal_buffer.visual_information=
				(XVisualInfo *)NULL;
		}
#endif
		/*???DB.  Is this needed ? */
#if defined (GL_API)
#if defined (IBM)
		(new->three_d_drawing).normal_buffer.window=0;
#endif
#endif
#if defined (PEXLIB_API)
#if defined (IBM)
		(new->three_d_drawing).normal_buffer.pipeline_context=
			(PEXPipelineContext)NULL;
		(new->three_d_drawing).normal_buffer.renderer=(PEXRenderer)NULL;
#endif
#if defined (VAX)
		(new->three_d_drawing).normal_buffer.pipeline_context=
			(pexPipelineContext)NULL;
		(new->three_d_drawing).normal_buffer.renderer=(pexRenderer)NULL;
#endif
#endif
	}
	else
	{
		printf("ThreeDDrawingInitialize.  Invalid argument(s)\n");
	}
} /* ThreeDDrawingInitialize */

static void ThreeDDrawingExpose(Widget widget,XEvent *event,Region region)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
The expose method.
???DB.  Glib has its own expose callback structure, but I won't bother at
present.
==============================================================================*/
{
	Widget old_current_ThreeDDrawing;
	Window window;
	X3dThreeDDrawCallbackStruct call_data;
#if defined (OPENGL_API)
	Dimension height,width;
#endif

	USE_PARAMETER(region);
	if (window=XtWindow(widget))
	{
#if defined (GL_API)
#if defined (IBM)
		winset(((ThreeDDrawingWidget)widget)->three_d_drawing.normal_buffer.window);
#endif
#if defined (SGI)
		GLXwinset(XtDisplay(widget),window);
#endif
		reshapeviewport();
#endif
#if defined (OPENGL_API)
		glXMakeCurrent(XtDisplay(widget),window,((ThreeDDrawingWidget)widget)->
			three_d_drawing.normal_buffer.rendering_context);
		XtVaGetValues(widget,
			XtNheight,&height,
			XtNwidth,&width,
			NULL);
		glViewport(0,0,(GLint)width,(GLint)height);
#endif
		/* SAB Keep the current_ThreeDDrawing and change it temporarily
			Unsure about the necessity to change the current context
			back to the current_ThreeDDrawing.  There may be some conflict 
			when different types of ThreeDDrawing are active and getting
			exposed but we aren't sure what.  Anyway, 
			X3dThreeDDrawingAddReadContext requires the ThreeDDrawing to be
			defined so to ensure compile texture works with a PBuffer I set
			it here and change it back after the callbacks. */
		old_current_ThreeDDrawing = current_ThreeDDrawing;
		current_ThreeDDrawing=widget;
	}
	/* set up the callback data */
	call_data.reason=X3dCR_EXPOSE;
	call_data.event=event;
	call_data.window=XtWindow(widget);
		/*???DB.  May need something different for multiple buffers or if want to
			pass GL window id */
	XtCallCallbackList(widget,(((ThreeDDrawingWidget)widget)->three_d_drawing).
		normal_buffer.expose_callback,(XtPointer)&call_data);
	if (window)
	{ 
		/* SAB Change the current_ThreeDDrawing back */
		current_ThreeDDrawing = old_current_ThreeDDrawing;
#if defined (GL_API)
		swapbuffers();
#endif
#if defined (OPENGL_API)
		/*???RC Shouldn't swap buffers here!*/
		/*glXSwapBuffers(XtDisplay(widget),window);*/
		if (current_ThreeDDrawing&&(widget!=current_ThreeDDrawing))
		{
			glXMakeCurrent(XtDisplay(current_ThreeDDrawing),
				XtWindow(current_ThreeDDrawing),
				((ThreeDDrawingWidget)current_ThreeDDrawing)->three_d_drawing.
				normal_buffer.rendering_context);
		}
#endif
	}
} /* ThreeDDrawingExpose */

static Boolean ThreeDDrawingSetValues(Widget old,Widget current,Widget request,
	ArgList arguments,Cardinal *number_of_arguments)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
The expose method.
???DB.  Glib has its own expose callback structure, but I won't bother at
present.
==============================================================================*/
{
	Boolean success;

	USE_PARAMETER(old);
	USE_PARAMETER(current);
	USE_PARAMETER(request);
	USE_PARAMETER(arguments);
	USE_PARAMETER(number_of_arguments);
	success=True;

	return (success);
} /* ThreeDDrawingSetValues */

#if defined (OPENGL_API)
GLXContext ThreeDDrawing_get_shareable_context(void)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Exports the shareable context for the digital media extensions.
==============================================================================*/
{
	return(shareable_context);
} /* ThreeDDrawing_get_shareable_context */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int ThreeDDrawing_get_glx_version(int *glx_major_version_number, 
	int *glx_minor_version_number)
/*******************************************************************************
LAST MODIFIED : 12 December 2001

DESCRIPTION :
Exports the glx version numbers.
==============================================================================*/
{
	int return_code;

	if (glx_major_version)
	{
 		*glx_major_version_number = glx_major_version;
 		*glx_minor_version_number = glx_minor_version;
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	return(return_code);
} /* ThreeDDrawing_get_glx_version */
#endif /* defined (OPENGL_API) */

static void ThreeDDrawingResize(Widget widget)
/*******************************************************************************
LAST MODIFIED : 11 February 1995

DESCRIPTION :
The resize method.
==============================================================================*/
{
	Widget old_current_ThreeDDrawing;
	Window window;
	X3dThreeDDrawCallbackStruct call_data;
#if defined (OPENGL_API)
	Dimension height,width;
#endif

	if (window=XtWindow(widget))
	{
#if defined (GL_API)
#if defined (IBM)
		winset(((ThreeDDrawingWidget)widget)->three_d_drawing.normal_buffer.window);
#endif
#if defined (SGI)
		GLXwinset(XtDisplay(widget),window);
#endif
		reshapeviewport();
#endif
#if defined (OPENGL_API)
		glXMakeCurrent(XtDisplay(widget),window,((ThreeDDrawingWidget)widget)->
			three_d_drawing.normal_buffer.rendering_context);
		XtVaGetValues(widget,
			XtNheight,&height,
			XtNwidth,&width,
			NULL);
		glViewport(0,0,(GLint)width,(GLint)height);
#endif
		/* SAB Keep the current_ThreeDDrawing and change it temporarily
			Unsure about the necessity to change the current context
			back to the current_ThreeDDrawing.  There may be some conflict 
			when different types of ThreeDDrawing are active and getting
			exposed but we aren't sure what.  Anyway, 
			X3dThreeDDrawingAddReadContext requires the ThreeDDrawing to be
			defined so to ensure compile texture works with a PBuffer I set
			it here and change it back after the callbacks. */
		old_current_ThreeDDrawing = current_ThreeDDrawing;
		current_ThreeDDrawing=widget;
	}
	/* set up the callback data */
	call_data.reason=X3dCR_RESIZE;
	call_data.event=(XEvent *)NULL;
	call_data.window=window;
		/*???DB.  May need something different for multiple buffers or if want to
			pass GL window id */
	XtCallCallbackList(widget,
		(((ThreeDDrawingWidget)widget)->three_d_drawing).resize_callback,
		(XtPointer)&call_data);
	if (window)
	{
		/* SAB Change the current_ThreeDDrawing back */
		current_ThreeDDrawing = old_current_ThreeDDrawing;
#if defined (GL_API)
		swapbuffers();
#endif
#if defined (OPENGL_API)
		/*???RC Shouldn't swap buffers here!*/
		/*glXSwapBuffers(XtDisplay(widget),window);*/
		if (current_ThreeDDrawing&&(widget!=current_ThreeDDrawing))
		{
			glXMakeCurrent(XtDisplay(current_ThreeDDrawing),
				XtWindow(current_ThreeDDrawing),
				((ThreeDDrawingWidget)current_ThreeDDrawing)->three_d_drawing.
				normal_buffer.rendering_context);
		}
#endif
	}
} /* ThreeDDrawingResize */

/*???DB.  static XtGeometryResult ThreeDDrawingQueryGeometry();*/

static void ThreeDDrawingDestroy(Widget widget)
/*******************************************************************************
LAST MODIFIED : 17 November 1997

DESCRIPTION :
The destroy method.
==============================================================================*/
{
#if defined (OPENGL_API)
	Display *display;
	ThreeDDrawingWidget drawing_widget;
#endif

#if defined (OPENGL_API)
	if ((display=XtDisplay(widget))&&(drawing_widget=(ThreeDDrawingWidget)widget))
	{
		if (current_ThreeDDrawing==widget)
		{
			current_ThreeDDrawing=(Widget)NULL;
		}
		/* destroy the OpenGL rendering context - unless it is the shared one! */
		/* X3dThreeDDrawingCleanUp is set up for destroying shareable_context */
		if ((drawing_widget->three_d_drawing).normal_buffer.rendering_context !=
			shareable_context)
		{
			glXDestroyContext(display,
				(drawing_widget->three_d_drawing).normal_buffer.rendering_context);
		}
		/* free the visual information */
		XFree((drawing_widget->three_d_drawing).normal_buffer.visual_information);
		/*???DB.  Free the colour map */
	}
#endif
#if defined (PEXLIB_API)
	/*???DB.  Free pipeline context and renderer */
	/*???DB.  Check */
#if defined (VAX)
	PEXTerminate(XtDisplay(widget),(XExtCodes *)NULL);
#endif
#endif
} /* ThreeDDrawingDestroy */

static void threeDDrawingInput(Widget widget,XEvent *event,String *parameters,
	Cardinal *number_of_parameters)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Called in response to input.
==============================================================================*/
{
	X3dThreeDDrawCallbackStruct call_data;

	USE_PARAMETER(parameters);
	USE_PARAMETER(number_of_parameters);
	call_data.reason=X3dCR_INPUT;
	call_data.event=event;
	call_data.window=XtWindow(widget);
		/*???DB.  May need something different for multiple buffers or if want to
			pass GL window id */
	XtCallCallbackList(widget,
		(((ThreeDDrawingWidget)widget)->three_d_drawing).input_callback,
		(XtPointer)&call_data);
} /* threeDDrawingInput */

#if defined (INSTALL_COLOURMAP)
static void threeDDrawingInstallColourmap(Widget widget,XEvent *event,
	String *parameters,Cardinal *number_of_parameters)
/*******************************************************************************
LAST MODIFIED : 11 February 1995

DESCRIPTION :
Called in response to pointer entering window.
==============================================================================*/
{
	USE_PARAMETER(event);
	USE_PARAMETER(parameters);
	USE_PARAMETER(number_of_parameters);
	XInstallColormap(XtDisplay(widget),
		(((ThreeDDrawingWidget)widget)->three_d_drawing).normal_buffer.colour_map);
} /* threeDDrawingInstallColourmap */

static void threeDDrawingUninstallColourmap(Widget widget,XEvent *event,
	String *parameters,Cardinal *number_of_parameters)
/*******************************************************************************
LAST MODIFIED : 11 February 1995

DESCRIPTION :
Called in response to pointer entering window.
==============================================================================*/
{
	USE_PARAMETER(event);
	USE_PARAMETER(parameters);
	USE_PARAMETER(number_of_parameters);
	XUninstallColormap(XtDisplay(widget),
		(((ThreeDDrawingWidget)widget)->three_d_drawing).normal_buffer.colour_map);
} /* threeDDrawingInstallColourmap */
#endif /* defined (INSTALL_COLOURMAP) */

/*
Actions
-------
*/
/*???DB.  Think more.  Spaceball ? */
static char defaultTranslations[]=
#if defined (INSTALL_COLOURMAP)
	"<BtnDown>: threeDDrawingInput() \n\
	<BtnMotion>: threeDDrawingInput() \n\
	<BtnUp>: threeDDrawingInput() \n\
	<KeyDown>: threeDDrawingInput() \n\
	<KeyUp>: threeDDrawingInput() \n\
	<Enter>: threeDDrawingInstallColourmap() \n\
	<Leave>: threeDDrawingUninstallColourmap()";
#else /* defined (INSTALL_COLOURMAP) */
	"<BtnDown>: threeDDrawingInput() \n\
	<BtnMotion>: threeDDrawingInput() \n\
	<BtnUp>: threeDDrawingInput() \n\
	<KeyDown>: threeDDrawingInput() \n\
	<KeyUp>: threeDDrawingInput()";
#endif /* defined (INSTALL_COLOURMAP) */

static XtActionsRec actions[]=
{
	{"threeDDrawingInput",threeDDrawingInput},
#if defined (INSTALL_COLOURMAP)
	{"threeDDrawingInstallColourmap",threeDDrawingInstallColourmap},
	{"threeDDrawingUninstallColourmap",threeDDrawingUninstallColourmap},
#endif /* defined (INSTALL_COLOURMAP) */
}; /* actions */

/*
Class record
------------
*/
ThreeDDrawingClassRec threeDDrawingClassRec=
/*******************************************************************************
LAST MODIFIED : 28 April 1994

DESCRIPTION :
The 3-D drawing widget class structure.
==============================================================================*/
{
	/* core_class part */
	{
		/* superclass */ (WidgetClass)&compositeClassRec,
		/* class_name */ "ThreeDDrawing",
		/* widget_size */ sizeof(ThreeDDrawingRec),
		/* class_initialize */ ThreeDDrawingClassInitialize,
		/* class_part_initialize */ NULL,
		/* class_inited */ False,
		/* initialize */ ThreeDDrawingInitialize,
		/* initialize_hook */ NULL,
		/* realize */ ThreeDDrawingRealize,
		/* actions */ actions,
		/* num_actions */ XtNumber(actions),
		/* resources */ resources,
		/* num_resources */ XtNumber(resources),
		/* xrm_class */ NULLQUARK,
		/* compress_motion */ True,
		/* compress_exposure */ True,
		/* compress_enterleave */ True,
		/* visible_interest */ False,
		/* destroy */ ThreeDDrawingDestroy,
		/* resize */ ThreeDDrawingResize,
		/* expose */ ThreeDDrawingExpose,
		/* set_values */ ThreeDDrawingSetValues,
		/* set_values_hook */ NULL,
		/* set_values_almost */ XtInheritSetValuesAlmost,
		/* get_values_hook */ NULL,
		/* accept_focus */ NULL,
		/* version */ XtVersion,
		/* callback_private */ NULL,
		/* tm_table */ defaultTranslations,
		/* query_geometry */ /*???DB.  ThreeDDrawingQueryGeometry */ NULL,
		/* display_accelerator */ NULL,
		/* extension */ NULL
	},
	/* composite_class part */
	{
		/* geometry_manager */ XtInheritGeometryManager,
		/* changed_managed */ XtInheritChangeManaged,
		/* insert_child */ XtInheritInsertChild,
		/* delete_child */ XtInheritDeleteChild,
		/* extension */ NULL
	},
	/* 3-D drawing widget part */
	{
		/* dummy */ 0
	},
}; /* threeDDrawingClassRec */

WidgetClass threeDDrawingWidgetClass=(WidgetClass)&threeDDrawingClassRec;

/*
Global functions
----------------
*/
int X3dThreeDisInitialised(Widget widget)
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns true if the X3dThreeD <widget> is initialised correctly.  This enables 
us to fail nicely if the Initialise routine was unable to complete properly, 
i.e. it couldn't create a valid rendering context.
==============================================================================*/
{
	int return_code;
	ThreeDDrawingWidget drawing_widget;

	return_code = 0;
	if ((drawing_widget=(ThreeDDrawingWidget)widget)&&
		(True==IsThreeDDrawing(widget)))
	{
#if defined (OPENGL_API)
		if ((drawing_widget->three_d_drawing).normal_buffer.rendering_context)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
#endif
	}
	else
	{
		printf("X3dThreeDDrawingMakeCurrent.  Missing or invalid widget\n");
		return_code = 0;
	}

	return (return_code);
} /* X3dThreeDisInitialised */

void X3dThreeDDrawingMakeCurrent(Widget widget)
/*******************************************************************************
LAST MODIFIED : 30 April 1997

DESCRIPTION :
Makes the widget the current one being drawn to.  Have to set viewport because
there may be multiple GL windows.
==============================================================================*/
{
	ThreeDDrawingWidget drawing_widget;
#if defined (OPENGL_API)
	Dimension height,width;
#endif

	/* check arguments */
	if ((drawing_widget=(ThreeDDrawingWidget)widget)&&
		(True==IsThreeDDrawing(widget)))
	{
#if defined (GL_API)
#if defined (IBM)
		winset((drawing_widget)->three_d_drawing.normal_buffer.window);
#endif
#if defined (SGI)
		GLXwinset(XtDisplay(widget),XtWindow(widget));
#endif
		reshapeviewport();
#endif
#if defined (OPENGL_API)
		glXMakeCurrent(XtDisplay(widget),XtWindow(widget),
			(drawing_widget)->three_d_drawing.normal_buffer.rendering_context);
		XtVaGetValues(widget,
			XtNheight,&height,
			XtNwidth,&width,
			NULL);
		glViewport(0,0,(GLint)width,(GLint)height);
#endif
		current_ThreeDDrawing=widget;
	}
	else
	{
		printf("X3dThreeDDrawingMakeCurrent.  Missing widget\n");
	}
} /* X3dThreeDDrawingMakeCurrent */

void X3dThreeDDrawingAddReadContext(GLXDrawable read_drawable)
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Makes the current ThreeDDrawing the current GL destination and the 
supplied drawable the current GL source.
==============================================================================*/
{
	ThreeDDrawingWidget drawing_widget;
#if defined (OPENGL_API)
#endif

	/* check arguments */
	if (read_drawable && current_ThreeDDrawing)
	{
		drawing_widget=(ThreeDDrawingWidget)current_ThreeDDrawing;
#if defined (GLX_SGI_make_current_read)
		if (!glXMakeCurrentReadSGI(XtDisplay(current_ThreeDDrawing),
			XtWindow(current_ThreeDDrawing),
			read_drawable,
			(drawing_widget)->three_d_drawing.normal_buffer.rendering_context))
		{
			printf("X3dThreeDDrawingMakeCurrent.  Unable to make current read and draw drawables\n");
		}
#endif /* defined (GLX_SGI_make_current_read) */
	}
	else
	{
		printf("X3dThreeDDrawingMakeCurrent.  Missing widget\n");
	}
} /* X3dThreeDDrawingMakeCurrent */

void X3dThreeDDrawingRemakeCurrent(void)
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Change the context to the last ThreeDDrawing that was made current
==============================================================================*/
{
	ThreeDDrawingWidget drawing_widget;

	/* check arguments */
	if (current_ThreeDDrawing)
	{
		drawing_widget=(ThreeDDrawingWidget)current_ThreeDDrawing;
#if defined (OPENGL_API)
		glXMakeCurrent(XtDisplay(current_ThreeDDrawing),XtWindow(current_ThreeDDrawing),
			(drawing_widget)->three_d_drawing.normal_buffer.rendering_context);
#endif
	}
	else
	{
		printf("X3dThreeDDrawingRemakeCurrent.  Missing widget\n");
	}
} /* X3dThreeDDrawingRemakeCurrent */

Widget X3dThreeDDrawingGetCurrent(void)
/*******************************************************************************
LAST MODIFIED : 26 April 2000

DESCRIPTION :
Returns the current X3d drawing widget; non-NULL if OpenGL already in-use.
==============================================================================*/
{
	Widget widget;

	widget=current_ThreeDDrawing;

	return (widget);
} /* X3dThreeDDrawingGetCurrent */

void X3dThreeDDrawingSwapBuffers(void)
/*******************************************************************************
LAST MODIFIED : 12 February 1995

DESCRIPTION :
Swaps the buffers current widget.
==============================================================================*/
{
	if (current_ThreeDDrawing)
	{
#if defined (GL_API)
		swapbuffers();
#endif
#if defined (OPENGL_API)
		glXSwapBuffers(XtDisplay(current_ThreeDDrawing),
			XtWindow(current_ThreeDDrawing));
#endif
	}
	else
	{
		printf("X3dThreeDDrawingSwapBuffers.  Missing current\n");
	}
} /* X3dThreeDDrawingSwapBuffers */

void X3dThreeDDrawingCleanUp(Display *display)
/*******************************************************************************
LAST MODIFIED : 17 November 1997

DESCRIPTION :
Routine for cleaning up any dynamic module variables created with the
Three_D_Drawing, eg. OpenGL shareable_context.
???RC not used yet??
==============================================================================*/
{
	if (display)
	{
#if defined (OPENGL_API)
		/* destroy the OpenGL sharable_context - if defined */
		if (shareable_context)
		{
			glXDestroyContext(display,shareable_context);
		}
#endif
	}
	else
	{
		printf("X3dThreeDDrawingCleanUp.  Missing display\n");
	}
} /* X3dThreeDDrawingCleanUp */

#if defined (OPENGL_API)
GLXContext X3dThreeDDrawingGetGLXContext(Widget widget)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
???DB.  Used in Scene_viewer for playing movies.  Would rather hide movie
playing in here
==============================================================================*/
{
	ThreeDDrawingWidget drawing_widget;
	GLXContext context;

	if ((drawing_widget=(ThreeDDrawingWidget)widget)&&
		(True==IsThreeDDrawing(widget)))
	{
		context=(drawing_widget)->three_d_drawing.normal_buffer.rendering_context;
	}
	else
	{
		printf("X3dThreeDDrawingGetGLXContex.  Missing widget\n");
		context=(GLXContext)NULL;
	}

	return (context);
} /* X3dThreeDDrawingGetGLXContex */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API) && defined (MOTIF)
int query_glx_extension(char *extName, Display *display, int screen)
/*******************************************************************************
LAST MODIFIED : 14 September 1998

DESCRIPTION :
Search for extName in the GLX extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from above
==============================================================================*/
{
	char *end,*p;
	int extNameLen, n;
	int return_code;

	/* check arguments */
	if (extName)
	{
		extNameLen=strlen(extName);
		p=(char *)glXQueryExtensionsString(display,screen);
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
} /* query_glx_extension */
#endif /* defined (OPENGL_API) && defined (MOTIF) */
